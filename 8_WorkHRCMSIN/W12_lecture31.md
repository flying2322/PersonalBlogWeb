下面按**讲座叙事**整理：先总图，再五条算法主线。每张图都能单独讲 3–5 分钟。

---

## 讲座开场：参数落在哪一层

一句话：**参数不改车端物理速度，改的是 Core「选谁、选哪条路、何时让行/绕障」的决策。**

```mermaid
flowchart TB
    subgraph P1["① 派单决策 order"]
        A1[freeVehicleFirst]
        A2[highPriorityFirst]
        A3[merge]
        A4[maxCacheTask]
    end

    subgraph P2["② 执行前干预 traffic"]
        B1[canPushVehicles]
        B2[pathAvoid / pathAvoidDelay]
        B3[rotateYawLimit]
    end

    subgraph P3["③ 路径代价 routePlanning + trafficFlow"]
        C1[Travel / Rotate 三系数]
        C2[detourFactor]
        C3[oppositeTravelFactor]
        C4[拥堵聚类两参数]
    end

    subgraph P4["④ 安全包络 collision"]
        D1[inflate 前后左右]
    end

    ORD[未派订单] --> P1
    P1 -->|绑车 + 取 Block| DISP[下发执行]
    DISP --> P2
    P2 --> PLAN[路径规划 / 重规划]
    PLAN --> P3
    PLAN --> TC[交管让行]
    TC --> P4
    TC --> MOVE[下发移动指令]
    MOVE --> B3
```

**讲法提示**：先让听众记住四层——**派谁 → 要不要推/绕 → 路怎么计价 → 包络多大**。

---

## 主线一：派单算法（最常问）

**对应参数**：`freeVehicleFirst` · `highPriorityFirst` · `merge` · `maxCacheTask`

### 1.1 总流程（讲座主图）

```mermaid
flowchart TD
    START([周期扫描未派订单]) --> FILTER[过滤可接单车辆]
    FILTER --> MCT{队列长度 ≥ maxCacheTask?}
    MCT -->|是| DROP[该车淘汰]
    MCT -->|否| SPLIT[拆成空闲车 / 忙碌车]

    SPLIT --> IDLE[空闲车：算到目标的最短估时]
    IDLE --> HAS_IDLE{找到空闲车?}

    HAS_IDLE -->|是| FVF{freeVehicleFirst<br/>或 priority > highPriorityFirst?}
    FVF -->|是| BIND_IDLE[直接绑空闲车 ✅]
    FVF -->|否| MERGE_CHK

    HAS_IDLE -->|否| MERGE_CHK{merge 开启?<br/>且非纯闲置单可例外}
    MERGE_CHK -->|否| WAIT[本轮不派 / 等空闲]
    MERGE_CHK -->|是| WORK[忙碌车：mergePath 插单估时]

    WORK --> CMP{空闲估时 vs 忙碌估时}
    CMP -->|空闲更优或仅有空闲| BIND_IDLE
    CMP -->|忙碌更优或仅有忙碌| BIND_WORK[绑忙碌车 ✅]
    CMP -->|都没有| FAIL[本轮失败]
```

### 1.2 可接单过滤（讲 `maxCacheTask` 时展开）

```mermaid
flowchart LR
    V[可控车辆] --> S1{接单状态 AVAILABLE?}
    S1 -->|否| X1[跳过]
    S1 -->|是| S2{指定车/车组匹配?}
    S2 -->|否| X2[跳过]
    S2 -->|是| S3{地图含订单点?}
    S3 -->|否| X3[跳过]
    S3 -->|是| S4{电量/充电策略允许?}
    S4 -->|否| X4[跳过]
    S4 -->|是| S5{缓存单数 < maxCacheTask?}
    S5 -->|否| X5[跳过]
    S5 -->|是| OK[进入空闲或忙碌池]
```

### 1.3 拼单 `mergePath`（讲 `merge=true` 时展开）

