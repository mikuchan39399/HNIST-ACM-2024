# 模板对拍资产表（自动生成）

<!-- 由 scripts/make_reliability.ps1 自动生成，请勿手工修改。 -->

A/B/C 只表示静态测试资产关系，不代表可靠程度或正确性。直接 include（包括条件分支中的引用）不能证明接口被调用、暴力答案独立或测试已通过；不统计宏式引用和传递依赖。

数据来自 zoi/_catalog.txt 和 algorithms/**/*_check.cpp。纯跳板解析到母版，忽略注释中的示例。运行结果、环境和历史审查记录见运行报告与[协作历史](sweep-history.md)。

更新：`./scripts/make_reliability.ps1`；只读检查：`./scripts/make_reliability.ps1 -Check`。普通回归不会改写本表。

## A 类：有直接对拍引用的现役模板 [52]

| 模板或源文件 | 直接引用它的对拍文件 |
|---|---|
| [seg - algorithms/数据结构/线段树/泛型线段树.cpp](<../algorithms/数据结构/线段树/泛型线段树.cpp>) | [algorithms/数据结构/线段树/对拍/seg_check.cpp](<../algorithms/数据结构/线段树/对拍/seg_check.cpp>) |
| [dynamicSeg - algorithms/数据结构/线段树/泛型动态线段树.cpp](<../algorithms/数据结构/线段树/泛型动态线段树.cpp>) | [algorithms/数据结构/线段树/对拍/seg_check.cpp](<../algorithms/数据结构/线段树/对拍/seg_check.cpp>) |
| [persistentSeg - algorithms/数据结构/线段树/可持久化线段树/主席树.cpp](<../algorithms/数据结构/线段树/可持久化线段树/主席树.cpp>) | [algorithms/数据结构/线段树/对拍/pst_check.cpp](<../algorithms/数据结构/线段树/对拍/pst_check.cpp>) |
| [persistentLeftist - algorithms/数据结构/堆/左偏树/可持久化左偏树.cpp](<../algorithms/数据结构/堆/左偏树/可持久化左偏树.cpp>) | [algorithms/数据结构/堆/左偏树/对拍/leftist_check.cpp](<../algorithms/数据结构/堆/左偏树/对拍/leftist_check.cpp>) |
| [leftist - algorithms/数据结构/堆/左偏树/左偏树.cpp](<../algorithms/数据结构/堆/左偏树/左偏树.cpp>) | [algorithms/数据结构/堆/左偏树/对拍/leftist_check.cpp](<../algorithms/数据结构/堆/左偏树/对拍/leftist_check.cpp>) |
| [dsu - algorithms/数据结构/并查集/DSU.cpp](<../algorithms/数据结构/并查集/DSU.cpp>) | [algorithms/数据结构/并查集/对拍/dsu_check.cpp](<../algorithms/数据结构/并查集/对拍/dsu_check.cpp>) |
| [wdsu - algorithms/数据结构/并查集/WDSU.cpp](<../algorithms/数据结构/并查集/WDSU.cpp>) | [algorithms/数据结构/并查集/对拍/dsu_check.cpp](<../algorithms/数据结构/并查集/对拍/dsu_check.cpp>) |
| [monoQueue - algorithms/数据结构/单调队列/单调队列.cpp](<../algorithms/数据结构/单调队列/单调队列.cpp>) | [algorithms/杂项/对拍/misc_check.cpp](<../algorithms/杂项/对拍/misc_check.cpp>) |
| [bit - algorithms/数据结构/树状数组/树状数组.cpp](<../algorithms/数据结构/树状数组/树状数组.cpp>) | [algorithms/数据结构/树状数组/对拍/bit_check.cpp](<../algorithms/数据结构/树状数组/对拍/bit_check.cpp>) |
| [bit2d - algorithms/数据结构/树状数组/二维树状数组.cpp](<../algorithms/数据结构/树状数组/二维树状数组.cpp>) | [algorithms/数据结构/树状数组/对拍/bit_check.cpp](<../algorithms/数据结构/树状数组/对拍/bit_check.cpp>) |
| [st - algorithms/数据结构/ST表/ST表.cpp](<../algorithms/数据结构/ST表/ST表.cpp>) | [algorithms/数据结构/线段树/对拍/seg_check.cpp](<../algorithms/数据结构/线段树/对拍/seg_check.cpp>) |
| [skiplist - algorithms/数据结构/有序表与平衡树/跳表.cpp](<../algorithms/数据结构/有序表与平衡树/跳表.cpp>) | [algorithms/数据结构/有序表与平衡树/对拍/oset_check.cpp](<../algorithms/数据结构/有序表与平衡树/对拍/oset_check.cpp>) |
| [avl - algorithms/数据结构/有序表与平衡树/AVL.cpp](<../algorithms/数据结构/有序表与平衡树/AVL.cpp>) | [algorithms/数据结构/有序表与平衡树/对拍/oset_check.cpp](<../algorithms/数据结构/有序表与平衡树/对拍/oset_check.cpp>) |
| [treap - algorithms/数据结构/有序表与平衡树/Treap.cpp](<../algorithms/数据结构/有序表与平衡树/Treap.cpp>) | [algorithms/数据结构/有序表与平衡树/对拍/oset_check.cpp](<../algorithms/数据结构/有序表与平衡树/对拍/oset_check.cpp>) |
| [fhq - algorithms/数据结构/有序表与平衡树/FHQ_Treap.cpp](<../algorithms/数据结构/有序表与平衡树/FHQ_Treap.cpp>) | [algorithms/数据结构/有序表与平衡树/对拍/oset_check.cpp](<../algorithms/数据结构/有序表与平衡树/对拍/oset_check.cpp>) |
| [fhqSeq - algorithms/数据结构/有序表与平衡树/FHQ_Treap_序列.cpp](<../algorithms/数据结构/有序表与平衡树/FHQ_Treap_序列.cpp>) | [algorithms/数据结构/有序表与平衡树/对拍/oset_check.cpp](<../algorithms/数据结构/有序表与平衡树/对拍/oset_check.cpp>) |
| [scapegoat - algorithms/数据结构/有序表与平衡树/替罪羊树.cpp](<../algorithms/数据结构/有序表与平衡树/替罪羊树.cpp>) | [algorithms/数据结构/有序表与平衡树/对拍/oset_check.cpp](<../algorithms/数据结构/有序表与平衡树/对拍/oset_check.cpp>) |
| [cartesian - algorithms/数据结构/有序表与平衡树/笛卡尔树.cpp](<../algorithms/数据结构/有序表与平衡树/笛卡尔树.cpp>) | [algorithms/数据结构/有序表与平衡树/对拍/oset_check.cpp](<../algorithms/数据结构/有序表与平衡树/对拍/oset_check.cpp>) |
| [splay - algorithms/数据结构/有序表与平衡树/Splay.cpp](<../algorithms/数据结构/有序表与平衡树/Splay.cpp>) | [algorithms/数据结构/有序表与平衡树/对拍/oset_check.cpp](<../algorithms/数据结构/有序表与平衡树/对拍/oset_check.cpp>) |
| [graph - algorithms/图论/图的存储/Graph.cpp](<../algorithms/图论/图的存储/Graph.cpp>) | [algorithms/图论/对拍/completed_graph_stress_check.cpp](<../algorithms/图论/对拍/completed_graph_stress_check.cpp>)<br>[algorithms/图论/对拍/graph_check.cpp](<../algorithms/图论/对拍/graph_check.cpp>)<br>[algorithms/图论/对拍/graph_core_check.cpp](<../algorithms/图论/对拍/graph_core_check.cpp>) |
| [scc - algorithms/图论/连通性相关/Tarjan_SCC.cpp](<../algorithms/图论/连通性相关/Tarjan_SCC.cpp>) | [algorithms/图论/优化建图/对拍/seggraph_check.cpp](<../algorithms/图论/优化建图/对拍/seggraph_check.cpp>)<br>[algorithms/图论/对拍/completed_graph_stress_check.cpp](<../algorithms/图论/对拍/completed_graph_stress_check.cpp>)<br>[algorithms/图论/连通性相关/对拍/conn_check.cpp](<../algorithms/图论/连通性相关/对拍/conn_check.cpp>) |
| [ebcc - algorithms/图论/连通性相关/Tarjan_EBCC.cpp](<../algorithms/图论/连通性相关/Tarjan_EBCC.cpp>) | [algorithms/图论/对拍/completed_graph_stress_check.cpp](<../algorithms/图论/对拍/completed_graph_stress_check.cpp>)<br>[algorithms/图论/连通性相关/对拍/conn_check.cpp](<../algorithms/图论/连通性相关/对拍/conn_check.cpp>) |
| [vbcc - algorithms/图论/连通性相关/Tarjan_VBCC.cpp](<../algorithms/图论/连通性相关/Tarjan_VBCC.cpp>) | [algorithms/图论/对拍/completed_graph_stress_check.cpp](<../algorithms/图论/对拍/completed_graph_stress_check.cpp>)<br>[algorithms/图论/连通性相关/对拍/conn_bct_check.cpp](<../algorithms/图论/连通性相关/对拍/conn_bct_check.cpp>)<br>[algorithms/图论/连通性相关/对拍/conn_check.cpp](<../algorithms/图论/连通性相关/对拍/conn_check.cpp>) |
| [bct - algorithms/图论/连通性相关/圆方树/BCT.cpp](<../algorithms/图论/连通性相关/圆方树/BCT.cpp>) | [algorithms/图论/对拍/completed_graph_stress_check.cpp](<../algorithms/图论/对拍/completed_graph_stress_check.cpp>)<br>[algorithms/图论/连通性相关/对拍/conn_bct_check.cpp](<../algorithms/图论/连通性相关/对拍/conn_bct_check.cpp>)<br>[algorithms/图论/连通性相关/对拍/conn_check.cpp](<../algorithms/图论/连通性相关/对拍/conn_check.cpp>) |
| [lca - algorithms/图论/树上问题/最近公共祖先/DFN_LCA.cpp](<../algorithms/图论/树上问题/最近公共祖先/DFN_LCA.cpp>) | [algorithms/图论/对拍/graph_check.cpp](<../algorithms/图论/对拍/graph_check.cpp>)<br>[algorithms/图论/树上问题/虚树/对拍/lca_vt_stress_check.cpp](<../algorithms/图论/树上问题/虚树/对拍/lca_vt_stress_check.cpp>)<br>[algorithms/图论/树上问题/虚树/对拍/vt_check.cpp](<../algorithms/图论/树上问题/虚树/对拍/vt_check.cpp>) |
| [hldlca - algorithms/图论/树上问题/最近公共祖先/HLD_LCA.cpp](<../algorithms/图论/树上问题/最近公共祖先/HLD_LCA.cpp>) | [algorithms/图论/对拍/graph_check.cpp](<../algorithms/图论/对拍/graph_check.cpp>)<br>[algorithms/图论/树上问题/虚树/对拍/lca_vt_stress_check.cpp](<../algorithms/图论/树上问题/虚树/对拍/lca_vt_stress_check.cpp>) |
| [hld - algorithms/图论/树上问题/树链剖分/HLD.cpp](<../algorithms/图论/树上问题/树链剖分/HLD.cpp>) | [algorithms/图论/对拍/completed_graph_stress_check.cpp](<../algorithms/图论/对拍/completed_graph_stress_check.cpp>)<br>[algorithms/图论/树上问题/树链剖分/对拍/hld_check.cpp](<../algorithms/图论/树上问题/树链剖分/对拍/hld_check.cpp>) |
| [virtualTree - algorithms/图论/树上问题/虚树/二次排序.cpp](<../algorithms/图论/树上问题/虚树/二次排序.cpp>) | [algorithms/图论/树上问题/虚树/对拍/lca_vt_stress_check.cpp](<../algorithms/图论/树上问题/虚树/对拍/lca_vt_stress_check.cpp>)<br>[algorithms/图论/树上问题/虚树/对拍/vt_check.cpp](<../algorithms/图论/树上问题/虚树/对拍/vt_check.cpp>) |
| [virtualTreeStack - algorithms/图论/树上问题/虚树/单调栈.cpp](<../algorithms/图论/树上问题/虚树/单调栈.cpp>) | [algorithms/图论/树上问题/虚树/对拍/lca_vt_stress_check.cpp](<../algorithms/图论/树上问题/虚树/对拍/lca_vt_stress_check.cpp>)<br>[algorithms/图论/树上问题/虚树/对拍/vt_check.cpp](<../algorithms/图论/树上问题/虚树/对拍/vt_check.cpp>) |
| [diameterDfs - algorithms/图论/树上问题/树的直径/两次dfs.cpp](<../algorithms/图论/树上问题/树的直径/两次dfs.cpp>) | [algorithms/图论/对拍/completed_graph_stress_check.cpp](<../algorithms/图论/对拍/completed_graph_stress_check.cpp>)<br>[algorithms/图论/对拍/graph_check.cpp](<../algorithms/图论/对拍/graph_check.cpp>) |
| [diameterDp - algorithms/图论/树上问题/树的直径/树形dp法.cpp](<../algorithms/图论/树上问题/树的直径/树形dp法.cpp>) | [algorithms/图论/对拍/completed_graph_stress_check.cpp](<../algorithms/图论/对拍/completed_graph_stress_check.cpp>)<br>[algorithms/图论/对拍/graph_check.cpp](<../algorithms/图论/对拍/graph_check.cpp>) |
| [centroid - algorithms/图论/树上问题/树的重心/树的重心.cpp](<../algorithms/图论/树上问题/树的重心/树的重心.cpp>) | [algorithms/图论/对拍/completed_graph_stress_check.cpp](<../algorithms/图论/对拍/completed_graph_stress_check.cpp>)<br>[algorithms/图论/对拍/graph_check.cpp](<../algorithms/图论/对拍/graph_check.cpp>) |
| [center - algorithms/图论/树上问题/树的中心/树的中心.cpp](<../algorithms/图论/树上问题/树的中心/树的中心.cpp>) | [algorithms/图论/对拍/completed_graph_stress_check.cpp](<../algorithms/图论/对拍/completed_graph_stress_check.cpp>)<br>[algorithms/图论/树上问题/树的中心/对拍/center_check.cpp](<../algorithms/图论/树上问题/树的中心/对拍/center_check.cpp>) |
| [topo - algorithms/图论/拓扑排序/拓扑排序.cpp](<../algorithms/图论/拓扑排序/拓扑排序.cpp>) | [algorithms/图论/对拍/completed_graph_stress_check.cpp](<../algorithms/图论/对拍/completed_graph_stress_check.cpp>)<br>[algorithms/图论/对拍/graph_check.cpp](<../algorithms/图论/对拍/graph_check.cpp>) |
| [segGraph - algorithms/图论/优化建图/线段树优化建图.cpp](<../algorithms/图论/优化建图/线段树优化建图.cpp>) | [algorithms/图论/优化建图/对拍/seggraph_check.cpp](<../algorithms/图论/优化建图/对拍/seggraph_check.cpp>) |
| [dij - algorithms/图论/最短路问题/单源最短路径/dijkstra_heap.cpp](<../algorithms/图论/最短路问题/单源最短路径/dijkstra_heap.cpp>) | [algorithms/图论/优化建图/对拍/seggraph_check.cpp](<../algorithms/图论/优化建图/对拍/seggraph_check.cpp>)<br>[algorithms/图论/对拍/graph_check.cpp](<../algorithms/图论/对拍/graph_check.cpp>) |
| [dijN - algorithms/图论/最短路问题/单源最短路径/dijkstra.cpp](<../algorithms/图论/最短路问题/单源最短路径/dijkstra.cpp>) | [algorithms/图论/对拍/graph_check.cpp](<../algorithms/图论/对拍/graph_check.cpp>) |
| [spfa - algorithms/图论/最短路问题/单源最短路径/spfa 存图.cpp](<../algorithms/图论/最短路问题/单源最短路径/spfa%20存图.cpp>) | [algorithms/图论/对拍/graph_check.cpp](<../algorithms/图论/对拍/graph_check.cpp>) |
| [bf - algorithms/图论/最短路问题/单源最短路径/Bellman-Ford存图.cpp](<../algorithms/图论/最短路问题/单源最短路径/Bellman-Ford存图.cpp>) | [algorithms/图论/对拍/graph_check.cpp](<../algorithms/图论/对拍/graph_check.cpp>) |
| [bfRing - algorithms/图论/最短路问题/判断负环/bf_-ring.cpp](<../algorithms/图论/最短路问题/判断负环/bf_-ring.cpp>) | [algorithms/图论/对拍/graph_check.cpp](<../algorithms/图论/对拍/graph_check.cpp>) |
| [spfaRing - algorithms/图论/最短路问题/判断负环/spfa_-ring.cpp](<../algorithms/图论/最短路问题/判断负环/spfa_-ring.cpp>) | [algorithms/图论/对拍/graph_check.cpp](<../algorithms/图论/对拍/graph_check.cpp>) |
| [trie - algorithms/字符串/字典树/字典树.cpp](<../algorithms/字符串/字典树/字典树.cpp>) | [algorithms/字符串/字典树/对拍/trie_check.cpp](<../algorithms/字符串/字典树/对拍/trie_check.cpp>) |
| [persistentTrie - algorithms/字符串/字典树/可持久化字典树.cpp](<../algorithms/字符串/字典树/可持久化字典树.cpp>) | [algorithms/字符串/字典树/对拍/trie_capacity_check.cpp](<../algorithms/字符串/字典树/对拍/trie_capacity_check.cpp>)<br>[algorithms/字符串/字典树/对拍/trie_check.cpp](<../algorithms/字符串/字典树/对拍/trie_check.cpp>) |
| [mint - algorithms/数学/数论/mint.cpp](<../algorithms/数学/数论/mint.cpp>) | [algorithms/数学/数论/对拍/mint_check.cpp](<../algorithms/数学/数论/对拍/mint_check.cpp>) |
| [segSieve - algorithms/数学/数论/区间筛质数.cpp](<../algorithms/数学/数论/区间筛质数.cpp>) | [algorithms/数学/数论/对拍/number_boundary_check.cpp](<../algorithms/数学/数论/对拍/number_boundary_check.cpp>) |
| [intSqrt - algorithms/数学/数论/整数开方.cpp](<../algorithms/数学/数论/整数开方.cpp>) | [algorithms/数学/数论/对拍/number_boundary_check.cpp](<../algorithms/数学/数论/对拍/number_boundary_check.cpp>) |
| [divisorSums - algorithms/数学/数论/约数个数及约数和公式法.cpp](<../algorithms/数学/数论/约数个数及约数和公式法.cpp>) | [algorithms/数学/数论/对拍/number_boundary_check.cpp](<../algorithms/数学/数论/对拍/number_boundary_check.cpp>) |
| [bigint - algorithms/数学/高精度计算/高精度.cpp](<../algorithms/数学/高精度计算/高精度.cpp>) | [algorithms/数学/高精度计算/对拍/bigint_check.cpp](<../algorithms/数学/高精度计算/对拍/bigint_check.cpp>) |
| [rw - algorithms/杂项/快读快写/快读快写.cpp](<../algorithms/杂项/快读快写/快读快写.cpp>) | [algorithms/杂项/对拍/misc_check.cpp](<../algorithms/杂项/对拍/misc_check.cpp>) |
| [rnd - algorithms/杂项/随机数/z_rnd.cpp](<../algorithms/杂项/随机数/z_rnd.cpp>) | [algorithms/杂项/随机数/对拍/rnd_check.cpp](<../algorithms/杂项/随机数/对拍/rnd_check.cpp>) |
| [customHash - algorithms/杂项/防卡/哈希防卡.cpp](<../algorithms/杂项/防卡/哈希防卡.cpp>) | [algorithms/杂项/对拍/misc_check.cpp](<../algorithms/杂项/对拍/misc_check.cpp>) |
| [discrete - algorithms/杂项/离散化/离散化.cpp](<../algorithms/杂项/离散化/离散化.cpp>) | [algorithms/杂项/对拍/misc_check.cpp](<../algorithms/杂项/对拍/misc_check.cpp>) |

