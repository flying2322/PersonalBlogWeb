dev_mem_leak.md

结论先说：**初步诊断里“托管矩阵运算泄漏”方向偏了**。`Sineva.Matrix.E` 把 `libmatrix_core` 加载进同一进程，当前最像真泄漏、且能解释 RSS 缓慢爬到 12GB 的，是 **Core `vehicleStatus()` 的 Native 内存从未释放**（每 100ms 一次），而不是典型的 .NET 矩阵缓存。

---

## 根因 P0：`vehicleStatus` SWIG 所有权错误（Native 泄漏）

### 链路

```text
RobotsStatusIngestTimer (100ms)
  → CoreAppService.VehicleStatus()
  → CoreApplication::vehicleStatus()   // C++ new VehicleResponse
  → SWIG: new VehicleResponse(cPtr, false)  // cMemoryOwn=false
  → using Dispose()  // 因 swigCMemOwn=false，不调用 delete
  → Native 堆永久泄漏
```

### 代码证据

C++ 每次 `new`，且是唯一返回裸指针的 Core API：

```912:915:core/src/core_application.cpp
VehicleResponse* CoreApplication::vehicleStatus() {
    std::lock_guard<std::mutex> lock(access_);
    VehicleResponse* response = new VehicleResponse();
```

SWIG 包装时 **明确不接管所有权**：

```11963:11965:engine/Sineva.Matrix.Core/api/core.cs
  public VehicleResponse vehicleStatus() {
    global::System.IntPtr cPtr = libmatrix_corePINVOKE.CoreApplication_vehicleStatus(swigCPtr);
    VehicleResponse ret = (cPtr == global::System.IntPtr.Zero) ? null : new VehicleResponse(cPtr, false);
```

`Dispose` 只在 `swigCMemOwn==true` 时才 `delete`：

```9020:9026:engine/Sineva.Matrix.Core/api/core.cs
  protected virtual void Dispose(bool disposing) {
    lock(this) {
      if (swigCPtr.Handle != global::System.IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          libmatrix_corePINVOKE.delete_VehicleResponse(swigCPtr);
```

Engine 侧以为 `using` 会释放，实际不会：

```178:181:engine/Sineva.Matrix.Engine.Service/Implement/CoreAppService.cs
            var result = ExecuteWithLogging(methodName, parameters, () =>
            {
                using var response = action();
                var managedResponse = mapper(response);
```

调用频率：`RobotsStatusIngestTimer` **每 100ms** 一次（约 864,000 次/天）。每次响应含全车状态、路径点、告警等，体积不小 → 与 **VmRSS 缓慢上涨、GC 忙但堆降不下来、SHR 很小** 高度吻合（泄漏在 **anon/native**，GC 管不到）。

### 为何看起来像“GC 无效”

| 现象                         | 解释                                         |
| -------------------------- | ------------------------------------------ |
| Server GC 忙、堆不降            | 托管侧也在狂分配（JSON clone/回放），但 **大头可能在 native** |
| VIRT 很大                    | .NET + native 地址空间预留                       |
| `dumpheap -stat` 未必看到 12GB | 很多泄漏在 **非托管堆**，托管快照会低估                     |

### 验证（生产可做）

1. 对比 **托管堆 vs RSS**：`dotnet-counters` 看 `gc-heap-size`；若 RSS≈12GB 而托管堆只有 1–3GB → 基本坐实 native。
2. 临时停 `RobotsStatusIngestTimer` 或把周期改成 5s，观察 RSS 是否平台期。
3. 修所有权后压测：RSS 应趋稳。

### 修复方向（任选其一）

1. **推荐**：C++ 改成按值返回 `VehicleResponse vehicleStatus()`（与其它 API 一致），重新生成 SWIG。  
2. SWIG 加 `%newobject CoreApplication::vehicleStatus;`，使 `cMemoryOwn=true`。  
3. 临时热修：C# 侧在 Mapster 拷贝后手动 `delete_VehicleResponse`（或临时把 `false` 改 `true` 并确认无 double-free）。

---

## 次要嫌疑（会放大 RSS / 托管压力）

| 优先级 | 位置                                                | 问题                                                                           |
| --- | ------------------------------------------------- | ---------------------------------------------------------------------------- |
| P1  | `DynamicCache`                                    | 无 SizeLimit；无 TTL 的 `Set` 永久进 `_persistentStore`；`CleanupExpired()` **从未调用** |
| P2  | `SceneMapDataCache`                               | 所有启用场景逻辑地图常驻内存                                                               |
| P3  | 告警缓存                                              | DB 按时间删行，内存缓存不同步；Engine 自产告警未必被 ingest 清掉                                    |
| P4  | 100ms 状态链路                                        | `CloneVehicleStatus` JSON 往返 + 回放 ingest → 高分配，加重 GC（本身不一定是“只涨不跌”的主因）        |
| P5  | Core `localRoadLines` / section / 无 clear 的 fault | 长跑次要增长点                                                                      |

你文档里的 `DOTNET_GC_VM_RELEASE` 等只能缓解 **托管空闲页归还**，**治不了** 上述 `new` 不 `delete`。

---

## 建议处置顺序

1. **立刻**：低峰重启缓解 OOM（临时）。  
2. **优先修**：`vehicleStatus` 所有权 / 改为按值返回（预期收益最大）。  
3. **确认**：`dotnet-counters` 对比 `gc-heap-size` 与 `/proc/<pid>/status` 的 `VmRSS`。  
4. **再治**：`DynamicCache` 上限 + 定时 `CleanupExpired`；场景地图懒加载；告警缓存与 DB 对齐。

需要的话可切到实现：先改 Core 返回值 + 修 SWIG/`core.cs` 所有权，并给一个可回滚的验证步骤。



---



已修好并完成全量 SWIG 重生。

### 原因

`-outdir /tmp/swig_out` 目录不存在，SWIG 无法写出 `VectorString.cs`。

### 已执行

```bash
mkdir -p /tmp/swig_out
cd core/src
swig -c++ -csharp -namespace Sineva.Matrix.Core -outdir /tmp/swig_out -o core_application_wrap.cxx core_application.i
```

并把生成的 98 个 `.cs` 合并进 `engine/Sineva.Matrix.Core/api/core.cs`。

### 关键结果已确认

```csharp
public VehicleResponse vehicleStatus() {
    VehicleResponse ret = new VehicleResponse(..., true);  // 接管所有权
}
```

C++ wrap 侧按值拷贝后再 `new`，与 `ioInfo` 一致。

### 编译

- `libmatrix_core.so`：通过  
- `Sineva.Matrix.Core`：通过  

以后可直接用：

```bash
mkdir -p /tmp/swig_out && cd ~/Documents/Matrix/core/src && \
swig -c++ -csharp -namespace Sineva.Matrix.Core -outdir /tmp/swig_out -o core_application_wrap.cxx core_application.i
```

代码待你审核，未提交。
