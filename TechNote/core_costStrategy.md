## 总体架构

`SearchGenericPlanner` 在路网图上做搜索：节点是 **Position**，边是 **RoadLine**。每扩展一条边 `curr → next` 时，构造 `CostEvaluateContext`，交给 `CostEvaluator` 累加各策略的代价，**最终只取 `timeSeconds()` 累加到 `costSoFar`**：

```158:163:core/module/planning/include/planning/planner_search_generic.hpp
                        auto costResult = costEvaluator_.evalute(costEvaluateContext);
                        double evaluteCost = costResult.timeSeconds();
                        if(maths::isNan(evaluteCost)){ continue; }

                        double costSoFar = curr.costSoFar + costResult.timeSeconds();
```

`CostEvaluateResult` 同时携带 `timeSeconds_` 和 `distanceMeters_`，但 A\* 的 g 值、路径 `positionCosts`、超时判断用的都是 **时间（秒）**。

A\* 默认模式是 `TIME_TRAVERSAL_HEURISTICS`：

```16:17:core/module/planning/include/planning/planner_a_star.hpp
            AStarPlanner() : planner_(PriorityMode::TIME_TRAVERSAL_HEURISTICS) { 
                planner_.registerStrategyBack(std::make_shared<planning::TravelCostStrategy>());
```

即 `f = g(累计时间) + h(启发式)`，目标是最小化 **预计通行时间**。

---

## 代价如何组成：策略链叠加

`CostEvaluator` 把所有策略的结果 **按时间、距离分别相加**：

```25:35:core/module/planning/include/planning/cost_evaluator.hpp
            CostEvaluateResult evalute(const CostEvaluateContext& context) const{
                CostEvaluateResult result = CostEvaluateResult::zero();
                for(auto& strategy : strategies_){
                    auto c = strategy->evalute(context);
                    if(!c.validated()){
                        return CostEvaluateResult::invalidate();
                    }
                    result += c;
                }
                return result;
            }
```

典型注册顺序（`planning::plan()`）：

| 顺序    | 策略                     | 贡献                          |
| ----- | ---------------------- | --------------------------- |
| Front | `RotationCostStrategy` | 旋转时间（秒）                     |
| Back  | `TravelCostStrategy`   | 线段行驶时间（秒）+ 距离（米）            |
| 可选    | 禁行区 / 绕路 / 车流 / 禁止点    | 返回 `invalidate()` 剪枝，或加惩罚时间 |

所以：**是的，边代价最终统一成“时间”这个量纲**（外加并行累计距离，供 `distanceSoFar` 用）。

---

## 1. 线段时间：`TravelCostStrategy`

```15:20:core/module/planning/include/planning/cost_strategy_travel.hpp
            virtual CostEvaluateResult evalute(const CostEvaluateContext& context) override {
                auto speed = scenes::routePlanning<double>(scenes::keys::routePlanning_vehicleTravelSpeedFactor, 1.0) * context.vehicleModel()->maxSpeed();
                auto currLine = context.currRoadLine();

                return CostEvaluateResult::make(context.currRoadLine()->dis / speed, context.currRoadLine()->dis);
            }
```

公式：

\[
t_{\text{travel}} = \frac{\text{roadLine.dis}}{\text{vehicleTravelSpeedFactor} \times \text{maxSpeed}}
\]

### 车辆 `maxSpeed` 的影响

**有，且直接影响规划。**

- `maxSpeed` 越大 → 同样 `dis` 时间越短 → 该边在 A\* 中更“便宜”
- 可通过场景配置 `vehicleTravelSpeedFactor` 整体缩放（默认 1.0）

### 没有用到的参数

- `VehicleModel::maxAcceleration` / `maxDeceleration` — **不参与**规划代价
- `RoadLine::limitV` / `limitW` — **不参与**（只用 `dis`，不用路段限速）
- 贝塞尔真实弧长 — 地图加载时已算进 `dis`，规划不再单独处理

这是 **匀速模型**：`时间 = 距离 / 最高线速度`，没有加减速梯形曲线。

---

## 2. 节点旋转时间：`RotationCostStrategy`

在 `planning::plan()` 里注入，并传入车辆当前 yaw：

