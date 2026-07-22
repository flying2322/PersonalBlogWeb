## 结论

R4 不是没分到去 `K_P_102` 的任务，而是**离开工作点 `K_P_42`（WorkMarker→ACTIONPOINT）时路径下发被拒**，随后 `sendIndex` 已推进却未真正下发，车辆永久卡在工作点。

## 日志证据

| 时间       | 现象                                                              |
| -------- | --------------------------------------------------------------- |
| 18:21:15 | UNLOAD 规划成功：`K_P_42 → … → K_P_102`，block 已进交管                   |
| 18:21:15 | **ERROR**: `rotationAllowed:true, but position is actionPoint`  |
| 之后       | `step:-1, send:4, limit:5`，`localPath size:0`，无任何 `addLinePath` |
| 全程       | R4 停在 `K_P_42`，订单 UNLOAD 一直 RUNNING                             |

根因链路：

1. `applyRoadLines` 先把 `sendIndex` 从 `-1` 推到 `4`
2. 因在 ACTIONPOINT 且道路 `rotationAllowed=true`，直接 `continue`，**不下发**
3. `passedRoadLineCount_=3` 导致后续再也无法重新下发 → 永久占住工作点

## 修复（已实施，待你审查）

文件：`core/module/traffic/src/traffic_control_service_regulation.cpp`

1. **离开工作点**：仅当 `rotationAllowed=false` 时倒车离开；允许旋转时改为**前进+旋转**正常下发（不再 abort）
2. **恢复**：若 FREE、无 local 路径、`sendIndex` 已推进但仍在路线起点 → 重置 `sendIndex=-1` 允许重发

`matrix_core` 已编译通过。请重启 Engine 验证 R4：LOAD 完成后应出现 `leave actionPoint K_P_42 via rotationAllowed road, dispatch forward`，并驶向 `K_P_102`。

需要我提交的话直接说即可。
