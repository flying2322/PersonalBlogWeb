# 4. 任务事件回调接口

//任务的异常，失败

**回调事件编码枚举**

|  No.  |            EVENT Code                |       EVENT            |
| -- | --------------------------------------- | ----------------- |
| 4.1  | CALLBACK\_OF\_TOTE\_LOADED\_BY\_ROBOT   | 机器人取箱成功事件回调       |
| 4.2  | CALLBACK\_OF\_TOTE\_UNLOADED\_BY\_ROBOT | 机器人放箱成功事件回调       |
| 4.3  | CALLBACK\_OF\_TOTE\_LOAD\_FAILED        | 机器人取箱失败事件回调       |
| 4.4  | CALLBACK\_OF\_TOTE\_UNLOAD\_FAILED      | 机器人放箱失败事件回调       |
| 4.5  | CALLBACK\_OF\_ROBOT\_REACH\_STATION     | 机器人到达工作站事件回调      |
| 4.6  | CALLBACK\_OF\_TASK\_FINISHED            | 任务完成事件回调          |
| 4.7  | CALLBACK\_OF\_TASK\_FAILED              | 任务失败事件回调          |
| 4.8  | CALLBACK\_OF\_TASK\_CANCELLED           | 任务取消事件回调          |
| 4.9  | CALLBACK\_OF\_TASK\_SUSPENDED           | 任务挂起事件回调          |
| 4.10 | CALLBACK\_OF\_STATION\_GROUP\_CHANGED   | 缓存货架工作站组控制权切换事件回调 |
| 4.11 | CALLBACK\_OF\_LOCATION\_ABNORMAL        | 工作位异常事件回调         |
| 4.12 | CALLBACK\_OF\_ROBOT\_ABNORMAL           | 机器人异常事件回调         |
| 4.13 | CALLBACK\_OF\_TOTE\_WEIGHED\_BY\_ROBOT  | 机器人称重成功事件回调       |
| 4.14 | CALLBACK\_OF\_TOTE\_RFID\_BY\_ROBOT     | 机器人RFID识别成功事件回调   |



## 4.1 机器人取箱成功事件回调

**接口URL**：

**调用方法：** 当机器人取到对应料箱后回调对应任务信息

**回调报文：**

```json
{
    "eventCode":"CALLBACK_OF_TOTE_LOADED_BY_ROBOT",
    "taskGroupCode":"",
    "taskCode":"2:load-1705067052610360576",
    "taskStatus":"FINISHED",
    "taskTemplateCode":"",
    "actionCode":"load",
    "robotCode":"kubot-2",
    "containerCode":"bin4-05-02",
    "trayLevel":64,
    "stationCode":"2",
    "locationCode":"LT_CONVEYOR_OUTPUT:POINT:3920:5070",
    "updateTime":1626078657284
}
```

**响应报文：**
```json
{
    "code": 0, // 0 代表正常，其余代表异常
    "msg": “详情”,
    "data": {返回数据对象，如果需要}
} 
```
**回调参数说明：**

|                  |          |                |    |                                       |
| ---------------- | -------- | -------------- | -- | ------------------------------------- |
| 字段               | 名称       | 定义             | 必选 | 备注                                    |
| eventCode        | 事件编码     | string         | Y  | CALLBACK\_OF\_TOTE\_LOADED\_BY\_ROBOT |
| taskGroupCode    | 任务组号     | string         | Y  | 任务组号                                  |
| taskCode         | 任务号      | string         | Y  | 任务号                                   |
| taskStatus       | 任务状态     | string         | Y  | FINISHED 成功                           |
| taskTemplateCode | 任务模板     | string         | Y  |                                       |
| actionCode       | 任务行为编号   | string         | Y  | LOAD取箱成功                              |
| robotCode        | 机器人编码    | string         | Y  | 机器人编码                                 |
| containerCode    | 货箱编码     | string         | Y  |                                       |
| trayLevel        | 背篓层号     | int            | Y  | 从0层开始编号，64表示放在了货叉上                    |
| locationCode     | 库位编码     | string         | Y  |                                       |
| stationCode      | 工作站编码    | string         | N  |                                       |
| updateTime       | 更新时间     | long           | Y  | 时间戳，ms                                |
| rfidInfo         | rfid盘点信息 | list< string > | N  | 仅限于rfid盘点                             |
| weight           | 重量       | int            | N  | 单位g 仅限于重量盘点                           |

## 4.2 **机器人取箱失败事件回调**

**接口URL**：

**调用方法：** 当机器人取对应料箱失败后回调对应任务信息

**回调报文：**

```json
{
    "eventCode":"CALLBACK_OF_TOTE_LOAD_FAILED",
    "taskGroupCode":"",
    "taskCode":"2:load-1705067052610360576",
    "taskStatus":"FAILED",
    "taskTemplateCode":"",
    "actionCode":"load",
    "robotCode":"kubot-2",
    "containerCode":"bin4-05-02",
    "trayLevel":64,
    "stationCode":"2",
    "locationCode":"LT_CONVEYOR_OUTPUT:POINT:3920:5070",
    "updateTime":1626078657284
}
```
**响应报文：**
```json
{
    "code": 0, // 0 代表正常，其余代表异常
    "msg": “详情”,
    "data": {返回数据对象，如果需要}
 } 
```

**回调参数说明：**
|                  |        |        |    |                                  |
| ---------------- | ------ | ------ | -- | -------------------------------- |
| 字段               | 名称     | 定义     | 必选 | 备注                               |
| eventCode        | 事件编码   | string | Y  | CALLBACK\_OF\_TOTE\_LOAD\_FAILED |
| taskGroupCode    | 任务组号   | string | Y  | 任务组号                             |
| taskCode         | 任务号    | string | Y  | 任务号                              |
| taskStatus       | 任务状态   | string | Y  | FAILED 失败                        |
| taskTemplateCode | 任务模板   | string | Y  |                                  |
| actionCode       | 任务行为编号 | string | Y  | LOAD取箱失败                         |
| robotCode        | 机器人编码  | string | Y  | 机器人编码                            |
| containerCode    | 货箱编码   | string | Y  |                                  |
| locationCode     | 库位编码   | string | Y  |                                  |
| stationCode      | 工作站编码  | string | N  |                                  |
| updateTime       | 更新时间   | long   | Y  | 时间戳，ms                           |

## 4.3 机器人到达工作站回调

**调用方法：** **机器人** **到达后通知客户系统** **，仅人工工作站（直接在机器人身上操作容器）的场景使用。**

**接口URL**：

