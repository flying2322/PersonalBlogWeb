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

`scene_manager.cpp` 第 908 行误留的 `+`（导致编译失败）已去掉。

核心场景（重启后车已在目标点、FREE、订单仍 RUNNING）现在会：恢复绑定 → 重挂 traffic → 空 Control → 同点完成 → 订单结束。
