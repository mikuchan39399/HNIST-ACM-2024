// 七组真实插件: 独立数组暴力、标记顺序、双引擎及默认 20 万规模; 不复制待测代数
#include "../泛型线段树.cpp"
#include "../泛型动态线段树.cpp"
#include "../泛型插件/区间加区间和.cpp"
#include "../泛型插件/区间赋值加法与最值.cpp"
#include "../泛型插件/区间仿射与平方和.cpp"
#include "../泛型插件/区间异或与按位计数.cpp"
#include "../泛型插件/区间01赋值翻转与连续段.cpp"
#include "../泛型插件/区间赋值与最大子段和.cpp"
#include "../泛型插件/区间等差加法与区间和.cpp"
#include "../泛型插件/区间加区间和.cpp" // 同时 include 与重复 include
#include <random>

constexpr LL MOD_TEST = 2147483647; // 模数上界, 独立暴力用 128 位
using Infos = tuple<SegAdd::Info, SegAssignAdd::Info, SegAffine::Info<MOD_TEST>,
    SegXor::Info<31>, SegBinary::Info, SegMaxSubarray::Info, SegLinear::Info>;
using Tags = tuple<SegAdd::Tag, SegAssignAdd::Tag, SegAffine::Tag<MOD_TEST>,
    SegXor::Tag, SegBinary::Tag, SegMaxSubarray::Tag, SegLinear::Tag>;
