#include <iostream>
#include <vector>
#include <set>

using namespace std;

// ======= 模板参数配置区 =======
const int N = 2e5 + 10;      // 序列最大长度
const int M = 1.2e7 + 10;    // 线段树节点池大小 (通常 N * logN * 4)
// ==============================

int n, m;
int root[N], a[N], f[N]; // root: 各区间对应的线段树根, f: 区间排序标记(0升序/1降序)
set<int> st;

// --- 内存池与垃圾回收 ---
struct Node 
{
    int lc, rc, cnt;
} tr[M];
int rub[M], cnt, idx;

inline void del(int p) 
{
    rub[++cnt] = p;
    tr[p] = {};
}
inline int get_node() 
{
    return cnt ? rub[cnt--] : ++idx;
}
inline void pushup(int p) 
{
    tr[p].cnt = tr[tr[p].lc].cnt + tr[tr[p].rc].cnt;
}

// --- 线段树核心操作 ---
void modify(int& p, int l, int r, int x, int k) 
{
    if (!p) p = get_node();
    if (l == r) 
    {
        tr[p].cnt += k;
        return;
    }
    int mid = l + (r - l) / 2;
    if (x <= mid) modify(tr[p].lc, l, mid, x, k);
    else modify(tr[p].rc, mid + 1, r, x, k);
    pushup(p);
}

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
    tr[x].lc = merge(tr[x].lc, tr[y].lc, l, mid);
    tr[x].rc = merge(tr[x].rc, tr[y].rc, mid + 1, r);
    pushup(x);
    del(y);
    return x;
}

void split(int& u, int& v, int k, int flag) 
{
    if (!u || tr[u].cnt == k) return;
    v = get_node();
    if (!flag) 
    { // 升序分裂
        int lcnt = tr[tr[u].lc].cnt;
        if (k <= lcnt) 
        {
            tr[v].rc = tr[u].rc; tr[u].rc = 0;
            split(tr[u].lc, tr[v].lc, k, flag);
        } 
        else 
        {
            split(tr[u].rc, tr[v].rc, k - lcnt, flag);
        }
    } 
    else 
    { // 降序分裂
        int rcnt = tr[tr[u].rc].cnt;
        if (k <= rcnt) 
        {
            tr[v].lc = tr[u].lc; tr[u].lc = 0;
            split(tr[u].rc, tr[v].rc, k, flag);
        } 
        else 
        {
            split(tr[u].lc, tr[v].lc, k - rcnt, flag);
        }
    }
    pushup(u); pushup(v);
}

// --- ODT 核心操作 ---
using IT = set<int>::iterator;

IT find(int x) 
{
    IT it = st.lower_bound(x);
    if (it != st.end() && *it == x) return it;
    it--;
    int d = x - *it;
    split(root[*it], root[x], d, f[*it]);
    f[x] = f[*it];
    return st.insert(x).first; // O(1) 返回新插入节点的迭代器
}

// 执行区间 [l, r] 的排序, op: 0升序, 1降序
void sort_interval(int l, int r, int op) 
{
    IT b = find(r + 1), a = find(l); // 必须先切右，再切左
    for (IT it = next(a); it != b; ++it) 
    {
        root[l] = merge(root[l], root[*it], 1, n);
    }
    f[l] = op;
    st.erase(next(a), b);
}

// 单点查询 p 位置上的元素
int query(int p, int l, int r) 
{
    if (l == r) return l;
    int mid = l + (r - l) / 2;
    if (tr[tr[p].lc].cnt) return query(tr[p].lc, l, mid);
    else return query(tr[p].rc, mid + 1, r);
}

// ==============================

void solve() 
{
    cin >> n >> m;
    // 1. 初始化每个单点为一个独立区间
    for (int i = 1; i <= n; i++) 
    {
        cin >> a[i];
        modify(root[i], 1, n, a[i], 1);
        st.insert(i);
    }
    st.insert(n + 1); // 边界哨兵

    // 2. 处理局部排序操作
    for (int i = 1; i <= m; i++) 
    {
        int op, l, r; 
        cin >> op >> l >> r;
        sort_interval(l, r, op);
    }

    // 3. 询问并输出
    int q; 
    cin >> q;
    find(q + 1); find(q); // 切割出单点
    cout << query(root[q], 1, n) << '\n';
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    solve();
    return 0;
}