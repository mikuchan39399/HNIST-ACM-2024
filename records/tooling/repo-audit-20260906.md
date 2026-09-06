# 全库设计、同步与工作区审查 (2026-09-06)

本轮检查 catalog 的 74 个现役代码条目、17 个笔记条目、23 个测试套件,
扫描 116 份当前 Markdown 的本地文件链接, 对照规则、使用指南、工具实现及 CI。
这是架构/一致性审查和定点复现, **不是 74 个算法都已完成独立极限对拍的证明**。
逐算法最终验证继续以工作板和带指纹证据为准。

## 已处理

| 问题 | 改动与依据 |
|---|---|
| rule 的轻件免对拍、collab 的启发式件对拍可选, 与当前最终覆盖冲突 | 以用户最新要求统一为现役件按契约验证; 旧“只登记不补测试”留言移入历史 |
| 工作板只突出 P04-P11, 后续现役件容易遗漏 | 明确列出剩余家族, 按 catalog/验证表核销; “平衡树七件”改为实际八件 |
| Info 条件方法写成可选 | SegTree/DySegTree 的 modify 实际直接调用 break_cond/tag_cond; rule 改为非势能场景也提供 false/true, 不改引擎 |
| 维护文档仍称现有报告无指纹, 验证指南仍称只登记连通性四件 | 纠正为现行 runner 有依赖指纹、范围查生成表; 去掉旧的硬编码数量和优先级 |
| 功能页重复解释验证维护机制 | 保留用户能力与边界, 具体流程链接唯一验证指南; 不把生成目录变成第二份手填清单 |
| 队友包漏 CI 配置 | 压力脚本 snapshot 必读 .github/workflows/ci.yml, 原打包目录和扩展名均漏收; 现已收配置, 现有 check_setup 新增真实 ZIP 内容回归, 自动随 CI 的 PS5.1/PS7 执行 |
| 压力脚本不同 profile 的手动默认报告目录相同 | 默认目录改为 stress-PROFILE, 不再覆盖另一 profile; CI 已显式分目录, 保持不变 |
| Linux 编译中间文件仍可能落系统临时目录 | 压力子进程 TEMP/TMP/TMPDIR 全部指向库内报告目录, 补真实子进程自检 |
| 工作区约定埋在维护文档末尾 | collab 集中定义临时文件、子进程环境和长期交接; AGENTS/rule/功能页路由到该正文, 不增加第二套记忆系统 |
| docs 导航把 ZIP/备份说成根目录路径 | 改为实际 docs/releases 与 docs/backups |

## 需要继续推进的设计问题

1. **存量模板的组合契约尚未统一。** 排除 utils/rw 后, 42 个条目有标准头 include,
   53 个有顶层 using namespace std; 18 个没有 #ifndef 或 #pragma once。
   数字来自静态扫描, 不把它当算法错误率。Misra–Gries 重复 include 已实际复现
   majority_element_k 重定义 (GCC 15.2, -std=c++20 -fsyntax-only, exit=1),
   现场在 .zoi-checks/codex-work/audit-repeat-include.*。
   缺防重的完整名单见下面快照。存量调整随逐模板独立/极限验证再整理, 不以正则整库删头代替验证。
2. **VBCC/BCT 的重复实现。** 对比显示核心 Tarjan、块成员及圆方树构建基本相同,
   差异主要为类型、守卫、依赖路径和写法。合并能减少两处同步修复成本,
   但当前 rule 明确要求 BCT 独立且不复用 VBCC。现阶段保留这一裁决;
   若以后改为别名或公共实现, 必须说明类型身份/独立使用和纸质化取舍, 重新验收组合接口。
3. **登记整表参与每套指纹。** Get-VSnapshot 将完整 rules/verification.json 入哈希,
   因而改一个家族说明会令其他家族待重验。不是假失败, 是当前有意的保守失效策略。
   后续可改成每套件对应的规范化范围片段, 但须验证相关条目增删改必失效、无关改动不失效、
   旧证据迁移不会误继承。收益是减少无关重跑; 本轮不悄悄削弱证据判定。
