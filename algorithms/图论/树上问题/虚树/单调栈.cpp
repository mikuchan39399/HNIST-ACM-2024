// zoi: virtualTreeStack
#ifndef Z_OI_VT_STACK
#define Z_OI_VT_STACK

#include "../../图的存储/Graph.cpp"
#include "../../../杂项/utils/utils.cpp"

// 无向虚树, 保留原点号, 边权为路径权和(须在 LL 内); LCA 须有 dfn/rt/lca/dist
// 输入点已建表且编号不超容量; root 为额外必选点, 不改变原树根
// 空输入不保留 root; 单点无边, 需自行记录. 空间 O(max_n)
struct VirtualTreeStack
{
    Graph<false, LL> tree;
    VI stk;
    // 按原树最大点号预留. 时空 O(max_n)
    VirtualTreeStack(int max_n = 0) : tree(max_n, max_n)
    {
        stk.reserve(max_n + 10);
    }
    // 清空并保留容量. 时间 O(s), s 为旧虚树大小
    void clear()
    {
        tree.clear();
        stk.clear();
    }
    // 用整个 nodes 重建, 允许重复且不改输入; 空集/与 root 跨树时清空
    // 时间 O(s+k log(k+1)+kT), 额外空间 O(k); k=输入长度, T=LCA/距离查询时间
    template <class LCA>
    void build(const VI& nodes, LCA& lca, int root = 1)
    {
        clear();
        if (nodes.empty()) return;
        for (int x : nodes)
            if (lca.rt[x] != lca.rt[root]) return;
        VI ns = nodes;
        ns.push_back(root);
        sort(ns.begin(), ns.end(), [&](int a, int b){
            return lca.dfn[a] < lca.dfn[b];
        });
        ns.erase(unique(ns.begin(), ns.end()), ns.end());
        stk.push_back(ns[0]);
        for (int u : ns)
        {
            if (u == ns[0]) continue;
            int p = lca.lca(u, stk.back());
            if (p != stk.back())
            {
                while (stk.size() > 1 && lca.dfn[stk[stk.size() - 2]] >= lca.dfn[p])
                {
                    tree.add(stk[stk.size() - 2], stk.back(), lca.dist(stk[stk.size() - 2], stk.back()));
                    stk.pop_back();
                }
                if (stk.back() != p)
                {
                    tree.add(p, stk.back(), lca.dist(p, stk.back()));
                    stk.back() = p;
                }
            }
            stk.push_back(u);
        }
        for (size_t i = 0; i + 1 < stk.size(); i++)
        {
            tree.add(stk[i], stk[i + 1], lca.dist(stk[i], stk[i + 1]));
        }
    }
};
#endif

/* Usage
// 先 include "lca.h", 用 DFN_LCA 为原树建表
Graph<false, LL> g(4, 3);
g.add(1, 2, 3);
g.add(2, 3, 5);
g.add(2, 4, 7);
LCA lca(4);
lca.build(g, 4);
VirtualTreeStack vt(4);
VI keys{4, 3, 4};
vt.build(keys, lca, 1); // keys 不变, 虚树边为 1-2, 2-3, 2-4
cout << vt.tree.edge_cnt() << endl; // 3
for (auto& e : vt.tree[2])
    cout << e.v << ' ' << e.w << endl;
vt.build(VI{3}, lca, 4); // root 可不是祖先, 保留 3, 4 及其 LCA 2
vt.build(VI{}, lca); // 清空, 连 root 也不保留
vt.build(VI{1}, lca); // 单点 1 无边, 不靠 tree.used 枚举它
vt.clear(); // 同一原树反复 build 会自动 clear

*/