## B 类：未发现直接对拍引用的现役模板 [22]

| 模板或源文件 | 直接引用它的对拍文件 |
|---|---|
| [segSplit - algorithms/数据结构/线段树/线段树合并分裂/线段树按值域分裂.cpp](<../algorithms/数据结构/线段树/线段树合并分裂/线段树按值域分裂.cpp>) | 未发现直接引用 |
| [segSplitSort - algorithms/数据结构/线段树/线段树合并分裂/线段树分裂解决区间排序后单点查询问题.cpp](<../algorithms/数据结构/线段树/线段树合并分裂/线段树分裂解决区间排序后单点查询问题.cpp>) | 未发现直接引用 |
| [segMergeTree - algorithms/数据结构/线段树/线段树合并分裂/线段树合并解决树上查分词频问题.cpp](<../algorithms/数据结构/线段树/线段树合并分裂/线段树合并解决树上查分词频问题.cpp>) | 未发现直接引用 |
| [segFhq - algorithms/数据结构/树套树/线段树套FHQ_Treap.cpp](<../algorithms/数据结构/树套树/线段树套FHQ_Treap.cpp>) | 未发现直接引用 |
| [treeKnapDfs - algorithms/动态规划/背包DP/树上背包/dfn序.cpp](<../algorithms/动态规划/背包DP/树上背包/dfn序.cpp>) | 未发现直接引用 |
| [treeKnapBound - algorithms/动态规划/背包DP/树上背包/上下界.cpp](<../algorithms/动态规划/背包DP/树上背包/上下界.cpp>) | 未发现直接引用 |
| [binliftLca - algorithms/图论/树上问题/最近公共祖先/树上倍增LCA.cpp](<../algorithms/图论/树上问题/最近公共祖先/树上倍增LCA.cpp>) | 未发现直接引用 |
| [eulerLca - algorithms/图论/树上问题/最近公共祖先/欧拉序LCA.cpp](<../algorithms/图论/树上问题/最近公共祖先/欧拉序LCA.cpp>) | 未发现直接引用 |
| [tarjanLca - algorithms/图论/树上问题/最近公共祖先/tarjan.cpp](<../algorithms/图论/树上问题/最近公共祖先/tarjan.cpp>) | 未发现直接引用 |
| [eulerUndirected - algorithms/图论/欧拉图/无向图欧拉边路径.cpp](<../algorithms/图论/欧拉图/无向图欧拉边路径.cpp>) | 未发现直接引用 |
| [eulerDirected - algorithms/图论/欧拉图/有向图欧拉边路径.cpp](<../algorithms/图论/欧拉图/有向图欧拉边路径.cpp>) | 未发现直接引用 |
| [eulerPointUndirected - algorithms/图论/欧拉图/无向图欧拉点路径.cpp](<../algorithms/图论/欧拉图/无向图欧拉点路径.cpp>) | 未发现直接引用 |
| [eulerPointDirected - algorithms/图论/欧拉图/有向图欧拉点路径.cpp](<../algorithms/图论/欧拉图/有向图欧拉点路径.cpp>) | 未发现直接引用 |
| [floyd - algorithms/图论/最短路问题/全源最短路径/floyd.cpp](<../algorithms/图论/最短路问题/全源最短路径/floyd.cpp>) | 未发现直接引用 |
| [divCeilFloor - algorithms/数学/数论/除法上下取整.cpp](<../algorithms/数学/数论/除法上下取整.cpp>) | 未发现直接引用 |
| [lucas - algorithms/数学/组合数学/卢卡斯定理求组合数.cpp](<../algorithms/数学/组合数学/卢卡斯定理求组合数.cpp>) | 未发现直接引用 |
| [binomLoop - algorithms/数学/组合数学/循环求组合数.cpp](<../algorithms/数学/组合数学/循环求组合数.cpp>) | 未发现直接引用 |
| [binomPascal - algorithms/数学/组合数学/打表杨辉三角求组合数.cpp](<../algorithms/数学/组合数学/打表杨辉三角求组合数.cpp>) | 未发现直接引用 |
| [binomFactInv - algorithms/数学/组合数学/阶乘表及阶乘逆元表求组合数.cpp](<../algorithms/数学/组合数学/阶乘表及阶乘逆元表求组合数.cpp>) | 未发现直接引用 |
| [utils - algorithms/杂项/utils/utils.cpp](<../algorithms/杂项/utils/utils.cpp>) | 未发现直接引用 |
| [i128 - algorithms/杂项/128位整数/128int.cpp](<../algorithms/杂项/128位整数/128int.cpp>) | 未发现直接引用 |
| [misraGries - algorithms/杂项/主元素问题/Misra-Gries.cpp](<../algorithms/杂项/主元素问题/Misra-Gries.cpp>) | 未发现直接引用 |

