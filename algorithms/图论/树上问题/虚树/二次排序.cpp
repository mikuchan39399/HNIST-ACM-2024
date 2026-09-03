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
    // 用关键点集建虚树: 两两 LCA 的祖先链并入点集, tree 得压缩子树
    // (至多 2k-1 点); 关键点跨树时静默得到空树; 不修改调用方容器
    // 时间: O(k log k) | 空间: 虚树至多 2k-1 点
    void build(const VI& nodes, LCA& lca, int root = 1)
    {
        clear();
        VI ns = nodes;              // 内部拷贝, 调用方列表不被消耗
        if (ns.empty()) return;
        for (int x : ns)
            if (lca.rt[x] != lca.rt[root]) return;
        ns.push_back(root);
        sort(ns.begin(), ns.end(), [&](int a, int b){
            return lca.dfn[a] < lca.dfn[b];
        });
        int sz = ns.size();
        for (int i = 1; i < sz; i++)
        {
            int p = lca.lca(ns[i - 1], ns[i]);
            ns.push_back(p);
        }
        sort(ns.begin(), ns.end(), [&](int a, int b){
            return lca.dfn[a] < lca.dfn[b];
        });
        ns.erase(unique(ns.begin(), ns.end()), ns.end());
        for (int i = 1; i < (int)ns.size(); i++)
        {
            int p = lca.lca(ns[i - 1], ns[i]);
            tree.add(p, ns[i], lca.dist(p, ns[i]));
        }
    }
};
#endif
