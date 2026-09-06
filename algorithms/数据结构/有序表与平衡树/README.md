# 有序表与平衡树

八份源码尾部都提供可运行 Usage。按数据语义选择, 不把集合、序列和建树工具混用。

| 模板 | 语义与复杂度 | 结点预算 |
|---|---|---|
| SkipList | 去重集合, 层数匹配规模时期望 O(log n) | 峰值存活, 删除回收, K=20 每结点 176 B |
| AVL | 重复集合, 最坏 O(log n), 同值用 cnt | 累计新建, 每结点 32 B |
| Treap | 重复集合, 期望 O(log n), 同值用 cnt | 累计新建, 每结点 32 B |
| FHQ_Treap | 重复集合, 期望 O(log n), 公开值域 split/merge | 累计插入, 每结点 24 B |
| SGTree | 重复集合, 插删均摊 O(log n), 原位重建 | 峰值存活, 每结点 24 B |
| Splay | 重复集合, 访问伸展, 均摊 O(log n) | 累计插入, 每结点 24 B |
| FHQ_Seq | 按位置维护序列, 普通区间操作期望 O(log n), 批插/删除另加元素数 | 峰值存活, 每结点 96 B |
| Cartesian | O(n) 双堆型建树; build_bst 排序 O(n log n) | 图与映射按 n 分配 |

集合的 get_rank(v) 返回严格小于 v 的元素数量, 不是从 1 开始的名次。
get_kth 从 1 开始, 越界返回 INF; 前驱/后继都是严格关系, 无解返回 -INF/INF。
键应在 (-INF, INF) 内。五种树 erase 返回是否删掉一个元素;
跳表 insert 返回既有或新 id, erase 返回被删 id, 无此值为 -1, 删除后 id 可能被复用。

clear 保留容量并恢复哨兵。AVL/Treap 删除不回收, 重复键只增 cnt, 预算按累计新建结点保守估算;
FHQ/Splay 每次插入各开一个结点。SGTree/SkipList/FHQ_Seq 的预算按峰值存活,
回收编号数组另占 O(预算) 空间。Treap/FHQ/Splay 的 build 输入必须升序, 且 a[0] 不参与。

FHQ_Seq build 接 a[1..m], 批量 insert 接整个 vector; insert(pos) 是插入后的位置,
move_interval(l,r,pos) 是切出后放到剩余序列前 pos 个元素之后。
sum/min/max 支持全部操作; 最大子段和按非空段定义, 全负返回最大负值,
仅在建树/插删/覆盖/翻转/移动域保证有效, 加乘后不查受影响数据, 全覆盖或重建可恢复。
LLONG_MIN 哨兵不参与求和, 但所有中间和、乘法和标记叠加仍须不溢出。

Cartesian 的 build 用数组下标控制中序、值控制堆序, 同值保留原次序;
build_bst 用排序次序作新编号, key/orig 返回原键与插入编号, EqLeft 选择重复键往左还是往右插。
树不保证平衡, 单调数组可能生成长链; 下游遍历另考虑栈深。总点数取 ct.n,
Graph.node_cnt 不统计没有边的孤点, key/orig 仅最近调用 build_bst 后有效。

原 oset_check 的小暴力已增强到 20 万目标规模并自动进入 CI, 记录见
[P06 验证](../../../records/verification/oset-20260906.md)。固定的是操作种子 42,
随机树内部优先级仍沿用 z_rnd 的时间种子, 不承诺每次内部树形相同。