**回调报文：**
```json
{
    "eventCode":"CALLBACK_OF_ROBOT_REACH_STATION",
    "robotCode":"kubot-9",
    "robotTypeCode":"RT_KUBOT",
    "stationCode":"8",
    "locationCode":"kubot:labor:8",
    "updateTime":1623917325531,
    "trays":[
        {
            "containerCode":"",
            "trayLevel":0,
            "positionCode":"kubot-9#0",
            "containerFace": "A"
        },
        {
            "containerCode":"",
            "trayLevel":64,
            "positionCode":"kubot-9#64",
            "containerFace": "A"
        },
        {
            "containerCode":"",
            "trayLevel":1,
            "positionCode":"kubot-9#1",
            "containerFace": "A"
        }
    ]
}
```
**响应报文：**
```json
{
    "code": 0, // 0 代表正常，其余代表异常
    "msg": “详情”,
    "data": {返回数据对象，如果需要}
 } 
```


**回调参数说明：**

|               |         |              |    |                                     |
| ------------- | ------- | ------------ | -- | ----------------------------------- |
| 字段            | 名称      | 定义           | 必选 | 备注                                  |
| eventCode     | 事件编码    | string       | Y  | CALLBACK\_OF\_ROBOT\_REACH\_STATION |
| robotCode     | 机器人编码   | string       | Y  |                                     |
| robotTypeCode | 机器人类型   | string       | Y  |                                     |
| stationCode   | 工作站编码   | string       | Y  |                                     |
| locationCode  | 工作位编码   | string       | N  |                                     |
| trays         | 机器人背篓信息 | list\<tray > | Y  |                                     |
| updateTime    | 更新时间    | long         | Y  | 时间戳，ms                              |

**tray ：**

|               |      |                |    |               |
| ------------- | ---- | -------------- | -- | ------------- |
| 字段            | 名称   | 定义             | 必选 | 备注            |
| trayLevel     | 背篓序号 | int            | Y  | 货叉标识64        |
| containerCode | 货箱编码 | string         | N  | 若背篓为空则为空      |
| taskCodes     | 业务任务 | list< string > | N  |               |
| positionCode  | 位置编码 | string         | Y  | 格式为机器人id#背篓序号 |
| containerFace | 容器朝向 | string         | N  |               |

## 4.4 //~~机器人离开工作站回调~~

## 4.5 **机器人**放箱成功事件回调

**接口URL**：

**调用方法：** 当机器人投箱后回调对应任务信息

**回调报文：**
```json
{
    "eventCode":"CALLBACK_OF_TOTE_UNLOADED_BY_ROBOT",
    "taskGroupCode":"",
    "taskCode":"return:task-1704610570194392320",
    "taskStatus":"FINISHED",
    "taskTemplateCode":"",
    "actionCode":"unload",
    "robotCode":"kubot-2",
    "containerCode":"bin4-05-10",
    "stationCode":"LA_SHELF_STORAGE",
    "locationCode":"4-05-10",
    "updateTime":1625643320317
}
```

**响应报文：**
```
{
    "code": 0, // 0 代表正常，其余代表异常
    "msg": “详情”,
    "data": {返回数据对象，如果需要}
 } 
```

**回调参数说明：**

|                  |        |        |    |                                         |
| ---------------- | ------ | ------ | -- | --------------------------------------- |
| 字段               | 名称     | 定义     | 必选 | 备注                                      |
| eventCode        | 事件编码   | string | Y  | CALLBACK\_OF\_TOTE\_UNLOADED\_BY\_ROBOT |
| taskGroupCode    | 任务组号   | string | Y  | 任务组号                                    |
| taskCode         | 任务号    | string | Y  | 业务任务号                                   |
| taskStatus       | 任务状态   | string | Y  | FINISHED完成                              |
| taskTemplateCode | 任务模板   | string | N  |                                         |
| actionCode       | 任务行为编号 | string | Y  | UNLOAD放箱成功                              |
| robotCode        | 机器人编码  | string | Y  | 机器人编码                                   |
| containerCode    | 货箱编码   | string | Y  |                                         |
| stationCode      | 工作站编码  | string | N  |                                         |
| locationCode     | 工作位编码  | string | N  | 工作站：指的是输送线的哪个出入口库位：库位编码                 |
| updateTime       | 更新时间   | long   | Y  | 时间戳，ms                                  |


## 4.6 **机器人放箱失败事件回调** **Container**

**接口URL**：

**调用方法：**当机器人放对应料箱失败后回调对应任务信息

**回调报文：**
```json
{
    "eventCode":"CALLBACK_OF_TOTE_UNLOAD_FAILED",
    "taskGroupCode":"",
    "taskCode":"2:unload-1705067052610360576",
    "taskStatus":"FAILED",
    "taskTemplateCode":"",
    "actionCode":"load",
    "robotCode":"kubot-2",
    "containerCode":"bin4-05-02",
    "trayLevel":64,
    "stationCode":"2",
    "locationCode":"LT_CONVEYOR_INPUT:POINT:3920:5070",
    "updateTime":1626078657284
}
```

**响应报文：**

```json
{
    "code": 0, // 0 代表正常，其余代表异常
    "msg": “详情”,
    "data": {返回数据对象，如果需要}
} 
```

**回调参数说明：**

|                      |          |            |       |                                        |
| -------------------- | -------- | ---------- | ----- | -------------------------------------- |
| 字段                   | 名称       | 定义         | 必选    | 备注                                     |
| eventCode            | 事件编码     | string     | Y     | **CALLBACK\_OF\_TOTE\_UNLOAD\_FAILED** |
| taskGroupCode        | 任务组号     | string     | Y     | 任务组号                                   |
| taskCode             | 任务号      | string     | Y     | 任务号                                    |
| taskStatus           | 任务状态     | string     | Y     | FAILED 失败                              |
| ~~taskTemplateCode~~ | ~~任务模板~~ | ~~string~~ | ~~Y~~ |                                        |
| actionCode           | 任务行为编号   | string     | Y     | UNLOAD取箱失败                             |
| robotCode            | 机器人编码    | string     | Y     | 机器人编码                                  |
| containerCode        | 货箱编码     | string     | Y     |                                        |
| locationCode         | 库位编码     | string     | Y     |                                        |
| stationCode          | 工作站编码    | string     | N     |                                        |
| updateTime           | 更新时间     | long       | Y     | 时间戳，ms                                 |






## 4.7 **机器人称重成功事件回调**

**接口URL**：

**调用方法：**当机器人称重完对应料箱后回调对应任务信息

