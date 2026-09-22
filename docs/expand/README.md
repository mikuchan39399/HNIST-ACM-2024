# 跳板、展开与恢复

先按 [安装说明](../setup/README.md) 注册任务。写题时 include 所需跳板，保存当前文件后，在 VS Code 运行：

推荐 `#include "bit.h"`, 已知 zoi 跳板的 `<bit.h>` 也支持; 普通 `<vector>` 等标准头不展开。
展开前会按源码稳定短名同步库内路径；自己移动母版仍可继续使用原跳板，详见[目录同步](../maintenance/layout.md)。恢复、状态查询和解除管理不执行目录迁移。
`Ctrl+Alt+Z` 直接展开当前 C++ 文件, `Ctrl+Alt+R` 恢复 include（操作前先保存）, `Ctrl+Alt+T` 打开 zoi 任务列表; 有快捷键冲突时使用
`Ctrl+P` 输入 `task zoi-`。不要在 tasks.json 标签激活时展开, 不要点击右侧配置齿轮代替运行。
`0 blocks` 表示没有发现模板引用, 不是已插入模板; 先检查已保存源码。

## 插件提交时自动展开

安装提交适配后，直接点击 **CPH 的提交**或**洛谷插件的提交代码**即可。CPH 按题目 URL 分流到 Codeforces 或 VJudge。
插件读取当前 `.cpp` 内容，展开本地头文件后交给原有提交流程；编辑器里的 `include` 写法和旁置状态文件保持原样。
洛谷使用当前编辑内容，包含未保存修改；CPH 保留原有的提交前保存行为。展开期间再修改代码会取消本次提交，需重新点击。
展开失败会停止提交并显示原因，不会退回提交带本地依赖的源码；非 `.cpp` 文件沿用原行为。CPH 的 Kattis 提交不在本适配范围内。

在库根按需要选择以下命令；安装或撤销适配后，执行 VS Code 的 `Developer: Restart Extension Host`：

```powershell
./scripts/install-zoi.ps1 -LuoguShortcuts  # 可选：安装洛谷快捷键
node scripts/install_submit_bridge.cjs  # 安装提交适配
node scripts/install_submit_bridge.cjs --check  # 检查已安装的适配
node scripts/install_submit_bridge.cjs --uninstall  # 仅在需要卸载时执行
```

上面的安装器同时绑定 `Ctrl+Alt+P` 查看洛谷题目、`Ctrl+Alt+Enter` 提交当前代码；恢复键 `Ctrl+Alt+R` 随普通安装提供。
原有同键绑定保留，CPH 的 `Ctrl+Alt+S` 不变。快捷键属于用户安装配置；插件适配属于扩展目录，二者分别安装和卸载。

适配 CPH `2026.9.1789578855` 与洛谷 `4.16.0`／`4.18.1` 的提交入口；这属于本地插件补丁，插件升级可能覆盖，需要重新安装适配。
需要保持这套适配时，可在 VS Code 分别关闭 CPH 和洛谷插件的自动更新，其他插件照常更新。主动升级后先运行 `node scripts/install_submit_bridge.cjs --check`；检查未通过时重新安装适配，再重启扩展宿主。
安装器先核对两个插件的代码形态，不匹配则停止，不猜改新版代码。可用 `--extensions-dir "路径"` 指定其他扩展目录。
插件目录保存原文件 `.zoi-submit-backup` 供故障恢复；正常卸载只撤回提交适配，保留其他修复（例如洛谷记录格式兼容）。
库路径移动后需要重新安装适配。运行时要求受信任的工作区，以及 Windows PowerShell（其他系统使用 `pwsh`）。

适配使用 `zoi.ps1 export <文件.cpp>` 输出展开代码，`-FromStdin` 允许传入编辑器快照；不改源码、不复制剪贴板。
它复用原展开器的解析边界，但不自动迁移库目录；手动移动母版后先运行 `scripts/sync_layout.ps1`。
已有管理状态会检查生成块是否被改动；旧 `.zoi.cpp/.zoi.sha` 状态先手动 expand 迁移，未完成事务先 expand/restore 恢复。

### VJudge：Chrome、Companion 与 CPH

首次安装：先运行上面的 `node scripts/install_submit_bridge.cjs` 并重启 VS Code 扩展宿主；在 Chrome 打开 `chrome://extensions`，开启开发者模式，选择“加载已解压的扩展程序”，选中本库的 `scripts/vjudge-extension` 文件夹。扩展名是 **ZOI VJudge Submit**。文件夹需要留在原处，更新后在扩展页面点“重新加载”。卸载时移除这个 Chrome 扩展，并运行上述 `--uninstall` 撤回 VS Code 补丁。

当前版本为 **1.0.4**。扩展启用后，还需允许它自动访问 `https://vjudge.net/*`、`https://vjudge.net.cn/*` 和 `http://127.0.0.1/*`；只显示“已启用”不代表网站权限已放行。只更新浏览器扩展时重载 Chrome 扩展即可；修改 VS Code 提交适配后才需要重启扩展宿主。

装好后可运行 `node scripts/check_vjudge_chrome.cjs` 检查真实连接；它打开 CodeForces-1A 的 VJudge 表单，准备测试文本并执行编译器、代码内容与提交选项的提交前校验，但**不点击提交**。也可传入其他 VJudge 题目 URL 检查自动回退。看到 PASS 后取消该表单即可。

日常使用：

