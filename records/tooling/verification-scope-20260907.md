# 验证状态按范围失效 (2026-09-07)

用户授权解决验证状态噪声，并委托实现取舍。本轮仅修改验证工具、已有自检和相关说明，未修改算法模板；此前未提交改动保留，未提交推送。现行行为见 [验证指南](../../docs/verification/README.md#什么时候需要重验)。

## 问题与最终设计

原 Get-VSnapshot 将整份 verification.json 与套件源码放进同一个哈希。修改 mint 的登记会使其他套件全部显示“代码或验证条件变了”，既无法定位原因，也迫使无关模板反复刷新证据。

新版 fingerprintVersion=2 保留两层信息：

- 套件指纹仍包含对拍文件、传递本地依赖、runner、进程执行、资产扫描与校验脚本；继续归一化换行，不忽略源码注释，条件 include 和宏 include 仍保守追踪。
- 每个 template/suite 条目分别记录 api/oracle/cases/limitations，以及自身 template/suite 的哈希。固定字段顺序，数组按 Ordinal 排序，以长度前缀避免拼接歧义；JSON 格式和属性/条目顺序不参与判断，summary 不代表测试内容，所以只更新显示。
- 运行前后分别判断套件源码和每条范围是否稳定。同套件的 A 改范围不会使 B 失效；同套件代码变化仍影响所有依赖该套件的条目。新增范围不沿用登记前的通过。
- 用户表直接给复验原因；AI 表新增每模板范围判断，最新运行表列源码/依赖/测试/工具变化文件。套件代码通过不代表后来新增范围也通过。
- 外层运行 JSON schema 仍为 1；旧结果没有 fingerprintVersion=2 时保留历史但标需新版复验，不修改旧文件或伪造范围。通过真实运行自然建立新版证据。

这种拆分没有引入新的维护清单，也没有把旧全文件哈希直接丢弃后强行标绿。新增登记仍写原 verification.json，用户运行命令不变。

## 验证结果

| 检查 | 结果与本机日志 |
|---|---|
| Windows PowerShell 5.1 分项验证自检 | PASS；`.zoi-checks/verification-test-328de075feb54c149b0d9d9a6b0ed9cc` |
| PowerShell 7 分项验证自检 | PASS；`.zoi-checks/verification-test-6d21a89076714ef49261c9b41be1f630` |
| PowerShell 5.1 runner 失败路径 | 9/9；`.zoi-checks/runner-test-3808707ad2dc466d9970897ccd8c3157` |
| PowerShell 7 runner 失败路径 | 9/9；`.zoi-checks/runner-test-1b958206051a41d0a9043f15a55d96d7` |
| 全部默认算法回归 | 25/25，GCC 15.2，C++20/O2/Wall/Wextra/Werror/UNDEBUG；编译与运行耗时合计 145.613 秒 |

完整回归的正式 [runner 证据](../verification/runs/036790918feb4279a51c4fe3dde2712d.json) 自动生成，全部源码与分项范围 stable=true；日志 `.zoi-checks/codex-work/verification-scope/full-regression`。重建后用户表为 45 个已登记模板通过、32 个未登记模板未知，没有待重验或旧版记录行。25 套件通过不把未知的 32 项自动宣布最终覆盖完成。

两版 PowerShell 首次真实夹具运行的源码指纹均为 `1a635de7e1a2871faba128bfecce2b9632d86f3146dcf11fed5046e1ef8c7dd1`，分项范围哈希均为 `557d12916cb7851ceb4d4de9d72e6d123bfeb4a8ae1b553d56965418280be142`。

专项测试覆盖：

- 同套件两模板的隔离：实际生成表中 A 范围变化待复验，B 保持通过。
- 无关套件登记与无关源码不影响；summary、JSON 排版/登记順序、API/用例列表顺序不导致误报。
- API/oracle/cases/limitations 改变、新增范围、运行期间相关范围改变必须失效。
- 源码/对拍/执行脚本改变、依赖删除、条件 include 缺文件后新增、宏 include 依赖变化仍失效。
- 旧证据不提升为新通过；最新失败优先、语法不覆盖回归、CI 证据仅落附件目录、表头列数一致。
- runner 原有成功、警告、编译错误、断言、超时、零匹配、坏跳板、重复登记等路径保持原判定；预期失败日志不代表自检失败。

## 自动化与边界

既有 `.github/workflows/ci.yml` 普通回归作业已调用 check_verification_test.ps1 和 check_runner.ps1，本次直接扩展原脚本，CI 自动执行新增检查，无需再登记测试名称。本轮未改变 workflow，也未推送；Linux CI 的本轮执行结果仍待实际运行，本地新增验证为 Windows 两版 PowerShell。

源码与执行脚本变化仍可让整个套件失效，本次只消除无关范围登记及展示性变更的连带失效。摘要修改后仍要运行 make_verification 更新静态 Markdown；不增加文件监听器。历史 JSON 保持原样，不以当前字段推断过去到底测过什么。

规则、用户功能说明、测试说明、验证指南和生成的两层表已同步；本轮不涉及 PDF 内容或模板 README。
