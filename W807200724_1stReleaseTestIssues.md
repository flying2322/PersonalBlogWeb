系统恢复链路已按状态补齐，`matrix_core` 编译通过。

### 恢复链路（按系统状态重建调度）

1. **`loadOrders` → `restoreVehicleRuntimeState`**
   
   - 绑定 `orderId` / `blockTaskId` / `taskType`
   - 活跃订单将 `orderTake` 置为 `AVAILABLE`；PAUSE 保持 `UNAVAILABLE`

2. **`dispatchBlockTasks`**
   
   - `vehicleNeedsActiveTaskRecovery`：有 CREATED/DISPATCHED/RUNNING 且 traffic 未挂载时强制恢复
   - `orderTake` 不可用时仍允许恢复重挂

3. **`dispatchTask`**
   
   - RUNNING/DISPATCHED：只重新 `addVehicleTask`，不回写成 DISPATCHED

4. **Traffic**
   
   - `updateBlockTasks`：RUNNING 且无 Control 时重建
   - `runBlockTask`：已在目标点且 FREE → 建空 Control，交给同点完成路径收尾/下发操作；否则重新规划

### 附带修复

核心场景（重启后车已在目标点、FREE、订单仍 RUNNING）现在会：恢复绑定 → 重挂 traffic → 空 Control → 同点完成 → 订单结束。

---

# 不接单

**结论先说：本次未提交的 801/1008 告警修复，本身不会改写“接单/不接单”状态，也不会导致分配算法突然拒车。**  
现场“车不动 / 经常被设成不接单”，更吻合 **Matrix 既有的 `setOrderTakeState` 调度语义 + Engine 终止运单默认 `disableVehicle=true`**，而不是这次告警条件态改动引入的回归。

---

## 1. 这次未提交改动到底改了什么

工作区相关 diff 实质只有：

- 新增 `order_take_faults.hpp` → `syncOrderTakeFaults()`
- 把原来散落的 `warn(801/1008)` / `clear(801/1008)` 换成统一同步
- **没有**改 `canWorkVehicles` 的“只有 AVAILABLE 才可分配”过滤
- **没有**新增/改变 `setOrderTakeState(...)` 的赋值规则（只在原有赋值后多调了一次告警同步）

```9:21:core/module/fault_diagnosis/src/order_take_faults.hpp
// 801 / 1008 are condition-state alarms for explicit "not accepting orders" (INACTIVE).
// AVAILABLE clears both. UNAVAILABLE (e.g. paused holding resource) does not raise these.
inline void syncOrderTakeFaults(...) {
    if (AVAILABLE) { clear 801/1008; return; }
    if (INACTIVE)  { warn 801/1008; }
    // UNAVAILABLE: 既不 raise 也不 clear
}
```

所以：

| 现象             | 是否由本次 801/1008 修复引起                |
| -------------- | ---------------------------------- |
| 车被设成不接单        | **否**（状态赋值逻辑未改）                    |
| 任务等机器人、车看起来可分配 | **基本否**（分配过滤条件未变）；更可能是别的过滤或状态显示不一致 |
| 801/1008 告警显示  | **有关**（语义收窄：只对 INACTIVE 告警）        |

---

## 2. 接单状态语义（Matrix 调度侧）

```104:108:core/module/domain/include/vehicle_types.hpp
INACTIVE   = 0  // 不可接单，占资源
AVAILABLE  = 1  // 可接单
UNAVAILABLE= 2  // 不接单，不占资源
```

**谁会改状态（与本次告警修复无关，是原逻辑）：**

| 入口                                                                     | 结果                           |
| ---------------------------------------------------------------------- | ---------------------------- |
| 车辆默认构造                                                                 | **INACTIVE**                 |
| `cancelOrder` / `TerminateOrder(disableVehicle=true)`                  | **INACTIVE**                 |
| `cancelOrder(..., false)`                                              | AVAILABLE                    |
| `pauseOrder`                                                           | UNAVAILABLE                  |
| `resumeOrder`                                                          | AVAILABLE                    |
| 恢复进行中运单                                                                | AVAILABLE；暂停运单 → UNAVAILABLE |
| API `modifyVehicleOrderTaskState` / HTTP `modifyVehicleOrderTakeState` | 按入参 0/1/2                    |

分配侧：

- `canWorkVehicles`：`!= AVAILABLE` 直接 `continue`（不接单）
- `allControlableVehicles`：直接踢掉 **UNAVAILABLE**（不占分配池）

日志里 `order take:true/false` 只表示是否 `== AVAILABLE`，区分不了 0 和 2。

---

## 3. 问题2：为什么经常被设成不接单 —— 是 Matrix 调度行为

