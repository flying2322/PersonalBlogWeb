# 简介
"Akka" 是一个开源的分布式计算框架，基于 Actor 模型并使用 Scala 和 Java 编写。它提供了一种构建高并发、分布式、可扩展系统的方式。

在 Akka 中，核心的概念是 Actor，它是一个轻量级的并发实体，可以处理消息、执行计算、管理状态，以及与其他 Actor 进行通信。这使得开发者能够以更容易理解和管理的方式处理并发和分布式问题。

下面是一个简单的 Akka Actor 示例（使用 Scala）：
```scala
import akka.actor.{Actor, ActorSystem, Props}

// 定义一个简单的 Actor
class MyActor extends Actor {
  def receive: Receive = {
    case "Hello" => println("Hello back at you!")
    case _       => println("Unknown message")
  }
}

object AkkaExample extends App {
  // 创建 Actor 系统
  val system = ActorSystem("MySystem")

  // 创建 MyActor 实例
  val myActor = system.actorOf(Props[MyActor], name = "myActor")

  // 发送消息给 MyActor
  myActor ! "Hello"

  // 关闭 Actor 系统
  system.terminate()
}
```

在这个例子中，MyActor 是一个简单的 Actor，它能够处理 "Hello" 消息并回应。AkkaExample 创建了一个 Actor 系统，创建了一个 MyActor 实例，然后向它发送了一条消息。

请注意，使用 Akka 可能需要引入相关的库和依赖，这些代码只是一个简单的示例，具体的用法会根据你的应用场景而有所不同。如果你有特定的问题或用例，欢迎提出，我将尽力提供帮助。

## 可以给一段用java调用akka actor来实现和某个设备通信的例子吗？
当使用 Akka 进行设备通信时，通常会创建一个 Actor 代表设备，并使用消息传递的方式进行通信。以下是一个简单的示例，展示如何使用 Java 调用 Akka Actor 与设备通信：

首先，确保你的项目中包含 Akka 库的依赖。这可能包括 Akka Actor、Akka Remote 等，具体取决于你的需求。

然后，创建一个设备 Actor 类，例如：

```java
import akka.actor.AbstractActor;
import akka.actor.Props;

public class DeviceActor extends AbstractActor {
    // 设备状态
    private String deviceStatus = "Off";

    // 设备 Actor 的 Props 工厂方法
    static Props props() {
        return Props.create(DeviceActor.class);
    }

    // 处理消息的方法
    @Override
    public Receive createReceive() {
        return receiveBuilder()
                .match(TurnOn.class, this::turnOn)
                .match(TurnOff.class, this::turnOff)
                .match(GetStatus.class, this::getStatus)
                .build();
    }

    // 模拟设备开启操作
    private void turnOn(TurnOn turnOn) {
        deviceStatus = "On";
        System.out.println("Device is turned on.");
    }

    // 模拟设备关闭操作
    private void turnOff(TurnOff turnOff) {
        deviceStatus = "Off";
        System.out.println("Device is turned off.");
    }

    // 获取设备状态
    private void getStatus(GetStatus getStatus) {
        getSender().tell(deviceStatus, getSelf());
    }

    // 消息类：设备开启
    public static class TurnOn {
    }

    // 消息类：设备关闭
    public static class TurnOff {
    }

    // 消息类：获取设备状态
    public static class GetStatus {
    }
}
```
接下来，创建一个主类来演示如何使用 Java 调用这个设备 Actor：

```java
import akka.actor.ActorRef;
import akka.actor.ActorSystem;

public class Main {

    public static void main(String[] args) {
        // 创建 Actor 系统
        ActorSystem system = ActorSystem.create("DeviceSystem");

        // 创建设备 Actor
        ActorRef deviceActor = system.actorOf(DeviceActor.props(), "deviceActor");

        // 发送消息给设备 Actor
        deviceActor.tell(new DeviceActor.TurnOn(), ActorRef.noSender());
        deviceActor.tell(new DeviceActor.GetStatus(), ActorRef.noSender());
        deviceActor.tell(new DeviceActor.TurnOff(), ActorRef.noSender());
        deviceActor.tell(new DeviceActor.GetStatus(), ActorRef.noSender());

        // 关闭 Actor 系统
        system.terminate();
    }
}
```
在这个例子中，DeviceActor 表示一个简单的设备，可以接收开启、关闭和获取状态的消息。在 Main 类中，我们创建了一个 Akka Actor 系统，然后通过 tell 方法向设备 Actor 发送消息。

