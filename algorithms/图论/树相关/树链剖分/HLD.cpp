#include <vector>
#include <algorithm>
#include <iostream>

using namespace std;
using VI = vector<int>;
using LL = long long;

template<typename... CS>
void z_fill_n(int n, int val, CS&... cs) 
{
    (fill(cs.begin(), cs.begin() + min((size_t)(n + 10), cs.size()), val), ...);
}
#define lc (p << 1)
#define rc (p << 1 | 1)
struct SegTree
{
    struct Node
    {
        int l, r;
        LL add, sum;
    };
    vector<Node> tr;
    SegTree(int max_n = 0) : 
        tr((max_n + 10) << 2, {0, 0, 0, 0})
    {}
    void push_up(int p)
    {
        tr[p].sum = tr[lc].sum + tr[rc].sum;
    }
    // a 为原数组, seg 为 dfn 到 原节点编号的映射
    void build(int p, int l, int r, const VI& a, const VI& seg)
    {
        tr[p] = {l, r, 0, 0};
        if(l == r)
        {
            tr[p].sum = a[seg[l]]; // 叶子节点的值来自映射后的原数组
            return;
        }
        int mid = (l + r) >> 1;
        build(lc, l, mid, a, seg);
        build(rc, mid + 1, r, a, seg);
        push_up(p);
    }
    void lazy(int p, LL add)
    {
        int l = tr[p].l, r = tr[p].r;
        tr[p].add += add;
        tr[p].sum += add * (r - l + 1);
    }
    void push_down(int p)
    {
        if (tr[p].add == 0 || tr[p].l == tr[p].r) 
        {
            return;
        }
        lazy(lc, tr[p].add);
        lazy(rc, tr[p].add);
        tr[p].add = 0;
    }
    void modify(int p, int x, int y, LL k)
    {
        int l = tr[p].l, r = tr[p].r;
        if(l >= x && r <= y)
        {
            lazy(p, k);
            return;
        }
        push_down(p);
        int mid = (l + r) >> 1;
        if(x <= mid) modify(lc, x, y, k);
        if(y > mid) modify(rc, x, y, k);
        push_up(p);
    }
    LL query(int p, int x, int y)
    {
        int l = tr[p].l, r = tr[p].r;
        if(l >= x && r <= y)
        {
            return tr[p].sum;
        }
        push_down(p);
        LL sum = 0;
        int mid = (l + r) >> 1;
        if(x <= mid) sum += query(lc, x, y);
        if(y > mid) sum += query(rc, x, y);
        return sum;
    }
};

struct HLD 
{
    int n, dfn_idx;
    VI fa, dep, sz, son, top, dfn, seg;
    HLD(int max_n = 0) : n(max_n), dfn_idx(0), 
        fa(max_n + 10, 0), dep(max_n + 10, 0), sz(max_n + 10, 0), 
        son(max_n + 10, 0), top(max_n + 10, 0), 
        dfn(max_n + 10, 0), seg(max_n + 10, 0) 
    {}
    void init(int _n) 
    {
        n = _n;
        dfn_idx = 0;
        z_fill_n(n, 0, fa, dep, sz, son, top, dfn, seg);
    }
    template <class G>
    void dfs1(int u, int f, G& g) 
    {
        fa[u] = f;
        dep[u] = dep[f] + 1;
        sz[u] = 1;
        son[u] = 0;
        
        for (auto& e : g[u]) 
        {
            int v = e.v;
            if (v == f) continue;
            dfs1(v, u, g);
            sz[u] += sz[v];
            if (sz[v] > sz[son[u]]) 
            {
                son[u] = v;
            }
        }
    }
    template <class G>
    void dfs2(int u, int t, G& g) 
    {
        top[u] = t;
        dfn_idx++;
        dfn[u] = dfn_idx;
        seg[dfn_idx] = u;
        
        if (son[u]) 
        {
            dfs2(son[u], t, g); 
        }
        for (auto& e : g[u]) 
        {
            int v = e.v;
            if (v == fa[u] || v == son[u]) continue;
            dfs2(v, v, g); 
        }
    }
    template <class G>
    void build(G& g, int root = -1) 
    {
        if (root != -1)
        {
            dfs1(root, 0, g);
            dfs2(root, root, g);
            return;
        }
        for (int i = 1; i <= n; i++)
        {
            if (!dfn[i])
            {
                dfs1(i, 0, g);
                dfs2(i, i, g);
            }
        }
    }
};

const int N = 1e5 + 10;
Graph<false, Empty> g{N, N};
HLD hld{N};
SegTree tr{N}; 
VI a(N); // a[N] -- 原始数组，存初始点权

// 树上两点路径加法
void modify_path(int u, int v, LL k) 
{
    while (hld.top[u] != hld.top[v]) 
    {
        if (hld.dep[hld.top[u]] < hld.dep[hld.top[v]]) swap(u, v);
        tr.modify(1, hld.dfn[hld.top[u]], hld.dfn[u], k);
        u = hld.fa[hld.top[u]];
    }
    if (hld.dep[u] > hld.dep[v]) swap(u, v);
    tr.modify(1, hld.dfn[u], hld.dfn[v], k);
}
// 查询树上两点路径上和
LL query_path(int u, int v) 
{
    LL res = 0;
    while (hld.top[u] != hld.top[v]) 
    {
        if (hld.dep[hld.top[u]] < hld.dep[hld.top[v]]) swap(u, v);
        res += tr.query(1, hld.dfn[hld.top[u]], hld.dfn[u]);
        u = hld.fa[hld.top[u]];
    }
    if (hld.dep[u] > hld.dep[v]) swap(u, v);
    res += tr.query(1, hld.dfn[u], hld.dfn[v]);
    return res;
}
// 子树加法
void modify_subtree(int u, LL k)
{
    tr.modify(1, hld.dfn[u], hld.dfn[u] + hld.sz[u] - 1, k);
}
// 子树求和
LL query_subtree(int u)
{
    return tr.query(1, hld.dfn[u], hld.dfn[u] + hld.sz[u] - 1);
}

/* Usage: 
void solve() 
{
    int n, m, root, mod; // 例: 求和并取模
    cin >> n >> m >> root >> mod;
    g.clear();
    hld.init(n);
    for (int i = 1; i <= n; i++) cin >> a[i];
    for (int i = 1; i < n; i++) 
    {
        int u, v; cin >> u >> v;
        g.add(u, v);
    }
    // 跑树链剖分
    hld.build(g);
    // 建立线段树。注意传入 a 和 seg
    tr.build(1, 1, n, a, hld.seg);
    // 后续根据题目要求直接调用 modify_path / query_path 即可
}
*/
