// zoi: segFhq
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>

using namespace std;

const int N = 2e5 + 10;
const int inf = 2147483647;
// 线段树套 FHQ, a[1..n]; 值域须落在下面的二分边界内, 中间运算不溢出 int
// 修改/排名/前驱/后继期望 O(log² n), kth 再乘 O(log 值域), 栈 O(log n)
const int MIN_VAL = -1; // 二分下界
const int MAX_VAL = 1e8 + 1; // 二分上界

int n, m, a[N];

int root[N << 2]; // 各区间的 Treap 根
int idx;

struct Node
{
    int lc, rc;
    int val, rd, sz;
} tr[40 * N]; // 不回收; 总用点 O((n+修改次数)log n), 须小于 40N

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

// 按值分为 <=v 和 >v 两棵树. 期望时间/栈 O(log n)
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

// 合并, 要求 max(x)<=min(y). 期望时间/栈 O(log n)
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

// 返回 <v 的元素个数
int get_rank(int& rt, int v)
{
    int x, y;
    split(rt, v - 1, x, y);
    int ret = tr[x].sz;
    rt = merge(x, y);
    return ret;
}

// 子树第 k 小, k 必须合法
int get_val(int x, int k)
{
    if(tr[tr[x].lc].sz >= k) return get_val(tr[x].lc, k);
    else if(tr[tr[x].lc].sz + 1 == k) return tr[x].val;
    else return get_val(tr[x].rc, k - tr[tr[x].lc].sz - 1);
}

// 严格前驱, 无则 -inf
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

// 严格后继, 无则 inf
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

void build_node(int p, int l, int r)
{
    for(int i = l; i <= r; i++) insert(root[p], a[i]);
    if(l == r) return;
    int mid = l + (r - l) / 2;
    build_node(p << 1, l, mid);
    build_node(p << 1 | 1, mid + 1, r);
}

// 先填 a[1..n], 再重建并清空旧点池; 0<=n<N. 期望时间 O(n log² n), 用点 O(n log n)
void build(int _n)
{
    n = _n;
    idx = 0;
    tr[0] = {};
    fill(root, root + max(1, 4 * n) + 1, 0);
    if (n) build_node(1, 1, n);
}

// 将位置 x 改为 k, 调用后再更新 a[x]=k
void modify(int p, int l, int r, int x, int k)
{
    erase(root[p], a[x]);
    insert(root[p], k);
    if(l == r) return;
    int mid = (l + r) >> 1;
    if(x <= mid) modify(p << 1, l, mid, x, k);
    else modify(p << 1 | 1, mid + 1, r, x, k);
}

// 返回 [x,y] 中 <k 的数量(排名需再 +1)
int query_rank(int p, int l, int r, int x, int y, int k)
{
    if(x <= l && r <= y) return get_rank(root[p], k);
    int mid = (l + r) >> 1, sum = 0;
    if(x <= mid) sum += query_rank(p << 1, l, mid, x, y, k);
    if(y > mid) sum += query_rank(p << 1 | 1, mid + 1, r, x, y, k);
    return sum;
}

// [x,y] 第 k 小, k 必须合法
int query_kth(int x, int y, int k)
{
    int l = MIN_VAL - 1, r = MAX_VAL + 1;
    while(l + 1 != r)
    {
        int mid = l + (r - l) / 2;
        if(query_rank(1, 1, n, x, y, mid) + 1 <= k) l = mid;
        else r = mid;
    }
    return l;
}

// [x,y] 严格前驱, 无则 -inf
int query_pre(int p, int l, int r, int x, int y, int k)
{
    if(l >= x && r <= y) return get_pre(root[p], k);
    int mid = (l + r) >> 1;
    int ret = -inf;
    if(x <= mid) ret = max(ret, query_pre(p << 1, l, mid, x, y, k));
    if(y > mid) ret = max(ret, query_pre(p << 1 | 1, mid + 1, r, x, y, k));
    return ret;
}

// [x,y] 严格后继, 无则 inf
int query_suf(int p, int l, int r, int x, int y, int k)
{
    if(l >= x && r <= y) return get_suf(root[p], k);
    int mid = (l + r) >> 1;
    int ret = inf;
    if(x <= mid) ret = min(ret, query_suf(p << 1, l, mid, x, y, k));
    if(y > mid) ret = min(ret, query_suf(p << 1 | 1, mid + 1, r, x, y, k));
    return ret;
}

/* Usage
int main()
{
    a[1] = 3; a[2] = 1; a[3] = 2;
    build(3); // 先填 a 再 build
    cout << query_kth(1, 3, 2) << '\n'; // 2
    modify(1, 1, n, 2, 7); a[2] = 7; // modify 使用旧 a[x], 随后维护原数组
    build(3); // 保留当前 a, 重新建立全部树
    cout << query_kth(1, 3, 2) << '\n'; // 3
}
*/
