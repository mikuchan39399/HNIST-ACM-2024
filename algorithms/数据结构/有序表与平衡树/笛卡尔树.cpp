// zoi: cartesian
#ifndef Z_OI_CARTESIAN
#define Z_OI_CARTESIAN

#include "../../图论/图的存储/Graph.cpp"
#include "../../杂项/utils/utils.cpp"

// 笛卡尔树, 等权时先出现的点优先; 区间最值=两端点 LCA 的权值
// tree 存无向边, 总点数取 n(不能取 node_cnt); 空树根为 0. 空间 O(n)
struct Cartesian
{
    int n, rt;
    Graph<false> tree;
    VLL key;   // build_bst 后, 结点 r 的原键
    VI orig;   // build_bst 后, 结点 r 的原下标
    // 预留 max_n 个点, build 可扩容. 时空 O(max_n)
    Cartesian(int max_n = 0) : n(0), rt(0), tree(max_n, max_n)
    {}
    // 用 a[1..n] 建树并返回根, true 小根 / false 大根. 时间 O(n), 栈 O(n)
    // 孩子编号小于父亲为左子, 大于父亲为右子
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
    // 构建按 a[1..n] 顺序插入的 BST, 结点按键排序编号. 时间 O(n log n), 空间 O(n)
    // EqLeft=false 时等值向右插, true 时向左插; key/orig 仅本接口有效
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
