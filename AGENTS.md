# AI 工作入口

1. 先读 [协作与执行约定](rules/collab.md)，按其中[以最终结果为准](rules/collab.md#以最终结果为准)的约定交付，不为日常任务保存逐步 diff 或操作流水。涉及仓库改动或接手时查看 [当前工作板](rules/sweep-board.md)，确认范围、已有改动与占用文件；登记条件与临时产物位置按协作正文执行。
2. 按下表读取本次任务需要的正文；不要默认加载整个历史目录。先定位材料可用 [用户首页](README.md)、[文档导航](docs/README.md) 和 [算法说明目录](docs/features/catalog.md#算法说明文档)。
   学习路线在 [九方向路线图](docs/roadmaps/README.md), 源码同目录 README 是用户选择的随模板说明; 新建与打印约定见 rule, 不自动给每个模板补 README。
3. 一次处理一个明确范围。功能变更收尾执行 [rule 的双向同步要求](rule.md#适用范围与阅读顺序)，同时核对给 LLM 的规则与给用户的功能表。讲解算法、接手或建成模板时执行 [学习进度同步约定](docs/progress/README.md#ai-何时询问如何同步)，只查相关条目；收尾核对同步结果。按[工作区约定](rules/collab.md#工作区与沉淀)清理本任务已用完的一次性测试文件，保留正式对拍、CI 依赖和最终交付物。结束时说明最终结果、必要的验证与未解决问题，不强制追加历史；未经用户授权不提交或推送。

| 本次任务 | 必读正文 |
|---|---|
| 算法讲解、陪练、学习状态更新 | [学习进度同步约定](docs/progress/README.md#ai-何时询问如何同步)及对应方向表 |
| 算法模板、底座、catalog、跳板生成 | [模板契约](rule.md)与[对应条目的设计分类](docs/features/template-design.md); 实现或依赖变化时执行 [验证同步](docs/verification/README.md#ai-维护顺序); 涉及工具时加读维护流程 |
| 对拍、测试入口、CI | [验证标准](rule.md#9-验证标准)、[对拍坑位](rules/pitfalls.md)、[测试说明](scripts/checks.md)、[两层验证表维护](docs/verification/README.md) |
| 安装、展开、恢复、打包、手册脚本 | [文档目录](docs/README.md) 中对应指南、[维护流程](rules/workflow.md) |
| 文档整理 | 将要修改的正文及引用它的入口；按[导航维护](docs/maintenance/README.md#文档导航维护)核对用户/AI 双入口；维护现行结论，既有历史按需查阅，不强制新建过程档案 |

用户明确要求优先。各正文只管自己的范围，入口不复制条款；冲突先查实现和现行正文，无法判定时向用户说明。
模板是否配 README 只按用户明确决定，创建或整理前核对 [当前授权范围](rule.md#31-启发式模板-2026-09-07-用户裁决)，不得以文件已存在推定授权。
用户已持续授权随功能变更同步 rule.md 的功能事实与执行说明，按其双向同步要求维护；改变既定算法取舍/协作政策或修改 records/tooling/rule_history.md 仍需对应的明确授权。已读且未变化的正文不用重复加载；缺文件先查路径，仍找不到再询问用户。
当前验证先查 [AI 明细](docs/verification/details.md) 中相关条目, 必要时从[专项验证索引](records/verification/README.md)或 runs 读取对应证据; 不默认加载全部运行记录
历史方案从[工具历史索引](records/tooling/README.md)和[清扫历史](rules/sweep-history.md)按问题进入; [Cline 入口](.clinerules/zoi.md)仅转到本页
