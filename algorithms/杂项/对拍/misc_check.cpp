// ============ misc_check 单调队列/单调栈/快读写/离散化/防卡哈希 回归套件 ============
// 覆盖: 滑窗/前驱 DP/最近边界的小暴力及 20 万规模独立参照 | Dcr 对拍 sort+unique |
//       custom_hash 插 1e5 查全中+桶分布粗检 | rw read/write 字节往返
//       (保存并恢复标准流文件描述符, Windows/Linux 都执行)
// 纪律: 改动上述任一模板, 必重跑本套件
// 跑法: g++ -std=c++20 -Wall -Wextra -O2 misc_check.cpp -o misc_check && ./misc_check
#include <cstdio>
#include <climits>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include <cstdlib>
#include <iostream>
#include <vector>
#include <cassert>
#include <random>
#include <algorithm>
#include <set>
#include <unordered_map>
#include "../../数据结构/单调队列/单调队列.cpp"
#include "../../数据结构/单调栈/单调栈.cpp"
#include "../快读快写/快读快写.cpp"
#include "../离散化/离散化.cpp"
#include "../防卡/哈希防卡.cpp"

using namespace std;
using LL = long long;

// ============ 段 1: 单调队列 滑窗对拍暴力 ============
// 独立固定窗口参照: 按 k 个元素分块, 用块后缀与下一块前缀拼答案
static VLL window_reference(const VLL& a, int n, int k, bool maximum)
{
    VLL pre(n + 1), suf(n + 2), res;
    auto best = [=](LL x, LL y) { return maximum ? max(x, y) : min(x, y); };
    for (int i = 1; i <= n; ++i)
        pre[i] = (i - 1) % k == 0 ? a[i] : best(pre[i - 1], a[i]);
    for (int i = n; i >= 1; --i)
        suf[i] = i == n || i % k == 0 ? a[i] : best(suf[i + 1], a[i]);
    for (int r = k; r <= n; ++r) res.push_back(best(suf[r - k + 1], pre[r]));
    return res;
}

template<class T, class Bad>
static void check_window(const vector<T>& a, int k, Bad bad)
{
    auto got = mono_window(a, k, bad);
    int n = (int)a.size() - 1;
    assert((int)got.size() == n + 1 && got[0] == 0);
    for (int r = 1; r <= n; ++r)
    {
        if (r < k) { assert(got[r] == 0); continue; }
        int best = r - k + 1;
        for (int i = best + 1; i <= r; ++i) if (bad(a[best], a[i])) best = i;
        assert(got[r] == best);
    }
}

static VLL window_values(const VLL& a, int k, bool maximum)
{
    auto id = maximum ? mono_window(a, k, less_equal<LL>{}) : mono_window(a, k);
    VLL res;
    for (int i = k; i < (int)a.size(); ++i) res.push_back(a[id[i]]);
    return res;
}

static void test_mono_queue()
{
    mt19937 rng(42);
    for (int tc = 0; tc < 400; ++tc)
    {
        int n = rng() % 61, k = 1 + rng() % (n + 3);
        VLL a(n + 1, LLONG_MIN);
        for (int i = 1; i <= n; ++i) a[i] = (LL)(rng() % 41) - 20;
        auto before = a;
        check_window(a, k, greater_equal<LL>{});
        check_window(a, k, greater<LL>{});
        check_window(a, k, less_equal<LL>{});
        check_window(a, k, less<LL>{});
        assert(a == before);
    }
    // 平手策略要逐下标检查, 只比最值会掩盖 >= 与 > 的差别
    for (int n : {1, 61, 1, 61})
    {
        VLL a(n + 1, -7);
        for (int k : {1, n, n + 1})
        {
            check_window(a, k, greater_equal<LL>{});
            check_window(a, k, greater<LL>{});
            check_window(a, k, less_equal<LL>{});
            check_window(a, k, less<LL>{});
        }
    }
    vector<int> small{0, INT_MIN, INT_MAX, INT_MIN};
    check_window(small, 2, greater_equal<int>{});
    struct Item { LL x, y; };
    for (int tc = 0; tc < 300; ++tc)
    {
        vector<Item> a(21);
        for (auto& x : a) x = {(int)(rng() % 21) - 10, (int)(rng() % 21) - 10};
        auto bad = [](Item old, Item now) { return old.x + old.y >= now.x + now.y; };
        check_window(a, 1 + rng() % 23, bad);
    }
    constexpr int N = 200000;
    for (int n : {N, 1, 257, N})
        for (int shape = 0; shape < 6; ++shape)
        {
            VLL a(n + 1);
            for (int i = 1; i <= n; ++i)
            {
                if (shape == 0) a[i] = i;
                if (shape == 1) a[i] = -i;
                if (shape == 2) a[i] = -7;
                if (shape == 3) a[i] = i % 2 ? LLONG_MIN : LLONG_MAX;
                if (shape == 4) a[i] = i % 257 - 128;
                if (shape == 5) a[i] = (LL)(rng() % 2000001) - 1000000;
            }
            auto original = a;
            for (int k : {1, min(2, n), min(257, n), max(1, n / 2), max(1, n - 1), n})
            {
                assert(window_values(a, k, false) == window_reference(a, n, k, false));
                assert(window_values(a, k, true) == window_reference(a, n, k, true));
                assert(a == original);
            }
            assert(mono_window(a, n + 1) == VI(n + 1, 0));
        }
}

