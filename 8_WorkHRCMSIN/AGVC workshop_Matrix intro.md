# Matrix 调度系统介绍

| 项 | 内容 |

|----|------|

| 文档类型 | 系统介绍 / 讲座演示稿 |

| 适用对象 | 研发、实施、联调、客户技术交流 |

| 配套文档 | [PRD.md](./PRD.md)、[CoreDesign.md](./CoreDesign.md)、[培训-任务分配](./培训-任务分配与拼车顺风车逻辑.md)、[培训-路径规划](./培训-路径规划逻辑.md)、[培训-交通管制](./培训-交通管制逻辑.md) |

| 代码依据 | `core/`、`engine/`、`webUI/` / `h5UI/` |

> **讲座一句话**：Matrix = **前端运维** + **Engine 业务编排** + **Core 调度内核**，把「下单 → 选车 → 规划 → 交管 → 下发车辆」跑通。

---

## 目录

1. [系统架构](#1-系统架构)

2. [前端功能](#2-前端功能)

3. [模块功能与技术实现](#3-模块功能与技术实现)
- [任务分配](#31-任务分配)

- [机器人管理](#32-机器人管理)

- [路径规划](#33-路径规划)

- [交通管制](#34-交通管制)

- [异常处理](#35-异常处理)
4. [仿真平台](#4-仿真平台)

5. [讲座速记卡片](#5-讲座速记卡片)

---

## 1. 系统架构

### 1.1 三层产品视角

```mermaid
flowchart TB

subgraph Access["接入层 Access"]

UI["webUI / h5UI<br/>Vue3 运维控制台"]

WMS["WMS / 上位系统<br/>HTTP 接口"]

WS["WebSocket<br/>实时状态推送"]

end



subgraph Schedule["调度层 Scheduling"]

ENG["Engine (.NET 6)<br/>任务模板 · Block 编排 · 告警 · 持久化"]

CORE["Core (C++ libmatrix_core.so)<br/>派单 · A* · 交管 · 车辆指令"]

end



subgraph Exec["执行层 Execution"]

AGV["AGV / 仿真车"]

DEV["设备门禁 · PLC · Modbus"]

SCR["车端脚本 / 动作"]

end



UI -->|REST + WS| ENG

WMS -->|Interface API| ENG

ENG -->|SWIG / PInvoke| CORE

CORE -->|协议指令| AGV

CORE -->|设备进出| DEV

ENG -->|非导航 Block| DEV

ENG -->|Script / HTTP Block| SCR

AGV -->|遥测 Meta| CORE

CORE -->|状态| ENG

ENG --> WS --> UI

```

| 层 | 职责（讲座话术） | 主要落点 |

|----|------------------|----------|

| **接入层** | 人怎么下单、怎么看图、外部系统怎么对接 | Controllers、Interface、WebSocket、前端页面 |

| **调度层** | 业务任务怎么拆、哪辆车做、怎么走、怎么避让 | Engine `TaskExecuteService` + Core `TaskAssign` / `Planning` / `Traffic` |

| **执行层** | 真正动起来的东西 | 车辆协议、设备、脚本；仿真时由 `SimulationService` 驱动 |

### 1.2 Engine ↔ Core 协作关系

```mermaid
sequenceDiagram

participant FE as 前端 / WMS

participant ENG as Engine

participant TE as TaskExecuteService

participant CA as CoreAppService

participant CORE as Matrix Core

participant AGV as 车辆



FE->>ENG: 触发任务 / 下运单

ENG->>TE: 创建 Task，跑 Block 树

TE->>CA: CreateOrder → AddOrder / AddBlocks

CA->>CORE: Order + BlockTask 入库

Note over CORE: TaskAssignService ~1s 选车绑单

CORE->>CORE: Traffic 规划 + 让行 + 下发路径

CORE->>AGV: VehicleCommandSender

AGV-->>CORE: 遥测 / 控制应答

CORE-->>ENG: 订单/车辆状态

ENG-->>FE: WS 推送 robotsStatus 等

```

### 1.3 Core 内部模块地图

```mermaid
flowchart LR

subgraph Input

ORD[Order / Block]

CFG[Scene / Map]

TEL[车辆遥测]

end



subgraph CoreMods["Core 模块"]

TA[task_assign<br/>任务分配]

TG[task_generate<br/>闲置/充电]

PL[planning<br/>A* 规划]

TR[traffic<br/>交通管制]

TP[traffic_predict<br/>拥堵预测]

DM[domain<br/>订单/车/图/设备]

FD[fault_diagnosis<br/>故障码]

SIM[simulation<br/>仿真]

end



subgraph Output

CMD[车辆指令]

ALM[故障/告警]

end



ORD --> TA

CFG --> DM

TEL --> DM

TA --> TR

TG --> TA

TR --> PL

TP --> PL

TR --> CMD

DM --> FD --> ALM

SIM -.->|仿真车| DM

```

**启动顺序（`Application`）**：

```text

ChargingManager → LeisureManager → TaskAssignService

→ RegulationTrafficControlService → SimulationService → LogService → DeviceService

```

各服务多为 `LoopService`，默认约 **1s** 一拍（仿真循环约 100ms）。

### 1.4 端到端主链路（讲座总图）

```mermaid
flowchart TD

A[触发：API / 界面 / 呼叫器 / 随机单] --> B[Engine 任务模板]

B --> C[Block 图执行]

C --> D{导航类 Block?}

D -->|CreateOrder| E[Core 创建 Order]

D -->|Modbus/HTTP/Script…| F[Engine 本地执行]

E --> G[TaskAssign 选车]

G --> H[Block 进入交管]

H --> I[A* 规划路径]

I --> J[让行 / 资源占用 / 绕行]

J --> K[分段下发路段]

K --> L[车辆执行]

L --> M{到点有操作?}

M -->|Script/动作| N[下发操作]

M -->|仅移动| O[完成 Block]

N --> O

O --> P[订单完成 / 下一 Block]

```

---

## 2. 前端功能

### 2.1 前端产品矩阵

| 产品 | 技术栈 | 定位 |

|------|--------|------|

| **webUI** | Vue 3 + Vite + Element Plus | 主运维控制台 |

| **h5UI** | Vue 3 + Vite | H5 / 移动侧同类能力 |

| **electronUI** | Electron | **离线回放播放器**（非在线调度） |

通信方式：**REST API + WebSocket**（JWT + RBAC）。

### 2.2 功能地图（讲座可投影）

```mermaid
mindmap

root((Matrix 前端))

调度监控

首页地图

运单/订单

机器人状态

交管可视化

任务配置

任务模板

Block 编排

场景参数

设备与接口

PLC / Modbus

呼叫器

外部接口

仿真与回放

仿真页

回放监控

运维分析

告警/故障

统计报表

日志下载

系统管理

用户角色

权限树

在线脚本

```

### 2.3 与调度强相关的页面

| 页面（`webUI/src/views`） | 对应后端能力 |

|---------------------------|--------------|

| `home` / `dispatch` | 地图监控、实时车态 |

| `robotManagement` | 车辆注册、控制权、暂停恢复 |

| `task` / `taskTemplate` / `taskManagement` | 任务与模板 |

| `orderDetail` | 运单详情 |

| `scenarioConfiguration` | 场景 / 拼单等开关 |

| `simulation` | 仿真车配置与运行 |

| `replayMonitoring` | 调度回放 |

| `faultRecord` / `schedulingFaultStatistics` | 故障与统计 |

| `interfaceConfiguration` / `InterfaceManagement` | 上位对接 |

| `plcDevice*` / `modbusSetting` / `pager` | 设备与呼叫 |

> **讲座提示**：前端不直接做 A* / 交管；它只触发 Engine，再由 Core 计算，状态经 WS 回流到地图。

---

## 3. 模块功能与技术实现

### 3.1 任务分配

#### 功能目标

把 **未绑车订单** 分配给合适车辆：优先空闲近车；开启拼单时可 **在途插单**。

#### 核心概念

| 概念 | 说明 |

|------|------|

| Order | 业务派工单元（MOVE / LOAD / UNLOAD / IDLE / CHARGE…） |

| BlockTask | 交管真正执行的一段 |

| externalId | 外部单号；相同则倾向同车成组 |

| merge | 场景开关：是否允许在途插单 |

```mermaid
stateDiagram-v2

[*] --> CREATED

CREATED --> DISPATCHED: 分配成功绑车

DISPATCHED --> RUNNING: 交管开始执行

RUNNING --> PAUSE: 暂停 / 丢控制权

PAUSE --> RUNNING: 恢复

RUNNING --> FINISHED: 正常完成

RUNNING --> TERMINATED: 终止

PAUSE --> TERMINATED: 终止

```

#### 流程

```mermaid
flowchart TD

A[TaskAssignService 每秒循环] --> B[dispatchOrders]

B --> C[按 externalId 分组]

C --> D{指定 vehicleId?}

D -->|是且车存在| E[直接绑定]

D -->|否| F[OptimizeTaskAssigner]

F --> G[约束过滤：容量/电量/超时]

G --> H[空闲车：算到目标时间代价]

G --> I{merge 开启?}

I -->|是| J[在途车 mergePath 评估插单]

I -->|否| H

H --> K[取最小代价车]

J --> K

E --> L[VehicleTaskManager 入队]

K --> L

L --> M[dispatchBlockTasks]

M --> N[addVehicleTask → 交管]

```

#### 关键代码

| 类 | 路径 |

|----|------|

| `TaskAssignService` | `core/module/task_assign/src/task_assign_service.cpp` |

| `OptimizeTaskAssigner` | `core/module/task_assign/src/task_assigner_optimize.cpp` |

| `VehicleTaskManager` | `core/module/task_assign/include/.../vehicle_task_manager.hpp` |

| 约束 | `VehicleCapacityConstraint` / `VehicleEnergyConstraint` / `OrderTimeoutConstraint` |

| Engine 侧 | `TaskExecuteService` → `CreateOrderHandler` → `CoreAppService.AddOrder` |

```text

选型要点（可板书）：

• 算法 = 贪心最近路径 + HardConstraint，不是匈牙利 / ILP

• 在途插单 = mergePath 逐步「原下一站 vs 新站」二选一

• Engine 负责业务 Block 树；Core 负责「选哪辆车」

```

---

### 3.2 机器人管理

#### 功能目标

车辆 **注册 / 注销**、**状态维护**、**电量监控**、控制权与指令通道。

#### 流程：注册与生命周期

```mermaid
sequenceDiagram

participant UI as 前端/场景配置

participant ENG as Engine CoreAppService

participant VM as VehicleManager

participant V as Vehicle

participant S as Session 协议会话

participant SIM as SimulationService



UI->>ENG: AddVehicle

ENG->>VM: addVehicle

VM->>V: 创建实体 + RealtimeData

alt 真车

V->>S: 登录 / 心跳 / 遥测

else 仿真车

V->>SIM: 挂接 Simulator

end

S-->>V: 位姿、电量、故障、控制权

V-->>ENG: 状态汇总

ENG-->>UI: WS robotsStatus

```

#### 状态维护要点

```mermaid
flowchart LR

META[车端 Meta 遥测] --> RT[VehicleRealtimeData]

RT --> POS[位姿 / positionId]

RT --> BAT[电量 / charging]

RT --> CR[controlRight MRCS/…]

RT --> ST[FREE / RUNNING / …]

RT --> ERR[error / 急停]

POS --> ASSIGN[派单可用性]

BAT --> CHARGE[ChargingManager]

CR --> DISP[是否可下发任务]

ERR --> FAULT[FaultMessageManager]

```

| 能力 | 实现要点 | 代码落点 |

|------|----------|----------|

| 注册/注销 | `AddVehicle` / `RemoveVehicle` | `vehicle_manager.hpp` |

| 状态维护 | 遥测解析 → `VehicleRealtimeData` | `vehicle_realtime_data.hpp`、`vehicle.cpp` |

| 电量监控 | 阈值触发充电任务；Engine 电池配置 API | Core `ChargingManager`；Engine `RobotBatteryAppService` |

| 指令下发 | 移动 / 动作 / 脚本 / 取消 / 控权 | `vehicle_command_sender.hpp` |

| 控制权 | MRCS 持权才可稳定调度；丢权暂停并取消移动 | `task_assign` + `traffic` 协同 |

```text

讲座口诀：

有车 → 有位姿 → 有控权 → 有电量余量 → 才能稳定接单执行。

缺一项，就会表现为「派不出去 / 跑不动 / 到点不下操作」。

```

---

### 3.3 路径规划

#### 功能目标

在路网图上搜索从当前位置到目标站点的可行路径，并输出可下发的路段序列。

#### 流程

```mermaid
flowchart TD

A[交管 / 派单需要路径] --> B[planning::plan / multiPlan]

B --> C[AStarPlanner 在路网点边搜索]

C --> D[启发式：曼哈顿]

C --> E[代价：行驶 + 旋转 + 可选交通流/绕行/禁行区]

E --> F[得到站点/路段序列]

F --> G[地图边 Bezier 插值 midPoses<br/>路网几何平滑]

G --> H[交给交管分段下发]

```

#### 「动态避障」在 Matrix 中的真实含义

```mermaid
flowchart LR

subgraph Scheduler["调度侧（Core）"]

Y[让行规则]

D[绕行重规划 Detour]

C[拥堵代价 traffic_predict]

end

subgraph Vehicle["车端"]

L[局部雷达/避障]

W[现场等待]

end

Y --> Stop[停在安全点等待]

D --> Replan[换一条全局路径]

C --> Prefer[规划时倾向更畅通路]

L -.->|不在 Core 内| W

```

| 说法 | 代码实际 |

|------|----------|

| A* | `AStarPlanner`（`planner_a_star.hpp`） |

| 路径平滑 | 主要是路网解析时的 **Bezier 均匀插值**，不是规划后轨迹优化 |

| 动态避障 | **让行 + 绕行重规划 + 交通流代价**；局部避障在车端 |

| 类 | 路径 |

|----|------|

| `AStarPlanner` | `core/module/planning/include/planning/planner_a_star.hpp` |

| `MultiRoutePlanner` | 多车推闲置车让道 |

| 代价策略 | `cost_strategy_travel` / `rotation` / `TrafficFlow` / `VehicleDetour` / `ProhibittedArea` |

| 插值 | `uniform_interpolation.hpp` |

---

### 3.4 交通管制

#### 功能目标

多车共享路网时，保证 **安全通行**：路口/对向/同向冲突协调，必要时绕行解除死锁。

#### 控制模型

```mermaid
flowchart TB

CB[ControlBlock<br/>一车一执行中 Block]

CB --> SI[sendIndex 已下发进度]

CB --> LI[limitIndex 允许前进上限]

CB --> STEP[stepId]

SEC[RouteSectionPartition]

SEC --> SAME[同向 SAME]

SEC --> OPP[对向 OPPOSITE]

SEC --> INT[路口 INTERSECTION]

```

#### 主循环（约 1s）

```mermaid
flowchart TD

A[updateBlockTasks] --> B[更新车辆位置]

B --> C[构建/更新路段分区]

C --> D[让行规则评估]

D --> E{可前进?}

E -->|否| F[等待 / 记录依赖]

E -->|是| G[applyRoadLines 下发路段]

G --> H[设备门禁 enter/leave]

H --> I[交通流评估]

I --> J{长时间受阻?}

J -->|是| K[detour 绕行重规划]

J -->|否| L[下一拍]

K --> L

F --> M{依赖成环?}

M -->|是| N[死锁：停驶 + 触发绕行策略]

M -->|否| L

```

#### 让行与「资源锁」

```mermaid
flowchart LR

subgraph SoftLock["软资源占用（非命名互斥锁）"]

P[点位占用 PositionOccupied]

S[路段占用 RouteSection]

A[区域占用 Area]

R[依赖关系 DependencyRelation]

end

SoftLock --> DEC[本拍谁能走 / 谁必须让]

DEC --> WIN[先到先得 / 规则优先级]

```

| 能力 | 实现 | 代码 |

|------|------|------|

| 路口/对向分配 | 路段分区 + 让行规则 | `route_section_partition.hpp`、`regulations/*` |

| 死锁检测 | 依赖环检测；久堵触发绕行 | `DependencyRelationYieldRegulation`、`VehicleDetourService` |

| 资源锁管理 | `sendIndex`/`limitIndex` + 分区占用 | `ControlBlock` / `ControlBlockManager` |

| 碰撞几何 | GEOS + 车体尺寸 | `VehicleCollisionChecker` |

```text

讲座口诀：

交管不是「一次性发完全程」，而是「按拍推进走廊占用」。

死锁常见形态 = 对向互相礼让成环；解法 = 绕行换路，而不是无限等待。

```

---

### 3.5 异常处理

#### 功能目标

对 **任务超时、车辆故障、通信中断** 等异常可发现、可告警、可恢复或可终止。

#### 总览

```mermaid
flowchart TB

subgraph Detect["发现"]

T1[订单超时约束]

T2[车辆断连 / 位姿为空]

T3[车端 error / 急停]

T4[交管久堵 / 派单失败]

end



subgraph Handle["处理"]

H1[FaultMessageManager 抬升/清除故障码]

H2[暂停订单 / cancelMovement]

H3[终止订单 terminateOrder]

H4[Engine 告警推送前端]

end



Detect --> Handle

Handle --> UI[故障记录 / 调度故障统计]

```

#### 分类与落点

| 异常类型 | 典型表现 | 处理策略 | 关键代码 / 错误码 |

|----------|----------|----------|-------------------|

| **任务超时** | 长时间无法分配或无法完成 | `OrderTimeoutConstraint` 过滤；业务侧终止 | `constraint_order_timeout.hpp`；`ASSIGN_*` |

| **车辆故障** | error / 急停 / 软急停 | 停派、暂停任务、告警 | `VehicleRealtimeData` + `FaultMessageManager` |

| **通信中断** | 断连、丢遥测、位姿空 | 故障码 809/810；不可再下发依赖控权的任务 | `VEHICLE_DISCONNECTED`、`VEHICLE_POSITION_EMPTY` |

| **丢控制权** | MRCS → 非 MRCS | 暂停 + `cancelMovement`（边沿触发） | `task_assign_service` / `traffic` |

| **派单失败** | 无车 / 无路 / 电量不足 | 记录失败原因，订单保持待派 | `ASSIGN_NO_VEHICLE` 等 1000–1008 |

| **交管受阻** | 让行等待过久 | 绕行；必要时人工干预 | `TRAFFIC_CONTROL_*` 1200+ |

```mermaid
sequenceDiagram

participant V as 车辆

participant CORE as Core

participant FM as FaultMessageManager

participant ENG as Engine

participant FE as 前端



V--xCORE: 通信中断

CORE->>FM: raise VEHICLE_DISCONNECTED

CORE->>CORE: 停止向该车派新执行

FM-->>ENG: 故障状态

ENG-->>FE: 告警 / 故障记录

V-->>CORE: 恢复连接

CORE->>FM: clear 故障码

Note over CORE: 位姿与控权恢复后可再接单

```

---

## 4. 仿真平台

### 4.1 定位

在 **无真车 / 少真车** 时，用 Core 内仿真器驱动标记为仿真的车辆，走完整派单 → 规划 → 交管链路，便于联调与演示。

```mermaid
flowchart LR

UI[前端 simulation 页] --> ENG[SetSimulationVehicles]

ENG --> CORE[SimulationService]

CORE --> SIM[Simulator ~100ms]

SIM --> V[仿真 Vehicle 位姿/电量]

V --> TA[任务分配]

TA --> TR[交管]

TR --> SIM

SIM --> UI2[地图上看到车在跑]

```

### 4.2 组成

| 组件 | 说明 | 路径 |

|------|------|------|

| `SimulationService` | 仿真总服务 | `core/module/simulation/` |

| `Simulator` | 单车运动/电量推进 | 同模块 |

| `SimulationTask` / `SimulationBattery` | 任务与电量模拟 | 同模块 |

| 前端 | `webUI` / `h5UI` → `views/simulation` | — |

| 随机单 | Engine `RandomOrderService` | 压测/演示流量 |

| 回放（相关） | `Sineva.Matrix.Engine.Replay` + electronUI | **离线回放**，不是物理仿真 |

### 4.3 与真车对比（讲座对照表）

| 维度 | 真车 | 仿真车 |

|------|------|--------|

| 通信 | 协议会话登录/遥测 | 内存推进状态 |

| 指令 | `VehicleCommandSender` 真实下发 | 仿真器消化任务状态 |

| 交管/规划 | **同一套 Core 逻辑** | **同一套** |

| 用途 | 现场生产 | 联调、培训、方案演示 |

```text

讲座强调：

仿真验证的是「调度逻辑对不对」，不是替代车端局部避障与现场标定。

回放验证的是「当时发生了什么」，用于事后复盘。

```

---

## 5. 讲座速记卡片

### 5.1 三层一句话

| 层 | 一句话 |

|----|--------|

| 接入层 | 谁下单、谁看图、谁对接外部 |

| 调度层 | Engine 编排业务，Core 选车/规划/交管 |

| 执行层 | 车、设备、脚本真正执行 |

### 5.2 五大模块一句话

| 模块 | 一句话 |

|------|--------|

| 任务分配 | 1 秒贪心选车，可在途插单 |

| 机器人管理 | 注册 + 遥测状态机 + 电量与控权 |

| 路径规划 | 路网 A*；平滑在路网几何；避障靠让行/绕行 |

| 交通管制 | 分段占用走廊；死锁靠依赖检测 + 绕行 |

| 异常处理 | 故障码抬升/清除 + 暂停终止 + 前端告警 |

### 5.3 推荐演示路径（15–20 分钟）

```mermaid
flowchart LR

A[1.架构总图] --> B[2.前端地图点开]

B --> C[3.下一单看派车]

C --> D[4.看路径与交管让行]

D --> E[5.仿真多车冲突]

E --> F[6.拔网线/丢控权看异常]

```

### 5.4 深入阅读

| 想深挖 | 文档 |

|--------|------|

| 详细设计与选型 | [CoreDesign.md](./CoreDesign.md) |

| 派单 / 拼单 | [培训-任务分配与拼车顺风车逻辑.md](./培训-任务分配与拼车顺风车逻辑.md) |

| A* 与代价 | [培训-路径规划逻辑.md](./培训-路径规划逻辑.md) |

| 让行与死锁 | [培训-交通管制逻辑.md](./培训-交通管制逻辑.md) |

| Engine 工程结构 | `engine/README.md` |

---

*文档生成说明：内容对齐当前仓库实装（Core C++ / Engine .NET / Vue 前端），流程图面向讲座投影；细节实现以配套培训文档与代码为准。*