这只是一个简单的示例，具体的实现可能会根据你的设备通信协议和需求有所不同。希望这个例子能帮助你入门 Akka Actor 在设备通信中的使用。

# java typesafe.config.Config Config
在 Java 中使用 Typesafe Config 库，你需要导入相应的包，然后创建一个 `Config` 对象。以下是一个简单的示例：

首先，确保你的项目中包含了 Typesafe Config 库的依赖。如果你使用 Maven，可以在 `pom.xml` 文件中添加如下依赖：

```xml
<dependencies>
    <!-- Other dependencies -->
    <dependency>
        <groupId>com.typesafe</groupId>
        <artifactId>config</artifactId>
        <version>1.4.1</version> <!-- 使用最新版本 -->
    </dependency>
</dependencies>
```

如果你使用 Gradle，可以在 `build.gradle` 文件中添加如下依赖：

```groovy
dependencies {
    // Other dependencies
    implementation 'com.typesafe:config:1.4.1' // 使用最新版本
}
```

接下来，你可以在 Java 代码中使用 Typesafe Config。以下是一个简单的示例：

```java
import com.typesafe.config.Config;
import com.typesafe.config.ConfigFactory;

public class ConfigExample {

    public static void main(String[] args) {
        // 加载默认配置文件（application.conf）
        Config config = ConfigFactory.load();

        // 从配置文件中获取值
        String appName = config.getString("app.name");
        int maxConnections = config.getInt("app.maxConnections");

        // 打印配置值
        System.out.println("Application Name: " + appName);
        System.out.println("Max Connections: " + maxConnections);
    }
}
```

上述代码假设你的项目中有一个默认的 Typesafe Config 配置文件，通常命名为 `application.conf`。你可以在这个配置文件中设置键值对，然后通过 `Config` 对象获取这些值。

示例的 `application.conf` 文件内容可能如下：

```conf
app {
  name = "MyApp"
  maxConnections = 100
}
```

在这个例子中，`ConfigExample` 类加载默认的配置文件，并获取了 `app.name` 和 `app.maxConnections` 的值。

请确保你的项目结构和依赖配置正确，以便成功使用 Typesafe Config。

