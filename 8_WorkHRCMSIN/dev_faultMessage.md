告警条件态已落地，`matrix_core` 编译通过。待你审查，未提交。

### 核心改动

`FaultMessageManager` 从「只升不降」改为条件态：

- 新增 `clear` / `clearExceptVehicles`
- 条件仍在时 `warn` 会 `touch` 刷新存活时间
- TTL 兜底：`1201/1200=10s`，`801/1008=60s`（ERROR 不超时）

### 成对 raise/clear

| 码            | 行为                                                  |
| ------------ | --------------------------------------------------- |
| **1201**     | 避让中 warn；本拍不在让行集合则 clear                            |
| **1200**     | 无管制车 warn；有车可管制则 clear                              |
| **801/1008** | `orderTake≠AVAILABLE` warn；恢复可接单 clear（周期同步 + 派单路径） |
| **809/810**  | 断连/空位 warn；恢复 clear                                 |

### 顺带修正

队列满时误报 **801** 已去掉（原先把「车满」当成「不接单」）。

主要文件：`fault_message_manager.hpp`、`fault_message.hpp`、`traffic_control_service_regulation.cpp`、`task_assigner_optimize.cpp`、`task_assign_service.cpp`、`application.hpp`。

---

已加上 **1005** 的条件清除，编译通过。

### 行为

- **规划失败** → 继续 warn `1005`
- **规划成功**（该车有可通行路径）→ 立即 clear
- **车进入 `RUNNING`** → 周期同步里 clear（开始运动后消失）
- **TTL 兜底 30s**（派单停了但仍未运动时防漏清）

未提交，可一并审查。

---

已按你的要求补完，编译通过，未提交。

### 行为

| 码        | 条件清除                        | 最长停留    |
| -------- | --------------------------- | ------- |
| **809**  | 已登录 → clear                 | **60s** |
| **810**  | `positionId` 非空 → clear     | **60s** |
| **1200** | 有车正常上报（已登录）或交管有可管制车 → clear | **10s** |

### 实现要点

- TTL 按 **首次上报**（`raiseSeconds`）计时，周期 `warn` 不再续命。
- 超时后进入抑制，避免每秒再弹；**条件恢复并 `clear` 后**才允许再次上报。
- `1200` 在 `displayVehicleInfos` 里：任一机器人 `loggedIn` 即 clear。

主要改动：`fault_message.hpp`、`fault_message_manager.hpp`、`application.hpp`。可直接审阅。
