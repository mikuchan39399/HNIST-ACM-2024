// zoi: treap
#ifndef Z_OI_TREAP
#define Z_OI_TREAP

#include <vector>
#include <cassert>
#include <climits>
#include "../../杂项/随机数/z_rnd.cpp"
#include "../../杂项/utils/utils.cpp"

using namespace std;

// ============ Treap 旋转平衡树 (可重复集合) ============
// 升序维护 LL 集合, 插/删/排名/第k小/前驱/后继期望 O(log n)
// 值域约定: 元素取值在 (-INF, INF) 内, 前驱/后继无解返回 ∓INF, 第k小越界返回 INF
// 内存账: 每结点 32B, 默认预算 4000010 结点(按累计插入计, 删除不回收)
//   ≈ 128MB, 超预算插入触发 assert
struct Treap
{
    static constexpr LL INF = 0x3f3f3f3f3f3f3f3f;
    struct node
    {
        int lc = 0, rc = 0, cnt = 0, sz = 0, rd = 0;
        LL val = 0;
    };
private:
    vector<node> tr;
    int idx, root, budget;
    int newnode(LL v)
    {
        assert(idx < budget);
        tr.push_back(node());
        idx++;
        tr[idx].cnt = tr[idx].sz = 1;
        tr[idx].val = v;
        tr[idx].rd = z_rnd(INT_MAX);
        return idx;
    }
    void pushup(int x)
    {
        if (!x) return;
        tr[x].sz = tr[tr[x].lc].sz + tr[tr[x].rc].sz + tr[x].cnt;
    }
    void finish(int x)
    {
        if (!x) return;
        finish(tr[x].lc);
        finish(tr[x].rc);
        pushup(x);
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
    void insert(int& x, LL v)
    {
        if (!x)
        {
            x = newnode(v);
            return;
        }
        if (tr[x].val == v) tr[x].cnt++;
        else if (tr[x].val > v)
        {
            insert(tr[x].lc, v);
            if (tr[tr[x].lc].rd < tr[x].rd) ro_right(x);
        }
        else
        {
            insert(tr[x].rc, v);
            if (tr[tr[x].rc].rd < tr[x].rd) ro_left(x);
        }
        pushup(x);
    }
    void erase(int& x, LL v)
    {
        if (!x) return;
        if (tr[x].val == v)
        {
            if (tr[x].cnt > 1) tr[x].cnt--;
            else
            {
                if (!tr[x].lc || !tr[x].rc) x = tr[x].lc + tr[x].rc;
                else if (tr[tr[x].lc].rd < tr[tr[x].rc].rd)
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
        else v < tr[x].val ? erase(tr[x].lc, v) : erase(tr[x].rc, v);
        pushup(x);
    }
    int rank_of(int x, LL v)
    {
        if (!x) return 0;
        if (tr[x].val >= v) return rank_of(tr[x].lc, v);
        return tr[x].cnt + tr[tr[x].lc].sz + rank_of(tr[x].rc, v);
    }
    LL kth_of(int x, int k)
    {
        if (tr[tr[x].lc].sz >= k) return kth_of(tr[x].lc, k);
        if (tr[tr[x].lc].sz + tr[x].cnt >= k) return tr[x].val;
        return kth_of(tr[x].rc, k - tr[tr[x].lc].sz - tr[x].cnt);
    }
    LL pre_of(int x, LL v)
    {
        if (!x) return -INF;
        if (tr[x].val >= v) return pre_of(tr[x].lc, v);
        return max(tr[x].val, pre_of(tr[x].rc, v));
    }
    LL suf_of(int x, LL v)
    {
        if (!x) return INF;
        if (tr[x].val <= v) return suf_of(tr[x].rc, v);
        return min(tr[x].val, suf_of(tr[x].lc, v));
    }
public:
    // 构造: 预算 max_nodes 结点(按累计插入计, 删除不回收), 哨兵 0 号就位
    // 时间: O(1) | 空间: O(预算) (账目见类头)
    Treap(int max_nodes = 4000010) : idx(0), root(0), budget(max_nodes)
    {
        tr.reserve(budget + 1);
        tr.push_back(node());
    }
    // 从升序 a[1..m] 笛卡尔树(右脊栈)线性建树, 替换现有集合 (a.size() = m + 1)
    // 契约: a[1..m] 已升序(允许重复, 相邻等值合并进 cnt), 违约触发 assert
    // 时间: O(m) | 空间: 右脊栈, 期望 O(log m)
    void build(const VLL& a)
    {
        clear();
        int m = (int)a.size() - 1;
        for (int i = 2; i <= m; i++) assert(a[i - 1] <= a[i]);
        VI stk;
        stk.reserve(m + 1);
        int last_node = 0;
        for (int i = 1; i <= m; i++)
        {
            if (last_node && tr[last_node].val == a[i])
            {
                tr[last_node].cnt++;
                continue;
            }
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
            last_node = cur;
        }
        root = stk.empty() ? 0 : stk[0];
        finish(root);
    }
    // 插入 v (允许重复)
    // 时间: 期望 O(log n) | 空间: O(1)
    void insert(LL v) { insert(root, v); }
    // 删除一个 v, 返回是否存在并删除
    // 时间: 期望 O(log n) | 空间: O(1)
    bool erase(LL v)
    {
        int b = tr[root].sz;
        erase(root, v);
        return tr[root].sz < b;
    }
    // 返回 < v 的元素个数 (含重复)
    // 时间: 期望 O(log n) | 空间: O(1)
    int get_rank(LL v) { return rank_of(root, v); }
    // 返回第 k 小 (1-based 含重复), k 越界返回 INF
    // 时间: 期望 O(log n) | 空间: O(1)
    LL get_kth(int k)
    {
        if (k < 1 || k > tr[root].sz) return INF;
        return kth_of(root, k);
    }
    // 返回 < v 的最大值, 无前驱返回 -INF
    // 时间: 期望 O(log n) | 空间: O(1)
    LL get_pre(LL v) { return pre_of(root, v); }
    // 返回 > v 的最小值, 无后继返回 INF
    // 时间: 期望 O(log n) | 空间: O(1)
    LL get_suf(LL v) { return suf_of(root, v); }
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
};
#endif
/* Usage:
    Treap trp;                     // 默认预算 4e6 结点
    trp.build(a);                  // 升序 a[1..m] 线性建树(替换现有)
    trp.insert(x);                 // 允许重复
    trp.erase(x);                  // 删一个, 返回 bool
    trp.get_rank(x);               // <x 的元素个数(含重复)
    trp.get_kth(k);                // 第 k 小, 越界返回 INF
    trp.get_pre(x);                // 严格前驱, 无则 -INF
    trp.get_suf(x);                // 严格后继, 无则 INF
    trp.size();                    // 元素个数(含重复)
    trp.clear();                   // 多测复位, 容量保留
*/
