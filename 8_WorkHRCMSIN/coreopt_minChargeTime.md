## 好的，基于以上分析，彻底修复之前因为车端状态跳变导致的充不够时间就提前离开充电桩的问题，要求只要车辆minChargeTime未到，ChargedOK未到任一条件不满足都不要离开充电桩，请对照当前 feat/coreopt 分支给出具体补全 diff 方案



## 修复已完成

已在 `feat/coreopt` 上补全 minChargeTime 修复，构建与单测均通过。

### 改动概览（6 个文件 + 1 个新文件）

| 文件                                                                    | 作用                                                                                            |
| --------------------------------------------------------------------- | --------------------------------------------------------------------------------------------- |
| **`core/module/domain/include/battery/charge_session_policy.hpp`**（新） | 充电会话策略：桩位会话同步、`canLeaveChargePoint` / `mustStayOnChargePoint`                                 |
| **`vehicle_realtime_data.hpp/cpp`**                                   | 新增 `chargeSessionStartTime` / `chargeSessionPositionId`；`charging:false` 抖动时若会话仍在桩上则**不清零**计时 |
| **`task_assigner_optimize.cpp`**                                      | 派单前同步会话；**FREE 分支前**拦截：在桩且未满足 minChargeTime/chargedOk 时不接非 CHARGE 单                           |
| **`task_assign_service.cpp`**                                         | 下发层：`charging \|\| onChargePoint` 时检查；软回充抢占也需满足离桩条件                                           |
| **`charging_manager.cpp`**                                            | `stopCharge` 仅在 `canLeaveChargePoint` 时执行；让位同样要求满条件                                           |
| **`traffic_control_service_regulation.cpp`**                          | 路径下发前：在桩且未满足条件则 **hold movement**，不发 stopCharge/路线                                            |

### 核心设计

**1. 桩位会话（抗 `charging` 跳变）**

```cpp
// 在 CHARGEPOINT 上建立 sticky session，离开桩位才清除
syncChargeSession(meta, mapManager, mapId, positionId, now);
```

- 进入 `CHARGEPOINT` → 记录 `chargeSessionStartTime`
- 车端 `charging:false` 抖动 → **会话和计时保留**
- 离开桩位 → 清除会话

**2. 离桩条件（与需求一致）**

```cpp
canLeaveChargePoint = (battery >= chargedOk) AND (elapsed >= minChargeTime * 60)
```

任一不满足 → 不接单、不下发、不 stopCharge、不让位。

**3. 三层门禁**

```
派单 (canWorkVehicles)     → 在桩且 mustStay → continue
下发 (dispatchBlockTasks)  → 在桩且 !canLeave → 阻塞 waitVehicles
交管 (updateRoutes)        → 在桩且 mustStay → hold movement
停充 (charging_manager)    → !canLeave → 不 stopCharge
```

### 验证

```
unit_tests_battery_policy        — 10 assertions passed
unit_tests_charge_session_policy — 7 assertions passed
domain / task_assign / task_generate / traffic — 编译通过
```

### 建议回归场景

1. 自动上桩 → `FREE + charging:true`，minChargeTime 内下发 MOVE → 应看到 `minChargeTime/chargedOk not met`，无路线下发  
2. 模拟 `charging:false` 抖动（仍在桩位）→ 计时不清零，仍不可离桩  
3. 满 10 分钟且电量 ≥ chargedOk → 正常接单、stopCharge、下发路线  

如需我帮你整理 commit message 或补充集成测试用例，可以说一下。
