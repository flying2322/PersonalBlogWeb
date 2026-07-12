## 问题与方案

原先 `AutomaticDoor::doExecuteCommands()` 在检测到门已打开后，会从队列中移除 Open 命令，因此开门 Modbus 信号只发一次。`DeviceService` 每 1s 轮询一次，门控信号无法持续保持。

新逻辑：

1. 机器人 `enter()` → 记录通行 token，开启 sustain open 模式，发送首次开门信号
2. 每次 `executeCommands()`（约 1s 一次）→ 若 sustain 仍活跃，重复发送开门信号
3. 机器人 `leave()` → 移除 token；仅当所有通行 token 都离开 才发送关门信号并停止 sustain
4. 手动关门 / 急停 → 清空 sustain session，停止持续开门

## 改动文件

| 文件                                | 说明                  |
| --------------------------------- | ------------------- |
| `automatic_door_sustain.hpp`      | 通行 token 会话管理（支持多车） |
| `automatic_door.hpp`              | 持续开门/关门核心逻辑         |
| `test_automatic_door_sustain.cpp` | 单元测试（mock 机器人持续通行）  |
| `unit_tests/CMakeLists.txt`       | 新增测试目标              |

## 单元测试覆盖

- 单车通行：`enter` → 多次 `executeCommands` → 持续累加开门次数 → `leave` → 发一次关门 → 之后不再开门
- 双车通行：第一辆车 `leave` 后仍持续开门，第二辆车 `leave` 后才关门

## 编译与测试命令

cd /home/wp/Documents/Matrix/core/build

# 若 CMake 尚未配置过新测试目标，先执行：

cmake ..

# 编译单元测试 + 主程序

cmake --build . --target unit_tests_automatic_door_sustain matrix_core -j$(nproc)

# 运行自动门相关用例

./module/domain/unit_tests/unit_tests_automatic_door_sustain "[autodoor]"

实测结果：All tests passed。

## 实车联调建议

关注 Core 日志中同一自动门在机器人通过期间是否周期性出现 open 写寄存器；机器人完全离开后应出现一次 close，且之后不再有 open。如有异常，把 `DeviceService` 周期内该门的 execute 日志贴过来即可继续排查。

`feature/autodoor` 分支上的开发已完成：`unit_tests_automatic_door_sustain` 与 `matrix_core` 均已编译通过，自动门用例 **All tests passed (33 assertions in 2 test cases)**。

实现效果：机器人 `enter` 后持续发送开门信号（约每 1s 一次），直到 `leave` 且所有通行 token 清空后才发关门并停止开门；双车场景下也会保持开门直到最后一车离开。

改动在本地未提交，请你审阅后自行提交。
