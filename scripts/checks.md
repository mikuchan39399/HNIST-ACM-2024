# 对拍怎么跑

目标是尽早发现会丢区域赛分数的错误。随机对拍、确定性边界、语法检查和 sanitizer 分工不同，不能互相替代。

在仓库根目录运行：

```powershell
./scripts/run_checks.ps1 -Filter bit
./scripts/run_checks.ps1 -Mode All
./scripts/run_checks.ps1 -Mode Syntax
./scripts/run_checks.ps1 -Sanitize -Compiler g++-13 -TimeoutSec 180 -CompileTimeoutSec 180
./scripts/check_runner.ps1
./scripts/zoi_check.ps1
```

- 默认只跑回归，`Filter` 是套件文件名中的普通子串，匹配不到会失败；它不按模板依赖自动选测试。
- `Mode All` 跑全部对拍和全部非对拍 cpp 的语法检查；普通编译统一 C++20、O2、Wall/Wextra/Werror，并保留 assert。
- `Sanitize` 需要支持 ASan/UBSan 的 GCC，推荐 Linux；启用越界、未定义行为和标准库边界检查，错误立即失败，不修改模板编译契约。
- 默认每次编译和执行各限 120 秒，可分别调整；超时会结束进程树并记为失败，继续检查其他套件。
- `Compiler` 指定实际评测版本。CI 固定操作系统镜像并打印编译器版本，不宣称与所有区域赛评测机完全一致。
- `BuildRoot` 默认是仓库下 `.zoi-checks`，每次建唯一子目录，测试在此执行，不把输入输出写到源码目录。Windows 的 MinGW 如不支持中文临时路径，把它设为可写的纯英文路径。
- `ReportDir` 可指定日志目录，默认放在本次临时目录的 `logs` 下。并发运行须使用不同 ReportDir；默认整套串行执行。
- `summary.json` 记录目标、阶段、退出码、结果和耗时；公共编译参数和编译器版本也有记录。失败时查看同名前缀的 stderr/stdout 日志。
- 本地 `.zoi-checks` 和 `.ci-results` 不进 Git，确认无用后可清理；GitHub 上传日志保存 7 天。报告耗时仅用于定位慢测试，不作为 OJ 性能结论。

## 如何补测试

先读 `rule.md` 第 9 节和 `rules/pitfalls.md`。每个新 bug 留一个确定性反例；同类接口再用独立暴力对拍，固定种子和操作顺序，以便复现。

边界按契约选择：空态/单元素、全相等/全负值、合法数值上界、首尾与完整区间、容量恰好用满、大—小—大复位、历史版本分叉与回访、链/星/森林/重边。返回值哨兵与合法数据范围要分清，不拿越界输入制造伪 bug。

新套件放对应算法目录的 `对拍/` 下，名字以 `_check.cpp` 结尾，入口会自动发现。不需要手工把新套件加到 CI。

`TEST GAP` 只表示没有直接 include 证据，include 过也不代表所有接口和边界都已验证。历史清扫与验证记录见 `rules/sweep-board.md`，不能替代本次运行报告。

## CI 与入口自检

push、PR 和手动运行触发 CI；Windows 安装卸载、普通回归与 sanitizer 各一个作业。回归失败后仍跑语法扫描并上传日志，任何失败都会让作业失败。快读写在 Linux 也做字节比对。

`check_runner.ps1` 在独立小仓库中实际制造编译错误、警告、断言失败、超时、零匹配、缺 catalog、错跳板和重复条目，检查这些情况不会被入口误报成功。它只验证测试入口，不代替算法对拍。

`zoi_check.ps1` 单独验证头文件展开与恢复：钻石依赖、重复展开后加 include、保留题解修改、生成块冲突、旧状态迁移、故障恢复和并发锁；再用真实 seg/bit/hld 组合编译运行紧凑态与展开态。CI 普通作业也运行它。可传 `-Compiler`、`-BuildRoot`；每次夹具和子进程日志留在 `.zoi-checks/expand-test-*`，其中故意构造的冲突和孤儿备份是测试证据，不是你的题目存档。脚本禁止测试时写剪贴板。

每次报告只证明本次源码通过相应检查。已发布提交的 CI 绿灯不覆盖工作区未提交的改动。

## 自动生成测试资产总览

`scripts/check_inventory.ps1` 是 runner 和总览的共用事实来源：校验 catalog/跳板/戳/豁免，收集当前 C++、笔记、对拍文件和直接 include 关系。`TEST GAP` 与总览 B 组使用同一份映射，仍只提示，不代表行为覆盖，也不作为失败门禁。

```powershell
./scripts/make_reliability.ps1
./scripts/make_reliability.ps1 -Check
./scripts/check_inventory_test.ps1
```

第一条更新 `rules/reliability.md`；第二条只比较，过期返回非零、不改文件；第三条在隔离目录验证映射与生成器。CI 校验生成物并运行自检。普通 `run_checks` 不改总览；仅修改算法正文无需重生成，增加/删除源文件、catalog 或测试引用关系后才需要。

A/B/C 分别是带直接引用的 catalog 引擎、未发现直接引用的引擎、纯文本笔记；豁免 C++ 单列。注释和原始字符串中的示例不计入，条件 include 只记静态关系，宏式与传递 include 不统计。套件列提供文件链接，不推断独立暴力或所有 API 覆盖。旧手工表已完整归档至 sweep-history，历史标记不继承为当前正确性评级。实际通过情况仍查运行报告。
## 安装与卸载自检

./scripts/check_setup.ps1 覆盖全新 Profile、重复安装/卸载、JSONC 注释与嵌套同名键、既有 -I 参数、后续用户修改、安装状态异常、写入中断恢复和受管包自删除。Windows CI 使用 PS5.1 与 PS7 各跑一遍，日志保留在 .zoi-checks/setup-test-*。真实 Win11 的插件或编译器不由安装器安装，使用说明见 [队友安装说明](../docs/setup/README.md)。


## 自动汇总验证现状

runner 自动记录源码与依赖指纹、环境和运行结果, 并生成 [口语概览](../docs/verification/status.md) 与 [AI 明细](../docs/verification/details.md). 只重新判断当前源码是否仍被旧结果覆盖时运行 `./scripts/make_verification.ps1`. 具体范围登记、CI 导入和状态规则见 [指南](../docs/verification/README.md). 原 reliability 表继续只负责静态资产关联.