**回调报文：**
```json
{
    "eventCode":"CALLBACK_OF_TOTE_WEIGHED_BY_ROBOT",
    "taskGroupCode":"",
    "taskCode":"2:load-1705067052610360576",
    "taskStatus":"FINISHED",
    "taskTemplateCode":"",
    "actionCode":"weigh",
    "robotCode":"kubot-2",
    "containerCode":"bin4-05-02",
    "trayLevel":64,
    "stationCode":"",
    "locationCode":"",
    "weight": "500",
    "updateTime":1626078657284
}
```
**响应报文：**
```json
{
    "code": 0, // 0 代表正常，其余代表异常
    "msg": “详情”,
    "data": {返回数据对象，如果需要}
 } 
```

**回调参数说明：**

|                  |        |        |    |                                        |
| ---------------- | ------ | ------ | -- | -------------------------------------- |
| 字段               | 名称     | 定义     | 必选 | 备注                                     |
| eventCode        | 事件编码   | string | Y  | CALLBACK\_OF\_TOTE\_WEIGHED\_BY\_ROBOT |
| taskGroupCode    | 任务组号   | string | Y  | 任务组号                                   |
| taskCode         | 任务号    | string | Y  | 任务号                                    |
| taskStatus       | 任务状态   | string | Y  | FINISHED 成功                            |
| taskTemplateCode | 任务模板   | string | Y  |                                        |
| actionCode       | 任务行为编号 | string | Y  | LOAD取箱成功                               |
| robotCode        | 机器人编码  | string | Y  | 机器人编码                                  |
| containerCode    | 货箱编码   | string | Y  | 实际扫描到的箱码                               |
| trayLevel        | 背篓层号   | int    | Y  | 从0层开始编号，64表示放在了货叉上                     |
| locationCode     | 库位编码   | string | N  |                                        |
| stationCode      | 工作站编码  | string | N  |                                        |
| updateTime       | 更新时间   | long   | Y  | 时间戳，ms                                 |
| weight           | 重量     | int    | N  | 单位g 仅限于重量盘点                            |


## 4.8 **机器人RFID识别成功事件回调**

**接口URL**：

**调用方法：**当机器人识别对应料箱RFID后回调对应任务信息

**回调报文：**
```json
{
    "eventCode":"CALLBACK_OF_TOTE_RFID_BY_ROBOT",
    "taskGroupCode":"",
    "taskCode":"2:load-1705067052610360576",
    "taskStatus":"FINISHED",
    "taskTemplateCode":"",
    "actionCode":"rfid",
    "robotCode":"kubot-2",
    "containerCode":"bin4-05-02",
    "trayLevel":64,
    "stationCode":"2",
    "locationCode":"LT_CONVEYOR_OUTPUT:POINT:3920:5070",
    "rfid": ["663164","303169"],
    "updateTime":1626078657284
}
```

## 4.9 任务分配回调

**调用方法：**

- 系统任务(由业务任务生成的系统任务)分配到机器人后进行回调

**接口URL**：

**回调报文：**

```json
{
    "eventCode":"CALLBACK_OF_TASK_ALLOCATED",
    "taskGroupCode":"shareinside210707",
    "taskCode":"2021-07-07 15:35:11:491shareinput",
    "taskStatus":"PROCESSING",
    "taskTemplateCode":"TOTE_INBOUND",
    "robotCode":"kubot-2",
    "containerCode":"bin4-05-10",
    "locationCode":"4-05-10",
    "updateTime":1625643320317
}
```
**响应报文：**

```json
{
    "code": 0, // 0 代表正常，其余代表异常
    "msg": “详情”,
    "data": {返回数据对象，如果需要}
 } 

```

**回调参数说明：**




## 4.10 任务完成回调

**调用方法：**

- 对于出库

  - 输送线工作站：是货箱卸载后任务完成
  - 人工工作站：到达工作站后任务完成

- 对于入库，是货箱卸载到库位后完成

**接口URL**：

**回调报文：**

```json
{
    "eventCode":"CALLBACK_OF_TASK_FINISHED",
    "taskGroupCode":"shareinside210707",
    "taskCode":"2021-07-07 15:35:11:491shareinput",
    "taskStatus":"FINISHED",
    "taskTemplateCode":"TOTE_INBOUND",
    "robotCode":"kubot-2",
    "containerCode":"bin4-05-10",
    "stationCode":"LA_SHELF_STORAGE",
    "locationCode":"4-05-10",
    "updateTime":1625643320317 ,
    "isLocationHasContainer":false
}
```

**响应报文：**
```json
{
    "code": 0, // 0 代表正常，其余代表异常
    "msg": “详情”,
    "data": {返回数据对象，如果需要}
} 
```

**回调参数说明：**
|                        |         |         |    |                              |
| ---------------------- | ------- | ------- | -- | ---------------------------- |
| 字段                     | 名称      | 定义      | 必选 | 备注                           |
| eventCode              | 事件编码    | string  | Y  | CALLBACK\_OF\_TASK\_FINISHED |
| taskGroupCode          | 任务组号    | string  | Y  | 任务组号                         |
| taskCode               | 任务号     | string  | Y  | 业务任务号                        |
| taskStatus             | 任务状态    | string  | Y  | FINISHED执行中                  |
| taskTemplateCode       | 任务模板    | string  | N  |                              |
| robotCode              | 机器人编码   | string  | N  |                              |
| containerCode          | 货箱编码    | string  | N  |                              |
| stationCode            | 工作站编码   | string  | N  |                              |
| locationCode           | 工作位编码   | string  | N  |                              |
| updateTime             | 更新时间    | long    | Y  | 时间戳，ms                       |
| isLocationHasContainer | 库位是否有容器 | boolean | N  | 库位是否有容器的盘点任务才有返回值            |


## 4.11 任务失败回调

**调用方法：**

- 当任务状态置为FAILED 时进行回调

**接口URL**：

**回调报文：**
```json 
{
    "eventCode":"CALLBACK_OF_TASK_FAILED",
    "taskGroupCode":"TA_2021-07-10-17-12_00032",
    "taskCode":"A442126548654326",
    "taskStatus":"FAILED",
    "failureReason":"NO_AVAILABLE_ROBOT",
    "taskTemplateCode":"TOTE_CONVEYOR_STATION_OUTBOUND",
    "robotCode":"",
    "containerCode":"A1000016258274560",
    "locationCode":"CH08-25-03",
    "updateTime":1625908425065
}
```

**响应报文：**
```json
{
    "code": 0, // 0 代表正常，其余代表异常
    "msg": “详情”,
    "data": {返回数据对象，如果需要}
 }
```

