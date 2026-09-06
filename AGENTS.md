# AI 工作入口

1. 先读 [协作与执行约定](rules/collab.md) 和 [当前工作板](rules/sweep-board.md)，确认任务范围、已有改动与占用文件；遵守其中的库内工作区与沉淀约定。
2. 按下表读取本次任务需要的正文；不要默认加载整个历史目录。
3. 一次处理一个明确范围。功能变更收尾执行 [rule 的双向同步要求](rule.md#适用范围与阅读顺序)，同时核对给 LLM 的规则与给用户的功能表。讲解算法、接手或建成模板时执行 [学习进度同步约定](docs/progress/README.md#ai-何时询问如何同步)，只查相关条目；收尾核对同步结果。结束时说明改动、验证结果和未解决问题；未经用户授权不提交或推送。

| 本次任务 | 必读正文 |
|---|---|
| 算法讲解、陪练、学习状态更新 | [学习进度同步约定](docs/progress/README.md#ai-何时询问如何同步)及对应方向表 |
| 算法模板、底座、catalog、跳板生成 | [模板契约](rule.md); 实现或依赖变化时执行 [验证同步](docs/verification/README.md#ai-维护顺序); 涉及工具时加读维护流程 |
| 对拍、测试入口、CI | [验证标准](rule.md#9-验证标准)、[对拍坑位](rules/pitfalls.md)、[测试说明](scripts/checks.md)、[两层验证表维护](docs/verification/README.md) |
| 安装、展开、恢复、打包、手册脚本 | [文档目录](docs/README.md) 中对应指南、[维护流程](rules/workflow.md) |
| 文档整理 | 将要修改的正文及引用它的入口；保留约束与历史，不重复编写第二套规则 |

用户明确要求优先。各正文只管自己的范围，入口不复制条款；冲突先查实现和现行正文，无法判定时向用户说明。
用户已持续授权随功能变更同步 rule.md 的功能事实与执行说明，按其双向同步要求维护；改变既定算法取舍/协作政策或修改 records/tooling/rule_history.md 仍需对应的明确授权。已读且未变化的正文不用重复加载；缺文件先查路径，仍找不到再询问用户。
当前验证先查 [AI 明细](docs/verification/details.md) 中相关条目, 必要时读 records/verification/runs 的对应证据; 不默认加载全部运行记录
历史方案在 records/tooling、records/tooling/rule_history.md、rules/sweep-history.md, 只有追溯特定问题时才读
