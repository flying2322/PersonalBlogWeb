基于 `logs/20260807/core-20260807.1.log` 与当前 Core 代码，结论如下。

---

## 一、结论（先给答案）

**不是“Core 没检测到抢控”，而是“检测到之后发出的停车指令对车端无效”。**

| 维度         | 结论                                                                                       |
| ---------- | ---------------------------------------------------------------------------------------- |
| **控制权检测**  | 正常。S800_4# 共 5 次 MRCS→RTT 抢控，Core 均触发 `[control-lost]`                                   |
| **调度逻辑**   | 已执行 pauseOrder + 3 次 cancelMovement，但车仍 RUNNING                                          |
| **车端状态上报** | 正常。抢控后正确上报 `controlRight:RTT`、`state:RUNNING`、`localPath` 持续缩减                           |
| **根因**     | **抢控完成后 Matrix 已无 MRCS 控制权，`cancelMovement` 虽 TCP 发送成功，车端不执行/不回 ACK，惯性继续跑 Matrix 已下发路径** |

两版修改（`controlRightChanged` 边沿 + `dispatchBlockTasks` 停车）只解决了“何时发 cancel”，**没解决“丢控后发 cancel 无效”**这一核心问题。

---

## 二、日志证据链（13:54:08 典型事件）

```
13:54:07.612  controlRight:MRCS, state:RUNNING, localPath:30, P_6
13:54:08.278  pauseOrder → cancelMovement #1 (traffic) cancelOk:true
13:54:08.293  cancelMovement #2 (abort) cancelOk:true
13:54:08.296  [control-lost] takenBy:RTT, Core state:PAUSE, localPath:30
13:54:08.297  cancelMovement #3 sent OK — "stopped on control lost"
13:54:08.613  车端上报: controlRight:RTT, state:RUNNING, localPath:30  ← 未停
13:54:09~16   物理位移 P_6→P_5→P_4→P_3，localPath 27→2
```

关键事实：

1. **三次 cancel 在 20ms 内发出**，均 `cancelOk:true`（仅表示 TCP 写入成功）。
2. **该时刻没有任何 `cancel movement task result:true/false`** — 车端未 ACK cancel。
3. Core 日志写 `state:PAUSE` 来自 `applyTaskPauseVehicleState()` 本地写入；**约 300ms 后车端状态覆盖为 RUNNING**（`vehicle_realtime_data` 每帧用上报覆盖 `state_`）。
4. CARE 在 proto 里映射为 **RTT(1)**，不是 ROBOSHOP(3)，但边沿检测 `!= MRCS` 仍有效。

---

## 三、代码路径梳理

```535:548:core/module/task_assign/src/task_assign_service.cpp
            if (vehicle->controlRight().type != ControlRightType::MRCS) {
                if (traffic::mrcs_control_lost::shouldStopOnMrcsControlLost(vehicle->controlRight().type,
                                                                            vehicle->controlRightChanged())) {
                    if (!vehicle->orderId().empty()) {
                        pauseOrder(vehicle->orderId());
                    }
                    traffic::mrcs_control_lost::stopVehicleOnMrcsControlLost(vehicle, title_, vehicle->controlRight().type);
                    vehicle->clearControlRightChanged();
                }
                // ...
                continue;
            }
```

```326:329:core/module/domain/src/vehicle_realtime_data.cpp
            if (wasMRCS != isMRCS) {
                controlRightChanged_.store(true);
            }
```

时序问题：

```
车端 CARE 抢控 → 状态帧已是 RTT → Core 解析并置 controlRightChanged
                → 1s dispatch 循环处理 → sendCancelMovement
                → 此时车端已是 RTT 持有者，MRCS 通道命令被忽略
```

`stopVehicleOnMrcsControlLost` 只调 `sendCancelMovement()`，**不校验车端 ACK，也不清理 traffic 运行时**（pauseOrder 会调 traffic，但 cancel 本身无效）。

---

## 四、排查项清单

### A. 已排除（本次日志可证伪）

| #   | 排查项                            | 结果   | 依据                                      |
| --- | ------------------------------ | ---- | --------------------------------------- |
| A1  | Core 未检测 MRCS→非 MRCS           | ❌ 排除 | 5 次 `[control-lost]` 日志                 |
| A2  | CARE 类型映射错误导致不触发               | ❌ 排除 | `takenBy:RTT`，`shouldStop` 对 RTT 为 true |
| A3  | `controlRightChanged` 粘性标志丢失   | ❌ 排除 | 每次抢控均触发且 clear                          |
| A4  | 未调 pauseOrder / traffic cancel | ❌ 排除 | 13:54:08 有 pause + 3 次 cancel           |
| A5  | 车端未上报控制权变化                     | ❌ 排除 | 08.613 起持续 RTT                          |
| A6  | TCP 发送失败                       | ❌ 排除 | 三次 `cancelOk:true`                      |

