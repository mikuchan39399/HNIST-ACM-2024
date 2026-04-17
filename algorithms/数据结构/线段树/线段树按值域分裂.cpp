#include <iostream>
#include <vector>

using namespace std;
using LL = long long;

const int N = 2e5 + 10;   // 区间大小
const int M = 1.2e7 + 10; // 动态开点最大节点数，通常设为 N * logN 的常数倍

#define lc(p) tr[p].lc
#define rc(p) tr[p].rc

struct Node 
{
    int lc, rc;
    LL cnt;
} tr[M];

int root[N];      // 记录每个线段树的根节点
int rub[M];       // 垃圾桶，回收废弃节点
int idx, rub_cnt; // idx: 节点分配器, rub_cnt: 垃圾桶计数

// --- 核心逻辑 ---

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

// 单点修改 (在第 p 棵树的 x 位置增加 k)
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

// 线段树合并 (将 y 树合并到 x 树)
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

// 线段树分裂 (把 u 树中 [x, y] 的部分分裂出来，形成以 v 为根的新树)
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

// 区间查询 (查询 p 树中 [x, y] 范围内的 cnt 总和)
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

// 查询第 k 小 (查询 p 树中的第 k 小的值)
LL query_kth(int p, int l, int r, LL k) 
{
    if (l == r) return l;
    int mid = l + (r - l) / 2;
    LL left_cnt = tr[lc(p)].cnt;
    
    if (k <= left_cnt) return query_kth(lc(p), l, mid, k);
    else return query_kth(rc(p), mid + 1, r, k - left_cnt);
}

// --- 使用样例 ---
void example_usage() 
{
    int max_val = 100000; // 权值最大范围
    int id = 1;           // 树的编号分配器
    
    // 1. 初始化并插入数据：向第 1 棵树中插入数字 5，数量为 3
    modify(root[1], 1, max_val, 5, 3);
    modify(root[1], 1, max_val, 10, 2);
    
    // 2. 分裂：把第 1 棵树中范围 [1, 7] 的元素分裂到新树 (编号 2) 中
    ++id; 
    split(root[1], 1, max_val, root[id], 1, 7);
    
    // 3. 区间查询：查询第 2 棵树中范围 [1, 10] 的数字总数
    LL sum = query_sum(root[2], 1, max_val, 1, 10);
    
    // 4. 合并：将第 2 棵树合并回第 1 棵树
    root[1] = merge(root[1], root[2], 1, max_val);
    
    // 5. 第 K 小查询：查询第 1 棵树中第 4 小的元素
    LL kth_val = query_kth(root[1], 1, max_val, 4);
}