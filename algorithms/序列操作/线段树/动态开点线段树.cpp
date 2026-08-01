#include <iostream>

using namespace std;

using LL = long long;

// 动态开点的空间: 操作数 M * log2(线段总长)
const int N = 1.5e7 + 10; 

int root = 0; // 根节点，初始为 0 表示尚未分配
int idx = 0;  // 内存池分配指针

struct Node 
{
    int lc, rc; // 左、右子节点在内存池中的编号。0 表示暂未开点
    LL sum;
    LL add;
} tr[N];

void lazy(int& p, int l, int r, LL k) 
{
    // 动态开点核心：按需分配。如果当前节点尚未分配编号，则从内存池中取一个新编号
    if (!p) 
    {
        p = ++idx; 
    }
    tr[p].sum += 1ll * (r - l + 1) * k;
    tr[p].add += k;
}

void pushdown(int p, int l, int r) 
{
    if (tr[p].add) 
    {
        int mid = l + (r - l) / 2;
        // 标记下传时，如果子节点不存在，lazy 函数内部会自动为其开点
        lazy(tr[p].lc, l, mid, tr[p].add);
        lazy(tr[p].rc, mid + 1, r, tr[p].add);
        tr[p].add = 0;
    }
}

void pushup(int p) 
{
    tr[p].sum = tr[tr[p].lc].sum + tr[tr[p].rc].sum;
}

void modify(int& p, int l, int r, int x, int y, LL k) 
{
    // 走到哪开到哪: 只要访问到了当前区间，无论是否完全覆盖，都需要保证该节点存在
    if (!p) 
    {
        p = ++idx;
    }
    
    if (l >= x && r <= y) 
    {
        lazy(p, l, r, k);
        return;
    }
    
    pushdown(p, l, r);
    int mid = l + (r - l) / 2;
    
    if (x <= mid) 
    {
        modify(tr[p].lc, l, mid, x, y, k);
    }
    if (y > mid)  
    {
        modify(tr[p].rc, mid + 1, r, x, y, k);
    }
    
    pushup(p);
}

LL query(int p, int l, int r, int x, int y) 
{
    // 动态开点查询剪枝: 如果当前节点为 0，说明该区间及其子区间从未被修改过，直接返回默认值 0 即可，无需继续向下递归
    if (!p) 
    {
        return 0;
    }
    
    if (l >= x && r <= y) 
    {
        return tr[p].sum;
    }
    
    pushdown(p, l, r);
    int mid = l + (r - l) / 2;
    LL sum = 0;
    
    if (x <= mid) 
    {
        sum += query(tr[p].lc, l, mid, x, y);
    }
    if (y > mid)  
    {
        sum += query(tr[p].rc, mid + 1, r, x, y);
    }
    return sum;
}