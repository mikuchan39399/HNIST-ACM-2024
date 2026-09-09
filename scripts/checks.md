# 对拍怎么跑

目标是尽早发现会丢区域赛分数的错误。随机对拍、确定性边界、语法检查和 sanitizer 分工不同，不能互相替代。

生命周期专项覆盖完整 build/run 不先 init 的复用路径。`lifecycle_check` 每组 350 轮, 核对备用三份 LCA、连通性派生图重建、普通线段树旧懒标记以及必须保留的离线输入/重心点权/持久化版本; `segfhq_lifecycle_check` 用 350×100 独立数组操作与 20万-1-0-257-20万复用验证旧树套树新入口。既有图论和普通线段树套件也已迁移为直接 build/run, 两份新套件由普通及 sanitizer CI 自动发现。全部现役条目与例外见 [生命周期清单](../records/verification/lifecycle-20260909.md)。

线段树常用插件用 `./scripts/run_checks.ps1 -Filter seg_plugins`。套件直接 include 七份插件，逐数组暴力检查全部维护量，普通/动态树各 64 轮×320 次；默认另跑每组 20万-1-257-20万复位、长短区间交错修改和查询。Binary/Linear 只验完整 build，其余五组另验 1e9 稀疏零域。`*_check.cpp` 自动发现，普通与 sanitizer CI 都执行，无需手动添加 suite 名。专题边界与证据见 [插件验证](../records/verification/seg-plugins-20260907.md)。

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
- 本地 `.zoi-checks` 和 `.ci-results` 不进 Git; 新版入口完成后标记现场, 成功的默认缓存每类自动保留三份,
  FAIL/无标记/人工 codex-work 保留。`clean_checks.ps1` 默认只预览, `-Apply` 才清理, 不删除 `.ci-results` 或长期证据。
  GitHub 上传日志保存 7 天。报告耗时仅用于定位慢测试，不作为 OJ 性能结论。

## 如何补测试

BigInt 和 rw 的原生套件分别用 `-Filter bigint`、`-Filter rw_check`；rw 被共享的 `misc_check` 引用，修改后还须跑 `-Filter misc`。

先读 `rule.md` 第 9 节和 `rules/pitfalls.md`。每个新 bug 留一个确定性反例；同类接口再用独立暴力对拍，固定种子和操作顺序，以便复现。

边界按契约选择：空态/单元素、全相等/全负值、合法数值上界、首尾与完整区间、容量恰好用满、大—小—大复位、历史版本分叉与回访、链/星/森林/重边。返回值哨兵与合法数据范围要分清，不拿越界输入制造伪 bug。

新套件放对应算法目录的 `对拍/` 下，名字以 `_check.cpp` 结尾，入口会自动发现并无参数运行。额外命令行压力模式不会因此自动执行, 用于验收的模式必须另接入 CI, 不能只留手动命令。

`TEST GAP` 只表示没有直接 include 证据，include 过也不代表所有接口和边界都已验证。历史清扫与验证记录见 `rules/sweep-board.md`，不能替代本次运行报告。

`seggraph_check` 无参数即执行独立 Floyd/可达性对拍、SCC 组装和 20 万点/操作及中继点压力,
因此自动进入普通与 sanitizer CI, 不需要另加压力 profile。范围与数值限制见优化建图源码契约与验证明细。

`treegraph_check` 无参数执行 int/LL/Empty 各 400 组独立 BFS 路径展开和 Floyd/可达性对拍，核对所有原点及返回中继点。
默认另验 5万-1-17-5万深链与每轮百万混合操作、20万点星/二叉/孤立森林、单原点20万中继、断连原子性与边数上界。
深链使用闭式祖先提供者，随机森林和20万浅树使用真实 LCA，不改变原 LCA 的递归栈契约；套件自动进入普通与 sanitizer CI。

`graph_check` 的最短路部分使用独立 i128 Floyd，int/LL × 有向/无向共 4800 图，逐合法源点核对四版距离与两版全图判环。默认覆盖重复/空多源、不可达负环、INF-1、LL 极值负环溢出反例、20 万容量与多测、1800 点稠密/逆链及普通 SPFA 反复入队。普通 SPFA 的 20 万点坏顺序反例曾触发 120 秒超时，默认用 1800 点同形态保留退化验证，不把友好规模通过宣称为最坏性能。

