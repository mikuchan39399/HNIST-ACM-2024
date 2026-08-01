#include <iostream>
#include <climits>
#include <random>

using namespace std;
const int N = 1e5 + 10;
mt19937 rnd(random_device{}());
struct Treap_node
{
    int lc, rc;
    int cnt, sz, rd, val;
} tr[N];
int n, root, idx;

int newnode(int v)
{
    idx++;
    tr[idx].cnt = tr[idx].sz = 1;
    tr[idx].val = v;
    tr[idx].rd = rnd();
    return idx;
}

void pushup(int x)
{
    if(!x) return;
    tr[x].sz = tr[tr[x].lc].sz + tr[tr[x].rc].sz + tr[x].cnt;
}

void ro_left(int& x)
{
    int y = tr[x].rc;
    tr[x].rc = tr[y].lc;
    tr[y].lc = x;
    x = y;
}

void ro_right(int& x)
{
    int y = tr[x].lc;
    tr[x].lc = tr[y].rc;
    tr[y].rc = x;
    x = y;
}

void insert(int& x, int v)
{
    if(!x)
    {
        x = newnode(v);
        return;
    }
    if(tr[x].val == v) tr[x].cnt++;
    else if(tr[x].val > v)
    {
        insert(tr[x].lc, v);
        if(tr[tr[x].lc].rd < tr[x].rd) ro_right(x);
    }
    else
    {
        insert(tr[x].rc, v);
        if(tr[tr[x].rc].rd < tr[x].rd) ro_left(x);
    }
    pushup(x);
}

void erase(int& x, int v)
{
    if(tr[x].val == v)
    {
        if(tr[x].cnt > 1) tr[x].cnt--;
        else
        {
            if(!tr[x].lc || !tr[x].rc) x = tr[x].lc + tr[x].rc;
            else
            {
                if(tr[tr[x].lc].rd < tr[tr[x].rc].rd)
                {
                    ro_right(x);
                    erase(tr[x].rc, v);
                }
                else
                {
                    ro_left(x);
                    erase(tr[x].lc, v);
                }
            }
        }
    }
    else v < tr[x].val ? erase(tr[x].lc, v) : erase(tr[x].rc, v);
    pushup(x);
}

int get_rank(int x, int k) // AVL中有多少节点比 k 小
{
    if(!x) return 0;
    if(tr[x].val >= k) return get_rank(tr[x].lc, k);
    else return tr[x].cnt + tr[tr[x].lc].sz + get_rank(tr[x].rc, k);
}

int get_val(int x, int k) // 查询目标节点在 AVL 中排名第 k 的数字
{
    if(tr[tr[x].lc].sz >= k) return get_val(tr[x].lc, k);
    else if(tr[tr[x].lc].sz + tr[x].cnt >= k) return tr[x].val;
    else return get_val(tr[x].rc, k - tr[tr[x].lc].sz - tr[x].cnt);
}

int get_pre(int x, int k) // 寻找前驱节点
{
    if(!x) return INT_MIN;
    if(tr[x].val >= k) return get_pre(tr[x].lc, k);
    else return max(tr[x].val, get_pre(tr[x].rc, k));
}

int get_suf(int x, int k) // 寻找后继节点
{
    if(!x) return INT_MAX;
    if(tr[x].val <= k) return get_suf(tr[x].rc, k);
    else return min(tr[x].val, get_suf(tr[x].lc, k));
}