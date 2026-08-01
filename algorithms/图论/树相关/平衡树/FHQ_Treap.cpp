// FHQ Treap.cpp
#include <random>
using namespace std;
const int N = 1e5 + 10;
mt19937 rnd(random_device{}());
struct Treap_Node
{
    int lc, rc;
    int sz, val, rd;
} tr[N];
int idx, root;

int newnode(int v)
{
    idx++;
    tr[idx].val = v;
    tr[idx].sz = 1;
    tr[idx].rd = rnd();
    return idx;
}

void pushup(int x)
{
    tr[x].sz = tr[tr[x].lc].sz + tr[tr[x].rc].sz + 1;
}

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

int merge(int x, int y)
{
    if(!x || !y) return x + y;
    if(tr[x].rd < tr[y].rd)
    {
        tr[x].rc = merge(tr[x].rc, y);
        pushup(x);
        return x;
    }
    else
    {
        tr[y].lc = merge(x, tr[y].lc);
        pushup(y);
        return y;
    }
}

void insert(int v)
{
    int x, y;
    split(root, v, x, y);
    root = merge(merge(x, newnode(v)), y);
}

void erase(int v)
{
    int x, y, z;
    split(root, v, x, z);
    split(x, v - 1, x, y);
    y = merge(tr[y].lc, tr[y].rc);
    root = merge(merge(x, y), z);
}

int get_rank(int v)
{
    int x, y;
    split(root, v - 1, x, y);
    int ret = tr[x].sz;
    root = merge(x, y);
    return ret + 1;
}

int get_val(int x, int k)
{
    if(tr[tr[x].lc].sz >= k) return get_val(tr[x].lc, k);
    else if(tr[tr[x].lc].sz + 1 == k) return tr[x].val;
    else return get_val(tr[x].rc, k - tr[tr[x].lc].sz - 1);
}

int get_pre(int v)
{
    int x, y;
    split(root, v - 1, x, y);
    int ret = get_val(x, tr[x].sz);
    root = merge(x, y);
    return ret;
}

int get_suf(int v)
{
    int x, y;
    split(root, v, x, y);
    int ret = get_val(y, 1);
    root = merge(x, y);
    return ret;
}