# ASIO
## Boost.Asio io_context 详解

`boost::asio::io_context` 是 Boost.Asio 库的**核心 I/O 服务对象**，负责事件循环和异步操作调度。

## 核心概念

`io_context` 充当 I/O 服务与操作系统之间的桥梁：
- 管理 I/O 对象（socket、timer 等）
- 调度异步操作
- 执行回调函数
- 维护事件队列

## 主要用法

### 1. 基本使用
```cpp
timer.async_wait( [](const boost::system::error_code& ec) { ... } );
// \_________/    \_____________________________________________/
//   1. 异步触发                     2. 回调函数 (Lambda)
```
这行代码的作用是启动一个异步等待。它告诉定时器：“你在后台默默倒计时 1 秒，不要阻塞我（主线程）。等时间到了，你再通知 io_context 去执行我传给你的这个 Lambda 表达式（回调函数），打印出 'Timer expired!'”。
```cpp
#include <boost/asio.hpp>
#include <iostream>

int main() {
    // 创建 io_context 对象
    boost::asio::io_context io;
    
    // 1. 同步操作 - 直接运行
    // 会阻塞直到所有工作完成
    io.run();  // 没有工作，立即返回
    
    // 2. 添加工作后运行
    boost::asio::steady_timer timer(io, std::chrono::seconds(1));// 1 second tragger timer setted
    timer.async_wait([](const boost::system::error_code& ec) {std::cout << "Timer expired!" << std::endl;});
    
    io.run();  // 阻塞 1 秒，执行回调后返回
    
    return 0;
}
```
1. `timer.async_wait(...)`（非阻塞注册）
- 非阻塞： 与同步的 `timer.wait()` 不同，`async_wait` 调用的瞬间就会立即返回，不会在这一行死等 1 秒。

- 注册任务： 它只是向底层操作系统（或 io_context）注册了一个“定时”任务，并把事件发生时要执行的“锦囊妙计”（回调函数）交代好。

#### 为了更清楚它为什么能工作，我们需要结合 io.run() 来看整个程序的生命周期：

1. 创建定时器： boost::asio::steady_timer timer(io, std::chrono::seconds(1)); 设定了一个 1 秒后到期的定时器。

2. 登记任务（当前代码行）： 执行 timer.async_wait(...)。此时主线程只是登了个记，把回调函数挂载到 io 对象的任务队列里，然后立刻执行下一行。

3. 启动事件循环： 程序运行到后面的 io.run();。

    - 因为有了 async_wait 登记的任务，io.run() 发现自己有工作要做，于是阻塞主线程，开始等待。

    - 1 秒钟过去后，操作系统通知 io_context 定时器时间已到。

    - io.run() 收到通知，在主线程中调用刚才登记的 Lambda 表达式，屏幕上打印出 "Timer expired!"。

    - 任务全部完成，io.run() 退出，程序结束。

### 2. 异步操作调度

```cpp
class AsyncServer {
    boost::asio::io_context io_;
    boost::asio::ip::tcp::acceptor acceptor_;
    
public:
    AsyncServer(short port) 
        : io_()
        , acceptor_(io_, boost::asio::ip::tcp::endpoint(
            boost::asio::ip::tcp::v4(), port)) {
        startAccept();
    }
    
    void startAccept() {
        auto socket = std::make_shared<boost::asio::ip::tcp::socket>(io_);
        acceptor_.async_accept(*socket, 
            [this, socket](const boost::system::error_code& ec) {
                if (!ec) {
                    std::cout << "New connection accepted" << std::endl;
                    handleClient(socket);
                }
                startAccept();  // 继续接受下一个连接
            });
    }
    
    void run() {
        io_.run();  // 启动事件循环
    }
    
private:
    void handleClient(std::shared_ptr<boost::asio::ip::tcp::socket> socket) {
        auto buffer = std::make_shared<std::array<char, 1024>>();
        socket->async_read_some(boost::asio::buffer(*buffer),
            [this, socket, buffer](const boost::system::error_code& ec, size_t length) {
                if (!ec) {
                    std::cout << "Received: " << std::string(buffer->data(), length);
                    // 回显
                    boost::asio::async_write(*socket, 
                        boost::asio::buffer(*buffer, length),
                        [socket](const boost::system::error_code& ec, size_t) {
                            if (ec) {
                                std::cerr << "Write error: " << ec.message() << std::endl;
                            }
                        });
                }
            });
    }
};
```

### 3. 多线程事件循环