## C 类：笔记条目（不参与编译） [17]

| 模板或源文件 | 直接引用它的对拍文件 |
|---|---|
| [knapsack - algorithms/动态规划/背包DP/背包问题.txt](<../algorithms/动态规划/背包DP/背包问题.txt>) | 不适用（笔记） |
| [lisLcs - algorithms/动态规划/线性DP/LIS与LCS.txt](<../algorithms/动态规划/线性DP/LIS与LCS.txt>) | 不适用（笔记） |
| [intervalDp - algorithms/动态规划/区间DP/区间DP.txt](<../algorithms/动态规划/区间DP/区间DP.txt>) | 不适用（笔记） |
| [digitDp - algorithms/动态规划/状压与数位DP/数位DP.txt](<../algorithms/动态规划/状压与数位DP/数位DP.txt>) | 不适用（笔记） |
| [bitmaskDp - algorithms/动态规划/状压与数位DP/状压DP.txt](<../algorithms/动态规划/状压与数位DP/状压DP.txt>) | 不适用（笔记） |
| [slopeOpt - algorithms/动态规划/优化技术/斜率优化.txt](<../algorithms/动态规划/优化技术/斜率优化.txt>) | 不适用（笔记） |
| [dinic - algorithms/图论/网络流/最大流/Dinic.txt](<../algorithms/图论/网络流/最大流/Dinic.txt>) | 不适用（笔记） |
| [mcmf - algorithms/图论/网络流/最小费用最大流/MCMF.txt](<../algorithms/图论/网络流/最小费用最大流/MCMF.txt>) | 不适用（笔记） |
| [acam - algorithms/字符串/AC自动机/AC自动机.txt](<../algorithms/字符串/AC自动机/AC自动机.txt>) | 不适用（笔记） |
| [sam - algorithms/字符串/后缀自动机/后缀自动机.txt](<../algorithms/字符串/后缀自动机/后缀自动机.txt>) | 不适用（笔记） |
| [suffixArray - algorithms/字符串/后缀数组/后缀数组.txt](<../algorithms/字符串/后缀数组/后缀数组.txt>) | 不适用（笔记） |
| [mobius - algorithms/数学/数论/莫比乌斯反演.txt](<../algorithms/数学/数论/莫比乌斯反演.txt>) | 不适用（笔记） |
| [matrixTree - algorithms/数学/组合数学/矩阵树定理.txt](<../algorithms/数学/组合数学/矩阵树定理.txt>) | 不适用（笔记） |
| [fft - algorithms/数学/多项式/FFT.txt](<../algorithms/数学/多项式/FFT.txt>) | 不适用（笔记） |
| [geoBasic - algorithms/计算几何/二维基础/二维基础.txt](<../algorithms/计算几何/二维基础/二维基础.txt>) | 不适用（笔记） |
| [convex - algorithms/计算几何/凸包/凸包.txt](<../algorithms/计算几何/凸包/凸包.txt>) | 不适用（笔记） |
| [hpi - algorithms/计算几何/半平面交/半平面交.txt](<../algorithms/计算几何/半平面交/半平面交.txt>) | 不适用（笔记） |

