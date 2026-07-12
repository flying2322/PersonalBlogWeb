# Gray

```math
\int_0^\infty \frac{x^3}{e^x-1}\,dx = \frac{\pi^4}{15}
```


$$
\int_0^\infty \frac{x^3}{e^x-1}\,dx = \frac{\pi^4}{15}
$$

```math
\begin{equation}
2(x+5)-7 = 3(x-2)
\end{equation}
```

```math
\begin{equation}
2x+10-7 = 3x-6
\end{equation}
```

```math
\begin{equation}
9 = x
\end{equation}
```



# White
容器属性：高长宽重材向码



# 2.query container
API address: http//{server_url}/conveyor/queryConveyor

Request:
```json
{
  "conveyorCodes": ["strign"]
}
```=

Response:

```json
{
   "code": 0,
   "msg": "success",
   "data": {
     "conveyors": [
        {
           "code": "1",
           "nodeStates": [
               {
                 "slotCode": "1-5",
                 "hasContainer": true,
                 "containerCode": "F00001",
                 "lastReadTime": 1635402778670,
                 "lastReportTime": 1635402776860,
                 "lastHasContainerTime": 1635402270067
               }
            ]
        }
      ]
   }
}
```

# 3. load container request
API address: http://{server_url}/conveyor/loadContainerReq

Request:
```json
{
  "slotCode": "c1-1", 
  "containerCode": "container001"
}
```

Response:
```json
{
  "code": 0, //0 stand for normal, other for abnormal
  "msg": "details",
  "data": {
    "allow": true
  }
}
```


# 4. Laod container finish
API: http://{server_url}//conveyor/loadContainerFinish

Request:
```json
{ 
  "slotCode": "string",
  "containerCode": "string",
}
```

Response:
```json
{
  "code": 0, // 0 for normal, and others for abnormal
  "msg": "details",
  "data": {return data object, if needed}
}
```

# 5. Unload container request
API: http://{server_url}/conveyor/unload/ContainerReq

Request:
```json
{
  "slotCode": "string"
  "containerCode":"string"
}
```
Response:
```json
{
  "code":0, // 0 stand for normal, others for abnormal
  "msg": "details"
  "data": {
    "allow":true   //true: permit false or null mean not permit
  }
}
``` 

# 6. Unload container finish

can be customerized by customer and can refer to this API: http://{server_url}/conveyor/unloadContainerFinish

Request:
```json
{
  "slotCode":"string",
  "containerCode": "string",
  "containeAttribute": containerAttribute
}
```

Response:
```json
{
  "code": 0, // 0 for normal ant others for abnormal
  "msg": "details",
  "data": {return data object, if needed}
}

```


# 7. Container arrived

API: http://{server_url}/conveyor/containerArrived

```json
{ 
  "slotCode": "string", //ku wei bian ma
  "containerCode": "string", // rongqi bianma
  "containerAttribute": containerAttribute // rongqi shuxing
}
```

ModBus API definition

![clipboard.png](inkdrop://file:r_uwMC3KU)

When the barcode reader of the picking location failed to read the barcode. the tote stays in this position and the conveyor will give an alarm. Manually remove and reset the tote status of this position on the touch screen to resume operation.

| Signal name | PLC address | Type | Direction | Descritption |
| ------- | ------- | ------- | ------- | ------- |
| Location No.1 Unload permission | VB2000 (Start address) | Byte(2) | Conveyor->ESS | 1: allowed 0: not allowed |
| The tote has been placed on Location NO.1 | VB2002 | Byte(2) | 1: ESS->Conveyor 0: Conveyor | Default: 0 indicates that the conveyor has been picked up this signal, 1 indicates that the tote has been placed on this location |
| The barcode of location No.1 | VB2004 | Byte(20) | ESS->Conveyor | Default: 0, After the robot has finished placing the tote, the tote code will be written to the storage area |
| Location No.7 picking request | VB2026 | Byte(2) | Conveyor->ESS | Default: 0, 1 indicates that the tote has been assived on this location |
| the barcode of location No. 7 | VB2028 | Byte(2) | Conveyor->ESS | The default value is 0, After the robot has finished placing the tote, the tote code will be written to the storage area |
| Picking complete on Location No.7 | VB2050 | Byte(2) | 1. ESS->Conveyor 2. ESS->Conveyor 0:Conveyor | Default value 0, 1 indicates that the tote can leave, and 2 indicate that the tote has been removed artificially. After the tote is picked, 1 indicates that the tote needs to be returned to its location, the conveyor reads 1 and starts to flow and resets this address to 0; If the totes is removed artificially(such as empty tote, abnormal tote), the conveyor reads 2 and clears the tote code at this location and resets the address to 0 |
| Location No.12 returning request | VB2052 | Byte(2) | Conveyor->ESS | Default value is 0. 1 indicate that the tote which needs to be returned to its location has been arrived on this location |
| the barcode of location No.12 | VB2054 | Byte(20) | Conveyor->ESS | The default value is 0. After the robot has finished placing the tote, the tote code will be written to the storage area |
| The tote on No.12 has been loaded  | VB2076 | Byte(2) | 1.ESS->Conveyor 2. ESS->Conveyor 0: Conveyor | Default vaule is 0. 1. indicate that the robot has finished loading tote; 2 indicates that the tote is abnormal; 0 indicates that the conveyor has picked up the signal |