```cpp
#include <thread>
#include <vector>

class ThreadPool {
    boost::asio::io_context io_;
    std::vector<std::thread> threads_;
    
public:
    ThreadPool(size_t thread_count = std::thread::hardware_concurrency()) {
        // 添加工作防止 io_context 在没有任务时退出
        auto work = std::make_shared<boost::asio::io_context::work>(io_);
        
        for (size_t i = 0; i < thread_count; ++i) {
            threads_.emplace_back([this]() {
                io_.run();  // 每个线程运行事件循环
            });
        }
    }
    
    ~ThreadPool() {
        io_.stop();  // 停止所有线程
        for (auto& t : threads_) {
            if (t.joinable()) t.join();
        }
    }
    
    boost::asio::io_context& get_io_context() { return io_; }
    
    void post(std::function<void()> task) {
        boost::asio::post(io_, task);
    }
};

// 使用示例
int main() {
    ThreadPool pool(4);
    
    // 投递任务到线程池
    for (int i = 0; i < 10; ++i) {
        pool.post([i]() {
            std::cout << "Task " << i << " executed by thread " 
                      << std::this_thread::get_id() << std::endl;
        });
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}
```

## 核心方法详解

### 1. `run()` - 运行事件循环

```cpp
boost::asio::io_context io;
io.run();  // 阻塞直到没有工作

// 保持工作状态
auto work = std::make_shared<boost::asio::io_context::work>(io);
io.run();  // 即使没有异步操作也会阻塞
work.reset();  // 释放工作，run() 返回
```

### 2. `poll()` - 非阻塞运行

```cpp
boost::asio::io_context io;
boost::asio::steady_timer timer(io, std::chrono::seconds(1));
timer.async_wait([](auto& ec) { std::cout << "Done" << std::endl; });

io.poll();  // 立即返回，不等待定时器
std::this_thread::sleep_for(std::chrono::seconds(2));
io.poll();  // 执行完成回调
```

### 3. `stop()` 和 `stopped()`

```cpp
boost::asio::io_context io;
boost::asio::steady_timer timer(io, std::chrono::seconds(10));
timer.async_wait([](auto& ec) { 
    if (ec == boost::asio::error::operation_aborted) {
        std::cout << "Timer cancelled" << std::endl;
    }
});

std::thread t([&io]() {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    io.stop();  // 停止事件循环
});

io.run();  // 1 秒后停止
t.join();
```

### 4. `post()` 和 `dispatch()` - 任务投递

```cpp
boost::asio::io_context io;

// post: 异步执行，立即返回
boost::asio::post(io, []() {
    std::cout << "Async task" << std::endl;
});

// dispatch: 可能在当前线程立即执行
boost::asio::dispatch(io, []() {
    std::cout << "Dispatched task" << std::endl;
});

// defer: 延迟执行
boost::asio::defer(io, []() {
    std::cout << "Deferred task" << std::endl;
});

// 区别示例
boost::asio::io_context io2;
io2.dispatch([]() { 
    std::cout << "Executed immediately" << std::endl; 
});  // 立即执行，不需要 run()

io2.post([]() { 
    std::cout << "Executed in run()" << std::endl; 
});  // 需要 run() 才执行
io2.run();
```

## 常见应用场景

### 场景1：网络服务器

```cpp
class HttpServer {
    boost::asio::io_context io_;
    boost::asio::ip::tcp::acceptor acceptor_;
    std::vector<std::thread> workers_;
    
public:
    HttpServer(short port, size_t thread_count = 4) 
        : acceptor_(io_, boost::asio::ip::tcp::endpoint(
            boost::asio::ip::tcp::v4(), port)) {
        
        for (size_t i = 0; i < thread_count; ++i) {
            workers_.emplace_back([this]() { io_.run(); });
        }
        
        startAccept();
    }
    
    ~HttpServer() {
        io_.stop();
        for (auto& t : workers_) {
            if (t.joinable()) t.join();
        }
    }
    
private:
    void startAccept() {
        auto socket = std::make_shared<boost::asio::ip::tcp::socket>(io_);
        acceptor_.async_accept(*socket, 
            [this, socket](const boost::system::error_code& ec) {
                if (!ec) {
                    handleRequest(socket);
                }
                startAccept();
            });
    }
    
    void handleRequest(std::shared_ptr<boost::asio::ip::tcp::socket> socket) {
        auto buffer = std::make_shared<std::vector<char>>(8192);
        socket->async_read_some(boost::asio::buffer(*buffer),
            [this, socket, buffer](const boost::system::error_code& ec, size_t length) {
                if (!ec) {
                    std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello World!";
                    boost::asio::async_write(*socket, 
                        boost::asio::buffer(response),
                        [socket](const boost::system::error_code& ec, size_t) {
                            // 响应已发送
                        });
                }
            });
    }
};
```

### 场景2：定时器任务调度