**是的，Matrix 会主动设不接单**，最常见路径：

1. Engine 任务异常/终止时：
   
   ```353:353:engine/.../TaskExecuteService.cs
   TerminateOrder(new() { OrderId = order.OrderId, DisableVehicle = true });
   ```

2. 人工 Abort 默认也是 `disableVehicle = true`（`OrderController`）

对应 Core：

```98:98:core/module/task_assign/src/task_assign_service.cpp
setOrderTakeState(disableVehicle ? INACTIVE : AVAILABLE);
```

你们当天日志也能对上：`11:17:34` 对 `7ebaa580...` 终止且 `disableVehicle:true` 之后，`S800_4#` 长期：

`order take:false` + `FREE` + 无 order  

这不是 801/1008 告警代码写坏了，而是 **终止运单故意把车打成 INACTIVE**，之后必须再手动/接口设回可接单，否则新任务会一直“等机器人”。

另外：Core 重启后车辆默认也是 **INACTIVE**，若没人点“可接单”，会出现整场“看着连着车但分配不上”。

---

## 4. 问题1：任务等机器人，但界面显示可接单

本次告警修复**不会**把 AVAILABLE 车从分配池拿掉。若 UI 显示可接单仍分不出去，应查分配日志里的其它过滤（与 801/1008 无关），例如：

- `[not take order] ... force battery`（已提交的 autoCharging 策略）
- group / 指定车 / `vehicle is full`
- 电量、充电中、控制权非 MRCS、软急停等
- UI 与 Core 状态不同步（Engine 侧 `OrderTakeState != AVAILABLE` 才算不可用）

建议在出现“等机器人”时直接搜 core 日志：

`order no available vehicle` / `[not take order]` / `order take:false`

---

## 5. 本次修复里真正不顺的点（告警逻辑小瑕疵，不是“设不接单”）

1. **`UNAVAILABLE` 分支什么都不做**  
   若先 INACTIVE（已 raise 801/1008），再 pause → UNAVAILABLE，告警可能**残留**，看起来像仍“设置不接单”，但状态已是 2。  
   更合理：`UNAVAILABLE` 也应 `clear` 801/1008（只对 INACTIVE 保留告警）。

2. **HTTP `modifyVehicleOrderTakeState` 仍不调 `syncOrderTakeFaults`**  
   改状态后告警要等下一轮 `displayVehicleInfos`/分配循环才同步；和 `modifyVehicleOrderTaskState` 不一致。

3. **告警同步 ≠ 状态设置**  
   每秒 `display` / `dispatch` 里调 `syncOrderTakeFaults` 只会 raise/clear 告警，**不会**把车改成不接单。

---

## 6. 建议怎么验证

1. 出现“不接单”时看 Core 日志是否刚有：  
   `TerminateOrder` / `cancel order` + `disableVehicle` / `pause order` / `modifyVehicleOrderTaskState`
2. 看 `order take:` 是 `false` 还是 UI 误报；并用实际枚举 0/1/2 区分 INACTIVE vs UNAVAILABLE
3. “等机器人但可接单”时抓同一时刻的 `[not take order]` 行，不要只看 801/1008

---

**总结：**  

- 未提交的 801/1008 改动：**告警条件态整理，不是设不接单的根因**。  
- 车经常不接单：**Matrix + Engine 终止默认 `DisableVehicle=true` 的产品逻辑**（再叠加默认 INACTIVE）。  
- 要修“终止后不应禁用接单”，应改 Engine 终止参数或 Core `cancelOrder` 策略；要修告警残留，应让 `UNAVAILABLE` 也 clear 801/1008。  

我当前是 Ask 模式，不能直接改代码；若要落地修复可切 Agent 模式。

---

**目的：** 在运行态日志里周期性打出「当前跑的是哪一版 Core + 何时启动」，方便排障时不用翻到文件开头也能对齐二进制。

你看到的这行：

`matrix version:0.3.203 (aae0f3b) built on 2026-07-17 16:26:36 started date time:2026-07-23 17:20:45.905`

来自：

```230:232:core/src/application.hpp
void displayMatrixInfo() {
    SPDLOG_INFO("{} matrix version:{} started date time:{}", title_, version::getVersionString(), startedDateTime_);
}
```

由 `LogService` 每秒调一次 `displayInfos()` → 先打这行，再打车辆/运单摘要。

---

## 引入历史

