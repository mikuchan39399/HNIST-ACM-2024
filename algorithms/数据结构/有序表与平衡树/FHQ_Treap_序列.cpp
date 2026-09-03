// zoi: fhqSeq
#ifndef Z_OI_FHQ_SEQ
#define Z_OI_FHQ_SEQ

#include <vector>
#include <cassert>
#include <climits>
#include "../../杂项/随机数/z_rnd.cpp"
#include "../../杂项/utils/utils.cpp"

using namespace std;

// ================= FHQ Treap 序列维护 =================
// [复杂度]
// O(n)     : 线性建树
// O(log n) : 区间 插删/加/乘/覆盖/翻转/求和/移动，区间最大子段和/最值
// [操作限制与有效性]
// - 最大子段和：区间加/乘会破坏其正确性，直到该子树再次被覆盖 (assign) 方可恢复。
//               其余操作 (覆盖/翻转/插删/建树) 下恒正确。全负序列返回最大负值。
// - 最值(mx/mn)：对所有操作始终有效。
// - 区间覆盖：底层复用乘法 Tag (即乘 0 视为覆盖)。
// [接口约定]
// - 入参格式：build 传入 1-based 数组 a[1..m]；insert 等其余接口传 0-based vector。
// - insert(pos)：统一为“插入到第 pos 位” (pos ∈ [1, n+1])。
//                若题意为“在第 x 个之后插入”，需传入 x+1。
// - move(l, r, pos)：要求区间 [l, r] 合法，且 pos ∈ [0, n - 区间长]。
// [内存]
// - 每结点 96B; 预算 = 峰值存活结点数(回收复用), 1e6 ≈ 96MB
// =======================================================
struct FHQ_Seq
{
    static constexpr LL NEG_INF = LLONG_MIN / 4; // 空子树哨兵(不参与求和)
    struct node
    {
        int lc = 0, rc = 0, sz = 0, rd = 0;
        LL val = 0, sum = 0;
        LL lmax = NEG_INF, rmax = NEG_INF, tmax = NEG_INF;
        LL mx = LLONG_MIN, mn = LLONG_MAX;
        LL tm = 1, ta = 0;
        bool rev = 0;
    };
    vector<node> tr;
    VI rub;
    int idx, root, budget;
    // 把以 p 为根的子树按位置分裂, 中序前 k 个结点分给 x, 其余分给 y
    // 时间: 期望 O(log n) | 空间: O(log n)
    void split_rank(int p, int k, int& x, int& y)
    {
        if (!p)
        {
            x = y = 0;
            return;
        }
        pushdown(p);
        if (tr[tr[p].lc].sz + 1 <= k)
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
    // 把子树 x 和 y 合并成一棵并返回新根, 要求 x 的结点都在 y 前面(与值无关)
    // 时间: 期望 O(log n) | 空间: O(log n)
    int merge(int x, int y)
    {
        if (!x || !y) return x + y;
        if (tr[x].rd < tr[y].rd)
        {
            pushdown(x);
            tr[x].rc = merge(tr[x].rc, y);
            pushup(x);
            return x;
        }
        pushdown(y);
        tr[y].lc = merge(x, tr[y].lc);
        pushup(y);
        return y;
    }
    // 中序遍历以 x 为根的子树, 把值依次追加到 out (时间 O(子树大小))
    void walk(int x, VLL& out)
    {
        if (!x) return;
        pushdown(x);
        walk(tr[x].lc, out);
        out.push_back(tr[x].val);
        walk(tr[x].rc, out);
    }
    // 构造: 预算 max_nodes 结点(按峰值存活计, 回收复用), 空序列
    // 时间: O(1) | 空间: O(预算)
    FHQ_Seq(int max_nodes = 1000010) : idx(0), root(0), budget(max_nodes)
    {
        tr.reserve(budget + 1);
        rub.reserve(budget);
        tr.push_back(node());
    }
    // 从 a[1..m] 线性建树, 替换现有序列 (a.size() = m + 1)
    // 时间: O(m) | 空间: 期望 O(log m)
    void build(const VLL& a)
    {
        clear();
        root = build_sub(a, 1, (int)a.size() - 1);
    }
    // 在第 pos 位插入 v (pos ∈ [1, n+1])
    // 时间: 期望 O(log n) | 空间: O(1)
    void insert(int pos, LL v)
    {
        int x, y;
        split_rank(root, pos - 1, x, y);
        root = merge(merge(x, newnode(v)), y);
    }
    // 在第 pos 位批量插入 a[0..m-1]
    // 时间: O(m + log n) | 空间: O(1)
    void insert(int pos, const VLL& a)
    {
        int sub = build_sub(a, 0, (int)a.size() - 1);
        int x, y;
        split_rank(root, pos - 1, x, y);
        root = merge(merge(x, sub), y);
    }
    // 删除区间 [l, r], 结点回收
    // 时间: 期望 O(log n + 区间长) | 空间: O(1)
    void erase(int l, int r)
    {
        int x, y, z;
        split_rank(root, r, x, z);
        split_rank(x, l - 1, x, y);
        recycle(y);
        root = merge(x, z);
    }
    // 返回 [l, r] 区间和
    // 时间: 期望 O(log n) | 空间: O(1)
    LL get_sum(int l, int r)
    {
        int x, y, z;
        split_rank(root, r, x, z);
        split_rank(x, l - 1, x, y);
        LL ret = tr[y].sum;
        root = merge(merge(x, y), z);
        return ret;
    }
    // [l, r] 整体加 d (此后最大子段和失效, 见类头语义域)
    // 时间: 期望 O(log n) | 空间: O(1)
    void modify(int l, int r, LL d)
    {
        int x, y, z;
        split_rank(root, r, x, z);
        split_rank(x, l - 1, x, y);
        aff(y, 1, d);
        root = merge(merge(x, y), z);
    }
    // [l, r] 整体乘 m (此后最大子段和失效, 见类头语义域; m=0 即覆盖为 0)
    // 时间: 期望 O(log n) | 空间: O(1)
    void mul(int l, int r, LL m)
    {
        int x, y, z;
        split_rank(root, r, x, z);
        split_rank(x, l - 1, x, y);
        aff(y, m, 0);
        root = merge(merge(x, y), z);
    }
    // [l, r] 整体覆盖为 v (恢复最大子段和有效性)
    // 时间: 期望 O(log n) | 空间: O(1)
    void assign(int l, int r, LL v)
    {
        int x, y, z;
        split_rank(root, r, x, z);
        split_rank(x, l - 1, x, y);
        aff(y, 0, v);
        root = merge(merge(x, y), z);
    }
    // [l, r] 区间翻转
    // 时间: 期望 O(log n) | 空间: O(1)
    void reverse(int l, int r)
    {
        int x, y, z;
        split_rank(root, r, x, z);
        split_rank(x, l - 1, x, y);
        rev_tag(y);
        root = merge(merge(x, y), z);
    }
    // 把 [l, r] 切出移到剩余序列前 pos 个元素之后 (pos ∈ [0, n-区间长])
    // 时间: 期望 O(log n) | 空间: O(1)
    void move_interval(int l, int r, int pos)
    {
        int w, x, y, z;
        split_rank(root, r, w, z);
        split_rank(w, l - 1, w, x);
        y = merge(w, z);
        split_rank(y, pos, w, z);
        root = merge(w, merge(x, z));
    }
    // 返回全序列最大子段和 (非空段, 全负返回最大负值; 空序列无定义)
    // 时间: O(1) | 空间: O(1)
    LL get_max_sum() { return tr[root].tmax; }
    // 返回 [l, r] 最大子段和 (非空段; 语义域见类头)
    // 时间: 期望 O(log n) | 空间: O(1)
    LL get_max_sum(int l, int r)
    {
        int x, y, z;
        split_rank(root, r, x, z);
        split_rank(x, l - 1, x, y);
        LL ret = tr[y].tmax;
        root = merge(merge(x, y), z);
        return ret;
    }
    // 返回全序列最大权值 (对含加/乘在内全部操作恒有效)
    // 时间: O(1) | 空间: O(1)
    LL get_max() { return tr[root].mx; }
    // 返回 [l, r] 最大权值 (对含加/乘在内全部操作恒有效)
    // 时间: 期望 O(log n) | 空间: O(1)
    LL get_max(int l, int r)
    {
        int x, y, z;
        split_rank(root, r, x, z);
        split_rank(x, l - 1, x, y);
        LL ret = tr[y].mx;
        root = merge(merge(x, y), z);
        return ret;
    }
    // 返回全序列最小权值 (对含加/乘在内全部操作恒有效)
    // 时间: O(1) | 空间: O(1)
    LL get_min() { return tr[root].mn; }
    // 返回 [l, r] 最小权值 (对含加/乘在内全部操作恒有效)
    // 时间: 期望 O(log n) | 空间: O(1)
    LL get_min(int l, int r)
    {
        int x, y, z;
        split_rank(root, r, x, z);
        split_rank(x, l - 1, x, y);
        LL ret = tr[y].mn;
        root = merge(merge(x, y), z);
        return ret;
    }
    // 中序收集整个序列, 追加到 out 尾部
    // 时间: O(n) | 空间: O(1)
    void collect(VLL& out) { walk(root, out); }
    // 返回序列长度
    // 时间: O(1) | 空间: O(1)
    int size() { return tr[root].sz; }
    // 多测复位: 清空序列, 容量保留
    // 时间: O(1) | 空间: O(1)
    void clear()
    {
        idx = 0;
        root = 0;
        rub.clear();
        tr.clear();
        tr.push_back(node());
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
        tr[id].sum = tr[id].val = v;
        tr[id].lmax = tr[id].rmax = tr[id].tmax = v;
        tr[id].mx = tr[id].mn = v;
        tr[id].rd = z_rnd(INT_MAX);
        return id;
    }
    void aff(int x, LL m, LL a)
    {
        if (!x) return;
        tr[x].val = tr[x].val * m + a;
        tr[x].sum = tr[x].sum * m + a * tr[x].sz;
        LL hi = tr[x].mx * m + a, lo = tr[x].mn * m + a;
        tr[x].mx = max(hi, lo);
        tr[x].mn = min(hi, lo);
        if (m == 0)
            tr[x].lmax = tr[x].rmax = tr[x].tmax = a > 0 ? a * tr[x].sz : a;
        tr[x].ta = tr[x].ta * m + a;
        tr[x].tm = tr[x].tm * m;
    }
    void rev_tag(int x)
    {
        if (!x) return;
        swap(tr[x].lc, tr[x].rc);
        swap(tr[x].lmax, tr[x].rmax);
        tr[x].rev ^= 1;
    }
    void pushdown(int x)
    {
        if (tr[x].rev)
        {
            rev_tag(tr[x].lc);
            rev_tag(tr[x].rc);
            tr[x].rev = 0;
        }
        if (tr[x].tm != 1 || tr[x].ta != 0)
        {
            aff(tr[x].lc, tr[x].tm, tr[x].ta);
            aff(tr[x].rc, tr[x].tm, tr[x].ta);
            tr[x].tm = 1;
            tr[x].ta = 0;
        }
    }
    void pushup(int x)
    {
        node& u = tr[x];
        node& L = tr[u.lc];
        node& R = tr[u.rc];
        u.sz = L.sz + R.sz + 1;
        u.sum = L.sum + R.sum + u.val;
        u.mx = max(max(L.mx, R.mx), u.val);
        u.mn = min(min(L.mn, R.mn), u.val);
        u.lmax = max(L.lmax, L.sum + u.val + max(0LL, R.lmax));
        u.rmax = max(R.rmax, R.sum + u.val + max(0LL, L.rmax));
        u.tmax = max(max(L.tmax, R.tmax), max(0LL, L.rmax) + u.val + max(0LL, R.lmax));
    }
    void finish(int x)
    {
        if (!x) return;
        finish(tr[x].lc);
        finish(tr[x].rc);
        pushup(x);
    }
    int build_sub(const VLL& a, int lo, int hi)
    {
        int m = hi - lo + 1;
        VI stk;
        stk.reserve(m + 1);
        for (int i = lo; i <= hi; i++)
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
        int r = stk.empty() ? 0 : stk[0];
        finish(r);
        return r;
    }
    void recycle(int x)
    {
        if (!x) return;
        recycle(tr[x].lc);
        recycle(tr[x].rc);
        rub.push_back(x);
    }
};
#endif
/* Usage:
    FHQ_Seq s;                     // 默认预算 1e6 结点(峰值存活计)
    s.build(a);                    // a[1..m] 线性建树
    s.insert(pos, v);              // 第 pos 位插入 v
    s.insert(pos, a);              // 批量插入 a[0..m-1] (整个 vector)
    s.erase(l, r);                 // 删除区间 [l, r], 结点回收
    s.get_sum(l, r);               // 区间和
    s.modify(l, r, d);             // 区间加 d (maxsum 失效)
    s.mul(l, r, m);                // 区间乘 m (maxsum 失效)
    s.assign(l, r, v);             // 区间覆盖 v (maxsum 恢复)
    s.reverse(l, r);               // 区间翻转
    s.move_interval(l, r, pos);    // [l,r] 移到剩余前 pos 个之后
    s.get_max_sum();               // 全局最大子段和(非空段)
    s.get_max_sum(l, r);           // 区间最大子段和
    s.get_max();                   // 全局最大权值(全域有效)
    s.get_max(l, r);               // 区间最大权值
    s.get_min();                   // 全局最小权值(全域有效)
    s.get_min(l, r);               // 区间最小权值
    s.collect(out);                // O(n) 全序收集(追加到 out)
    s.size();                      // 序列长度
    s.clear();                     // 多测复位, 容量保留
    // 底层接口(结点句柄进出, 直接可用):
    // int a, b; s.split_rank(p, k, a, b); // 分裂 p: 前 k 个给 a, 其余给 b
    // p = s.merge(a, b);                  // 合并, 要求 a 都在 b 前面
    // s.walk(p, out);                     // 中序收集 p 子树, 追加到 out
*/