# docker compose confign
```docker
version: '3'
services:
    nova-ess-p:
            #image: 172.20.8.203/hairounova/nova-ess-p:4.0.0.20
        image: 172.20.8.203/hairounova-test/nova-ess-p:merge_23.3.1.0_230918173810
        container_name: nova-ess-p
        network_mode: bridge
        restart: always
        extra_hosts:
            - "nova-iwms-core:172.18.81.106"
            - "nova-iwms-station:172.18.81.106"
            - "nova-iwms-adapter:172.18.81.106"

            - "nova-ess-p:172.18.81.106"
            - "nova-algo-mc:172.18.81.106"
            - "nova-algo-pp:172.18.81.106"
            - "nova-algo-charging:172.18.81.106"
            - "nova-linklayer:172.18.81.106"
            - "nova-datax-gateway:172.18.81.106"

            - "nova-kafka:172.18.81.106"
            - "nova-redis:172.18.81.106"

            - "nova-algo:172.18.81.106"
            - "mc.algo.hr.com:172.18.81.106"
            - "pp.algo.hr.com:172.18.81.106"
            - "charge.algo.hr.com:172.18.81.106"
            - "op.algo.hr.com:172.18.81.106"

        environment:
            - TZ=Asia/Shanghai
            - JAVA_OPTS= -javaagent:/agent/normandy-agent.jar -Dapp.name=nova-ess-p -Dreport.step=5 -Dpushgateway.address=172.18.81.106:5299 -Dcom.sun.management.jmxremote -Djava.rmi.server.hostname=172.18.81.106 -Dcom.sun.management.jmxremote.port=4765 -Dcom.sun.management.jmxremote.rmi.port=4765 -Dcom.sun.management.jmxremote.authenticate=false -Dcom.sun.management.jmxremote.ssl=false  -Xms4G -Xmx4G -XX:NewRatio=1 -XX:SurvivorRatio=6 -XX:MetaspaceSize=512m -XX:MaxMetaspaceSize=512m -XX:MaxDirectMemorySize=1g -XX:+DisableExplicitGC -XX:+UseConcMarkSweepGC -XX:CMSMaxAbortablePrecleanTime=5000 -XX:+CMSClassUnloadingEnabled -XX:CMSInitiatingOccupancyFraction=80 -XX:+UseCMSInitiatingOccupancyOnly -XX:+ExplicitGCInvokesConcurrent -Dsun.rmi.dgc.client.gcInterval=72000000 -XX:ParallelGCThreads=4
            - ESS_REDIS_IP=nova-redis
            - ESS_REDIS_PORT=6379
            - KAFKA_SERVERS=nova-kafka:9092
            - ESS_EVENT_DIR=/hairou/data/
            #- PLAY_HTTP_ROUTER=facade.Routes
            #- ESS_LL_ENABLE=on
            - ESS_SLAVE=off
            - ESS_EVENT_BACKUP=off

            - PERSIST_LOCAL_HOST=127.0.0.1
            - PERSIST_LOCAL_PORT=9901

            - PERSIST_REMOTE_HOST1=127.0.0.1
            - PERSIST_REMOTE_PORT1=9901


            # 生产环境
            #- ESS_TEST_ONLY=on
            #- MOCK_SCAN_RESULT=on
            #- ESS_LL_PRODUCER=com.hairoutech.ess.actor.robot.kubot.comm.EssLinkLayerProducerActor

            # 测试环境
            - ESS_TEST_ONLY=on
            - MOCK_SCAN_RESULT=on
            - ESS_LL_PRODUCER=com.hairoutech.ess.actor.robot.kubot.mock.EssMockLlActor$$CommandRoute

            - SWAGGER_UI_DIR=/hairou/public/swagger-ui/
            - CONVEYOR_TYPE=modbus
            - WMS_CORE_URL=http://nova-iwms-core
            - WMS_STATION_URL=http://nova-iwms-station

            - ESS_SECURITY=off

        ports:
            - 9000:9000
            - 9901:9901
            # JMX 使用，请勿修改
            - 4765:4765
            - 8886:8886
            - 8888:8888
            - 9877:9877

        healthcheck:
          test: ["CMD", "curl", "-f", "http://172.18.81.106:9000/actor/query"]
          interval: 2m
          timeout: 10s
          retries: 3

        entrypoint: sh -c 'rm  -f /hairou/RUNNING_PID;exec ess-gateway'

        volumes:
            - /etc/localtime:/etc/localtime
            - /usr/share/zoneinfo:/usr/share/zoneinfo
            - /data/ess_data/data/:/hairou/data
            - /data/ess_data/map/:/hairou/map
            - /data/ess_data/ptl/:/hairou/ptl
            - ./key/:/hairou/key
            - /haiq_logs/nova-ess-p/:/hairou/logs
            - /data/app/normandy-5101/lib/normandy-agent.jar:/agent/normandy-agent.jar

```

# JAVA_OPTS

这是一个Java应用程序的启动配置，通过设置`JAVA_OPTS`环境变量来传递。下面是这个配置中各个参数的含义：

1. **`-javaagent:/agent/normandy-agent.jar`**:
   - 启用Java代理，用于监控和收集性能数据。

2. **`-Dapp.name=nova-ess-p`**:
   - 设置应用程序的名称为"nova-ess-p"，这个信息可能会用于标识不同的应用实例。

3. **`-Dreport.step=5`**:
   - 设置报告的步长为5，具体含义可能与应用程序的业务逻辑相关。

