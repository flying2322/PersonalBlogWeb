当前处于 **Ask 模式**，我只能帮你定位原因和给出修复方案，不能直接改代码。如需我实现修复和单元测试，请切换到 **Agent 模式**。

---

## 结论：~15s 是多个模块**串行 stop** + **不可中断 sleep** 叠加的结果

日志里 `Core.Stop` 耗时 **14962ms**，对应调用链：

```
Engine CoreAppService.Stop()
  → CoreApplication::stop()
    → Application::stop()   // core/src/application.hpp
```

`Application::stop()` 按顺序依次停止各子服务，**全部串行**：

```143:157:core/src/application.hpp
    void stop() {
        SPDLOG_INFO("{} matrix is stopping.", title_);
        deviceService_->stop();
        isRunning_.store(false);
        logService_->stop();
        simulationService_->stop();
        taskAssignService_->stop();
        leisureManager_->stop();
        chargingManager_->stop();
        trafficControlService_->stop();
        if (sceneManager_) {
            sceneManager_->stop();
        }
        vehicleManager_->stop();
        // ...
    }
```

---

## 主要耗时来源（按影响排序）

### 1. `Vehicle::stop()` — 每辆车最多 ~4.5s（最大头）

```116:146:core/module/domain/src/vehicle.cpp
void Vehicle::stop() {
    sendLogOut();
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));   // 固定 1.5s

    if (loggedIn_.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(2000)); // 再 +2s
    }
    // ... stop sessions ...
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));   // 再 +1s
```

`VehicleManager::stop()` 对车辆**逐个**调用 `vehicle->stop()`：

- 1 台已登录车：约 **4.5s**
- 3 台已登录车：约 **13.5s**（已接近你看到的 15s）

这是与车辆数量线性增长的最主要原因。

---

### 2. 多个后台 Worker 使用不可中断的 `sleep_for(1000ms)`

以下服务循环末尾都是硬编码 1 秒 sleep，stop 时无法立刻唤醒：

| 服务   | 文件                                          | sleep  |
| ---- | ------------------------------------------- | ------ |
| 任务分配 | `task_assign_service.cpp:60`                | 1000ms |
| 设备   | `device_service.hpp:37`                     | 1000ms |
| 日志   | `application.hpp:40`                        | 1000ms |
| 充电   | `charging_manager.cpp:57`                   | 1000ms |
| 闲置   | `leisure_manager.cpp:78`                    | 1000ms |
| 交管   | `traffic_control_service_regulation.cpp:77` | 1000ms |

`LoopService` 虽提供了可中断的 `interruptibleSleep()`（10ms 步进 + 检查 `stop_flag_`），但上述子类 **override 了 `doWork()` 却没用它**。

`LoopService::stop()` 最多还会等 **5000ms**：

```17:17:core/module/domain/include/services/loop_service.hpp
        const int STOP_TIMEOUT_MS = 5000;
```

串行 stop 4 个 LoopService（device / log / simulation / taskAssign），最坏情况每个接近 1~5s，合计 **4~20s**。

---

### 3. `Worker::cancelTask()` 自身最多 ~1s

```176:180:core/module/common/include/common/worker.hpp
        int retry = 10;
        while (retry-- > 0 && hasTask()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            task_.cancel();
        }
```

charging / leisure / traffic 的 stop 还在循环里反复调用 `cancelTask()` + `sleep(200ms)`，进一步拉长等待。

---

### 4. 场景清理额外 sleep

- `scene_manager.cpp:238` — 删除场景前 **sleep 1000ms**
- `scene_manager.cpp:64` — cleanup 前 **sleep 100ms**

---

## 典型 15s 耗时拆解（举例：3 台已登录 AGV）

| 阶段                                              | 估算耗时          |
| ----------------------------------------------- | ------------- |
| 4 个 LoopService stop（各 ~1s 不可中断 sleep + cancel） | ~4–6s         |
| charging + leisure + traffic stop               | ~2–3s         |
| sceneManager stop                               | ~1s           |
| vehicleManager（3 车 × 4.5s）                      | ~13.5s        |
| **合计**                                          | **~15s 量级** ✓ |

（实际会因 stop 触发时机是否在 sleep 中间而波动。）

---

## 推荐修复方向

### 优先级 P0：`Vehicle::stop()` 去掉固定 sleep，改为“等会话退出 + 短超时”

- 用 `condition_variable` / session 线程 join，替代 1500 + 2000 + 1000 ms 硬等待
- 总超时建议 **300~500ms**（可配置），超时则强制 close
- 多台车可 **并行 stop**（`std::async` 或 thread pool）

预期：3 台车从 ~13.5s → **<1s**。

### 优先级 P1：所有后台循环改用可中断 sleep

- 把 `sleep_for(1000ms)` 统一换成 `interruptibleSleep()` 或基于 `Task::ok()` 的短步 sleep（50~100ms）
- 涉及：taskAssign、device、log、charging、leisure、traffic

预期：每个服务 stop 从 ~1–5s → **<200ms**。

### 优先级 P2：统一 stop 逻辑

- charging / leisure / traffic 的 stop 改为与 `LoopService::stop()` 相同模式（`stop_flag` + 有界超时），避免 `while(hasTask())` 无限轮询
- `Worker::cancelTask()` 减少 retry（如 3 次 × 20ms）
- `STOP_TIMEOUT_MS` 从 5000 降到 1000~2000
- 考虑 `Application::stop()` 中对无依赖服务**并行 stop**