static void test_mono_dp()
{
    mt19937 rng(42);
    for (int tc = 0; tc < 600; ++tc)
    {
        int n = rng() % 61, k = 1 + rng() % (n + 3);
        VLL cost(n + 1, LLONG_MAX), expected(n + 1);
        for (int i = 1; i <= n; ++i)
        {
            cost[i] = (LL)(rng() % 41) - 20;
            LL best = LLONG_MAX;
            for (int j = max(0, i - k); j < i; ++j) best = min(best, expected[j]);
            expected[i] = best + cost[i];
        }
        auto before = cost;
        assert(mono_dp(cost, k) == expected && cost == before);
    }
    for (auto cost : {VLL{0, LLONG_MIN}, VLL{0, LLONG_MAX}, VLL{0, LLONG_MAX, -LLONG_MAX}, VLL{0, LLONG_MIN, LLONG_MAX}})
        for (int k : {1, 2, 3})
        {
            VLL expected(cost.size());
            for (int i = 1; i < (int)cost.size(); ++i)
            {
                LL best = expected[max(0, i - k)];
                for (int j = max(0, i - k); j < i; ++j) best = min(best, expected[j]);
                expected[i] = best + cost[i];
            }
            assert(mono_dp(cost, k) == expected);
        }
    constexpr int N = 200000;
    for (int n : {N, 0, 1, 257, N})
        for (int k : {1, 257, max(1, n), n + 1})
            for (LL c : {-1LL, 0LL, 1LL})
            {
                VLL cost(n + 1, c);
                auto f = mono_dp(cost, k);
                assert(f.size() == cost.size() && f[0] == 0);
                for (int i = 1; i <= n; ++i)
                    assert(f[i] == (c < 0 ? -i : c == 0 ? 0 : (i + k - 1) / k));
            }
    // 大规模随机状态以 multiset 维护全部合法前驱, 不做支配淘汰
    for (int k : {257, 100003})
    {
        VLL cost(N + 1), expected(N + 1);
        multiset<LL> active{0};
        for (int i = 1; i <= N; ++i)
        {
            cost[i] = ((LL)(rng() % 2000001) - 1000000) * 1000000;
            if (i > k) active.erase(active.find(expected[i - k - 1]));
            expected[i] = *active.begin() + cost[i];
            active.insert(expected[i]);
        }
        assert(mono_dp(cost, k) == expected);
    }
}
template<class T, class Bad>
static void check_stack(const vector<T>& a, Bad bad)
{
    auto got = mono_stack(a, bad);
    assert(got.size() == a.size() && got[0] == 0);
    for (int i = 1; i < (int)a.size(); ++i)
    {
        int j = i - 1;
        while (j && bad(a[j], a[i])) --j;
        assert(got[i] == j);
    }
}