| 阶段                 | 提交                                      | 说明                                               |
| ------------------ | --------------------------------------- | ------------------------------------------------ |
| 仓初始化就有同类逻辑         | `71aa21b`（2025-11-20，yangyong）          | 主循环每秒 `displayInfos()`，含 version + started time  |
| 函数形态整理             | `3c69dbf2`（2025-12-02，zhxx）「更改多车通讯为单线程」 | blame 上的 `displayMatrixInfo()`                   |
| 版本串变成「版本+git+编译时间」 | `c4d8964a`（2026-02-06，zhxx）「增加自动生成版本文件」 | `MATRIX_VERSION` → `version::getVersionString()` |

所以不是近期 801/1008 或调度改动带来的，而是很早就有的「运行态身份戳」。

---

## 有没有必要

**信息本身有必要，每秒打一遍没必要。**

有价值的一面：

- 日志按天滚动后，新文件里仍能看到版本/启动时间  
- 现场多人排障时，一眼确认是否旧二进制（你这条就能看出跑的是 7/17 编的 `aae0f3b`，不是今天刚编的）  
- 和启动时的 ASCII `displayLogo()` 互补：logo 只在启动瞬间，且版本号还可能是硬编码（如 `0.3.20.3`），和真实 `getVersionString()` 不一定一致  

浪费的一面：

- 每秒一行、内容几乎不变，长时间跑会占日志量、干扰检索  
- 和旁边每秒刷的 `vehicle info` / `order info` 叠在一起，信噪比差  

---

## 看法（建议）

保留「版本 + 启动时间」这个语义，但改成：

1. **启动时打一次完整版**（可直接用已有 `version::getBuildInfo()`）  
2. **之后降频**（例如每 5～10 分钟，或仅在日志轮转/新会话时再打）  
3. 若继续做秒级 `displayInfos`，可把 `displayMatrixInfo` 从每秒路径里拿掉，只留车辆/运单状态

**结论：** 设计意图合理（运行身份可追溯）；作为每秒心跳冗余。值得保留功能，不值得按当前频率保留。



---

# Engine break down 2nd - 0723

**结论：崩溃在 Engine 进程（含进程内嵌的 Core Native），不是独立 Core 调度算法逻辑崩掉。** 现有日志无法定位到某一行业务 if/else，但可以明确责任面和改法。

---

## 1. 责任划分

| 证据                                                      | 含义                                        |
| ------------------------------------------------------- | ----------------------------------------- |
| `Sineva.Matrix.E[26133]: segfault ... in libcoreclr.so` | 崩的是 **Engine 进程** 的 .NET CLR              |
| `./Sineva.Matrix.Engine` exit 139                       | systemd 确认是 Engine                        |
| 崩溃前无 C# 托管异常栈                                           | 不是普通 `CustomException`，是 **原生层/运行时** 写坏内存 |
| `segfault at 8` + `error 6`                             | 典型对 **空指针+偏移 8** 写（`*(null+8)=...`）       |
| Engine 已跑 `Consumed 8h 38min`                           | 长跑后偶发，像竞态/内存腐蚀，不像启动必现                     |
| `CoreAppService` 无锁、多线程直调 SWIG `CoreApplication`        | Engine↔Core 互操作高风险区                       |

架构上：Engine 通过 SWIG **进程内加载** `libmatrix_core`（`new CoreApplication()`）。因此：

- **进程归属：Engine**
- **根因可能：Engine 托管代码 / SWIG 互操作 / 嵌入式 Core 原生在并发下写坏内存，最后死在 CLR**
- **不是**：独立 Core 守护进程里某条“分配算法判错”导致的 segfault（当前 `core-20260723.log` 也是重启后才从 `17:20:45` 起写，崩前 Core 侧无独立崩溃栈）

---

## 2. 崩溃前业务时间线（Engine 日志）

```
18:20:08~09  大量 WebSocket 身份认证失败（噪声，通常不致 segfault）
18:20:23     启动 dl_task_loop_move (2d26ac08...)
18:20:23     Core.AddOrder 成功
18:20:24     分车 S800_4#，进入 Loop → RobotCommAction b3
18:20:25     Core.AddBlocks 成功 (b26d2fe3...)
18:20:25     b3 Running；机器人状态 → Running
18:20:25.765 最后一行 INFO
18:20:42     journal：segfault（约 17s 静默）
17:20:43     App starting...（重启；与 journal 差约 1h，像时区/校时，不影响先后关系）
```

崩溃窗口正好落在：**AddBlocks 成功之后、`WaitBkockFinishedAsync` 开始轮询 `QueryOrder` 的阶段**。  
此前认证失败是管理面噪声，与崩点弱相关。

---

## 3. 更可能的技术原因（按优先级）

1. **多线程无锁调用嵌入式 Core（最可疑）**  
   任务线程 `QueryOrder`/`AddBlocks` + Timer/WS/其它 API 同时进 `_coreApp`，SWIG/C++ 非线程安全 → 堆损坏 → 稍后 CLR 在 `libcoreclr.so` 空写崩溃。

