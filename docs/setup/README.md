# 给队友：Windows 11 安装与卸载

本包只包含模板库、跳板和自动化脚本，不安装 VS Code、编译器或插件，也不修改系统 PATH。

## 安装

1. 安装编译器与 VS Code, 并在当前 Profile 安装/启用 Microsoft 的 **C/C++ (`ms-vscode.cpptools`)**。用 CPH 跑题时另装 CPH; 本安装器不安装扩展或编译器。
2. 下载队友包并解压到固定、无空格的目录, 例如 `D:\zoi\HNIST-ZOI`。找到里面同时包含 `scripts` 和 `zoi` 的目录, 它才是库根; 不要在压缩包预览里运行。
3. 双击 `scripts\install-zoi.cmd`, 确认末尾 `[OK] installed` 或 `[OK] already installed and checked`。输出会列出实际修改的设置、任务与快捷键文件路径。出现 `[FAIL]` 不算安装成功。
4. 在 VS Code 用“文件 → 打开文件夹”打开自己的刷题目录, 重新加载窗口后运行 **zoi-configure** 一次。它会给该目录的每个 C/C++ 配置补上库路径, 避免项目 includePath 覆盖用户默认值。
5. 打开并保存题目的 `.cpp`, 写 `#include "bit.h"`, 使用下面的快捷入口展开。库目录和刷题目录、VS Code 安装目录无需相同, 也无需在同一磁盘。

安装器注册九个用户级任务：

| 任务 | 范围 |
|---|---|
| zoi-expand | 展开当前文件并复制；没有原默认构建任务时设为默认构建 |
| zoi-restore | 恢复当前文件，保留生成块外修改 |
| zoi-restore-all | 恢复当前工作区内的管理文件 |
| zoi-status | 只读查看当前文件状态 |
| zoi-forget | 保留当前代码，解除当前文件的展开管理 |
| zoi-configure | 配置当前刷题目录的 includePath、补全和错误提示; 需打开文件夹 |
| zoi-doctor | 显示配置、路径、编译器与常见覆盖项; 需打开文件夹 |
| zoi-package | 从库当前源码生成 ZIP, 输出到库的 docs/releases/ |
| zoi-clean-checks | 清理有完成标记的旧成功测试现场, 每类保留最近三份 |

**快捷入口:** `Ctrl+Alt+T` 打开过滤为 `zoi-` 的任务列表, `Ctrl+Alt+Z` 展开当前 C++ 文件。
已有同名快捷键会保留并显示提示, 可用 `Ctrl+P` 输入 `task zoi-` 替代。也可打开“任务: 运行任务”并选择任务名称;
右侧齿轮用于编辑配置, 不执行任务。`Ctrl+Shift+P` 的命令排序由 VS Code 管理, 本库不改命令历史或覆盖该快捷键。
`Ctrl+Shift+B` 只有在 zoi-expand 是默认构建任务时才用于展开, 不会抢占原有默认构建。

安装器配置用户级库路径、CPH 的 `-I` 参数, 启用 C/C++ 补全/错误提示/IntelliSense, 缺省 C++ 标准设为 C++20。
其他 include 路径、编译参数和任务保留, 但不会替自写编译任务或 Code Runner 添加参数;
这些编译命令仍需带 `-I<库目录>/zoi`。IntelliSense 标准头依赖实际 compilerPath, 不应把系统头目录逐一塞入 includePath。

zoi-configure 修改当前文件夹的 `.vscode/c_cpp_properties.json` 与 `.vscode/settings.json`, 保留已有 include、defines 和编译器;
仅缺省 compilerPath 时采用 PATH 中找到的 g++。configurationProvider/compile_commands.json 和 `.code-workspace` 的配置仍可能优先,
工具会提示, 不擅自关闭这些项目机制。它不扫描其他刷题目录。配置前快照留在 `.vscode/.zoi-workspace-state.json`。

CPH 直接按空格拆参数，不能靠加引号解决。库路径有空格时安装器尝试 Windows 短路径；如果磁盘未提供可用短路径，安装会在改配置前停止，移到无空格目录再试即可。

默认配置位置为 `%APPDATA%\Code\User`。便携版、Insiders 或自定义 Profile 可用：

```powershell
./scripts/install-zoi.ps1 -SettingsFile "实际用户目录/settings.json" -TasksFile "实际用户目录/tasks.json"
```

快捷键默认与 settings.json 放在同一目录, 可另传 `-KeybindingsFile`。当前 Profile 的位置可通过
“首选项: 打开用户设置 (JSON)”确认; 默认路径并不适用于所有便携版、Insiders、远程窗口或自定义 Profile。
卸载时使用同样的参数。安装后不要随意移动库目录；先撤销工作区配置和用户安装配置，再移动、重新安装。

