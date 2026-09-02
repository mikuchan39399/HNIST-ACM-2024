// ============ seg_check SegTree/DySegTree 回归套件 ============
// 覆盖: SegTree(区间加/单点/双通道查询/find_first/last 二分+覆盖 Tag 变体
//       含 {0,false} 空标记 no-op, static 实例 init 多测复用) | DySegTree
//       (build 常规域 + 1e9 大值域稀疏域不 build, 虚点 len/零值语义, map 暴力)
// 纪律: 改动 泛型线段树/泛型动态线段树 模板, 必重跑本套件
// 跑法: g++ -std=c++20 -Wall -Wextra -O2 seg_check.cpp -o seg_check && ./seg_check
#include <iostream>
#include <vector>
#include <cassert>
#include <algorithm>
#include <climits>
#include <map>
#include <random>
#include "../泛型动态线段树.cpp"
#include "../泛型线段树.cpp"
#include "../../ST表/ST表.cpp"

using namespace std;
using LL = long long;

// 注: C++ 局部类不能定义 friend operator+, Info/Tag 须放函数外
// 组 1: 区间加, 维护 sum/mx 双查询通道
struct TagS
{
    LL add = 0;
    void apply(const TagS& t) { add += t.add; }
    void clear() { add = 0; }
    bool has_tag() const { return add != 0; }
};
struct InfoS
{
    LL len = 0, sum = 0, mx = 0;
    bool break_cond(const TagS&) const { return false; }
    bool tag_cond(const TagS&) const { return true; }
    void apply(const TagS& t) { sum += t.add * len; mx += t.add; }
    friend InfoS operator+(const InfoS& a, const InfoS& b)
    {
        if (a.len == 0) return b;
        if (b.len == 0) return a;
        InfoS r;
        r.len = a.len + b.len;
        r.sum = a.sum + b.sum;
        r.mx = max(a.mx, b.mx);
        return r;
    }
};

// 组 2: 区间覆盖 (assign 型 Tag, 验证 Info/Tag 代数解耦; has=false 空标记)
struct TagC
{
    LL v = 0;
    bool has = false;
    void apply(const TagC& t)
    {
        if (t.has)
        {
            v = t.v;
            has = true;
        }
    }
    void clear() { v = 0; has = false; }
    bool has_tag() const { return has; }
};
struct InfoC
{
    LL len = 0, sum = 0, mx = 0;
    bool break_cond(const TagC&) const { return false; }
    bool tag_cond(const TagC&) const { return true; }
    void apply(const TagC& t)
    {
        if (t.has) { sum = t.v * len; mx = t.v; }
    }
    friend InfoC operator+(const InfoC& a, const InfoC& b)
    {
        if (a.len == 0) return b;
        if (b.len == 0) return a;
        InfoC r;
        r.len = a.len + b.len;
        r.sum = a.sum + b.sum;
        r.mx = max(a.mx, b.mx);
        return r;
    }
};

