W15_0907_0911

# Ubuntu clash

Ubuntu 上所谓「两行命令一键开关」，指的是：**内核已经在 `127.0.0.1:7890` 跑着之后**，用环境变量告诉终端走不走代理。你这台机器上目前还没装 Clash/mihomo，所以要先把内核和订阅配置跑起来，那两行命令才会生效。

另外：这两行只影响当前终端里的 `curl` / `git` / `wget` 等。Firefox、Chrome 不会自动跟着走，浏览器要另设系统代理，或用带图形界面的客户端。

---

## 原理（和 Clash for Windows 的对应关系）

| Windows / macOS                 | Ubuntu                                 |
| ------------------------------- | -------------------------------------- |
| Clash for Windows / Clash Verge | 图形界面：Clash Verge Rev；纯命令行：mihomo       |
| 订阅配置                            | YAML（机场 Clash 订阅），常用名 `config.yaml`    |
| 混合端口 7890                       | `mixed-port: 7890`（HTTP + SOCKS5 同一个口） |
| 系统代理开关                          | 终端：`export` / `unset`；桌面：系统设置里的代理      |

`default.conf` 只要内容是 Clash YAML，都可以用，内核默认会找目录里的 `config.yaml`，复制或软链一下即可。

---

## 方案 A：桌面环境，最接近 Clash for Windows（推荐）

有图形桌面时，直接装 **Clash Verge Rev**，用法和 CFW 几乎一样：导入订阅 → 开系统代理 / TUN。

1. 从官方仓库下 Ubuntu 的 `.deb`：  
   https://github.com/clash-verge-rev/clash-verge-rev/releases  
   选 `Clash.Verge_*_amd64.deb`（ARM 机器选 `arm64`）。
2. 安装：

```bash
sudo apt install -y ./Clash.Verge_*_amd64.deb
```

3. 打开软件 → 导入和 Win/Mac 同一份 Clash 订阅 → 打开 **系统代理**（要像 CFW 那样接管浏览器，再开 **TUN**）。

这样 Google / YouTube 在浏览器里就能用，不必自己写那两行 `export`。

---

## 方案 B：纯命令行（你说的「两行命令」）

### 1. 安装 mihomo（原 Clash Meta）

原版 Clash 已停更，用 mihomo：

```bash
# 看架构：x86_64 用 amd64，aarch64 用 arm64
uname -m

# 到 https://github.com/MetaCubeX/mihomo/releases 下载对应 linux 包，例如：
# mihomo-linux-amd64-vX.Y.Z.gz

gunzip mihomo-linux-amd64-*.gz
chmod +x mihomo-linux-amd64-*
sudo mv mihomo-linux-amd64-* /usr/local/bin/mihomo
mihomo -v
```

### 2. 放配置并启动

```bash
mkdir -p ~/.config/clash
cp /path/to/default.conf ~/.config/clash/config.yaml
# 或从机场 Clash 订阅拉取：
# curl -L "你的订阅URL" -o ~/.config/clash/config.yaml
```

确认 YAML 里有（没有就补上）：

```yaml
mixed-port: 7890
allow-lan: false
mode: rule
```

启动（前台看日志，确认没报错）：

```bash
mihomo -d ~/.config/clash
```

看到监听 `7890` 就对了。要后台常驻可以用 systemd，或：

```bash
nohup mihomo -d ~/.config/clash >/tmp/mihomo.log 2>&1 &
```

### 3. 传说中的两行开关

**开墙（当前这个终端）：**

```bash
export https_proxy=http://127.0.0.1:7890 http_proxy=http://127.0.0.1:7890 all_proxy=socks5://127.0.0.1:7890
```

**关墙：**

```bash
unset https_proxy http_proxy all_proxy HTTPS_PROXY HTTP_PROXY ALL_PROXY
```

写进 `~/.bashrc`，以后输入 `proxy_on` / `proxy_off` 即可：

```bash
proxy_on() {
  export http_proxy=http://127.0.0.1:7890
  export https_proxy=http://127.0.0.1:7890
  export all_proxy=socks5://127.0.0.1:7890
  export HTTP_PROXY="$http_proxy" HTTPS_PROXY="$https_proxy" ALL_PROXY="$all_proxy"
  export no_proxy=localhost,127.0.0.1,::1
  echo "proxy on"
}

proxy_off() {
  unset http_proxy https_proxy all_proxy HTTP_PROXY HTTPS_PROXY ALL_PROXY
  echo "proxy off"
}
```

然后：

```bash
source ~/.bashrc
proxy_on    # 开
proxy_off   # 关
```

验证（不要用 `ping`，ICMP 不走代理）：

```bash
curl -I https://www.google.com
```

能返回 `HTTP/` 头就说明终端代理通了。

---

## 浏览器（Google / YouTube）怎么走代理

终端 `export` **不会**自动作用到 GUI 浏览器。任选一种：

1. **Clash Verge Rev**：打开系统代理或 TUN（最省事，接近 CFW）。
2. **GNOME 系统代理**：设置 → 网络 → 网络代理 → 手动  
   HTTP / HTTPS：`127.0.0.1` `7890`  
   SOCKS：`127.0.0.1` `7890`  
   关掉时改回「关闭」。
3. 浏览器自己填 HTTP 代理 `127.0.0.1:7890`。

---

## 常见坑

- 先确认内核在听：`ss -tlnp | grep 7890`。没进程，两行 `export` 也没用。
- 机场订阅要用 **Clash / Clash Meta** 格式，不要用 Surge / Quantumult 专用链接。
- Win/Mac 能用、Ubuntu 连不上，多半是协议太新（Hysteria2、Reality 等），必须用 mihomo，不要用老 Clash。
- `git` 的 SSH（`git@github.com`）不走 `http_proxy`，HTTPS 远程才走。
- 国内网站变慢，说明规则没分流，把 `mode` 设为 `rule`，不要用 `global`。

有图形桌面、主要想刷网页：装 Clash Verge Rev。只在终端里 `git`/`curl`/下载：mihomo + `proxy_on` / `proxy_off` 就够。 
