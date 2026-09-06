# 给队友：Windows 11 安装与卸载

本包只包含模板库、跳板和自动化脚本，不安装 VS Code、编译器或插件，也不修改系统 PATH。

## 安装

1. 下载队友包并解压，建议放到固定目录，例如 `C:\zoi\HNIST-ZOI`。不要直接在压缩包预览里运行。
2. 双击 `scripts\install-zoi.cmd`。无需管理员；使用 Win11 自带 Windows PowerShell 5.1。
3. 在 VS Code 保存题目后，运行任务；若任务列表未刷新，执行 Reload Window。

安装器注册五个用户级任务：

| 任务 | 范围 |
|---|---|
| zoi-expand | 展开当前文件并复制；没有原默认构建任务时设为默认构建 |
| zoi-restore | 恢复当前文件，保留生成块外修改 |
| zoi-restore-all | 恢复当前工作区内的管理文件 |
| zoi-status | 只读查看当前文件状态 |
| zoi-forget | 保留当前代码，解除当前文件的展开管理 |

同时配置 IntelliSense 的库路径和 CPH 的 `-I` 参数。原有其他 include 路径、编译参数和任务均保留。自己编写的编译任务仍需自行带 `-I<库目录>/zoi`；本安装器不重写你的编译命令。

CPH 直接按空格拆参数，不能靠加引号解决。库路径有空格时安装器尝试 Windows 短路径；如果磁盘未提供可用短路径，安装会在改配置前停止，移到无空格目录再试即可。

默认配置位置为 `%APPDATA%\Code\User`。便携版、Insiders 或自定义 Profile 可用：

```powershell
./scripts/install-zoi.ps1 -SettingsFile "实际用户目录/settings.json" -TasksFile "实际用户目录/tasks.json"
```

卸载时使用同样的参数。安装后不要随意移动库目录；先卸载配置，再移动、重新安装。

## 卸载

双击 `scripts\uninstall-zoi.cmd`：

这是独立窗口入口，完成后关闭该命令提示符窗口；在已有终端操作请用下面的 `.ps1` 命令。

- 撤销本次安装的配置和任务，删除安装状态与临时文件，不留下 `.bak`。
- 配置没有后续修改时，恢复安装前的原文（包括注释、BOM 和换行）。后续增加的其他配置/任务保留。
- 对有本包清单且内容未改的队友包，接着删除解压目录。新增或改动过文件时保留整个库并报告原因，不删除题解或自改模板。
- 下载的 ZIP 位于你自己选择的位置，卸载器不猜测它在哪里；需要时可自行删除。

只卸载 VS Code 配置、保留库源码：

```powershell
./scripts/uninstall-zoi.ps1
```

Git 克隆或普通源码 ZIP 没有队友包归属清单，卸载只撤配置，不自动删仓库。安装记录缺失、属于另一份库、旧格式状态或冲突任务同名时会保留现场并报明原因，不能据此猜删原有数据。运行中断后重跑同一个命令；若中断后配置又被人为修改，则先保留现场处理冲突。

“清干净”指本工具创建的配置、状态、临时文件及未改动的受管解压目录，不包括 Windows/VS Code 自己记录的历史。

## 给维护者

```powershell
./scripts/check_setup.ps1
./scripts/make_team_package.ps1 -OutputPath "D:/分享/HNIST-ZOI.zip"
```

测试使用隔离配置，绝不卸载当前电脑的真实配置。打包器校验跳板和 catalog，收源码、文档、验证证据及压力脚本所需的 CI 配置，不收 `.git`、个人安装状态、编译产物、测试日志或编辑器配置。

参考：[VS Code 用户任务](https://code.visualstudio.com/docs/debugtest/tasks#_user-level-tasks)、[CPH 参数解析](https://raw.githubusercontent.com/agrawal-d/cph/main/src/preferences.ts)。
