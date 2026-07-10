## 问题根因

`TaskType::MOVE` 且终点为 **充电点（CHARGEPOINT）** 时，Core 仍按普通移动任务处理，未走自动回充（`TaskType::CHARGE`）的专用调度链路：

1. **未自动开启 recognize / recognizeFile** → 不会在 dock 点分段下发路径  
2. **未设置 dockId** → 一次性把 dock→charge 的 **BACKWARD** 段和其他段一起下发  
3. **未在 dock 点做倒车朝向处理**（`turnAroundAfterArrived` 仅对 `CHARGE` 生效）  
4. **`vehicle_command_sender` 额外要求 `taskType == CHARGE`**，即使上层设置了 `turnAroundAfterArrived` 也不会执行倒车旋转  

车端收到朝向/连接不一致的连续路径，就会报 **error code 5（路径不连续）**。自动回充正常，是因为 `ChargingManager` 创建的是 `TaskType::CHARGE` 且带 `recognize=true`。

---

## 修复方案

引入统一判断：**MOVE + 充电点目标 = 走充电调度流程**，但 block 类型仍保持 `MOVE`（满足你的预期）。

### 核心改动

| 文件                                             | 改动                                                               |
| ---------------------------------------------- | ---------------------------------------------------------------- |
| `charge_dock_utils.hpp`                        | 新增 `shouldUseChargeDispatch`、`enrichBlockForChargeGoal` 等 helper |
| `traffic_control_service_regulation.cpp`       | dock 分段、倒车、startCharge、完成判定均扩展到 MOVE+充电点                         |
| `vehicle_command_sender.hpp`                   | 移除 `turnAroundAfterArrived` 对 `TaskType::CHARGE` 的硬限制            |
| `task_assign_service.cpp`                      | MOVE 到充电点也占用充电位                                                  |
| `core_application.cpp` / `http_server_api.cpp` | addBlocks 时自动补全 recognize 参数                                     |
| `test_auto_charging_route.cpp`                 | 新增 MOVE→充电点单元测试                                                  |

### 修复后的调度流程（与自动回充一致）

```
规划路径 → ... → dock点(T3a_P_54) → 充电点(T3a_P_53)
              ↓
         先到 dock，下发 startCharge + recognize
              ↓
         倒车进充电位，等待 charging=true 后任务完成
```

---

## 验证

已编译通过，单元测试通过：

```bash
cd core/build
./module/planning/unit_tests/unit_tests_planning "[move]"
# All tests passed (21 assertions in 1 test case)
```

---

## 请你实车/联调时关注

下发 MOVE 到充电点后，Core 日志应出现：

- block 仍为 `MOVE`，但 `recognize=true`、`recognizeFile` 来自地图  
- 路径在 **dock 点** 分段（`dockId` 被设置）  
- 到达 dock 后调用 `startCharge`  
- 不再出现 error code 5  

代码已改好，**未提交**，请你审核后自行提交。若联调还有问题，把 Core 日志里 `travel poses`、`pathSize`、`dockId` 相关片段贴过来即可继续排查。
