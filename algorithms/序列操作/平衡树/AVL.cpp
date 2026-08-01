#include <iostream>
#include <climits>

using namespace std;
const int N = 1e5 + 10;
struct AVL_node
{
    int lc, rc;
    int val, cnt, h, sz;
}tr[N];
int n, root, idx;

void pushup(int x)
{
    if(!x) return;
    tr[x].h = max(tr[tr[x].lc].h, tr[tr[x].rc].h) + 1;
    tr[x].sz = tr[tr[x].lc].sz + tr[tr[x].rc].sz + tr[x].cnt; 
}

void ro_left(int& x)
{
    int y = tr[x].rc;
    tr[x].rc = tr[y].lc;
    tr[y].lc = x;
    pushup(x);
    pushup(y);
    x = y;
}

void ro_right(int& x)
{
    int y = tr[x].lc;
    tr[x].lc = tr[y].rc;
    tr[y].rc = x;
    pushup(x);
    pushup(y);
    x = y;
}

void rotate(int& x)
{
    if(!x) return;    
    if(tr[tr[x].lc].h - tr[tr[x].rc].h > 1) // L
    {
        if(tr[tr[tr[x].lc].lc].h >= tr[tr[tr[x].lc].rc].h) // LL
        {
            ro_right(x);
        }
        else // LR
        {
            ro_left(tr[x].lc);
            ro_right(x);
        }
    }
    else if(tr[tr[x].lc].h - tr[tr[x].rc].h < -1) // R
    {
        if(tr[tr[tr[x].rc].rc].h >= tr[tr[tr[x].rc].lc].h) // RR
        {
            ro_left(x);
        }
        else // RL
        {
            ro_right(tr[x].rc);
            ro_left(x);
        }
    }
}

void insert(int& x, int v)
{
    if(!x)
    {
        x = ++idx;
        tr[x].cnt = tr[x].h = tr[x].sz = 1;
        tr[x].val = v;
        return;
    }
    if(tr[x].val == v) tr[x].cnt++;
    else if(tr[x].val < v) insert(tr[x].rc, v);
    else insert(tr[x].lc, v);
    pushup(x);
    rotate(x);
}

void erase(int& x, int v)
{
    if(!x) return;
    if(tr[x].val == v)
    {
        if(tr[x].cnt > 1) tr[x].cnt--;
        else
        {
            if(!tr[x].lc || !tr[x].rc) x = tr[x].lc + tr[x].rc;
            else
            {
                int y = tr[x].lc;
                while(tr[y].rc) y = tr[y].rc;
                tr[x].cnt = tr[y].cnt;
                tr[x].val = tr[y].val;
                tr[y].cnt = 0;
                erase(tr[x].lc, tr[y].val);
            }
        }
    }
    else
    {
        if(tr[x].val < v) erase(tr[x].rc, v);
        else erase(tr[x].lc, v);
    }
    pushup(x);
    rotate(x);
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