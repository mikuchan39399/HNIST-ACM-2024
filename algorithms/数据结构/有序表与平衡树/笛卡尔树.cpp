// zoi: cartesian
#ifndef Z_OI_CARTESIAN
#define Z_OI_CARTESIAN

#include "../../图论/图的存储/Graph.cpp"
#include "../../杂项/utils/utils.cpp"

// 笛卡尔树线性建树, 下标控制中序次序, a[i] 决定堆序, 右脊栈不下降建小根堆, 不上升建大根堆
// 等权点保留原顺序; 小根堆中区间最小值等于两端点 LCA 的权值, 大根堆对偶
// tree 为无向图, 总点数用 n, 单点时 Graph::node_cnt() 为 0; key/orig 仅在最近调用 build_bst 后有效
// 图边每半边 8 B, 点表约 12n B, 临时栈 4n B; build_bst 的映射和排序数组另计 O(n)
struct Cartesian
{
    int n, rt;
    Graph<false> tree;
    VLL key;   // build_bst 后: 节点 r 的键(第 r 小的值)
    VI orig;   // build_bst 后: 节点 r 的原数组下标(插入时间)
    // 预分配 max_n 点的图容器, build 可自动扩容
    // 时间: O(max_n) | 空间: O(max_n)
    Cartesian(int max_n = 0) : n(0), rt(0), tree(max_n, max_n)
    {}
    // 用 a[1..n] 建笛卡尔树, min_heap 为 true 建小根堆, 返回根, n=0 返回 0
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
    // 时间: O(n log n) | 空间: O(n)
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

/* Usage:
int main()
{
    Cartesian ct;
    VLL a = {0, 3, 1, 2};
    cout << ct.build(a) << "\n"; // 2, 下标为 2 的点是小根堆根
    cout << ct.build(a, false) << "\n"; // 1, 大根堆根
    int rt = ct.build_bst(a);
    cout << ct.orig[rt] << " " << ct.key[rt] << "\n"; // 1 3, 插入时间与原键
    ct.build_bst<true>(VLL{0, 2, 2}); // 等值向左插, 结点编号按键的排序次序
    // tree 可接树算法, 总点数显式传 ct.n, 单点图没有边
}
*/
