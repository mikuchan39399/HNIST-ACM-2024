# 模板生命周期接口统一

2026-09-09。用户要求统一 build/init/clear, 并明确要求不遗漏算法。本次按 catalog 的稳定短名逐项核对全部 78 份现役源码: 74 份纳入接口审查, 欧拉相关 4 份按先前明确指令暂缓。另扫描 algorithms 下所有非对拍源码的同名函数; 17 份登记笔记无类式复位协议, 插件 Tag.clear 是标记代数操作, 例题与明确未维护代码不冒充现役引擎。

## 对使用者的规则

- 完整输入已经具备时, build/run 自行复位本轮计算状态, 无需先调用 init。外置图仍由调用者 clear 和重新填边。
- init(n,配置) 建立可继续填输入或做操作的状态, 比如 DSU、BIT、带权重心、离线 Tarjan 和预处理查询表; 不为了命名统一拆出无意义阶段。
- clear 丢弃内容并保留容量/既有配置; 新建持久化版本、共享池内建新根和局部递归不清共享池。
- 构造容量与本轮 n 分开; 只按实际 n、实际边和上轮结果复位, 不从 Graph 点容量或 node_cnt 推断 n。

## 调用迁移

| 原调用 | 本轮调用 |
|---|---|
| lca.init(n); lca.build(g) | lca.build(g,n) |
| hld.init(n); hld.build(g,root) | hld.build(g,n,root) |
| hld.init(n); hld.build(g) | hld.build(g,n,-1) |
| scc/ebcc/vbcc/bct.init(n); build(g,n) | 直接 build(g,n), 指定根仍可传第三参数 |
| dij/spfa/bf.init(n); run(s,g) | run(s,g,n), 堆 Dijkstra 多源同样传 n |
| bfRing/spfaRing.init(n); run(g) | run(g,n) |
| seg.init(n); seg.build(a) | seg.build(a), a 为 1-based, 本轮 n=a.size()-1 |
| 旧线段树套 FHQ build(1,1,n) | 先填 a, 再 build(n); build_node 仅作递归内部入口 |
| PersistentLeftist.init() | PersistentLeftist.clear() |
| 备用倍增 LCA build() / 欧拉序 LCA build(root) | build(n) / build(n,root), 保留原前向星输入 |

HLD/HLD_LCA 的 root 参数故意不设默认值, 使旧 build(g,root) 编译失败, 避免第二参数静默变成点数。库内源码、Usage 和对拍已迁移; 仓库外题解不会自动改写, 用上表迁移。保留的 init 辅助方法可以显式清状态, 正常完整重建不再依赖它。

普通 SegTree 在建树访问到的结点覆写 Tag{}, 不先重复扫描 4n 表。SCC/EBCC/VBCC/BCT 的派生图重建只清自己的 dag/tree, 不抹原图或分解结果。TreeCentroid 的 pt、TarjanLCA 的边和询问是输入, 重建必须保留; PersSegTree.build 同值域追加新根必须保留旧版本。

## 逐项清单