```cpp
class TaskScheduler {
    boost::asio::io_context io_;
    std::unique_ptr<boost::asio::io_context::work> work_;
    std::thread worker_;
    
public:
    TaskScheduler() 
        : work_(std::make_unique<boost::asio::io_context::work>(io_))
        , worker_([this]() { io_.run(); }) {}
    
    ~TaskScheduler() {
        work_.reset();  // 允许 io_context 退出
        if (worker_.joinable()) worker_.join();
    }
    
    void scheduleOnce(std::chrono::milliseconds delay, std::function<void()> task) {
        auto timer = std::make_shared<boost::asio::steady_timer>(io_, delay);
        timer->async_wait([timer, task](const boost::system::error_code& ec) {
            if (!ec) {
                task();
            }
        });
    }
    
    void scheduleRepeating(std::chrono::milliseconds interval, std::function<void()> task) {
        auto timer = std::make_shared<boost::asio::steady_timer>(io_, interval);
        timer->async_wait([this, timer, interval, task](const boost::system::error_code& ec) {
            if (!ec) {
                task();
                // 重新安排
                timer->expires_at(timer->expiry() + interval);
                timer->async_wait([this, timer, interval, task](const auto& ec) {
                    if (!ec) task();
                });
            }
        });
    }
};
```

### 场景3：异步信号处理

```cpp
class SignalHandler {
    boost::asio::io_context io_;
    boost::asio::signal_set signals_;
    
public:
    SignalHandler() 
        : signals_(io_, SIGINT, SIGTERM) {
        startWait();
    }
    
    void run() {
        io_.run();
    }
    
private:
    void startWait() {
        signals_.async_wait([this](const boost::system::error_code& ec, int signal) {
            if (!ec) {
                std::cout << "Received signal " << signal << ", shutting down..." << std::endl;
                io_.stop();
            }
        });
    }
};

// 使用
int main() {
    SignalHandler handler;
    std::cout << "Press Ctrl+C to stop" << std::endl;
    handler.run();
    return 0;
}
```

### 场景4：串口通信

```cpp
class SerialPortReader {
    boost::asio::io_context io_;
    boost::asio::serial_port port_;
    std::array<char, 256> buffer_;
    
public:
    SerialPortReader(const std::string& device, int baud_rate)
        : port_(io_, device) {
        port_.set_option(boost::asio::serial_port::baud_rate(baud_rate));
        startRead();
    }
    
    void run() {
        io_.run();
    }
    
private:
    void startRead() {
        port_.async_read_some(boost::asio::buffer(buffer_),
            [this](const boost::system::error_code& ec, size_t length) {
                if (!ec) {
                    std::cout << "Read: " << std::string(buffer_.data(), length);
                    startRead();  // 继续读取
                }
            });
    }
};
```

## 性能优化技巧

### 1. 使用多个 io_context 实例

```cpp
// 不同 I/O 类型使用不同的 io_context
class OptimizedServer {
    boost::asio::io_context network_io_;   // 网络 I/O
    boost::asio::io_context disk_io_;      // 磁盘 I/O
    boost::asio::io_context timer_io_;     // 定时器
    
public:
    void run() {
        std::thread t1([&]() { network_io_.run(); });
        std::thread t2([&]() { disk_io_.run(); });
        std::thread t3([&]() { timer_io_.run(); });
        // ...
    }
};
```

### 2. 使用 `make_strand` 避免数据竞争

```cpp
class ThreadSafeObject {
    boost::asio::io_context io_;
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    
public:
    ThreadSafeObject() 
        : strand_(boost::asio::make_strand(io_)) {}
    
    void asyncOperation() {
        // 保证回调在 strand 中串行执行
        boost::asio::post(strand_, [this]() {
            // 线程安全的代码
        });
    }
};
```

## 常见陷阱和注意事项

### 1. 忘记保持工作状态

```cpp
// ❌ 错误：run() 立即返回
boost::asio::io_context io;
boost::asio::steady_timer timer(io, std::chrono::seconds(1));
timer.async_wait([](auto& ec) {});
io.run();  // 立即返回，回调永远不会执行

// ✅ 正确：保持工作
auto work = std::make_shared<boost::asio::io_context::work>(io);
io.run();  // 阻塞
```

### 2. 回调中的异常处理

```cpp
// ❌ 错误：未捕获异常导致程序终止
io.post([]() {
    throw std::runtime_error("Error");
});
io.run();  // 异常会传播

// ✅ 正确：在回调中捕获异常
io.post([]() {
    try {
        throw std::runtime_error("Error");
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
});
```

## 总结

**`io_context` 的核心作用**：
- 事件循环引擎
- 异步操作调度器
- 回调函数执行器

**关键特性**：
- 线程安全（可以从多线程调用 `post`、`dispatch`）
- 可扩展（多线程运行）
- 灵活（可停止、重启）

**适用场景**：
- 网络编程（TCP/UDP）
- 定时任务
- 信号处理
- 串口/文件 I/O
- 任何异步操作

**性能建议**：
- 使用 `work` 对象保持运行
- 多线程运行 `run()` 提高吞吐量
- 使用 `strand` 避免数据竞争
- 合理设置线程数（通常 = CPU 核心数）