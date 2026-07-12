已为 `Response` 和 `FaultInfo` 增加 `messageParams`（`std::vector<std::string>`），并打通从故障产生到 API/WebSocket 下发的完整链路。

## 改动概览

### 1. API 模型（`core_application.hpp`）

```cpp
class Response {
    int code;
    std::string message;
    std::string createOn;
    std::vector<std::string> messageParams;  // 新增
};

class FaultInfo {
    std::string code;
    std::string message;
    std::string time;
    std::vector<std::string> messageParams;  // 新增
};
```

### 2. 故障消息链路

- **`FaultMessage`**：新增 `messageParams_` 字段及 getter/setter
- **`FaultMessageManager`**：新增支持 `vector<string>` 的 `warn/error` 重载；原有 `vehicleId` 重载会自动填充 `messageParams = {vehicleId}`
- **`Internationalization`**：新增 `codeMessage(int, const vector<string>&)`，支持多参数模板（如 `车辆:{}`）

### 3. 数据填充（`core_application.cpp`）

- 新增 `buildFaultInfo()`，从 `FaultMessage` 构建 `FaultInfo` 时自动填充 `messageParams`
- `failed()` 支持传入 `messageParams`；仅当响应类型有该字段时才赋值（`Response` 有，其他 `*Response` 不受影响）

### 4. WebSocket（`web_socket_server.hpp`）

告警 JSON 增加 `messageParams` 数组字段。

### 5. C# 绑定

- `CoreResponse.MessageParams`
- `CoreFaultInfo.MessageParams`
- 手动更新了 `core.cs` 和 `core_application_wrap.cxx` 的 SWIG 绑定

## 使用示例

**上报带车辆 ID 的故障（现有代码无需改）：**

```cpp
faults::FaultMessageManager::instance()->warn(
    faults::FaultCategory::VEHICLE,
    common::Code::VEHICLE_DISCONNECTED,
    vehicle->id);  // 自动写入 messageParams = {"S800_1#"}
```

**多参数场景：**

```cpp
faults::FaultMessageManager::instance()->error(
    faults::FaultCategory::TASK_ASSIGN,
    common::Code::ASSIGN_VEHICLE_NOT_EXIST,
    std::vector<std::string>{vehicleId, vehicleName});
```

**API 失败响应：**

```cpp
failed(response, common::Code::VEHICLE_NONE, std::vector<std::string>{request.vehicleId});
```

## 返回 JSON 示例

```json
{
  "code": "809",
  "message": "车辆通讯断开, 车辆:S800_1#",
  "time": "2026-06-05 12:00:00",
  "messageParams": ["S800_1#"]
}
```

`message` 仍是格式化后的可读文本；`messageParams` 提供结构化参数，前端可按需展示车辆 ID、名称等。

---

`matrix_core` 已编译通过。部署时需将新的 `libmatrix_core.so` 复制到 `engine/Sineva.Matrix.Core/core/` 目录。若你本地有 SWIG 环境，建议后续用 SWIG 重新生成绑定以替代手动补丁。