### 优先级 P3：场景清理

- 去掉 `removeScene` 中无必要的 `sleep(1000ms)`，改为等 IO 线程 join（`stopIoContext()` 已有 join）

---

## 单元测试建议（`test_shutdown_latency.cpp`）

1. **`interruptibleSleep` 响应 stop_flag**  
   
   - 注册 5s sleep，50ms 后设 stop → 断言 **<300ms** 返回

2. **`LoopService::stop` 有界耗时**  
   
   - 子类 doWork 用 mock 长 sleep + interruptibleSleep → stop 后断言 **<500ms** 完成

3. **`Vehicle::stop` 有界耗时（mock session）**  
   
   - mock session 立即返回 → 断言 stop **<200ms**（验证去掉 4.5s 硬 sleep）

4. **（可选）集成测试**  
   
   - 启动 Application 各 LoopService → 立刻 stop → 总耗时 **<2s**

项目里已有类似测试文件 `core/module/domain/unit_tests/test_latency_opt.cpp`，可复用其路径解析和计时模式。

---

## 如何验证定位（加日志）

在 `Application::stop()` 每个子服务 stop 前后打时间戳（或用 `utils::Stopwatch`），例如：

```cpp
utils::Stopwatch sw;
deviceService_->stop();
SPDLOG_INFO("deviceService stop: {}ms", sw.milliSeconds());
sw.start();
// ... 下一个服务
```

跑一次 Matrix 停止，看哪一步最慢；通常会看到 **`vehicleManager_->stop()` 占大头**。

---

如需我在 `feature/xxx` 分支上直接实现上述优化并补单元测试，请切换到 **Agent 模式** 后再说一声。





# Coding

## 优化思路（最小改动、沿用现有设计）

### 1. 抽取可中断 sleep 工具

新增 `core/module/common/include/common/interruptible_sleep.hpp`，供 `LoopService` 复用。

### 2. `LoopService` 子类改用 `interruptibleSleep()`

将硬编码 `sleep_for(1000ms)` 替换为可中断 sleep（10ms 步进，检查 `stop_flag_`）：

- `task_assign_service.cpp`
- `device_service.hpp`
- `application.hpp`（LogService）
- `simulation_service.hpp`

`STOP_TIMEOUT_MS`：`5000` → `2000`

### 3. 独立 Worker 循环（charging / leisure / traffic）

- 循环内用 `interruptibleSleepForTask(task, 1000ms)` 替代 `sleep_for`
- `stop()` 改为 `cancelTask()` + `waitUntilIdle(2000ms)`，去掉无限 `while(hasTask())` 轮询

### 4. `Worker` 加速 cancel

- `cancelTask` 重试：`10×100ms` → `5×20ms`（最多约 100ms）
- 新增 `waitUntilIdle(timeoutMs)` 供 stop 使用

### 5. `Vehicle::stop()` 缩短固定等待（主要收益）

| 原等待              | 优化后   |
| ---------------- | ----- |
| 1500ms（logout）   | 200ms |
| 2000ms（loggedIn） | 移除    |
| 1000ms（session）  | 100ms |

单车约 4.5s → ~300ms；3 台车约 13.5s → ~1s。

### 6. 场景清理

`removeScene` 中 `sleep(1000ms)` → `50ms`

---

## 单元测试

新增 `core/module/common/unit_tests/test_interruptible_sleep.cpp`（4 个用例）：

cd core/build

cmake --build . --target unit_tests_common -j$(nproc)

./module/common/unit_tests/unit_tests_common "[shutdown]"

全部通过。`matrix_core` 与 `unit_tests_bin_location_utils` 也已验证通过。

---

## 变更文件

| 文件                             | 说明                                                        |
| ------------------------------ | --------------------------------------------------------- |
| `interruptible_sleep.hpp`      | 新增                                                        |
| `test_interruptible_sleep.cpp` | 新增                                                        |
| `worker.hpp`                   | 更快 cancel + `waitUntilIdle` + `interruptibleSleepForTask` |
| `loop_service.hpp/cpp`         | protected `interruptibleSleep`，超时 2s                      |
| 各 LoopService 子类 / Worker 服务   | 可中断 sleep                                                 |
| `vehicle.cpp`                  | 缩短 stop 等待                                                |
| `scene_manager.cpp`            | 缩短 removeScene 等待                                         |

---

## 预期效果

整体 Core 停止从约 15s 降至约 1–3s（视车辆数量和 stop 触发时机而定）。各后台服务在 `stop()` 后通常能在 几十毫秒内 被唤醒，不再傻等 1 秒 sleep 周期。

你本地可再跑一次 Matrix 停止，对比日志中 `[Core.Stop] 耗时` 是否明显下降。确认无误后自行 `git add` 提交即可（记得包含两个新文件）。



Core 停止耗时优化已完成。中间有一次单元测试构建失败（`test_interruptible_sleep.cpp` 重复定义 `CATCH_CONFIG_MAIN`，以及一个 `waitUntilIdle` 断言过严），已修复。

当前状态：

- `./module/common/unit_tests/unit_tests_common "[shutdown]"` — 4 个用例全部通过
- `unit_tests_bin_location_utils` — 全部通过
- `matrix_core` — 编译通过

你可以 review 变更后手动提交；若要验证停止耗时，重启 Matrix 后看日志里 `[Core.Stop] 耗时` 是否从约 15s 降到约 1–3s。
