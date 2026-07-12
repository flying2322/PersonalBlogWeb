APPS

- Xmind

- Clion

- Office365

- Fei
   Shu Pro

- AeoSun-Sunflower

Ubuntu
tips

Top
apps:

- Cursor

- Vscode

- Clion

better
trash-collector

sudo
apt install trash-cli

alias
rm=trash-put # write this in ~/.bashrc

After
this setting, when we trash-put a.sh, We can restore the file by
trash-restore

$
trash-restore

0
2026-05-20 10:31:43 /home/xxx/a.sh

1
2026-05-25 14:22:10 /home/xxx/b.txt

2
2026-05-27 09:15:02 /home/xxx/old_dir

What
file to restore [0...2]:

Hard
Part:

1.boost::asio::io_context

async
task: TCP connect, data Read/Write, Timer, Callback

run()

在这行构造函数里：

    Vehicle(ProtocolType protocol, boost::asio::io_context& ioContext);

io_context 来自 **Boost.Asio**，可以理解成：异步 **I/O** 的“事件循环引擎”。

---

## 它是什么

boost::asio::io_context 负责：

- 调度异步任务（TCP
   连接、读数据、定时器、回调等）

- 在某个线程里不断 **run()**，处理“就绪”的事件并执行对应回调

可以把它想成：

> 一个总调度中心：网络读写、超时、重连定时器等都向它登记，到时间了由它来执行你的回调函数。

参数写成 **io_context&**（引用）
是因为：

- 不拥有这个对象（生命周期由外部管理）

- 多个 Vehicle、TcpClient、定时器可以共用同一个 io_context

---

## 在本项目里干什么

构造时保存到成员变量：

Vehicle::Vehicle(ProtocolType protocol, boost::asio::io_context& ioContext)  
    : protocol(protocol), ioContext_(ioContext), ...

启动车辆时，用它创建 TCP 客户端：

void Vehicle::createSessionClient(boost::asio::io_context& io, ...) {  
    auto client = std::make_shared<TcpClient<Handler>>(io, ip, port, handler);  
    client->start();  
}

TcpClient 里的 socket、重连定时器 reconnectTimer_ 等都依赖这个 io_context 做异步通信。

全局由 VehicleManager 注册并管理：

    void addIoContext(std::shared_ptr<boost::asio::io_context> ioContext);  
    boost::asio::io_context& getIoContext();

---

## 和“普通同步代码”的对比

| 同步写法           | 用 io_context 的异步写法    |
| -------------- | --------------------- |
| connect() 阻塞等待 | async_connect()，完成后回调 |
| 线程里 sleep 轮询   | steady_timer 定时触发     |
| 一个连接一个线程       | 一个 io_context 可驱动很多连接 |

所以 AGV 调度系统里：很多车辆 **TCP** 连接不必各占一个阻塞线程，而是挂在同一个（或几个）io_context 上异步处理。

---

## 典型生命周期（概念）

1. 上层创建 io_context  
2. 某线程调用 io_context.run()   ← 开始处理事件  
3. Vehicle / TcpClient 把异步操作注册到 io_context  
4. 网络数据到达 / 定时器到期 → run() 里执行回调  
5. 程序退出时 stop io_context

---

## 为什么 Vehicle 构造就要传它

因为 Vehicle 后续要：

- 和 AGV 建 **TCP** 会话（控制、状态、任务等）

- 用
   定时器（如下载地图等，见 downloadTimer_）

这些 Asio 对象创建时都必须绑定一个 io_context，所以构造器需要传入。

---

一句话：io_context 是 Boost.Asio 的异步 **I/O** 事件循环；这里传给 Vehicle，是为了让车辆的网络连接、定时器等异步操作有统一的调度中心，并由 VehicleManager 统一管理。

# .hpp vs .h+.cpp

This project uses a lot of .hpp,
but **not mainly because it’s more compact or easier to read**.
It’s mostly **C++ convention + project structure + some
implementation-in-header habits**.

## 1. .hpp vs .h — mostly

naming

| Extension | Meaning                                                       |
| --------- | ------------------------------------------------------------- |
| .h        | C / old C++ style (this repo also has frame.h,<br> spdlog .h) |
| .hpp      | Explicitly **C++ header**                                     |

Functionally they’re the same kind of file (declarations,
templates, inline code). The team chose .hpp for C++ modules — that’s style, not a technical advantage.

## 2. How this repo is actually organized

Typical layout:

module/domain/  
  include/.../*.hpp   ← 类声明 + 很多实现也写在这里  
  src/*.cpp           ← 较大、较独立的实现

Examples:

- **vehicle.cpp** (~1450 行)
   — 构造、启动、会话等核心逻辑在 .cpp

- **vehicle_collision_checker.hpp** — 碰撞检测逻辑几乎全在头文件里

- **tcp_client.hpp** (~430 行)
   — 整个 TCP 客户端是 header 里的模板类

- **automatic_door.hpp** (~900 行)
   — 大量业务逻辑直接在头文件

所以：不是“只用 **hpp** 当源码”，而是 **hpp** 声明 **+** 部分**/**全部实现混用。

## 3. 为什么很多实现写在 .hpp 里

### 必须用头文件的情况

- 模板（如 TcpClient<Handler>、createSessionClient<SessionT>）  
  编译器要在每个使用点看到完整定义，不能只放在 .cpp。

### 项目里常见、但可选的做法

- 小函数 **inline** 在类定义里（getter、简单工具）

- **header-only** 风格：一个 #include 就能用，少配链接

- 历史**/**团队习惯：AGV
   调度这类项目常快速迭代，头文件改一处多处生效

这些和
“更紧凑、更好查” 没有必然关系；有时反而让单个文件
更长、更难读（例如 900 行的 automatic_door.hpp）。

## 4. “更紧凑、更好查”吗？

| 说法  | 实际情况                                              |
| --- | ------------------------------------------------- |
| 更紧凑 | ❌ Often **no** — 大段实现堆在 .hpp 里，文件会很大              |
| 更好查 | ⚠️ 看情况<br> — 声明和实现在一起，小类好查；大文件 Ctrl+F<br> 也费劲     |
| 编译  | ⚠️ 头文件改一处，所有 **include** 它的 **.cpp** 都要重编，大项目编译更慢 |
| 跳转  | 和 .h 一样，取决于 clangd / compile_commands.json，与扩展名无关 |

更常见的工程做法是：

- **.hpp**：接口、小 inline、模板

- **.cpp**：大段实现（这个项目对部分模块也是这样，如 vehicle.cpp、scene_manager.cpp）

## 5. 结合你看到的代码

Vehicle(ProtocolType protocol, boost::asio::io_context& ioContext);

- 声明在 **vehicle.hpp**

- 定义在 **vehicle.cpp**

而像 VehicleCollisionChecker::checkLine 这类，声明和实现都在同一个 **.hpp**，所以 Ctrl+click 在同文件里就能跳到实现，不需要 .cpp。

---

总结：  
.hpp 在这里主要是 **C++** 头文件命名习惯
和 **include/src** 目录结构；很多逻辑写在头文件里，是因为
模板、**inline**、团队风格，不是因为 .hpp 本身更紧凑或更好查。大逻辑放在 .hpp 里往往会
增加文件长度和编译依赖，并不总是更易维护。

---