### B. 已确认的问题

| #   | 排查项                              | 严重度   | 说明                                                                      |
| --- | -------------------------------- | ----- | ----------------------------------------------------------------------- |
| B1  | **丢控后 cancelMovement 无效**        | 🔴 根因 | 无 `RESPONSE_CANCEL_MOVEMENT` ACK，车仍 RUNNING + localPath 缩减              |
| B2  | **检测与响应偏晚**                      | 🟡 加剧 | 仅在 1s `dispatchBlockTasks` 处理；本案例 ~666ms（07.612→08.278）                 |
| B3  | **“sent OK = 已停车”误导**            | 🟡 运维 | 日志写 stopped，但只代表发送成功                                                    |
| B4  | **Core PAUSE vs 车端 RUNNING 不一致** | 🟡 表象 | `setState(PAUSE)` 被下一帧上报覆盖                                              |
| B5  | **orderTake 不可用提前 continue**     | 🟡 隐患 | `!orderTakeAvailable && !needsRecovery` 在控制权检查之前 `continue`，部分场景会跳过抢控停车 |
| B6  | **traffic 仍在跑时抢控**               | 🟡 时序 | 13:54:07.978 仍有 block running，08.278 才 pause                            |

### C. 待现场/车端/CARE 侧验证

| #   | 排查项                                         | 方法                                            |
| --- | ------------------------------------------- | --------------------------------------------- |
| C1  | 车端固件：控制权切换时是否自动 cancel 旧持有者 movement task   | 抓车端 log / 问固件团队                               |
| C2  | CARE 抢控时是否主动发 stop/cancel                   | CARE 侧 log                                    |
| C3  | MRCS 丢控后 `CMD_CANCEL_MOVEMENT` 是否被协议层拒绝     | 车端 command 日志                                 |
| C4  | 是否存在“丢控前窗口”可 preempt cancel                 | 高频率拉状态 + 示波对比抢控时刻                             |
| C5  | `apply control right result:false` 时是否应触发停车 | 13:44:03 S800_3# 案例：已 RTT 仍 RUNNING+localPath |

---

## 五、问题归因

```
                    ┌─────────────────────────────────────┐
                    │  CARE 抢控（车端控制权 → RTT）         │
                    └─────────────────┬───────────────────┘
                                      ▼
              ┌───────────────────────────────────────────────┐
              │ 车端状态上报：controlRight=RTT, RUNNING, localPath>0 │
              └─────────────────┬─────────────────────────────┘
                                ▼
         ┌──────────────────────────────────────────────────────┐
         │ Core：controlRightChanged + dispatchBlockTasks (≤1s)    │
         │   → pauseOrder + cancelMovement × N                    │
         └─────────────────┬────────────────────────────────────┘
                           ▼
              ┌────────────────────────────────────────┐
              │ 车端：已是 RTT 持有者，忽略 MRCS cancel   │  ← 根因
              │ Matrix 已下发路径惯性继续执行              │
              └────────────────────────────────────────┘
```

**责任划分：**

- **Core 逻辑**：检测与触发基本正确，但依赖“丢控后 cancel”这一错误假设；存在 dispatch 延迟、orderTake 跳过等隐患。
- **车端/协议**：抢控后 Matrix 命令无效属预期行为；**缺“控制权切换时自动停车”机制**才是现场不停车的直接原因。
- **状态上报**：无异常，反而证明车确实在跑。

---

## 六、彻底修复方案（需多层联动）

只做 Core 小改**无法单独保证**现场立刻停车，必须分层：

### 层 1 — 车端固件 / CARE（必须，否则无法“立即”停）

1. **控制权从 MRCS 切走时，车端自动 cancel 当前 movement task**（或进入 PAUSE）。
2. **CARE 抢控流程**：takeover 后第一件事发 stop/cancel，不能沿用 Matrix 缓冲路径。

### 层 2 — Core 调度（应做，补窗口 + 修隐患）

