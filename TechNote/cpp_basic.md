# std::atan2(dy,dx)

`atan2` 在 C++ 的头文件 `<cmath>` 中（也可来自 C 的 `<math.h>`）。使用示例：`#include <cmath>` 然后调用 `double a = std::atan2(y, x);`。



# constexpr

是的，`constexpr` 是 C++ 的关键字。

它的含义是告诉编译器这个表达式、变量、函数或构造函数可以在编译期求值。简单来说：

- `constexpr` 变量：表示常量，必须在编译期即可确定其值。
- `constexpr` 函数：如果参数都是常量表达式，函数调用可以在编译期计算结果。
- `constexpr` 构造函数：可以创建可以在编译期初始化的对象。

具体用途：

- 提高性能：编译期计算可以减少运行期开销。
- 用于数组大小、模板参数、`static_assert`、`std::array` 等要求常量表达式的场景。
- 保证值在编译期可知，增强类型和语义安全。

需要注意：

- `constexpr` 函数内部必须只有一条 `return` 语句（C++11），后续标准放宽了限制，可包含分支、循环等，但仍要求表达式在编译期可求值。
- 并不是所有 `constexpr` 函数调用都会在编译期执行；如果参数不是常量表达式，则会退回到运行期执行。
- `constexpr` 不是强制“必定编译期执行”，而是“允许编译期求值”。

例子：

```cpp
constexpr int square(int x) {
    return x * x;
}

constexpr int n = square(5); // 编译期计算，n = 25
```

所以，`FMT_CONSTEXPR` 这种宏相当于在支持的环境下把函数标记为 `constexpr`，让格式化库的解析逻辑有可能在编译期运行。


# SFINAE

我用**最通俗、最不绕弯、C++工程师面试标准答案**的方式给你讲清楚：
**SFINAE 是什么 + 用来干什么 + 怎么配合模板做「编译期方法检测」**。

---

# 1）SFINAE 是什么？
全称：
## **Substitution Failure Is Not An Error**
**替换失败不是错误**

这是 C++ 模板的**核心规则**：

> 当编译器在**模板参数推导 / 替换**的时候，如果某个重载决议失败了（比如语法不合法），
> **编译器不会直接报错，而是直接丢掉这个重载，继续尝试下一个。**

一句话人话：
## **不合适的模板，悄悄扔掉，不报错，继续找能用的。**

---

# 2）它到底解决什么问题？
你想在**编译期**判断：
- 这个类型 T **有没有成员函数 xxx**？
- 这个类型 T **能不能调用某个方法**？
- 这个类型 T **是否支持某种操作**？

没有 SFINAE，你一写就直接编译报错。
有了 SFINAE，你可以**安全地检测**。

---

# 3）最简单的 SFINAE 例子（判断是否有 .clear() 方法）
我直接给你**工业界最标准、最常用**的写法：

```cpp
#include <type_traits>

// 模板1：如果 T 有 clear()，就匹配这个
template <typename T, typename = void>
struct has_clear : std::false_type {};

// 模板2：SFINAE 检测！只有 T 能调用 .clear() 才生效
template <typename T>
struct has_clear<T, std::void_t<decltype(std::declval<T>().clear())>> : std::true_type {};
```

使用：
```cpp
has_clear<std::vector<int>>::value; // true ✅
has_clear<int>::value;              // false ✅
```

### 这里的 SFINAE 原理：
1. 编译器先尝试匹配**第二个模板**
2. 如果 `T().clear()` 不合法 → **替换失败**
3. 根据 SFINAE 规则 → **不报错，直接丢掉这个版本**
4. 自动回退到**第一个模板（false_type）**

---

# 4）你问的重点：
## **用 模板 + SFINAE 在编译期判断类型是否支持指定方法签名**

比如你要检测：
**类型 T 是否有方法 `send(const Frame&)`**

代码直接给你（可直接复制进项目）：

```cpp
template <typename T, typename = void>
struct has_send : std::false_type {};

// SFINAE 核心：检测是否能调用 send(const Frame&)
template <typename T>
struct has_send<T, std::void_t<
    decltype(std::declval<T>().send(std::declval<const Frame&>()))
>> : std::true_type {};
```

