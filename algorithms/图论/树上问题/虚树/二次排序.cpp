// zoi: virtualTree
#ifndef Z_OI_VT_SORT
#define Z_OI_VT_SORT

#include <vector>
#include <algorithm>
#include "../../图的存储/Graph.cpp"
#include "../../../杂项/utils/utils.cpp"

using namespace std;

struct VirtualTree
{
    Graph<false, LL> tree;
    VirtualTree(int max_n = 0) : tree(max_n, max_n * 2) {}
    void clear() { tree.clear(); }
    template<typename LCA>
    void build(VI& nodes, LCA& lca, int root = 1)
    {
        clear();
        if (nodes.empty()) return;
        for (int x : nodes)
            if (lca.rt[x] != lca.rt[root]) return;
        nodes.push_back(root);
        sort(nodes.begin(), nodes.end(), [&](int a, int b){
            return lca.dfn[a] < lca.dfn[b];
        });
        int sz = nodes.size();
        for (int i = 1; i < sz; i++)
        {
            int p = lca.lca(nodes[i - 1], nodes[i]);
            nodes.push_back(p);
        }
        sort(nodes.begin(), nodes.end(), [&](int a, int b){
            return lca.dfn[a] < lca.dfn[b];
        });
        nodes.erase(unique(nodes.begin(), nodes.end()), nodes.end());
        for (int i = 1; i < (int)nodes.size(); i++)
        {
            int p = lca.lca(nodes[i - 1], nodes[i]);
            tree.add(p, nodes[i], lca.dist(p, nodes[i]));
        }
    }
};
#endif
