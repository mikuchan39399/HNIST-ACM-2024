// zoi: fhqSeq
#ifndef Z_OI_FHQ_SEQ
#define Z_OI_FHQ_SEQ

#include "../../杂项/随机数/z_rnd.cpp"
#include "../../杂项/utils/utils.cpp"

// 隐式 FHQ 按中序位置维护序列, 随机优先级决定树形, 拆出区间后修改或查询再合并
// sum/mx/mn 支持全部操作; 最大子段和只在建树/插删/覆盖/翻转/移动域保证有效, 区间须非空
// 加/乘后不查询受影响数据的最大子段和, 全序列 assign 或重建可恢复, 不溢出须覆盖中间表达式
// build 用 a[1..m], 批量 insert 用整个 vector; insert 的 pos 为新元素的位置, move 的 pos 按剩余序列计
// 每结点 96 B, 预算按峰值存活数, 1e6 约 96 MB; 回收编号数组另预留 4 MB
struct FHQ_Seq
{
    static constexpr LL NEG_INF = LLONG_MIN; // 空子树哨兵(不参与求和)
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
    // 将 x 子树按中序追加到 out
    // 时间: O(子树大小) | 额外空间: O(子树大小), 追加到 out
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
    // 时间: O(m) | 额外空间: O(m), 栈容器按 m 预留
    void build(const VLL& a)
    {
        clear();
        root = build_sub(a, 1, (int)a.size() - 1);
    }
    // 在第 pos 位插入 v (pos ∈ [1, n+1])
    // 时间: 期望 O(log n) | 额外空间: O(log n)
    void insert(int pos, LL v)
    {
        int x, y;
        split_rank(root, pos - 1, x, y);
        root = merge(merge(x, newnode(v)), y);
    }
    // 在第 pos 位批量插入 a[0..m-1]
    // 时间: 期望 O(m + log n) | 额外空间: O(m + log n)
    void insert(int pos, const VLL& a)
    {
        int sub = build_sub(a, 0, (int)a.size() - 1);
        int x, y;
        split_rank(root, pos - 1, x, y);
        root = merge(merge(x, sub), y);
    }
    // 删除区间 [l, r], 结点回收
    // 时间: 期望 O(log n + 区间长) | 额外空间: O(log n)
    void erase(int l, int r)
    {
        int x, y, z;
        split_rank(root, r, x, z);
        split_rank(x, l - 1, x, y);
        recycle(y);
        root = merge(x, z);
    }
    // 返回 [l, r] 区间和
    // 时间: 期望 O(log n) | 额外空间: O(log n)
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
    // 时间: 期望 O(log n) | 额外空间: O(log n)
    void modify(int l, int r, LL d)
    {
        int x, y, z;
        split_rank(root, r, x, z);
        split_rank(x, l - 1, x, y);
        aff(y, 1, d);
        root = merge(merge(x, y), z);
    }
    // [l, r] 整体乘 m (此后最大子段和失效, 见类头语义域; m=0 即覆盖为 0)
    // 时间: 期望 O(log n) | 额外空间: O(log n)
    void mul(int l, int r, LL m)
    {
        int x, y, z;
        split_rank(root, r, x, z);
        split_rank(x, l - 1, x, y);
        aff(y, m, 0);
        root = merge(merge(x, y), z);
    }
    // [l, r] 整体覆盖为 v (恢复最大子段和有效性)
    // 时间: 期望 O(log n) | 额外空间: O(log n)
    void assign(int l, int r, LL v)
    {
        int x, y, z;
        split_rank(root, r, x, z);
        split_rank(x, l - 1, x, y);
        aff(y, 0, v);
        root = merge(merge(x, y), z);
    }
    // [l, r] 区间翻转
    // 时间: 期望 O(log n) | 额外空间: O(log n)
    void reverse(int l, int r)
    {
        int x, y, z;
        split_rank(root, r, x, z);
        split_rank(x, l - 1, x, y);
        rev_tag(y);
        root = merge(merge(x, y), z);
    }
    // 把 [l, r] 切出移到剩余序列前 pos 个元素之后 (pos ∈ [0, n-区间长])
    // 时间: 期望 O(log n) | 额外空间: O(log n)
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
    // 时间: 期望 O(log n) | 额外空间: O(log n)
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
    // 时间: 期望 O(log n) | 额外空间: O(log n)
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
    // 时间: 期望 O(log n) | 额外空间: O(log n)
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
    // 时间: O(n) | 额外空间: O(n), 追加到 out
    void collect(VLL& out) { walk(root, out); }
    // 返回序列长度
    // 时间: O(1) | 空间: O(1)
    int size() { return tr[root].sz; }
    // 多测复位: 清空序列, 容量保留
    // 时间: O(idx) | 空间: O(1)
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
int main()
{
    FHQ_Seq s(16);
    s.build(VLL{0, 1, -2, 3});
    s.insert(2, VLL{4, 5});          // 1, 4, 5, -2, 3
    s.erase(4, 4);                  // 1, 4, 5, 3
    s.reverse(1, 4);                // 3, 5, 4, 1
    s.move_interval(1, 2, 2);       // 4, 1, 3, 5
    cout << s.get_sum(1, 4) << "\n"; // 13
    s.modify(1, 4, 2);
    s.mul(1, 4, -1);                // 此阶段只查和/最值, 不查最大子段和
    cout << s.get_min() << " " << s.get_max() << "\n"; // -7 -3
    s.assign(1, 4, -2);             // 全覆盖恢复最大子段和
    cout << s.get_max_sum() << "\n"; // -2
    int x, y;
    s.split_rank(s.root, 2, x, y);
    VLL out;
    s.walk(x, out);                 // 追加 x 子树内容
    s.root = s.merge(x, y);
    s.clear();
    cout << s.size() << "\n"; // 0
}
*/
