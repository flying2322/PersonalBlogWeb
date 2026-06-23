utils

## C++ 中的 Stopwatch 工具：核心功能详解

`Stopwatch` 是 C++ 中一个非常实用的工具类，主要用于**高精度测量代码执行时间**，是性能测试和算法优化的必备工具。很多开发者（包括我自己）在优化代码时都会用它来精确定位性能瓶颈。

### 📊 核心功能与中文对照

根据历史文档和现代实现，`Stopwatch` 的核心功能可以总结如下：

| 功能分类      | 核心函数/属性               | 中文含义   | 作用说明                                           |
|:--------- |:--------------------- |:------ |:---------------------------------------------- |
| **控制操作**  | `start()`             | 开始/启动  | 开始计时，或从暂停处恢复计时                                 |
|           | `stop()`              | 停止     | 停止计时，记录经过的时间段                                  |
|           | `reset()`             | 重置     | 将所有计时数值归零，但保持当前运行状态不变（运行中或停止中）                 |
|           | `restart()`           | 重启     | 这是一个便捷操作，等同于先`reset()`再`start()`，用于开始一段全新的测量   |
| **时间读取**  | `elapsed`             | 经过时间   | 获取从开始到当前的总时间，通常以 `TimeSpan` 对象形式返回             |
|           | `elapsedMilliseconds` | 毫秒数    | 返回经过时间的毫秒数，比较直观，最常用                            |
|           | `elapsedTicks`        | 计时器滴答数 | 返回计时器的原始单位（Tick），精度最高，但需要除以 `Frequency` 才能换算成秒 |
|           | `real()`              | 真实时间   | 指现实世界的时间，也叫"挂钟时间"（Wall Clock Time）             |
|           | `user()`              | 用户时间   | 代码在用户态运行所消耗的 CPU 时间                            |
|           | `system()`            | 系统时间   | 代码在内核态（如系统调用）消耗的 CPU 时间                        |
| **状态与信息** | `isRunning`           | 是否正在运行 | 一个布尔值，用于判断计时器当前是否在工作                           |
|           | `frequency`           | 计时频率   | 计时器每秒的滴答数，用于将 `Ticks` 转换为秒                     |
|           | `resolution()`        | 分辨率    | 返回计时器所能测量的最小时间单位，系统不同，精度也不同                    |

### ⏱️ 三种时间的区别

理解三种时间的区别是精准分析程序性能的关键：

- **`real()` (真实时间/挂钟时间)**：**物理时间**，从代码开始执行到结束，你拿着秒表卡的时间。它包含了代码运行、I/O等待、甚至线程被CPU挂起的所有时间。
- **`user()` (用户时间)**：**CPU 执行用户代码的时间**。比如循环、计算、函数调用，这部分时间才算程序的"有效工作"时间。
- **`system()` (系统时间)**：**CPU 在内核态执行的时间**。当程序调用系统级功能（比如操作文件、分配内存、网络通信）时，这部分工作在操作系统层面完成，消耗的就是系统时间。

> **举个例子**：你去餐厅吃饭，从进门到出门是`real时间`；厨师真正炒菜是`user时间`；服务员传菜、收碗是`system时间`。如果`real`远大于`user`+`system`，说明菜大部分时间在"等"（等待I/O或CPU调度）。

### 🔧 典型用法示例

通常的使用模式是 RAII 模式或显式调用控制函数。

**1. 显式控制模式**
这是最灵活、最常见的方式，适用于测量明确代码块的耗时：

```cpp
#include <iostream>
#include <chrono> // C++11 起的高精度计时库

class Stopwatch {
public:
    Stopwatch() : start_time(std::chrono::high_resolution_clock::now()) {}

    void start() {
        start_time = std::chrono::high_resolution_clock::now();
    }

    void stop() {
        stop_time = std::chrono::high_resolution_clock::now();
    }

    double elapsedMilliseconds() {
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time);
        return duration.count();
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time, stop_time;
};

int main() {
    Stopwatch sw;
    sw.start();

    // 这里是你要测量的代码
    for (int i = 0; i < 1000000; ++i) {
        // 模拟一些工作
    }

    sw.stop();
    std::cout << "运行耗时：" << sw.elapsedMilliseconds() << " 毫秒" << std::endl;
    return 0;
}
```

**2. RAII 自动模式**
利用 C++ 对象生命周期自动计时，非常方便：

```cpp
class AutoStopwatch {
public:
    AutoStopwatch() : start(std::chrono::high_resolution_clock::now()) {}
    ~AutoStopwatch() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "代码块执行耗时：" << duration.count() << " 毫秒\n";
    }
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
};

int main() {
    {
        AutoStopwatch watch; // 进入作用域，开始计时
        // 这里是你要测量的代码
        for (int i = 0; i < 1000000; ++i);
    } // 离开作用域，析构函数自动停止并打印时间

    return 0;
}
```

### 💡 最佳实践与注意事项

1. **处理嵌套调用**：如果你的函数会递归调用或嵌套计时，简单的 Stopwatch 可能不准。高级实现会通过**入口计数器**来记录嵌套深度，确保只有最外层的调用测量完整时间。
2. **校准测量开销**：Stopwatch 自身也有运行开销（函数调用、系统调用）。测量极短的代码块时，可以运行一个空循环来测量基准开销，然后从结果中减去。
3. **多次测量取平均**：为了获得更可靠的数据，特别是面对有波动的代码，**多次运行取平均值**是更好的选择。