void test_seg_tree()
{
    mt19937 rng(1926);
    // 模式 A+B: 区间加/单点/查询/二分, static 实例跨组 init 复用
    static SegTree<InfoS, TagS> seg(70);
    for (int tc = 0; tc < 300; tc++)
    {
        int n = 1 + rng() % 60;
        vector<InfoS> a(n + 1);
        vector<LL> ref(n + 1);
        for (int i = 1; i <= n; i++)
        {
            LL v = (LL)(rng() % 41) - 20;
            a[i] = {1, v, v};
            ref[i] = v;
        }
        seg.init(n);
        seg.build(a);
        for (int t = 0, ops = 100; t < ops; t++)
        {
            int op = rng() % 5;
            int l = 1 + rng() % n, r = 1 + rng() % n;
            if (l > r) swap(l, r);
            if (op == 0)
            {
                LL d = (LL)(rng() % 11) - 5;
                seg.modify(l, r, {d});
                for (int i = l; i <= r; i++) ref[i] += d;
            }
            else if (op == 1)
            {
                LL d = (LL)(rng() % 11) - 5;
                seg.modify(l, l, {d});
                ref[l] += d;
            }
            else if (op == 2)
            {
                LL s = 0, m = LLONG_MIN;
                for (int i = l; i <= r; i++)
                {
                    s += ref[i];
                    m = max(m, ref[i]);
                }
                InfoS res = seg.query(l, r);
                assert(res.sum == s && res.mx == m);
            }
            else if (op == 3)
            {
                LL x = (LL)(rng() % 41) - 20;
                int exp = -1;
                for (int i = l; i <= n; i++)
                    if (ref[i] > x) { exp = i; break; }
                assert(seg.find_first(l, [x](const InfoS& in) { return in.mx > x; }) == exp);
            }
            else
            {
                LL x = (LL)(rng() % 41) - 20;
                int exp = -1;
                for (int i = r; i >= 1; i--)
                    if (ref[i] > x) { exp = i; break; }
                assert(seg.find_last(r, [x](const InfoS& in) { return in.mx > x; }) == exp);
            }
        }
        LL s = 0, m = LLONG_MIN;
        for (int i = 1; i <= n; i++)
        {
            s += ref[i];
            m = max(m, ref[i]);
        }
        InfoS res = seg.query(1, n);
        assert(res.len == n && res.sum == s && res.mx == m);
    }
    // 模式 C: 覆盖型 Tag + 空标记 ({0,false} 必须是 no-op)
    for (int tc = 0; tc < 150; tc++)
    {
        int n = 1 + rng() % 60;
        SegTree<InfoC, TagC> seg(n);
        vector<InfoC> a(n + 1);
        vector<LL> ref(n + 1);
        for (int i = 1; i <= n; i++)
        {
            LL v = (LL)(rng() % 21) - 10;
            a[i] = {1, v, v};
            ref[i] = v;
        }
        seg.init(n);
        seg.build(a);
        for (int t = 0, ops = 60; t < ops; t++)
        {
            int l = 1 + rng() % n, r = 1 + rng() % n;
            if (l > r) swap(l, r);
            int op = rng() % 3;
            if (op == 0)
            {
                LL v = (LL)(rng() % 21) - 10;
                seg.modify(l, r, {v, true});
                for (int i = l; i <= r; i++) ref[i] = v;
            }
            else if (op == 1)
            {
                seg.modify(l, r, {0, false});      // 空标记 no-op
            }
            else
            {
                LL s = 0, m = LLONG_MIN;
                for (int i = l; i <= r; i++)
                {
                    s += ref[i];
                    m = max(m, ref[i]);
                }
                InfoC res = seg.query(l, r);
                assert(res.sum == s && res.mx == m);
            }
        }
    }
}

