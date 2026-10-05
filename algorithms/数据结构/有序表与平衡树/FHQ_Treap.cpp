// zoi: fhq
#ifndef Z_OI_FHQ
#define Z_OI_FHQ

#include "../../杂项/随机数/z_rnd.cpp"
#include "../../杂项/utils/utils.cpp"

// LL 有序多重集合, 值域 (-INF,INF); 增删查、分裂合并期望时间/栈 O(log n)
// 点池按累计插入次数预算, 删除不回收; 每点约 24B
struct FHQ_Treap
{
    struct node
    {
        int lc = 0, rc = 0, sz = 0, rd = 0;
        LL val = 0;
    };
    vector<node> tr;
    int idx, root, budget;
    // 按值分裂: <=v 给 x, >v 给 y
    void split(int p, LL v, int& x, int& y)
    {
        if (!p)
        {
            x = y = 0;
            return;
        }
        if (tr[p].val <= v)
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
    // 合并并返回根, 要求 max(x)<=min(y)
    int merge(int x, int y)
    {
        if (!x || !y) return x + y;
        if (tr[x].rd < tr[y].rd)
        {
            tr[x].rc = merge(tr[x].rc, y);
            pushup(x);
            return x;
        }
        tr[y].lc = merge(x, tr[y].lc);
        pushup(y);
        return y;
    }
    // 子树第 k 小, 要求 1<=k<=子树大小
    LL kth_of(int x, int k)
    {
        if (tr[tr[x].lc].sz >= k) return kth_of(tr[x].lc, k);
        if (tr[tr[x].lc].sz + 1 == k) return tr[x].val;
        return kth_of(tr[x].rc, k - tr[tr[x].lc].sz - 1);
    }
    // 预留 max_nodes 个点, 初始为空. 时间 O(1), 空间 O(max_nodes)
    FHQ_Treap(int max_nodes = 4000010) : idx(0), root(0), budget(max_nodes)
    {
        tr.reserve(budget + 1);
        tr.push_back(node());
    }
    // 从升序 a[1..m] 重建, 允许重复. 时空 O(m)
    void build(const VLL& a)
    {
        clear();
        int m = (int)a.size() - 1;
        for (int i = 2; i <= m; i++) assert(a[i - 1] <= a[i]);
        VI stk;
        stk.reserve(m + 1);
        for (int i = 1; i <= m; i++)
        {
            int cur = newnode(a[i]);
            int last = 0;
            while (!stk.empty() && tr[stk.back()].rd > tr[cur].rd)
            {
                last = stk.back();
                stk.pop_back();
            }
            tr[cur].lc = last;
            if (!stk.empty()) tr[stk.back()].rc = cur;
            stk.push_back(cur);
        }
        root = stk.empty() ? 0 : stk[0];
        finish(root);
    }
    // 插入 v, 允许重复
    void insert(LL v)
    {
        int x, y;
        split(root, v, x, y);
        root = merge(merge(x, newnode(v)), y);
    }
    // 删除一个 v, 返回是否成功
    bool erase(LL v)
    {
        int b = tr[root].sz;
        int x, y, z;
        split(root, v, x, z);
        split(x, v - 1, x, y);
        y = merge(tr[y].lc, tr[y].rc);
        root = merge(merge(x, y), z);
        return tr[root].sz < b;
    }
    // 返回 <v 的元素数(含重复)
    int get_rank(LL v)
    {
        int x, y;
        split(root, v - 1, x, y);
        int ret = tr[x].sz;
        root = merge(x, y);
        return ret;
    }
    // 第 k 小(1-based), 越界返回 INF
    LL get_kth(int k)
    {
        if (k < 1 || k > tr[root].sz) return INF;
        return kth_of(root, k);
    }
    // 严格前驱, 无则 -INF
    LL get_pre(LL v)
    {
        int x, y;
        split(root, v - 1, x, y);
        LL ret = x ? kth_of(x, tr[x].sz) : -INF;
        root = merge(x, y);
        return ret;
    }
    // 严格后继, 无则 INF
    LL get_suf(LL v)
    {
        int x, y;
        split(root, v, x, y);
        LL ret = y ? kth_of(y, 1) : INF;
        root = merge(x, y);
        return ret;
    }
    // 元素数(含重复). O(1)
    int size() { return tr[root].sz; }
    // 清空并保留容量. 时间 O(idx)
    void clear()
    {
        idx = 0;
        root = 0;
        tr.clear();
        tr.push_back(node());
    }
private:
    int newnode(LL v)
    {
        assert(idx < budget);
        tr.push_back(node());
        tr[++idx].sz = 1;
        tr[idx].val = v;
        tr[idx].rd = z_rnd(INT_MAX);
        return idx;
    }
    void pushup(int x)
    {
        tr[x].sz = tr[tr[x].lc].sz + tr[tr[x].rc].sz + 1;
    }
    void finish(int x)
    {
        if (!x) return;
        finish(tr[x].lc);
        finish(tr[x].rc);
        pushup(x);
    }
};
#endif

/* Usage:
int main()
{
    FHQ_Treap s(16);
    s.build(VLL{0, 2, 2, 5});
    cout << s.get_rank(5) << " " << s.get_kth(2) << "\n"; // 2 2
    s.erase(2);                      // 只删一个 2
    cout << s.get_pre(5) << " " << s.get_suf(2) << "\n"; // 2 5
    int x, y;
    s.split(s.root, 2, x, y);         // <= 2 与 > 2, 分裂后维护返回根
    cout << s.kth_of(x, 1) << "\n"; // 2, 子树内 k 必须合法
    s.root = s.merge(x, y);
    s.clear();
    cout << s.size() << "\n"; // 0
}
*/
