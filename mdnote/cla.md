##   
## 🧱** Clash for Linux（Headless）完整配置与使用笔记**  
  
适用于：国内服务器 / 无 GUI Linux / 通过 SSH 使用  
测试环境：Ubuntu 20.04 + clash-linux-amd64  
  
⸻  
  
1️⃣** Clash 是什么？在服务器上它扮演什么角色**  
  
Clash 本质是一个 **本地代理转发与规则引擎**：  
  
```
程序 / 命令行工具
        ↓
   HTTP / SOCKS5 代理
        ↓
      Clash
        ↓
   机场节点（SS / VMess / Trojan 等）
        ↓
     墙外互联网

```
  
在服务器上：  
	•	Clash **监听一个本地端口**（如 7890）  
	•	所有需要翻墙的流量 **显式或隐式** 走这个端口  
  
⸻  
  
2️⃣** Clash 运行所需的最小文件结构**  
  
```
~/clash/
├── clash-linux-amd64    # 主程序
├── config.yaml          # 核心配置文件（必须）
├── Country.mmdb         # IP 地理库（自动下载）

```
  
启动命令：  
  
```
cd ~/clash
chmod +x clash-linux-amd64
./clash -d .

```
  
-d . 表示：  
	•	使用当前目录作为工作目录  
	•	自动加载 config.yaml  
  
⸻  
  
3️⃣** config.yaml 的整体结构（先有全局观）**  
  
```
# ① 基础端口与运行模式
port:
socks-port:
mixed-port:
mode:

# ② DNS（可选但推荐）
dns:

# ③ 节点定义
proxies:

# ④ 节点分组
proxy-groups:

# ⑤ 规则系统
rules:

```
  
👉 **顺序不严格，但逻辑是：**  
  
端口 → DNS → 节点 → 怎么选节点 → 哪些流量走代理  
  
⸻  
  
4️⃣** 端口配置（最基础但最重要）**  
  
```
mixed-port: 7890
allow-lan: false
bind-address: '*'
mode: rule

```
log-level: info  
  
各字段解释  
  
**字段**	**作用**  
mixed-port	同时支持 HTTP + SOCKS5（最省事）  
allow-lan	是否允许局域网访问（服务器一般关）  
mode	rule / global / direct  
log-level	日志等级  
  
📌 **为什么用 mixed-port？**  
	•	curl / wget / git / apt 都能用  
	•	不用区分 HTTP / SOCKS  
  
⸻  
  
5️⃣** DNS 配置（不配可能“能连但慢/怪”）**  
  
```
dns:
  enable: true
  listen: 0.0.0.0:1053
  enhanced-mode: fake-ip
  nameserver:
    - 223.5.5.5
    - 119.29.29.29
  fallback:
    - 8.8.8.8

```
    - 1.1.1.1  
  
为什么需要** DNS？**  
	•	防止 **DNS 污染**  
	•	提升海外站点解析成功率  
	•	规则匹配依赖域名  
  
⸻  
  
6️⃣** proxies：节点定义（机场提供 or 订阅生成）**  
  
示例（手写 SS 节点）：  
  
```
proxies:
  - name: "US-01"
    type: ss
    server: x.x.x.x
    port: 8388
    cipher: aes-256-gcm

```
    password: "password"  
  
📌 实际使用中：  
	•	**一般不手写**  
	•	来自：  
	•	机场订阅转换  
	•	provider（自动更新）  
  
⸻  
  
7️⃣** proxy-groups：节点选择逻辑（核心）**  
  
```
proxy-groups:
  - name: "自动选择"
    type: url-test
    proxies:
      - US-01
      - JP-01
    url: http://www.gstatic.com/generate_204
    interval: 300

  - name: "手动选择"
    type: select
    proxies:
      - 自动选择
      - US-01

```
      - JP-01  
  
常见类型说明  
  
**类型**	**用途**  
select	手动选择  
url-test	自动测速  
fallback	主挂了换  
load-balance	负载均衡  
  
  
⸻  
  
8️⃣** rules：决定「哪些流量走代理」**  
  
最小可用规则：  
  
```
rules:
  - DOMAIN-SUFFIX,google.com,自动选择
  - DOMAIN-SUFFIX,github.com,自动选择
  - GEOIP,CN,DIRECT

```
  - MATCH,自动选择  
  
规则匹配顺序  
  
**从上到下，第一个命中就停止**  
  
📌 含义：  
	•	国内直连  
	•	国外默认走代理  
	•	可精细到单域名  
  
⸻  
  
9️⃣** 验证 Clash 是否真的生效**  
  
1️⃣** Clash 启动成功标志**  
  
```
RESTful API listening at: 127.0.0.1:9090
Mixed(http+socks5) proxy listening at: :7890

```
  
  
⸻  
  
2️⃣** 命令行测试（推荐）**  
  
```
curl --proxy http://127.0.0.1:7890 https://www.google.com

```
  
返回：  
	•	HTTP 200  
	•	server: gws  
✅ 翻墙成功  
  
⸻  
  
🔟** 让系统工具走代理（不是写进 config.yaml）**  
  
Clash **只提供代理，不自动接管系统**  
  
临时方式（推荐）  
  
```
export http_proxy=http://127.0.0.1:7890
export https_proxy=http://127.0.0.1:7890

```
  
验证  
  
```
git clone https://github.com/torvalds/linux

```
  
  
⸻  
  
1️⃣1️⃣** 常见坑总结（血泪版）**  
  
**问题**	**原因**  
YAML 报错	缩进 / 冒号后没空格  
能启动但不上网	没走代理  
GitHub 慢	DNS / 节点质量  
订阅失效	节点过期  
  
  
⸻  
  
1️⃣2️⃣** 推荐你下一步补充的高级内容（可选）**  
	•	provider 订阅自动更新  
	•	systemd 后台服务  
	•	iptables 全局透明代理  
	•	rule-set 外置规则  
  
⸻  
  
✅** 你现在拥有的是什么能力？**  
  
✔️ 在任何一台国内服务器  
✔️ 不依赖 GUI  
✔️ 不依赖 Shadowrocket  
✔️ 用一份配置复现翻墙能力  

This is a Test