4. **压力流程重复与命名。** completed-graph 默认浅结构同时进普通、sanitizer 和 stress;
   stress 还有相同的浅结构确认, 属于独立入口完整性与 CI 成本的重复。
   当前实测单次普通约数秒、sanitizer 数十秒, 先保留。check_lca_vt_extreme.py 已承担两种 profile,
   文件名不再概括全部职责; 当前 docs 已解释, 以后扩展可一次性改通用名称, 无需现在加插件式框架。
5. **目录登记不等于统一接口。** 74 个现役条目中 15 个有正式行为登记, 59 个尚未登记;
   其中一些有旧回归。未登记不是零测试, 有测试也不是最终验收。库中仍有全局数组、
   题目式 main、缺 Usage 的存量代码, 不能把“有跳板”宣传成全部可无脑组合。
6. **不按文件行数判臃肿。** 较长的 BigInt、FHQ_Seq、左偏树包含多操作或示例契约,
   没有仅为缩短文件而合并不同算法/删接口。utils 集中基础依赖、现有 catalog 共用扫描器、
   验证两层表同源生成都是有效设计, 不再造平行目录、覆盖表或通用多引擎框架。

## 工作区与磁盘

- 本项目临时现场: F:/c++/my_code/HNIST-ACM-2024/.zoi-checks/codex-work。
  审查脚本和扫描 JSON 放这里; 可长期使用的结论写入本记录、现行规则和工作板。
- 审查开始时 .zoi-checks 约 140.7 MiB, .ci-results 约 0.75 MiB,
  自动验证证据约 0.58 MiB, 手册 output 约 7.86 MiB, 都在 F 盘。
- 当时 C 盘空闲约 17.8 GiB, F 盘约 523 GiB。
  已知 C 盘旧项目目录 Documents/Codex/2026-09-05/f-c-my-code-hnist-acm
  及本任务 visualizations 目录都没有文件, 不存在本轮可再次迁走的项目现场。
  没有删除无关用户文件, 也没有宣称本轮释放了 C 盘空间。
- 原有迁移清单 .zoi-checks/codex-work-migration.json 和历史记录已在库内。
  本轮不迁移应用自身日志、缓存或 WSL 系统盘, 不修改系统全局环境。
- 忽略目录不能承担长期记忆: 入口负责引导读取, 正文保存约束, 工作板保存当前状态,
  历史和专项记录保存完成证据。这样跨任务可重新读取, 不承诺模型永远记住会话。

## 验证

- Linux GCC 13.1 两个 profile **不传 report-dir 并行运行**, 分别生成各自默认报告,
  快照 stable=true。completed-graph 的普通/ASan/UBSan 浅结构与增栈深链四项 PASS;
  LCA/虚树百万浅结构及增栈深链 PASS, 默认栈两探针仍为 STACK_LIMIT, 不当作通过。
  原始摘要: [completed-graph](../verification/stress/completed-graph-workspace-20260906.json) /
  [lca-vt](../verification/stress/lca-vt-workspace-20260906.json)。
- 压力入口的非零、超时、栈诊断分类及 TEMP/TMP/TMPDIR 实际子进程自检通过。
- check_setup 在 Windows PS7 与 PS5.1 均 7 组、24 次命令通过, 含真实队友 ZIP 内容检查;
  源码及库内私人文件保留检查有效。日志 .zoi-checks/setup-test-3551258e0d874cfe8779b823a4f82b5c
  与 .zoi-checks/setup-test-59fc227dab454d1eae6b0d611b3a0850。
- 本轮没有改算法实现、catalog 或普通 runner 的指纹机制, 沿用前轮 23 套常规回归证据,
  不为文档修改再跑整套算法。资产/功能生成物、当前链接和 diff 在收尾复核。
- 个人学习状态未改。未提交、未推送, 当前远程 CI 待获准推送后验证。

## 现役条目逐项快照

以下为本次静态扫描快照, 不是另一份需手工常驻维护的 catalog。
“套件”仅直接 include 关系; “登记”仅行为说明条目数, 两者都不是通过次数。
头数/namespace 包含源码中的匹配, utils/rw 有明确自包含职责; 防重列指 Z_OI 守卫,
已另核实所有缺失项也没有其他 #ifndef 或 #pragma once。后续最新状态查生成目录与验证表。