1. **在 `updateSinevaVehicle` 解析到 `wasMRCS && !isMRCS` 时同步触发停车**（不等 1s dispatch），在更新 `controlRight_` 之前发 cancel（争最后窗口）。
2. **MRCS + RUNNING + localPath>0 时提高状态拉取频率**（如 100~200ms `sendVehicleBaseState`），缩短发现→cancel 窗口。
3. **将控制权检查移到 `orderTakeAvailable` continue 之前**，避免 UNAVAILABLE 时跳过抢控处理。
4. **`stopVehicleOnMrcsControlLost` 增强**：
   - 走 `VehicleCommandSender` + **等待/校验 `RESPONSE_CANCEL_MOVEMENT`**
   - 2~3s 内仍 RUNNING 且 localPath 缩减 → 打 fault + 明确 warn（非“stopped”）
   - 同步 `delTraffic` / `clearVehicleTrafficRuntime`
5. **修正日志**：`sent OK` 改为 `cancel requested`；仅 ACK + 车端 PAUSE/FREE + localPath=0 才记 `stopped`。

### 层 3 — 验证与回归

1. 单元测试：orderTake UNAVAILABLE + 抢控边沿仍触发 stop。
2. 集成测试：模拟 MRCS→RTT，断言 fault 在“无 ACK 仍 RUNNING”时触发。
3. 现场用例：S800_4# 调度中 CARE 抢控 → 要求 **≤500ms 内 state≠RUNNING 或 twist≈0**。

---

## 七、为何前两版改完仍复现

| 版本改动                                          | 解决了什么                  | 没解决什么                           |
| --------------------------------------------- | ---------------------- | ------------------------------- |
| v1: `controlRightChanged` 边沿                  | 避免 CARE 稳定持控时重复 cancel | 丢控后 cancel 仍无效                  |
| v2: dispatch 内 `stopVehicleOnMrcsControlLost` | 抢控时会 pause + cancel    | 仍依赖丢控后发 cancel；无 ACK 校验；1s 循环延迟 |

---

## 八、建议的修复优先级

1. **P0（车端/CARE）**：控制权切换自动停车 — 唯一能保证“立即停”的路径。
2. **P1（Core）**：状态更新即时响应 + orderTake 跳过修复 + ACK 校验与 fault。
3. **P2（Core）**：高频状态拉取 + 日志修正 + 集成测试。

---

如果你希望我开始落地 **P1 Core 改动**（状态边沿即时停车、orderTake 顺序修复、ACK 校验与 fault），我可以直接改代码并补单元测试。同时建议把 **P0 车端/CARE 需求**同步给固件和 CARE 团队，否则 Core 侧只能“尽力 cancel + 告警”，无法保证物理立即停止。







---

# Duty Diff

当前控制权在调度时，其他模块比如RTT枪走控制权是车端的逻辑码？RTT的抢夺控制权一定就会成功吗?当前控制权在调度，调度不可以立即停车再把控制权给到RTT吗？

## 三个问题的直接回答

### 1. RTT 抢控制权是车端逻辑吗？

**是，仲裁在车端固件，不在 Matrix Core。**

协议里控制权是车端维护的状态，Matrix 和 RTT/CARE 都是连到同一台车的客户端：

```836:843:core/module/domain/src/main.proto
message ControlRight {
    uint32 type = 1;  // 0-无 1-rtt 2-mrcs
    uint32 port = 2;  // 端口
    string ip = 3;  // ip
    string nick_name = 4;  // 昵称
    string time = 5;  // 抢占时间
};
```

流程大致是：

```
RTT/CARE  ──CMD_APPLY_CONTROL_RIGHT──►  车端固件（仲裁者）
Matrix    ──CMD_APPLY/RELEASE────────►  车端固件
                ▲
                └── VehicleBaseState.control_right 周期性上报当前持有者
```

Matrix Core **不参与** RTT 申请控制权的协商，只是从状态上报里看到 `MRCS → RTT` 变化，再触发 pauseOrder / cancelMovement。  
CARE 在 proto 里通常显示为 **RTT(1)**，不是 ROBOSHOP(3)，但都是“非 MRCS 持有者”。

---

### 2. RTT 抢控制权一定成功吗？

**不一定，由车端固件策略决定。**

Matrix 侧有明确的成功/失败反馈：

- `apply control right result:true` — 申请成功  
- `apply control right result:false` — 申请失败（已有其他持有者）

现场日志里两种都有，例如 S800_3# 在 13:44:03 已有 RTT 持控时，Matrix 再申请会 `result:false`。

| 场景                 | 典型结果                         |
| ------------------ | ---------------------------- |
| 无人持控，Matrix/RTT 申请 | 通常成功                         |
| MRCS 持控，RTT 申请抢控   | **看固件策略**：可能强制抢占，也可能拒绝       |
| RTT 已持控，Matrix 再申请 | 通常失败（日志已证实）                  |
| Matrix 主动释放        | `RELEASE_CONTROL_RIGHT`，一般成功 |

