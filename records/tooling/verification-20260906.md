# 两层验证表落地记录

2026/9/6, astra, 用户授权自主实施, 未提交

- rules/verification.json 记录模板与套件、API、暴力参照、数据、口语摘要及限制, 首批核实四件连通性模板
- check_verification.ps1 提供传递依赖指纹与证据状态判定, LF 归一化, Ordinal 排序, 检测运行前后变更
- run_checks.ps1 保存带环境、参数、结果、完整文件指纹的 JSON, 并自动生成用户 status.md 与 AI details.md
- 本地证据进入 records/verification/runs, CI 进入各自 ReportDir 并由现有 artifact 上传, 不自动提交
- 用户表不把 include 当覆盖, 明细保留静态关联与未登记项; 失败、待重验、无记录分别显示, 语法通过不顶替行为对拍
- 导航、rule、AGENTS、测试指南与队友打包同步

验证: PS5.1 与 PS7 验证系统自检通过, 包括传递依赖变化、CRLF 等价、运行期间变化标记、最新失败优先、语法结果隔离、CI 输出隔离. 最终 Windows conn 两套通过并自动生成证据及两层表, 日志 .zoi-checks/59a44a7b4fb14e798f2e26360acf0393/logs. 原 runner 九项自检在 PS5.1 通过, 日志 .zoi-checks/runner-test-f680212b3abe4e9eb0324a2104b4869b. 自检脚本补显式 exit 0, 避免外层对退出状态的歧义. actionlint、diff --check、ASCII 编码核对通过, 队友包内 manifest / 脚本 / 证据 / 两层表均存在且无沙箱产物

限制: 历史无指纹日志不补造通过, 新表目前仅 Windows 普通回归有本轮真实记录. Linux 由后续 CI 产物导入, 新版 CI 尚未推送运行. 范围清单全文件参与指纹, 任一条修改可能使别的记录待重验; 注释也保守触发重验. 系统头文件不逐文件散列, 编译器版本与参数独立记录. 静态表必须在源码变更后重新生成才能反映最新状态