## 豁免代码：无目录跳板的 C++ 文件（不等同于笔记） [15]

| 模板或源文件 | 直接引用它的对拍文件 |
|---|---|
| [algorithms/图论/最短路问题/全源最短路径/P6125 无向图的最小环.cpp](<../algorithms/图论/最短路问题/全源最短路径/P6125%20无向图的最小环.cpp>) | 未发现直接引用 |
| [algorithms/图论/欧拉图/例题/Libre10105.cpp](<../algorithms/图论/欧拉图/例题/Libre10105.cpp>) | 未发现直接引用 |
| [algorithms/图论/欧拉图/例题/Luogu_P_1127.cpp](<../algorithms/图论/欧拉图/例题/Luogu_P_1127.cpp>) | 未发现直接引用 |
| [algorithms/图论/欧拉图/例题/Luogu_P_1333.cpp](<../algorithms/图论/欧拉图/例题/Luogu_P_1333.cpp>) | 未发现直接引用 |
| [algorithms/图论/欧拉图/例题/Luogu_P_1341.cpp](<../algorithms/图论/欧拉图/例题/Luogu_P_1341.cpp>) | 未发现直接引用 |
| [algorithms/图论/欧拉图/例题/Luogu_P_2731.cpp](<../algorithms/图论/欧拉图/例题/Luogu_P_2731.cpp>) | 未发现直接引用 |
| [algorithms/图论/欧拉图/例题/Luogu_P_3520.cpp](<../algorithms/图论/欧拉图/例题/Luogu_P_3520.cpp>) | 未发现直接引用 |
| [algorithms/图论/欧拉图/例题/Luogu_P_6066.cpp](<../algorithms/图论/欧拉图/例题/Luogu_P_6066.cpp>) | 未发现直接引用 |
| [algorithms/图论/欧拉图/例题/Luogu_P_7771.cpp](<../algorithms/图论/欧拉图/例题/Luogu_P_7771.cpp>) | 未发现直接引用 |
| [algorithms/数据结构/堆/左偏树/k短路未维护.cpp](<../algorithms/数据结构/堆/左偏树/k短路未维护.cpp>) | 未发现直接引用 |
| [algorithms/数据结构/线段树/泛型插件/P2572 序列操作.cpp](<../algorithms/数据结构/线段树/泛型插件/P2572%20序列操作.cpp>) | 未发现直接引用 |
| [algorithms/数据结构/线段树/泛型插件/主席树/主席树单点修改范围求和.cpp](<../algorithms/数据结构/线段树/泛型插件/主席树/主席树单点修改范围求和.cpp>) | 未发现直接引用 |
| [algorithms/数据结构/线段树/泛型插件/区间加区间和.cpp](<../algorithms/数据结构/线段树/泛型插件/区间加区间和.cpp>) | 未发现直接引用 |
| [algorithms/数据结构/线段树/泛型插件/区间历史最值.cpp](<../algorithms/数据结构/线段树/泛型插件/区间历史最值.cpp>) | 未发现直接引用 |
| [algorithms/数据结构/线段树/泛型插件/区间取模区间求和.cpp](<../algorithms/数据结构/线段树/泛型插件/区间取模区间求和.cpp>) | 未发现直接引用 |