template<int K> using I = tuple_element_t<K, Infos>;
template<int K> using T = tuple_element_t<K, Tags>;
LL norm(__int128 v) { return LL((v % MOD_TEST + MOD_TEST) % MOD_TEST); }
template<int K> I<K> leaf(LL v, int p)
{
    if constexpr (K == 6) return I<K>(v, p);
    else return I<K>(v);
}
template<int K> void verify(const I<K>& got, const vector<LL>& a, int l, int r)
{
    assert(got.len == r - l + 1);
    LL sum = 0, mn = a[l], mx = a[l];
    for (int i = l; i <= r; i++) { sum += a[i]; mn = min(mn, a[i]); mx = max(mx, a[i]); }
    if constexpr (K == 0 || K == 1) assert(got.sum == sum && got.mn == mn && got.mx == mx);
    if constexpr (K == 2)
    {
        __int128 sq = 0;
        for (int i = l; i <= r; i++) sq += (__int128)a[i] * a[i];
        assert(got.sum == norm(sum) && got.sq == norm(sq));
    }
    if constexpr (K == 3)
    {
        unsigned xr = 0;
        array<int, 31> cnt{};
        for (int i = l; i <= r; i++)
        {
            xr ^= unsigned(a[i]);
            for (int b = 0; b < 31; b++) cnt[b] += (a[i] >> b) & 1;
        }
        assert(got.sum == sum && got.xr == xr && got.cnt == cnt);
    }
    if constexpr (K == 4)
    {
        assert(got.ones == sum);
        for (int b = 0; b < 2; b++)
        {
            int pre = 0, suf = 0, best = 0, run = 0;
            for (int i = l; i <= r && a[i] == b; i++) pre++;
            for (int i = r; i >= l && a[i] == b; i--) suf++;
            for (int i = l; i <= r; i++) { run = a[i] == b ? run + 1 : 0; best = max(best, run); }
            assert(got.pre[b] == pre && got.suf[b] == suf && got.best[b] == best);
        }
    }
    if constexpr (K == 5)
    {
        LL pre = a[l], suf = a[r], best = a[l], cur = 0;
        for (int i = l; i <= r; i++) { cur += a[i]; pre = max(pre, cur); }
        cur = 0;
        for (int i = r; i >= l; i--) { cur += a[i]; suf = max(suf, cur); }
        // 小数据枚举每个子段, 大数据单独用线性扫描计算 best。
        if (r - l < 80)
            for (int i = l; i <= r; i++)
            {
                cur = 0;
                for (int j = i; j <= r; j++) { cur += a[j]; best = max(best, cur); }
            }
        else
        {
            LL prefix = 0, low = 0;
            for (int i = l; i <= r; i++) { prefix += a[i]; best = max(best, prefix - low); low = min(low, prefix); }
        }
        assert(got.sum == sum && got.pre == pre && got.suf == suf && got.best == best);
    }
    if constexpr (K == 6)
        assert(got.sum == sum && got.index_sum == (LL(l) + r) * (r - l + 1) / 2);
}
template<int K> T<K> operation(int mode, LL x, LL y, int l)
{
    if constexpr (K == 0) return {x};
    if constexpr (K == 1) return mode % 3 == 0 ? T<K>::assign(x) : T<K>::increase(x);
    if constexpr (K == 2) return T<K>(x, y);
    if constexpr (K == 3) return {unsigned(x) & 0x7fffffffU};
    if constexpr (K == 4) return {mode % 3 - 1, (x & 1) != 0};
    if constexpr (K == 5) return mode % 3 == 0 ? T<K>{} : T<K>::assign(x);
    if constexpr (K == 6) return T<K>::progression(l, x, y);
}
template<int K> void update(vector<LL>& a, int l, int r, int mode, LL x, LL y)
{
    for (int i = l; i <= r; i++)
    {
        if constexpr (K == 0) a[i] += x;
        if constexpr (K == 1) { if (mode % 3 == 0) a[i] = x; else a[i] += x; }
        if constexpr (K == 2) a[i] = norm((__int128)x * a[i] + y);
        if constexpr (K == 3) a[i] ^= unsigned(x) & 0x7fffffffU;
        if constexpr (K == 4) { if (mode % 3) a[i] = mode % 3 - 1; a[i] ^= x & 1; }
        if constexpr (K == 5) { if (mode % 3) a[i] = x; }
        if constexpr (K == 6) a[i] += x + (i - l) * y;
    }
}
template<int K> void small()
{
    mt19937 gen(901207 + K);
    SegTree<I<K>, T<K>> seg(40);
    DySegTree<I<K>, T<K>> dy(40, 1000);
    for (int tc = 0; tc < 64; tc++)
    {
        int n = tc % 8 == 0 ? 1 : 2 + int(gen() % 35);
        vector<LL> a(n + 1);
        vector<I<K>> init(n + 1);
        for (int i = 1; i <= n; i++)
        {
            a[i] = tc % 4 == 0 ? -7 : int(gen() % 101) - 50;
            if constexpr (K == 2) a[i] = norm(a[i]);
            if constexpr (K == 3) a[i] = gen() & 0x7fffffffU;
            if constexpr (K == 4) a[i] = (i + tc) % 2;
            init[i] = leaf<K>(a[i], i);
        }
        seg.build(init); dy.build(init);
        // 初始化、空单位元、跨左/右的合并顺序。
        verify<K>(seg.query(1, n) + I<K>{}, a, 1, n);
        verify<K>(I<K>{} + dy.query(1, n), a, 1, n);
        for (int q = 0; q < 320; q++)
        {
            int l = 1 + int(gen() % n), r = 1 + int(gen() % n);
            if (l > r) swap(l, r);
            if (q % 5 < 2) { l = 1; r = n; } // 连续整段标记, 再局部下传
            if (q % 5 == 2) r = l;
            int mode = q % 9;
            LL x = int(gen() % 21) - 10, y = int(gen() % 9) - 4;
            if (q % 11 == 0) x = 0;
            if constexpr (K == 2) if (q % 7 == 0) x = MOD_TEST - 1;
            if constexpr (K == 3) x = q % 7 == 0 ? (1LL << 30) : gen() & 0x7fffffffU;
            auto tag = operation<K>(mode, x, y, l);
            seg.modify(l, r, tag); dy.modify(l, r, tag);
            update<K>(a, l, r, mode, x, y);
            int ql = 1 + int(gen() % n), qr = 1 + int(gen() % n);
            if (ql > qr) swap(ql, qr);
            verify<K>(seg.query(ql, qr), a, ql, qr);
            verify<K>(dy.query(ql, qr), a, ql, qr);
            if (q % 13 == 0)
            {
                seg.modify(1, n, T<K>{}); dy.modify(1, n, T<K>{});
                verify<K>(seg.query(1, n), a, 1, n);
                verify<K>(dy.query(1, n), a, 1, n);
            }
        }
        for (int l = 1; l <= n; l++) for (int r = l; r <= n; r++) verify<K>(seg.query(l, r), a, l, r);
    }
    if constexpr (K != 4 && K != 6)
    {
        // 已声明可补 len 表示零段的五组, 真正从未 build 的稀疏树起步。
        dy.init(1000000000);
        dy.modify(999999991, 1000000000, operation<K>(1, 7, 2, 999999991));
        vector<LL> a(11, 0); update<K>(a, 1, 10, 1, 7, 2);
        verify<K>(dy.query(999999991, 1000000000), a, 1, 10);
        assert(dy.query(1, 999999990).sum == 0);
        dy.clear(); assert(dy.query(1, 1000000000).sum == 0);
    }
}
template<int K> void large()
{
    constexpr int n = 200000;
    SegTree<I<K>, T<K>> seg(n);
    // 同一对象反复大-小-大, 覆盖旧标记清除; 各组顺序运行控制峰值内存。
    for (int size : {n, 1, 257, n})
    {
        vector<LL> a(size + 1);
        vector<I<K>> init(size + 1);
        for (int i = 1; i <= size; i++)
        {
            a[i] = i % 2 ? -1000000000LL : 1000000000LL;
            if constexpr (K == 2) a[i] = norm(a[i]);
            if constexpr (K == 3) a[i] = (1LL << 30) + i;
            if constexpr (K == 4) a[i] = i % 2;
            init[i] = leaf<K>(a[i], i);
        }
        seg.build(init);
        verify<K>(seg.query(1, size), a, 1, size);
        for (int q = 0; q < 20; q++)
        {
            int l = q % 2 ? 1 : max(1, size / 3), r = size;
            LL x = q % 3 == 0 ? 0 : (q % 2 ? -13 : 17), y = q % 5 - 2;
            seg.modify(l, r, operation<K>(q, x, y, l)); update<K>(a, l, r, q, x, y);
            verify<K>(seg.query(1, size), a, 1, size);
            verify<K>(seg.query(l, l), a, l, l);
        }
        // 20 万次修改/查询交替, 不把大数组配几个操作称为目标规模。
        for (int q = 0; q < size; q++)
        {
            int p = int((1LL * q * 100003) % size) + 1;
            int r = min(size, p + q % 31);
            seg.modify(p, r, operation<K>(q, q % 19 - 9, 2, p));
            update<K>(a, p, r, q, q % 19 - 9, 2);
            verify<K>(seg.query(p, p), a, p, p);
            if (q % 251 == 0) verify<K>(seg.query(p, r), a, p, r);
        }
        verify<K>(seg.query(1, size), a, 1, size);
    }
    cout << "[PASS] plugin " << K << ": both engines, n/q=200000, reset\n";
}
template<int K> void run() { small<K>(); large<K>(); }
int main()
{
    run<0>(); run<1>(); run<2>(); run<3>(); run<4>(); run<5>(); run<6>();
    cout << "seg_plugins_check PASS\n";
}