| 短名 | 源码 | 本轮结论 |
|---|---|---|
| seg | [泛型线段树.cpp](<../../algorithms/数据结构/线段树/泛型线段树.cpp>) | 已改: build(a) 从 a.size()-1 设置长度, 在建树遍历中覆写懒标记, 不重复全表清零 |
| dynamicSeg | [泛型动态线段树.cpp](<../../algorithms/数据结构/线段树/泛型动态线段树.cpp>) | 保留: 完整 build 已自行复位/覆写本轮结果; 同步受影响 Usage 与调用 |
| persistentSeg | [主席树.cpp](<../../algorithms/数据结构/线段树/可持久化线段树/主席树.cpp>) | 保留: clear 使全部版本失效; 新建根或派生版本不清共享池, 同值域旧根仍可查询 |
| segSplit | [线段树按值域分裂.cpp](<../../algorithms/数据结构/线段树/线段树合并分裂/线段树按值域分裂.cpp>) | 已检查旧全局题目骨架: 不存在本轮所改的类式 build 协议, 不把一次性 main 或局部递归改成自动清池, 未宣称可任意多测复用 |
| segSplitSort | [线段树分裂解决区间排序后单点查询问题.cpp](<../../algorithms/数据结构/线段树/线段树合并分裂/线段树分裂解决区间排序后单点查询问题.cpp>) | 已检查旧全局题目骨架: 不存在本轮所改的类式 build 协议, 不把一次性 main 或局部递归改成自动清池, 未宣称可任意多测复用 |
| segMergeTree | [线段树合并解决树上查分词频问题.cpp](<../../algorithms/数据结构/线段树/线段树合并分裂/线段树合并解决树上查分词频问题.cpp>) | 已检查旧全局题目骨架: 不存在本轮所改的类式 build 协议, 不把一次性 main 或局部递归改成自动清池, 未宣称可任意多测复用 |
| persistentLeftist | [可持久化左偏树.cpp](<../../algorithms/数据结构/堆/左偏树/可持久化左偏树.cpp>) | 已改: 纯池复位 init() 更名 clear(); merge/insert/pop 仍保留其他版本 |
| leftist | [左偏树.cpp](<../../algorithms/数据结构/堆/左偏树/左偏树.cpp>) | 保留: init 设置规模/模式/初值后继续操作, 是有意义的初始状态, 不引入多余 build |
| dsu | [DSU.cpp](<../../algorithms/数据结构/并查集/DSU.cpp>) | 保留: init 设置规模/模式/初值后继续操作, 是有意义的初始状态, 不引入多余 build |
| wdsu | [WDSU.cpp](<../../algorithms/数据结构/并查集/WDSU.cpp>) | 保留: init 设置规模/模式/初值后继续操作, 是有意义的初始状态, 不引入多余 build |
| monoQueue | [单调队列.cpp](<../../algorithms/数据结构/单调队列/单调队列.cpp>) | 保留: 无需 init+build 的外置完整重建协议; 局部算法/值对象/IO/随机状态各按现有契约 |
| monoStack | [单调栈.cpp](<../../algorithms/数据结构/单调栈/单调栈.cpp>) | 保留: 无需 init+build 的外置完整重建协议; 局部算法/值对象/IO/随机状态各按现有契约 |
| bit | [树状数组.cpp](<../../algorithms/数据结构/树状数组/树状数组.cpp>) | 保留: init 设置规模/模式/初值后继续操作, 是有意义的初始状态, 不引入多余 build |
| bit2d | [二维树状数组.cpp](<../../algorithms/数据结构/树状数组/二维树状数组.cpp>) | 保留: init 设置规模/模式/初值后继续操作, 是有意义的初始状态, 不引入多余 build |
| segFhq | [线段树套FHQ_Treap.cpp](<../../algorithms/数据结构/树套树/线段树套FHQ_Treap.cpp>) | 已改: build(n) 保留已填 a 并清根/池; 原递归入口改为 build_node, 不在递归中清池 |
| st | [ST表.cpp](<../../algorithms/数据结构/ST表/ST表.cpp>) | 保留: 完整 build 已自行复位/覆写本轮结果; 同步受影响 Usage 与调用 |
| skiplist | [跳表.cpp](<../../algorithms/数据结构/有序表与平衡树/跳表.cpp>) | 保留: clear 丢弃当前内容并保留容量; 后续由 add/insert 填充, 无额外计算前复位要求 |
| avl | [AVL.cpp](<../../algorithms/数据结构/有序表与平衡树/AVL.cpp>) | 保留: clear 丢弃当前内容并保留容量; 后续由 add/insert 填充, 无额外计算前复位要求 |
| treap | [Treap.cpp](<../../algorithms/数据结构/有序表与平衡树/Treap.cpp>) | 保留: 完整 build 已自行复位/覆写本轮结果; 同步受影响 Usage 与调用 |
| fhq | [FHQ_Treap.cpp](<../../algorithms/数据结构/有序表与平衡树/FHQ_Treap.cpp>) | 保留: 完整 build 已自行复位/覆写本轮结果; 同步受影响 Usage 与调用 |
| fhqSeq | [FHQ_Treap_序列.cpp](<../../algorithms/数据结构/有序表与平衡树/FHQ_Treap_序列.cpp>) | 保留: 完整 build 已自行复位/覆写本轮结果; 同步受影响 Usage 与调用 |
| scapegoat | [替罪羊树.cpp](<../../algorithms/数据结构/有序表与平衡树/替罪羊树.cpp>) | 保留: clear 丢弃当前内容并保留容量; 后续由 add/insert 填充, 无额外计算前复位要求 |
| cartesian | [笛卡尔树.cpp](<../../algorithms/数据结构/有序表与平衡树/笛卡尔树.cpp>) | 保留: 完整 build 已自行复位/覆写本轮结果; 同步受影响 Usage 与调用 |
| splay | [Splay.cpp](<../../algorithms/数据结构/有序表与平衡树/Splay.cpp>) | 保留: 完整 build 已自行复位/覆写本轮结果; 同步受影响 Usage 与调用 |
| treeKnapDfs | [dfn序.cpp](<../../algorithms/动态规划/背包DP/树上背包/dfn序.cpp>) | 已检查旧全局题目骨架: 不存在本轮所改的类式 build 协议, 不把一次性 main 或局部递归改成自动清池, 未宣称可任意多测复用 |
| treeKnapBound | [上下界.cpp](<../../algorithms/动态规划/背包DP/树上背包/上下界.cpp>) | 已检查旧全局题目骨架: 不存在本轮所改的类式 build 协议, 不把一次性 main 或局部递归改成自动清池, 未宣称可任意多测复用 |
| graph | [Graph.cpp](<../../algorithms/图论/图的存储/Graph.cpp>) | 保留: clear 丢弃当前内容并保留容量; 后续由 add/insert 填充, 无额外计算前复位要求 |
| scc | [Tarjan_SCC.cpp](<../../algorithms/图论/连通性相关/Tarjan_SCC.cpp>) | 已改: build(g,n[,root]) 自动复位; build_dag/build_tree 重建自身派生图而非追加, 不清外置原图 |
| ebcc | [Tarjan_EBCC.cpp](<../../algorithms/图论/连通性相关/Tarjan_EBCC.cpp>) | 已改: build(g,n[,root]) 自动复位; build_dag/build_tree 重建自身派生图而非追加, 不清外置原图 |
| vbcc | [Tarjan_VBCC.cpp](<../../algorithms/图论/连通性相关/Tarjan_VBCC.cpp>) | 已改: build(g,n[,root]) 自动复位; build_dag/build_tree 重建自身派生图而非追加, 不清外置原图 |
| bct | [BCT.cpp](<../../algorithms/图论/连通性相关/圆方树/BCT.cpp>) | 已改: build(g,n[,root]) 自动复位; build_dag/build_tree 重建自身派生图而非追加, 不清外置原图 |
| lca | [DFN_LCA.cpp](<../../algorithms/图论/树上问题/最近公共祖先/DFN_LCA.cpp>) | 已改: build(g,n) 内部复位, 删除旧单参数调用 |
| hldlca | [HLD_LCA.cpp](<../../algorithms/图论/树上问题/最近公共祖先/HLD_LCA.cpp>) | 已改: build(g,n,root) 内部复位; root 显式传 -1 或指定根, 旧一/二参数调用编译失败 |
| binliftLca | [树上倍增LCA.cpp](<../../algorithms/图论/树上问题/最近公共祖先/树上倍增LCA.cpp>) | 已改: 保留绑定的前向星输入, build(n) 清 rt 并重建; 旧固定层数限制保留 |
| eulerLca | [欧拉序LCA.cpp](<../../algorithms/图论/树上问题/最近公共祖先/欧拉序LCA.cpp>) | 已改: 保留绑定输入, build(n,root) 重建; 查询仅在已建树内 |
| tarjanLca | [tarjan.cpp](<../../algorithms/图论/树上问题/最近公共祖先/tarjan.cpp>) | 已改: init(n,m) 清输入, add_edge/add_query 填输入; build() 仅复位并查集、访问与答案, 保留边和询问 |
| hld | [HLD.cpp](<../../algorithms/图论/树上问题/树链剖分/HLD.cpp>) | 已改: build(g,n,root) 内部复位; root 显式传 -1 或指定根, 旧一/二参数调用编译失败 |
| virtualTree | [二次排序.cpp](<../../algorithms/图论/树上问题/虚树/二次排序.cpp>) | 保留: 完整 build 已自行复位/覆写本轮结果; 同步受影响 Usage 与调用 |
| virtualTreeStack | [单调栈.cpp](<../../algorithms/图论/树上问题/虚树/单调栈.cpp>) | 保留: 完整 build 已自行复位/覆写本轮结果; 同步受影响 Usage 与调用 |
| diameterDfs | [两次dfs.cpp](<../../algorithms/图论/树上问题/树的直径/两次dfs.cpp>) | 保留: 完整 build 已自行复位/覆写本轮结果; 同步受影响 Usage 与调用 |
| diameterDp | [树形dp法.cpp](<../../algorithms/图论/树上问题/树的直径/树形dp法.cpp>) | 保留: 完整 build 已自行复位/覆写本轮结果; 同步受影响 Usage 与调用 |
| centroid | [树的重心.cpp](<../../algorithms/图论/树上问题/树的重心/树的重心.cpp>) | 保留: init(n) 把点权恢复为 1, build(g,n) 保留 pt 输入并重算结果; 禁止 build 内调用会抹点权的 init |
| center | [树的中心.cpp](<../../algorithms/图论/树上问题/树的中心/树的中心.cpp>) | 保留: 完整 build 已自行复位/覆写本轮结果; 同步受影响 Usage 与调用 |
| eulerUndirected | [无向图欧拉边路径.cpp](<../../algorithms/图论/欧拉图/无向图欧拉边路径.cpp>) | 用户明确暂缓: 欧拉相关四份旧模板, 本轮不改源码或新增行为验收 |
| eulerDirected | [有向图欧拉边路径.cpp](<../../algorithms/图论/欧拉图/有向图欧拉边路径.cpp>) | 用户明确暂缓: 欧拉相关四份旧模板, 本轮不改源码或新增行为验收 |
| eulerPointUndirected | [无向图欧拉点路径.cpp](<../../algorithms/图论/欧拉图/无向图欧拉点路径.cpp>) | 用户明确暂缓: 欧拉相关四份旧模板, 本轮不改源码或新增行为验收 |
| eulerPointDirected | [有向图欧拉点路径.cpp](<../../algorithms/图论/欧拉图/有向图欧拉点路径.cpp>) | 用户明确暂缓: 欧拉相关四份旧模板, 本轮不改源码或新增行为验收 |
| topo | [拓扑排序.cpp](<../../algorithms/图论/拓扑排序/拓扑排序.cpp>) | 保留: 完整 build 已自行复位/覆写本轮结果; 同步受影响 Usage 与调用 |
| segGraph | [线段树优化建图.cpp](<../../algorithms/图论/优化建图/线段树优化建图.cpp>) | 保留: 完整 build 已自行复位/覆写本轮结果; 同步受影响 Usage 与调用 |
| treeGraph | [树上倍增优化建图.cpp](<../../algorithms/图论/优化建图/树上倍增优化建图.cpp>) | 保留: 完整 build 已自行复位/覆写本轮结果; 同步受影响 Usage 与调用 |
| dij | [dijkstra_heap.cpp](<../../algorithms/图论/最短路问题/单源最短路径/dijkstra_heap.cpp>) | 已改: run(source,g,n) 自动复位; 同时接受整个源点数组, 空多源清掉旧答案 |
| dijN | [dijkstra.cpp](<../../algorithms/图论/最短路问题/单源最短路径/dijkstra.cpp>) | 已改: run(source,g,n) 自动复位, 不再要求先 init |
| spfa | [spfa 存图.cpp](<../../algorithms/图论/最短路问题/单源最短路径/spfa 存图.cpp>) | 已改: run(source,g,n) 自动复位, 不再要求先 init |
| bf | [Bellman-Ford存图.cpp](<../../algorithms/图论/最短路问题/单源最短路径/Bellman-Ford存图.cpp>) | 已改: run(source,g,n) 自动复位, 不再要求先 init |
| bfRing | [bf_-ring.cpp](<../../algorithms/图论/最短路问题/判断负环/bf_-ring.cpp>) | 已改: run(g,n) 显式设置本轮规模并自动复位, 不再要求先 init |
| spfaRing | [spfa_-ring.cpp](<../../algorithms/图论/最短路问题/判断负环/spfa_-ring.cpp>) | 已改: run(g,n) 显式设置本轮规模并自动复位, 不再要求先 init |
| floyd | [floyd.cpp](<../../algorithms/图论/最短路问题/全源最短路径/floyd.cpp>) | 已检查旧全局题目骨架: 不存在本轮所改的类式 build 协议, 不把一次性 main 或局部递归改成自动清池, 未宣称可任意多测复用 |
| kruskal | [Kruskal.cpp](<../../algorithms/图论/生成树问题/最小生成树/kruskal/Kruskal.cpp>) | 保留: 完整 build 已自行复位/覆写本轮结果; 同步受影响 Usage 与调用 |
| prim | [Prim.cpp](<../../algorithms/图论/生成树问题/最小生成树/prim/Prim.cpp>) | 保留: 完整 build 已自行复位/覆写本轮结果; 同步受影响 Usage 与调用 |
| trie | [字典树.cpp](<../../algorithms/字符串/字典树/字典树.cpp>) | 保留: clear 丢弃当前内容并保留容量; 后续由 add/insert 填充, 无额外计算前复位要求 |
| persistentTrie | [可持久化字典树.cpp](<../../algorithms/字符串/字典树/可持久化字典树.cpp>) | 保留: clear 使全部版本失效; 新建根或派生版本不清共享池, 同值域旧根仍可查询 |
| mint | [mint.cpp](<../../algorithms/数学/数论/mint.cpp>) | 保留: 配置/预处理接口一次建立可查询状态, 已覆盖旧表; init_fact 是专用共享缓存接口, 不按名字机械改动 |
| eulerSieve | [欧拉筛.cpp](<../../algorithms/数学/数论/质数筛/欧拉筛.cpp>) | 保留: 配置/预处理接口一次建立可查询状态, 已覆盖旧表; init_fact 是专用共享缓存接口, 不按名字机械改动 |
| eratosthenes | [埃氏筛.cpp](<../../algorithms/数学/数论/质数筛/埃氏筛.cpp>) | 保留: 配置/预处理接口一次建立可查询状态, 已覆盖旧表; init_fact 是专用共享缓存接口, 不按名字机械改动 |
| segSieve | [区间筛质数.cpp](<../../algorithms/数学/数论/质数筛/区间筛质数.cpp>) | 保留: 无需 init+build 的外置完整重建协议; 局部算法/值对象/IO/随机状态各按现有契约 |
| divisorSums | [约数个数及约数和公式法.cpp](<../../algorithms/数学/数论/约数/约数个数及约数和公式法.cpp>) | 保留: 无需 init+build 的外置完整重建协议; 局部算法/值对象/IO/随机状态各按现有契约 |
| bigint | [高精度.cpp](<../../algorithms/数学/高精度计算/高精度.cpp>) | 保留: 无需 init+build 的外置完整重建协议; 局部算法/值对象/IO/随机状态各按现有契约 |
| lucas | [卢卡斯定理求组合数.cpp](<../../algorithms/数学/组合数学/卢卡斯定理求组合数.cpp>) | 保留: 配置/预处理接口一次建立可查询状态, 已覆盖旧表; init_fact 是专用共享缓存接口, 不按名字机械改动 |
| binomLoop | [循环求组合数.cpp](<../../algorithms/数学/组合数学/循环求组合数.cpp>) | 保留: 无需 init+build 的外置完整重建协议; 局部算法/值对象/IO/随机状态各按现有契约 |
| binomPascal | [打表杨辉三角求组合数.cpp](<../../algorithms/数学/组合数学/打表杨辉三角求组合数.cpp>) | 保留: 配置/预处理接口一次建立可查询状态, 已覆盖旧表; init_fact 是专用共享缓存接口, 不按名字机械改动 |
| binomFactInv | [阶乘表及阶乘逆元表求组合数.cpp](<../../algorithms/数学/组合数学/阶乘表及阶乘逆元表求组合数.cpp>) | 保留: 配置/预处理接口一次建立可查询状态, 已覆盖旧表; init_fact 是专用共享缓存接口, 不按名字机械改动 |
| utils | [utils.cpp](<../../algorithms/杂项/utils/utils.cpp>) | 保留: 无需 init+build 的外置完整重建协议; 局部算法/值对象/IO/随机状态各按现有契约 |
| rw | [快读快写.cpp](<../../algorithms/杂项/快读快写/快读快写.cpp>) | 保留: 无需 init+build 的外置完整重建协议; 局部算法/值对象/IO/随机状态各按现有契约 |
| i128 | [128int.cpp](<../../algorithms/杂项/128位整数/128int.cpp>) | 保留: 无需 init+build 的外置完整重建协议; 局部算法/值对象/IO/随机状态各按现有契约 |
| rnd | [z_rnd.cpp](<../../algorithms/杂项/随机数/z_rnd.cpp>) | 保留: 无需 init+build 的外置完整重建协议; 局部算法/值对象/IO/随机状态各按现有契约 |
| customHash | [哈希防卡.cpp](<../../algorithms/杂项/防卡/哈希防卡.cpp>) | 保留: 无需 init+build 的外置完整重建协议; 局部算法/值对象/IO/随机状态各按现有契约 |
| discrete | [离散化.cpp](<../../algorithms/杂项/离散化/离散化.cpp>) | 保留: clear 丢弃候选值, add 收集输入, build 排序去重并保留候选集合 |
| misraGries | [Misra-Gries.cpp](<../../algorithms/杂项/主元素问题/Misra-Gries.cpp>) | 保留: 无需 init+build 的外置完整重建协议; 局部算法/值对象/IO/随机状态各按现有契约 |

