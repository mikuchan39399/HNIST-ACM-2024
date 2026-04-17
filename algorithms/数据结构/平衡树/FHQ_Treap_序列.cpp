// FHQ_Treap_序列.cpp
#include <random>
using namespace std;
const int N = 1e5 + 10;
mt19937 rnd(random_device{}());
struct Treap_Node
{
    int lc, rc;
    int sz, rd, sum, val;
    int add;
} tr[N];
int idx, root;

int newnode(int v)
{
    idx++;
    tr[idx].sum = tr[idx].val = v;
    tr[idx].sz = 1;
    tr[idx].rd = rnd();
    return idx;
}

void lazy(int x, int k)
{
    if(!x) return;
    tr[x].add += k;
    tr[x].val += k;
    tr[x].sum += tr[x].sz * k;
}

void pushdown(int x)
{
    if(tr[x].add)
    {
        lazy(tr[x].lc, tr[x].add);
        lazy(tr[x].rc, tr[x].add);
        tr[x].add = 0;
    }
}

void pushup(int x)
{
    tr[x].sz = tr[tr[x].lc].sz + tr[tr[x].rc].sz + 1;
    tr[x].sum = tr[tr[x].lc].sum + tr[tr[x].rc].sum + tr[x].val;
}

void split_rank(int p, int k, int& x, int& y)
{
    if(!p)
    {
        x = y = 0;
        return;
    }
    pushdown(p);
    if(tr[tr[p].lc].sz + 1 <= k)
    {
        x = p;
        split_rank(tr[p].rc, k - tr[tr[p].lc].sz - 1, tr[x].rc, y);
    }
    else
    {
        y = p;
        split_rank(tr[p].lc, k, x, tr[y].lc);
    }
    pushup(p);
}

int merge(int x, int y)
{
    if(!x || !y) return x + y;
    if(tr[x].rd < tr[y].rd)
    {
        pushdown(x);
        tr[x].rc = merge(tr[x].rc, y);
        pushup(x);
        return x;
    }
    else 
    {
        pushdown(y);
        tr[y].lc = merge(x, tr[y].lc);
        pushup(y);
        return y;
    }
}

void add(int pos, int v)
{
    int x, y;
    split_rank(root, pos - 1, x, y);
    root = merge(merge(x, newnode(v)), y);
}

int get_sum(int l, int r)
{
    int x, y, z;
    split_rank(root, r, x, z);
    split_rank(x, l - 1, x, y);
    int ret = tr[y].sum;
    root = merge(merge(x, y), z);
    return ret;
}

void move_interval(int l, int r, int pos)
{
    int w, x, y, z;
    if(r < pos)
    {
        split_rank(root, pos - 1, w, z);
        split_rank(w, r, w, y);
        split_rank(w, l - 1, w, x);
        root = merge(w, merge(y, merge(x, z)));
    }
    else
    {
        split_rank(root, r, x, z);
        split_rank(x, l - 1, x, y);
        split_rank(x, pos, x, w);
        root = merge(x, merge(y, merge(w, z)));
    }
}


void modify(int l, int r, int d)
{
    int x, y, z;
    split_rank(root, r, x, z);
    split_rank(x, l - 1, x, y);
    lazy(y, d);
    root = merge(merge(x, y), z);
}