使用：
```cpp
has_send<CommunicationClient>::value; // true / false
```

---

# 5）超级大白话总结（你记这个就够）
## SFINAE = 编译器的“容错跳过机制”
- 模板不对 → **悄悄跳过**
- 不报错 → **继续找能用的**
- 用来做**编译期类型能力检测**

## 用途（你项目里正好用）
- 判断某个类**有没有某个函数**
- 判断某个类**能不能调用某个接口**
- 判断两个类型**能不能互相转换**
- 让模板**自动选择正确的实现**

---

# 6）最终一句人话记忆
## **SFINAE = 模板不合适就悄悄扔掉，不报错，继续试 → 实现编译期类型检测 ✅**


---


# CallableHandler

`CallbackHandler` 是一个面向对象的适配器，它持有一个对目标对象的弱引用，并提供几个标准回调入口：`onConnected`、`onData` 和 `onError`。它通过 `WeakTargetPtr = std::weak_ptr<T>` 定义目标对象的弱指针类型，避免因为回调保留共享指针而造成循环引用或延长对象寿命。

```cpp

template <typename T>
class CallbackHandler {
   public:
    using WeakTargetPtr = std::weak_ptr<T>;

    CallbackHandler(const std::string& id, WeakTargetPtr target) : id_(id), target_(std::move(target)) {}

    void onConnected(bool connected) {
        if (auto t = target_.lock()) {
            invokeIfCallable(&T::onConnected, t.get(), connected);
        }
    }

    void onData(const std::string& rawData) {
        if (auto t = target_.lock()) {
            invokeIfCallable(&T::onData, t.get(), rawData);
        }
    }

    void onError(const boost::system::error_code& ec) {
        if (auto t = target_.lock()) {
            invokeIfCallable(&T::onError, t.get(), ec);
        }
    }

    const std::string& id() const {
        return id_;
    }

   private:
    template <typename Method, typename Obj, typename... Args>
    auto invokeIfCallable(Method method, Obj* obj, Args&&... args) -> std::enable_if_t<std::is_invocable_v<Method, Obj*, Args...>> {
        (obj->*method)(std::forward<Args>(args)...);
    }

    template <typename Method, typename Obj, typename... Args>
    auto invokeIfCallable(Method, Obj*, Args&&...) -> std::enable_if_t<!std::is_invocable_v<Method, Obj*, Args...>> {}
    std::string id_;
    std::weak_ptr<T> target_;
};
```


构造函数 `CallbackHandler(const std::string& id, WeakTargetPtr target)` 接收一个字符串 ID 和一个弱指针目标，并将它们分别保存到成员变量 `id_` 和 `target_`。`id()` 函数返回该对象的 ID 引用，用于识别或比较回调处理器。

每个回调入口首先尝试将弱指针锁定为 `shared_ptr`：`if (auto t = target_.lock())`。如果目标对象仍然存在，则继续调用 `invokeIfCallable`，并传入目标类型成员函数指针以及对应参数。比如 `onConnected` 试图调用 `T::onConnected`，`onData` 试图调用 `T::onData`，`onError` 试图调用 `T::onError`。

`invokeIfCallable` 使用模板和 SFINAE 技术在编译期判断目标类型是否真正支持所需的方法签名。第一个重载在 `std::is_invocable_v<Method, Obj*, Args...>` 为真时启用，直接执行 `(obj->*method)(std::forward<Args>(args)...)`。第二个重载在该表达式不可调用时启用，什么也不做。这样一来，如果某个 `T` 类型没有实现某个回调方法，`CallbackHandler` 仍然可以编译，通过空操作安全忽略该回调。

这种设计的好处是它可以将回调分发到可选实现的目标对象上，而无需在运行时检查方法是否存在；目标对象的生命周期通过弱指针保障，不再持有目标的强引用。

# invokeIfCallable

Optimized tool selection`invokeIfCallable` 这里是一个模板辅助函数，用来在编译期判断目标对象是否真的可以调用某个成员函数，并根据结果有选择地执行或忽略调用。

它有两个重载：