// 独立参照: 按值离散化, Fenwick 前缀最大下标给出最近合格位置
static VI stack_reference(const VLL& a, bool maximum, bool strict)
{
    VLL values(a.begin() + 1, a.end());
    sort(values.begin(), values.end());
    values.erase(unique(values.begin(), values.end()), values.end());
    int m = (int)values.size();
    VI tree(m + 1), ans(a.size());
    for (int i = 1; i < (int)a.size(); ++i)
    {
        int rank = (int)(lower_bound(values.begin(), values.end(), a[i]) - values.begin()) + 1;
        if (maximum) rank = m - rank + 1;
        for (int p = rank - strict; p; p -= p & -p) ans[i] = max(ans[i], tree[p]);
        for (int p = rank; p <= m; p += p & -p) tree[p] = max(tree[p], i);
    }
    return ans;
}

static void test_mono_stack()
{
    mt19937 rng(42);
    for (int tc = 0; tc < 600; ++tc)
    {
        int n = rng() % 61;
        VLL a(n + 1, LLONG_MIN);
        for (int i = 1; i <= n; ++i) a[i] = (LL)(rng() % 41) - 20;
        auto before = a;
        check_stack(a, greater_equal<LL>{});
        check_stack(a, greater<LL>{});
        check_stack(a, less_equal<LL>{});
        check_stack(a, less<LL>{});
        assert(a == before);
        // 反转非哨兵部分再映射下标, 验证 README 的右侧用法
        reverse(a.begin() + 1, a.end());
        auto rev = mono_stack(a);
        for (int i = 1; i <= n; ++i)
        {
            int j = i + 1;
            while (j <= n && before[j] >= before[i]) ++j;
            int p = rev[n - i + 1];
            assert((p ? n - p + 1 : 0) == (j <= n ? j : 0));
        }
    }
    check_stack(vector<int>{0, INT_MAX, INT_MIN, INT_MIN}, greater_equal<int>{});
    struct Item { LL x, y; };
    for (int tc = 0; tc < 300; ++tc)
    {
        vector<Item> a(21);
        for (auto& x : a) x = {(int)(rng() % 21) - 10, (int)(rng() % 21) - 10};
        check_stack(a, [](Item old, Item now) { return old.x + old.y >= now.x + now.y; });
    }
    constexpr int N = 200000;
    for (int n : {N, 0, 1, 257, N})
        for (int shape = 0; shape < 6; ++shape)
        {
            VLL a(n + 1);
            for (int i = 1; i <= n; ++i)
            {
                if (shape == 0) a[i] = i;
                if (shape == 1) a[i] = -i;
                if (shape == 2) a[i] = -7;
                if (shape == 3) a[i] = i % 2 ? LLONG_MIN : LLONG_MAX;
                if (shape == 4) a[i] = i % 257 - 128;
                if (shape == 5) a[i] = (LL)(rng() % 2000001) - 1000000;
            }
            auto before = a;
            assert(mono_stack(a) == stack_reference(a, false, true));
            assert(mono_stack(a, greater<LL>{}) == stack_reference(a, false, false));
            assert(mono_stack(a, less_equal<LL>{}) == stack_reference(a, true, true));
            assert(mono_stack(a, less<LL>{}) == stack_reference(a, true, false));
            assert(a == before);
        }
}

// ============ 段 2: 离散化 对拍 sort+unique ============
static void test_discrete()
{
    mt19937 rng(777);
    for (int tc = 0; tc < 300; tc++)
    {
        int n = 1 + rng() % 60;
        vector<LL> a(n + 1);
        for (int i = 1; i <= n; i++) a[i] = (LL)(rng() % 41) - 20;
        vector<LL> sorted(a.begin() + 1, a.end());
        sort(sorted.begin(), sorted.end());
        sorted.erase(unique(sorted.begin(), sorted.end()), sorted.end());
        Dcr<LL> d;
        for (int i = 1; i <= n; i++) d.add(a[i]);
        d.build();
        assert(d.size() == (int)sorted.size());
        for (int i = 1; i <= n; i++)
        {
            int rk = (int)(lower_bound(sorted.begin(), sorted.end(), a[i]) - sorted.begin()) + 1;
            assert(d(a[i]) == rk && d[rk] == a[i]);
        }
    }
}