**回调参数说明：**
|                  |       |        |    |                                                                                                                  |
| ---------------- | ----- | ------ | -- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 字段               | 名称    | 定义     | 必选 | 备注                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        |
| eventCode        | 事件编码  | string | Y  | CALLBACK\_OF\_TASK\_FAILED                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                |
| taskGroupCode    | 任务组号  | string | Y  | 任务组号                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      |
| taskCode         | 任务号   | string | Y  | 业务任务号                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     |
| taskStatus       | 任务状态  | string | Y  | FAILED任务失败                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                |
| taskTemplateCode | 任务模板  | string | N  |                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           |
| failureReason    | 失败原因  | string | N  | NO\_DESTINATION\_CODE = 1; // 无目的地NO\_CONTAINER\_CODE = 2; // 没有容器UNKNOWN\_DESTINATION\_CODE = 3; // 目的地不存在NO\_AVAILABLE\_ROBOT = 4; // 没有可用的机器人 NO\_SPECIFIED\_ROBOT = 5; // 未指定机器人 CONTAINER\_IS\_NOT\_ON\_ROBOT = 6; // 容器不在机器人身上UNKNOWN\_TASK = 7; // 任务无法识别 STATION\_IS\_NOT\_EXIST = 8; // 工作站不存在 TRY\_ACTION\_LOCATION\_NOT\_FOUND = 21; // location不存在 TRY\_ACTION\_LOCATION\_NOT\_SUPPORTED\_ACTION = 22; // action不支持 TRY\_ACTION\_LOCATION\_ACTION\_NOT\_IMPLEMENT = 23; // action功能未实现 TRY\_ACTION\_LOCATION\_NOT\_LOADING\_CONTAINER = 24; // location上没有容器 TRY\_ACTION\_LOCATION\_CONTAINER\_NOT\_SAME\_WITH\_TASK = 25; // 容器与任务不一样 TRY\_ACTION\_LOCATION\_ALREADY\_LOADING\_CONTAINER = 26; // location上已经有容器 END\_ACTION\_FAILED = 31; // 行为失败*TASK\_TEMPLATE\_NOT\_EXIST：任务模板不存在**ACTION\_PARAM\_ARRAY\_SIZE\_NOT\_MATCH：行为参数数组大小不匹配**NO\_SPECIFIED\_CONTAINER：未指定容器**TASK\_ALREADY\_EXIST：任务已经存在**CONTAINER\_PUTAWAY\_PROCESSING：容器上架处理中**CODE\_ALREADY\_EXIST：编码已经存在* |
| robotCode        | 机器人编码 | string | N  |                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           |
| containerCode    | 货箱编码  | string | N  |                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           |
| locationCode     | 工作位编码 | string | N  |                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           |
| updateTime       | 更新时间  | long   | Y  | 时间戳，ms                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    |

**失败原因**

|                                                                 |                      |    |
| --------------------------------------------------------------- | -------------------- | -- |
|                                                                 | 名称                   | 备注 |
| *TASK\_TEMPLATE\_NOT\_EXIST*                                    | 任务模板不存在              |    |
| *ACTION\_PARAM\_ARRAY\_SIZE\_NOT\_MATCH*                        | 行为参数数组大小不匹配          |    |
| *UNKNOWN\_DESTINATION\_CODE*                                    | 目的地不存在               |    |
| *NO\_SPECIFIED\_ROBOT*                                          | 未指定机器人               |    |
| *NO\_SPECIFIED\_CONTAINER*                                      | 未指定容器                |    |
| *NO\_AVAILABLE\_ROBOT*                                          | 没有可用的机器人             |    |
| *TASK\_ALREADY\_EXIST*                                          | 任务已经存在               |    |
| *CONTAINER\_PUTAWAY\_PROCESSING*                                | 容器上架处理中              |    |
| *CODE\_ALREADY\_EXIST*                                          | 编码已经存在               |    |
| *NO\_DESTINATION\_CODE*                                         | 无目的地                 |    |
| *NO\_CONTAINER\_CODE*                                           | 没有容器、                |    |
| *UNKNOWN\_DESTINATION\_CODE*                                    | 目的地不存在               |    |
| *NO\_AVAILABLE\_ROBOT*                                          | 没有可用的机器人             |    |
| *NO\_SPECIFIED\_ROBOT*                                          | 未指定机器人               |    |
| *CONTAINER\_IS\_NOT\_ON\_ROBOT*                                 | 容器不在机器人身上            |    |
| *UNKNOWN\_TASK*                                                 | 任务无法识别               |    |
| *STATION\_IS\_NOT\_EXIST*                                       | *工作站不存在*             |    |
| *CONTAINER\_WITHOUT\_LOCATION*                                  | 容器没在库位上              |    |
| *ROBOT\_CANNOT\_CONTINUE\_EXECUTION*                            | 容器任务没法继续执行           |    |
| *RELATED\_SHALLOW\_LOCATION\_TASK\_FAIL*                        | 关联的浅库位任务失败导致该深库位任务失败 |    |
| *RELATED\_FRONT\_LOCATION\_LOAD\_TASK\_FAIL*                    | 关联的前置取箱任务失败          |    |
| *TRY\_ACTION\_LOCATION\_NOT\_FOUND*                             | location不存在          |    |
| *TRY\_ACTION\_LOCATION\_NOT\_SUPPORTED\_ACTION*                 | action不支持            |    |
| *TRY\_ACTION\_LOCATION\_ACTION\_NOT\_IMPLEMENT*                 | action功能未实现          |    |
| *TRY\_ACTION\_LOCATION\_NOT\_LOADING\_CONTAINER*                | location上没有容器        |    |
| *TRY\_ACTION\_LOCATION\_CONTAINER\_NOT\_SAME\_WITH\_TASK*       | 容器与任务不一样             |    |
| *TRY\_ACTION\_LOCATION\_ALREADY\_LOADING\_CONTAINER*            | location上已经有容器       |    |
| *TRY\_ACTION\_ROBOT\_NOT\_HAVE\_ENOUGH\_AVAILABLE\_TRAY\_COUNT* | 机器人无足够可用数量的背篓和货叉     |    |
| *END\_ACTION\_FAILED*                                           | 行为失败                 |    |


## 4.12 任务取消回调

**调用方法：**

- 当任务状态置为CANCELLED时进行回调

**接口URL**：

**回调报文：**
```json
{
    "eventCode":"CALLBACK_OF_TASK_CANCELLED",
    "taskGroupCode":"shareoutside210712",
    "taskCode":"2021-07-12 16:53:50:218shareoutput",
    "taskStatus":"CANCELLED",
    "taskTemplateCode":"TOTE_CONVEYOR_STATION_OUTBOUND",
    "updateTime":1626080144764,
    "robotCode":"",
    "containerCode":"bin4-05-09",
    "locationCode":""
}
```

```json
{
    "eventCo",
    "taskGroupCode":"InsideESSP",
    "taskCode":"Input91632988404",
    "taskStatus":"CANCELLED",
    "taskTemplateCode":"TOTE_INBOUND",
    "updateTime":1632988411468
}

```

