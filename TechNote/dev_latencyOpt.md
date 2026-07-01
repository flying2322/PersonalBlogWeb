因为Matrix-Core多处用到多线程启动服务，导致当前Matrix在启动时耗时较长，并且在下载地图文件时，前端也经常会报异常和失败，帮我基于当前develop分支新建一个feature/latencyopt分支优化启动和地图下载时多线程的处理逻辑。





已在 `feature/latencyopt` 分支（基于 `develop`）完成启动与地图下载相关的延迟优化，并通过编译和单元测试。

## 问题根因

1. 启动慢：`Application::start()` 在标记启动完成后 固定 sleep 30 秒，阻塞 `CoreApplication::start()` 返回，Engine 长时间无法就绪。
2. 地图下载失败/超时：`Vehicle::sendDownloadMap()` 用 固定 sleep 10 秒 轮询结果，既不准确又阻塞 SWIG/HTTP 调用线程；大文件 10 秒内未完成即失败，与 Engine 120s 超时也不匹配。
3. 重复 io 线程：`SceneContext::startIoContext()` 与 `VehicleManager::start()` 对同一 `io_context` 各起一个 `run()` 线程，增加竞争和调度开销。
4. 地图加载串行：`MapManager::updateMap()` 多地图文件时顺序加载，场景初始化偏慢。

## 改动摘要

| 模块                    | 改动                                                                                |
| --------------------- | --------------------------------------------------------------------------------- |
| `application.hpp`     | 移除 30s 启动阻塞 sleep                                                                 |
| `scene_context.hpp`   | 场景 io 仅标记 active，事件循环统一由 `VehicleManager` 驱动                                      |
| `vehicle.cpp`         | 地图下载改用 `condition_variable` 等待（最长 120s）；同车并发下载互斥；Seer 响应正确写入 `downloadedMapData_` |
| `map_manager.cpp`     | 多地图文件 并行加载（`std::async`）                                                          |
| `vehicle_manager.cpp` | 启动等待由 100ms 降至 10ms                                                               |

## 地图下载新逻辑

API 线程 sendDownloadMap()

→ 加锁，拒绝同车并发下载

→ 发送下载指令

→ condition_variable 等待（最长 120s）

← 车辆响应线程 notifyDownloadFinished()

→ 返回结果（成功则立即返回，不再固定等 10s）

## 单元测试

新增 `test_latency_opt.cpp`，4 个用例、11 条断言全部通过：

cd core/build

make unit_tests_latency_opt -j$(nproc)

./module/domain/unit_tests/unit_tests_latency_opt -r compact

##### Passed all 4 test cases with 11 assertions.

覆盖：下载 wait/notify 机制、并发拒绝、MapManager 并行加载、下载超时常量 120s。

## 预期效果

- Matrix-Core / Engine 启动：去掉 30s 阻塞，启动就绪时间显著缩短。
- 前端地图下载：响应到达即返回；超时对齐 120s；同车重复请求不再互相干扰。
- 多地图场景：加载可并行，初始化更快。

当前改动尚未 commit。需要我帮你提交或开 PR 可以说一声。



`feature/latencyopt` 上的延迟优化已完成，`matrix_core` 编译通过，单元测试 **4/4 通过**。

**主要改动：**

- 去掉启动时 30 秒阻塞 sleep
- 地图下载改为 condition_variable 等待（最长 120s），替代固定 10s sleep
- 消除 Scene / VehicleManager 重复 io 线程
- 多地图文件并行加载

**验证：**

```bash
./module/domain/unit_tests/unit_tests_latency_opt -r compact
# Passed all 4 test cases with 11 assertions.
```

改动目前在分支上，尚未 commit。需要提交或开 PR 时告诉我即可。