void test_dy_seg_tree()
{
    mt19937 rng(810);
    // 模式 A: build 常规域
    for (int tc = 0; tc < 300; tc++)
    {
        int n = 1 + rng() % 60;
        vector<InfoS> a(n + 1);
        vector<LL> ref(n + 1);
        for (int i = 1; i <= n; i++)
        {
            LL v = (LL)(rng() % 41) - 20;
            a[i] = {1, v, v};
            ref[i] = v;
        }
        DySegTree<InfoS, TagS> s(n, 100000);
        s.build(a);
        for (int t = 0, ops = 80; t < ops; t++)
        {
            int op = rng() % 5;
            int l = 1 + rng() % n, r = 1 + rng() % n;
            if (l > r) swap(l, r);
            if (op == 0)
            {
                LL d = (LL)(rng() % 11) - 5;
                s.modify(l, r, {d});
                for (int i = l; i <= r; i++) ref[i] += d;
            }
            else if (op == 1)
            {
                LL d = (LL)(rng() % 11) - 5;
                s.modify(l, l, {d});
                ref[l] += d;
            }
            else if (op == 2)
            {
                LL sm = 0, m = LLONG_MIN;
                for (int i = l; i <= r; i++)
                {
                    sm += ref[i];
                    m = max(m, ref[i]);
                }
                InfoS res = s.query(l, r);
                assert(res.len == r - l + 1 && res.sum == sm && res.mx == m);
            }
            else if (op == 3)
            {
                LL x = (LL)(rng() % 41) - 20;
                LL exp = -1;
                for (int i = l; i <= n; i++)
                    if (ref[i] > x) { exp = i; break; }
                assert(s.find_first(l, [x](const InfoS& in) { return in.mx > x; }) == exp);
            }
            else
            {
                LL x = (LL)(rng() % 41) - 20;
                LL exp = -1;
                for (int i = r; i >= 1; i--)
                    if (ref[i] > x) { exp = i; break; }
                assert(s.find_last(r, [x](const InfoS& in) { return in.mx > x; }) == exp);
            }
        }
        LL sm = 0, m = LLONG_MIN;
        for (int i = 1; i <= n; i++)
        {
            sm += ref[i];
            m = max(m, ref[i]);
        }
        InfoS res = s.query(1, n);
        assert(res.len == n && res.sum == sm && res.mx == m);
    }
    // 模式 B: 1e9 大值域稀疏域, 不 build; 活动小窗 [1,200] + 远点, map 暴力
    // 语义: 未触碰下标恒为 0 (虚点零值), find 的 pred 取 x >= 0 仅实点可命中
    for (int tc = 0; tc < 300; tc++)
    {
        const LL N = 1000000000;
        DySegTree<InfoS, TagS> s(N, 100000);
        map<LL, LL> mp;
        vector<LL> far(5);
        for (auto& f : far) f = 201 + rng() % (N - 200);
        for (int t = 0, ops = 80; t < ops; t++)
        {
            int op = rng() % 4;
            if (op == 0) // 窗内区间加
            {
                int l = 1 + rng() % 200, r = 1 + rng() % 200;
                if (l > r) swap(l, r);
                LL d = (LL)(rng() % 11) - 5;
                s.modify(l, r, {d});
                for (int i = l; i <= r; i++) mp[i] += d;
            }
            else if (op == 1) // 单点: 窗内或远点
            {
                LL x = rng() % 2 ? 1 + rng() % 200 : far[rng() % 5];
                LL d = (LL)(rng() % 11) - 5;
                s.modify(x, x, {d});
                mp[x] += d;
            }
            else if (op == 2) // 区间查: 窗内 or 跨远点, 虚点 len/零值推导
            {
                LL l, r;
                if (rng() % 2)
                {
                    l = 1 + rng() % 200;
                    r = 1 + rng() % 200;
                    if (l > r) swap(l, r);
                }
                else
                {
                    l = 1;
                    r = far[rng() % 5];   // 全窗 + 大段虚点
                }
                LL sm = 0, m;
                if (r <= 200)
                {
                    m = LLONG_MIN;
                    for (LL k = l; k <= r; k++)   // 精确逐位: 未触碰位值 0
                    {
                        LL v = mp.count(k) ? mp[k] : 0;
                        sm += v;
                        m = max(m, v);
                    }
                }
                else
                {
                    m = 0;   // 范围含海量未触碰位(值 0)
                    for (auto& [k, v] : mp)
                    {
                        if (k >= l && k <= r)
                        {
                            sm += v;
                            m = max(m, v);
                        }
                    }
                }
                InfoS res = s.query(l, r);
                assert(res.len == r - l + 1 && res.sum == sm && res.mx == m);
            }
            else // find_first(start): x ∈ [0,2], 虚点(0)不满足
            {
                LL start = 1 + rng() % 200;
                LL x = rng() % 3;
                LL exp = -1;
                for (auto& [k, v] : mp)
                    if (k >= start && v > x) { exp = k; break; }
                assert(s.find_first(start, [x](const InfoS& in) { return in.mx > x; }) == exp);
            }
        }
        // 全值域尾对账
        LL sm = 0, m = 0;
        for (auto& [k, v] : mp)
        {
            sm += v;
            m = max(m, v);
        }
        InfoS res = s.query(1, N);
        assert(res.len == N && res.sum == sm && res.mx == m);
    }
}

void test_st_table()
{
    mt19937 rng(20260901);
    for (int tc = 0; tc < 200; tc++)
    {
        int n = 1 + rng() % 80;
        vector<LL> a(n + 1);
        for (int j = 1; j <= n; j++) a[j] = (LL)(rng() % 201) - 100;
        for (int mode = 0; mode < 2; mode++)
        {
            ST st(1);              // 故意开小, 验自动扩容
            st.build(a, mode == 0);
            int q = 30;
            while (q--)
            {
                int l = 1 + rng() % n, r = l + rng() % (n - l + 1);
                LL ref = a[l];
                for (int j = l; j <= r; j++) ref = mode == 0 ? max(ref, a[j]) : min(ref, a[j]);
                assert(st.query(l, r) == ref);
            }
            assert(st.query(1, 1) == a[1] && st.query(n, n) == a[n]);
            assert(st.query(1, n) == (mode == 0 ? *max_element(a.begin() + 1, a.end())
                                                : *min_element(a.begin() + 1, a.end())));
        }
    }
}

int main()
{
    test_seg_tree();
    test_dy_seg_tree();
    test_st_table();
    cout << "seg_check passed: SegTree / DySegTree / ST all tests ok\n";
    return 0;
}