`mst_check` 直接引用 Kruskal/Prim，两种权值各 800 图用全部边子集和独立重染色求最小森林。默认另验负自环、重边、断连、LL 极值、选边与原图不变、20 万点 Kruskal 链/星/森林和 2000 点完全图，含大-小-大重建。新套件与升级后的 graph_check 都被普通及 sanitizer CI 自动发现，无额外手动参数。

`mint_check` 保留原 300 轮并默认加入 15 种模数各 350 轮、i128 最小负指数、constexpr 除法和双向比较。
独立参照采用倍加模乘、高位扫描幂、Pascal 与埃氏筛加 Legendre；默认另验 20 万连续运算、20 万位输入及四种模数的 20万-0-1-257-20万表重建。
运行 `./scripts/run_checks.ps1 -Filter mint_check` 即包含全部深测，普通与 sanitizer CI 自动发现，无需额外参数。合数模只测试少量大 k 查询，不承诺高频查询性能，见 [mint 验证](../records/verification/mint-20260907.md)。

## CI 与入口自检

`graph_capacity_check` 对 25 份统一 Graph 体系模板补容量专项：对象一次构造、20 万原点容量、外置图零边预留、四组各 1000 轮小测，核对闭式答案、实际编号和高位工作区哨兵。`floyd_capacity_check` 调用旧 Floyd 的真实 main，独立 Dijkstra 核验 304 组非负无向图及 109-1-109-1 复用，检查本轮矩阵外的哨兵不被清空。两者默认被普通与 sanitizer CI 发现；这是容量专项，不代替各模板完整语义验收，也不把边扩容等同于点表自动扩容。

`graph_check` 的拓扑回归另含 8520710 点预留容量下连续 1000 组 0 到 240 点测试, 独立三色 DFS 判环并核对拓扑序和原图入度。入度副本的有效范围限定为 0..n, 用结构断言防止退回整容量复制, 不用机器相关的耗时门槛。20 万实际点数的大/空/单点/大复用继续由 `completed_graph_stress_check` 覆盖; 两个套件均自动进入普通与 sanitizer CI。

`python scripts/check_layout.py --shell pwsh` 验证真实目录移动后的跳板/include 编译、元数据与链接迁移、新短名发现、只读幂等及重复/歧义拒绝；Windows 另用 `--shell powershell` 验 PS5.1。普通 CI 自动运行。同步不会生成独立暴力或提升学习状态，操作边界见[目录同步](../docs/maintenance/layout.md)。

