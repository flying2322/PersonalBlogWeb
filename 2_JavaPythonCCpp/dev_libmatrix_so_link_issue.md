

已修复 `matrix_install_v2` 一键部署中 SO 依赖未安装的问题。

## 根因

1. **`run_with_libs.sh` 并非安装脚本** — 只临时设置 `LD_LIBRARY_PATH` 并做 `ldd` 验证，**不会**把库复制到 `/usr/local/lib`，也**不会**执行 `ldconfig`。
2. **`matrix_engine_start.sh` 未设置 `LD_LIBRARY_PATH`** — Engine 启动时系统找不到这些 `.so`。
3. **`matrix_auto_install.sh` 未切到脚本目录** — 相对路径 `matrix_libs_pack/...`、`app/` 可能因执行目录不对而失败。

`matrix_libs_pack/libs/` 里其实已有全部依赖，只是从未被安装到系统。

---

## 修改内容

| 文件                                                       | 改动                                                                                                 |
| -------------------------------------------------------- | -------------------------------------------------------------------------------------------------- |
| **`matrix_libs_pack/install_libs.sh`**（新增）               | 将 `libs/*` 复制到 `/usr/local/lib`，执行 `ldconfig`，校验 5 个关键库                                            |
| **`matrix_auto_install.sh`**                             | 固定 `SCRIPT_DIR`；调用 `install_libs.sh` 替代 `run_with_libs.sh`；路径全部改为绝对路径；增加 `systemctl daemon-reload` |
| **`config/matrix_engine_start.sh`**                      | 增加 `LD_LIBRARY_PATH=/usr/local/lib:/opt/sinevaAGV/matrix_engine`                                   |
| **`config/matrix_engine.service`**                       | 增加 `Environment=LD_LIBRARY_PATH=...`                                                               |
| **`run_with_libs.sh`**                                   | 改为安装后验证脚本                                                                                          |
| **`readme.txt` / `环境配置文档.md` / `packet_matrix_deps.sh`** | 同步说明                                                                                               |
| **`missingso/`**                                         | 已删除（定位用目录）                                                                                         |

---

## 在新服务器上使用

若已跑过旧版安装脚本，只需补装依赖并更新启动配置：

```bash
cd /path/to/matrix_install_v2
sudo bash matrix_libs_pack/install_libs.sh
sudo cp config/matrix_engine_start.sh /etc/systemd/system/
sudo cp config/matrix_engine.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo service matrix_engine restart

# 验证
ldd /opt/sinevaAGV/matrix_engine/libmatrix_core.so | grep "not found"
# 应无输出
```

全新部署仍用：

```bash
sudo bash matrix_auto_install.sh
```

安装完成后可通过 DeployKit 升级 `matrix_engine`，无需再手动拷贝 `missingso` 里的文件。





**不完全是。** `install_libs.sh` 解决的是 **运行 `libmatrix_core.so` 所需的运行时动态库**，不是 **编译 Matrix-Core C++ 代码所需的完整开发环境**。

## `install_libs.sh` 实际做了什么

```bash
sudo bash matrix_install_v2/matrix_libs_pack/install_libs.sh
```

它会：

1. 把 `matrix_libs_pack/libs/` 里已打包好的 `.so` 复制到 `/usr/local/lib/`
2. 执行 `ldconfig`
3. 校验 mysql、geos、modbus、protobuf、minizip 等 **运行时** 库是否可用

这些库来自 `packet_matrix_deps.sh` 对 **已编译好的** `libmatrix_core.so` 做 `ldd` 打包，对应的是 **部署/运行 Engine** 时的依赖，和你在服务器上手动拷 `missingso` 是同一类问题。

---

## 它不能替代什么（C++ 开发依赖）

要在本机 **编译** Matrix-Core，还需要例如：

| 类型    | 示例                                                    | `install_libs.sh` 是否提供 |
| ----- | ----------------------------------------------------- | ---------------------- |
| 头文件   | `geos`、`protobuf`、`modbus` 的 `.h`                     | 否                      |
| dev 包 | `libgeos-dev`、`libprotobuf-dev`、`libmysqlcppconn-dev` | 否                      |
| 构建工具  | cmake、g++、boost 开发包                                   | 否                      |
| 链接期库  | `.a` / `.so` 的 dev 符号链接                               | 部分仅有运行时 `.so`，无 dev 包  |

`core/README.md` 里列的是 **开发环境** 安装方式（boost、websocketpp、mysql-connector、zlib/minizip 等 **-dev** 包），和 `install_libs.sh` 用途不同。

---

## 两种场景对照

| 场景                                          | 用什么                                                             |
| ------------------------------------------- | --------------------------------------------------------------- |
| **新服务器部署**，让 Engine 能加载 `libmatrix_core.so` | `matrix_auto_install.sh` 或单独 `install_libs.sh`                  |
| **本机开发/编译** Matrix-Core                     | apt 装 dev 包 + 按 `core/README.md` / 你们现有的 `DEP_INSTALL_DIR` 环境配置 |

---

## 小结

- **`install_libs.sh`**：部署机上 **运行** Core 的 runtime `.so` 一键安装（你之前缺的那 5 个库就是这类）。
- **不能** 单靠它完成 Matrix-Core 的 **C++ 编译开发环境**；编译仍要装 dev 包和构建依赖。

若你希望 **在 `matrix_install_v2` 里再加一个 `install_dev_deps.sh`**，专门用 apt 装编译 Core 所需的 dev 包，可以说一下目标系统（Ubuntu 版本）和是否已有 `DEP_INSTALL_DIR`，我可以按项目 CMake 依赖帮你补一份脚本。







已更新 `core/README.md`，可在全新 **Ubuntu 20.04** 上按文档从零搭建 Matrix-Core 编译环境。要点如下。

## 文档结构

1. **开发 vs 运行**：区分 apt `-dev` 包（编译）与 `/usr/local/lib` 运行时 `.so`（部署/ldd）
2. **依赖对照表**：按 `core/CMakeLists.txt` 列出 CMake 依赖及安装方式
3. **apt 一键安装**：build-essential、cmake、Boost、GEOS、MySQL、modbus、OpenSSL、websocketpp、Catch2 等
4. **`/usr/local/lib` 运行时库**：对应此前 5 个 `not found` 的库，通过 `matrix_install_v2/matrix_libs_pack/install_libs.sh` 或从正常服务器拷贝
5. **Protobuf 专项（Ubuntu 20.04 必做）**：apt 仅 3.6.x，而 `main.pb.h` 要求 ≥ 3.9；文档给出拷贝或源码安装 3.20.x 两种方案
6. **编译步骤**：`cmake .. -DDEP_INSTALL_DIR=/opt/sinevaAGV` + `cmake --build . --target matrix_core`
7. **验证清单与 FAQ**：含 `install_libs.sh` 不能替代 dev 环境的说明

## 关键结论

| 问题                         | 原因                      | README 中的处理          |
| -------------------------- | ----------------------- | -------------------- |
| 5 个 `.so` not found        | 缺 `/usr/local/lib` 运行时库 | 第三节                  |
| Protobuf 编译报错              | Ubuntu 20.04 apt 版本过旧   | 第四节（单独说明，未放入 apt 列表） |
| 只跑 `install_libs.sh` 仍无法编译 | 只有 `.so`，没有头文件/dev 包    | 文首对比表 + FAQ          |

仓库内 `spdlog`、`nlohmann/json` 等已在 `core/third_party/`，文档中已注明无需安装。

若你希望把 `libs/` 预打包进仓库，或补充 Ubuntu 20.04 上 GEOS pkg-config 的 workaround，可以继续说明需求。