1. `std::enable_if_t<std::is_invocable_v<Method, Obj*, Args...>>`
   - 这个版本只有在 `T` 类型确实有一个可调用签名为 `Method` 的成员函数时才存在。
   - 如果成立，它就执行 `(obj->*method)(std::forward<Args>(args)...)`，也就是调用目标对象的该成员函数。

2. `std::enable_if_t<!std::is_invocable_v<Method, Obj*, Args...>>`
   - 这个版本在上述调用不可行时启用。
   - 它什么都不做，直接返回空。

这样设计的好处是：
- 如果目标对象实现了 `onConnected`、`onData`、`onError` 中的某个函数，它就会被正常调用；
- 如果某个函数在目标类型中不存在，编译器会选择第二个空实现，程序仍然可以正常编译并安全跳过该回调。

简而言之，`invokeIfCallable` 通过 SFINAE 做“是否可调用”的判断，既保证了灵活性，也避免了运行时检查。




# DiDo

`DiDo` 是 **Digital Input/Digital Output**（数字输入/数字输出）的缩写，在这里用于定义搬运车的**数字信号点**。每个 `DiDo` 结构体代表一个数字输入或输出通道，通过 `id` 标识具体通道，`status` 表示该通道的开关状态（true=有信号/激活，false=无信号/未激活）。

---

## DiDo 的详细定义与作用

### 1. **硬件层面含义**

在工业自动化领域，DiDo 用于表示设备上的数字信号接口：
- **DI (Digital Input)**：数字输入端口，用于接收外部信号（如传感器触发、按钮按下）
- **DO (Digital Output)**：数字输出端口，用于发送控制信号（如电机启停、警报灯闪烁）

### 2. **搬运车调度中的应用场景**

在这个调度系统中，每个搬运车（AGV/AMR）会有一组 `DiDo` 结构体数组，用于监控和控制车辆的硬件状态：

```cpp
// 示例：一辆搬运车的所有数字信号点
std::vector<DiDo> vehicle_dido = {
    {0, false},  // 0号：充电接口连接检测（DI）
    {1, true},   // 1号：货叉到位传感器（DI）
    {2, false},  // 2号：急停按钮状态（DI）
    {3, true},   // 3号：警报指示灯控制（DO）
    {4, false}   // 4号：升降电机控制（DO）
};
```

### 3. **典型用途**

- **状态监控**（DI）：
  - 货物是否在位
  - 充电桩连接状态
  - 安全触边是否触发
  - 电池电量阈值检测

- **动作控制**（DO）：
  - 启动/停止搬运任务
  - 控制升降台
  - 控制声光报警器
  - 开启/关闭夹具

### 4. **调度算法中的实际代码示例**

```cpp
// 检查车辆是否可以开始搬运任务
bool canStartTask(const Vehicle& vehicle) {
    // 检查是否有急停信号（假设 id=2 是急停输入）
    for (const auto& dido : vehicle.didos) {
        if (dido.id == 2 && dido.status == true) {
            return false;  // 急停按下，不能调度
        }
    }
    
    // 检查货叉是否已升起（假设 id=5 是货叉到位传感器）
    for (const auto& dido : vehicle.didos) {
        if (dido.id == 5 && dido.status == false) {
            return false;  // 货叉未升起，不能卸货
        }
    }
    
    return true;
}

// 执行搬运任务时的控制指令
void executeTask(Vehicle& vehicle, const Task& task) {
    // 启动车辆行驶（假设 id=10 是行驶允许信号）
    setDiDo(vehicle, 10, true);
    
    // 到达目标点后，控制升降台下降（id=11 是升降控制）
    setDiDo(vehicle, 11, true);
    
    // 等待到位传感器信号（id=12 是升降到位检测）
    while (!getDiDo(vehicle, 12)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // 关闭升降控制
    setDiDo(vehicle, 11, false);
}
```

### 5. **与 AlarmInfo/Alarms 的配合**

```cpp
// 监控 DiDo 状态并产生告警
void monitorDiDo(const Vehicle& vehicle) {
    Alarms alarms;
    
    for (const auto& dido : vehicle.didos) {
        // 急停触发（id=2）且 status=true 时产生错误告警
        if (dido.id == 2 && dido.status == true) {
            alarms.errors.push_back({
                1001,
                "Emergency stop activated on vehicle",
                getCurrentTime()
            });
        }
        
        // 电池低电量警告（假设 id=7 是低电量信号）
        if (dido.id == 7 && dido.status == true) {
            alarms.warnings.push_back({
                2001,
                "Vehicle battery low",
                getCurrentTime()
            });
        }
    }
}
```