**响应报文：**
```json
{
    "code": 0, // 0 代表正常，其余代表异常
    "msg": “详情”,
    "data": {返回数据对象，如果需要}
} 

```

|                  |       |        |    |                               |
| ---------------- | ----- | ------ | -- | ----------------------------- |
| 字段               | 名称    | 定义     | 必选 | 备注                            |
| eventCode        | 事件编码  | string | Y  | CALLBACK\_OF\_TASK\_CANCELLED |
| taskGroupCode    | 任务组号  | string | Y  | 任务组号                          |
| taskCode         | 任务号   | string | Y  | 业务任务号                         |
| taskStatus       | 任务状态  | string | Y  | CANCELLED                     |
| taskTemplateCode | 任务模板  | string | N  |                               |
| robotCode        | 机器人编码 | string | N  |                               |
| containerCode    | 货箱编码  | string | N  |                               |
| locationCode     | 工作位编码 | string | N  |                               |
| stationCode      | 工作站编码 | string | N  |                               |
| updateTime       | 更新时间  | long   | Y  | 时间戳，ms                        |


## 4.13 任务挂起回调

**调用方法：**

- 当任务状态置为*SUSPEND*时进行回调

**接口URL**：

**回调报文：**
```json
{
    "eventCode":"CALLBACK_OF_TASK_SUSPENDED",
    "taskGroupCode":"shareinside210707",
    "taskCode":"2021-07-07 15:35:11:491shareinput",
    "sysTaskCode":"container:task-1741819833419306240",
    "taskStatus":"SUSPENDED",
    "taskTemplateCode":"TOTE_INBOUND",
    "robotCode":"kubot-2",
    "containerCode":"bin4-05-10",
    "stationCode":"LA_SHELF_STORAGE",
    "locationCode":"4-05-10",
    "message":"box tag is not detected at the specified location!",
    "updateTime":1625643320317
}
```
**响应报文：**
```json
{
    "code": 0, // 0 代表正常，其余代表异常
    "msg": “详情”,
    "data": {返回数据对象，如果需要}
} 
```

**回调参数说明：**
|                  |       |        |    |                               |
| ---------------- | ----- | ------ | -- | ----------------------------- |
| 字段               | 名称    | 定义     | 必选 | 备注                            |
| eventCode        | 事件编码  | string | Y  | CALLBACK\_OF\_TASK\_SUSPENDED |
| taskGroupCode    | 任务组号  | string | Y  | 任务组号，货架取箱场景下才有                |
| taskCode         | 任务号   | string | Y  | 业务任务号，货架取箱场景下才有               |
| sysTaskCode      | 系统任务号 | string | Y  | 系统任务号                         |
| taskStatus       | 任务状态  | string | Y  | FINISHED执行中                   |
| taskTemplateCode | 任务模板  | string | N  |                               |
| robotCode        | 机器人编码 | string | N  |                               |
| containerCode    | 货箱编码  | string | N  |                               |
| stationCode      | 工作站编码 | string | N  |                               |
| locationCode     | 工作位编码 | string | N  |                               |
| message          | 挂起原因  | string | N  |                               |
| updateTime       | 更新时间  | long   | Y  | 时间戳，ms                        |



## 4.14 //缓存货架工作站组控制权切换回调

**调用方法：**

当缓存货架工作站的组的控制权发生变化时由ESS系统发起回调。

**接口URL**：

**回调报文：**
```json
{
    "eventCode":"CALLBACK_OF_STATION_GROUP_CHANGED",
    "stationCode":"10",
    "groupCode":"group1",
    "lockStatus":"WMS",
    "updateTime":1626328637734
}
```

**响应报文：**
```json
{
    "code": 0, // 0 代表正常，其余代表异常
    "msg": “详情”,
    "data": {返回数据对象，如果需要}
} 
```

**回调参数说明：**
|             |       |        |    |                                       |
| ----------- | ----- | ------ | -- | ------------------------------------- |
| 字段          | 名称    | 定义     | 必选 | 备注                                    |
| eventCode   | 事件编码  | string | Y  | CALLBACK\_OF\_STATION\_GROUP\_CHANGED |
| stationCode | 工作站编码 | string | Y  |                                       |
| groupCode   | 组编码   | string | Y  |                                       |
| lockStatus  | 锁定状态  | string | Y  | WMS。当ESS不再锁定时，即本缓存货架放货完成后，会发起此事件。     |
| updateTime  | 更新时间  | long   | Y  | 时间戳ms                                 |

## 4.15 工作位异常回调

**调用方法：**

当工作站异常时由ESS系统发起回调。

**接口URL**：

**回调报文：**
```json
{
    "eventCode":"CALLBACK_OF_LOCATION_ABNORMAL",
    "stationCode":"10",
    "locationCode":"L010100037.02",
    "containerCode":"bin4-05-10",
    "message":"TRY_ACTION_LOCATION_NOT_LOADING_CONTAINER",
    "updateTime":1626328637734
}
```
**响应报文：**
```json
{
    "code": 0, // 0 代表正常，其余代表异常
    "msg": “详情”,
    "data": {返回数据对象，如果需要}
} 
```

**回调参数说明：**
|               |         |        |    |                                  |
| ------------- | ------- | ------ | -- | -------------------------------- |
| 字段            | 名称      | 定义     | 必选 | 备注                               |
| eventCode     | 事件编码    | string | Y  | CALLBACK\_OF\_LOCATION\_ABNORMAL |
| stationCode   | 工作站编码   | string | Y  |                                  |
| locationCode  | 工作位编码   | string | Y  |                                  |
| containerCode | 容器编码    | string | Y  |                                  |
| message       | 工作位异常信息 | string | N  | 见 AbnormalReason                 |
| updateTime    | 更新时间    | long   | Y  | 时间戳ms                            |

**AbnormalReason**
|                                               |            |    |
| --------------------------------------------- | ---------- | -- |
| 字段                                            | 名称         | 备注 |
| *CONTAINER\_HAS\_NOT\_PUTAWAY\_TASK*          | 容器上没有上架任务  |    |
| *LOAD\_FAILED\_COUNT\_EXCEEDED\_THE\_LIMIT*   | 取箱失败次数达到上限 |    |
| *UNLOAD\_FAILED\_COUNT\_EXCEEDED\_THE\_LIMIT* | 放箱失败次数达到上限 |    |
| *CONTAINER\_LOCATION\_UNMATCHED*              | 容器位置不匹配    |    |
| *CONTAINER\_IS\_NOT\_EXIST\_AFTER\_SCAN*      | 容器不存在      |    |
| *SCAN\_FAILED*                                | 扫码失败       |    |
| *EQUIPMENT\_ABNORMAL*                         | 设备异常       |    |
| *NOT\_ALLOWED\_LOAD*                          | 不允许取箱      |    |
| *NOT\_ALLOWED\_UNLOAD*                        | 不允许放箱      |    |


