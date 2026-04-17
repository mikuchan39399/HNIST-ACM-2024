#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

const int N = 1e5 + 10;

// ==================【变量定义区】==================
// 1. 图与树上倍增相关
vector<int> edges[N];
int fa[N][25], dep[N];

// 2. 主席树与离散化相关
int w[N];               // 节点原始权值
vector<int> vals;       // 离散化去重后的权值数组
int root[N];            // 每个节点对应的主席树根节点版本号
int idx;                // 主席树节点分配动态开点

struct Node 
{
    int lc, rc, cnt;
} tr[30 * N];           // 空间一般开 N * (log2(N) + 2) 左右

#define lc(x) tr[x].lc
#define rc(x) tr[x].rc

// ==================【工具函数区】==================
// 获取离散化后的排名(1-based)
int get_id(int x) 
{
    return lower_bound(vals.begin(), vals.end(), x) - vals.begin() + 1;
}

// ==================【主席树核心区】==================
void pushup(int p) 
{
    tr[p].cnt = tr[lc(p)].cnt + tr[rc(p)].cnt;
}

// 动态开点插入，v 为上一个版本，x 为要插入的值的离散化离散化排名，c 为增加的数量
void modify(int& p, int l, int r, int v, int x, int c) 
{
    p = ++idx;
    tr[p] = tr[v];
    if (l == r) 
    {
        tr[p].cnt += c;
        return;
    }
    int mid = l + (r - l) / 2;
    if (x <= mid) modify(lc(p), l, mid, lc(v), x, c);
    else modify(rc(p), mid + 1, r, rc(v), x, c);
    pushup(p);
}

// 树上容斥查询第 k 小
// u, v 为路径两端点，la 为 LCA，fa_la 为 LCA 的父节点
int query(int u, int v, int la, int fa_la, int l, int r, int k)
{
    if (l == r) return l; // 返回的是离散化数组的下标(1-based)
    // 树上容斥原理公式
    int cnt = tr[lc(u)].cnt + tr[lc(v)].cnt - tr[lc(la)].cnt - tr[lc(fa_la)].cnt;
    
    int mid = l + (r - l) / 2;
    if (k <= cnt) return query(lc(u), lc(v), lc(la), lc(fa_la), l, mid, k);
    else return query(rc(u), rc(v), rc(la), rc(fa_la), mid + 1, r, k - cnt);
}

// ==================【树上倍增与预处理区】==================
// DFS 遍历：预处理深度、倍增祖先，并建好前缀主席树
void dfs(int x, int f) 
{
    fa[x][0] = f;
    for (int i = 1; i <= 20; i++) 
    {
        fa[x][i] = fa[fa[x][i - 1]][i - 1];
    }
    
    // 基于父亲节点的版本，插入当前节点的值
    modify(root[x], 1, vals.size(), root[f], get_id(w[x]), 1);
    
    for (int y : edges[x]) 
    {
        if (y == f) continue;
        dep[y] = dep[x] + 1;
        dfs(y, x);
    }
}

int lca(int u, int v) 
{
    if (dep[u] > dep[v]) swap(u, v);
    for (int i = 20; i >= 0; i--) {
        if (dep[fa[v][i]] >= dep[u]) v = fa[v][i];
    }
    if (u == v) return u;
    for (int i = 20; i >= 0; i--) 
    {
        if (fa[u][i] != fa[v][i]) 
        {
            u = fa[u][i];
            v = fa[v][i];
        }
    }
    return fa[u][0];
}