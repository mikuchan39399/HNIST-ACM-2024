// ============ misc_check 单调队列/快读写/离散化/防卡哈希 回归套件 ============
// 覆盖: MonotonicQueue 滑窗 min/max 对拍暴力 | Dcr 离散化对拍 sort+unique |
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
#include <unordered_map>
#include "../../数据结构/单调队列/单调队列.cpp"
#include "../快读快写/快读快写.cpp"
#include "../离散化/离散化.cpp"
#include "../防卡/哈希防卡.cpp"

using namespace std;
using LL = long long;

// ============ 段 1: 单调队列 滑窗对拍暴力 ============
static void test_mono_queue()
{
    mt19937 rng(4242);
    static MonotonicQueue mq{61};
    // 大-小-大复用, k=1/k=n 与全相等; init 不能破坏已分配数组的大小
    for (int n : {61, 1, 61})
    {
        mq.init();
        VLL a(n + 1, -7);
        assert(mq.get_min(a, n, 1) == VLL(n, -7));
        assert(mq.get_max(a, n, n) == VLL(1, -7));
        for (int i = 1; i <= n; i++) a[i] = i;
        assert(mq.get_min(a, n, n) == VLL(1, 1));
        assert(mq.get_max(a, n, n) == VLL(1, n));
    }
    for (int tc = 0; tc < 300; tc++)
    {
        int n = 1 + rng() % 60;
        int k = 1 + rng() % n;
        VLL a(n + 1);
        for (int i = 1; i <= n; i++) a[i] = (LL)(rng() % 41) - 20;
        mq.init();
        vector<LL> mn = mq.get_min(a, n, k), mx = mq.get_max(a, n, k);
        assert((int)mn.size() == n - k + 1 && (int)mx.size() == n - k + 1);
        for (int i = k; i <= n; i++)
        {
            LL lo = a[i], hi = a[i];
            for (int j = i - k + 1; j <= i; j++) { lo = min(lo, a[j]); hi = max(hi, a[j]); }
            assert(mn[i - k] == lo && mx[i - k] == hi);
        }
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
    test_discrete();
    test_custom_hash();
    test_rw();
    printf("misc_check passed: monoQueue/discrete/customHash/rw all tests ok\n");
    return 0;
}