| 跳板 | 源码 | 直接套件 | 登记 | 标准头数 | 顶层 std 命名空间 | Z_OI 防重 | Usage 标记 |
|---|---|---:|---:|---:|---|---|---|
| seg | [源码](<../../algorithms/数据结构/线段树/泛型线段树.cpp>) | 1 | 0 | 2 | 有 | 有 | 有 |
| dynamicSeg | [源码](<../../algorithms/数据结构/线段树/泛型动态线段树.cpp>) | 1 | 0 | 2 | 有 | 有 | 未检出 |
| persistentSeg | [源码](<../../algorithms/数据结构/线段树/可持久化线段树/主席树.cpp>) | 1 | 0 | 2 | 有 | 有 | 有 |
| segSplit | [源码](<../../algorithms/数据结构/线段树/线段树合并分裂/线段树按值域分裂.cpp>) | 0 | 0 | 2 | 有 | 待补 | 有 |
| segSplitSort | [源码](<../../algorithms/数据结构/线段树/线段树合并分裂/线段树分裂解决区间排序后单点查询问题.cpp>) | 0 | 0 | 3 | 有 | 待补 | 未检出 |
| segMergeTree | [源码](<../../algorithms/数据结构/线段树/线段树合并分裂/线段树合并解决树上查分词频问题.cpp>) | 0 | 0 | 3 | 有 | 待补 | 未检出 |
| persistentLeftist | [源码](<../../algorithms/数据结构/堆/左偏树/可持久化左偏树.cpp>) | 1 | 0 | 3 | 有 | 有 | 有 |
| leftist | [源码](<../../algorithms/数据结构/堆/左偏树/左偏树.cpp>) | 1 | 0 | 6 | 有 | 有 | 有 |
| dsu | [源码](<../../algorithms/数据结构/并查集/DSU.cpp>) | 1 | 0 | 1 | 有 | 有 | 未检出 |
| wdsu | [源码](<../../algorithms/数据结构/并查集/WDSU.cpp>) | 1 | 0 | 3 | 有 | 有 | 有 |
| monoQueue | [源码](<../../algorithms/数据结构/单调队列/单调队列.cpp>) | 1 | 0 | 0 | 有 | 有 | 未检出 |
| bit | [源码](<../../algorithms/数据结构/树状数组/树状数组.cpp>) | 1 | 0 | 2 | 有 | 有 | 有 |
| bit2d | [源码](<../../algorithms/数据结构/树状数组/二维树状数组.cpp>) | 1 | 0 | 1 | 有 | 有 | 有 |
| segFhq | [源码](<../../algorithms/数据结构/树套树/线段树套FHQ_Treap.cpp>) | 0 | 0 | 3 | 有 | 待补 | 未检出 |
| st | [源码](<../../algorithms/数据结构/ST表/ST表.cpp>) | 1 | 0 | 2 | 有 | 有 | 有 |
| skiplist | [源码](<../../algorithms/数据结构/有序表与平衡树/跳表.cpp>) | 1 | 0 | 3 | 有 | 有 | 有 |
| avl | [源码](<../../algorithms/数据结构/有序表与平衡树/AVL.cpp>) | 1 | 0 | 2 | 有 | 有 | 有 |
| treap | [源码](<../../algorithms/数据结构/有序表与平衡树/Treap.cpp>) | 1 | 0 | 3 | 有 | 有 | 有 |
| fhq | [源码](<../../algorithms/数据结构/有序表与平衡树/FHQ_Treap.cpp>) | 1 | 0 | 3 | 有 | 有 | 有 |
| fhqSeq | [源码](<../../algorithms/数据结构/有序表与平衡树/FHQ_Treap_序列.cpp>) | 1 | 0 | 3 | 有 | 有 | 有 |
| scapegoat | [源码](<../../algorithms/数据结构/有序表与平衡树/替罪羊树.cpp>) | 1 | 0 | 3 | 有 | 有 | 有 |
| cartesian | [源码](<../../algorithms/数据结构/有序表与平衡树/笛卡尔树.cpp>) | 1 | 0 | 1 | 有 | 有 | 有 |
| splay | [源码](<../../algorithms/数据结构/有序表与平衡树/Splay.cpp>) | 1 | 0 | 2 | 有 | 有 | 有 |
| treeKnapDfs | [源码](<../../algorithms/动态规划/背包DP/树上背包/dfn序.cpp>) | 0 | 0 | 14 | 有 | 待补 | 未检出 |
| treeKnapBound | [源码](<../../algorithms/动态规划/背包DP/树上背包/上下界.cpp>) | 0 | 0 | 14 | 有 | 待补 | 未检出 |
| graph | [源码](<../../algorithms/图论/图的存储/Graph.cpp>) | 3 | 3 | 0 | 无 | 有 | 有 |
| scc | [源码](<../../algorithms/图论/连通性相关/Tarjan_SCC.cpp>) | 2 | 2 | 0 | 无 | 有 | 有 |
| ebcc | [源码](<../../algorithms/图论/连通性相关/Tarjan_EBCC.cpp>) | 2 | 2 | 0 | 无 | 有 | 有 |
| vbcc | [源码](<../../algorithms/图论/连通性相关/Tarjan_VBCC.cpp>) | 3 | 2 | 0 | 无 | 有 | 有 |
| bct | [源码](<../../algorithms/图论/连通性相关/圆方树/BCT.cpp>) | 3 | 2 | 0 | 无 | 有 | 有 |
| lca | [源码](<../../algorithms/图论/树上问题/最近公共祖先/DFN_LCA.cpp>) | 3 | 2 | 0 | 无 | 有 | 有 |
| hldlca | [源码](<../../algorithms/图论/树上问题/最近公共祖先/HLD_LCA.cpp>) | 2 | 2 | 0 | 无 | 有 | 有 |
| binliftLca | [源码](<../../algorithms/图论/树上问题/最近公共祖先/树上倍增LCA.cpp>) | 0 | 0 | 3 | 有 | 待补 | 未检出 |
| eulerLca | [源码](<../../algorithms/图论/树上问题/最近公共祖先/欧拉序LCA.cpp>) | 0 | 0 | 3 | 有 | 待补 | 未检出 |
| tarjanLca | [源码](<../../algorithms/图论/树上问题/最近公共祖先/tarjan.cpp>) | 0 | 0 | 2 | 有 | 待补 | 未检出 |
| hld | [源码](<../../algorithms/图论/树上问题/树链剖分/HLD.cpp>) | 2 | 2 | 0 | 无 | 有 | 有 |
| virtualTree | [源码](<../../algorithms/图论/树上问题/虚树/二次排序.cpp>) | 2 | 2 | 0 | 无 | 有 | 有 |
| virtualTreeStack | [源码](<../../algorithms/图论/树上问题/虚树/单调栈.cpp>) | 2 | 2 | 0 | 无 | 有 | 有 |
| diameterDfs | [源码](<../../algorithms/图论/树上问题/树的直径/两次dfs.cpp>) | 2 | 2 | 0 | 无 | 有 | 有 |
| diameterDp | [源码](<../../algorithms/图论/树上问题/树的直径/树形dp法.cpp>) | 2 | 2 | 0 | 无 | 有 | 有 |
| centroid | [源码](<../../algorithms/图论/树上问题/树的重心/树的重心.cpp>) | 2 | 2 | 0 | 无 | 有 | 有 |
| center | [源码](<../../algorithms/图论/树上问题/树的中心/树的中心.cpp>) | 2 | 2 | 0 | 无 | 有 | 有 |
| eulerUndirected | [源码](<../../algorithms/图论/欧拉图/无向图欧拉边路径.cpp>) | 0 | 0 | 0 | 有 | 有 | 未检出 |
| eulerDirected | [源码](<../../algorithms/图论/欧拉图/有向图欧拉边路径.cpp>) | 0 | 0 | 0 | 有 | 有 | 未检出 |
| eulerPointUndirected | [源码](<../../algorithms/图论/欧拉图/无向图欧拉点路径.cpp>) | 0 | 0 | 2 | 有 | 待补 | 未检出 |
| eulerPointDirected | [源码](<../../algorithms/图论/欧拉图/有向图欧拉点路径.cpp>) | 0 | 0 | 1 | 有 | 待补 | 未检出 |
| topo | [源码](<../../algorithms/图论/拓扑排序/拓扑排序.cpp>) | 2 | 2 | 0 | 无 | 有 | 有 |
| segGraph | [源码](<../../algorithms/图论/优化建图/线段树优化建图.cpp>) | 1 | 0 | 0 | 有 | 有 | 有 |
| dij | [源码](<../../algorithms/图论/最短路问题/单源最短路径/dijkstra_heap.cpp>) | 2 | 0 | 1 | 有 | 有 | 有 |
| dijN | [源码](<../../algorithms/图论/最短路问题/单源最短路径/dijkstra.cpp>) | 1 | 0 | 0 | 有 | 有 | 有 |
| spfa | [源码](<../../algorithms/图论/最短路问题/单源最短路径/spfa 存图.cpp>) | 1 | 0 | 1 | 有 | 有 | 有 |
| bf | [源码](<../../algorithms/图论/最短路问题/单源最短路径/Bellman-Ford存图.cpp>) | 1 | 0 | 0 | 有 | 有 | 有 |
| bfRing | [源码](<../../algorithms/图论/最短路问题/判断负环/bf_-ring.cpp>) | 1 | 0 | 0 | 有 | 有 | 有 |
| spfaRing | [源码](<../../algorithms/图论/最短路问题/判断负环/spfa_-ring.cpp>) | 1 | 0 | 1 | 有 | 有 | 有 |
| floyd | [源码](<../../algorithms/图论/最短路问题/全源最短路径/floyd.cpp>) | 0 | 0 | 2 | 有 | 待补 | 未检出 |
| trie | [源码](<../../algorithms/字符串/字典树/字典树.cpp>) | 1 | 0 | 4 | 有 | 有 | 有 |
| persistentTrie | [源码](<../../algorithms/字符串/字典树/可持久化字典树.cpp>) | 2 | 0 | 4 | 有 | 有 | 有 |
| mint | [源码](<../../algorithms/数学/数论/mint.cpp>) | 1 | 0 | 9 | 有 | 有 | 有 |
| segSieve | [源码](<../../algorithms/数学/数论/区间筛质数.cpp>) | 1 | 0 | 0 | 无 | 有 | 有 |
| intSqrt | [源码](<../../algorithms/数学/数论/整数开方.cpp>) | 1 | 0 | 0 | 无 | 有 | 有 |
| divisorSums | [源码](<../../algorithms/数学/数论/约数个数及约数和公式法.cpp>) | 1 | 0 | 0 | 无 | 有 | 有 |
| divCeilFloor | [源码](<../../algorithms/数学/数论/除法上下取整.cpp>) | 0 | 0 | 0 | 无 | 待补 | 未检出 |
| bigint | [源码](<../../algorithms/数学/高精度计算/高精度.cpp>) | 1 | 0 | 9 | 有 | 有 | 有 |
| lucas | [源码](<../../algorithms/数学/组合数学/卢卡斯定理求组合数.cpp>) | 0 | 0 | 1 | 有 | 待补 | 未检出 |
| binomLoop | [源码](<../../algorithms/数学/组合数学/循环求组合数.cpp>) | 0 | 0 | 1 | 有 | 待补 | 未检出 |
| binomPascal | [源码](<../../algorithms/数学/组合数学/打表杨辉三角求组合数.cpp>) | 0 | 0 | 1 | 有 | 待补 | 未检出 |
| binomFactInv | [源码](<../../algorithms/数学/组合数学/阶乘表及阶乘逆元表求组合数.cpp>) | 0 | 0 | 1 | 有 | 待补 | 未检出 |
| utils | [源码](<../../algorithms/杂项/utils/utils.cpp>) | 0 | 0 | 27 | 有 | 有 | 未检出 |
| rw | [源码](<../../algorithms/杂项/快读快写/快读快写.cpp>) | 1 | 0 | 4 | 有 | 有 | 未检出 |
| i128 | [源码](<../../algorithms/杂项/128位整数/128int.cpp>) | 0 | 0 | 0 | 有 | 有 | 未检出 |
| rnd | [源码](<../../algorithms/杂项/随机数/z_rnd.cpp>) | 1 | 0 | 0 | 有 | 有 | 有 |
| customHash | [源码](<../../algorithms/杂项/防卡/哈希防卡.cpp>) | 1 | 0 | 0 | 有 | 有 | 未检出 |
| discrete | [源码](<../../algorithms/杂项/离散化/离散化.cpp>) | 1 | 0 | 0 | 有 | 有 | 未检出 |
| misraGries | [源码](<../../algorithms/杂项/主元素问题/Misra-Gries.cpp>) | 0 | 0 | 2 | 有 | 待补 | 未检出 |
