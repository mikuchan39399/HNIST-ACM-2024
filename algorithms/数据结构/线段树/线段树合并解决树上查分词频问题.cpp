#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;
using LL = long long;

const int N = 1e5 + 10;
const int M = 8e6 + 10; // 动态开点空间：大概 N * 60 ~ N * 80
const int UP = 1e5;

int root[N], idx;
int n, m;
struct Node 
{
    int lc, rc;
    int maxc, id;
} tr[M];

#define lc(p) tr[p].lc
#define rc(p) tr[p].rc

vector<int> edges[N];
int fa[N][25], dep[N];
int ret[N];

void pushup(int p) 
{
    if (tr[lc(p)].maxc >= tr[rc(p)].maxc) 
    {
        tr[p].maxc = tr[lc(p)].maxc;
        tr[p].id = tr[lc(p)].id;
    } 
    else 
    {
        tr[p].maxc = tr[rc(p)].maxc;
        tr[p].id = tr[rc(p)].id;
    }
}

void modify(int& p, int l, int r, int pos, int val) 
{
    if (!p) p = ++idx;
    if (l == r) 
    {
        tr[p].maxc += val;
        tr[p].id = l;
        return;
    }
    int mid = l + (r - l) / 2;
    if (pos <= mid) modify(lc(p), l, mid, pos, val);
    else            modify(rc(p), mid + 1, r, pos, val);
    pushup(p);
}

// 
int merge(int x, int y, int l, int r) 
{
    // 核心逻辑 1：移花接木。如果有空节点，直接返回另一个节点的指针（子树整体嫁接）
    if (!x || !y) return x | y;
    // 核心逻辑 2：叶子节点硬刚。两个树在同一种类都有值，直接累加频次 (破坏性合并至 x)
    if (l == r) 
    {
        tr[x].maxc += tr[y].maxc;
        return x;
    }
    int mid = l + (r - l) / 2;
    // 核心逻辑 3：继续向下“拉拉链”，并重新接好当前节点 x 的左右儿子
    lc(x) = merge(lc(x), lc(y), l, mid);
    rc(x) = merge(rc(x), rc(y), mid + 1, r);
    pushup(x);
    return x;
}

void dfs(int x, int f) 
{
    fa[x][0] = f;
    dep[x] = dep[f] + 1;
    for (int i = 1; i <= 20; i++) 
    {
        fa[x][i] = fa[fa[x][i - 1]][i - 1];
    }
    
    for (int y : edges[x]) 
    {
        if (y == f) continue;
        dfs(y, x);
    }
}

int lca(int x, int y) 
{
    if (dep[x] > dep[y]) swap(x, y);
    
    for (int i = 20; i >= 0; i--) 
    {
        if (dep[x] <= dep[fa[y][i]]) y = fa[y][i];
    }
    if (x == y) return x;
    
    for (int i = 20; i >= 0; i--) 
    {
        if (fa[x][i] != fa[y][i]) 
        {
            x = fa[x][i];
            y = fa[y][i];
        }
    }
    return fa[x][0];
}

void dp_merge(int x, int f) 
{
    for (int y : edges[x]) 
    {
        if (y == f) continue;
        dp_merge(y, x);
        root[x] = merge(root[x], root[y], 1, UP);
    }
    ret[x] = tr[root[x]].maxc == 0 ? 0 : tr[root[x]].id;
}

void solve() 
{
    cin >> n >> m;
    dfs(1, 0);
    while(m--) 
    {
        int u, v, val; cin >> u >> v >> val;
        int ancestor = lca(u, v);
        modify(root[u], 1, UP, val, 1);
        modify(root[v], 1, UP, val, 1);
        modify(root[ancestor], 1, UP, val, -1);
        modify(root[fa[ancestor][0]], 1, UP, val, -1);
    }
    dp_merge(1, 0);
}