## 4.16 机器人异常回调

**调用方法：**

当机器人异常时由ESS系统发起回调。

**接口URL**：

**回调报文：**

```JSON
{

    "eventCode":"CALLBACK_OF_ROBOT_ABNORMAL",

    "robotCode":"kubot-2",

    "errorCode":"",

    "message":"box tag is not detected at the specified location!",

    "updateTime":1626328637734

}
```

**响应报文：**

```JSON
{

    "code": 0, // 0 代表正常，其余代表异常

    "msg": “详情”,

    "data": {返回数据对象，如果需要}

 } 
```

**回调参数说明：**

|            |         |        |    |                               |
| ---------- | ------- | ------ | -- | ----------------------------- |
| 字段         | 名称      | 定义     | 必选 | 备注                            |
| eventCode  | 事件编码    | string | Y  | CALLBACK\_OF\_ROBOT\_ABNORMAL |
| robotCode  | 机器人编码   | string | Y  |                               |
| errorCode  | 异常code  | string | N  |                               |
| message    | 机器人异常信息 | string | Y  |                               |
| updateTime | 更新时间    | long   | Y  | 时间戳ms                         |

## 4.17 任务重传

**调用方法：**

重试回传任务

**接口URL**：/ess-api/callback/resend

**回调报文：**
```json
{
    "callbackIds":[63525288737,87373542325]
}
```
**响应报文：**

```json
{
    "code": 0, // 0 代表正常，其余代表异常
    "msg": “详情”,
    "data": {返回数据对象，如果需要}
} 
```

## 4.18 申请库位
**调用方法：**

容器入库时向上游申请库位分配

**接口URL**：

配置在callback 中的 customerAssignLocationUrl

**请求报文：**

```json
{
    "mapNo": "1",
    "robotCode": "robot-1",
    "robotTypeCode": "RT_KUBOT",
    "containerTaskDtoList": [{
        "containerNo": "container1",
        "taskNo": "task1"
    }]
}
```
**响应报文：**
```json
{
    "code": 1, // 1 代表正常，其余代表异常
    "msg": “详情”,
    "data": {返回数据对象，如果需要}
 }
```
**回调参数说明：**

|                      |           |                              |    |                         |
| -------------------- | --------- | ---------------------------- | -- | ----------------------- |
| 字段                   | 名称        | 定义                           | 必选 | 备注                      |
| mapNo                |           | string                       | Y  |                         |
| robotCode            | 机器人编码     | string                       | Y  |                         |
| robotTypeCode        | 机器人类型编码   | string                       | Y  | 根据机器人类型填写，默认值为RT\_KUBOT |
| containerTaskDtoList | 需要申请库位的容器 | list< containerTaskDtoList > | Y  |                         |

**containerTaskDtoList:**
|             |           |        |    |                   |
| ----------- | --------- | ------ | -- | ----------------- |
| 字段          | 名称        | 定义     | 必选 | 备注                |
| containerNo | 容器编码      | string | Y  |                   |
| taskNo      | 申请库位的任务编码 | string | Y  | 适用于下发入库任务时没有指定目的地 |




# 5. 资源更新请求wms

本部分描述资源的更新相关接口。

## 5.1 容器相关

本部分描述容器相关的更新内容。

### 5.1.1 容器创建

**接口URL**：POST /ess-api/container/add

**调用方法：** 客户需要在ESS系统内使用一个容器时，需要通过该接口首先将容器创建出来。创建出来后容器在系统中为库外

// 0715 考虑到更多项目场景使用，container/add 接口改成容器在任意位置都可以修改容器类型和容器标识

**请求报文：**
```json
{
  "containerAdds": [
    {
      "containerCode": "container001",
      "containerTypeCode": "CT_KUBOT_STANDARD",
      "storageTag": ""
    }
  ]
}
```

**响应报文：**
```json
{
    "code": 0, // 0 代表正常，其余代表异常
    "msg": “详情”,
    "data": {
       "container001":"OK"
    }
} 
```

**请求参数说明：**

|               |      |                     |    |    |
| ------------- | ---- | ------------------- | -- | -- |
| 字段            | 名称   | 定义                  | 必选 | 备注 |
| containerAdds | 容器操作 | list\<containerAdd> | Y  |    |

**containerAdd：**
|                   |       |        |    |                    |
| ----------------- | ----- | ------ | -- | ------------------ |
| 字段                | 名称    | 定义     | 必选 | 备注                 |
| containerCode     | 容器编码  | string | Y  | 创建容器时容器编码需要在系统内不存在 |
| containerTypeCode | 容器型号  | string | Y  | 必须指定一个型号才能进行创建     |
| storageTag        | 工作位标签 | string | N  | 用于逻辑分区分配对应工作位      |


**请求参数说明：**
|      |            |                     |    |                   |
| ---- | ---------- | ------------------- | -- | ----------------- |
| 字段   | 名称         | 定义                  | 必选 | 备注                |
| data | 容器创建请求响应结果 | map\<string,string> | Y  | Key 容器编码Value 状态码 |

Value enum

|                             |          |                             |
| --------------------------- | -------- | --------------------------- |
| 字段                          | 名称       | 备注                          |
| OK                          | 成功       |                             |
| CONTAINER\_TYPE\_NOT\_FOUND | 容器类型不存在  |                             |
| CODE\_ALREADY\_USED         | 容器编码已经使用 | 例： container001 已经被创建成为库位编码 |



### 5.1.2 容器入场
**调用方法：** 容器由离场转为置于一个机器人身上或一个工作位（库位，工作站工作位等）上

- 若在输送线的入库口更新一个容器，则会触发机器人取箱行为，此时指的是将容器放在了工作站的一个工作位上，当机器人取到后会转为入场状态
- 若容器和位置相同，多次请求，则系统认为是人工在重置，系统同样会成功

**接口URL**：POST /ess-api/container/moveIn

**请求报文：**

```json
{
  "containerMoveIns": [
    {
      "containerCode": "container001",
      "positionCode": "single-IO-conveyor1-output1"
    }
  ]
}
```



**响应报文：**

```json
{
    "code": 0, // 0 代表正常，其余代表异常
    "msg": “详情”,
    "data": {返回数据对象，如果需要}
} 
```



**请求参数说明：**
|                  |      |                        |    |    |
| ---------------- | ---- | ---------------------- | -- | -- |
| 字段               | 名称   | 定义                     | 必选 | 备注 |
| containerMoveIns | 容器入场 | list< containerMoveIn> | Y  |    |