### 6. **为什么需要 DiDo 结构体**

- **标准化**：统一表示所有数字信号，无需为每个传感器单独定义变量
- **扩展性**：新增硬件设备只需增加 `DiDo` 条目，不修改核心代码
- **通信友好**：便于通过网络传输车辆状态（序列化为 JSON/Protobuf）
- **调试便利**：可以打印所有 `DiDo` 状态快速诊断问题

### 7. **与其他结构体的关系**

```
Vehicle (车辆)
  ├── Storage[]         # 货舱状态
  ├── Alarms            # 告警信息
  └── DiDo[]            # 数字信号点 ← 硬件底层状态
       ├── id: 0, status: true   # 充电检测
       ├── id: 1, status: false  # 货叉传感器
       └── id: 2, status: true   # 急停按钮
```

## 总结

`DiDo` 定义了搬运车与物理世界交互的**数字信号接口映射**。调度算法通过读取这些信号了解车辆实时状态（如是否急停、货物是否到位），并通过写入信号控制车辆动作（如启动行驶、升降货叉）。这是自动化调度系统与硬件设备之间的**关键数据桥梁**。


---

## TL;DR

`class Vehicle : public std::enable_shared_from_this<Vehicle>` 是一个 C++ 技巧，允许**从 Vehicle 类的成员函数内部获取指向当前对象的 `shared_ptr`**，解决在异步回调或类内部无法直接获取 `shared_ptr` 的问题。

---

## 为什么需要 enable_shared_from_this

### 问题场景

```cpp
class Vehicle {
public:
    void doAsyncTask() {
        // ❌ 错误：this 是原始指针，外部可能已经销毁对象
        async_operation([this]() {
            this->onComplete();  // 危险！对象可能已被删除
        });
    }
    
    void onComplete() {
        std::cout << "Task completed" << std::endl;
    }
};

// 使用
auto v = std::make_shared<Vehicle>();
v->doAsyncTask();
v.reset();  // 销毁对象
// 异步回调执行时，this 已经无效 → 崩溃！
```

### 解决方案

```cpp
class Vehicle : public std::enable_shared_from_this<Vehicle> {
public:
    void doAsyncTask() {
        // ✅ 正确：获取指向自己的 shared_ptr，延长生命周期
        auto self = shared_from_this();
        async_operation([self]() {
            self->onComplete();  // 安全！对象不会在回调执行前销毁
        });
    }
    
    void onComplete() {
        std::cout << "Task completed" << std::endl;
    }
};
```

---

## 工作原理

### 1. **enable_shared_from_this 内部机制**

```cpp
// 简化的内部实现
template<typename T>
class enable_shared_from_this {
    mutable weak_ptr<T> weak_this_;
    
public:
    shared_ptr<T> shared_from_this() const {
        return shared_ptr<T>(weak_this_);  // 从 weak_ptr 提升为 shared_ptr
    }
};

// 当创建 shared_ptr<Vehicle> 时
shared_ptr<Vehicle> sp(new Vehicle());
// shared_ptr 的构造函数会检测 Vehicle 是否继承自 enable_shared_from_this
// 如果是，将 weak_this_ 指向 this
```

### 2. **使用限制**

```cpp
class Vehicle : public std::enable_shared_from_this<Vehicle> {
public:
    void correct() {
        // ✅ 正确：对象已被 shared_ptr 管理
        auto self = shared_from_this();
    }
    
    void wrong() {
        // ❌ 错误：对象不在 shared_ptr 管理中（栈上或 new 创建）
        auto self = shared_from_this();  // 抛出 std::bad_weak_ptr 异常
    }
};

// 正确使用
auto v1 = std::make_shared<Vehicle>();
v1->correct();  // OK

// 错误使用
Vehicle v2;  // 栈对象
v2.correct();  // 抛出异常

Vehicle* v3 = new Vehicle();
v3->correct();  // 抛出异常
delete v3;
```

