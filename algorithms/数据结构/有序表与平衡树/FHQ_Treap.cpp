// zoi: fhq
#ifndef Z_OI_FHQ
#define Z_OI_FHQ

#include <vector>
#include <cassert>
#include <climits>
#include "../../杂项/随机数/z_rnd.cpp"
#include "../../杂项/utils/utils.cpp"

using namespace std;

// ============ FHQ_Treap (可重复集合) ============
// 升序维护 LL 集合, 插/删/排名/第k小/前驱/后继期望 O(log n)
// 值域约定: 元素取值在 (-INF, INF) 内, 前驱/后继无解返回 ∓INF, 第k小越界返回 INF
// 内存: 每结点 32B; 预算 = 总插入次数(删除不回收), 4e6 结点 ≈ 128MB
struct FHQ_Treap
{
    static constexpr LL INF = 0x3f3f3f3f3f3f3f3f;
    struct node
    {
        int lc = 0, rc = 0, sz = 0, rd = 0;
        LL val = 0;
    };
    vector<node> tr;
    int idx, root, budget;
    // 把以 p 为根的子树按值分裂, 值 <= v 的结点分给 x, 值 > v 的分给 y
    // 时间: 期望 O(log n) | 空间: O(log n)
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
    // 把子树 x 和 y 合并成一棵并返回新根, 要求 x 中所有值 <= y 中所有值
    // 时间: 期望 O(log n) | 空间: O(log n)
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
    // 返回以 x 为根的子树中第 k 小的值 (时间期望 O(log n))
    LL kth_of(int x, int k)
    {
        if (tr[tr[x].lc].sz >= k) return kth_of(tr[x].lc, k);
        if (tr[tr[x].lc].sz + 1 == k) return tr[x].val;
        return kth_of(tr[x].rc, k - tr[tr[x].lc].sz - 1);
    }
    // 构造: 预算 max_nodes 结点(按累计插入计, 删除不回收), 哨兵 0 号就位
    // 时间: O(1) | 空间: O(预算)
    FHQ_Treap(int max_nodes = 4000010) : idx(0), root(0), budget(max_nodes)
    {
        tr.reserve(budget + 1);
        tr.push_back(node());
    }
    // 从升序 a[1..m] 笛卡尔树(右脊栈)线性建树, 替换现有集合 (a.size() = m + 1)
    // 契约: a[1..m] 已升序(允许重复), 违约触发 assert
    // 时间: O(m) | 空间: 右脊栈, 期望 O(log m)
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
    // 插入 v (允许重复)
    // 时间: 期望 O(log n) | 空间: O(1)
    void insert(LL v)
    {
        int x, y;
        split(root, v, x, y);
        root = merge(merge(x, newnode(v)), y);
    }
    // 删除一个 v, 返回是否存在并删除
    // 时间: 期望 O(log n) | 空间: O(1)
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
    // 返回 < v 的元素个数 (含重复)
    // 时间: 期望 O(log n) | 空间: O(1)
    int get_rank(LL v)
    {
        int x, y;
        split(root, v - 1, x, y);
        int ret = tr[x].sz;
        root = merge(x, y);
        return ret;
    }
    // 返回第 k 小 (1-based 含重复), k 越界返回 INF
    // 时间: 期望 O(log n) | 空间: O(1)
    LL get_kth(int k)
    {
        if (k < 1 || k > tr[root].sz) return INF;
        return kth_of(root, k);
    }
    // 返回 < v 的最大值, 无前驱返回 -INF
    // 时间: 期望 O(log n) | 空间: O(1)
    LL get_pre(LL v)
    {
        int x, y;
        split(root, v - 1, x, y);
        LL ret = x ? kth_of(x, tr[x].sz) : -INF;
        root = merge(x, y);
        return ret;
    }
    // 返回 > v 的最小值, 无后继返回 INF
    // 时间: 期望 O(log n) | 空间: O(1)
    LL get_suf(LL v)
    {
        int x, y;
        split(root, v, x, y);
        LL ret = y ? kth_of(y, 1) : INF;
        root = merge(x, y);
        return ret;
    }
    // 返回元素个数 (含重复)
    // 时间: O(1) | 空间: O(1)
    int size() { return tr[root].sz; }
    // 多测复位: 清全部元素, 容量保留
    // 时间: O(1) | 空间: O(1)
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
    FHQ_Treap fhq;                 // 默认预算 4e6 结点
    fhq.build(a);                  // 升序 a[1..m] 线性建树(替换现有)
    fhq.insert(x);                 // 允许重复
    fhq.erase(x);                  // 删一个, 返回 bool
    fhq.get_rank(x);               // <x 的元素个数(含重复)
    fhq.get_kth(k);                // 第 k 小, 越界返回 INF
    fhq.get_pre(x);                // 严格前驱, 无则 -INF
    fhq.get_suf(x);                // 严格后继, 无则 INF
    fhq.size();                    // 元素个数(含重复)
    fhq.clear();                   // 多测复位, 容量保留
    // 底层接口(结点句柄进出, 直接可用):
    // int a, b; fhq.split(p, v, a, b);  // 分裂 p: <= v 给 a, > v 给 b
    // p = fhq.merge(a, b);              // 合并, 要求 a 的值都 <= b 的值
    // LL ret = fhq.kth_of(p, k);        // 查 p 子树第 k 小(配 split 句柄用)
*/
