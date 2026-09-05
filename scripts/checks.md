# 对拍怎么跑

目标是尽早发现会丢区域赛分数的错误。随机对拍、确定性边界、语法检查和 sanitizer 分工不同，不能互相替代。

在仓库根目录运行：

```powershell
./scripts/run_checks.ps1 -Filter bit
./scripts/run_checks.ps1 -Mode All
./scripts/run_checks.ps1 -Mode Syntax
./scripts/run_checks.ps1 -Sanitize -Compiler g++-13 -TimeoutSec 180 -CompileTimeoutSec 180
./scripts/check_runner.ps1
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

push、PR 和手动运行触发 CI；普通回归与 sanitizer 各一个作业。回归失败后仍跑语法扫描并上传日志，任何失败都会让作业失败。快读写在 Linux 也做字节比对。

`check_runner.ps1` 在独立小仓库中实际制造编译错误、警告、断言失败、超时、零匹配、缺 catalog、错跳板和重复条目，检查这些情况不会被入口误报成功。它只验证测试入口，不代替算法对拍。

每次报告只证明本次源码通过相应检查。已发布提交的 CI 绿灯不覆盖工作区未提交的改动。
