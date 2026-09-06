# 跳板、展开与恢复

先按 [安装说明](../setup/README.md) 注册任务。写题时 include 所需跳板，保存当前文件后，在 VS Code 运行：

推荐 `#include "bit.h"`, 已知 zoi 跳板的 `<bit.h>` 也支持; 普通 `<vector>` 等标准头不展开。
`Ctrl+Alt+Z` 直接展开当前 C++ 文件, `Ctrl+Alt+T` 打开 zoi 任务列表; 有快捷键冲突时使用
`Ctrl+P` 输入 `task zoi-`。不要在 tasks.json 标签激活时展开, 不要点击右侧配置齿轮代替运行。
`0 blocks` 表示没有发现模板引用, 不是已插入模板; 先检查已保存源码。

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
