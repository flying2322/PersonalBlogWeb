core_charge_turnAround

已按「**永远后退对接充电桩**」的逻辑完成优化，核心变化如下。

## 问题

原逻辑在 dock 点用 `rotateYaw += π` 做粗暴调头，没有和 **dock→charge 线段的几何方向** 对齐，也无法处理「到达 dock 时已对齐 → 无需旋转」的情况。

## 新逻辑

新增 `vehicles/charge_dock_utils.hpp`，统一计算后退对接朝向：

```cpp
// 线段正向（dock → charge）
forwardYaw = angle(dock, charge)

// 后退对接时车头应朝向（与线段正向相反）
backwardDockYaw = normalize(forwardYaw + π)
```

对应你举的例子：若 dock 出度方向是 **π**，则 `backwardDockYaw ≈ 0`；车辆若已以 π 到达 dock，需旋转 `|π - 0| = π`；若已以 0 对齐后退方向，则 **无需旋转**。

## 修改点

### 1. `addRouteToControl` — 规划时写入 dock 的 travelPose

最后一段为 **BACKWARD + CHARGEPOINT** 时，dock 点的 `targetYaw` 设为 `backwardDockYaw`，不再用线段正向角。

### 2. `updateRoutes` — 触发 dock 旋转的条件

仅在 **CHARGE 任务 + 最后一段 BACKWARD 到充电点** 时设置 `turnAroundAfterArrived`（表示需要在 dock 做后退对齐旋转）。

### 3. `vehicle_command_sender` — 下发旋转指令

去掉原来的 `rotateYaw += π`，改为：

- 目标角 = `dockRotateTargetYaw(dock→charge线段)` = `backwardDockYaw`
- 到达角 = travelPose 的 `sourceYaw`（或车辆当前 yaw）
- 仅当 `|差值| > rotateRadLimit` 时才下发旋转
- 线段方向设为 `BACKWARD`

实车仍保持原流程：dock 处只发旋转，不发最后一段线，到位后 `startCharge`。

## 流程示意（T3a: P_54 → P_53）

```
P_55 ──forward──> P_54(dock) ──rotate to backwardDockYaw──> startCharge(backward)
                      ↑
              对齐方向 = reverse(L_54_53 正向)
              不是盲目 +180°
```

## 测试

`test_auto_charging_route.cpp` 已更新，验证：

- `backwardYaw = forwardYaw + π`
- 已对齐时不旋转
- 仅朝线段正向到达时需旋转

全部 61 个断言通过。