`check_verification_test.ps1` 自动验证分项范围隔离、摘要/JSON 排版不失效、源码/依赖/执行脚本变化、运行期间变化、旧证据处理、最新失败优先和语法/回归隔离；包含同套件两个模板的真实编译与生成表验收。既有普通 CI 已调用这个脚本，升级测试直接自动生效。维护这部分仍需本地 PS5.1/PS7 双版本验证，运行 `./scripts/check_verification_test.ps1`；状态解释见 [验证指南](../docs/verification/README.md#什么时候需要重验)。

`python scripts/check_docs.py --self-test` 与 `python scripts/check_docs.py` 自动在普通 CI 执行。
前者验证断链/孤页/错锚点等失败路径, 后者分别检查用户 README 与 AI AGENTS 对全部受管 Markdown 的可达性,
以及本地链接和标题锚点; 原文快照只检查能否被找到, 不改写其历史路径。算法说明新增后先运行 make_features。

`misc_check` 默认包含单调队列滑窗四种平手策略、有限前驱 DP、单调栈四种最近关系的小暴力,
以及 20 万长度的六种滑窗/栈形态与大 DP。大规模参照分别为分块前后缀、multiset 和离散化加 Fenwick,
不以两份同构单调结构互拍; 普通及 sanitizer CI 自动采用, 无需额外参数。见 records/verification/mono-20260907.md。

`dsu_check` 默认执行染色/平移暴力、六模式独立约束图 BFS, 20 万点链/星/平衡合并/分块森林,
以及跨模式大—小—大复位、原容量/地址不变和大模数边界; 普通与 sanitizer CI 自动采用。
两份 find 已改迭代, 默认压力无需增栈; 原失败基线和修复证据见 records/verification/dsu-20260907.md。

`leftist_check` 默认执行普通双堆型 20 万点混合修改/惰删/弹空、20 万层父链反例与堆级标记,
持久化双堆型三形态 20 万版本/完整弹空及独立历史快照/旧结点不变检查;
自动进入普通和 sanitizer CI, 不启用源码中注释掉的 RV 扩展。

`bit_check` 默认保留一维/二维各 300 组数组暴力, 另跑 20 万长度/修改查询的独立分块参照,
2000×2000 矩阵的在线矩形交集与离线静态前缀和参照, 以及大—小—大复位和合法大数抵消;
自动进入普通及 sanitizer CI, 不需要额外压力参数。

`pst_check` 默认包含 400 组历史快照分叉和多根路径计数、20 万前缀计数版本,
以及 20 万长度上的 10 万基线/10 万范围分支; 核对旧结点不变与 query/find 零分配,
随原扫描器自动进入普通和 sanitizer CI, 无需额外模式。

`oset_check` 默认包含八件的 20 万规模: 六类集合按正序/倒序/重复值插入后混合操作,
FHQ 序列按循环坐标公式核对变换, 笛卡尔树迭代验证退化链及 BST 映射;
原 set/multiset/vector 小暴力保留, 自动进入普通/sanitizer CI, 无需额外模式。

`seg_check` 默认运行 SegTree/DySegTree 的仿射、可选标记分裂与势能分支对拍,
20 万长度/范围操作、1e12 值域 20 万稀疏插点, 以及 ST 的极值和 20 万点重建;
动态版还验证 clear/init/build 复位和预算复用。全部自动进入普通与 sanitizer CI, 不需另传压力参数。

push、PR 和手动运行触发 CI；Windows 安装卸载、普通回归、sanitizer 和独立压力作业自动执行。回归失败后仍跑语法扫描并上传日志, 正确性用例失败会阻断 CI。快读写在 Linux 也做字节比对。

当前压力作业在 Linux 以 matrix 自动执行 `python3 scripts/check_lca_vt_extreme.py --profile NAME`,
NAME 为 `lca-vt` 或 `completed-graph`, 无需另行手动触发。`lca-vt` (也是默认 profile) 执行:

- 两份 LCA 与两份虚树四种组合, 百万点星/二叉/随机树/链块森林, 必须通过
- 百万点深链在测试子进程 256 MiB 栈下执行, 必须通过
- 两份 LCA 各自用 ASan/UBSan 在 8 MiB 栈下探测 20 万点链; 正常完成记 PASS, 只有退出码 1 且明确诊断 `ERROR: AddressSanitizer: stack-overflow` 才记 STACK_LIMIT。超时、普通段错误、断言失败、其他内存错误及 UB 仍判失败
- `--self-test` 检查失败分类、非零退出和超时，并走两种 profile 的真实参数解析/源码定位/快照/报告路径；依赖快照忽略 Usage、注释和 raw string 中的示例，真实依赖缺失仍失败。CI 先自检再跑完整压力

报告 `.ci-results/stress-lca-vt/summary.json` 记录每项命令、编译参数、栈限制、退出码、状态和源码/依赖哈希, stdout/stderr 单列, 随 `stress-*` artifact 上传。STACK_LIMIT 是已记录的环境限制, 不表示该深链用例通过; 小规模/20 万点默认套件继续在普通及 sanitizer 作业中自动执行。

`completed-graph` 编译 `completed_graph_stress_check.cpp`, 普通及 ASan/UBSan 分别运行
`--large 200000` (8 MiB 栈) 和 `--deep 200000` (256 MiB 栈), 四次都必须成功,
不接受 STACK_LIMIT。默认套件覆盖 Graph/Topo/直径双实现/重心/中心/HLD/连通性四件套;
深链模式只跑递归的树属性/HLD/连通性, 包含大环及圆方树接 HLD。
该 profile 的附件为 `.ci-results/stress-completed-graph/`, 不覆盖 LCA 报告。
手动调用不传 `--report-dir` 时也按 profile 选择各自目录; 编译/测试子进程的
TEMP/TMP/TMPDIR 均指向报告目录, 避免在 WSL 系统盘产生本项目编译临时文件。
旧套件保留随机或穷举语义暴力, 新套件负责结构化规模补验和 HLD 任意根暴力;
范围与限制见 [本轮复审](../records/verification/completed-template-audit-20260906.md)。

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
该自检还实际生成队友 ZIP, 检查压力入口及 CI 配置齐全, 本地日志、编译产物和私人备份没有入包。

`check_deployment.ps1` 验证 v2 到 v3 升级、缺失任务修复、快捷键冲突、手写任务显式迁移、
工作区多个 C/C++ 配置与撤销、修改后保护、真实 ZIP 解压再打包后安装及个人状态排除,
还验证缓存清理预览/保留上限/失败与人工现场保护。CI setup 作业在 PS5.1/PS7 自动运行并上传日志。


## 自动汇总验证现状

runner 自动记录源码与依赖指纹、环境和运行结果, 并生成 [口语概览](../docs/verification/status.md) 与 [AI 明细](../docs/verification/details.md). 只重新判断当前源码是否仍被旧结果覆盖时运行 `./scripts/make_verification.ps1`. 具体范围登记、CI 导入和状态规则见 [指南](../docs/verification/README.md). 原 reliability 表继续只负责静态资产关联.

## 杂项底座深度验证

`hash_check` 用 SipHash 作者的 CC0 参考代码核对 4-8 轮实现，参考路径先核对官方 64 个 2-4 向量。
默认包含 4000 个随机密钥/字节用例、百万字节输入、20 万容器操作及构造性同桶夹具；
计时只打印本机样本，桶阈值只检查固定用例，不能用有限随机实验声称密码安全或最坏常数复杂度。
`tool_core_check` 验 utils/i128/Dcr/Misra-Gries，含 128 位完整上下界、20 万流输入、20 万离散化与小 k 主元素；
`utils_local_check` 独立核对 LOCAL 调试文本，`rnd_check` 补所有整数类别、有限浮点极值与可复现引擎。
这几份无参数套件均由普通/sanitizer CI 自动发现，原 misc 的组装回归保留。
utils 与 i128 是公共依赖，改动后重跑受影响全体，不只跑新增用例；详细结果与哈希来源见 [专项记录](../records/verification/misc-20260908.md)。

## 组合数学四件深度验证

`./scripts/run_checks.ps1 -Filter comb_check` 直接运行循环法、杨辉三角、素数阶乘表和 Lucas 的真实源码。
无参数默认用例含精确 u128 小组合数、整数因子约分、埃氏筛/Legendre、p 进单位阶乘等独立参照；
大规模包括 20 万循环、2000 杨辉表、500 万阶乘表、20 万 Lucas 查询，以及换模数和大—小—大重建。
Lucas 随机测试同时构造非零答案，避免只测到数位不合法的零结果；模 2 有专门回归。
同套件重复 include 四份源码，防重名/守卫退化；由既有普通与 sanitizer CI 自动发现，无额外参数入口。
四份 Usage 的本地编译、指定输入/空输入、展开恢复及旧错误反例另见 [专项证据](../records/verification/combinatorics-20260908.md)。

## bigint 与 rw 深度验证

`python scripts/check_bigint_rw.py --compiler g++ --report-dir .zoi-checks/codex-work/bigint-rw-depth` 使用 Python 3.8+ 标准库大整数提供独立答案；Linux 加 `--sanitize` 验 ASan/UBSan。脚本在指定目录编译、执行并保存带依赖 SHA-256 的 `summary.json`，失败或超时不报通过，原生套件的 runner 记录仍单独生成。

固定种子 20260908，共 5663 组 Python 答案，含 40 块 Karatsuba 临界、除法商估计与规范化、正负号、平方数及前后邻值、十万位进借位/乘除/开方、不等长乘除、连续 Fibonacci 的 gcd/lcm、非负幂与 100000!。不是只用 BigInt 自身乘法验证自己的除法或开方。

rw 原生套件含全部整数类型与 i128/u128 边界、bool、最大有限 double/负零/次正规数、超 4 MiB 浮点 token、六种 ASCII 空白、文本和百万整数逐字节比对。深测另用独立子进程验证退出析构刷写，并从两份源码提取原样 Usage，实际编译、核对正常输入及 EOF。

既有普通/sanitizer runner 自动发现 `rw_check.cpp`；额外 Python 深测由 CI 的 `numeric-depth` 普通与 sanitizer 两个模式自动执行。专项报告从 [验证索引](../records/verification/README.md) 进入；学习状态与测试结果分别维护。

## Trie 双件深度验证

`./scripts/run_checks.ps1 -Filter trie` 运行普通/持久化 Trie 的两套默认测试，由既有普通及 sanitizer CI 自动发现。
原有字符串、整数、区间和第 k 大随机暴力保留，新增五种字符集及六种位深的历史分叉，逐结点验证旧池不变。
版本树路径独立沿 parent 枚举，字符串直接扫描快照，第 k 大展开小笛卡尔积排序。
默认还跑 20 万长串三轮复用、20 万 HB30 版本两轮与大区间全扫描，28 亿多根中间计数及抵消、400 亿配对的第 k 大分界。
容量套件覆盖 HB=0..63 的精确预算、根/哨兵区别、空串复制根与多测复用；契约外负数、非法字符、逐值为负的差集不当作合法输入。
专项记录见 [Trie 报告](../records/verification/trie-20260908.md)，其中 1 MiB 栈对照是本地额外诊断，默认 CI 不主动修改栈限制。

## 数论基础与两种筛

`./scripts/run_checks.ps1 -Filter number` 自动运行 `number_boundary_check` 与 `number_sieve_check`，两套默认 main 均由既有普通/sanitizer CI 发现，无额外压力参数。
两筛按千万—0—1—2—257—百万—千万重建；小表以试除核对，大表由独立 Sundaram 奇合数枚举逐点核验，并以唯一分解检查最小质因子。
区间筛覆盖百万宽区间及约1e12的平方数前后，独立试除负责高坐标短区间。约数统计在1..20万逐点与倍数累加参照比对。
取整除法覆盖20万组完整64位有符号随机值与全符号边界，参照提升i128；整数开方以整数二分验40万组级数据及LL上界。
除零与LLONG_MIN/-1因结果无定义或超出返回类型不作为合法调用。莫比乌斯笔记未列入可运行验收；完整说明见[数论报告](../records/verification/number-theory-20260908.md)。

## 手册自动检查

样例含七层嵌套目录；PDF 检查读取实际字号及位置，验证目录页和正文都逐层收敛、子级缩进正确且目录行不重叠。深目录也必须出现在目录页中，不能用截断深度掩盖样式问题。

`./scripts/check_booklet.ps1` 在 PS 5.1/7 检查发现和 Markdown 转换，不要求 Typst；`-Render` 实际编译样例与 1200 行增长/奇数页场景。构建审计检查实现连同说明各占独立页段，PDF 检查逐项核对文件夹与源码的书签层级、纸面目录文本和起页。共 18 次构建包含新增/改名目录、纯空目录筛选、旧题记不再读取、同目录源码全部收录与逐实现起页、目录与唯一同名源码合用标题，README 主副标题、自动编号小节、公式标签、连续提示与 90 行长表跨栏，以及行内/独立/表格公式、分式/上下标/求和/伸缩括号和代码中的美元符号，以及未知命令、错误分组、未闭合公式、原始 Typst 注入的拒绝路径。诊断放 `.zoi-checks/booklet-test-*`，按现有工作区清理约定管理。

`python scripts/check_booklet_pdf.py <PDF> [--root <样例库根>]` 需 pypdf，独立将完整 PDF 的目录清单与实际算法目录树对账，核对目录/说明标题、编号正文、数学字体、增长行序与 MIKU 页脚。对样例 PDF 另读取实际绘制的主副标题、小节、公式标签和提示字号，核对字面量保留，并自动检查同目录 `long-manual.pdf` 的 90 行顺序和跨栏/页重复表头；构建另核对数学节点数量，视觉抽查负责确认符号含义与布局。CI booklet 作业每次自动完整生成并执行这些检查，成功附件是对应提交的手册；操作见 [手册指南](../docs/booklet/README.md)。
