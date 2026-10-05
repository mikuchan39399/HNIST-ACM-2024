// zoi: segSplit
#include <iostream>
#include <vector>

using namespace std;
using LL = long long;

const int N = 2e5 + 10;   // 区间大小
const int M = 1.2e7 + 10; // 结点池上限, 按点改次数 * log(值域) 预留

#define lc(p) tr[p].lc
#define rc(p) tr[p].rc

struct Node
{
    int lc, rc;
    LL cnt;
} tr[M];

int root[N];      // 记录每个线段树的根节点
int rub[M];       // 回收池
int idx, rub_cnt;

int get_node()
{
    return rub_cnt ? rub[rub_cnt--] : ++idx;
}

void del(int p)
{
    rub[++rub_cnt] = p;
    tr[p] = {};
}

void pushup(int p)
{
    tr[p].cnt = tr[lc(p)].cnt + tr[rc(p)].cnt;
}

// 根 p 的值 x 增加 k; 时间/新增空间 O(log V), V=值域大小
void modify(int& p, int l, int r, int x, int k)
{
    if (!p) p = get_node();
    if (l == r)
    {
        tr[p].cnt += k;
        return;
    }
    int mid = l + (r - l) / 2;
    if (x <= mid) modify(lc(p), l, mid, x, k);
    else modify(rc(p), mid + 1, r, x, k);
    pushup(p);
}

// 合并到 x 并返回根, y 失效; 时间按重叠结点计, 栈 O(log V)
int merge(int x, int y, int l, int r)
{
    if (!x || !y) return x + y;
    if (l == r)
    {
        tr[x].cnt += tr[y].cnt;
        del(y);
        return x;
    }
    int mid = l + (r - l) / 2;
    lc(x) = merge(lc(x), lc(y), l, mid);
    rc(x) = merge(rc(x), rc(y), mid + 1, r);
    pushup(x);
    del(y);
    return x;
}

// 从 u 移出 [x,y] 到空根 v; 时间/新增空间 O(log V)
void split(int& u, int l, int r, int& v, int x, int y)
{
    if (!u) return;
    if (l >= x && r <= y)
    {
        v = u;
        u = 0;
        return;
    }
    v = get_node();
    int mid = l + (r - l) / 2;
    if (x <= mid) split(lc(u), l, mid, lc(v), x, y);
    if (y > mid) split(rc(u), mid + 1, r, rc(v), x, y);
    pushup(u);
    pushup(v);
}

// 根 p 的 [x,y] 计数和; 时间/栈 O(log V)
LL query_sum(int p, int l, int r, int x, int y)
{
    if (!p) return 0;
    if (l >= x && r <= y) return tr[p].cnt;

    int mid = l + (r - l) / 2;
    LL sum = 0;
    if (x <= mid) sum += query_sum(lc(p), l, mid, x, y);
    if (y > mid) sum += query_sum(rc(p), mid + 1, r, x, y);
    return sum;
}

// 根 p 的第 k 小值, 计数非负且 k 合法; 时间/栈 O(log V)
LL query_kth(int p, int l, int r, LL k)
{
    if (l == r) return l;
    int mid = l + (r - l) / 2;
    LL left_cnt = tr[lc(p)].cnt;

    if (k <= left_cnt) return query_kth(lc(p), l, mid, k);
    else return query_kth(rc(p), mid + 1, r, k - left_cnt);
}

// 用法
void example_usage()
{
    int max_val = 100000; // 值域上限
    int id = 1;           // 树编号

    // 插入三个 5
    modify(root[1], 1, max_val, 5, 3);
    modify(root[1], 1, max_val, 10, 2);

    // 把 [1,7] 移到新树
    ++id;
    split(root[1], 1, max_val, root[id], 1, 7);

    query_sum(root[2], 1, max_val, 1, 10);

    // 合并后停用 root[2]
    root[1] = merge(root[1], root[2], 1, max_val);

    query_kth(root[1], 1, max_val, 4);
}