---

## 在 Vehicle 类中的实际应用

### 场景1：异步操作中的回调

```cpp
class Vehicle : public std::enable_shared_from_this<Vehicle> {
    std::shared_ptr<NetworkClient> client_;
    
public:
    void sendCommand(const std::string& cmd) {
        auto self = shared_from_this();  // 捕获 shared_ptr
        
        client_->async_send(cmd, [self](const Response& resp) {
            // 即使外部 shared_ptr 被释放，self 仍持有对象
            self->handleResponse(resp);
        });
    }
    
private:
    void handleResponse(const Response& resp) {
        // 处理响应
    }
};
```

### 场景2：注册回调到调度器

```cpp
class Vehicle : public std::enable_shared_from_this<Vehicle> {
public:
    void registerToScheduler(TaskScheduler& scheduler) {
        auto self = shared_from_this();
        
        // 注册周期性任务
        scheduler.addPeriodicTask([self]() {
            self->updateState();      // 安全访问
            self->checkBattery();
        }, std::chrono::seconds(1));
    }
    
    void updateState() {
        // 更新车辆状态
    }
    
    void checkBattery() {
        // 检查电池
    }
};
```

### 场景3：多线程环境中的生命周期管理

```cpp
class Vehicle : public std::enable_shared_from_this<Vehicle> {
    std::thread worker_;
    std::atomic_bool stop_{false};
    
public:
    void startWorker() {
        auto self = shared_from_this();  // 延长生命周期
        
        worker_ = std::thread([self]() {
            while (!self->stop_.load()) {
                self->doWork();
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        });
    }
    
    void stop() {
        stop_ = true;
        if (worker_.joinable()) {
            worker_.join();
        }
    }
    
    ~Vehicle() {
        stop();  // 确保线程停止
        std::cout << "Vehicle destroyed" << std::endl;
    }
    
private:
    void doWork() {
        // 实际工作
    }
};

// 使用
{
    auto v = std::make_shared<Vehicle>();
    v->startWorker();
    // v 离开作用域，但 worker 线程仍持有 self，对象不会立即销毁
}  // 当 worker 线程结束后，对象才真正销毁
```

### 场景4：工厂模式返回 shared_ptr

```cpp
class Vehicle : public std::enable_shared_from_this<Vehicle> {
public:
    static std::shared_ptr<Vehicle> create() {
        // 不能直接 return std::shared_ptr<Vehicle>(new Vehicle());
        // 因为 shared_from_this() 需要 shared_ptr 的构造信息
        
        struct MakeSharedEnabler : public Vehicle {};
        return std::make_shared<MakeSharedEnabler>();
    }
    
    std::shared_ptr<Vehicle> getShared() {
        return shared_from_this();  // 返回自身的 shared_ptr
    }
    
private:
    Vehicle() = default;  // 私有构造函数
};

// 使用
auto v = Vehicle::create();  // 正确创建
auto v2 = v->getShared();    // 获取另一个 shared_ptr（引用计数+1）
```

---

## 与普通 shared_ptr this 的对比

### 错误方式：手动创建 shared_ptr

```cpp
class Vehicle {
public:
    void doAsyncTask() {
        // ❌ 错误：创建新的 shared_ptr，会导致多个独立的管理器
        std::shared_ptr<Vehicle> self(this);  // 危险！
        async_operation([self]() {
            self->onComplete();
        });
    }
};

// 问题：
auto v1 = std::make_shared<Vehicle>();
auto v2 = std::shared_ptr<Vehicle>(v1.get());  // 两个独立的引用计数
// 会导致 double free 或 use-after-free
```

### 正确方式：使用 enable_shared_from_this

```cpp
class Vehicle : public std::enable_shared_from_this<Vehicle> {
public:
    void doAsyncTask() {
        // ✅ 正确：从 weak_ptr 提升，共享同一个引用计数
        auto self = shared_from_this();
        async_operation([self]() {
            self->onComplete();
        });
    }
};
```

---

## 常见陷阱

### 陷阱1：在构造函数中使用

