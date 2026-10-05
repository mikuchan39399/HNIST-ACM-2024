# 跳板、展开与恢复

先按 [安装说明](../setup/README.md) 注册任务。写题时 include 所需跳板，保存当前文件后，在 VS Code 运行：

推荐 `#include "bit.h"`, 已知 zoi 跳板的 `<bit.h>` 也支持; 普通 `<vector>` 等标准头不展开。
展开前会按源码稳定短名同步库内路径；自己移动母版仍可继续使用原跳板，详见[目录同步](../maintenance/layout.md)。恢复、状态查询和解除管理不执行目录迁移。
`Ctrl+Alt+Z` 直接展开当前 C++ 文件, `Ctrl+Alt+R` 恢复 include（操作前先保存）, `Ctrl+Alt+T` 打开 zoi 任务列表; 有快捷键冲突时使用
`Ctrl+P` 输入 `task zoi-`。不要在 tasks.json 标签激活时展开, 不要点击右侧配置齿轮代替运行。
`0 blocks` 表示没有发现模板引用, 不是已插入模板; 先检查已保存源码。

## 插件提交时自动展开

安装提交适配后，直接点击 **CPH 的提交**或**洛谷插件的提交代码**即可。CPH 按题目 URL 分流到 Codeforces、VJudge 或 AtCoder。
插件读取当前 `.cpp` 内容，展开本地头文件后交给原有提交流程；编辑器里的 `include` 写法和旁置状态文件保持原样。
洛谷使用当前编辑内容，包含未保存修改；CPH 保留原有的提交前保存行为。展开期间再修改代码会取消本次提交，需重新点击。
展开失败会停止提交并显示原因，不会退回提交带本地依赖的源码；VJudge / AtCoder 自动提交仅支持 `.cpp`，其余平台的非 `.cpp` 文件沿用原行为。CPH 的 Kattis 提交不在本适配范围内。

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

### 浏览器扩展：Chrome、Companion 与 CPH

首次安装：先运行上面的 `node scripts/install_submit_bridge.cjs` 并重启 VS Code 扩展宿主；在 Chrome 打开 `chrome://extensions`，开启开发者模式，选择“加载已解压的扩展程序”，选中本库的 `scripts/vjudge-extension` 文件夹。扩展名是 **ZOI Submit**（原名 ZOI VJudge Submit）；为兼容已有安装，目录名保持不变。文件夹需要留在原处，更新后在扩展页面点“重新加载”。卸载时移除这个 Chrome 扩展，并运行上述 `--uninstall` 撤回 VS Code 补丁。

当前版本为 **1.4.1**。扩展启用后，还需允许它自动访问 VJudge 两个域名、AtCoder、Codeforces（含 www、m1、m2）和 `http://127.0.0.1/*`；题单功能另需允许 `https://youkn0wwho.academy/*`，脚本只匹配 `/topic-list*` 页面。只显示“已启用”不代表网站权限已放行。从 1.0.x 升级需重新运行提交适配安装器并重启 VS Code 扩展宿主；已有版本重载 Chrome 扩展、允许新增网站并刷新题单。右侧题面另装下文的 ZOI 题面 VSIX，提交适配与题面扩展分别维护。

装好后可运行 `node scripts/check_vjudge_chrome.cjs` 检查真实连接；它打开 CodeForces-1A 的 VJudge 表单，准备测试文本并执行编译器、代码内容与提交选项的提交前校验，但**不点击提交**。也可传入其他 VJudge 题目 URL 检查自动回退。看到 PASS 后取消该表单即可。

#### VJudge

日常使用：

