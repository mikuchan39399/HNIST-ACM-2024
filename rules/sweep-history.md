# 清扫历史代记

## 2026-09-08 Codex: 数论两筛与现役四件

- 新增欧拉筛最小质因子/分解与位压缩埃氏筛，区间筛复用埃氏筛；约数统计改返回值、除法补守卫与合法域，整数开方保留整数校正，六份完整Usage
- 默认两套number对拍覆盖千万表复建、百万宽区间、接近1e12平方边界、40万组级开方、20万约数答案与20万完整64位除法；Windows2/2及Linux ASan/UBSan2/2通过，六份Usage和联合链接通过
- 莫比乌斯笔记补反演方向与改题贡献，仍非可调用引擎；mint保持已验收实现；两种筛按用户确认记“学过待复习”
- 同步catalog/跳板、设计分类/汇总、功能与验证文档；证据见 [数论验收](../records/verification/number-theory-20260908.md)，现场归档后清理；未新建算法README、未重建PDF、未提交或推送

## 2026-09-08 Codex: Trie 双件深度对拍与 Usage

- 持久化字符串插入改为迭代复制路径，多根计数使用 LL，复现并修复1 MiB栈下20万长串溢出与28亿累计溢出；普通 clear/const查询和容量说明整理
- 原有两套默认测试补历史分叉与旧池不可变、20万长串/版本、全64种HB精确预算、400亿配对第k大；Windows2/2与Linux ASan/UBSan1 MiB栈下2/2通过
- 两份完整Usage实际编译、展开独立运行、恢复核对；验证表、功能说明、测试资产与客观进度同步，个人学习状态未推断
- 证据见 [Trie验收](../records/verification/trie-20260908.md)，任务目录归档后清理；未新增算法README、未重建PDF、未提交或推送

## 2026-09-08 Codex: 杂项六件与 SipHash-4-8

- custom_hash 按用户防卡优先要求升级作者保守版 SipHash-4-8, 独立参考/官方向量/同桶夹具/20 万容器验收, 支持整数、字符串、整数对; 不承诺绝对无碰撞
- 修复独立 i128 最小负数读入溢出、rnd bool 编译和浮点大区间、utils fast_io 多翻译单元重定义; Dcr 补 clear/排名契约, Misra–Gries 补守卫与小 k 使用说明, 六份完整 Usage 通过展开恢复
- Windows 31 回归+99 编译共 130 项全过; 公共 utils/i128 影响的全部默认 31 套件 Linux ASan/UBSan 全过, 旧额外百万压力 profile 未重跑
- 已同步验证范围/两层表/资产表/功能说明/客观进度, 证据见 [杂项验收](../records/verification/misc-20260908.md); 参考代码与许可留正式对拍, 临时现场归档后清理, 未新增算法 README、未提交或推送

## 2026-09-08 Codex: 组合数学四件深度对拍与 Usage

- 循环法、杨辉三角、素数阶乘表、Lucas 保留运行时模数, 明确与 mint 的重合和不可替代范围; 加守卫、独立对象与完整 Usage, Lucas 复用 PrimeComb
- 复现并修复 Lucas 模 2 死循环、循环乘法与杨辉加法溢出、模 1 首列取模; 阶乘法明确 n<p, 旧全局接口迁移说明随功能文档同步
- 新 comb_check 默认约 3988 万次比较, 包含 500 万阶乘表、2000 杨辉表、20 万 Lucas 查询与独立参照; Windows 普通及 Linux ASan/UBSan 全部通过, 四份 Usage 跳板/展开/恢复通过
- 证据见 [组合数学验收](../records/verification/combinatorics-20260908.md), 登记/两层表/资产表/功能说明已同步; 个人学习状态未推定, 未新增算法 README, 未提交或推送

## 2026-09-08 Codex: BigInt 与 rw 深度对拍

- 按固定工具内核处理，保留接口并写完整可运行 Usage，不新增 README。修复 BigInt 商高位零和最小 i128 转换；修复 rw 浮点 token 越界、最大 double 输出截断，补 bool 和完整 ASCII 空白。
- 5663 个独立 Python 大数答案，十万位乘除/开方、100000!；rw 百万整数、4 MiB 跨界、浮点/EOF/文本/析构刷写。Windows GCC15 普通、WSL GCC13 普通与 ASan/UBSan 全部通过；两份 Usage 原样编译和空输入核对通过，另验展开编译及正文恢复。
- 正式 bigint/rw/misc runner 均通过，Python 深测接入普通/sanitizer CI，验证范围和两层表、rule、功能与测试说明同步；学习状态保留待确认。临时产物归档后清理；未提交推送。见 [专项记录](../records/verification/bigint-rw-20260908.md)。

## 2026-09-08 Codex: 手册说明统一排版

- 用户确认树中心样张后，推广到三份其他获准 README，并正式接入通用 Markdown 转换器；深绿标题、编号小节、带标签公式框、细线表格和竖线提示，保留算法条件与接口。
- PS 5.1 / PS 7 各 18 次构建通过，PS 7 实际渲染；样例 PDF 核对字号、字面量、90 行长表和 1200 行增长。正式手册 141 页、106 个独立实现页段、4 份说明、32 个公式、MIKU 第 39 页通过，四份说明实际页面已目视检查。
- 打印/维护指南、rule、用户功能表、测试说明与记录同步；文档双入口及功能生成检查通过。临时样张与本次测试产物清理，保留正式 PDF/Typst；未提交推送。详见 [排版与验证记录](../records/tooling/booklet-style-20260908.md)。

## 2026-09-07 Codex: 树上倍增优化建图

- 新增 TreeGraph 与 treeGraph.h, 复用外置森林 LCA 和既有 Graph; 每条路径最多四段、路径间最多八边, 定位仍为 O(T + log n), 中继容量单独预算。保留原 LCA、Graph 和最短路实现。
- 先独立对拍与规模验证再整理注释; int/LL/Empty 各400随机森林、5万点百万操作深链与20万点星/二叉/孤立森林通过，默认接入现有普通与 sanitizer CI。
- Windows 正式回归、Linux GCC13 普通与 ASan/UBSan、完整 Usage 与展开恢复通过; Linux 普通峰值 RSS 约230.1 MiB。深链使用闭式祖先提供者，不改原 LCA 栈契约。
- 功能、分类、跳板、两层验证表及学习进度同步；用户确认正在学且不配 README。本次临时产物在证据保存后清理，正式对拍源码保留。未提交推送，见[专项验证](../records/verification/treegraph-20260907.md)。

## 2026-09-07 Codex: 验证状态分项失效

- 用户委托解决全表指纹噪声；源码/执行脚本按套件，验证范围按模板-套件分开绑定。无关登记与 summary/JSON 排版不失效，真实范围和依赖变化仍要求复验；明细显示具体原因。
- 旧证据保持原样，通过实际重跑全部 25 套件建立新版 Windows 记录，全部通过；45 个已登记模板通过、32 个未登记仍未知。
- PS5.1/PS7 分项隔离与生成表自检通过，两版 runner 9/9 通过。原 CI 自动调用升级后的检查，规则与用户说明同步。未提交推送，详见 [实施证据](../records/tooling/verification-scope-20260907.md)。


## 2026-09-07 Codex: mint 审查与深测

- 用户确认保留基础模运算和内置组合数接口；修复最小 i128 负幂取负溢出、constexpr 除法缺失及反向整数比较编译失败。标准头文件/命名空间交给 utils，补契约与完整 Usage，不新增 README。
- 原 mint_check 默认升级 15 种模数、全部整数边界、20 万运算/位数/表重建；独立倍加、Pascal/Legendre 与表核对。最终 Windows 普通回归与 Linux ASan/UBSan 均通过，现有 CI 自动采用，无额外压力参数。
- 规则/功能/学习与两层验证表同步；正式手册 140 页，目录和 MIKU 核对通过，mint 两页已目视检查。合数 comb 适合少量查询，性能数字及全部证据见 [mint 专项](../records/verification/mint-20260907.md)。保留此前未提交改动，本轮未提交推送。


## 2026-09-07 Codex: 单源最短路、负环与最小生成树

- 原 graph_check 最短路段升级独立 i128 Floyd，4800 图全合法源点、20 万规模、多源/断连/负权与 1800 退化；两份判环的 LL 溢出经用户授权改用库内 i128。SPFA 坏顺序超时基线保留并明确最坏性能限制。
- 建成 Graph 版 Kruskal/朴素 Prim，共用最小生成森林输出；mst_check 独立枚举 1600 图，另验 20 万点 Kruskal 与 2000 点完全图、返回森林和大-小-大复位。两套默认接入现有普通及 sanitizer CI。
- Windows 普通、Linux GCC 13 ASan/UBSan、八份 Usage 独立/联合与展开恢复验证通过；规则/功能/分类/验证表同步，八项学习状态按用户确认记录学过待复习，没有新增模板 README。手册 140 页、187 目录、105 独立页段和四份说明，MIKU 与补写留白保留。详见 [专项证据](../records/verification/shortest-mst-20260907.md)。未提交推送。

## 2026-09-07 Codex: 模板 README 授权纠正

- 用户明确目前仅保留线段树插件和树属性三件套；核查实际十三份，撤出其余九份并合并存入 [历史快照与验证](../records/tooling/readme-scope-20260907.md)。算法、目录和学习确认保留。
- rule 与 AI 入口明确已有文件不等于授权，未获用户明确同意不创建说明；功能目录和现役链接同步。生成器仍动态发现同目录 README，不硬编码名单。
- 正式 PDF 137 页，187 个目录、103 个独立页段、4 份说明和第 39 页 MIKU 均通过检查；双入口 98/98 可达，分类与 features 检查通过。未提交推送。

## 2026-09-07 Codex: 七组常用线段树插件

- 升级原加和插件并新增六份代表代数，覆盖加/赋值/仿射平方和/异或计数/01 连续段/最大子段/等差加法；独立命名空间、utils 引用、完整 Usage，普通与动态树接口不变。旧势能专题不冒充本轮已验。
- 真实插件套件默认进入普通及 sanitizer CI，双引擎小数组暴力、每组 20 万规模与复位通过 Windows GCC 15.2、Linux GCC 13 ASan/UBSan；七份 Usage 单独编译运行通过。
- 用户确认配说明、七组均学过待复习；规则、功能、分类、学习与验证表同步。修复 Usage main 导致插件不入册及审计重定向 UTF-8 解码，双版脚本各 17 次构建通过；正式 PDF 138 页，MIKU 与逐实现预留保留。详见 [本轮证据](../records/verification/seg-plugins-20260907.md)。未提交推送。

## 2026-09-07 Codex: 恢复实现目录和排版预留

