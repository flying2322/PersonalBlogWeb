```protobuf
syntax = "proto3";

package proto.equipment.model;

option java_package = "com.hairoutech.proto.equipment.model";
option java_outer_classname = "EquipmentModel";
option java_multiple_files = true;

message PbEquipmentModel {
  string code = 1;

  oneof obj {
    PbEquipmentPtlTag ptl_tag = 10; //electrical tag: pick to light
    PbEquipmentPtlController ptl_controller = 11; // pick to light controller
    PbEquipmentHaiport haiport = 12;
    PbEquipmentDoor door = 13;
    PbEquipmentElevator elevator = 14;
    PbEquipmentManipulator manipulator = 15;   // ji xie bi?机械臂
    PbEquipmentHaiportP20 haiport_p20 = 16;
    PbEquipmentConveyor conveyor = 17;
    PbEquipmentSensor sensor = 18;
    PbEquipmentSafetyVest safety_vest = 19;
  }
}

message PbEssEquipmentRepository { // ESS equipment repo.
  repeated PbEquipmentModel equipment_model = 1;
  int64 update_sequence = 2;
  int64 system_start_time = 3;
}

message PbEquipmentPtlController {
  string code = 1; // 电子标签控制器code

  oneof type {
    Epostm epostm = 6;
    Epostp epostp = 7;
    Aioi aioi = 8;
    Rybo rybo = 9;
    Kotech kotech = 10;
    Atop atop = 11;
    EpostT epostt = 12;
  }

  message Epostm {
    string host_url = 1;
    string feedback_url = 2;
  }

  message Epostp {
    string pub_channel = 1; // 推送控制器指令的通道
    string sub_channel = 2; // 接收控制器回传的通道
    int64 station_id = 3; // 实际上是控制器id，设备程序需要这个字段
  }
  message Aioi {
    string host = 1;
    int32 port = 2;
  }
  message Rybo {
    string host = 1;
    int32 port = 2;
  }
  message Kotech {
    string to_host = 1;
    int32 to_port = 2;
    string from_host = 3;
    int32 from_port = 4;
  }
  message Atop {
    string host = 1;
    int32 port = 2;
  }
  message EpostT {
    string host_url = 1;
    string feedback_url = 2;
  }
}

message PbEquipmentPtlTag {
  string code = 1; // 电子标签别名，用于跟外部交互
  string index = 2; // 电子标签在控制器中的地址
  string controller_code = 3; // 所属控制器的code

  oneof type {
    Rybo rybo = 7;
    Epostm epostm = 8;
  }

  message Rybo {
    enum Type {
      RY30_1550 = 0;
      RY3Z_4831 = 1;
      RY30_IO11 = 2; // 巷道灯
    }
    Type type = 1;
  }

  message Epostm {
    enum Type {
      NUMBER = 0;
      CHINESE = 1;
    }
    Type type = 1;
  }
}

message PbEquipmentHaiport {
  string code = 1;
  bool is_enabled = 2; // 是否启用，如果启用将会创建子haiport actor尝试建立modbus tcp连接
  Action action = 3;
  map<int32, bool> tray_id_to_enabled = 4; // 可用tray的层数，从零开始
  PbEquipmentModbusTcpConfig modbus_config = 5;
  string location_code = 6; // 相关联的工作位
  int32 enough_tray_count_for_call_robot_at_put_haiport = 7; // tray有足够容器时，呼叫机器人上料
  int32 enough_tray_count_for_put_container_to_robot = 8; // tray有足够容器时，上料机上料

  Status status = 9;

  // real-time status
  int64 last_heartbeat_time_ms = 10; // 最后一次心跳时间

  // hardware status
  map<int32, int32> modbus_address_to_short = 11; // 读取的modbus地址 to actual value
  map<int32, string> tray_to_barcode = 12; // 层数对应的条码

  bool outside_after_take = 13; // 取下后箱子在库外
  map<int32, ConveyorContainerRequest> conveyor_layer_to_container_request = 14; // 层数对应的有无容器请求 key: conveyor layer, 从 0 开始
  map<int32, bool> conveyor_layer_to_is_rolling = 15; // 层数对应的输送线滚筒是否在转动 key: conveyor layer, 从 0 开始
  string zone = 16;
  int32 ptm_id = 17;

  enum Action {
    PUT = 0; // 给机器人上料的上料机
    TAKE = 1; // 给机器人卸料的卸料机
  }

  message Status {
    bool is_robot_in_place = 1; // 机器人是否在位
    bool has_abnormal = 2; // 是否有异常
    OperationState operation_state = 3; // 当前上/卸料状态
    bool can_operate = 4; // 是否可以上/卸料
    map<int32, WritingState> layer_to_writing_state = 5; // 层数对应的写码状态
    bool has_container = 6; // 是否有容器
    bool is_full = 7; // 是否满载
    bool allow_entry = 8; // 是否允许进入
    bool can_switch_mode = 9; // 是否可以切换模式（用于上卸一体）
    SwitchModeState switch_mode_state = 10; // 切换模式状态
    bool is_pause = 11; // 是否暂停
    // TODO: add boolean of can release robot
  }

  enum SwitchModeState {
    NO_SWITCH = 0;
    SWITCHING = 1;
    SWITCH_FINISH = 2;
    SWITCH_FAILED = 3;
    SWITCH_STATE_INVALID = 9;
  }

  enum OperationState {
    NO_OPERATION = 0;
    OPERATING = 1;
    ROBOT_NOT_IN_PLACE = 2;
    OPERATION_FINISH = 3;
    OPERATION_FAILED = 4;
    OPERATION_STATE_INVALID = 9;
  }

  // 外部写码状态
  enum WritingState {
    NO_WRITE = 0;
    WRITING = 1;
    WRITE_FINISH = 2;
    WRITE_BARCODE_IS_EXIST = 3;
    WRITE_STATE_INVALID = 9;
  }

  message ConveyorContainerRequest {
    string node_code = 1;
    bool has_container = 2;
  }
}

message PbEquipmentHaiportP20 {
  string code = 1;
  bool is_enabled = 2; // 是否启用，如果启用将会创建子haiport actor尝试建立modbus tcp连接
  int32 highest_available_layer = 3; // 最高可操作层数，最高上料层数为该值 -1。
  PbEquipmentModbusTcpConfig modbus_config = 5;
  string location_code = 6;
  int32 enough_conveyor_container_count_for_call_robot = 7;
  Status status = 9;

  // real-time status
  int64 last_heartbeat_time_ms = 10;

  // hardware status
  map<int32, int32> modbus_address_to_short = 11; // 读取的modbus地址 to actual value
  map<int32, string> tray_to_barcode = 12;
  string picking_port_barcode = 13; 
  bool outside_after_take = 14; // 取下后箱子在库外
  string zone = 15;
  int32 ptm_id = 16;

  message Status {
    bool is_robot_in_place = 1;
    bool has_abnormal = 2;
    OperationState operation_state = 3;
    bool can_operate = 4;
    bool has_container_putting = 6; // 指输送线或者提升机上料位有箱
    bool allow_entry = 8;
    int32 conveyor_container_signal = 9; // 第N bit表示输送线第N+1段有箱子（靠近卸料口的为第一段）
    int32 lifter_container_signal = 10; // 0 bit 表示上层（卸料）有箱，1 bit表示下层（上料）有箱
    // TODO: add boolean of can release robot
  }

  enum OperationState {
    NO_OPERATION = 0;
    REQUESTING_OPERATION = 1;
    OPERATING = 2;
    OPERATION_FINISH = 3;
    OPERATION_FAILED = 4;
    OPERATION_STATE_INVALID = 9;
  }
}


message PbEquipmentModbusTcpConfig {
  string host = 1;
  int32 port = 2;
  int32 slave_id = 3;
}
message PbEquipmentDoor { // 12 attributes: code type state zone pauseZoneAsOpen openStrategy updated lasdSendCommandTime isFinalState safetyAccessAreaCodes pointCode ptmID
  string code = 1;
  Type type = 2;
  State state = 3;
  string zone = 4;
  bool pause_zone_as_open = 5;
  OpenStrategy open_strategy = 6;
  int64 updated = 7;
  int64 last_send_command_time = 8;
  bool is_final_state = 9;
  repeated string safety_access_area_codes = 10; // 关联的绿色通道区域编码
  string point_code = 11;  // 关联的点位编码
  int32 ptm_id = 12;

  enum OpenStrategy {
    OPEN_WHILE_ROBOT_IS_PASSING = 0; // 经过打开
    ALWAYS_OPEN = 1; // 常开
    OPEN_WHILE_ROBOT_IS_IN = 2; // 机器人在门内时常开
  }

  enum Type { // safety door: 8  adidas haizhuo JD inland automatic http KJY oreal
    ADIDAS_SAFETY_DOOR = 0;
    HAIZHUO_AUTOMATIC_DOOR = 1;
    JD_LA_SAFETY_DOOR = 2;
    INLAND_SAFETY_DOOR = 3;
    AUTOMATIC_DOOR = 4;
    HTTP_INTERACTION_DOOR = 5;
    KJY_AUTOMATIC_DOOR = 6;
    OREAL_SAFETY_DOOR = 7;
  }

  enum State {
    OPEN = 0;
    CLOSED = 1;
  }

}

message PbEquipmentElevator {
  string code = 1;
  Type type = 2;
  Mode mode = 3;
  repeated int32 floor = 4;
  map<int32, Floor> floor_item = 5;
  bool is_final_state = 6; // 查询到的电梯状态是否是最终状态
  ElevatorState state = 7;
  int64 updated = 8;
  int64 system_start_time = 9;

  enum Mode {
    AUTO = 0; // 自动模式-机器人控制电梯
    MANUAL = 1; // 手动控制模式-人工控制电梯
  }
  enum ElevatorState {
    NORMAL = 0; // 正常状态
    UP = 1; // 上升
    DOWN = 2; // 下降
    ABNORMAL = 3; // 故障
  }
  message Floor {
    string zone = 1;
    FloorState state = 2;
    bool is_on_the_floor = 3; // 电梯是否在该楼层
    repeated string inside_point_code = 4;
  }
  enum FloorState {
    OPEN = 0;
    CLOSED = 1;
  }
  enum Type {
    HZMS = 0; // 项目：海卓明士
    JIN_BO = 1; // 供应商：金博
    KJY = 2; // 凯嘉亿
  }
}

message PbEquipmentManipulator {
  string code = 1;
  repeated PalletSlot pallet_slot = 2;
  map<string, Command> pallet_forklift_commands = 3;
  map<string, Command> manipulator_commands = 4;

  message Command {
    string task_code = 1;
    string from = 2;
    string to = 3;
    bool is_cross_floor = 4;
    string container_code = 5;
    int64 last_send_time = 6;
    bool is_new_pallet_slot = 7;
    string work_order_code = 8;
  }

  message PalletSlot {
    string code = 1;
    Type type = 2;
    string wave_code = 3;
    repeated string order_container_code = 4;
    State state = 5;
    bool can_not_pal = 6;
  }

  enum Type {
    NOTHING = 0; // 无托盘
    EMPTY_PALLET = 1; // 空托盘
    ORDER_PALLET = 2; // 订单托盘
    EMPTY_CONTAINER_PALLET = 3; // 空箱托盘
  }

  enum State {
    NO_TASK = 0;
    HAS_MANIPULATOR_TASK = 1;
    HAS_PALLET_FORKLIFT_TASK = 2;
  }
}

message PbEquipmentConveyor {
  string code = 1;
  map<int32, Node> id_to_node = 2; // 所有节点数据
  map<string, WaterLever> code_to_water_lever = 3; // 所有水位线数据

  message Node {
    int32 id = 1;
    string container_code = 2; // 节点上的容器
    int64 moving_time = 3; // 节点上的容器移动时间
    bool allow_move = 4; // 是否允许移动
    int64 container_arrived_time = 5; // 容器到达时间
    int32 next_node_id = 6; // 流向的节点id
    int64 notified_time = 7; // 上一次通知的时间
  }

  message WaterLever {
    string name = 1;
    repeated int32 node = 2; // 水位线的所有节点id
    int32 container_count = 3; // 水位线上的容器数量
    int32 capacity = 4; // 水位线的容量
  }
}

message PbEquipmentSensor {
  string code = 1;
  Type type = 2;
  State state = 3;
  string value = 4;
  string unit = 5; // 单位
  string reservoir_area = 6; // 传感器所在库区
  int64 last_read_success_time = 7; // 最后一次读取成功的时间
  enum Type {
    NO_SUPPORT = 0;
    TEMPERATURE = 1;
    HUMIDITY = 2;
  }

  enum State {
    UNKNOWN = 0;
    NORMAL = 1;
    ABNORMAL = 2;
  }
}

message PbEquipmentSafetyVest {
  string code = 1;
  Scopes scopes = 2;
  map<int64, UwbInfo> id_to_uwb = 3;
  string zone = 4;
  int64 updated = 5;
  repeated Scopes history_scopes = 6;

  message UwbInfo {
    int64 uwb_id = 1;
    int64 sys_id = 2;
    int64 role_id = 3;
    ReportInfo report_info = 4;
    repeated Anchor anchor = 5;
    State state = 6;
    int64 updated = 7;
  }

  message ReportInfo {
    int64 battery_vol = 1;                // 电池电压  单位 mA
    int64 battery_temp = 2;               // 电池温度
    int64 battery_run_time_to_empty = 3;  // 预计电池剩余运行时间
    int64 fault_status = 4;               // 故障标志位
    int64 wk_mode = 5;                    // 工作模式
    int64 system_status = 6;              // 系统运行状态
    string pwr_ad = 7;                    // 母线电压
    int64 battery_soc = 8;                // 电池剩余电量
    int32 rbt_num = 9;                    // 覆盖范围内RTM节点数量，1个本体有两个节点，可能只搜到一个
    int64 system_time = 10;               // 系统时间
  }

  message Anchor {
    int64 uwb_id =1;
    string code = 2;
    string zone = 3;
    AnchorType anchor_type = 4;
    string point_code = 5;
    int64 dis = 6;
  }

  enum AnchorType {
    HAIPORT = 0;
    SAFETY_DOOR = 1;
    ROBOT = 2;
    PTM = 3;
    OTHER_TYPE = 4;
  }

  enum State {
    OFFLINE = 0;
    REGISTERED = 1;
  }

  message Scopes {
    repeated Scope scope = 1;
  }

  message Scope {
    ScopeType scope_type = 1;
    int64 x = 2;
    int64 y = 3;
    int64 z = 4;
    int64 r = 5;
    int64 theta1 = 6;
    int64 theta2 = 7;
    int64 sigma_width = 8;
  }

  enum ScopeType {
    ARC = 0;
    CIRCULAR = 1;
  }
}

enum PbEquipmentModelUpdatedType {
  CREATED = 0;
  UPDATED = 1;
  REMOVED = 2;
}

message PbEquipmentModelUpdated {
  message Item {
    PbEquipmentModel current = 1;
    PbEquipmentModel previous = 2;
    PbEquipmentModelUpdatedType type = 3;
    int64 update_sequence = 4; // 更新序号从1开始，且连续，监听对象更新时如果发现序号不连续，可以重新获取全部对象及当前序号
  }
  repeated Item item = 1;
}

```