## 已发现的对拍套件 [23]

- [algorithms/图论/优化建图/对拍/seggraph_check.cpp](<../algorithms/图论/优化建图/对拍/seggraph_check.cpp>)
- [algorithms/图论/对拍/completed_graph_stress_check.cpp](<../algorithms/图论/对拍/completed_graph_stress_check.cpp>)
- [algorithms/图论/对拍/graph_check.cpp](<../algorithms/图论/对拍/graph_check.cpp>)
- [algorithms/图论/对拍/graph_core_check.cpp](<../algorithms/图论/对拍/graph_core_check.cpp>)
- [algorithms/图论/树上问题/树的中心/对拍/center_check.cpp](<../algorithms/图论/树上问题/树的中心/对拍/center_check.cpp>)
- [algorithms/图论/树上问题/树链剖分/对拍/hld_check.cpp](<../algorithms/图论/树上问题/树链剖分/对拍/hld_check.cpp>)
- [algorithms/图论/树上问题/虚树/对拍/lca_vt_stress_check.cpp](<../algorithms/图论/树上问题/虚树/对拍/lca_vt_stress_check.cpp>)
- [algorithms/图论/树上问题/虚树/对拍/vt_check.cpp](<../algorithms/图论/树上问题/虚树/对拍/vt_check.cpp>)
- [algorithms/图论/连通性相关/对拍/conn_bct_check.cpp](<../algorithms/图论/连通性相关/对拍/conn_bct_check.cpp>)
- [algorithms/图论/连通性相关/对拍/conn_check.cpp](<../algorithms/图论/连通性相关/对拍/conn_check.cpp>)
- [algorithms/字符串/字典树/对拍/trie_capacity_check.cpp](<../algorithms/字符串/字典树/对拍/trie_capacity_check.cpp>)
- [algorithms/字符串/字典树/对拍/trie_check.cpp](<../algorithms/字符串/字典树/对拍/trie_check.cpp>)
- [algorithms/数学/数论/对拍/mint_check.cpp](<../algorithms/数学/数论/对拍/mint_check.cpp>)
- [algorithms/数学/数论/对拍/number_boundary_check.cpp](<../algorithms/数学/数论/对拍/number_boundary_check.cpp>)
- [algorithms/数学/高精度计算/对拍/bigint_check.cpp](<../algorithms/数学/高精度计算/对拍/bigint_check.cpp>)
- [algorithms/数据结构/堆/左偏树/对拍/leftist_check.cpp](<../algorithms/数据结构/堆/左偏树/对拍/leftist_check.cpp>)
- [algorithms/数据结构/并查集/对拍/dsu_check.cpp](<../algorithms/数据结构/并查集/对拍/dsu_check.cpp>)
- [algorithms/数据结构/有序表与平衡树/对拍/oset_check.cpp](<../algorithms/数据结构/有序表与平衡树/对拍/oset_check.cpp>)
- [algorithms/数据结构/树状数组/对拍/bit_check.cpp](<../algorithms/数据结构/树状数组/对拍/bit_check.cpp>)
- [algorithms/数据结构/线段树/对拍/pst_check.cpp](<../algorithms/数据结构/线段树/对拍/pst_check.cpp>)
- [algorithms/数据结构/线段树/对拍/seg_check.cpp](<../algorithms/数据结构/线段树/对拍/seg_check.cpp>)
- [algorithms/杂项/对拍/misc_check.cpp](<../algorithms/杂项/对拍/misc_check.cpp>)
- [algorithms/杂项/随机数/对拍/rnd_check.cpp](<../algorithms/杂项/随机数/对拍/rnd_check.cpp>)