```mermaid
flowchart TD
    M0[已有订单点序列 path1<br/>新订单点序列 path2] --> M1[从当前位置出发]
    M1 --> M2[本步候选 = path1下一点 ∪ path2下一点]
    M2 --> M3[A* 选最近可达目标]
    M3 --> M4{容量 / 超时 / 电量约束}
    M4 -->|不满足| MFAIL[该忙碌车不可插单]
    M4 -->|满足| M5[累加估时；指针前进]
    M5 --> M6{两序列都走完?}
    M6 -->|否| M2
    M6 -->|是| MOK[得到插单总代价]
```

**口播金句**：  
`freeVehicleFirst` 是「有空车先用空车」；`merge` 是「没空车或关掉空车优先时，允许在途插单」；`maxCacheTask` 是「单车口袋容量」。

---

## 主线二：下发前推车

**对应参数**：`canPushVehicles`

```mermaid
flowchart TD
    B0[准备下发队首 Block] --> B1{canPushVehicles?}
    B1 -->|否| BGO[直接规划并下发]
    B1 -->|是| B2[收集：无任务空闲车 + 可用闲置点]
    B2 --> B3[multiPlan：本车去目标 + 挡路车去避让点]
    B3 --> B4{需要挪车?}
    B4 -->|否| BGO
    B4 -->|是| B5[给挡路车注册闲置/让行任务]
    B5 --> BGO
```

**口播**：推车只推「口袋空」的闲置车；干活的车不推，靠交管让行/绕障处理。

---

## 主线三：路径规划代价（选路的尺子）

**对应参数**：`vehicleTravelSpeedFactor` · `vehicleRotationalSpeedFactor` · `vehicleRotateFactor` · `detourFactor` · `oppositeTravelFactor`

```mermaid
flowchart TD
    E0[规划一条边 curr → next] --> E1[旅行代价]
    E1 --> E1a["time = dis / (TravelFactor × vmax) × weightRatio"]

    E0 --> E2[旋转代价]
    E2 --> E2a["Δθ / (RotSpeedFactor × ωmax)"]
    E2a --> E2b["再 × RotateFactor"]

    E0 --> E3[交通流代价]
    E3 --> E3a["+ oppositeTravelFactor<br/>（当前实现对边几乎恒加）"]

    E0 --> E4{本车有绕障信息?}
    E4 -->|禁行点| EINVAL[边无效]
    E4 -->|软绕行点| E4a["+ detourFactor"]
    E4 -->|无| E4b[0]

    E1a --> SUM[边总代价]
    E2b --> SUM
    E3a --> SUM
    E4a --> SUM
    E4b --> SUM
    SUM --> ASTAR[A* 累加选最短路径]
```

**口播对照表**（适合贴幻灯片）：

| 调大参数           | 规划更倾向           |
| -------------- | --------------- |
| TravelFactor   | 觉得「开得慢」→ 更惜路程   |
| RotateFactor   | 更讨厌转弯 → 愿绕直路    |
| RotSpeedFactor | 觉得「转得快」→ 转弯惩罚变小 |
| detourFactor   | 更不愿贴绕障点（软惩罚）    |

---

## 主线四：绕障 / 故障避让

**对应参数**：`pathAvoid` · `pathAvoidDelay` · `detourFactor`  
（拥堵侧：`congestionClusteringDistanceMeter` · `congestionLevelCount`；`slowPenaltyFactor` 当前未接线）

### 4.1 故障/挡路触发（核心）

```mermaid
flowchart TD
    T0([交管周期：绕障触发器]) --> T1{pathAvoid == true?}
    T1 -->|否| TEND[不触发绕障]
    T1 -->|是| T2{本车被障碍阻塞?}

    T2 -->|是且未在绕| T3[立刻标记前方点为绕障点]
    T2 -->|否| T4{故障/急停/error<br/>且持续 > pathAvoidDelay?}

    T4 -->|否| TWAIT[继续等延时]
    T4 -->|是| T5[登记故障占位点]
    T5 --> T6[找「被挡且绕行收益最大」的车]
    T6 --> T7[写入 DetourInfo]

    T3 --> T7
    T7 --> T8[重规划时挂 VehicleDetourCostStrategy]
    T8 --> T9[禁行 or +detourFactor]
```

