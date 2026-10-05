// zoi: vbcc
#ifndef Z_OI_VBCC
#define Z_OI_VBCC

#include "../图的存储/Graph.cpp"
#include "../../杂项/utils/utils.cpp"

// 点双; cut[u] 判割点, vbcc_cir[i] 存成员, 孤立点自成块
// 原图允许重边, 须过滤自环; 递归深度最坏 n
struct VBCC
{
    int n;
    int dfn_idx, vbcc_cnt;
    Graph<false, Empty> tree;
    VI dfn, low, sta, cut;
    VVI vbcc_cir;
    // 预留 max_n 个原图点及至多 2*max_n 点的圆方森林. 时空 O(max_n)
    VBCC(int max_n = 0) : n(max_n), dfn_idx(0), vbcc_cnt(0),
        tree(max_n * 2, max_n * 2),
        dfn(max_n + 10, 0), low(max_n + 10, 0), cut(max_n + 10, 0),
        vbcc_cir(1, VI{})
    {
        sta.reserve(max_n + 10);
    }
    // 清空结果, n<=容量; 原图另行 clear. 时间 O(n+旧结果大小)
    void init(int _n)
    {
        n = _n;
        tree.clear();
        z_fill_n(n, 0, dfn, low, cut);
        dfn_idx = vbcc_cnt = 0;
        sta.clear();
        vbcc_cir.assign(1, VI{});
    }
    // 求点双和割点, 自动复位; root=-1 扫全图, 否则仅扫所在连通块
    // 时间 O(n+m+旧结果大小), 额外空间 O(n)
    template <class G>
    void build(G& g, int _n, int root = -1)
    {
        init(_n);
        if (root != -1)
        {
            tarjan(g, root, root);
            return;
        }
        for (int i = 1; i <= n; i++)
        {
            if (!dfn[i])
            {
                tarjan(g, i, i);
            }
        }
    }
    // build 后重建圆方森林 tree: 圆点 1..n, 方点 n+i, 总点数 n+vbcc_cnt. 时空 O(n)
    void build_tree()
    {
        tree.clear();
        for (int i = 1; i <= vbcc_cnt; i++)
        {
            int u = n + i;
            for (int v : vbcc_cir[i])
                tree.add(u, v);
        }
    }
    // 返回 u 所属点双编号, 须先 build_tree. 时空 O(返回项数)
    VI get_bel_vbccs(int u)
    {
        VI res;
        for (auto& e : tree[u])
            res.push_back(e.v - n);
        return res;
    }
    // 返回第 i 块中的割点, i 越界返回空; 无需 build_tree. 时间 O(块大小), 空间 O(返回项数)
    VI get_cuts_vbcc(int i)
    {
        VI res;
        if (i < 1 || i > vbcc_cnt) return res;
        for (int v : vbcc_cir[i])
        {
            if (cut[v]) res.push_back(v);
        }
        return res;
    }
private:
    template <class G>
    void tarjan(G& g, int u, int root)
    {
        dfn_idx++;
        dfn[u] = low[u] = dfn_idx;
        sta.push_back(u);
        int child_cnt = 0;
        for (auto& e : g[u])
        {
            int v = e.v;
            if (!dfn[v])
            {
                child_cnt++;
                tarjan(g, v, root);
                low[u] = min(low[u], low[v]);
                if (low[v] >= dfn[u])
                {
                    if (u != root) cut[u] = 1;
                    vbcc_cnt++;
                    vbcc_cir.push_back(VI{});
                    int t;
                    do
                    {
                        t = sta.back();
                        sta.pop_back();
                        vbcc_cir[vbcc_cnt].push_back(t);
                    } while (t != v);
                    vbcc_cir[vbcc_cnt].push_back(u);
                }
            }
            else low[u] = min(low[u], dfn[v]);
        }
        if (u == root && child_cnt >= 2) cut[u] = 1;

        if (u == root && child_cnt == 0)
        {
            vbcc_cnt++;
            vbcc_cir.push_back(VI{u});
        }
    }
};

#endif
// Usage:
/*
int main()
{
    int n, m;
    cin >> n >> m;
    VBCC vbcc(n);
    Graph<false> g(n, m);

    for (int i = 1; i <= m; i++)
    {
        int u, v;
        cin >> u >> v;
        if (u != v) g.add(u, v); // 过滤自环, 保留重边
    }
    vbcc.build(g, n);
    for (int i = 1; i <= vbcc.vbcc_cnt; i++)
    {
        for (int u : vbcc.vbcc_cir[i]) cout << u << ' '; // 第 i 个点双的原图点
        cout << '\n';
        for (int u : vbcc.get_cuts_vbcc(i)) cout << u << ' '; // 其中的割点
        cout << '\n';
    }
    vbcc.build_tree();
    for (int i : vbcc.get_bel_vbccs(1)) cout << i << ' '; // 点 1 所属的点双编号
    cout << '\n';
    return 0;
}
*/