```cpp
class Vehicle : public std::enable_shared_from_this<Vehicle> {
public:
    Vehicle() {
        // ❌ 错误：构造函数中不能调用 shared_from_this()
        auto self = shared_from_this();  // 抛出异常
        // 原因：此时还没有 shared_ptr 指向 this
    }
    
    void init() {
        // ✅ 正确：在构造后单独调用
        auto self = shared_from_this();
    }
};

// 正确用法
auto v = std::make_shared<Vehicle>();
v->init();
```

### 陷阱2：多次继承

```cpp
// 错误：多个基类继承 enable_shared_from_this
class Vehicle : public std::enable_shared_from_this<Vehicle>,
                public std::enable_shared_from_this<VehicleStateUpdater> {
    // ❌ 编译错误：不能多次继承 enable_shared_from_this
};

// 正确：只在最终的类继承
class Vehicle : public VehicleStateUpdater,
                public std::enable_shared_from_this<Vehicle> {
    // ✅ OK
};
```

### 陷阱3：忘记包含头文件

```cpp
// ❌ 缺少 #include <memory>
class Vehicle : public std::enable_shared_from_this<Vehicle> {
    // 编译错误
};

// ✅ 正确
#include <memory>
class Vehicle : public std::enable_shared_from_this<Vehicle> {
    // OK
};
```

---

## 与 VehicleStateUpdater 的协同

```cpp
class VehicleStateUpdater {
public:
    virtual void updateState() = 0;
};

class Vehicle : public VehicleStateUpdater, 
                public std::enable_shared_from_this<Vehicle> {
    std::vector<std::function<void()>> callbacks_;
    
public:
    void updateState() override {
        // 实现状态更新
        auto self = shared_from_this();
        // 可以安全地注册需要 this 的回调
    }
    
    void registerCallback(std::function<void(std::shared_ptr<Vehicle>)> cb) {
        callbacks_.push_back([this, cb]() {
            cb(shared_from_this());  // 传递 shared_ptr
        });
    }
};
```

---

## 总结

| 特性 | 说明 |
|------|------|
| **主要作用** | 从对象内部获取管理自己的 `shared_ptr` |
| **使用场景** | 异步回调、多线程、需要延长对象生命周期的场合 |
| **前置条件** | 对象必须已被 `shared_ptr` 管理 |
| **禁止场景** | 构造函数、析构函数、栈对象、原始指针 |
| **性能开销** | 极小（只是 weak_ptr 的 lock 操作） |
| **头文件** | `<memory>` |

**核心记忆**：`enable_shared_from_this` 解决了"从对象内部无法获取自身 `shared_ptr`"的问题，是 C++ 异步编程中的重要工具，特别是在需要确保对象在异步回调执行期间仍然存活时。

---

## TL;DR

`std::this_thread::sleep_for` 用于**阻塞当前线程**指定的时长，让出 CPU 给其他线程执行。参数可以是任意 `std::chrono::duration` 类型（毫秒、秒、微秒等）。

---

## 基本用法

```cpp
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "Start" << std::endl;
    
    // 阻塞当前线程 100 毫秒
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::cout << "After 100ms" << std::endl;
    
    return 0;
}
```

---

## 常用时间单位

```cpp
#include <chrono>
#include <thread>

// 毫秒
std::this_thread::sleep_for(std::chrono::milliseconds(500));   // 500ms

// 秒
std::this_thread::sleep_for(std::chrono::seconds(2));         // 2秒

// 微秒
std::this_thread::sleep_for(std::chrono::microseconds(1500)); // 1.5ms

// 纳秒
std::this_thread::sleep_for(std::chrono::nanoseconds(100));   // 100ns（通常精度有限）

// 使用字面量（C++14）
using namespace std::chrono_literals;
std::this_thread::sleep_for(100ms);   // 100毫秒
std::this_thread::sleep_for(2s);      // 2秒
std::this_thread::sleep_for(500us);   // 500微秒
std::this_thread::sleep_for(10ns);    // 10纳秒
```

---

## 精度与误差

### 实际睡眠时间可能更长