### 4.2 和拥堵参数的关系（次要支线）

```mermaid
flowchart LR
    C0[在线车辆位姿] --> C1[DBSCAN 聚类<br/>距离 = congestionClusteringDistanceMeter]
    C1 --> C2[按均速分级<br/>档数 = congestionLevelCount]
    C2 --> C3[生成路段拥堵流]
    C3 --> C4[慢车/拥堵绕行触发器]
    C4 -.->|当前| C5[slowPenaltyFactor<br/>未参与代价计算]
```

**口播金句**：  
`pathAvoid` 管「开不开绕」；`pathAvoidDelay` 管「故障要僵多久才绕」；`detourFactor` 管「绕的时候有多讨厌原路线」。

---

## 主线五：碰撞膨胀 → 让行 → 旋转下发

**对应参数**：`inflate*` · `rotateYawLimit`

```mermaid
flowchart TD
    R0[两车已规划路径] --> R1["包络 = 车体/货物尺寸 + inflate 前后左右"]
    R1 --> R2[路径冲突分段 RouteSection]
    R2 --> R3[交管规则：让行 / 限速段 / 占点避让]
    R3 --> R4[滚动下发局部路段]
    R4 --> R5{"航向差 > rotateYawLimit?"}
    R5 -->|是| R6[插入原地旋转 + 行驶]
    R5 -->|否| R7[只下发行驶段]
    R6 --> R8[车端执行]
    R7 --> R8
```

**口播**：inflate 越大越「胆小」；`rotateYawLimit` 越大越「懒得转」（小角度偏差不点转）。

---

## 讲座收束：一张「决策时序」总图

适合最后 2 分钟串场：

```mermaid
sequenceDiagram
    participant O as 订单
    participant A as 派单<br/>(order 四参数)
    participant D as 下发<br/>(canPushVehicles)
    participant P as 规划<br/>(代价系数 / detour)
    participant T as 交管<br/>(inflate / rotate)
    participant V as 车辆

    O->>A: 未派订单
    Note over A: maxCacheTask 过滤<br/>freeVehicleFirst / highPriorityFirst<br/>merge 插单比代价
    A->>D: 绑车，取 Block
    D->>D: 可选推开闲置车
    D->>P: 初次规划 / 绕障重规划
    Note over P: Travel/Rotate 估时<br/>pathAvoid 触发后 +detourFactor
    P->>T: 全路径 + 冲突段
    T->>V: 滚动下发（含旋转阈值）
    V-->>T: 遥测 / 故障
    Note over T: pathAvoidDelay 后可触发绕障
    T->>P: 重规划
```

---

## 建议讲序（约 25–30 分钟）

| 时长  | 章节         | 只讲这些参数                                                      |
| --- | ---------- | ----------------------------------------------------------- |
| 3′  | 开场四层图      | 全部「归位」                                                      |
| 8′  | 主线一派单      | freeVehicleFirst / highPriorityFirst / merge / maxCacheTask |
| 3′  | 主线二推车      | canPushVehicles                                             |
| 5′  | 主线三代价      | Travel/Rotate 三因子 + detourFactor                            |
| 5′  | 主线四绕障      | pathAvoid / Delay / detour；提一句拥堵两参数                         |
| 4′  | 主线五安全      | inflate* / rotateYawLimit                                   |
| 2′  | 时序收束 + Q&A | 强调：`slowPenaltyFactor`、对向因子当前弱效                             |

如需，我可以把以上内容整理成 `core/docs` 下一份独立培训 Markdown（可直接投影），或导出为分页幻灯结构。







下面按**调用顺序**梳理：只挑「订单分配 + 路径规划」链路上、**实现超过约 50 行**、且讲座里最该讲的函数。

---

## 1. 核心函数清单（>50 行）

### 订单分配侧