2. **SWIG 对象生命周期**  
   `IDisposable` 响应/向量过早 Dispose，或跨线程再用已释放句柄。

3. **纯 CLR / 运行时问题（次要）**  
   长跑 + GC；需 dump 才能坐实，不能单靠业务日志排除。

4. **可排除为“主因”的**  
   
   - 身份认证失败（有完整托管栈，被 catch）  
   - Core 调度“分不出车/不接单”类逻辑错误（不会直接 `libcoreclr` segfault）

---

## 4. 修改方案（建议按序做）

### A. 取证（下次必做，否则难定根）

1. systemd 开 coredump：`LimitCORE=infinity`，装 `dotnet-dump` / `lldb`  
2. 崩溃后：`dotnet-dump analyze` → `clrstack` / `pe` / native bt  
3. Engine 启动加：`DOTNET_EnableCrashReport=1`，保留 `createdump`

### B. Engine 侧防护（优先改）

1. **`CoreAppService` 全局串行化 Native 调用**  
   
   - 对所有 `_coreApp.*` 包一层 `lock` 或单线程同步上下文  
   - 先验证能否消除长跑 segfault（即使略损吞吐）

2. **清点 SWIG `using`/Dispose**  
   
   - 保证响应对象在同线程创建、用完即弃，不缓存裸句柄  

3. **RobotCommAction 仍建议修**（Loop + `LastOrDefault`）  
   
   - 不直接解释本次 segfault，但同场景（loop_move + Wait）刚发生过逻辑异常，应一并修，减少异常路径压力  

### C. Core 侧（仅当 dump 指向 native）

1. 给 `CoreApplication` API 入口加线程断言/可重入锁（与 Engine 锁二选一，避免双重）  
2. ASan/TSan 编一版压测：`AddBlocks` + 多线程 `queryOrder`

### D. 运维缓解

1. 监控 Engine RSS/句柄，异常上涨则轮转重启  
2. 认证失败刷屏单独治（别和崩溃绑死）  
3. 统一 Engine 与 journal 时区，避免再出现 18:20 vs 17:20 对时困扰  

---

## 5. 一句话

**这是 Engine（进程内 Core Native）崩溃，不是 Core 算法业务逻辑“算错”；业务日志只说明崩在 loop_move/`AddBlocks` 后的等待轮询窗口。**  
下一步最有价值的是：**coredump + 给 `CoreAppService` 加全局互斥**，用运行证据确认是否并发互操作导致。

若要我在工程里直接加 `CoreAppService` 全局锁并改 RobotCommAction，切换到 Agent 模式即可。



---

# Control Right - 0724



已改完，`matrix_core` 编译通过。行为约束如下。

## 行为

| 场景                     | 是否发 `cancelMovement`                   |
| ---------------------- | -------------------------------------- |
| Care 稳定持权、正常遥控移动       | **否**（无 MRCS 边沿，不发）                    |
| Care 抢占 / 控制权从 MRCS 丢失 | **是**（边沿一次性：pause + cancel）            |
| 主动释放控制权                | **是**（释放前先 cancel，再 release）           |
| 手动终止运单                 | **是**（原有 `stopVehicleOnTaskCancel` 不变） |

## 关键改动

1. **`controlRightChanged` 改为 sticky**  
   仅在 MRCS↔非 MRCS 边沿置位，后续状态帧不再清掉；由 `dispatchBlockTasks` 处理后 `clearControlRightChanged()`。

2. **丢权边沿**（`task_assign_service.cpp`）  
   `pauseOrder` + `sendCancelMovement()`，然后清标志。稳定无控制权时只告警，不重复发指令。

3. **主动释放**（`vehicle_manager.cpp`）  
   仍持有控制权时先 `sendCancelMovement()`，再 `sendReleaseControlRight()`。

4. **`pauseVehicleTask`**  
   CREATED/DISPATCHED 路径也会 `cancelMovement`，避免已下发路径停不住。

验证时可看日志是否出现：`MRCS control lost, pause order and cancelMovement`，以及对应的 `doCancelMovement`。



46.85, yaw=3.13778)
10724 12:02:48.825692
10724 12:02:48.891184
10724 12:02:48.891305
6910 CareRcsCommandHandlers.cpp:811] [Care MappingStatus] mapping_
696 TaskQueue.cpp:539] ExecMotionTask: task nav motioin status running
695 MatrixHandlers.cpp:244] REQUEST_LOGIN
695 MatrixHandlers.cpp:262] USER ->_[ admin : log