所以：**RTT 抢控不是协议层“必然成功”**；你们现场 CARE 抢控经常能成，说明当前固件对 RTT 抢占策略偏宽松，但这不能当成 guaranteed behavior。

---

### 3. 调度持控时，能否“先停车，再把控制权给 RTT”？

要分 **谁发起** 两种完全不同的路径：

#### 路径 A：Matrix 主动交出去 — **可以，且代码已这么做**

`setVehicleControlRight(false)` 的设计就是：**还在 MRCS 时先 cancel，再 release**：

```484:488:core/module/domain/src/vehicle_manager.cpp
        } else {
            // Cancel Matrix-dispatched motion while MRCS still holds control, then release.
            responseType = proto::Response::RESPONSE_RELEASE_CONTROL_RIGHT;
            vehicle->sendCancelMovement();
            vehicle->sendReleaseControlRight();
```

这是正确的主动交控流程：

```
Matrix 仍持 MRCS
    → cancelMovement（此时命令有效）
    → releaseControlRight
    → 车端 control_right 变为 NONE/RTT
    → RTT 再 apply（若需要）
```

通过 API `/api/setControlRight` 或 Engine 的 `SetControlRight` 走的就是这条路径。

#### 路径 B：RTT/CARE 主动抢 — **Matrix 做不到“先停再交”**

RTT 抢控时 Matrix **不在协商链里**：

```
RTT 发 APPLY ──► 车端直接裁决、切换持有者
                      ↓
              状态上报已是 RTT
                      ↓
         Matrix 才发现 → 再发 cancel（往往已无效）
```

这就是 `logs/20260807` 里看到的现象：Core 已 pause + 发了 3 次 cancel，但车端已是 RTT，命令不被执行，车继续跑 Matrix 已下发的路径。

**Matrix 无法：**

- 在 RTT 抢控前收到“即将被抢”的通知  
- 在 RTT 抢控过程中插入“我先停，你再拿”的握手  
- 在丢控后用 MRCS 通道命令让车停下（协议上通常无效）

---

## 架构关系（一图看清）

```
┌─────────────┐                    ┌─────────────┐
│  Matrix     │                    │  RTT/CARE   │
│  (MRCS)     │                    │  (RTT)      │
└──────┬──────┘                    └──────┬──────┘
       │  APPLY / RELEASE / CANCEL        │  APPLY
       └────────────┬─────────────────────┘
                    ▼
         ┌──────────────────────┐
         │   车端固件（仲裁 + 执行）  │  ← 抢控逻辑在这里
         │   control_right 持有者   │
         │   movement task 执行    │
         └──────────────────────┘
                    │
                    ▼ 状态上报
         ┌──────────────────────┐
         │   Matrix Core         │  ← 只观察、后反应
         │   检测 MRCS→RTT 边沿    │
         └──────────────────────┘
```

---

## 对“立即停车”的实际含义

| 发起方                   | 能否保证立即停       | 原因                                   |
| --------------------- | ------------- | ------------------------------------ |
| **Matrix 主动 release** | ✅ 可以          | 还在 MRCS 时 cancel 有效                  |
| **RTT 主动抢控**          | ❌ Core 单独不能保证 | 发现时往往已丢控；cancel 无效                   |
| **车端固件**              | ✅ 应在此处保证      | 切换控制权时自动 cancel/pause 旧持有者的 movement |
| **CARE 侧**            | ✅ 应在此处保证      | 抢控后主动发 stop/cancel                   |

---

## 结论与建议

1. **抢控是车端逻辑**，Matrix 只是事后从状态上报感知。  
2. **RTT 抢控不保证成功**，但你们现场固件对 RTT 抢占偏宽松，所以经常能抢成。  
3. **“先停再交”只有 Matrix 主动 release 时成立**；RTT 强行抢时，Matrix 没有插手机会。

若要真正解决 CARE 抢控后仍滑行的问题，需要 **三层配合**：

- **车端**：控制权切换时自动 cancel 原持有者 movement（最根本）  
- **CARE**：抢控后立即 stop  
- **Matrix**：主动交控走 `cancel → release`；被动抢控只做 pause + 告警，不假设丢控后 cancel 有效  

如果需要，我可以继续梳理：**CARE 抢控前 Matrix 是否还能做什么**（例如更高频拉状态、在最后一次 MRCS 帧上 preempt cancel 等），以及哪些必须改车端固件。
