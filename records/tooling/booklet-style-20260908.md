# 四份手册说明统一排版

日期：2026-09-08。执行者：Codex。用户确认树的中心样张后，要求落实其他三份并写入文档；本次未提交、未推送。

## 结果与范围

- 将样张正式接入 [Markdown 转换器](../../scripts/booklet_markdown.ps1)，四份获准 README 都采用深绿主副标题、编号小节、浅灰公式框、细横线表格与竖线提示。样式不绑定文件名，未增加算法 README。
- 重排线段树插件、树的直径、树的重心，并将树中心样张落实到源码 README。保留原有接口、公式、正/零/负权条件、反例和复杂度；插件表改两列，注意事项回到对应小节。
- 标题与开头段落、后续定义保持相连，修复直径说明开头在栏底断开的实际版面。长表仍自动跨栏/页并重复表头；说明正文 7.1 pt，代码 6 pt。
- 写法与例子进入 [打印指南](../../docs/booklet/README.md#说明怎么排版)，实现约定进入 [维护指南](../../docs/maintenance/README.md#手册生成约定)，同步 rule、功能总览、测试说明。功能目录内容未变化，生成校验通过。

## 验证

环境：Windows，PS 5.1 / PS 7，Typst 0.15.1，Noto Sans SC、Source Sans Pro、Consolas、New Computer Modern Math，pypdf 与 Poppler。全部命令在库根执行，构建临时目录位于库内；这不是 Linux 本地验证。

| 检查 | 结果 |
|---|---|
| PS 5.1 `scripts/check_booklet.ps1` | 18 次构建通过，包含预期失败输入 |
| PS 7 `scripts/check_booklet.ps1 -Render` | 18 次构建通过，含 90 行长表、1200 行源码增长、奇数页 |
| 样例 `scripts/check_booklet_pdf.py` | 17 页、17 目录、1 说明、4 公式；七层目录样式、字号、字面量、长表行序与重复表头通过 |
| 正式 `scripts/make_booklet.ps1` | 106 个实现/笔记/插件各占独立页段；4 份相邻说明、187 个真实目录、32 个公式通过 |
| 正式 `scripts/check_booklet_pdf.py` | 141 页；275 个目录/实现书签、106 个独立起页；MIKU 仅在物理第 39 页 |
| `scripts/make_features.ps1 -Check` | 跳板与生成目录同步通过 |
| `scripts/check_docs.py`、`git diff --check` | 双入口链接与锚点、空白检查通过 |

最终正式 PDF 目视检查第 23 页插件、第 76 页直径、第 77 页重心、第 78 页中心，以及第 39 页 MIKU。标题、公式、表格、提示与跨栏无重叠、无越界；重心小表续栏时重复表头，长表另抽查两页。算法实现未修改，不把版式验证记为算法复验。新增测试由既有 CI 入口调用，在线 CI 待用户授权提交/推送后运行。

## 交付与清理

唯一正式 PDF 为 `docs/booklet/output/zoi-booklet-print.pdf`，同目录保留 `.typ`。用户批准的单页样张、渲染图片和本次测试目录在验证信息落入本记录后清理；不清来源不明或其他任务的产物。

本次交付 SHA-256（供核对本次产物，后续重建会变化）：

| 文件 | SHA-256 |
|---|---|
| `zoi-booklet-print.pdf` | `67A9EA22B6E7DA34679D966D00F94D61A33CF25FA59697CF9B72D5CE864EEF07` |
| `zoi-booklet-print.typ` | `1A034679208A4D8D76DF1E59321A426C6C65E4CC6449485F951ADA6563FF1EB3` |
| `scripts/booklet_markdown.ps1` | `CC63A18F34D711F1150F7C5F44C2799FBF994C4D6F1C9C1FEF7FBCAC318F339C` |
| `scripts/check_booklet.ps1` | `BBD4AB09C7704DD9B1009775102408D75035F867552B8E19404F3795DF8C8EF1` |
| `scripts/check_booklet_pdf.py` | `3590144D7BE244C356855993E9232F9A69DBD0DF1C2ED7ADFDB4E3098514F1C7` |
