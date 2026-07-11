已在 `matrix_install_deploy/matrix_auto_uninstall.sh` 添加一键卸载脚本，与 `matrix_auto_install.sh` 对应。

**用法：**

```bash
cd matrix_install_deploy
sudo bash matrix_auto_uninstall.sh          # 交互确认
sudo bash matrix_auto_uninstall.sh --yes    # 跳过确认
```

**卸载内容：**

| 类别      | 内容                                                                                 |
| ------- | ---------------------------------------------------------------------------------- |
| 服务      | 停止并禁用 `matrix_engine`、`matrix_deploykit`，结束相关进程                                    |
| systemd | 删除 4 个 unit/启动脚本                                                                   |
| 目录      | `/opt/sinevaAGV`、`/etc/sinevaAGV`、`/var/log/sinevaAGV`                             |
| Nginx   | 删除 `nginx_matrix.conf`，若有备份则恢复 `nginx.conf.bak`                                    |
| 运行时库    | 删除 `/usr/local/lib` 下 Matrix 安装的 7 个 `.so` 并执行 `ldconfig`                          |
| MySQL   | 删除 `sineva_matrix` 库；purge 所有 mysql/mariadb 包；删除 `/var/lib/mysql`、`/etc/mysql`、日志等 |
| apt 包   | `mysql-server`、`dotnet-sdk-6.0`、`nginx`、`net-tools`、`vim`、`curl`、`chrony` 等        |

**说明：** 不会卸载 `openssh-server`，避免远程 SSH 断开。重装前执行卸载脚本，再跑 `sudo bash matrix_auto_install.sh` 即可。


