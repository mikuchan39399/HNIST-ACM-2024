#ifndef Z_OI_VT_STACK
#define Z_OI_VT_STACK

#include <vector>
#include "../../图的存储/Graph.cpp"
#include "../../../杂项/utils/utils.cpp"

using namespace std;

struct VirtualTreeStack
{
    Graph<false, LL> tree;
    VI stk;
    VirtualTreeStack(int max_n = 0) : tree(max_n, max_n)
    {
        stk.reserve(max_n + 10);
    }
    void clear()
    {
        tree.clear();
        stk.clear();
    }
    // 传入关键点 VI 与 对原图建好的 DFN_LCA
    template<typename LCA>
    void build(VI& nodes, LCA& lca, int root = 1)
    {
        clear();
        if (nodes.empty()) return;
        sort(nodes.begin(), nodes.end(), [&](int a, int b){
            return lca.dfn[a] < lca.dfn[b];
        });
        stk.push_back(root);
        for (int u : nodes)
        {
            if (u == root) continue;
            int p = lca.lca(u, stk.back());
            if(p != stk.back())
            {
                while (stk.size() > 1 && lca.dfn[stk[stk.size() - 2]] >= lca.dfn[p]) // 次栈顶节点 >= 此次 lca
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