**Container movein**
|               |      |        |    |                                                                                        |
| ------------- | ---- | ------ | -- | -------------------------------------------------------------------------------------- |
| 字段            | 名称   | 定义     | 必选 | 备注                                                                                     |
| containerCode | 容器编码 | string | N  | 若不填写，则系统认为是一个UNKNOWN容器占用了该位置，在客户不扫描容器进行上架时使用人工工作站操作机器人上的容器：必填缓存货架工作站操作容器：非必填输送线工作站：非必填 |
| positionCode  | 位置编码 | string | Y  | 位置编码可以为库位编码机器人背篓：机器人id#背篓序号                                                            |

### 5.1.3 容器离场

**调用方法：** 该操作会将容器从系统中离场，系统将不能再操作该容器。

- 若在一个工作站的出库口上移除一个容器，则会触发其控制的机器人继续放箱行为（可以空调用触发放箱），此时容器是指离开了该工作站工作位，但并没有离场，再次调用则会将容器离场
- 若容器在库位或机器人背篓中，直接调用该接口，容器会直接离场

**URL**：POST /ess-api/container/moveOut

**请求报文：**
```json
{
  "containerMoveOuts": [
    {
      "containerCode": "container001",
      "positionCode": "single-IO-conveyor1-input1"
    }
  ]
}
```


**响应报文：**

```json
{
    "code": 0, // 0 代表正常，其余代表异常
    "msg": “详情”,
    "data": {返回数据对象，如果需要}
} 
```



**请求参数说明：**
|                   |      |                         |    |    |
| ----------------- | ---- | ----------------------- | -- | -- |
| 字段                | 名称   | 定义                      | 必选 | 备注 |
| containerMoveOuts | 容器离场 | list< containerMoveOut> | Y  |    |

**containerMoveOut：**

|               |      |        |    |                                                |
| ------------- | ---- | ------ | -- | ---------------------------------------------- |
| 字段            | 名称   | 定义     | 必选 | 备注                                             |
| containerCode | 容器编码 | string | N  | 当离场UNKNOWN容器时不需要填写，离场正常容器时必填，用户客户移除一个未扫描上架的容器  |
| positionCode  | 位置编码 | string | N  | 仅当离场一个UNKNOWN容器时使用位置编码可以为工作位编码机器人背篓：机器人id#背篓序号 |


### 5.1.4 容器移除

**调用方法：**上游需要将容器移除出系统时使用

限制条件：

- 容器入场状态必须为离场状态
- 容器没有关联的任务（包含系统未清除的已完成，失败，取消的任务）

**接口URL**：POST /ess-api/container/remove

**请求报文：**
```json
{
  "containerCodes": [
    "container001"
  ]
}
```

**响应报文：**

```json
{
    "code": 0, // 0 代表正常，其余代表异常
    "msg": “详情”,
    "data": {返回数据对象，如果需要}
} 
```
**请求参数说明：**
|                |      |               |    |    |
| -------------- | ---- | ------------- | -- | -- |
| 字段             | 名称   | 定义            | 必选 | 备注 |
| containerCodes | 容器编码 | list< string> | Y  |    |


### 5.1.5 容器更新位置至库位//todo

离场转为置于一个机器

## 5.2 工作站相关

### 5.2.1 呼叫机器人来工作位请求（非任务模式）

**接口URL**：POST /ess-api/station/callRobot

**调用方法：**呼叫机器人来工作站，适用不同的工作站类型

**请求报文：**
 
 ```json
{
  "stationCode": "1",
  "locationCode":"single-IO-conveyor1-output1"
  
}
```
**响应报文：**

```json
{
    "code": 0, // 0 代表正常，其余代表异常
    "msg": “详情”,
    "data": {返回数据对象，如果需要}
 } 
```
**请求参数说明：**

|              |      |        |    |                       |
| ------------ | ---- | ------ | -- | --------------------- |
| 字段           | 名称   | 定义     | 必选 | 备注                    |
| stationCode  | 工作站号 | string | N  | 呼叫机器人来工作站             |
| locationCode | 工作位号 | string | N  | 呼叫机器人来工作位，若都填写，只识别工作位 |

// Todo 指定机器人，指定个数

### 5.2.2 缓存货架释放


**调用方法：**

当上游系统对某组货架操作结束后，使用本接口归还控制权，ESS将根据组关联库位上的容器生成回库任务。

**接口URL**：POST /ess-api/station/changeGroupLock

**回调报文：**

```json
{
  "stationCode": "1",
  "groupCode": "group1",
  "lockStatus": "ESS"
}
```
**响应报文：**
```json
{
    "code": 0, // 0 代表正常，其余代表异常
    "msg": “详情”,
    "data": {返回数据对象，如果需要}
 } 
```


**回调参数说明：**

|             |       |        |    |                       |
| ----------- | ----- | ------ | -- | --------------------- |
| 字段          | 名称    | 定义     | 必选 | 备注                    |
| stationCode | 工作站编码 | string | Y  |                       |
| groupCode   | 组编码   | string | Y  |                       |
| lockStatus  | 锁定状态  | string | Y  | ESS。必须填写ESS才能表示将控制权归还 |


### 5.2.3 命令机器人离开工作位请求

**场景：**当需要让机器人从某个位置离开时使用

**接口URL**：POST /ess-api/station/letRobotGo

**请求报文：**
```json

{
  "stationCode": "1",
  "locationCode":"LT_KIVA_LABOR:POINT:32255:6936-A",
  "robotCode":"kubot-1"
}
```
**响应报文：**
```json
{
    "code": 0, // 0 代表正常，其余代表异常
    "msg": “详情”,
    "data": {返回数据对象，如果需要}
 } 

```

**qing'qiu参数说明：**
|              |       |        |    |                                |
| ------------ | ----- | ------ | -- | ------------------------------ |
| 字段           | 名称    | 定义     | 必选 | 备注                             |
| stationCode  | 工作站号  | string | N  | 会让本工作站的机器人离开                   |
| locationCode | 工作位号  | string | N  | 让工作位的机器人离开，若都填写，只识别工作位         |
| robotCode    | 机器人编码 | string | N  | 三选一填写，没有组合逻辑。 locationCode最优先。 |


### 5.2.4 工作站枚举表

|                                    |         |           |
| ---------------------------------- | ------- | --------- |
|                                    | 名称      | 备注        |
| *KUBOT\_STORAGE\_SHELF*            | 货架      |           |
| *KUBOT\_CHARGE\_OR\_REST*          | 充电/休息   |           |
| *KUBOT\_CONVEYOR\_MULTIPLE\_IO*    | 多节点输送线  | 目前只用作双入双出 |
| *KUBOT\_CONVEYOR\_SINGLE\_IO*      | 单入单出输送线 |           |
| *KUBOT\_CONVEYOR\_SHARE\_POSITION* | 共享输送线   |           |
| *KUBOT\_LABOR*                     | 人工      |           |
| *KUBOT\_CACHE\_SHELF*              | 缓存货架    |           |
| *KUBOT\_HAIPORT*                   | haiport |           |
| *KUBOT\_MAINTENANCE*               | 维修区     |           |
| *STATION\_NOT\_SET*                | 未设置     |           |


