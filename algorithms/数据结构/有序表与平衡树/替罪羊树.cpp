// zoi: scapegoat
#ifndef Z_OI_SGT
#define Z_OI_SGT

#include <vector>
#include <cassert>
#include <climits>
#include "../../杂项/utils/utils.cpp"

using namespace std;

// ============ SGTree 替罪羊树 (可重复集合) ============
// 升序维护 LL 集合, 插/删/排名/第k小/前驱/后继均摊 O(log n)
// 值域约定: 元素取值在 (-INF, INF) 内, 前驱/后继无解返回 ∓INF, 第k小越界返回 INF
// 平衡机制: α = 0.75, 条件 4*max(lc.sz, rc.sz) > 3*sz 触发子树原位重建
//   (插入/删除回溯均检查, 删小侧同样可触发); 重建 = 中序拍平 + 完美平衡二分,
//   原位复用结点 id; 高度恒 ≤ log(n)/log(4/3), 递归栈深安全
// 内存账: 每结点 24B, 重建/删除均回收 id (free 池), 预算按峰值存活计,
//   默认 1000010 ≈ 24MB, 超预算插入触发 assert
struct SGTree
{
    static constexpr LL INF = 0x3f3f3f3f3f3f3f3f;
    struct node
    {
        int lc = 0, rc = 0, sz = 0;
        LL val = 0;
    };
private:
    vector<node> tr;
    VI seq;
    VI rub;
    int idx, root, budget;
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
            int q = tr[p].rc;               // 两子: 后继值顶替, 右子树删后继
            while (tr[q].lc) q = tr[q].lc;
            tr[p].val = tr[q].val;
            bool dummy = false;
            tr[p].rc = erase_at(tr[p].rc, tr[q].val, dummy);
        }
        pushup(p);
        if (imbalanced(p)) return rebuild(p);
        return p;
    }
public:
    // 构造: 预算 max_nodes 结点(按峰值存活计), 哨兵 0 号就位
    // 时间: O(1) | 空间: O(预算) (账目见类头)
    SGTree(int max_nodes = 1000010) : idx(0), root(0), budget(max_nodes)
    {
        tr.reserve(budget + 1);
        tr.push_back(node());
        rub.reserve(budget);
    }
    // 插入 v (允许重复)
    // 时间: 均摊 O(log n) | 空间: O(1)
    void insert(LL v) { root = insert_at(root, v); }
    // 删除一个 v, 返回是否存在并删除
    // 时间: 均摊 O(log n) | 空间: O(1)
    bool erase(LL v)
    {
        bool ok = false;
        root = erase_at(root, v, ok);
        return ok;
    }
    // 返回 < v 的元素个数 (含重复)
    // 时间: O(log n) | 空间: O(1)
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
    // 返回第 k 小 (1-based 含重复), k 越界返回 INF
    // 时间: O(log n) | 空间: O(1)
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
    // 返回 < v 的最大值 (严格前驱), 无解返回 -INF
    // 时间: O(log n) | 空间: O(1)
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
    // 返回 > v 的最小值 (严格后继), 无解返回 INF
    // 时间: O(log n) | 空间: O(1)
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
    // 返回元素个数 (含重复)
    // 时间: O(1) | 空间: O(1)
    int size() { return tr[root].sz; }
    // 多测复位
    // 时间: O(1) | 空间: O(1)
    void clear()
    {
        idx = 0;
        root = 0;
        tr.clear();
        tr.push_back(node());
        seq.clear();
        rub.clear();
    }
};
#endif
/* Usage:
    SGTree sgt;                    // 默认预算 1e6 结点(峰值存活计)
    sgt.insert(x);                 // 允许重复
    sgt.erase(x);                  // 删一个, 返回 bool
    sgt.get_rank(x);               // <x 的元素个数(含重复)
    sgt.get_kth(k);                // 第 k 小, 越界返回 INF
    sgt.get_pre(x);                // 严格前驱, 无则 -INF
    sgt.get_suf(x);                // 严格后继, 无则 INF
    sgt.size();                    // 元素个数(含重复)
    sgt.clear();                   // 多测复位, 容量保留
*/