重复安装会补回缺失的受管任务和设置, v2 安装记录自动升级为 v3, 保留最初配置快照。
没有安装记录但已有手写 zoi 任务时, 默认保留并报冲突; 确认这些任务属于当前库后可显式执行:

```powershell
./scripts/install-zoi.ps1 -AdoptExistingTasks
```

该选项只接管指向当前库相应脚本的已知任务, 拒绝其他命令/库路径, 卸载时恢复原手写任务。

## 常见故障

| 现象 | 检查或处理 |
|---|---|
| 克隆 GitHub 超时 | 由维护者生成队友 ZIP, 通过 QQ/微信等传文件, 安装无需 GitHub 连接 |
| Legacy install state | 新包已排除旧个人状态; 旧副本先保留记录, 确认来源再迁移, 不用盲删绕过 |
| 找不到 C/C++ 命令 | 在当前 Profile 启用 Microsoft C/C++; clangd 不读取 C_Cpp 设置 |
| 默认 includePath 有库, 仍报找不到 bit.h | 对当前刷题文件夹执行 zoi-configure; 确认真实 zoi/bit.h 存在 |
| iostream/vector 等标准头找不到 | 在 C/C++ 配置中指定本机真实 g++.exe, 不照搬维护者机器路径 |
| 补全/红线都消失 | 检查 C_Cpp.autocomplete/errorSquiggles/intelliSenseEngine 是否被用户、项目或语言专属配置禁用 |
| 运行任务只打开 tasks.json | 确认选了任务名称而非齿轮; JSON 文件不能包含聊天里的 Markdown 代码围栏 |
| Expected a source .cpp | 当前激活的是 tasks.json 等文件; 切回保存后的题目 .cpp 再执行 |
| expanded (... 0 blocks) | 本次没有模板引用; 检查磁盘上的已保存源码, 空文件/只有标准头不会插入模板 |

展开推荐 `#include "bit.h"`; 已知 zoi 跳板的 `<bit.h>` 也可展开, 普通 `<vector>` 等标准头保持不变。
运行成功会把展开结果复制到剪贴板; 编辑器红线属于 C/C++ 扩展, 与展开是否成功分开诊断。

## 卸载

若用过 zoi-configure, 先对每个配置过的刷题目录撤销局部配置:

```powershell
./scripts/configure-zoi.ps1 -Workspace "D:/practice" -Undo
```

配置文件在此后被手动改过时会保留并报出具体路径, 不覆盖新修改; 用户级卸载不扫描或删除其他项目。

双击 `scripts\uninstall-zoi.cmd`：

这是独立窗口入口，完成后关闭该命令提示符窗口；在已有终端操作请用下面的 `.ps1` 命令。

- 撤销本次安装的配置和任务，删除安装状态与临时文件，不留下 `.bak`。
- 配置没有后续修改时，恢复安装前的原文（包括注释、BOM 和换行）。后续增加的其他配置/任务保留; 精确匹配的受管快捷键移除, 改写过的快捷键保留。
- 对有本包清单且内容未改的队友包，接着删除解压目录。新增或改动过文件时保留整个库并报告原因，不删除题解或自改模板。
- 下载的 ZIP 位于你自己选择的位置，卸载器不猜测它在哪里；需要时可自行删除。

只卸载 VS Code 配置、保留库源码：

```powershell
./scripts/uninstall-zoi.ps1
```

Git 克隆或普通源码 ZIP 没有队友包归属清单，卸载只撤配置，不自动删仓库。安装记录缺失、属于另一份库、未知格式状态或冲突任务同名时会保留现场并报明原因，不能据此猜删原有数据。运行中断后重跑同一个命令；若中断后配置又被人为修改，则先保留现场处理冲突。

“清干净”指本工具创建的配置、状态、临时文件及未改动的受管解压目录，不包括 Windows/VS Code 自己记录的历史。

## 给维护者

```powershell
./scripts/check_setup.ps1
./scripts/check_deployment.ps1
./scripts/make_team_package.ps1
```

测试使用隔离配置，绝不卸载当前电脑的真实配置。也可直接运行 zoi-package, 默认生成
`docs/releases/HNIST-ZOI-时间戳.zip`, 输出实际绝对路径, 不依赖当前打开的文件。
打包器校验跳板和 catalog，收当前源码(含未提交修改)、文档、验证证据及压力脚本所需的 CI 配置，
不收 `.git`、个人安装状态、编译产物、测试日志、旧发布包或编辑器配置; 不携带 Git 历史, 不自动重新生成手册 PDF。
已有同名 ZIP 不覆盖, 可显式传 `-OutputPath`。正式目录见 [发布包说明](../releases/README.md)。

参考：[VS Code 用户任务](https://code.visualstudio.com/docs/debugtest/tasks#_user-level-tasks)、[CPH 参数解析](https://raw.githubusercontent.com/agrawal-d/cph/main/src/preferences.ts)。