```54:56:core/module/planning/include/planning/utils.hpp
        std::unordered_map<std::string, double> yawVehicles;
        yawVehicles.emplace(vehicle->id, angles::normalizeRadian(vehicle->pose().yaw));
        planner->registerStrategyFront(std::make_shared<RotationCostStrategy>(yawVehicles));
```

旋转有效角速度：

\[
\omega_{\text{eff}} = \text{vehicleRotationalSpeedFactor} \times \text{maxRotationalSpeed}
\]

（默认 `vehicleRotationalSpeedFactor = 10.0`，相当于把旋转速度放大 10 倍来 **降低** 旋转惩罚。）

最终旋转代价：

\[
t_{\text{rotate}} = \text{vehicleRotateFactor} \times \sum \frac{|\Delta\theta|}{\omega_{\text{eff}}}
\]

分两种情况：

### A. 起点第一次出发（`curr == start`）

```24:32:core/module/planning/include/planning/cost_strategy_rotation.hpp
                if(context.start()->id == context.curr()->id) {
                    if(yawVehicles_.find(context.vehicleId()) != yawVehicles_.end()) {
                        // 车辆当前 yaw vs 第一条边方向角
                        double shorestRad = angles::shortestAngularRadianDistance(yaw, rad);
                        rotateCost += maths::abs(shorestRad) / rotationalSpeed;
                    }
                }
```

车辆当前朝向与 **第一条边方向** 的夹角 → 旋转时间。

只在从起点扩展第一条边时计一次。

### B. 路径中间节点转弯（有 prev、curr、next）

```35:44:core/module/planning/include/planning/cost_strategy_rotation.hpp
                if((nullptr != context.prev()) && (nullptr != context.curr()) && (nullptr != context.next())){
                    // p=prev, q=curr, r=next
                    double angleRad = geometries::angleRadian(p, q, r);  // 顶点内角
                    rotateCost += maths::abs(maths::pi - angleRadAbs) / rotationalSpeed;
                }
```

几何含义：

```
        prev
          \
           q (curr) ——→ next
```

- `angleRadian(p,q,r)` = 顶点 **q 处的内角**
- 直行时内角 ≈ π → `|π - π| = 0`，无旋转代价
- 直角弯内角 = π/2 → 需转 π/2
- 掉头内角 ≈ 0 → 需转 π

这是 **纯几何转弯角 / 角速度**，不考虑：

- 节点是否允许 `spin`（那是 traffic 下发层的事）
- 旋转加减速（`maxRotationalAcceleration` 未使用）

### 车辆 `maxRotationalSpeed` 的影响

**有，且只影响旋转部分。**

- `maxRotationalSpeed` 越大 → 同样转角时间越短 → 规划更倾向于接受大弯/多弯路径（相对直线+慢旋转）
- 再被 `vehicleRotationalSpeedFactor`（默认 10）和 `vehicleRotateFactor`（默认 1）调制

### 没有用到的参数

- `maxRotationalAcceleration` / `maxRotationalDeceleration`
- 点位 `spin` 标志
- 路段 `rotationAllowed`

---

## 3. 一条边的总代价

对边 `prev → curr → next`（扩展 `curr → next`）：

```
边代价 timeSeconds =
    TravelCostStrategy:     dis / (factor × maxSpeed)
  + RotationCostStrategy:   起点对齐角/转弯角 / (factor × maxRotationalSpeed)
  + 其他策略:               惩罚时间 或 NaN(剪枝)
```

`SearchGenericPlanner` 在扩展时传入：

```146:156:core/module/planning/include/planning/planner_search_generic.hpp
                        costEvaluateContext.setPrev(curr.prev)      // 上一节点
                                           .setCurr(curr.position)  // 当前节点
                                           .setNext(neighbour.second) // 下一节点
                                           .setPrevRoadLine(prevRoadLine)
                                           .setCurrRoadLine(roadLine);
```

- **Travel** 只看 `currRoadLine`（当前要走的那条边）
- **Rotation** 看三点拓扑 + 是否在起点

---

## 4. 启发式：并不完全与车辆速度一致

A\* 用的 Manhattan 启发式：

