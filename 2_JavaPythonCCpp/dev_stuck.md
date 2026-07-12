已在 `bugfix/stuck` 分支完成排查与修复。

## Bug 分析

遇障不重规划的核心原因有三处：

### 1. 重规划被 `sendIndex != stepId` 条件拦截（主因）

`detour()` 中只有当 `sendIndex == stepId`（路径已全部下发且车已走到当前段）才触发重规划：

```422:424:core/module/traffic/src/traffic_control_service_regulation.cpp
if ((control->sendIndex > -1) && (control->sendIndex != control->stepId)) {
    continue;
}
```

机器人遇障时通常 `sendIndex > stepId`（前方路段已下发但未走完），因此重规划被直接跳过。

### 2. 故障绕障目标车辆搞反

`FaultVehicleDetourTrigger` 在故障绕障时把**故障车**加入了 `collisionVehicles`，而真正需要重规划的是**前方受阻的行驶车**：

```106:108:core/module/traffic/include/traffic/detour/vehicle_detour_trigger_fault.hpp
info->addCollisionVehicle(std::get<1>(entry.second));  // 错误：加了故障车
info->addDetourPosition(std::get<1>(entry.second)->positionId());
```

### 3. 障碍检测不完整

原逻辑只检查告警码 `13`，未覆盖 `meta.blocked` 标志；部分机器人遇障只上报 `blocked=true` 而不带告警 13。

---

## 修复方案

| 文件                                       | 改动                                                                     |
| ---------------------------------------- | ---------------------------------------------------------------------- |
| `vehicle_detour_replan.hpp`（新增）          | 提取 `isObstacleBlockedVehicle()`、`shouldAllowDetourReplan()` 可测辅助函数     |
| `vehicle_detour_trigger_fault.hpp`       | 同时识别 `blocked` + 告警 13；故障绕障改为对**行驶车**重规划；移除 `stepId < sendIndex` 的错误拦截 |
| `traffic_control_service_regulation.cpp` | 遇障/故障场景绕过 `sendIndex` 限制，允许中途重规划                                       |

---

## 单元测试

新增 `test_vehicle_detour_replan.cpp`，覆盖 9 个断言：

- `blocked` 标志 / 告警码 13 识别
- 遇障时 `sendIndex > stepId` 仍允许重规划
- 死锁/慢速绕障仍保持原有 `sendIndex == stepId` 约束

```bash
./module/traffic/unit_tests/unit_tests_vehicle_detour_replan "[traffic][detour]"
# All tests passed (9 assertions in 2 test cases)
```

`matrix_core` 编译通过。改动尚未提交，需要的话可以帮你提交。













要在命令行跑这个测试，先编译，再在 `core/build` 目录下执行。

## 1. 编译

```bash
cd /home/wp/Documents/Matrix/core/build
cmake --build . --target unit_tests_vehicle_detour_replan -j$(nproc)
```

## 2. 运行

**跑全部用例：**

```bash
cd /home/wp/Documents/Matrix/core/build
./module/traffic/unit_tests/unit_tests_vehicle_detour_replan
```

**只跑 `[traffic][detour]` 标签（推荐）：**

```bash
./module/traffic/unit_tests/unit_tests_vehicle_detour_replan "[traffic][detour]"
```

**或用绝对路径：**

```bash
/home/wp/Documents/Matrix/core/build/module/traffic/unit_tests/unit_tests_vehicle_detour_replan "[traffic][detour]"
```

## 3. 常用 Catch2 参数

```bash
# 列出所有用例
./module/traffic/unit_tests/unit_tests_vehicle_detour_replan --list-tests

# 只跑某个用例名
./module/traffic/unit_tests/unit_tests_vehicle_detour_replan "detour:shouldAllowDetourReplan"

# 显示成功用例详情
./module/traffic/unit_tests/unit_tests_vehicle_detour_replan "[traffic][detour]" -s
```

如果提示找不到文件，说明还没编译成功，先执行第 1 步。