| 函数                                         | 约行数  | 一句话                               |
| ------------------------------------------ | ---- | --------------------------------- |
| `OptimizeTaskAssigner::canWorkVehicles`    | ~106 | 过滤可接单车，拆成空闲/忙碌池（含 `maxCacheTask`） |
| `OptimizeTaskAssigner::assignGroupOrders`  | ~63  | 空闲优先 / 拼单开关 / 比代价绑车               |
| `OptimizeTaskAssigner::assignIdleVehicles` | ~117 | 对空闲车调规划，选最短估时                     |
| `OptimizeTaskAssigner::mergePath`          | ~91  | 忙碌车在途插单：两路点序列交错取近                 |
| `OptimizeTaskAssigner::findBestRoute`      | ~51  | 派单侧规划入口（同层 / 跨层电梯）                |
| `TaskAssignService::dispatchBlockTasks`    | ~222 | 已绑车订单取队首 Block，推车后交给交管            |
| `TaskAssignService::processPushVehicles`   | ~53  | `canPushVehicles`：多车规划推开闲置车       |
| `TaskAssignService::dispatchPushYieldTask` | ~69  | 给被推车建闲置让行任务并规划                    |

### 路径规划侧

| 函数                                               | 约行数  | 一句话                              |
| ------------------------------------------------ | ---- | -------------------------------- |
| `planning::plan`（`utils.hpp`）                    | ~85  | 组装 A*、旋转/旅行代价策略，校验起终点            |
| `planning::multiPlan`（`utils.hpp`）               | ~106 | 推车用的多车联合规划入口                     |
| `MultiRoutePlanner::plan`                        | ~76  | 主车路径 → 找冲突闲置车 → 再规划避让            |
| `GenericSearchPlanner::doPlan`                   | ~100 | **真正的 A\* 搜索核**（open/closed、边代价） |
| `RegulationTrafficControlService::runBlockTask`  | ~59  | 交管接 Block：规划全路径写入 Control        |
| `RegulationTrafficControlService::getPlanResult` | ~80  | 交管规划：挂绕障/交通流/禁行区策略               |
| `RegulationTrafficControlService::detour`        | ~142 | 绕障触发后重规划                         |
| `RegulationTrafficControlService::updateRoutes`  | ~192 | 滚动下发路段、让行、旋转阈值                   |

> 说明：`updatePositions` 也很大，偏「位置推进/完成判定」，不是「选路算法核」，讲座可一笔带过。

---

## 2. 调用顺序 UML（序列图）——讲座主图

从「每秒派单拍」到「第一次真正 A\*」再到「交管落地规划」：

```mermaid
sequenceDiagram
    autonumber
    participant TAS as TaskAssignService
    participant OTA as OptimizeTaskAssigner
    participant Plan as planning::plan / multiPlan
    participant MRP as MultiRoutePlanner
    participant AStar as GenericSearchPlanner::doPlan
    participant TC as RegulationTrafficControlService

    Note over TAS: doWork() ~1s（入口较短）

    TAS->>OTA: dispatchOrders → assign → assignGroupOrders
    OTA->>OTA: canWorkVehicles（>50）
    OTA->>OTA: assignIdleVehicles（>50）
    OTA->>OTA: findBestRoute（>50）
    OTA->>Plan: plan(路网, 车, 起, 目标)
    Plan->>AStar: AStarPlanner → doPlan（>50）
    AStar-->>Plan: PlanResult
    Plan-->>OTA: 估时/距离

    opt merge 打开且要比忙碌车
        OTA->>OTA: assignWorkVehicles → mergePath（>50）
        OTA->>OTA: findBestRoute
        OTA->>Plan: plan(...)
        Plan->>AStar: doPlan
    end

    OTA->>OTA: assignOrderToVehicle（绑车）

    TAS->>TAS: dispatchBlockTasks（>50）
    opt canPushVehicles
        TAS->>TAS: processPushVehicles（>50）
        TAS->>Plan: multiPlan（>50）
        Plan->>MRP: MultiRoutePlanner::plan（>50）
        MRP->>AStar: doPlan（主车）
        MRP->>AStar: doPlan（挡路闲置车）
        TAS->>TAS: dispatchPushYieldTask（>50）
    end

    TAS->>TC: dispatchTask → addVehicleTask → runBlockTask（>50）
    TC->>TC: getPlanResult（>50）
    TC->>Plan: plan(..., 交通流/禁行区策略)
    Plan->>AStar: doPlan
    AStar-->>TC: 全路径写入 Control

    Note over TC: 后续周期 updateRoutes / detour<br/>滚动执行与绕障重规划
```

