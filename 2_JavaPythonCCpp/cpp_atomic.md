```cpp
/// @brief 多线程交互变量
struct Signal {
    std::atomic_bool chargingConfigUpdate;                  // 回充配置更新
    std::atomic_bool leisureConfigUpdate;                   // 闲置配置更新
    std::atomic_bool trafficConfigUpdate;                   // 交管配置更新
    std::atomic_bool orderConfigUpdate;                     // 运单配置更新
    std::atomic_bool trafficFlowUpdate;                     // 交通流更新
    std::atomic_bool sceneUpdate;                           // 场景更新
    std::atomic_bool trafficLimitAreaUpdate;                // 交管限制区配置更新
    std::atomic_bool trafficMutexAreaUpdate;                // 交管互斥区配置更新
    std::atomic_bool trafficAutoDoorAreaUpdate;             // 交管自动门区域配置更新
    std::atomic_bool trafficAirShowerAreaUpdate;            // 交管风淋门区域配置更新
    std::atomic_bool trafficElevatordoorAreaUpdate;         // 交管电梯门区域配置更新

    /// @brief 初始化数据
    void init() {
        chargingConfigUpdate.store(false);
        leisureConfigUpdate.store(false);
        trafficConfigUpdate.store(false);
        orderConfigUpdate.store(false);
        trafficFlowUpdate.store(false);
        sceneUpdate.store(false);
        trafficLimitAreaUpdate.store(false);
        trafficMutexAreaUpdate.store(false);
        trafficAutoDoorAreaUpdate.store(false);
        trafficAirShowerAreaUpdate.store(false);
        trafficElevatordoorAreaUpdate.store(false);
    }
};

// two main func: load() for read and store() for write
```

`Signal` 结构体定义了一组用于多线程交互的标志位，每个成员都是 `std::atomic_bool`，用于在不同线程之间安全地传递“某个配置是否需要更新”的状态。成员变量对应的更新类型有回充配置、闲置配置、交管配置、运单配置、交通流、场景，以及各种交管区域配置。

这些标志都以原子类型保存，避免了多线程访问时的数据竞争。`init()` 方法用于将所有标志初始化为 `false`，也就是说在开始运行时，所有更新请求都处于未触发状态。