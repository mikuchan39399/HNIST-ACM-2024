// zoi: cartesian
#ifndef Z_OI_CARTESIAN
#define Z_OI_CARTESIAN

#include <vector>
#include "../../图论/图的存储/Graph.cpp"
#include "../../杂项/utils/utils.cpp"

using namespace std;

// ============ 笛卡尔树 ============
// 笛卡尔建树为线性建树, 一个节点有两个属性, value 与 idx: 
// value 用于控制建成树中序遍历, idx 在建成树上呈堆; 单调栈内部单调方向决定什么堆,
// 单调不下降 idx 建成小根堆, 单调不上升 idx 建成大根堆
// value 单调递增则能建成 BST
// Treap/FHQ 的 build 按原句用: 排序的 value 定 BST, 随机权 rd 当 idx 保证平衡。
// 性质: 中序还原数组顺序; 子树的根是子树那段的最小值, 所以
//       min(a[l..r]) = l 和 r 在树上的 LCA 的值。
struct Cartesian
{
    int n, rt;
    Graph<false> tree;
    VLL key;   // build_bst 后: 节点 r 的键(第 r 小的值)
    VI orig;   // build_bst 后: 节点 r 的原数组下标(插入时间)
    Cartesian(int max_n = 0) : n(0), rt(0), tree(max_n, max_n)
    {}
    // 用 a[1..n] 建一颗序列 BST, min_heap 选堆型缺省小根, 返回根(空数组返回 0)
    // 树存 tree(无向边), 孩子方向看编号: 孩子 < 父 = 左子
    // 时间: O(n) | 空间: 右脊栈 O(n)
    int build(const VLL& a, bool min_heap = true)
    {
        n = (int)a.size() - 1;
        if ((int)tree.head.size() < n + 10) tree = Graph<false>(n, n);
        else tree.clear();
        VI stk;
        stk.reserve(n + 1);
        rt = 0;
        for (int i = 1; i <= n; i++)
        {
            int last = 0;
            while (!stk.empty() && (min_heap ? a[stk.back()] > a[i] : a[stk.back()] < a[i]))
            {
                int x = stk.back();
                stk.pop_back();
                if (last) tree.add(x, last);
                last = x;
            }
            if (last) tree.add(i, last);
            stk.push_back(i);
        }
        rt = stk.empty() ? 0 : stk[0];
        for (size_t k = 1; k < stk.size(); k++) tree.add(stk[k - 1], stk[k]);
        return rt;
    }
    // a[1..n] 顺序插入空 BST 的拓扑: 值当中序, 插入时间当小根堆
    // EqLeft 路由等值去向: false 往右插(先插者居左), true 往左插;
    // 时间: O(n log n) (值为排列可桶排 O(n)) | 空间: O(n)
    template <bool EqLeft = false>
    int build_bst(const VLL& a)
    {
        n = (int)a.size() - 1;
        VI ord(n);
        iota(ord.begin(), ord.end(), 1);
        sort(ord.begin(), ord.end(), [&](int x, int y)
        {
            if (a[x] != a[y]) return a[x] < a[y];
            if constexpr (EqLeft) return x > y;
            else return x < y;
        });
        VLL b(n + 1);
        key.assign(n + 1, 0);
        orig.assign(n + 1, 0);
        for (int r = 1; r <= n; r++)
        {
            b[r] = ord[r - 1];
            key[r] = a[ord[r - 1]];
            orig[r] = ord[r - 1];
        }
        return build(b);
    }
};
#endif
/*
 * Usage:
 * Cartesian ct(n);                // 预算 n 个点, 不够自动扩
 * int root = ct.build(a);         // 建树返回根, 缺省小根堆; build(a, false) 大根堆
 * ct.tree / ct.rt;                // 普通树视图, 直接喂 DFN_LCA
 * // 孩子方向: 遍历 ct.tree[u], 编号比 u 小的是左子
 * // 小根堆: min(a[l..r]) = l 和 r 的 LCA 的值; 大根堆对偶查区间最大
 * // Treap 的 O(n) 建树: a 换成随机权数组, 同一个 build
 * int r = ct.build_bst(a);         // a 顺序插入空 BST 的拓扑(等值往右)
 * ct.build_bst<true>(a);           // 等值往左版; key[r]/orig[r] 回读键与原下标
 */