// ============ 段 3: custom_hash 功能冒烟 ============
static void test_custom_hash()
{
    mt19937_64 rng(20260903);
    unordered_map<uint64_t, int, custom_hash> mp;
    mp.reserve(200000);
    vector<uint64_t> keys;
    for (int i = 0; i < 100000; i++)
    {
        uint64_t k = ((uint64_t)rng() << 32) ^ rng();
        keys.push_back(k);
        mp[k]++;
    }
    for (uint64_t k : keys)
    {
        auto it = mp.find(k);
        assert(it != mp.end() && it->second >= 1);
    }
    size_t nb = mp.bucket_count();
    size_t avg = mp.size() / nb + 1;
    int over = 0;
    for (size_t b = 0; b < nb; b++)
        if (mp.bucket_size(b) > 10 * avg) over++;   // 10 倍: 泊松涨落到不了, 只有构造性聚集会触发
    assert(over < (int)(nb / 100));
}

// 字节级验证: 跨 4MiB 缓冲区、EOF、整数极值、浮点科学计数法与无结尾字符
static void test_rw()
{
#ifdef _WIN32
    auto dup_fd = _dup;
    auto dup2_fd = _dup2;
    auto close_fd = _close;
    auto file_no = _fileno;
#else
    auto dup_fd = dup;
    auto dup2_fd = dup2;
    auto close_fd = close;
    auto file_no = fileno;
#endif
    const string ipath = "misc_rw_in.txt", opath = "misc_rw_out.txt";
    string longword(utils_io::BUFSZ + 17, 'a');
    string in(utils_io::BUFSZ - 1, ' ');
    in += "-9223372036854775808 9223372036854775807 0\r\n";
    in += "18446744073709551615 -170141183460469231731687303715884105728 ";
    in += "340282366920938463463374607431768211455 1.25e3 -2.5e-2 ";
    in += longword; // 最后一个词后无空白, 下一次读应返回 EOF
    FILE* f = fopen(ipath.c_str(), "wb");
    assert(f && fwrite(in.data(), 1, in.size(), f) == in.size());
    fclose(f);
    fflush(stdout);
    int saved_in = dup_fd(file_no(stdin)), saved_out = dup_fd(file_no(stdout));
    assert(saved_in >= 0 && saved_out >= 0);
    assert(freopen(ipath.c_str(), "rb", stdin));
    LL lo, hi, zero;
    unsigned long long umax;
    __int128 imin;
    unsigned __int128 uimax;
    double a, b;
    string word;
    assert(read(lo) && lo == LLONG_MIN);
    assert(read(hi) && hi == LLONG_MAX);
    assert(read(zero) && zero == 0);
    assert(read(umax) && umax == ULLONG_MAX);
    assert(read(imin) && imin == -((__int128)1 << 126) - ((__int128)1 << 126));
    assert(read(uimax) && uimax == ~(unsigned __int128)0);
    assert(read(a) && a == 1250.0);
    assert(read(b) && b == -0.025);
    assert(read(word) && word == longword);
    assert(!read(word));
    assert(!read(zero));
    assert(freopen(opath.c_str(), "wb", stdout));
    write(lo, ' '); write(hi, ' '); write(umax, ' ');
    write(imin, ' '); write(uimax, '\n');
    write(0, '\0'); write('X', '\0'); write("Y", '\0');
    write(string("Z"), '\0'); write(1.25, '\n');
    write(longword, '\0');
    utils_io::flush_io();
    fflush(stdout);
    assert(dup2_fd(saved_in, file_no(stdin)) >= 0);
    assert(dup2_fd(saved_out, file_no(stdout)) >= 0);
    close_fd(saved_in); close_fd(saved_out);
    clearerr(stdin); clearerr(stdout);
    string expected = "-9223372036854775808 9223372036854775807 18446744073709551615 ";
    expected += "-170141183460469231731687303715884105728 ";
    expected += "340282366920938463463374607431768211455\n0XYZ1.250000\n";
    expected += longword;
    f = fopen(opath.c_str(), "rb");
    assert(f);
    string output(expected.size() + 1, '\0');
    output.resize(fread(output.data(), 1, output.size(), f));
    assert(output == expected);
    fclose(f);
    remove(ipath.c_str()); remove(opath.c_str());
}

int main()
{
    test_mono_queue();
    test_mono_dp();
    test_mono_stack();
    test_discrete();
    test_custom_hash();
    test_rw();
    printf("misc_check passed: monoQueue/monoStack/discrete/customHash/rw all tests ok\n");
    return 0;
}
