// zoi: scapegoat
#ifndef Z_OI_SGT
#define Z_OI_SGT

#include "../../杂项/utils/utils.cpp"

// LL 有序多重集合, 值域 (-INF,INF); 增删均摊 O(log n), 查询 O(log n)
// 点池按峰值存活数预算, 删除回收; 每点约 24B, 重建额外 O(n) 空间
struct SGTree
{
    struct node
    {
        int lc = 0, rc = 0, sz = 0;
        LL val = 0;
    };
    vector<node> tr;
    VI seq;
    VI rub;
    int idx, root, budget;
    // 预留 max_nodes 个点, 初始为空. 时间 O(1), 空间 O(max_nodes)
    SGTree(int max_nodes = 1000010) : idx(0), root(0), budget(max_nodes)
    {
        tr.reserve(budget + 1);
        tr.push_back(node());
        rub.reserve(budget);
    }
    // 插入 v, 允许重复
    void insert(LL v) { root = insert_at(root, v); }
    // 删除一个 v, 返回是否成功
    bool erase(LL v)
    {
        bool ok = false;
        root = erase_at(root, v, ok);
        return ok;
    }
    // 返回 <v 的元素数(含重复)
    int get_rank(LL v)
    {
        int ret = 0, p = root;
        while (p)
        {
            if (tr[p].val < v)
            {
                ret += tr[tr[p].lc].sz + 1;
                p = tr[p].rc;
            }
            else p = tr[p].lc;
        }
        return ret;
    }
    // 第 k 小(1-based), 越界返回 INF
    LL get_kth(int k)
    {
        if (k < 1 || k > tr[root].sz) return INF;
        int p = root;
        while (true)
        {
            int lsz = tr[tr[p].lc].sz;
            if (k <= lsz) p = tr[p].lc;
            else if (k == lsz + 1) return tr[p].val;
            else
            {
                k -= lsz + 1;
                p = tr[p].rc;
            }
        }
    }
    // 严格前驱, 无则 -INF
    LL get_pre(LL v)
    {
        LL ret = -INF;
        int p = root;
        while (p)
        {
            if (tr[p].val < v)
            {
                ret = tr[p].val;
                p = tr[p].rc;
            }
            else p = tr[p].lc;
        }
        return ret;
    }
    // 严格后继, 无则 INF
    LL get_suf(LL v)
    {
        LL ret = INF;
        int p = root;
        while (p)
        {
            if (tr[p].val > v)
            {
                ret = tr[p].val;
                p = tr[p].lc;
            }
            else p = tr[p].rc;
        }
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
        seq.clear();
        rub.clear();
    }
private:
    int newnode(LL v)
    {
        int id;
        if (!rub.empty())
        {
            id = rub.back();
            rub.pop_back();
            tr[id] = node();
        }
        else
        {
            assert(idx < budget);
            tr.push_back(node());
            id = ++idx;
        }
        tr[id].sz = 1;
        tr[id].val = v;
        return id;
    }
    void pushup(int x)
    {
        tr[x].sz = tr[tr[x].lc].sz + tr[tr[x].rc].sz + 1;
    }
    bool imbalanced(int x)
    {
        int mx = max(tr[tr[x].lc].sz, tr[tr[x].rc].sz);
        return 4 * mx > 3 * tr[x].sz;
    }
    void collect(int x)
    {
        if (!x) return;
        collect(tr[x].lc);
        seq.push_back(x);
        collect(tr[x].rc);
    }
    int rebuild_range(int l, int r)
    {
        if (l > r) return 0;
        int mid = (l + r) >> 1;
        int p = seq[mid];
        tr[p].lc = rebuild_range(l, mid - 1);
        tr[p].rc = rebuild_range(mid + 1, r);
        pushup(p);
        return p;
    }
    int rebuild(int p)
    {
        seq.clear();
        collect(p);
        return rebuild_range(0, (int)seq.size() - 1);
    }
    int insert_at(int p, LL v)
    {
        if (!p) return newnode(v);
        if (v < tr[p].val) tr[p].lc = insert_at(tr[p].lc, v);
        else tr[p].rc = insert_at(tr[p].rc, v);
        pushup(p);
        if (imbalanced(p)) return rebuild(p);
        return p;
    }
    int erase_at(int p, LL v, bool& removed)
    {
        if (!p) return 0;
        if (v < tr[p].val) tr[p].lc = erase_at(tr[p].lc, v, removed);
        else if (v > tr[p].val) tr[p].rc = erase_at(tr[p].rc, v, removed);
        else
        {
            removed = true;
            if (!tr[p].lc || !tr[p].rc)
            {
                int ret = tr[p].lc + tr[p].rc;
                rub.push_back(p);
                return ret;
            }
            int q = tr[p].rc;
            while (tr[q].lc) q = tr[q].lc;
            tr[p].val = tr[q].val;
            bool dummy = false;
            tr[p].rc = erase_at(tr[p].rc, tr[q].val, dummy);
        }
        pushup(p);
        if (imbalanced(p)) return rebuild(p);
        return p;
    }
};
#endif

/* Usage:
int main()
{
    SGTree s(16);
    for (LL v : {2LL, 2LL, 5LL}) s.insert(v);
    cout << s.get_rank(5) << " " << s.get_kth(2) << "\n"; // 2 2
    s.erase(2);                      // 只删一个 2
    cout << s.get_pre(5) << " " << s.get_suf(2) << "\n"; // 2 5
    s.clear();
    cout << s.size() << "\n"; // 0
}
*/