1. Chrome 登录 VJudge，打开具体题目；比赛内先切到对应题目的 `#problem/A` 页面。
2. 点击 [Competitive Companion](https://github.com/jmerle/competitive-companion) 的绿色加号。它已支持 VJudge 的单题和比赛解析；CPH 沿用你原有的保存位置、C++ 模板和样例配置建立 `.cpp` 与 `.cph` 数据。不需要第二套建文件逻辑。
3. 在 VS Code 写题，`Ctrl+Alt+B` 跑 CPH 样例，`Ctrl+Alt+S` 或 CPH 面板“提交”发送当前题目。CPH 命令名称仍可能显示 Codeforces，但会根据 URL 分流。
4. 工具打开 Chrome，优先选 C++20 和非公开代码，优先使用公共提交账号；原 OJ 禁用公共账号时使用页面已选中的可用个人账号。没有可用 C++20 时自动选择最高可用的标准版本，并在真正提交前向 VS Code 弹出一次提示，显示实际编译器，无须确认。填入展开快照后提交；VJudge 返回提交编号才提示“已接收”，评测结果仍看网页。

提交目前限 Windows Chrome 和 `.cpp`。快照开头添加 `#pragma GCC optimize("O2")`，在 GCC 上启用 O2，不修改编辑器源码。VJudge 对各原 OJ 使用同一套语言识别和排序，不写死 OJ 编译器 ID：

- 支持 GNU C++ / G++ / GNU++ / CPP、CSES 的 `C++20`，以及 UVA 的 `C++11 5.3.0` 这类标准后带版本号的标签；兼容 `0x/1y/1z/2a/2b/2c` 标准别名，单个有效选项也能识别。
- 优先 C++20；没有时选最高可用 C++11+（可能是 C++23，也可能是 C++11）；同标准先明确 GNU，再优先 64 位。明确 C++98/03 的选项不选择。
- 没有明确标准时，普通 `C++` / `G++` 或带编译器版本的选项才作为最后选择，并提示无法确认 C++11 支持。`GCC 11.2`、`C++ 11.2.0` 都不能证明启用了 C++11。
- 明确标注 Clang、MSVC、Visual C++、C++/CLI 或 IOI-Style 的选项不自动选择；没有支持的选项时停止。无厂商标签不能证明使用 GCC，实际编译器和 O2 效果以原 OJ 为准。

YOUKNOWWHO 中跳转到 VJudge 的题目共用上述规则；只有搜索入口、尚未被 VJudge 收录或不能提交的题目，不会因此获得提交能力。Codeforces 原站沿用 CPH/浏览器提交插件配置的编译器，AtCoder 原站使用下文的动态选择。回退只切换编译器，不改写语法，代码需兼容所选版本。需要个人账号的题目，先在 VJudge 表单的“管理账号”绑定原 OJ 账号；工具不会代为登录或绕过限制。登录失效、验证码、比赛权限不足和网络结果不明时看网页提示；工具不会自动重试，重交前先查记录。

`kmp.h` 和 `utils.h` 支持 GNU C++11 及以上，并检查了 C++11/14/17/20/23 的实际展开代码。KMP 的构造、`build`、`find_first`、`find_all` 统一接收 `const string&`，普通字符串和字面量调用保持不变；已有 `string_view` 调用需先显式转换为 `string`，子串用 `s.substr(...)`。KMP 自己不需要条件编译；utils 的现代标准头、方向数组和 `z_fill_n` 保留按标准选择的写法。其他算法模板尚未逐件适配，不能由 KMP 的结果推断全库兼容。网页校验允许 CRLF/CR 转为 LF，其他源码改动仍停止提交；报错会区分代码、编译器、公开选项和账号，正常提交流程无须再手动点网页提交。

填表前会等待提交窗口实际显示，避免将弹窗动画期间的隐藏状态误判为关闭；填表后若关闭窗口仍会停止提交。错误同时显示在 VS Code 和网页可关闭的提示条中，不用阻塞网页的弹窗。

文件与题目的关联使用 CPH 保存的 URL，包含比赛编号与题号；自行新建且没有 VJudge URL 的本地题目需先用 Companion 导入。没有样例的题目或 Companion 未抓到样例时，仍需在 CPH 手动补样例。导入无响应时检查 CPH 已激活、Companion 能连接默认端口 `27121`，必要时运行 `Developer: Restart Extension Host`。

| 现象 | 处理 |
|---|---|
| Chrome 只打开题页，没有填入代码 | 核对扩展已重载、三个网站权限已允许，再运行连接检查 |
| 提示已有提交在处理中 | 先检查 Chrome 和 VJudge 提交记录；原任务最多等待 120 秒，结束后再交，避免重复提交 |
| 页面有 C++20，却提示没有可用 GNU C++ | 重载到 1.0.5；已兼容 CSES 等原 OJ 只写标准版本的语言名称 |
| UVA 有 C++11 5.3.0，却提示没有可用编译器 | 重载到 1.3.0；已支持标准名称后的编译器版本号 |
| UVA 编译 KMP 提示 string_view 未声明 | 更新 kmp.h 对应源码后重新展开；新接口使用 const string&，C++11 可用 |
| 旧版误报表单或代码发生变化 | 重载到 1.0.4；该版已处理 CRLF 换行转换和窗口延迟显示，真实代码或选项改动仍会停止提交 |
| CF 或 VJudge 拉题后没有创建文件 | 检查 CPH 的 `27121` 接收端口，重启 VS Code 扩展宿主后再点 Companion 绿色加号 |

Chrome 扩展申请 VJudge 两个域名、AtCoder、Codeforces（含 www、m1、m2）、YOUKNOWWHO、`127.0.0.1` 和脚本注入权限；不读取或保存 Cookie。每次提交只临时监听随机本机端口，领取快照和提交许可各限一次，120 秒后关闭；不占用 CPH 的端口。题面读取使用独立的只读通道。代码在准备期间变动会停止提交，已经发出的请求无法撤回。默认 Chrome 配置中需已登录目标 OJ；若平时用额外的 Chrome Profile，确保扩展与登录在打开链接的同一 Profile 中。

#### AtCoder

从 `https://atcoder.jp/contests/<比赛>/tasks/<题目>` 用 Companion 拉题后，在 CPH 点击“提交”或按 `Ctrl+Alt+S`。工具自动展开当前 `.cpp`，打开该题对应的 AtCoder 提交表单，选择题号、语言并填入代码；支持 Ace 和纯文本编辑器。代码可见性遵循 AtCoder 自身规则。

优先普通 GNU C++20，没有时选择最高可用普通 GNU C++11+，兼容标准别名，并向 VS Code 提示实际版本；不固定语言 ID，不选择 Clang 或 IOI-Style。当前普通 GCC 环境可能只有 C++23，以实际表单为准。展开代码连同 O2 pragma 的 UTF-8 大小不得超过 512 KiB；回退不改写语法。

提交时沿用页面原生校验和表单字段，由页面发送一次同源 POST，CSRF 与网页验证码保留在浏览器中。对比提交前后的个人记录，并核对新增记录的题号和完整源码后，才提示提交编号并打开详情。登录、参赛权限或验证码需要在网页处理；结果未知或同时出现多条新记录时不会自动重试，先查个人提交记录再决定是否重交。

真实连接检查：`node scripts/check_atcoder_chrome.cjs <AtCoder题目URL>`。它只选择题号、语言、填入测试代码并校验，不发送提交；检查后关闭测试页即可。

### YOUKNOWWHO 题单链接

打开 [YOUKNOWWHO 题单](https://youkn0wwho.academy/topic-list) 的任一主题，扩展会在题名旁补充小链接：

- CF / AtCoder 保留原题名链接，不再加重复的原站入口。
- CSES、SPOJ、UVA、LightOJ、CodeChef、Yosupo、USACO、Kattis 等已识别来源显示绿色 **VJudge**，打开对应题页；本来就链接 VJudge 的题目不重复添加。
- PDF 题面、未支持的网址或无法确认编号的题目显示 **VJudge 搜索**，按题名搜索。搜索不保证存在收录；原题名链接始终保留。
- 题目旁的蓝色 **VS Code** 按钮可一键导入文件、样例和右侧题面。CF / AtCoder 使用原站，其余来源使用 VJudge；没有可靠映射时先显示 VJudge 搜索结果，核对后选择。无法解析的网址不生成导入按钮。

链接在新标签页打开，筛选、排序或页面动态更新后会自动补齐。只处理题目表格，不修改资源、收藏、做题进度或提交功能，也不会自动请求各 OJ。UVA 内部 ID 与正式题号、LightOJ 旧题库的 slug 与旧编号使用随扩展附带的公开映射表；新题不在表中时使用搜索入口。VJudge 对应题页是否可提交，仍取决于它对原 OJ 的支持状态。

只用网页链接时，加载 Chrome 扩展即可；使用 **VS Code** 按钮还要安装下文的题面扩展与 CPH。两种功能均不要求安装 CPH 提交补丁。升级后刷新题单页；仍无按钮时检查 Chrome 扩展为 1.4.1 且已允许 YOUKNOWWHO 网站访问。

## 右侧题面与一键导入

安装独立的 **ZOI 题面 1.1.1** 扩展后，在 Companion 导入的代码文件上按 **Ctrl+Alt+O**，或运行命令 **ZOI: 在右侧查看当前 CPH 题面**。扩展从该文件的 `.cph` 数据读取题目链接，在右侧编辑器分栏显示正文；不是全站题库搜索侧栏。

首次安装或升级，在库根运行（需要 Python 3，运行扩展本身不需要 Python）：

```powershell
python scripts/package_statement.py
code --install-extension docs/releases/zoi-statement-1.1.1.vsix
```

也可在 VS Code 扩展页选择“从 VSIX 安装”，打开生成的文件。安装后执行 **Developer: Reload Window**。Chrome 端使用上文的 ZOI Submit 1.4.1；CF、AtCoder 原题与 VJudge 三种链接均可使用。工作区须为受信任的本地文件夹。

- **查看已导入题目**：用 Companion 绿色加号导入 → 打开代码 → Ctrl+Alt+O。面板支持公式、图片、样例、AtCoder 中提供的英日双语，以及 VJudge 原文/翻译版本切换；UVA 等 PDF 题面在面板内显示并可选中文字。
- **从题单一键开始**：先在 VS Code 打开刷题文件夹，再点 YOUKNOWWHO 题目旁的 **VS Code**。首次使用时 Chrome 可能询问是否打开 VS Code。题面解析后通过 CPH 接收服务生成文件与样例，继续使用 CPH 的语言、模板配置。已有相同题目的文件会直接打开，保留代码和样例。
- **刷新与网络**：题面缓存七天、最多一百条，面板“刷新”重新读取。CF / AtCoder 优先直接读取，遇到验证等情况改从 Chrome 读取；VJudge 从 Chrome 获取当前可见的题面版本。网页需要登录或验证时手动完成，再点“从 Chrome 读取”。
- **Gym 只有整场 PDF**：单题跳到附件页时仍保留原题号，自动读取对应 `Gym-比赛号题号` 的 VJudge 版本；也可点“整场 PDF”查看原站文件。PDF 有明确 `Problem X.` 标题时只显示当前题的页，无法可靠定位时显示整份并提示补样例。CPH 文件关联和提交目标仍是原 CF 单题，不会因换题面来源生成另一份题目。
- **题单链接只有附件或 PDF**：无法从整场链接确定单题时，VS Code 按钮改为 VJudge 的 Gym 题名搜索，由你选择正确题目，原题名链接保留。

LightOJ、UVA、CSES、SPOJ 等来源统一读取 VJudge 提供的题面，不要求配置这些原 OJ 的提交账号；能否显示取决于 VJudge 的收录、当前网页权限和题面格式。未知来源的题名搜索不保证找到结果。样例支持常见 HTML 格式和上下排列的 PDF `Sample Input` / `Sample Output`；扫描件、并排多栏等无法可靠识别时提示在 CPH 手动补样例，不能把“显示题面”等同于“完整提取样例”。

Gym PDF 另支持定位单题后，带 `Example` 和输入/输出列头的规则双栏单组样例；疑似多组样例分隔、跨栏正文或不明确的布局不会自动导入，避免把几组样例拼成一组。PDF 获取失败时保留原文链接并报告失败，不假装完成导入。

CPH 接收端口使用默认 `27121`，文件生成到 CPH 所在窗口的首个工作区文件夹。多个 VS Code 窗口可能抢占接收端口；若提示已发送但未确认文件，先看 CPH 的语言/保存提示和实际接收窗口，避免重复导入。普通本地代码若没有 `.cph` 关联，应先用 Companion 导入题目。

想省掉每次选择语言，在刷题工作区设置 **CPH: Default Language** 为 `cpp`。接收端口未启动时先执行 **Developer: Restart Extension Host** 并打开 CPH 面板；看到题面不代表 CPH 已建文件，以“已生成题目文件”提示和实际文件为准。

卸载题面功能：在 VS Code 卸载 **ZOI 题面**，或运行 `code --uninstall-extension zoi-local.zoi-statement`。这不会删除题解或 CPH 样例。只卸载题面时，可继续保留 Chrome 扩展的提交和 VJudge 链接功能。

## 把题目移入垃圾堆

ZOI 题面 1.1.1 同时提供 **Ctrl+Alt+Shift+T**（macOS 为 Cmd+Alt+Shift+T），命令名为 **ZOI: 勾选题目移入垃圾堆**。在编辑器或文件列表中按快捷键，输入题名筛选、勾选复选框、回车移动；Esc 取消。仅列出当前工作区根目录的 `.cpp/.cc/.cxx/.c`，不扫描子目录、头文件或现有 `trash heap`，不预选任何题目。

选中的源码移入该工作区的 `trash heap`；对应 CPH 样例一并移动，并更新源码路径和 CPH 文件名，使题目在垃圾堆中仍可打开样例与题面。也携带同名 `.exe/.out/.o` 和 ZOI 的 `.zoi.cpp/.zoi.sha/.zoi.state.json`。源码、样例内容和展开状态保留，完成提示中的 **撤回本次** 可恢复本次移动；目标已有同名文件或有未保存修改时整批停止，不覆盖内容。

此功能支持默认的题目旁 `.cph`，配置了 CPH 自定义数据目录时会提示停止。遇到符号链接、未恢复的展开事务或无法读取的关联数据也保留现场。先完成全部目标副本再移除源文件，普通错误会尝试回退；若进程被强制终止，可能两处都留有文件，应先核对，不能直接覆盖重试。不会发送提交，也不会清空垃圾堆。

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
