**Ubuntu 下 .deb 包 安装/查询/卸载 速查表**（你前面问的卸载也在里面）：

---

## 一、安装 deb（a.deb 为例）

### 1）推荐：用 apt（自动处理依赖）
```bash
cd 存放a.deb的目录
sudo apt install ./a.deb
```
（一定要加 `./`，表示当前目录）

### 2）底层：用 dpkg（不解决依赖）
```bash
sudo dpkg -i a.deb
```
如果报依赖错误，执行：
```bash
sudo apt -f install
```

---

## 二、查看 deb 信息 / 已安装包名

### 1）看 a.deb 里面真实包名（卸载要用）
```bash
dpkg -I a.deb | grep Package
```
输出类似：`Package: myapp` → 卸载用 `myapp`

### 2）列出系统里所有已安装包（找名字）
```bash
dpkg -l | grep -i myapp
```

---

## 三、卸载（重点）

### 1）普通卸载（保留配置）
```bash
sudo apt remove myapp
# 或
sudo dpkg -r myapp
```

### 2）彻底卸载（删程序+删配置，推荐）
```bash
sudo apt purge myapp
# 或
sudo dpkg -r --purge myapp
```

### 3）卸载后清理无用依赖（好习惯）
```bash
sudo apt autoremove
```

---

## 四、常用操作汇总（一行版）
```bash
# 安装
sudo apt install ./a.deb

# 查包名
dpkg -I a.deb | grep Package

# 卸载（彻底）
sudo apt purge 包名

# 清理依赖
sudo apt autoremove
```

---