```cpp
auto start = std::chrono::steady_clock::now();
std::this_thread::sleep_for(std::chrono::milliseconds(10));
auto end = std::chrono::steady_clock::now();

auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
std::cout << "Slept for: " << elapsed.count() << " ms" << std::endl;
// 可能输出: Slept for: 15 ms（实际睡眠时间可能多于10ms）
```

**原因**：
- 操作系统调度延迟
- 系统时钟精度限制
- 线程优先级影响

### 操作系统调度精度

```cpp
// Windows 通常精度 ~15ms
std::this_thread::sleep_for(std::chrono::milliseconds(1));  // 可能实际睡眠 15ms

// Linux 通常精度更高
std::this_thread::sleep_for(std::chrono::milliseconds(1));  // 可能实际睡眠 1-3ms

// 提高精度的方法（平台相关）
// Linux: 使用高精度定时器
// Windows: 使用 timeBeginPeriod(1)
```

---

## 常见应用场景

### 1. 实现轮询循环

```cpp
class VehicleMonitor {
    std::atomic_bool stop_{false};
    std::thread monitor_thread_;
    
public:
    void start() {
        monitor_thread_ = std::thread([this]() {
            while (!stop_) {
                checkVehicleStatus();
                
                // 每 100ms 检查一次，避免 CPU 占用过高
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        });
    }
    
    void stop() {
        stop_ = true;
        if (monitor_thread_.joinable()) {
            monitor_thread_.join();
        }
    }
    
private:
    void checkVehicleStatus() {
        // 检查车辆状态
    }
};
```

### 2. 实现超时机制

```cpp
bool waitForCondition(std::function<bool()> condition, int timeout_ms) {
    auto start = std::chrono::steady_clock::now();
    
    while (!condition()) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
        
        if (elapsed.count() >= timeout_ms) {
            return false;  // 超时
        }
        
        // 每 10ms 检查一次
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    return true;
}

// 使用
if (waitForCondition([]() { return sensor.isReady(); }, 5000)) {
    std::cout << "Sensor ready" << std::endl;
} else {
    std::cout << "Timeout" << std::endl;
}
```

### 3. 控制帧率

```cpp
class GameLoop {
    std::chrono::steady_clock::time_point last_frame_;
    
public:
    void run() {
        const auto frame_duration = std::chrono::milliseconds(16);  // 约 60 FPS
        
        while (running_) {
            auto frame_start = std::chrono::steady_clock::now();
            
            update();
            render();
            
            auto frame_end = std::chrono::steady_clock::now();
            auto elapsed = frame_end - frame_start;
            
            if (elapsed < frame_duration) {
                // 睡眠剩余时间
                std::this_thread::sleep_for(frame_duration - elapsed);
            }
        }
    }
};
```

### 4. 模拟耗时操作

```cpp
void simulateTask(const std::string& name, int duration_ms) {
    std::cout << name << " started" << std::endl;
    
    // 模拟工作
    std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms));
    
    std::cout << name << " completed" << std::endl;
}

// 使用
simulateTask("Loading data", 500);
simulateTask("Processing", 300);
```

### 5. 背压控制（限制处理速率）

```cpp
class RateLimiter {
    std::chrono::steady_clock::time_point last_;
    std::chrono::milliseconds interval_;
    
public:
    RateLimiter(int requests_per_second) 
        : interval_(1000 / requests_per_second) {}
    
    void acquire() {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = now - last_;
        
        if (elapsed < interval_) {
            std::this_thread::sleep_for(interval_ - elapsed);
        }
        
        last_ = std::chrono::steady_clock::now();
    }
};

// 使用
RateLimiter limiter(10);  // 每秒最多10次
for (int i = 0; i < 100; i++) {
    limiter.acquire();
    sendRequest();  // 每秒最多执行10次
}
```

---

## 与其他同步方式的对比

| 方法 | 用途 | 精度 | 可中断 | 适用场景 |
|------|------|------|--------|---------|
| `sleep_for` | 固定时间阻塞 | 中 | 否 | 轮询、延时 |
| `condition_variable::wait_for` | 等待条件或超时 | 中 | 可被 notify | 等待事件 |
| `std::this_thread::yield` | 让出时间片 | 不适用 | 否 | 忙等待优化 |
| 空循环（busy wait） | 极短等待 | 高 | 否 | 无锁编程中的微等待 |