1. Chrome 登录 VJudge，打开具体题目；比赛内先切到对应题目的 `#problem/A` 页面。
2. 点击 [Competitive Companion](https://github.com/jmerle/competitive-companion) 的绿色加号。它已支持 VJudge 的单题和比赛解析；CPH 沿用你原有的保存位置、C++ 模板和样例配置建立 `.cpp` 与 `.cph` 数据。不需要第二套建文件逻辑。
3. 在 VS Code 写题，`Ctrl+Alt+B` 跑 CPH 样例，`Ctrl+Alt+S` 或 CPH 面板“提交”发送当前题目。CPH 命令名称仍可能显示 Codeforces，但会根据 URL 分流。
4. 工具打开 Chrome，优先选 GNU C++20 和非公开代码，优先使用公共提交账号；原 OJ 禁用公共账号时使用页面已选中的可用个人账号。没有 GNU C++20 时自动选择最高可用的 GNU C++ 标准版本，并在真正提交前向 VS Code 弹出一次提示，显示实际编译器，无须确认。填入展开快照后提交；VJudge 返回提交编号才提示“已接收”，评测结果仍看网页。

提交目前限 Windows Chrome 和 `.cpp`。O2 通过快照开头的 `#pragma GCC optimize("O2")` 启用，不修改编辑器源码。有 GNU C++20 时优先使用它，否则按网页标明的标准版本选择最高项（可能是 C++23，也可能是 C++14）；同标准优先 64 位。未标明标准的 GNU C++ 仅作为最后选择，不从 GCC 编译器版本推断 C++ 标准；没有可用 GNU C++ 时停止。回退只切换编译器，不改写语法，代码需兼容所选版本。需要个人账号的题目，先在 VJudge 表单的“管理账号”绑定原 OJ 账号；工具不会代为登录或绕过限制。登录失效、验证码、比赛权限不足和网络结果不明时看网页提示；工具不会自动重试，重交前先查记录。

`utils.h` 公共底座支持 GNU C++14：现代标准头按语言版本启用，方向数组和 `z_fill_n` 提供旧标准写法；其他算法模板仍以 C++20 为基线，实际使用高版本特性时需要选择兼容的编译器。网页校验允许 CRLF/CR 转为 LF，其他源码改动仍停止提交；报错会区分代码、编译器、公开选项和账号，正常提交流程无须再手动点网页提交。

填表前会等待提交窗口实际显示，避免将弹窗动画期间的隐藏状态误判为关闭；填表后若关闭窗口仍会停止提交。错误同时显示在 VS Code 和网页可关闭的提示条中，不用阻塞网页的弹窗。

文件与题目的关联使用 CPH 保存的 URL，包含比赛编号与题号；自行新建且没有 VJudge URL 的本地题目需先用 Companion 导入。没有样例的题目或 Companion 未抓到样例时，仍需在 CPH 手动补样例。导入无响应时检查 CPH 已激活、Companion 能连接默认端口 `27121`，必要时运行 `Developer: Restart Extension Host`。

| 现象 | 处理 |
|---|---|
| Chrome 只打开题页，没有填入代码 | 核对扩展已重载、三个网站权限已允许，再运行连接检查 |
| 提示已有提交在处理中 | 先检查 Chrome 和 VJudge 提交记录；原任务最多等待 120 秒，结束后再交，避免重复提交 |
| 旧版误报表单或代码发生变化 | 重载到 1.0.4；该版已处理 CRLF 换行转换和窗口延迟显示，真实代码或选项改动仍会停止提交 |
| CF 或 VJudge 拉题后没有创建文件 | 检查 CPH 的 `27121` 接收端口，重启 VS Code 扩展宿主后再点 Companion 绿色加号 |

Chrome 扩展只申请 VJudge 两个域名、`127.0.0.1` 和脚本注入权限；不读取或保存 Cookie。每次提交只临时监听随机本机端口，领取快照和提交许可各限一次，120 秒后关闭；不占用 CPH 的端口。代码在准备期间变动会停止提交，已经发出的请求无法撤回。默认 Chrome 配置中需已登录 VJudge；若平时用额外的 Chrome Profile，确保提交扩展与登录在打开链接的同一 Profile 中。

## 手动展开与恢复

| 任务 | 做什么 |
|---|---|
| zoi-expand | 原地展开依赖并复制提交代码；展开后新增 include 可再次运行 |
| zoi-restore | 恢复当前文件的 include，保留生成块外的题解修改 |
| zoi-status | 查看当前文件的管理状态 |
| zoi-forget | 保留当前代码，解除当前文件管理并清理其管理文件 |
| zoi-restore-all | 批量恢复当前工作区的受管文件 |

改过生成的模板代码时，restore 会拒绝覆盖。想保留现状并清掉旁置管理文件，使用 zoi-forget；以后无法再借这份快照还原。操作前先保存编辑器。

展开成功不保证编辑器补全已配置。红线或 include 补全异常时, 先启用 Microsoft C/C++,
对当前刷题目录运行 zoi-configure, 再用 zoi-doctor 核对配置; 步骤见 [安装与故障排查](../setup/README.md)。

命令行也能用，在仓库根目录运行：

```powershell
./scripts/zoi.ps1 expand "F:/比赛/A.cpp"
./scripts/zoi.ps1 status "F:/比赛/A.cpp"
./scripts/zoi.ps1 restore "F:/比赛/A.cpp"
./scripts/zoi.ps1 forget "F:/比赛/A.cpp"
```

[原技术方案](../../records/tooling/expand/技术方案.md)与[实施记录](../../records/tooling/expand/实施记录.md)完整留档。后续旧 SHA 配对清理修复见 [协作历史](../../rules/sweep-history.md)。
