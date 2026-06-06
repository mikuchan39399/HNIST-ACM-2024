#include <iostream>
#include <vector>
#include <random>

using namespace std;


const int N = 2e5 + 10;
const int inf = 2147483647;
// 注意：如果题目值域不同，请务必修改这里的二分边界！
const int MIN_VAL = -1;       // 题目的最小值下界
const int MAX_VAL = 1e8 + 1;  // 题目的最大值上界

int n, m, a[N];

// ==========================================
// [内层：FHQ Treap 模板]
// ==========================================
int root[N << 2]; // 线段树每个节点对应的 Treap 根节点
int idx;          // Treap 节点总空间池指针

struct Node 
{
    int lc, rc;
    int val, rd, sz;
} tr[40 * N]; // 空间开 40倍N 足够 (N * logN 级别)

mt19937 rnd{random_device{}()};

int newnode(int v) 
{
    idx++;
    tr[idx].val = v;
    tr[idx].sz = 1;
    tr[idx].rd = rnd();
    tr[idx].lc = tr[idx].rc = 0;
    return idx;   
} 

void pushup(int p) 
{
    tr[p].sz = tr[tr[p].lc].sz + tr[tr[p].rc].sz + 1;
}

// 按值分裂：<= v 的在左树 x，> v 的在右树 y
void split(int p, int v, int& x, int& y) 
{
    if(!p) 
    {
        x = y = 0;
        return;
    }
    if(tr[p].val <= v) 
    {
        x = p;
        split(tr[p].rc, v, tr[x].rc, y);
    } 
    else 
    {
        y = p;
        split(tr[p].lc, v, x, tr[y].lc);
    }
    pushup(p);
}

// 合并：必须满足 x 树的所有值 <= y 树的所有值
int merge(int x, int y) 
{
    if(!x || !y) return x + y;
    if(tr[x].rd < tr[y].rd) {
        tr[x].rc = merge(tr[x].rc, y);
        pushup(x);
        return x;
    } else {
        tr[y].lc = merge(x, tr[y].lc);
        pushup(y);
        return y;
    }
}

void insert(int& rt, int v)
{
    int x, y;
    split(rt, v, x, y);
    rt = merge(merge(x, newnode(v)), y);
}

void erase(int& rt, int v) 
{
    int x, y, z;
    split(rt, v, x, z);
    split(x, v - 1, x, y);
    // 只删一个节点
    if (y) y = merge(tr[y].lc, tr[y].rc);
    rt = merge(x, merge(y, z));
}

// 查询排名 (比 v 小的数的个数 + 1, 此处返回的是 < v 的个数，外层需要按需 +1)
int get_rank(int& rt, int v) 
{
    int x, y;
    split(rt, v - 1, x, y);
    int ret = tr[x].sz;
    rt = merge(x, y);
    return ret;
}

// 查询第 k 小的值 (在单个 Treap 内)
int get_val(int x, int k) 
{
    if(tr[tr[x].lc].sz >= k) return get_val(tr[x].lc, k);
    else if(tr[tr[x].lc].sz + 1 == k) return tr[x].val;
    else return get_val(tr[x].rc, k - tr[tr[x].lc].sz - 1);
}

// 找前驱 (< v 的最大值)
int get_pre(int& rt, int v) 
{
    int x, y;
    split(rt, v - 1, x, y);
    if (!x) {
        rt = merge(x, y);
        return -inf;
    }
    int ret = get_val(x, tr[x].sz);
    rt = merge(x, y);
    return ret;
}

// 找后继 (> v 的最小值)
int get_suf(int& rt, int v) 
{
    int x, y;
    split(rt, v, x, y);
    if (!y) {
        rt = merge(x, y);
        return inf;
    }
    int ret = get_val(y, 1);
    rt = merge(x, y);
    return ret;
}


// ==========================================
// [外层：线段树 模板]
// ==========================================

void build(int p, int l, int r) 
{
    for(int i = l; i <= r; i++) insert(root[p], a[i]);
    if(l == r) return;
    int mid = l + (r - l) / 2;
    build(p << 1, l, mid);
    build(p << 1 | 1, mid + 1, r);
}

// 单点修改 a[x] = k
void modify(int p, int l, int r, int x, int k) 
{
    erase(root[p], a[x]);
    insert(root[p], k);
    if(l == r) return;
    int mid = (l + r) >> 1;
    if(x <= mid) modify(p << 1, l, mid, x, k);
    else modify(p << 1 | 1, mid + 1, r, x, k);
}

// 区间查排名 (返回区间内 < k 的数量)
int query_rank(int p, int l, int r, int x, int y, int k) 
{
    if(x <= l && r <= y) return get_rank(root[p], k);
    int mid = (l + r) >> 1, sum = 0;
    if(x <= mid) sum += query_rank(p << 1, l, mid, x, y, k);
    if(y > mid) sum += query_rank(p << 1 | 1, mid + 1, r, x, y, k);
    return sum;
}

// 区间第 k 小 (通过二分答案转化为区间排名判定)
int query_kth(int x, int y, int k) 
{
    int l = MIN_VAL - 1, r = MAX_VAL + 1; // 根据题意修改值域范围
    while(l + 1 != r) 
    {
        int mid = l + (r - l) / 2;
        if(query_rank(1, 1, n, x, y, mid) + 1 <= k) l = mid;
        else r = mid;
    }
    return l;
}

// 区间前驱
int query_pre(int p, int l, int r, int x, int y, int k) 
{
    if(l >= x && r <= y) return get_pre(root[p], k);
    int mid = (l + r) >> 1;
    int ret = -inf;
    if(x <= mid) ret = max(ret, query_pre(p << 1, l, mid, x, y, k));
    if(y > mid) ret = max(ret, query_pre(p << 1 | 1, mid + 1, r, x, y, k));
    return ret;
}

// 区间后继
int query_suf(int p, int l, int r, int x, int y, int k) 
{
    if(l >= x && r <= y) return get_suf(root[p], k);
    int mid = (l + r) >> 1;
    int ret = inf;
    if(x <= mid) ret = min(ret, query_suf(p << 1, l, mid, x, y, k));
    if(y > mid) ret = min(ret, query_suf(p << 1 | 1, mid + 1, r, x, y, k));
    return ret;
}