```15:26:core/module/planning/include/planning/heuristic_strategy.hpp
            double heuristic(const CostEvaluateContext& context) {
                // ...
                return minDistance / 1.0;  // 硬编码除以 1.0
            }
```

即 **假设剩余路程以 1 m/s 前进**，与 `maxSpeed` 无关。

影响：

- 若 `maxSpeed` 远大于 1 m/s，h 可能 **高估**，搜索仍正确但扩展节点更多
- 若 `maxSpeed` 远小于 1 m/s，h 可能 **低估**，A\* 仍能找到路但不保证最优性

g 值（真实累计时间）是准确的；f 值的启发式部分是近似。

---

## 5. 生产环境中的额外策略（traffic 层）

`getPlanResult()` 还会追加：

```974:984:core/module/traffic/src/traffic_control_service_regulation.cpp
    strategies.emplace_back(std::make_shared<TrafficFlowCostStrategy>(...));
    strategies.emplace_back(std::make_shared<ProhibittedAreaCostStrategy>(pas));
```

| 策略                            | 行为                               |
| ----------------------------- | -------------------------------- |
| `ProhibittedAreaCostStrategy` | 边在禁行区 → `invalidate()`，直接丢弃      |
| `VehicleDetourCostStrategy`   | 碰撞点 → 丢弃；绕路惩罚点 → +100s（可配置）      |
| `TrafficFlowCostStrategy`     | 对向车流 → +固定惩罚时间（与 `maxSpeed` 弱相关） |
| `ForbiddenCostStrategy`       | 禁止经过某点位 → 丢弃                     |

这些也都通过 `timeSeconds` 进入 g 值，或使边不可达。

---

## 6. 参数影响汇总

| 参数                             | 影响线段时间 | 影响旋转时间 | 备注            |
| ------------------------------ |:------:|:------:| ------------- |
| `maxSpeed`                     | ✅      | ❌      | `dis / speed` |
| `maxRotationalSpeed`           | ❌      | ✅      | `angle / ω`   |
| `maxAcceleration`              | ❌      | ❌      | 未建模           |
| `maxRotationalAcceleration`    | ❌      | ❌      | 未建模           |
| `RoadLine::limitV/limitW`      | ❌      | ❌      | 未使用           |
| `vehicleTravelSpeedFactor`     | ✅ 缩放   | ❌      | 场景配置          |
| `vehicleRotationalSpeedFactor` | ❌      | ✅ 缩放 ω | 默认 10.0       |
| `vehicleRotateFactor`          | ❌      | ✅ 总缩放  | 默认 1.0        |

---

## 7. 直观例子

假设 `maxSpeed=0.8 m/s`，`maxRotationalSpeed=0.8 rad/s`，factor 均为 1，某边 `dis=4m`，转弯角 `π/2`：

```
t_travel  = 4.0 / 0.8 = 5.0 s
t_rotate  = (π/2) / 0.8 ≈ 1.96 s
t_edge    ≈ 6.96 s
```

若 `maxSpeed` 改为 1.6，`t_travel` 减半，但 `t_rotate` 不变 → 规划会更偏好 **绕远但弯少** 的路（因为线段时间权重相对下降）。

默认 `vehicleRotationalSpeedFactor=10` 时，有效 `ω=8 rad/s`，`t_rotate≈0.2s`，旋转代价被大幅压低，规划几乎 **只主要由线段时间驱动**。

---

## 结论

1. **是的，边代价和路径总 cost 统一为时间（秒）**；距离单独累计但不驱动 A\* 优先级（在 TIME 模式下）。
2. **`maxSpeed` 只影响线段匀速行驶时间**；**`maxRotationalSpeed` 只影响节点旋转时间**；两者独立。
3. **加减速、路段限速、旋转加减速均未建模**，是非常简化的 kinematic upper-bound 估计。
4. 旋转代价绑定在 **图节点拓扑**（prev-curr-next 夹角 + 起点初始 yaw），不是绑定在 `RoadLine` 本身。
5. 启发式用固定 `1.0 m/s`，与车辆模型 **不完全一致**，这是实现上的简化，不是严格的 time-optimal A\*。