## 5.3 工作位相关

### 5.3.1 锁定工作位

**调用方法：**

- 当系统需要在后续的任务中不使用某些工作位，如在系统自动分配库位的回库任务时不再使用这些库位时，调用该接口。(目前支持锁定库位和充电位)

**接口URL**：POST /ess-api/location/lock

**回调报文：**

```json
{
  "locationCodes": [
    "4-05-10"
  ]
}

```
**响应报文：**
```json
{
    "code": 0, // 0 代表正常，其余代表异常
    "msg": “详情”,
    "data": {返回数据对象，如果需要}
 } 

```

**请求参数说明：**
|               |       |               |    |         |
| ------------- | ----- | ------------- | -- | ------- |
| 字段            | 名称    | 定义            | 必选 | 备注      |
| locationCodes | 工作位编码 | list\<string> | Y  | 未锁定的工作位 |

### 5.3.2 解锁工作位

**调用方法：**

- 当系统需要解锁已经锁定的工作位时，调用该接口。

接口URL：POST /ess-api/location/unlock

**回调报文：**

```json
{
  "locationCodes": [
    "4-05-10"
  ]
}

```
**响应报文：**
```json
{
    "code": 0, // 0 代表正常，其余代表异常
    "msg": “详情”,
    "data": {返回数据对象，如果需要}
 }

```

**请求参数说明：**


|               |       |               |    |         |
| ------------- | ----- | ------------- | -- | ------- |
| 字段            | 名称    | 定义            | 必选 | 备注      |
| locationCodes | 工作位编码 | list\<string> | Y  | 已锁定的工作位 |




### 5.3.3 工作位逻辑分区配置

**调用方法：**

- 设置工作位逻辑分区，可用于分配工作位给对应标签的容器

**接口URL**：POST /ess-api/location/setStorageTag

**回调报文：**

```json
{
    "storageTag": "",
    "locationCodes": "HAI-001-001-001,HAI-001-001-002"
}
```

**响应报文**
```json
{
    "code": 0, // 0 代表正常，其余代表异常
    "msg": “详情”,
    "data": {返回数据对象，如果需要}
}
```


### 5.3.4 工作位异常转正常接口

**调用方法：**

- 恢复系统异常的工作位

**接口URL**：POST /location/returnNormal

**回调报文：**
```json
{
  "locationCodes": [
    "4-05-10"
  ]
}
```


**响应报文：**
```json
{
    "code": 0, // 0 代表正常，其余代表异常
    "msg": “详情”,
    "data": {返回数据对象，如果需要}
}
```


**请求参数说明：**
|               |       |               |    |            |
| ------------- | ----- | ------------- | -- | ---------- |
| 字段            | 名称    | 定义            | 必选 | 备注         |
| locationCodes | 工作位编码 | list\<string> | Y  | 工作位必须是锁定状态 |



## 5.4 机器人相关

### 5.4.1 机器人软件暂停

**调用方法：**

- 软件暂停机器人

**接口URL**：POST /ess-api/robot/pauseRobot

**请求报文：**



```json
{
  "robotCode": "kubot-1,kubot-2"
}

```
**响应报文：**
```json
{
    "code": 0, // 0 代表正常，其余代表异常
    "msg": “详情”,
    "data": {返回数据对象，如果需要}
 } 

```

**回调参数说明：**




### 5.4.2 机器人软件恢复

**调用方法：**

- 软件恢复机器人

**接口URL**：POST /ess-api/robot/resumeRobot

**请求报文：**

```json
{
  "robotCode": "kubot-1,kubot-2" 
}

```
**响应报文：**
```json
{
    "code": 0, // 0 代表正常，其余代表异常
    "msg": “详情”,
    "data": {返回数据对象，如果需要}
}
```

**请求参数说明：**



### 5.4.3 机器人硬件重启

**调用方法：**

- 硬件重启机器人，等同于远程重启本体服务器

**接口URL**：POST /ess-api/robot/restartRobot

**请求报文：**


```json
{
  "robotCodes": [
    "kubot-1",
    "kubot-2"
  ]
}

```
**响应报文：**
```json
{
    "code": 0, // 0 代表正常，其余代表异常
    "msg": “详情”,
    "data": {返回数据对象，如果需要}
 } 

```

**请求参数说明：**



### 5.4.4 机器人硬件恢复

**调用方法：**

- 硬件恢复机器人，等同于远程本体钥匙打恢复自动

**接口URL**：POST /ess-api/robot/recoveryRobot

**请求报文：**



```json
{
  "robotCodes": [
    "kubot-1",
    "kubot-2"
  ]
}

```
**响应报文：**
```json
{
    "code": 0, // 0 代表正常，其余代表异常
    "msg": “详情”,
    "data": {返回数据对象，如果需要}
 } 

```

**请求参数说明：**


## 5.5 系统相关

### 5.5.1 系统暂停

**调用方法：**

- 系统暂停

**接口URL**：POST /ess-api/system/pause

**请求报文：**

```json
{

  "zone": "" //库区

}
```

**响应报文：**

```json
{

    "code": 0, // 0 代表正常，其余代表异常

    "msg": “详情”,

    "data": {返回数据对象，如果需要}

 } 
```

**请求参数说明：**

### 5.5.2 系统恢复

**调用方法：**

- 系统恢复

**接口URL**：POST /ess-api/system/resume

**请求报文：**



```json

{
  "zone": "" //库区
}
```
**响应报文：**
```json
{
    "code": 0, // 0 代表正常，其余代表异常
    "msg": “详情”,
    "data": {返回数据对象，如果需要}
 } 
```

**回调参数说明：**






```json


```
**响应报文：**
```json


```

**回调参数说明：**






```json


```
**响应报文：**
```json


```

**回调参数说明：**






```json


```
**响应报文：**
```json


```

**回调参数说明：**






```json


```
**响应报文：**
```json


```

**回调参数说明：**






```json


```
**响应报文：**
```json


```

**回调参数说明：**



**响应报文：**


**回调参数说明：**


**响应报文：**


**回调参数说明：**



**响应报文：**


**回调参数说明：**



**响应报文：**


**回调参数说明：**





**响应报文：**


**回调参数说明：**




**响应报文：**


**回调参数说明：**




**响应报文：**


**回调参数说明：**





**响应报文：**


**回调参数说明：**