# Windows PowerShell 手册审计引号修复

2026-09-11，用户使用 VS Code 的 Windows PowerShell 5.1 任务生成手册。PDF 编译成功后，Typst 审计将 `starts-with("entry-end:")` 收到为 `starts-with(entry-end:)`，报 expected expression 并退出 1。

## 原因与修复

错误位于 PowerShell 调用原生程序的参数绑定，生成的 Typst 正文没有该语法错误。旧 `& $typst eval $Expression --in $typPath` 在 PS 5.1 中丢失表达式内部引号。

- `make_booklet.ps1` 的生产审计函数改用 ProcessStartInfo；现代 .NET 直接填写 ArgumentList，.NET Framework 使用与库内进程工具相同的 CRT 引号/尾部反斜杠规则。
- stdout/stderr 同时异步读取，显式按 UTF-8 解码；根据实际进程 ExitCode 抛出包含 Typst 诊断的异常。删除依赖旧 LASTEXITCODE 的重复判断。
- `check_booklet.ps1 -AuditOnly` 从生产源码提取该函数，实际查询带引号的元数据、含转义引号/反斜杠的字面量和中文返回值，输入文件路径包含中文与空格；错误表达式必须报错。兼容 PS 5.1/7 对单元素 JSON 数组的不同展开行为。
- Windows setup CI 安装固定 Typst 0.15.1 后，分别用 PS 5.1/7 跑 AuditOnly，不依赖中文字体；原 SourceOnly 测试保留。Linux Render 测试也自动包含审计回归，再运行原有 18 次构建。

没有改变排版、算法或任务入口；原 VS Code 命令可直接继续使用。规则、用户功能表、手册指南、维护说明及测试入口同步。

## 实际验证

环境、脚本/正式产物 SHA-256、原始报错及最终日志见 [JSON 证据](booklet-argv-20260911.json)。

| 检查 | 结果 |
|---|---|
| 修复前，PS 5.1 / Typst 0.15.1 的 Render | 首次构建复现同样的引号丢失错误，退出 1 |
| 修复后，PS 5.1 Render | 18 次构建通过，含 1200 行增长、长表、奇数页及预期拒绝输入 |
| 修复后，PS 7 Render | 18 次构建通过，包含新增真实审计检查 |
| 最终版本 PS 5.1 / PS 7 AuditOnly | 两者均通过，覆盖合法查询与失败传播 |
| 原 Windows PowerShell 命令重建正式整本 | 退出 0；95 个 catalog 条目、11 个插件、4 份说明，106 个独立页段、189 个目录、32 个公式 |
| 独立 pypdf 全文检查 | 143 页；277 个目录/源码标题，106 个独立起页；MIKU 仅在第 39 页 |
| 样例 PDF 检查 | 17 页，七层目录样式、90 行长表顺序与重复表头、文字/公式、增长行序通过 |
| 视觉抽查 | 第 2 页目录和第 39 页三栏源码/MIKU 页脚正常 |

PS 5.1 Render 成功现场后来由原有三份 PASS 保留机制清理；成功结果已当场读取，最终 AuditOnly 日志单独保留。两次新增测试调试分别发现 PS 5.1/7 的 JSON 数组展开差异，修正的是测试断言；正式审计函数在这两次之间未变。

本轮没有在远端实际运行更新后的 CI，不把本地通过等同于线上通过；未重新测试无关算法或修改其验证状态。正式 PDF 和 Typst 仍在 `docs/booklet/output/`，不提交 Git。证据沉淀后已清理本轮 7 个临时目录（112 个文件），正式产物保留；用户原有 `_snap.ps1`、学习记录与此前 UVA1391 历史保留。未提交、未推送。
