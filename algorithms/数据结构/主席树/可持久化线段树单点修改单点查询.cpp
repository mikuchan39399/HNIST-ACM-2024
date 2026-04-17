#include <iostream>

using namespace std;

// 空间复杂度: 基础开树 4*N + 每次修改操作新开 log2(区间长度) 个节点
const int N = 1e6 + 10;
const int MAX_NODE = N * 30;

int root[N]; // 记录每个历史版本的根节点编号
int idx = 0; // 内存池分配指针

struct Node 
{
    int lc, rc; // 左、右子节点在内存池中的编号
    int val;
} tr[MAX_NODE];

// 初始建树: p 为当前节点，a 为初始数据数组
void build(int& p, int l, int r, const int* a) 
{
    p = ++idx;
    if (l == r) 
    {
        tr[p].val = a[l];
        return;
    }
    int mid = l + (r - l) / 2;
    build(tr[p].lc, l, mid, a);
    build(tr[p].rc, mid + 1, r, a);
}

// 可持久化单点修改: 走到哪复制到哪，生成新版本的节点
// p 为新版本当前节点，v 为旧版本对应节点，将位置 x 的值修改为 k
void modify(int& p, int l, int r, int v, int x, int k) 
{
    // 从内存池分配新节点，并直接拷贝旧版本的子节点指针与数据
    p = ++idx;
    tr[p] = tr[v]; 
    if (l == r) 
    {
        tr[p].val = k;
        return;
    }
    int mid = l + (r - l) / 2;
    // 递归修改受影响的路径，连接新创建的子节点
    if(x <= mid) modify(tr[p].lc, l, mid, tr[v].lc, x, k);
    else modify(tr[p].rc, mid + 1, r, tr[v].rc, x, k);
}

// 可持久化单点查询: 指定历史版本的根节点进行普通线段树查询
// p 为指定历史版本的当前节点
int query(int p, int l, int r, int x) 
{
    if (l == r) 
    {
        return tr[p].val;
    }
    int mid = l + (r - l) / 2;
    if(x <= mid) return query(tr[p].lc, l, mid, x);
    else return query(tr[p].rc, mid + 1, r, x);
}