```cpp
// 短时间等待（微秒级）- 忙等待
auto start = std::chrono::steady_clock::now();
while (std::chrono::duration_cast<std::chrono::microseconds>(
    std::chrono::steady_clock::now() - start).count() < 10) {
    // 忙等待，不放弃 CPU
}

// 毫秒级等待 - sleep_for
std::this_thread::sleep_for(std::chrono::milliseconds(10));

// 等待事件 - condition_variable
std::condition_variable cv;
cv.wait_for(lock, std::chrono::milliseconds(100), []{ return ready; });
```

---

## 线程安全注意事项

### sleep_for 与停止标志

```cpp
class Worker {
    std::atomic_bool stop_{false};
    std::thread worker_;
    
public:
    // ❌ 错误：sleep_for 无法被中断
    void badLoop() {
        while (!stop_) {
            doWork();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        // 问题：在 sleep 期间设置 stop_，需要等待 1 秒才能响应
    }
    
    // ✅ 正确：分段睡眠，频繁检查停止标志
    void goodLoop() {
        while (!stop_) {
            doWork();
            // 每 100ms 检查一次停止标志
            for (int i = 0; i < 10 && !stop_; i++) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    }
};
```

### sleep_for 与互斥锁

```cpp
std::mutex mtx;

// ❌ 错误：在持有锁时睡眠
void badLocking() {
    std::lock_guard<std::mutex> lock(mtx);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    // 其他线程被阻塞 1 秒
}

// ✅ 正确：睡眠前释放锁
void goodLocking() {
    {
        std::lock_guard<std::mutex> lock(mtx);
        updateSharedData();
    }  // 锁释放
    std::this_thread::sleep_for(std::chrono::seconds(1));  // 不持有锁
}
```

---

## 常用时间字面量（C++14）

```cpp
using namespace std::chrono_literals;

// 直观的时间表示
std::this_thread::sleep_for(100ms);   // 100毫秒
std::this_thread::sleep_for(5s);      // 5秒
std::this_thread::sleep_for(250us);   // 250微秒
std::this_thread::sleep_for(500ns);   // 500纳秒
std::this_thread::sleep_for(2min);    // 2分钟
std::this_thread::sleep_for(1h);      // 1小时

// 需要 C++14 编译选项
// g++ -std=c++14 main.cpp
```

---

## 实际示例：车辆调度系统中的应用

```cpp
class VehicleDispatcher {
    std::atomic_bool running_{true};
    std::thread dispatch_thread_;
    
public:
    void start() {
        dispatch_thread_ = std::thread([this]() {
            while (running_) {
                // 检查待分配任务
                auto tasks = getPendingTasks();
                
                if (tasks.empty()) {
                    // 无任务时，每 500ms 检查一次
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    continue;
                }
                
                // 分配任务
                for (auto& task : tasks) {
                    auto vehicle = selectOptimalVehicle(task);
                    if (vehicle) {
                        vehicle->assignTask(task);
                    }
                }
                
                // 避免过于频繁的调度（每 100ms 最多一次）
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        });
    }
    
    void stop() {
        running_ = false;
        if (dispatch_thread_.joinable()) {
            dispatch_thread_.join();
        }
    }
};
```

---

## 总结

| 要点 | 说明 |
|------|------|
| **作用** | 阻塞当前线程指定时长 |
| **头文件** | `<thread>` 和 `<chrono>` |
| **精度** | 取决于操作系统调度，实际睡眠时间 ≥ 请求时间 |
| **参数类型** | 任意 `std::chrono::duration` |
| **C++14 特性** | 支持 `100ms`、`2s` 等字面量 |
| **替代方案** | `sleep_until`（睡眠到绝对时间点） |
| **主要用途** | 轮询、延时、速率控制、模拟 |
| **注意事项** | 睡眠时无法被中断（除非使用 condition_variable） |

**核心记忆**：`sleep_for` 是线程的"暂停"功能，让出 CPU 给其他线程执行。适合毫秒级以上的延时，不适合高精度定时（纳秒/微秒级）。在需要可中断睡眠的场景，建议使用分段睡眠或 `condition_variable::wait_for`。