- 修复源码标题不进目录造成左偏树两版、平衡树八件缺失；家族内实现可并列，不为 PDF 套多余文件夹。187 个算法目录和 97 份源码/笔记/插件形成 268 个目录条目。
- 恢复逐实现另起页、说明紧随与页尾补写空间，祖先标题随首项起页，待建目录分段集中排；撤掉短题记配置、渲染和分类引言。正式手册恢复到 132 页，唯一 MIKU PDF 已更新。
- 两版 PowerShell 各 17 次构建、实际 PDF 目录/独立页段/增长/层级检查通过，80 份打印 C++ 正文不变。规则和用户指南已同步，现有 CI 自动执行新增检查；未提交推送。详见 [本轮证据](../records/tooling/booklet-readme-20260907.md#恢复逐实现预留与源码目录)。

## 2026-09-07 Codex: 修正目录父子视觉层级

- 修复前两级单独覆盖、深层回退默认样式导致子级更显眼的问题；目录与正文标题共用深度样式，字号/字重/颜色逐层收敛，目录逐层缩进并保留行距，不截断深度。
- 七层样例实际 PDF 字号、缩进和行距检查通过；两版 PowerShell 各 17 次构建通过。正式 PDF 仍 60 页、187 个目录、12 份说明，80 份打印代码完整保留，MIKU 仅第 39 页。
- 已同步规则和用户指南，测试沿现有 CI 自动执行；未提交推送。证据及新产物指纹见 [记录](../records/tooling/booklet-readme-20260907.md#目录视觉层级修正)。

## 2026-09-07 Codex: 按用户纠正恢复目录驱动的手册

- 用户明确“算法目录存在就代表已建或未来目标”，纠正此前不印空目录的误解；确认纯对拍目录不印。实际扫描 187 个算法目录全部入册，105 个空叶目录补 .gitkeep 保证克隆与 CI 可携带。
- 方向内自然连排，唯一同名源码与目录合用标题，插件回到所属目录，12 份说明与数学排版保留；可选短题记不充当收录名单。九方向分类去掉逐行套话，保留 213 项归类与家族短句。
- 两版 PowerShell 各 17 次构建通过，独立 PDF 检查真实目录集合、1200 行增长、数学字体与 MIKU；80 份打印代码完整保留，分类/导航/features/actionlint 检查通过。正式整本为 60 页；用户规则与指南同步，未提交推送。
- 决策更正、检查范围及正式产物指纹见 [验证记录](../records/tooling/booklet-readme-20260907.md#用户纠正目录驱动的纸质自动化)。此前“不印空目录、每目录换页”的决定仅保留为历史，不继续执行。

## 2026-09-07 Codex: 入册说明赛场化与公式排版

- 润色全部 12 份现有入册 README，移除学习资源、测试/CI、维护/迁移叙述，保留选择、性质公式、接口和易错边界；算法实现未改。rule 与用户指南、功能表同步，新建说明仍由用户决定。
- 增加受限 TeX 数学转换与数学字体，独立公式采用浅灰底；未知命令/错误分组及时失败，代码中的美元符号保持原文。自动 CI 沿现有入口获得新测试。
- 两版 PowerShell 各 13 次构建及非 ASCII 数字专项通过；1200 行增长 PDF、43 个数学节点、80 份打印代码一致性、导航/分类/生成目录/actionlint 检查通过。正式 PDF 重建为 92 页，第 39 页 MIKU 保留，抽查公式未越栏。
- 范围、限制、文件占用处理和产物指纹见 [验证记录](../records/tooling/booklet-readme-20260907.md#后续赛场文字与数学排版)。保留此前未提交改动，本轮未提交推送。

## 2026-09-07 Codex: 路线与说明分离及手册自动化

- 77 份路线/空壳 README 合并到九方向路线，12 份现有使用说明留在源码旁；213 项分类按方向与家族拆分，92 个 catalog 目标无遗漏，学习确认保留。新建模板说明由用户决定，相关规则和用户入口同步。
- booklet 自动发现同目录 README，跟随最后一个选中同目录实现，只印一次；同族连排，不继承父子目录说明、不打印路线/空目录。Markdown 转成紧凑正文，筛选保护正式文件，正式 PDF 更新为 92 页并保留物理第 39 页 MIKU。
- PS 7 Render 与 PS 5.1 发现各 8 次构建通过，1200 行增长及奇数页通过；独立 PDF 文本检查、代码一致性、双版本目录生成、全库分类/导航和 actionlint 通过。CI 自动编译并提供当次手册附件，尚未推送验证线上。
- 详细范围、日志、产物指纹和已知边界见 [验证记录](../records/tooling/booklet-readme-20260907.md)。本輪未改算法实现，不重跑无关对拍；此前未提交改动保留。

## 2026-09-07 Codex: 文档双入口导航审查

- 用户询问 AI 与用户能否被引导到全部文档。基线按实际 Markdown 链接检查 161 份受管文档: 用户 README 有 49 份不可达, AGENTS 有 50 份不可达; 主要是家族/父级 README、验证索引、备份/训练/比赛入口及归档原文, 不是所有正文都缺失
- make_features 新增按方向自动收集算法 Markdown 的说明导航, 补齐源码目录原本不导向 README 的缺口; 根算法 README 不再把全部说明称作占位。新说明触发 -Check 过期, 无需手填第二份说明目录
- AGENTS 补用户首页、算法说明、设计分类与专项验证的按任务入口; docs 导航补专项证据、悬赏、备份和训练/比赛入口; records/verification 新增 10 份专项报告索引, 归档目录 README 列出原文快照。未修改 records/tooling/rule_history.md 或冻结快照原文
- 新 scripts/check_docs.py 检查本地目标/ATX 锚点及用户、AI 两套可达性, 不把目录链接等同于其全部文档可达; 原文快照出链按旧位置解释而免验, 快照仍须有入口。外部网站、忽略目录私有产物及正文理解不在保证内。两处数组记号补行内代码, 避免 Markdown 把它们渲染成相对链接
- Windows 与 Linux 的全库检查均 161/161 可达、断链/错锚点为 0; 8 组故障自检均通过。自检发现 Windows resolve 会掩盖大小写差异, 已保留链接原始拼写检查; 目录无 README 仅作有效目录链接, 不误报文件缺失
- 扩展 check_inventory_test 为 5 组共 17 次生成命令, 包含说明发现/路径转义/过期只读/不变不重写; PS7 与 PS5.1 均通过。两版 features -Check、reliability -Check、actionlint 与 git diff --check 通过。现场在 .zoi-checks/inventory-test-abd76f39ab5847f1a64247142669d68b、inventory-test-18482af1994a485cacf395a981840d55, 基线日志 .zoi-checks/codex-work/docs-nav-before.log
- CI 普通作业已接入导航自检和全库检查, 随 push/PR 自动运行; rule、功能表、维护说明与测试说明同步。只修改文档/导航工具, 未重跑算法对拍; 当前尚未提交推送, 不称本次线上检查已运行

## 2026-09-07 Codex: 全库逐项三类设计表

- 将 [模板设计分类](../docs/features/template-design.md) 的剩余范围分组替换为全库逐项表, 九方向 213 项各归一类并附设计理由: 固定引擎 109、启发式骨架 61、固定内核加题目骨架 43
- 身份独立标注: 75 现役模板、17 登记笔记、14 插件/例题、1 未维护代码、1 已撤下例题历史行、105 待建知识点; 同算法不同实现/笔记/目录分列, 不把 213 称作独立算法数
- 从最终 Markdown 反向核对九方向进度表、catalog 的 92 个目标、算法目录 107 个非对拍代码/笔记和 140 个非例题/对拍叶子目录, 遗漏 0、重复 0、链接通过; 本地校验现场 .zoi-checks/codex-work/check-classification.py 与 classification-check.json
- 同步 rule 的逐项维护要求、功能总览和进度总表导航; features/reliability -Check、git diff --check 通过。未改算法、catalog、验证登记或个人学习状态, 无需重跑算法对拍; 未提交推送

## 2026-09-07 Codex: 启发式模板设计与剩余条目分类

- 按用户明确裁决写入 rule §3.1: 数学推导、DP、树上启发式合并等弱模板强思维算法以激发手感、快速回想和活化思维为目标, 用显式骨架/公式/代表例呈现; 不改变独立对拍、先验后注释与自动 CI 要求
- 按实际换题修改处区分固定引擎、启发式骨架、固定内核加题目骨架; 核对 catalog、剩余悬赏及相关学习目录, 分类现役与待建条目, 数学数值内核等不因学科归属被强拆
- [用户分类页](../docs/features/template-design.md) 说明普通分块偏启发式、莫队保留调度、块状容器偏固定接口; 同步功能总览、文档导航、分块家族入口与悬赏板引用
- 本轮仅设计文档, 不更改算法源码、catalog、验证登记或个人学习状态, 不提前宣称分类项已完成改造; 本地文档链接、features/reliability -Check、git diff --check 通过, rule 为 333 行, 无需重跑算法对拍; 未提交推送

## 2026-09-07 Codex: P11 单调队列与追加单调栈

- 用户批准显式 while 骨架并追加单调栈: 队列改为 mono_window 下标答案和 mono_dp 示范, 移除旧类/空 init/批量最值接口; 新增 monoStack 跳板, 保留可直接按题改的弹栈循环
- 先跑旧队列增强基线、新滑窗/DP 基线、新单调栈基线, 再润色注释和 README; 四种平手/大小关系的小暴力, 20 万长度和六种形态, 大规模分别用分块前后缀、multiset、离散化加 Fenwick 独立核对
- 最终 Windows 23 套回归零失败, Linux 8 MiB 普通及 ASan/UBSan 通过, 两份提取 Usage 重复 include 通过, 双 PowerShell 生成物检查通过; [完整证据](../records/verification/mono-20260907.md)
- catalog、跳板、规则、用户功能表、测试说明和验证两层表均同步; 原 misc 默认 main 自动进入普通/sanitizer CI。两项学习状态按用户确认记为学过待复习
- P10/P11 仍未提交推送; 优先队列已完成, 其余现役覆盖仍见工作板, 不声称全库最终覆盖已经完成

## 2026-09-07 Codex: P10 并查集双件最终验证

- 原两份 find 在 Windows 20 万点父链上栈溢出; 用户批准改迭代路径压缩并保留合并方向, 同时批准 WDSU 补 init(n,mod=0) 复用构造容量
- 修复大模数归一化额外加法的有符号溢出, 保持全部距离中间运算在 LL 内的契约; 先补六模式独立 BFS、原染色/平移暴力、20 万点四形态和跨模式复位, 完成默认栈增强基线后再润色注释
- 统一 utils 依赖, 补两份可运行 Usage 和并查集 README, 明确第二根挂第一根、单次线性、忽略矛盾、INF 碰撞与容量边界
- 最终 Windows 23 套回归零失败, Linux 8 MiB 栈普通/ASan+UBSan 通过, Usage 重复 include 和双 PowerShell 生成表检查通过; [完整证据](../records/verification/dsu-20260907.md)
- 默认用例自动进入普通/sanitizer CI; 同步 rule、功能表、测试说明、覆盖登记与两层表, 学习状态保留待确认。未提交推送, 下一件 P11; 前批 cae5292 五作业在线成功另见发布记录

## 2026-09-07 Codex: 队友部署与缓存治理（9 月 6 日启动）

- 移除误跟踪的私人旧安装状态；安装 v3 兼容 v2 升级，管理三份配置快照、九任务及可用快捷键，支持显式接管本库手写任务；真实用户配置已升级并保留原手册任务
- 新增 zoi-configure/doctor 修复项目 includePath 覆盖并诊断补全；已知尖括号跳板支持展开与原拼写恢复，零块输出增加说明
- zoi-package 默认生成正式 docs/releases 时间戳 ZIP；修复绝对路径过滤漏源、指南漏包和 stage 临时占用误报，私有状态及产物不入包
- 成功测试现场每类保留三份，清理加独占锁，未知/失败/人工现场保留；115 个旧目录先归档校验后移除，下载包与临时运行时按项清理
- Windows 全库 112 项零失败；PS5.1/PS7 安装各 7 组、部署各 20 命令通过；展开三环境覆盖 18 组，Linux 入口故障 9 项通过；生成表检查和 actionlint 通过
- CI 自动新增双 PowerShell 部署验证，rule、功能表、用户指南、测试说明均同步；[完整记录](../records/tooling/deployment-audit-20260906.md)。未提交推送，未声称队友 UI 或本轮远程 CI 已验收；下一项 P10

## 2026-09-06 Codex: P09 左偏树双件最终验证

- 20 万递减值连续合并后查最早点, 原 find_root 在 Windows 栈溢出; Linux 8 MiB ASan 同探针通过。用户批准仅查根改迭代路径压缩, 左偏树合并保留递归, 默认套件保留反例
- 补普通大小根堆 20 万点/10 万混合修改、惰删弹空、10 万堆级操作; 持久化双堆型三形态各 20 万版本/弹空, 300 组历史快照与旧结点字段不变、自定义载荷、共享合并及复位, 保留原两域暴力
- 先完成修复后增强基线 Windows/Linux 普通及 sanitizer, 再统一 utils、润色注释与 Usage; 纠正查根复杂度和 merge_raw 独占条件, 补堆 README 并修复父目录过期占位说明
- 最终 Windows 23 套回归、Linux 普通/ASan+UBSan 8 MiB 栈、Usage 重复 include 及 PS7/PS5.1 生成目录检查通过; [完整证据](../records/verification/leftist-20260906.md)
- 同步 rule、用户功能页、覆盖登记/两层表与测试说明; 所有新用例自动进入默认普通/sanitizer CI, 学习状态无新确认保持原值, 未提交推送; 下一件 P10

## 2026-09-06 Codex: P08 树状数组三引擎最终验证

- 保留一维/二维各 300 组数组暴力, 默认 bit_check 新增 20 万长度两轮各 20 万修改/查询的独立分块参照, 2000×2000 在线矩形交集及 10 万任意修改/10 万查询的离线前缀和; 覆盖退化行列、正负大数和大—小—大复位
- 原模板 Windows 与 Linux 普通/sanitizer 增强基线先通过, 再整理注释; 无算法错误, 保持接口, 标准头和命名空间集中到 utils, 明确坐标乘积等全部中间值须不溢出 LL, 补完整 Usage
- 最终 Windows 23 套全部 PASS; Linux GCC 13.1 普通约 1.89 秒、ASan/UBSan 约 6.60 秒, 8 MiB 栈通过且哈希稳定; PS7/PS5.1 生成目录/资产表检查及 Usage 重复 include 编译通过
- 同步 rule、功能页、README、测试说明和验证登记/两层表; 默认无参数运行自动进入普通/sanitizer CI, 无需增加工作流参数; 学习状态未有新确认, 保留原值
- 证据见 [P08 记录](../records/verification/bit-20260906.md); 本轮未提交推送, 不把本地结果当在线 CI; 保留 P05-P07 改动, 解除领取, 下一件 P09 左偏树双件

## 2026-09-06 Codex: P04 SegGraph 最终对拍与注释

- 原 300 组小测试两边共用 Dijkstra, Empty 只比数量。补 400 组独立矩阵 Floyd 全点对、BFS 可达性和 SCC 等价关系; 原点及返回中继点均作源, 加零权/负权 DAG/INF-1 固定用例
- 默认套件加入 20 万点/操作及 1/2 个原点新增 20 万中继、非对齐区间、SCC 分块和大小交替, 所有验收模式由原扫描器自动进入普通/sanitizer CI, 不加人工命令
- 旧 SegGraph(1,200) 连续造点经 ASan 复现点表堆越界。用户批准第三参数 max_extra 独立预算, 默认 max_n, 新建处检查预算, 保留两参数调用; 儿子表只分配骨架容量, Graph 不改
- 严格先完成 Windows/WSL 普通与 sanitizer 压力, 再润色容量、编号、时间空间和 Usage, 移除模板自己的 using namespace std。补 README 的两棵树机制、容量公式、下游限制, 同步 rule 与用户功能页
- 最终 Windows 23 套回归通过, Usage 说明最后调整后单独重跑 SegGraph 通过; Linux GCC 13.1 普通及 ASan/UBSan 在 8 MiB 栈通过且指纹稳定; 提取 Usage 重复 include 并核对输出通过。证据见 records/verification/seggraph-20260906.md 与 stress/seggraph-20260906.json, 两层表由 runner 自动刷新
- 用户确认正在学, 进度行与摘要已记录。未修改 Dijkstra/SCC 实现, 只登记本组合覆盖。未提交/推送, P04 移档解除领取, 下一件 P05
- 顺带核实前次已推送 c41a6c2: GitHub run 34014976649 的 setup/regression/sanitizer/stress(lca-vt)/stress(completed-graph) 五作业全 success, 五份附件存在且未过期; 不作为 P04 本轮在线结果

## 2026-09-06 Codex: 全库设计同步与工作区审查

- 检查 74 个现役 catalog 条目、23 套对拍、116 份当前 Markdown, 结合脚本/CI 实现与定点复现; 不是全算法极限验收。完整结果及逐项快照见 records/tooling/repo-audit-20260906.md
- 修正规则免验旧条款、Info 可选方法误述、工作板旧留言/缺漏、文档旧指纹和数量说明, 用户功能页去掉重复流程并链接唯一指南。18 个存量件缺防重, Misra–Gries 重复 include 已复现重定义; 随最终逐模板验证处理, 未批量改引擎
- 修复队友 ZIP 漏压力入口依赖的 CI 配置, 在现有 check_setup 内加入真实打包检查, 自动由既有 Windows CI 双 PowerShell 运行。PS7/PS5.1 均 7 组 24 命令通过, 不触碰真实安装配置
- 压力入口默认报告按 profile 分开, 子进程 TEMP/TMP/TMPDIR 归库; 自检及 Linux 两 profile 默认目录并行运行通过, LCA 两个默认栈探针仍仅记 STACK_LIMIT。专项原始摘要存 records/verification/stress/*-workspace-20260906.json
- 库内 .zoi-checks/codex-work 已有并继续使用, 约定提升至 collab 且由 AGENTS/rule/功能页路由。长期结论进规则/工作板/records, 不只留忽略目录或聊天。本次已知两个 C 盘项目现场为空, 未删无关文件、未声称释放空间, 不改变应用缓存或 WSL 存储位置
- VBCC/BCT 合并涉及已定独立实现取舍, 登记整表指纹优化涉及证据失效语义, 已列收益和代价, 本轮保留。资产和功能 -Check 在 PS7/PS5.1 通过, PowerShell 语法与 actionlint 通过, 生成表无需空改
- 未修改算法实现或个人学习状态, 未提交/推送; 解除领取, 下一件仍 P04, 远程 CI 待获准推送后验证

## 2026-09-06 全库审查迁出过期留言 (仅历史, 不再执行)

原工作板留言: 2026/9/6 GLM → astra: 用户指令已落板——悬赏池新增“优先队列”11 件
(P01-P11): 图论+数据结构已有套件的验证登记, 按序逐件领(登记行为→跑带指纹回归→
回填 status.md), 只登记不新写对拍; 线段树合并分裂与套 FHQ 移入暂缓区,
图论/DS 登记完再排。你正在跑的 verification 回填如与此重叠, 以你的进度为准,
板上序号跟着实际完成情况移档即可。

现行用户要求已变为先独立及目标规模验证、补足测试并自动接入 CI, 再润色注释;
本段仅保存当时交接, 不覆盖当前工作板和 rule 第 9 节。

以下为 GLM 建板及 astra 接手时的原文快照，保留追溯；其中勾选与锁定不代表当前状态，当前结论只看 sweep-board.md。

# 清扫状态板 (token 有限者的外挂记忆; 用法见 rules/collab.md)

打勾纪律: ☐ 待办 → 🔒 进行中(署名) → ✅ 完成(记提交号或结论);
发现问题开 ❌ 行。一次只做一个范围内的一件事。

## astra 2026/9/5 工业化大扫除 — 历史进度(种子状态由 GLM 依侦察代记,
astra 接手后自行更正补漏)

- [x] 回归入口重写: run_checks -Mode/-Sanitize/双超时(进程树)/summary
      报告/.zoi-checks 独立构建目录
- [x] 入口自检器 check_runner.ps1 + check_process.ps1(八种故障注入,
      验证入口不假绿)
- [x] CI 双作业(回归 + sanitizer), 失败仍跑语法扫并上传日志
- [x] 宪法分卷: rules/pitfalls.md(坑位表整迁) + rules/workflow.md
      (§10/13 整迁) + scripts/checks.md + rule.md 阅读顺序路由
- [x] 引擎修复: 区间筛质数(乘法溢出 + 浮点 sqrt→floor_isqrt + 守卫 +
      Usage) | 整数开方 | 约数个数及约数和公式法 | 字典树(构造契约注释)
      | 可持久化字典树 | 树状数组 + 二维 | 泛型线段树
- [x] 新对拍: trie_capacity_check | number_boundary_check;
      misc_check 大改; bit/pst/hld/conn/mint/trie 适配 -Werror
- [ ] -Mode All 全库 -Werror 终态未验证(GLM 后台发射的验证无输出)
- [ ] number_boundary_check.cpp 头部三件套 include 重复粘贴两遍
      (守卫化了, 无害待清)
- [ ] rule.md 头部 L9/L10 归档句重复; "最近一轮"未更新为本轮
- [ ] ❌ 分歧待裁: SCC 等 conn 四件注释二改删掉了鸭子协议语义
      (SegGraph 带权图直喂 / 输出恒无权), 与 GLM 精炼版并存两案
- [ ] 连通性相关/README.md 已删: booklet 组导语 10→9, 手册重建待验证
- [x] 对拍审查报告(原 对拍清单.md 头部 +18 行, 已由 GLM 核对并入本板;
      原文件经用户裁决删除, 使命由 run_checks 输出与 summary.json 取代):
  - 补齐: 树状数组(满容量/单行单列/首尾更新/1e12 量级/大-小-大复位) |
    HLD(单点/满容量链/星形/平衡树, 复用爬父暴力) | 字典树(空串/62 字符
    集/LLONG_MAX/第 62 位/重复值/空差集/kth 越界约定) | 主席树(任意历史
    版本分叉修改再随机回访, 原来只改最新版) | mint(64 位随机源+MOD-1,
    128 位随机避开最小负数取反) | 快读写(64/128 位极值/EOF/科学计数法/
    无终止符/跨 4MiB 缓冲/跨平台字节比对) | 入口(八种故障必失败+正常用
    例防"一律失败"伪自检)
  - 缺口如实保留: 线段树合并分裂三件 / 线段树套 FHQ / 树上背包两件 /
    备用 LCA 三件, 待实际使用时补独立暴力; utils/i128 可能被间接引用
    不判零覆盖; 轻件不为消灭黄牌强塞测试
  - 确认并修复的原入口五病: 拼错 Filter 假成功 | 无超时 | 警告不阻断
    (现 -Werror) | 回归失败跳过语法扫 | Linux 不跑快读写

## 当前领取

- [ ] 🔒 astra：单调队列 init 越界修复与 misc 定点验证；涉及单调队列.cpp、misc_check.cpp。

## 待办池(善后者按范围领, 一次一件; 领取时改 🔒 署名)

- [ ] 网络流: Dinic + MCMF 入库(§11 最优先缺口), 对拍暴力 = BFS 增广
- [ ] AC自动机: 对拍(暴力 = 逐模式串 KMP 匹配)
- [ ] 后缀数组 / 后缀自动机: 对拍(暴力 = 后缀排序 / 逐子串)
- [ ] 计算几何: 凸包 / 半平面交 / 二维基础 边界对拍
- [ ] TEST GAP 22 件无直接对拍引擎, 按域分批补(先数学数论系)

## 铺底日志(GLM 日常, 简记)

- 2026/9/6: 搭建本协作体系(AGENTS.md / collab.md / sweep-board.md),
      种子状态录入; astra 五小时大扫除期间未动其活跃区。

## 对拍清单.md 原始验证记录(快照恢复; 该文件于 271f6e6 删除)

以下为重构期逐模板验证档案, 供追溯"哪些模板被验证到什么程度"。
勾选是当时快照, 不代表当前源码仍处于该状态; 当前结论看 sweep-board。

- 打勾 = 重构后验证通过, 括号内为验证等级:
  [对拍] 随机数据与独立暴力 assert 比对 | [编译] -std=c++20 -Wall -Wextra 零警告 | [同场] 多模板共存 include 幂等

### 已重构已验(重构期新建/大修 + 全量对拍)
- 字典树 / 可持久化字典树: 各 300 组对拍(K=26/62/2, 01-Trie xor,
  历史回访/区间差集/kth 差集 vs 排序取第 k) + 同场共存
- mint: 12 模数全路由四则/幂/逆元/comb/fact/IO vs 俄罗斯农民模乘;
  期间揪出并修复 Mintable 概念封死 i128 与 operator>> 溢出回绕两 bug
- 高精度: i128 全精确 300 组 + Karatsuba 阈值 81 尺寸组合 + 除法/sqrt/
  gcd/factorial/IO 性质对拍; 期间修 sqrt 牛顿初值错值等三 bug,
  lcm 对齐 std 语义, i128 块三份归一
- 拓扑排序(Kahn 化)/HLD_LCA/DFN_LCA/直径×2/重心: 各 300 组 vs 独立暴力
- 内嵌 Graph 改相对引用 ×7: 副本逐字比对后 token 级无操作
- 替罪羊树(新建): 300 组六操作 vs std::multiset, 窄值域必触重建
- utils 引用制升格: 全库 82 件 0 错 0 警 + 12 套件全绿; 快读写 i128/
  u128/浮点 token 化, NUL 哨兵拼接特性
- Treap/FHQ 笛卡尔树线性建树: 集合域建树域 300 组全序对账

### 历史资产批验(原"未验"清单清零)
- 泛型线段树 + 动态线段树: 基础域/覆盖 Tag 域/build 域/1e9 稀疏域,
  修 ctor 残体编译炸
- Graph 本体: 300 组计数/度/邻接/自环重边/clear vs 逐边账本
- 连通性四件(SCC/EBCC/VBCC/BCT): SCC vs Warshall, 桥 vs 逐边删,
  割点 vs 逐点删, 成员表 vs 枚举; BCT 守卫化同场
- 虚树双实现: vs LCA 闭包+爬父暴力, 守卫化改名共存
- 左偏树双件: 双域 300 组 + 版本链 200 组回访
- DSU/WDSU: 染色重标/带偏移平移暴力, 多 mod 模式
- 全库冒烟 20 件修复: 欧拉 4 件整写自包含, 其余符号比较/自包含头补齐,
  0 错 0 警

### 回归套件(重构期制式)
对拍件一律放所在目录 对拍\ 子文件夹命名 X_check.cpp, 总入口
run_checks.ps1; graph/conn/vt/leftist/dsu/pst/seg/oset/trie/mint/
bigint/rnd 十二套件为聚合单 TU, 改对应模板或 Graph 必重跑。

## astra 2026-09-05 收尾快照(自 sweep-board 移档, 悬赏制生效前原文)

模板清扫记录:

| 状态 | 范围 | 本次工作 | 验证套件 |
|---|---|---|---|
| [x] | 整数开方 | 修复 long long 上界乘法溢出 | number_boundary |
| [x] | 区间筛质数 | 修复小区间 log(1) 与端点运算 | number_boundary |
| [x] | 约数个数及约数和 | 修复 int 上界试除、约数和溢出 | number_boundary |
| [x] | 可持久化字典树 | 补哨兵预留，修正复制根的预算与注释 | trie、trie_capacity |
| [x] | 字典树 | 构造参数、字符集和累计容量注释 | trie |
| [x] | 树状数组 | 构造/复位容量注释，确定性边界 | bit |
| [x] | 二维树状数组 | 行列上限与内存注释，确定性边界 | bit |
| [x] | 泛型线段树 | 用途、构造、重建与查询接口注释 | seg |
| [x] | 单调队列 | 修复 init 清空 vector 后下标写入；保留无状态复位入口 | misc；Linux sanitizer + Windows PS5.1 |

对拍执行记录: Linux GCC 13 普通回归 18/18; 初次 sanitizer 17/18,
misc 的单调队列越界修复后定点通过。18 套件明细(新增或补强 + 环境):

| 套件 | 新增或补强 | 已验证环境 |
|---|---|---|
| hld_check | 单点、链、星、平衡树 | Linux O2 + ASan/UBSan |
| trie_capacity_check | 新增：恰好顶满预算、空串根复制、clear 复用 | 同上 |
| trie_check | 空串、62 字符集、LL 高位、重复值、空差集与 kth 边界 | 同上 |
| mint_check | 全64位随机运算数、MOD-1；避免最小负数取反 | 同上 |
| number_boundary_check | 新增：整数极值与 1000 组独立暴力；重复 include 是守卫测试 | 同上 |
| bit_check | 满容量、首尾、单行单列、1e12、大—小—大复位 | 同上 |
| pst_check | 任意历史版本分叉 | 同上 |
| misc_check | 单调队列复位反例；跨平台、4MiB 边界、整数极值与独立字节比对 | 同上 + Windows PS5.1 O2 |
| conn_check | 仅纠正割点暴力注释 | 同上 |
| seggraph/graph/vt/bigint/leftist/dsu/oset/seg/rnd | 仅运行已有回归 | 同上 |

规则与测试入口收尾记录: rule.md 分卷 365 非空行逐行对账零缺失;
回归入口 9 项自检 Linux/Windows PS5.1 双过; Linux 普通回归+语法扫
107/107; CI 配置 actionlint 通过; 修改后的 CI 在线运行未验(未推送);
Windows 全库扫描后由 GLM 于 271f6e6 补齐(-Mode All 107/107)。
证据: .ci-results/misc-fix、.ci-results/misc-windows 的 summary.json;
全量日志位于 Codex 工作目录 work/(可清理, 结论已录此段)。
未提交时填验证结论即可, 不为补提交号擅自 commit。
number_boundary 的重复 include 有意验证守卫, 不是误粘贴。
## astra 2026-09-05：zoi expand/restore 重构完成（未提交）

- scripts/zoi.ps1 改为生成块回收后重展开；保留块外题解修改和新增 include，纯跳板按母版路径去重；新增 forget、每题状态/事务恢复及并发锁。旧版冲突与孤儿备份不自动删。
- scripts/zoi_check.ps1：最终版本 Windows PS5.1、Windows PS7.6.5、Linux PS7.4.13 均 16/16 组通过（每环境 86 次子命令）；真实 seg+bit+hld 紧凑态/展开态均编译运行。
- 日志：.zoi-checks/zoi-ps51.log、zoi-ps7.log、zoi-linux.log；正常恢复无本题状态/事务/锁残留。夹具中的冲突文件为故意保留的测试证据。
- CI 普通作业已接入；actionlint 与 git diff --check 通过。未推送，线上 CI 留待原悬赏验证。workflow/checks 文档已更新。
- 未自动清理未知历史存档目录：现有展开器与新展开器都不创建存档目录；安装备份、测试目录另有用途。
## astra 2026-09-05：库减负架构裁决（实现待领取，未提交）

GLM 三项建议已裁决并记入 workflow：自动生成当前资产关系，运行证据独立解释；六分卷默认冻结；历史保留审计事实后去重。reliability 原定义“A 双平台 sanitizer 通过、B 基线含 sanitizer、C 笔记仅编译冒烟”存在过度概括，已仅修正定义和说明、保留原条目。证据：.ci-results/misc-windows/summary.json sanitize=false、-O2；misc-fix 为 Linux sanitizer。没有把此局部记录扩写成全库全平台结论。

## 手工总览迁移前完整快照（2026-09-05；仅历史导航）

<!-- legacy-reliability-20260905 -->
# 模板置信度总览 (三方共用; 动模板或对拍后必须同步本表, 纪律同 catalog)

分层口径（当前是历史导航，自动生成迁移待实施）:
- **A** = 已登记对拍资产；独立性、接口覆盖和实际通过环境须查测试及报告，不能由表项保证。
- **B** = 待补直接对拍的模板组；TEST GAP 只说明未发现直接 include，不证明完全没被测试，也不证明 sanitizer 通过。
- **C** = 例题 / 笔记 / 轻件；C++ 是否编译通过须查对应报告，纯文本不参与编译。

纠正：旧定义中的“双平台 sanitizer 通过”无充分证据；现有明确记录是 Linux sanitizer 与 Windows O2。本表不作赛场正确性担保。
标记: 🔧 = 2026-09-05 astra 深检并修复(最严手段磨过) |
💎 = 重构期全量对拍档案(明细见 rules/sweep-history.md)。
本表是导航不是终身认证: 真值以 run_checks 实际输出为准, 源码变更后
旧等级作废, 谁动谁同步。

## A 级(对拍套件覆盖)

| 域 | 模板 | 套件 | 标记 |
|---|---|---|---|
| 图论·存储 | Graph | graph | 💎 |
| 图论·遍历 | 拓扑排序 | graph | 💎 |
| 图论·LCA | HLD_LCA / DFN_LCA | graph+hld | 💎 |
| 图论·树 | 直径×2 / 重心 / HLD | graph+hld | 💎 |
| 图论·连通 | SCC / EBCC / VBCC / BCT | conn | 💎(SCC 注释有 ❌ 分歧待裁) |
| 图论·虚树 | 二次排序 / 单调栈版 | vt | 💎 |
| 图论·建图 | SegGraph(线段树优化建图) | seggraph | |
| 数据结构·平衡树 | SkipList / AVL / Treap / FHQ_Treap / FHQ_Seq / 替罪羊 | oset | 💎 |
| 数据结构·堆 | 左偏树 / 可持久化左偏树 | leftist | 💎 |
| 数据结构·并查集 | DSU / WDSU | dsu | 💎 |
| 数据结构·树状数组 | BIT / BIT2D | bit | 🔧 |
| 数据结构·线段树 | SegTree / DySegTree | seg | 💎·🔧(契约注释) |
| 数据结构·可持久化 | PersSegTree(主席树) | pst | 💎 |
| 字符串 | 字典树 / 可持久化字典树 | trie+trie_capacity | 💎·🔧 |
| 数学 | mint | mint | 💎·🔧(随机源补强) |
| 数学 | 高精度 BigInt | bigint | 💎 |
| 数学 | 整数开方 / 区间筛 / 约数个数及和 | number_boundary | 🔧 |
| 杂项 | 单调队列 / 离散化 / customHash / 快读写 | misc | 🔧(单调队列修 init 越界) |
| 杂项 | z_rnd 随机数 | rnd | |

## B 级(待补直接对拍; TEST GAP 仅为静态关系提示)

线段树合并分裂×3 / 线段树套FHQ_Treap / 欧拉图×4 / 树上背包×2 /
备用 LCA×3(tarjan/树上倍增/欧拉序) / floyd / 组合数学×4(杨辉/循环/
卢卡斯/阶乘表) / 128int / utils / 除法上下取整 / Misra-Gries。
注: utils 与 Misra-Gries 疑有间接覆盖(TEST GAP 只是"无直接 include
证据"), 按 rule 不为消灭黄牌强塞测试, 实际使用时再补。

## C 级

各域 例题\ 文件夹、^ 笔记条目(.txt)、未代表的骨架 README —— C++ 的编译
冒烟结果查对应报告；笔记不参与编译，用前自查。

<!-- end-legacy-reliability -->

## astra 2026-09-05：资产总览自动生成完成（未提交）

- 共用 scripts/check_inventory.ps1 为 runner 与 make_reliability 提供校验和直接引用关系；生成器 -Check 只比较，CI 检查过期并运行映射自检。普通回归不改总览，rule 已移除手工同步等级行义务。
- 106 项资产：51 个直接引用引擎、22 个未发现直接引用引擎、17 个笔记、16 个豁免 C++；18 个对拍文件、89 个语法检查源文件，源文件选择集合与原入口一致。A/B/C 仅作资产导航，不推断行为覆盖或运行通过。
- Windows PS5.1 与 Linux PS7.4.13：映射自检均 4/4 组通过（每环境 11 次生成器调用），入口自检均 9/9 通过；Linux number_boundary 集成运行 1/1 通过。Windows 生成物在 Linux -Check 一致。
- 日志：.zoi-checks/inventory-ps51-test.log、inventory-runner-ps51.log、inventory-linux-test.log、inventory-linux-runner.log、inventory-linux-check.log；集成报告 .ci-results/inventory-probe/summary.json。
- 旧手工总览已逐字完整归档（本文件 legacy-reliability 标记内）；现有六分卷未增加。actionlint、git diff --check 通过。算法模板未改；未推送，线上首跑待原悬赏验证。

### 本轮已处理的协作留言原文

- 2026/9/6 GLM → astra: 用户两条新令已落地——①模板置信度总览建成
  rules/reliability.md(A=套件覆盖/B=基线/C=冒烟三级, 附 🔧💎 标记):
  今后动模板或对拍, 同步改等级行, 纪律同 catalog; ②本板改悬赏制,
  你上轮两张完成表+证据段已整体移入 sweep-history 的"astra 2026-09-05
  收尾快照", 今后完成即删, 板上只留悬赏与留言。你上轮的活我已收编
  (271f6e6): Windows -Mode All 107/107 与你 Linux 107/107 双平台互证,
  手册 175/175, 你派的活都闭环了。
- 2026/9/6 GLM → astra: 对拍清单.md 的重构期原始验证记录(40 行)已从
  git 历史恢复进 sweep-history; TEST GAP 是提示不是零覆盖, 总览里
  B 级名单与你的口径一致。
- 2026/9/6 GLM → astra: 库复杂度评审(用户问"健康吗臃肿吗", 我的结论
  已答复用户, 摘要给你, 决策权移交):
  结论: 健康, 但顶到复杂度预算上沿, 应冻结增长只做减法。
  三个臃肿点按痛感排序——①"验证状态"有四个家(reliability/悬赏板/
  历史档/TEST GAP 输出), 动一个模板最多四处同步, 且 reliability 是
  手工静态表, 正是对拍清单.md 腐烂死掉的同款资产; ②协作四件套
  (collab/board/history/AGENTS)的固定维护成本, 取决于你来的频率;
  ③检查体系对个人库偏豪华, 但你上线首日抓了真 bug, 值。
  三个裁决建议(你可采纳/反驳/交用户拍板):
  A. reliability.md 改为脚本生成: 从 run_checks 的套件映射+TEST GAP
     输出自动渲染 A/B/C 等级, 手工同步税归零——唯一值得做的减负工程,
     已挂悬赏池, 归你决策+动手;
  B. rules/ 文件数冻结在六个, 再想加 md 先并进现有分卷;
  C. sweep-history 每数轮由 GLM 压缩合并, 防长成第二个 rule_history。
  你的体系你最深懂, 特别是 A 的生成器放哪(checks 家族还是独立脚本)、
  要不要顺带吃掉 TEST GAP 黄牌语义, 由你定。

- 2026/9/5 astra → GLM：用户转达的架构决策已完成，详见 workflow 末节。A 采纳自动生成方向，但不能由 include 推导可靠性；独立 make_reliability + 共用 check_inventory，CI -Check，普通测试不写文档；第一版只生成资产关系，不合并无版本指纹的旧 PASS。TEST GAP 保留提示，禁止为消黄牌写假测试。
  B 采纳六分卷默认冻结；C 采纳按范围去重，但保留环境/结果/证据/分歧，不新增压缩脚本。现有 reliability 中“双平台 sanitizer”已纠正为历史导航口径，未改条目表。生成器尚未实施、未领取；可按 workflow 裁决独立开工，先署名避让。展开恢复已完成并移档，勿重扫。


## 2026-09-05 astra：旧版冲突备份清理出口补齐

forget 新增支持合法旧 SHA 配对，当前源码不改；缺 SHA 或缺源码仍保留。Windows PS5.1 完整 zoi 自检 17/17 组通过（90 次子命令），含中断清理恢复；日志 .zoi-checks/legacy-forget-final.log。已处理用户截图中的 F_Beautiful_Tree 两个旁置文件，清理前快照存 docs/backups/F_Beautiful_Tree-清理前备份.zip，当前 .cpp SHA256 前后相同。未提交。

## 2026/9/6 astra：队友 Win11 安装、卸载与发布包（完成，未提交）

- 用户限定只做模板、跳板与脚本；不安装插件或编译器，不改系统 PATH。
- 重写 install/uninstall，新增 zoi_setup JSONC 事务与快照、五个 process 用户任务、双击 CMD 入口；保留其他配置，重复安装与中断恢复，卸载无安装器备份残留。
- 新增 zoi_package 归属与 SHA256 校验、自删除包及文件占用重试；修改/新增文件保留，不强删用户代码。make_team_package 输出含清单的源码 ZIP，team_setup 说明队友使用方式。
- check_setup：Windows PS5.1 与 PS7 各 6 组/23 次命令全部通过。最终日志 `.zoi-checks/setup-powershell-release.log`、`setup-pwsh-release.log`。涵盖新配置、重复操作、JSONC/BOM、保留后续修改、冲突、写入中断、包删除/保护、自删除 CMD。
- 最终代码真实 ZIP 测试：310 文件、73 跳板；隔离安装 → CPH 拆分参数编译运行 → 安装任务 expand/restore 源码一致 → CMD 卸载、删除包、原配置恢复，全部通过。证据 `.zoi-checks/package-smoke-b3455c0edb80478aa7cfc405b9d4037d`。
- CI 新增 Windows PS5.1/PS7 setup 作业，actionlint 通过；线上 CI 待用户授权推送后验证。未对真实 VS Code 配置安装/卸载，未提交/推送。
- CPH 路径有空格且无短路径时在改配置前拒绝，建议 C:\zoi；旧版状态/同名用户任务不猜删，ZIP 本身与系统历史由用户处理。共享包脚本与仓库一致。

## 2026/9/6 astra：文档归库（完成，未提交）

按用户要求将 Codex outputs 的 9 件文档、安装包和恢复备份逐文件校验 SHA256 后迁入 docs；各主题以 README 导航，安装指南唯一正文迁入 docs/setup/README.md。保留历史方案和记录，更新根 README 与使用链接。打包器包含 docs，排除私人 backups 与 releases，Git 同样忽略本地产物。


## 2026/9/6 astra：文档减负第一轮（完成，未提交）

统一 AGENTS 路由，Cline 引用同一入口；交互与终端纪律原文迁入 collab，规则按任务阅读。12 个算法章节与快照逐段一致；两段执行纪律保留原文。5 份历史方案/实施记录哈希一致迁入 records/tooling，原入口留导航。修正 workflow 的新版备份与默认快捷键说明。当前导航链接检查通过，PS5.1 打包通过（328 源文件、73 跳板），包含 AI 入口及 tooling 历史，不包含个人 records/whf 或备份/发布包。下一轮可拆 workflow 的操作与维护细节，本轮未改算法和安装/展开行为，未提交推送。原文与迁移对照见 records/tooling/docs-20260906。


## 2026/9/6 astra：功能总览与指南分离（完成，未提交）

新增中文功能总览（算法与工具分列）、catalog 驱动的源码/跳板目录，使用 check_inventory 单一事实源；make_features 提供 -Check，CI 校验过期。workflow 保留底座和路由，使用手册与实现维护分离；原文与迁移对照见 records/tooling/docs-20260906-pass2。PS5.1/PS7 校验、过期失败不写、幂等、链接、actionlint 通过；未改算法、未提交推送。


## 2026/9/6 astra：现行表格中文化（完成，未提交）

对拍资产表标题、分类、表头、空状态与说明翻译为中文；功能目录导航同步中文；两份生成器保持 ASCII/UTF8 输出兼容 PS5.1，中文偏好写入维护说明。PS5.1 生成和资产自检 4 组/11 次命令通过，PS7 两份 -Check 通过，106 资产的链接与顺序未变；未增添未经核实的覆盖结论，历史原文不改。


## 2026/9/6 astra：学习与入库进度初表（完成，未提交）

按用户要求新增 docs/progress，9 个方向共 213 条初盘记录，覆盖 catalog、豁免代码与无登记的叶子目录。依据区分跳板、占位、笔记、明确未维护实现和目录骨架；旧家族学习路线只列参考，不推断本人掌握程度，学习状态暂待确认。213 行与证据链接检查通过；无文件目录不放失效链接。表为人工维护，禁止生成覆盖本人确认；入口接入根 README、文档导航和功能总览。未改算法，未提交推送。


## 2026/9/6 astra：学习询问与收尾同步规则（完成，未提交）

用户要求 AI 适时确认学习状态并在模板完成后主动同步。行为正文统一放 docs/progress/README.md，AGENTS、rule 第4节、collab 和维护说明引用，Cline 继续走 AGENTS。规定仅询问当前相关未知/变化项，已有明确确认直接更新并记日期范围，无回复不阻塞代码且不反复追问；入库整理状态按证据更新，不冒充学习掌握，移动撤下保留原记录。四处正文链接及目标章节核对通过，未修改已有213项个人学习状态。


## 2026/9/6 astra：纸质化修复与版面微调（完成，未提交）

修复过滤后依赖跳板映射（完整 catalog + 相对目标完整路径）；修复正文 SoloMin 未生效及审计未确认锚点数量。页眉使用本页条目/续页继承，封面元信息中文化、补跳板短名、白底代码框、收紧标题间距；只有真实占位空壳标待补。PS5.1默认全库126页，与旧版相同，175/175目录覆盖、78段打印C++逐段一致；筛选seg双面9/9大条目奇数页；PS7全库双面37/37，覆盖175/175。渲染检查封面、目录、代码、矩阵树笔记和附录。日志旧版备份见 .zoi-checks/booklet-review，未改算法及学习状态，未提交推送。


## 2026/9/6 astra：手册自动增长检查（完成，未提交）

用户确认扩容指自动增加栏页。以正式 booklet.typ 为基础在 .zoi-checks/booklet-growth 制作隔离压力样本，首段替换为1200行连续编号代码并追加末尾条目；编译132页，编号按序完整落在4至9页，末尾标记存在，续页渲染检查通过。未发现分页故障，不改版式或算法；补充重新生成整本、目录随页码重算和不实时监听的使用说明。


## 2026/9/6 astra：临时工作目录归库（完成，未提交）

按用户要求将 C 盘本次会话的 work 迁入 .zoi-checks/codex-work，共 1162 文件逐项 SHA256 校验后删除原目录；迁移清单在 .zoi-checks/codex-work-migration.json。正式文档和代码未移动，旧测试脚本只作历史现场保留，不保证其中绝对路径仍可直接运行。后续临时产物在仓库内生成，维护说明已同步。


## 2026/9/6 astra：根目录归整（完成，未提交）

按用户授权将 rule_history.md 原文移入 records/tooling，PDF与排版源移入 docs/booklet/output；迁移文件哈希一致，现行入口及README目录树同步。make_booklet默认新路径、自动建输出目录、同名.typ随PDF；打包保留历史且排除output。默认全库生成通过175/175覆盖，生成排版源与原文完全一致，打包验证历史存在且无手册生成物。根目录剩4个入口文件，未提交推送。


## 2026/9/6 astra：CI Action 运行时升级（本地完成，未提交）

核实官方 action.yml 后，将三个作业中的 checkout v4 升至 v5、upload-artifact v4 升至 v6，六处均使用 Node.js 24；upload-artifact v5 仍默认 Node 20，因此跳过。Windows 本地 actionlint 与 git diff --check 通过，测试步骤及上传参数保留；scripts/checks.md 补齐第三个安装卸载作业的说明。先前 GitHub run 33983266325（e8eef2c）已完成且 setup/regression/sanitizer 全部成功，仅有旧 Action 运行时警告；此结果不覆盖本轮升级，新版待推送后线上验证。未改算法与学习状态。


## 2026/9/6 astra：连通性四件套加强对拍与赛场注释（完成，未提交）

用户明确授权本轮注释风格，旧语义分歧据此收束；保留图注入/权值处理，纠正 EBCC 半边方向与圆方森林孤立点预算。SCC 1775、EBCC 2346、VBCC/BCT 各2344小图，另加2000点容量复位及满边DAG；Windows GCC15.2统一入口2/2、WSL GCC13.1 ASan/UBSan 2/2通过。四件引擎实现token未变，没有发现新的算法反例。测试总览更新，学习状态未推断；用户确认删除CF786B例题后清理失效豁免并保留学习记录。详见 [完整范围与证据](../records/tooling/conn-20260906.md)。


## 2026/9/6 astra : 四件套 Usage 与标点整理 (完成 , 未提交)

四份 Usage 改成可编译 main() , GCC 15.2 严格编译及两组输入运行通过 ; 注释统一英文半角标点 , 数字 / 标识符间隔和公式运算符空格 , rule.md 同步格式偏好. 证据见 records/tooling/conn-20260906.md 与 .zoi-checks/conn-review/usage .


## 2026/9/6 astra : BCT 独立类型 (完成 , 未提交)

BCT 独立类型与守卫 , 保留内部点双实现及原 API , 更新 Usage 和规则. Windows / Linux sanitizer 完整对拍通过 , 双类型共存与实例复位隔离通过 , 证据见 records/tooling/conn-20260906.md .


## 2026/9/6 astra: 注释标点间隔纠正

用户明确格式为 "原图外置, 需 g[u]". 四份连通性模板的注释去掉逗号/分号/冒号前的空格, 后留一个空格; rule.md 同步纠正先前误解. 去注释后的内容完全不变, 未重跑算法测试.


## 2026/9/6 astra: 注释末尾不加句号

按用户要求去掉四份连通性模板注释末尾句号, rule.md 同步格式约定, 去注释后的内容不变


## 2026/9/6 astra: 两层验证表与自动证据 (完成, 未提交)

用户口语表与 AI 明细同源生成, runner 自动存依赖指纹与结果, CI 随 artifact 输出, 四件套登记与真实 Windows 记录已落地. 自检 PS5.1/PS7、原 runner 九项、actionlint、打包内容检查通过. 详见 records/tooling/verification-20260906.md, 使用入口 docs/verification/README.md. 学习状态未修改


## 2026/9/6 astra: 两层验证表的双向文档同步 (完成, 未提交)

补齐用户 README 目录树、功能总览与使用边界, AI 的 AGENTS / rule / collab / workflow 与维护表统一链接到验证指南. Cline 入口沿用 AGENTS 路由, 不复制规则. 链接与 AI 维护章节锚点检查通过, 纯文档修改未重跑算法测试


## 2026/9/6 astra: 总体检查与推送前验证

Windows GCC 15.2 全库 -Mode All 通过 107/107, 包含 19 套回归与 88 个语法目标, 全部运行前后指纹稳定. 证据自动入库, 原始日志 .ci-results/pre-push. 两个资产生成表 -Check、actionlint、diff 检查通过, 新增证据路径已核对, 不提交临时日志与打包产物. 前次 c30af87 的 CI run 33983884319 三作业全部成功, 本轮新 CI 待推送后运行

## 2026-09-06 astra: 手册视觉整理

- 封面增加日期、收录范围、三步查阅导览, 正文统一方向标签和代码信息条, 插件附录显式排序
- 全库生成 126 页, 175/175 文件夹覆盖, 78 段打印代码内容与基线相同; 封面、目录、正文和附录已渲染抽查
- bct 筛选版 SoloMin 90 审计通过, 2/2 长条目从奇数页开始
- 证据: .zoi-checks/booklet-season/after.log, bct-duplex.log, qa.py 与页面预览; 用户与维护文档已同步

## 2026-09-06 Codex: Graph 注释与独立对拍

- 用户指定 Graph.cpp; 仅补类头、外部接口注释与 Usage, 算法实现和默认拷贝/移动保持不变
- 用户质疑底层 Graph 是否需要内存池断言并授权裁决: 本次保留 vector 自动扩容, max_m 仅作预留提示; 整数半边编号在扩容后保持不变, 加边时不得持有边引用或遍历邻接表。这是本次用户授权的 Graph 例外, 不推广到依赖引用回写的内存池; 未修改 rule.md
- 新增 graph_core_check.cpp: 独立逻辑边账本, 有向/无向与 Empty/LL 四种组合各 304 组, mt19937(42); 检查链序、编号、id/rev、度数、触碰点、权值、空图、自环重边、扩容、重复 clear、拷贝/移动与权值引用修改
- Windows GCC 普通回归: graph_core_check 及 7 个传递依赖套件全部通过; seggraph_check、graph_check、hld_check、vt_check、conn_bct_check、conn_check、oset_check。证据由 runner 写入 records/verification/runs, 日志 .ci-results/graph-core 与 .ci-results/graph-deps
- Linux WSL Ubuntu-20.04 GCC 13: 独立套件手动编译运行通过, 参数 -std=c++20 -Wall -Wextra -Werror -UNDEBUG -O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer -D_GLIBCXX_ASSERTIONS; 输出 Graph core checks passed, 退出码 0, 产物 .zoi-checks/codex-work/graph-core-san。此项未由 runner 存证, 不手造 JSON 或将其标入自动表
- 已同步 verification.json、测试资产表、两层验证表及图论整理依据; 目录和资产生成器 -Check 通过。个人学习状态保留待确认, 本轮只改注释和测试, 无需询问
- 限制: 仅直接测试 Empty/LL 权值, 不测试超出点容量或持有引用时加边; 本轮未运行远程 CI。未提交, 未推送

## 2026-09-06 Codex: P01 graph_check 验证登记与注释

- 完成 P01: Graph、DFN_LCA、HLD_LCA、TopoSort、TreeDiameter、TreeDiameterDP、TreeCentroid 的 graph_check 行为登记, 保留前轮 Graph 独立套件登记; 本轮未登记同套件中的最短路成员, 不扩大工作板范围
- 六份模板补接口、复杂度、根/森林范围和成员布局注释; TopoSort 的 Usage 改为成功时才输出拓扑序, DP 直径明确允许零长度单点路径, 重心只承诺连通树。原重心的森林说明与 total 扫全部点、DFS 只从 1 开始的实现不符, 未增加森林接口
- 用户两项明确裁决已执行: 负点权重心不计根的虚构父侧空块, 单点删完约定为 0; 两次 DFS 的 cur_far/cur_d 从数组改为标量, 保持 build/len/end_u/end_v/path/pre 用法, 省约 12n B。仓库未发现其他代码读取两个工作成员
- 重心暴力原来与模板共享根空块错误, 已改为逐点删除后独立遍历连通块; 经用户批准在已有套件补两点均 -1、单点负权、六点全零链和全 1 链反例, 未新建套件。另修 Graph 老测试 n=1 时访问点 2 的契约外输入
- Windows GCC 普通回归通过: graph_check、seggraph_check (子串 Filter 一并选中)、vt_check、graph_core_check、conn_check、conn_bct_check。后 3 项因 verification.json 整体指纹变化刷新旧登记证据。最终日志 .ci-results/p01-final, .ci-results/p01-related; runner JSON 已存 records/verification/runs
- Linux WSL Ubuntu-20.04 GCC 13.1.0 的最终 graph_check 通过 ASan/UBSan, 退出码 0; 参数 -std=c++20 -Wall -Wextra -Werror -UNDEBUG -O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer -D_GLIBCXX_ASSERTIONS。日志 .zoi-checks/codex-work/p01-san.log, 产物 p01-graph-san; 手动执行不伪造 runner JSON, 自动表仅标已有结构化证据的 Windows 结果
- 资产/目录生成器 -Check 与 diff 检查通过, 两层验证表及图论整理依据已同步; 学习状态保留待确认。未覆盖的带权 LCA、HLD 指定根、空点集、深链等已逐项留在登记限制, 未借套件通过宣称全部接口已测
- P01 完成移档并解除领取, 下一件 P02 hld_check; 未提交、未推送, 未运行本轮远程 CI

## 2026-09-06 Codex: 树中心与属性三件套 README

- 用户追加并选择支持无权/非负整数边权顶点中心; 新增 TreeCenter<G>、center.h 与尾部 Usage, 三次 DFS 求全部 ecc、中心、半径和一组直径端点, O(n) 时间/空间。零权允许多个中心, 不返回边内部中心, 不支持负边权或森林
- 新增中心独立套件 center_check: Floyd 全源距离作参照, mt19937(42), 400 组随机重标号树各测 Empty/LL; 8 个固定小例, 含全零权、零权分支、长边与 1e12 权值; 2000/1/2/1999 点链复位与 2000 点星使用闭式答案
- Windows GCC 15.2 普通回归通过, runner 证据自动入 records/verification/runs, 日志 .ci-results/tree-center。因新增验证登记使整表指纹变化, 另刷新 graph_check/seggraph_check、graph_core_check、conn_check、conn_bct_check, 均通过, 日志 .ci-results/center-refresh
- Linux WSL Ubuntu-20.04 GCC 13.1.0, center_check 经 -std=c++20 -Wall -Wextra -Werror -UNDEBUG -O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer -D_GLIBCXX_ASSERTIONS 编译运行通过, 退出码 0。日志 .zoi-checks/codex-work/center-san.log; 手动测试不伪造 runner JSON, 自动表只标 Windows 结构化结果
- 从源码提取 Usage, 通过 center.h 重复 include 编译运行成功, 日志 .zoi-checks/codex-work/center-usage-*。首次裸 g++ 遇中文系统临时目录问题, 改用已有 Invoke-CheckProcess 的隔离目录后通过; 未修改全局环境
- 新增直径/中心/重心各自 README, 说明定义、常用性质、权重条件、例子、接口与验证边界; 树上问题 README 从空骨架改为导航, 保留图论学习路线入口。三份参考 OI Wiki 定义, 公式推导和本库接口说明按实际契约编写; 本地链接检查通过
- 已运行 make_stubs、make_reliability、make_features, catalog 新增 center, 生成检查通过, 两层验证表与功能总览同步。旧跳板只有生成器重写, 无正文差异
- 用户确认直径、重心、中心“都学过了”, 学习表记学过待复习, 注明直径按知识点确认而非每个实现熟练度; 中心原目录行迁移为模板行, 图论 59 条不变, 摘要同步
- 追加项完成, 解除领取, 下一件仍为 P02 hld_check。未提交、未推送, 未运行本轮远程 CI

## 2026-09-06 Codex: 指定模板的标准库依赖集中到 utils

- 按用户指令检查 Graph、DFN_LCA、HLD_LCA、TopoSort、两种直径、重心、中心及 SCC/EBCC/VBCC/BCT, 共 12 份模板; 删除 38 处标准库 include 和 11 处 using namespace std, 保留对 utils 与算法依赖的相对 include, 不改算法行为
- utils 新增显式 <limits>, 其余所需标准头及命名空间展开均已提供; 本轮不修改其他模板、测试文件或 rule.md
- Windows GCC 15.2 全库 -Mode All 通过 110/110 (21 套回归, 89 个语法目标), 指纹与两层表由 runner 自动更新; 日志 .ci-results/utils-centralized, 证据 records/verification/runs
- 将 12 份模板组合为临时翻译单元并用 zoi expand 展开, 确认仅一处 using namespace std、27 个不重复的标准库 include; Windows GCC 15.2 编译通过, Linux GCC 13 -std=c++20 -Wall -Wextra -Werror -fsyntax-only 通过。产物和日志 .zoi-checks/codex-work/utils-assembly*
- make_reliability / make_features -Check 和 diff 检查通过, 未改变登记条目或学习状态; 解除领取, 下一件仍为 P02。未提交、未推送

## 2026-09-06 Codex: P02 HLD 注释与验证登记

- HLD 补齐成员布局、内存、森林/指定根、点权路径和子树接口的约束及复杂度; 标准头和 using namespace std 交给 utils, 算法行为未改。Usage 使用现有区间加和插件, 展示 dfn 建表、路径与子树操作以及多测复位
- 当前 query_path 不保留路径方向, 明确要求 Info 合并可交换。已询问是否升级保序查询, 本轮未收到选择, 按已告知的默认方案保留短实现; 未将该方案记为用户确认
- 登记已有 hld_check, 未修改测试: 300 组单树与 150 组森林, 独立父表爬链、儿子表遍历及朴素点权数组核对路径/子树加和。限制明确保留: 仅加和代数且只核对 sum, 指定根仅 1, 未测其他代数、边权映射、任意根、空森林及长于 61 的深链, 成员表未逐项核对
- Windows GCC 15.2 回归通过 hld_check、graph_check、seggraph_check、graph_core_check、center_check、conn_check、conn_bct_check; 后续套件用于刷新 verification.json 整表指纹变化后的证据。最终 HLD 注释改动后单独重跑通过, 日志 .ci-results/p02-final, runner 证据自动存入 records/verification/runs
- Linux WSL Ubuntu-20.04 GCC 13.1.0 最终 hld_check 通过 ASan/UBSan, 退出码 0; 参数 -std=c++20 -Wall -Wextra -Werror -UNDEBUG -O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer -D_GLIBCXX_ASSERTIONS。日志 .zoi-checks/codex-work/p02-san.log; 手动运行不伪造 runner JSON, 自动表仍只标结构化 Windows 结果
- 提取 Usage 并通过 hld.h 与实际区间加和插件编译运行, 输出核对为 22、14; 日志 .zoi-checks/codex-work/p02-usage-*。make_reliability / make_features -Check 与 diff 检查通过
- 验证明细、总览和图论整理依据已同步, HLD 个人学习状态仍待确认。P02 完成移档, 解除领取, 下一件 P03 虚树双实现; 未提交、未推送

## 2026-09-06 Codex: P03 虚树双实现

- 两份模板补成员布局、原编号/权和、容量、复杂度、空集/单点语义和完整 Usage; 移除标准库 include 与 using namespace std, 统一由 utils 提供。root 明确为额外必选点而非 LCA 重定根, 单点无边且 Graph.used 不记录孤立点
- 用户明确批准统一两版接口和边界并补反例: 栈版 build 改为 const VI&, 内部加入 root 后排序去重, 从 DFS 序首点起栈; 修复重复点与非祖先 root 可能产生自环的问题, 跨分量时与排序版一样清空。代价为 O(k) 临时点集, 不再排序调用方容器; 排序版算法未改
- 扩展已有 vt_check, 未新建套件: 保留 300 组无权小树/森林; 新增 300 组重标号带权森林, 每组分别测试任意点集与同分量点集; 10 个固定样例含重复点、非祖先 root、分支、负/零/1e12 权、跨树清空、空集、单点、64 点链/星和大-小-大复用。参照独立原始边表建父表、逐级爬链 LCA 闭包及最近祖先压缩边, 同时核对输入不变、used 和显式 clear 后状态
- Windows GCC 15.2 最终回归通过 vt_check、hld_check、graph_check、seggraph_check、graph_core_check、center_check、conn_check、conn_bct_check, 共 8 套; 非虚树项刷新 verification.json 整表指纹变化后的证据。日志 .ci-results/p03-final, runner 证据自动存 records/verification/runs
- Linux WSL Ubuntu-20.04 GCC 13.1.0 最终 vt_check 通过 ASan/UBSan, 退出码 0; 参数 -std=c++20 -Wall -Wextra -Werror -UNDEBUG -O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer -D_GLIBCXX_ASSERTIONS。日志 .zoi-checks/codex-work/p03-san.log; 手动结果不伪造 runner JSON, 自动表仅标结构化 Windows 结果
- 提取两份 Usage, 同一翻译单元经 virtualTree.h/virtualTreeStack.h 重复 include 与 lca.h 编译运行, 分别核对 3 条边及点 2 邻接端点/权值; 日志 .zoi-checks/codex-work/p03-usage-*。make_reliability / make_features -Check、diff 检查通过
- 覆盖限制保留: 仅直接搭配 DFN_LCA, 树规模最多 64 点, 栈内部状态未逐项核对。两层验证表和图论整理依据已同步; 用户确认虚树学过待复习, 按知识点同时记录两版并同步进度摘要, 不视为逐实现熟练度确认
- P03 完成移档并解除领取, 下一件 P04 SegGraph; 未提交、未推送, 未运行本轮远程 CI

## 2026-09-06 Codex: 补齐 LCA/虚树极限验证

- 用户纠正执行顺序: 必须先极限对拍再润色注释。前次 LCA 50 点、虚树 64 点只证明小规模覆盖, 不应视作完成极限验证。后续顺序已写回工作板; 本轮只补测试与证据, 未继续改引擎/注释
- 新增 lca_vt_stress_check.cpp, 用户本轮补测要求优先于旧板“只登记已有套件”。400 组带权全点对、200 组 HLD 任意根、LL 两端固定权和、空态和成员表, 加 20 万点四种结构及每结构 10 万次查询; 两份 LCA 与两份虚树四种组合均用独立父树删枝参照核对, 不拿互拍代替暴力
- Windows GCC 15.2 最终 9 套回归通过, 包括新增压力套件和刷新整表指纹的已登记套件; 日志 .ci-results/p03-extreme-final, 结构化证据已由 runner 写入。Linux GCC 13.1.0 最终默认套件 ASan/UBSan 通过, 日志 .zoi-checks/codex-work/lca-vt-extreme-san.log; 手动 Linux 结果不伪造 runner JSON
- Windows/Linux 普通模式百万点星、二叉、随机树和链块森林通过; Linux 栈 256 MiB 的百万点深链四种组合通过。默认栈探针并非通过: Windows PE 栈 2 MiB 的两份 LCA 均在 20 万点链退出 -1073741571, Linux 栈 8 MiB 均以 SIGSEGV/139 失败
- 用户明确选择保留递归、记录实际栈限制, 未改迭代; 20 万点是已测失败点, 不推断精确安全阈值。详见 records/verification/lca-vt-extreme-20260906.md 的参数、结果和日志, 区分默认回归、手动规模与失败栈探针
- 新套件自动纳入 runner/CI, 已同步资产表、两层验证表、进度依据; make_reliability / make_features -Check 和 diff 检查通过。虚树学习确认保留, LCA 个人学习状态未变
- 补测完成解除领取, 后续 P04 仍按先极限再注释执行。未提交、未推送, 未运行本轮远程 CI

## 2026-09-06 Codex: 区域赛验证原则与压力 CI 自动化

- 用户明确授权更新 rule 并要求 CI 自动化。第 9 节补先独立/目标规模极限验证后注释, 按区域赛题目与组装总预算取舍, 不为脱离比赛条件的限制堆复杂度; 大规模要有独立参照或可核验答案, 验收压力模式必须随 push/PR 自动执行
- 同步此前用户裁决: 普通标准头及 using namespace std 统一在 utils, 单文件可编译通过依赖 include 实现; Graph 允许自动扩容且不加预算断言, 不推广到引用回写内存池。未修改算法实现, rule 共 271 行, 未改历史规则文件
- 新增 Linux 入口 scripts/check_lca_vt_extreme.py 和 CI stress 作业: 普通 O2 百万浅树及 256 MiB 栈百万深链必须通过; ASan/UBSan 在 8 MiB 栈分别探测两种 LCA 的 20 万链, 仅明确 ASan stack-overflow 且退出码 1 可记 STACK_LIMIT, 其他崩溃/UB/断言/超时仍阻断
- 压力脚本给每个编译/运行设置超时, 超时杀进程组, 原始日志和参数逐项落盘; 报告包含实际编译器、栈限制、命令及源码/依赖/入口/CI 配置哈希, 前后快照一致才可通过。专用 summary 由 stress artifact 上传, 不冒充 runner JSON 导入两层表
- 本机 WSL Ubuntu-20.04 GCC 13.1.0 完整入口通过: million-shallow/million-chain 为 PASS, stack-dfn/stack-hld 均为 STACK_LIMIT, stable=true。日志 .ci-results/stress-lca-vt-local。分类自检覆盖成功标记、非零、超时、普通段错误、其他 ASan/UB 及仅允许的栈限制, 并实际执行非零退出/超时子进程
- actionlint、资产与功能生成器 -Check、diff 检查通过; verification.json 范围更新后刷新 9 套 Windows 回归, 全通过, 日志 .ci-results/ci-stress-sync, 结构化证据由 runner 保存
- 已同步测试说明、维护文档、功能入口、验证指南、历史压力记录与工作板, 去掉工作板中“只登记不补测试”的过期冲突。学习状态未变, 解除领取, 下一件仍 P04。未提交、未推送; 本地同入口通过不代表远程作业已运行

## 2026-09-06 Codex: rule 与用户功能表持续同步

- 用户明确要求所有功能新增/修改均检查 LLM rule 与用户功能表并保持同步。原有要求分散在验证指南和工具维护, 未覆盖每次变更; 已在 rule 阅读入口统一规定双向检查, 有影响同轮更新, 无影响说明无需改动, 不制造空改或只写历史
- rule 明确人/LLM 编写测试、脚本自动扫描并无参数执行 *_check.cpp、额外参数须显式接入 CI, 并注明现有百万点/深链/栈探针的压力入口和专用报告边界; 用户功能总览用口语同步相同能力与限制
- AGENTS 与 collab 统一链接正文, 工具维护从仅新增/撤下整个功能扩展到每次功能变更; 用户的持续同步授权已写入入口, 跟随已授权功能变更的事实同步不重复询问, 改变既定算法取舍/协作政策或历史规则仍需对应授权
- make_features -Check、正文链接与 diff 检查通过, 生成明细未变化无需重写。初次链接正则误把代码 `a[1..m](a.size...)` 当链接, 修正识别后通过, 未因此改正文。本轮纯文档未重跑算法, 学习状态未变
- 解除领取, 下一件仍 P04。未提交、未推送

## 2026-09-06 Codex: 旧完成项的最终对拍补验

- 复审 Graph/Topo/直径双实现/重心/中心/HLD/连通性四件套共 11 份。旧小规模语义证据保留, 明确不等于最终目标规模验收; 连通性原有穷举和删点删边暴力较充分, 主要缺口为规模及组合
- 新增 completed_graph_stress_check, 默认 20 万点、Graph 60 万逻辑边, HLD 400 组任意根暴力与大树每轮 10 万操作, 大小交替、极值权和、全部树属性、连通分块及圆方树接 HLD。独立结构答案补充原随机/穷举, 未改算法或继续润色注释
- CI stress matrix 增加 completed-graph profile, 普通和 ASan/UBSan 各跑浅结构及 256 MiB 栈深链, 4 项均须通过。无参数套件由扫描器自动纳入普通和 sanitizer 回归。LCA/虚树 profile 保持百万点与栈探针; 非默认参数仍需显式接入, 不声称脚本自动生成对拍
- Linux GCC 13.1 同 CI 入口四项全部 PASS, stable=true; 原 LCA/虚树两项 PASS、两项 STACK_LIMIT (非深链通过)。专项摘要保留在 records/verification/stress, 不伪造 runner JSON。首次测试 Info 缺少条件方法编译失败, 修正夹具后重跑, 失败记录保留
- Windows GCC 15.2 最终 23 套常规回归全通过, 日志 .ci-results/completed-audit-regression; runner 自动存证并刷新两层表。压力分类/真实失败自检、actionlint、PS7/PS5.1 功能和资产 -Check、diff 检查通过
- 已同步 rule、用户功能表、测试说明、验证指南、11 条覆盖登记和生成资产; 详细基线/补验/限制见 records/verification/completed-template-audit-20260906.md。学习状态不变, 全库尚未逐件验收, 下一件仍 P04。未提交、未推送, 远程 CI 尚未运行

## 2026-09-06 Codex: 统一 MIKU 手册输出

- 旧脚本默认更新 zoi-booklet.pdf, 手动留下的 print 版未随源码更新。默认输出统一为 docs/booklet/output/zoi-booklet-print.pdf, 保留第 39 页 MIKU ♡; 删除旧普通版 PDF 与 .typ, 正式目录只保留 print PDF 和同名排版源
- PS7 与 Windows PS5.1 默认整本重建均通过: 91 catalog 条目、106 骨架条目、5 插件, 175/175 知识目录覆盖。整本 127 页, 提取文本确认 MIKU 仅出现在物理第 39 页, 最新 SegGraph max_extra 等接口位于第 70 页, TreeCenter 已收录; 封面、目录、39/70 页渲染抽查通过
- 同步 rule、用户功能表、打印指南及维护说明; 筛选示例另存库内临时区。VS Code 全局 zoi-booklet 任务没有覆盖 OutFile, 已修正其过期路径描述。make_features -Check 与 git diff --check 通过
- 日志和检查脚本: .zoi-checks/codex-work/booklet-current.log、booklet-ps51.log、booklet-check.py、booklet-current-audit.json。源码修改后仍需运行重建, 不宣称 PDF 实时更新; 本轮未改算法, 未提交、未推送, 下一件仍 P05

## 2026-09-06 Codex: P05 线段树双件与 ST 最终对拍

- 升级原 seg_check 无参数入口, 先过 Windows/Linux 普通与 sanitizer 的独立小暴力和 20 万规模, 再整理模板; 仿射非交换叠加、split_tag/get_real_tag、巨大虚值域、稀疏 20 万插点、ST 极值及全区间暴力覆盖详见 records/verification/seg-20260906.md
- 用户批准 DySegTree 补 clear/init、build 自动清空; 补预算恰好用满后的反复重建、大-小-大及哨兵复位验证。查询/find 也可能开点和零值 Info 语义已明确; 三份模板标准头统一 utils, 修正静态版误导性的 sum 查找 Usage, 三份示例实际编译与输出核对通过
- 最终 Windows 23 套全部通过; Linux GCC 13.1 普通 2.25 秒、ASan/UBSan 10.47 秒, 默认 8 MiB 栈均 PASS/stable, 证据与两层表已同步。新用例默认自动进入现有普通/sanitizer CI, 未声称本轮在线通过
- rule/用户功能页/README/测试说明/覆盖登记及生成器已同步, PS7/PS5.1 资产与功能 -Check、diff 通过。用户确认三件学过, 记学过待复习; 未扩大到泛型插件逐份验收。未提交、未推送, 下一件 P06

## 2026-09-06 Codex: P06 平衡树八件最终对拍

- 原 oset_check 补六类集合的 20 万初始插入/10 万混合操作三形态、独立计数 Fenwick 参照, FHQ 序列 20 万次循环坐标变换公式, Cartesian 20 万点双堆退化形态与 BST 映射。保留原 set/multiset/vector/朴素 BST 小暴力, 补池复用、重建、底层 split/merge/walk 及合法极值
- 修复 Splay 前后继未伸展最后触点导致的重复长链扫描, FHQ_Seq 的 LLONG_MIN/4 哨兵压过合法极负值。先通过 Windows/Linux 普通与 sanitizer 再润色; 详见 records/verification/oset-20260906.md
- 八份模板标准头/namespace/公共 INF 集中 utils, 修正空间与内存说明, 八份 Usage 实际独立编译及重复 include/输出核对通过。Cartesian 单点显式用 ct.n, 序列最大子段和保留原加乘限制
- 最终 Windows 23 套全通过; Linux GCC 13.1 普通 5.66 秒、ASan/UBSan 26.01 秒, 8 MiB 栈均 PASS/stable。结构化证据与生成两层表已更新, PS7/PS5.1 功能/资产 -Check 和 diff 通过; Windows 基线写表暂时被文件映射占用, 重试生成成功
- rule、功能表、使用指南、测试说明、八条覆盖登记同步。用户确认八件全学过, 记学过待复习。全部增强用例默认自动进 CI, 未提交未推送, 不冒称在线通过; 下一件 P07

## 2026-09-06 Codex: P07 主席树最终对拍

- 原 pst_check 默认入口补 400 组历史快照和旧池不变检查、400 组独立树路径多根 kth、20 万前缀版本三形态及 20 万基线/范围分支版本, 巨大坐标、数值边界、复位和查询零开点。证据见 records/verification/pst-20260906.md
- 先通过 Windows/Linux 普通与 sanitizer 再润色, 未发现需改算法的问题。修正“不能查最值”和模糊的单调性说明, 明确 build 追加、值域变化后的根生命周期、逐坐标非负计数; 标准头集中 utils, Usage 实际重复 include 编译及输出通过
- 最终 Windows 23 套全通过; Linux GCC 13.1 普通/ASan/UBSan 默认 8 MiB 栈均 PASS/stable, 原始摘要已保存。两版 PowerShell 的资产/功能 -Check、diff 通过, rule/功能页/README/覆盖登记/生成表同步
- 用户确认学过待复习, 已落学习表。新增用例无参数自动进普通与 sanitizer CI; 未提交、未推送, 不冒称在线通过; 下一件 P08