## 验证范围

既有 graph/conn/HLD/LCA-VT/优化建图和普通线段树回归改为直接 build/run, 保留独立暴力、规模和大—小—大测试。新增 lifecycle_check 每组 350 轮: 备用三份 LCA 用独立父链/BFS 全点对, SCC 派生图重复构建用星形图闭式边数, 普通树含 20 万长度重建和遗留懒标记, 另验证持久化旧根与重心点权保留。segfhq_lifecycle_check 用 350×100 数组排序对拍四种查询与单点修改, 再跑 20万—1—0—257—20万重建。

备用 LCA 仍保留原接口的数据结构、固定层数/递归限制, 本轮不声称已完成全部现代化或最大规模验收。旧全局题目骨架没有借本轮检查升级为通用多测引擎。欧拉四份保持原状。学习状态没有新的用户陈述, 不作提升。

实际修改 19 份引擎的实现或接口, 另 4 份优化建图/虚树源码仅迁移 Usage; 保留项逐行见上表。

- Windows / g++ 15.2 / C++20 O2、警告视为错误、断言开启: [全量 135/135 通过](runs/eff3a528f52a49fe9a424f5f2ec64258.json), 包含 36 套回归和 99 份语法目标。运行后有少量接口断言、调用说明及范围登记更新, 对应套件又实际补跑, 不把早期结果冒充最终指纹。
- 最终补跑证据: [recheck-completed_graph](runs/8e779668a54246ad965aad5e144cdf12.json)、[recheck-hld_check](runs/95fd797bab5f47378fdf2f7478de166a.json)、[recheck-conn_check](runs/f4f0d744884d4f55a64cb32f5d6c4ed7.json)、[recheck-conn_bct](runs/689506af04a4407cac9916f5036bc596.json)、[recheck-lifecycle](runs/f9a0094498d44cd6b35565724fff2566.json)、[scope-final-graph_check](runs/146c190bc1234c0885d3c0fbe0d7a046.json)、[scope-final-hld_check](runs/d42dde9f3dae4b9e9080c64e1665d111.json)、[scope-final-lca_vt_stress_check](runs/2a4207a7796d491ba9c71d13cfd1f0dd.json)、[scope-final-seg_check](runs/16ad48ae88944956ae15d424ef3e7ed2.json)。
- Linux / Ubuntu-20.04 / g++-13 13.1 / ASan+UBSan: [15 套受影响回归全部通过](lifecycle-sanitizer-20260909.json), 包含编译参数、标准输出/错误及依赖哈希; 最终收尾再次核对所有依赖哈希一致。
- [LCA/虚树规模测试](lifecycle-lca-vt-stress-20260909.json): 百万点浅树(8 MiB 栈)和深链(256 MiB 栈)通过。20 万点递归 DFN/HLD 在 sanitizer、8 MiB 栈的两个探针均明确报 stack-overflow, 记为 STACK_LIMIT; 不宣称默认栈支持该深度。
- [图论规模测试](lifecycle-completed-graph-stress-20260909.json): 20 万点浅图与深链, 普通和 sanitizer 四项全部通过; 深链使用 256 MiB 栈, 浅图 8 MiB。

这是一轮生命周期接口与相关行为验证, 不是全库所有算法已完成最大规模验收。9 个无直接对拍 include 的历史缺口仍保留: 三份线段树合并分裂、两份树上背包及用户暂缓的四份欧拉。备用三份 LCA 的固定层数、递归和前向星接口限制仍按上文记录。

文档检查通过: 114 份 Markdown 的链接/锚点与双入口、222 个设计条目、功能目录及验证资产表同步检查均通过; 当前已登记范围没有待重验项, git diff --check 通过。

证据沉淀后已核对精确目录与无重解析点, 删除本轮 `.zoi-checks/codex-work/lifecycle-20260909` 的 834 个临时文件(36,220,888 字节)。未提交、未推送; 用户原有 scripts/_snap.ps1 未修改, 仓库外题解未修改。