4. **`-Dpushgateway.address=172.18.81.106:5299`**:
   - 指定Pushgateway的地址和端口，可能用于将监控数据推送到Prometheus等监控系统。

5. **JMX设置**:
   - `-Dcom.sun.management.jmxremote`: 启用JMX。
   - `-Djava.rmi.server.hostname=172.18.81.106`: 指定RMI服务的主机名。
   - `-Dcom.sun.management.jmxremote.port=4765`: 指定JMX远程端口。
   - `-Dcom.sun.management.jmxremote.rmi.port=4765`: 指定RMI端口。
   - `-Dcom.sun.management.jmxremote.authenticate=false`: 禁用JMX身份验证。
   - `-Dcom.sun.management.jmxremote.ssl=false`: 禁用JMX的SSL。

6. **堆内存配置**:
   - `-Xms4G`: 设置初始堆大小为4GB。
   - `-Xmx4G`: 设置最大堆大小为4GB。
   - `-XX:NewRatio=1`: 设置新生代与老年代的堆大小比例。

7. **垃圾回收相关配置**:
   - `-XX:+DisableExplicitGC`: 禁用显式的垃圾回收。
   - `-XX:+UseConcMarkSweepGC`: 使用CMS（Concurrent Mark-Sweep）垃圾回收器。
   - `-XX:CMSMaxAbortablePrecleanTime=5000`: 设置CMS回收器最大可中止的预清理时间。
   - `-XX:+CMSClassUnloadingEnabled`: 启用CMS回收器中类的卸载。
   - `-XX:CMSInitiatingOccupancyFraction=80`: CMS触发垃圾回收的阈值。
   - `-XX:+UseCMSInitiatingOccupancyOnly`: 仅在达到CMS触发阈值时执行垃圾回收。

8. **其他配置**:
   - `-XX:+ExplicitGCInvokesConcurrent`: 显式的GC调用并行执行。
   - `-Dsun.rmi.dgc.client.gcInterval=72000000`: 指定远程对象的垃圾回收间隔时间。
   - `-XX:ParallelGCThreads=4`: 设置并行垃圾回收的线程数。

总体而言，这个配置文件包含了一系列的JVM参数，用于调整Java应用程序的性能、内存管理和监控设置。配置中的具体数值可能需要根据应用程序的需求进行调整。


# setiprestart.sh /data/workspace/
```shell
#!/bin/bash
cur_dir=$(cd $(dirname $0);pwd)
IP=$1
normandy=$2
name=`hostname` 
cat > /tmp/daemon.json << "EOFFF"
{
    "log-opts": {
       "max-size": "50m",
       "max-file": "5"
    },
    "registry-mirrors": ["https://zvqzjr1b.mirror.aliyuncs.com"],
    "insecure-registries":["172.20.8.205:5000","172.20.8.203","172.18.81.65:5000"]
}
EOFFF
echo hairou|sudo -S mv /tmp/daemon.json /etc/docker
echo hairou|sudo -S systemctl restart docker
rm -f /data/workspace/nginx-www/www/index.html
echo hairou|sudo -S sed -i -e 's/export HOST_IP=.*/export HOST_IP='$IP'/g' /etc/profile
echo hairou|sudo -S /bin/bash -c 'echo "nameserver 114.114.114.114" >> /etc/resolv.conf;chown -R hairou:hairou ess_data/ imhs_data/'
mysql -uroot -proot -e "create user nacos@'"${IP}"' identified  with mysql_native_password  by 'NAC0S@mysql';"
mysql -uroot -proot -e "grant all on nova_config.* to nacos@'"${IP}"'"
sed -i 's/172.18.81.70/'$IP'/g' /data/app/normandy-5101/conf/project_config.json
source /etc/profile
cd /data/app
wget http://172.20.8.10:6205/haiq-devops/$normandy
unzip -o $normandy
chmod +x /data/app/normandy-5101/bin/*
echo hairou|sudo -S chown hairou:hairou /haiq_logs/
/data/app/normandy-5101/bin/server.sh restart
```

