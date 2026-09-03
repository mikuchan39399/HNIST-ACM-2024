// ============ misc_check 单调队列/快读写/离散化/防卡哈希 回归套件 ============
// 覆盖: MonotonicQueue 滑窗 min/max 对拍暴力 | Dcr 离散化对拍 sort+unique |
//       custom_hash 插 1e5 查全中+桶分布粗检 | rw read/write 字节往返
//       (freopen 临时文件, 动 stdio, 放最后独占跑)
// 纪律: 改动上述任一模板, 必重跑本套件
// 跑法: g++ -std=c++20 -Wall -Wextra -O2 misc_check.cpp -o misc_check && ./misc_check
#include <cstdio>
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

// ============ 段 4: rw 字节往返 (freopen 独占, 放最后) ============
static void test_rw()
{
    mt19937 rng(123);
    vector<LL> vals;
    string in;
    for (int i = 0; i < 500; i++)
    {
        LL v = (LL)(rng() % 2000001) - 1000000;
        vals.push_back(v);
        in += to_string(v);
        in += (i % 3 == 0 ? '\n' : ' ');
    }
    const char* tmp = getenv("TEMP");
    string ipath = string(tmp ? tmp : ".") + "/misc_rw_in.txt";
    string opath = string(tmp ? tmp : ".") + "/misc_rw_out.txt";
    FILE* f = fopen(ipath.c_str(), "w");
    assert(f);
    fputs(in.c_str(), f);
    fclose(f);
    assert(freopen(ipath.c_str(), "r", stdin));
    for (size_t i = 0; i < vals.size(); i++)
    {
        LL v;
        assert(read(v));
        assert(v == vals[i]);
    }
    assert(freopen(opath.c_str(), "w", stdout));
    for (size_t i = 0; i < vals.size(); i++)
        write(vals[i], i + 1 == vals.size() ? '\n' : ' ');
    utils_io::flush_io();
    fflush(stdout);
    assert(freopen("CON", "r", stdin));
    assert(freopen("CON", "w", stdout));
    f = fopen(opath.c_str(), "r");
    assert(f);
    for (size_t i = 0; i < vals.size(); i++)
    {
        long long v;
        assert(fscanf(f, "%lld", &v) == 1);
        assert(v == vals[i]);
    }
    fclose(f);
    remove(ipath.c_str());
    remove(opath.c_str());
}

int main()
{
    test_mono_queue();
    test_discrete();
    test_custom_hash();
    test_rw();                       // 动 stdin/stdout, 放最后
    printf("misc_check passed: monoQueue/discrete/customHash/rw all tests ok\n");
    return 0;
}
