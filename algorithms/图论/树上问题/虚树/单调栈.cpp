// zoi: virtualTreeStack
#ifndef Z_OI_VT_STACK
#define Z_OI_VT_STACK

#include "../../图的存储/Graph.cpp"
#include "../../../杂项/utils/utils.cpp"

// tree 存原点编号的无向虚树, stk 存尚未连边的祖先链; LCA 须提供 dfn/rt/lca/dist
// 输入点与 root 须已在 LCA 中建表, 编号不超过构造容量, 距离运算须在 LL 内
// root 是额外必选点, 不改变 LCA 的根; 边权为原树路径权和, 关键点允许重复
// K 为关键点与 root 去重后的点数, 虚树至多 2K - 1 点且不超过原树点数
// 空输入不保留 root; 仅剩一个点时无边, Graph 不记录孤立点, 单点由调用方保存
// LL 半边为 16 B, 预留约 2 * max_n 条半边, 点表及栈约 16 * max_n B
// 合计约 48 * max_n B, max_n = 2e5 时约 9.6 MB, 临时点集和外置 LCA 另计
struct VirtualTreeStack
{
    Graph<false, LL> tree;
    VI stk;
    // 按原树最大点编号分配图和栈, 首次可直接 build
    // 时间 O(max_n) | 空间 O(max_n)
    VirtualTreeStack(int max_n = 0) : tree(max_n, max_n)
    {
        stk.reserve(max_n + 10);
    }
    // 清空上次虚树和栈并保留容量
    // 时间 O(s) | 额外空间 O(1), s 为上次虚树点数
    void clear()
    {
        tree.clear();
        stk.clear();
    }
    // 用整个 0-based 点集 nodes 重建 tree, 不修改输入; 空集或与 root 跨树时清空
    // 时间 O(s + k log(k + 1) + kT) | 额外空间 O(k), k 为输入长度, T 为一次 LCA/距离查询时间
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
lca.build(g);
VirtualTreeStack vt(4);
VI keys{4, 3, 4};
vt.build(keys, lca, 1);       // keys 不变, 虚树边为 1-2, 2-3, 2-4
cout << vt.tree.edge_cnt() << endl; // 3
for (auto& e : vt.tree[2])
    cout << e.v << ' ' << e.w << endl;
vt.build(VI{3}, lca, 4);      // root 可不是祖先, 保留 3, 4 及其 LCA 2
vt.build(VI{}, lca);          // 清空, 连 root 也不保留
vt.build(VI{1}, lca);         // 单点 1 无边, 不靠 tree.used 枚举它
vt.clear();                  // 同一原树反复 build 会自动 clear
// 换原树时复用 g.clear(), lca.init(n), 重建 LCA 后再 build 虚树
*/