---

## 3. 结构关系 UML（类图）——讲「谁调谁」

```mermaid
classDiagram
    class TaskAssignService {
        +doWork()
        +dispatchOrders()
        +dispatchBlockTasks()  ≈222
        +processPushVehicles() ≈53
        +dispatchPushYieldTask() ≈69
        +dispatchTask()
    }

    class OptimizeTaskAssigner {
        +assignGroupOrders() ≈63
        +canWorkVehicles() ≈106
        +assignIdleVehicles() ≈117
        +assignWorkVehicles()
        +mergePath() ≈91
        +findBestRoute() ≈51
        +assignOrderToVehicle()
    }

    class planning_utils {
        +plan() ≈85
        +multiPlan() ≈106
    }

    class MultiRoutePlanner {
        +plan() ≈76
    }

    class AStarPlanner {
        +plan()
    }

    class GenericSearchPlanner {
        +doPlan() ≈100
    }

    class RegulationTrafficControlService {
        +runBlockTask() ≈59
        +getPlanResult() ≈80
        +detour() ≈142
        +updateRoutes() ≈192
    }

    TaskAssignService --> OptimizeTaskAssigner : 选车
    OptimizeTaskAssigner --> planning_utils : findBestRoute
    TaskAssignService --> planning_utils : 推车 multiPlan
    planning_utils --> MultiRoutePlanner : 多车
    planning_utils --> AStarPlanner : 单车
    MultiRoutePlanner --> AStarPlanner
    AStarPlanner --> GenericSearchPlanner : doPlan
    TaskAssignService --> RegulationTrafficControlService : 下发 Block
    RegulationTrafficControlService --> planning_utils : getPlanResult
```

---

## 4. 按调用顺序「怎么讲」（口播提纲）

1. **`assignGroupOrders`**：决策中枢——空闲优先 / merge / 比代价。  
2. **`canWorkVehicles`**：谁有资格进池（电量、车组、地图、`maxCacheTask`）。  
3. **`assignIdleVehicles` / `mergePath`**：分别算空闲直达与忙碌插单；都靠 **`findBestRoute`**。  
4. **`planning::plan` → `doPlan`**：派单阶段的「估时尺子」（A\* + 旅行/旋转代价）。  
5. **`dispatchBlockTasks` → `processPushVehicles` → `multiPlan`**：执行前能不能把挡路闲置车推开。  
6. **`runBlockTask` → `getPlanResult` → `plan` → `doPlan`**：交管第一次生成**可执行全路径**（比派单多挂交通流/禁行区等策略）。  
7. **`updateRoutes` / `detour`**：跑起来以后的滚动下发与绕障重规划（仍会再进 `getPlanResult`/`doPlan`）。

---

## 5. 两张「算法核」对比（适合收尾）

|     | 派单规划              | 交管规划              |
| --- | ----------------- | ----------------- |
| 入口  | `findBestRoute`   | `getPlanResult`   |
| 目的  | 选车、比 ETA          | 生成/重规划真路径         |
| 策略  | 旅行 + 旋转为主         | + 交通流、禁行区、可选绕障    |
| 搜索核 | 同一个 `doPlan`（A\*） | 同一个 `doPlan`（A\*） |

**金句**：订单分配和路径规划共用一个 A\* 核（`doPlan`）；差别在**谁调用、挂什么代价策略、结果用来选车还是开车**。

若需要，我可以再拆一张「仅 `mergePath` ↔ `doPlan`」的放大序列图，专门讲拼单时怎么反复调规划。












