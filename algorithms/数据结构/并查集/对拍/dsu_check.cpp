// ============ dsu_check 并查集双件回归 ============
// 覆盖: DSU(merge 返回值/same/size vs 染色重标暴力, init 多测复用) |
//       WDSU 真实距离模式 & 逻辑归一模式 mod∈{2,3,5} (merge/query/same vs
//       带偏移平移暴力, INF 断连, 真实模式反对称性)
// 纪律: 改动 DSU/WDSU, 必重跑本套件
// 跑法: g++ -std=c++20 -Wall -Wextra -O2 dsu_check.cpp -o dsu_check && ./dsu_check
#include <iostream>
#include <vector>
#include <cassert>
#include <algorithm>
#include <set>
#include <random>
#include "../DSU.cpp"
#include "../WDSU.cpp"

using namespace std;
using LL = long long;
using VI = vector<int>;

static void test_dsu(mt19937& rng)
{
    static DSU dsu(20);
    for (int tc = 0; tc < 300; tc++)
    {
        int n = 1 + rng() % 12;
        dsu.init(n);
        VI lab(n + 1), cnt(n + 1, 1);
        for (int i = 1; i <= n; i++) lab[i] = i;
        for (int op = 0, ops = 80; op < ops; op++)
        {
            int u = 1 + rng() % n, v = 1 + rng() % n;
            int act = rng() % 3;
            if (act == 0)
            {
                assert(dsu.merge(u, v) == (lab[u] != lab[v]));
                if (lab[u] != lab[v])
                {
                    int from = lab[v], to = lab[u];
                    for (int i = 1; i <= n; i++)
                        if (lab[i] == from) lab[i] = to;
                    cnt[to] += cnt[from];
                    cnt[from] = 0;
                }
            }
            else if (act == 1) assert(dsu.same(u, v) == (lab[u] == lab[v]));
            else assert(dsu.size(u) == cnt[lab[u]]);
            assert(dsu.same(u, 1) == (lab[u] == lab[1]));
        }
    }
}

// WDSU 暴力: 每点 val(相对组内锚点), merge 把小组整体平移
static void wdsu_case(mt19937& rng, LL mod)
{
    for (int tc = 0; tc < 300; tc++)
    {
        int n = 1 + rng() % 10;
        WDSU ds(n, mod);
        VI lab(n + 1);
        vector<LL> val(n + 1, 0);
        for (int i = 1; i <= n; i++) lab[i] = i;
        auto norm = [&](LL v) { return mod ? (v % mod + mod) % mod : v; };
        for (int op = 0, ops = 60; op < ops; op++)
        {
            int u = 1 + rng() % n, v = 1 + rng() % n;
            LL w = (LL)(rng() % 41) - 20;
            int act = rng() % 3;
            if (act == 0)
            {
                ds.merge(u, v, w);
                if (lab[u] != lab[v])
                {
                    LL off = val[u] - w - val[v];
                    int from = lab[v], to = lab[u];
                    for (int i = 1; i <= n; i++)
                        if (lab[i] == from)
                        {
                            lab[i] = to;
                            val[i] = norm(val[i] + off);
                        }
                }
            }
            else if (act == 1) assert(ds.same(u, v) == (lab[u] == lab[v]));
            else
            {
                LL got = ds.query(u, v);
                if (lab[u] != lab[v]) assert(got == WDSU::INF);
                else assert(got == norm(val[u] - val[v]));
            }
        }
        // 真实模式反对称性抽检: query(x,y) == -query(y,x)
        if (mod == 0)
        {
            int u = 1 + rng() % n, v = 1 + rng() % n;
            if (lab[u] == lab[v]) assert(ds.query(u, v) == -ds.query(v, u));
        }
    }
}

// 独立参照只存成功约束边, 用 BFS 沿路径累加, 不维护并查集势能
static void test_constraints(mt19937& rng, LL mod)
{
    auto norm = [=](LL x) { if (!mod) return x; x %= mod; return x < 0 ? x + mod : x; };
    for (int tc = 0; tc < 350; ++tc)
    {
        int n = 1 + rng() % 18;
        WDSU ds(n, mod);
        vector<vector<pair<int, LL>>> g(n + 1);
        auto path = [&](int s, int t) -> pair<bool, LL>
        {
            VI seen(n + 1), q{s};
            VLL val(n + 1);
            seen[s] = 1;
            for (size_t i = 0; i < q.size(); ++i)
                for (auto [v, w] : g[q[i]]) if (!seen[v])
                {
                    seen[v] = 1;
                    val[v] = val[q[i]] + w;
                    q.push_back(v);
                }
            return {seen[t], norm(val[t])};
        };
        for (int op = 0; op < 150; ++op)
        {
            int u = 1 + rng() % n, v = 1 + rng() % n;
            auto [connected, expected] = path(u, v);
            LL w = ((LL)(rng() % 2000001) - 1000000) * 1000000;
            int root = ds.find(u);
            ds.merge(u, v, w);
            if (!connected)
            {
                g[u].push_back({v, w});
                g[v].push_back({u, -w});
                assert(ds.find(v) == root);
            }
            else assert(ds.query(u, v) == expected); // 已连通的矛盾声明也应忽略
            if (op % 15 == 0)
                for (int x = 1; x <= n; ++x)
                    for (int y = 1; y <= n; ++y)
                    {
                        auto [ok, distance] = path(x, y);
                        assert(ds.same(x, y) == ok);
                        assert(ds.query(x, y) == (ok ? distance : WDSU::INF));
                    }
        }
    }
}

static void test_dsu_large(mt19937& rng)
{
    constexpr int N = 200000;
    DSU ds(N);
    for (int n : {N, 0, 1, 37, N})
    {
        ds.init(n);
        for (int i = 1; i <= n; ++i) assert(ds.find(i) == i && ds.size(i) == 1);
        if (!n) continue;
        for (int shape = 0; shape < 3; ++shape)
        {
            ds.init(n);
            if (shape < 2)
                for (int i = 2; i <= n; ++i)
                    assert(shape == 0 ? ds.merge(i, i - 1) : ds.merge(1, i));
            else
                for (int len = 1; len < n; len *= 2)
                    for (int l = 1; l + len <= n; l += 2 * len) assert(ds.merge(l, l + len));
            int root = shape == 0 ? n : 1;
            assert(ds.find(1) == root);
            for (int i = n; i >= 1; --i)
            {
                assert(ds.find(i) == root && ds.size(i) == n);
                assert(!ds.merge(i, root) && !ds.merge(i, i));
            }
        }
        ds.init(n);
        constexpr int block = 257;
        for (int i = 1; i < n; ++i) if (i % block) assert(ds.merge(i + 1, i));
        for (int op = 0; op < n; ++op)
        {
            int u = 1 + rng() % n, v = 1 + rng() % n;
            int group = (u - 1) / block;
            assert(ds.same(u, v) == (group == (v - 1) / block));
            assert(ds.size(u) == min(block, n - group * block));
        }
    }
}

static void test_weight_large(mt19937& rng, LL mod)
{
    constexpr int n = 200000;
    auto potential = [](int x) { return (x % 2 ? 1LL : -1LL) * x * 1000000 + x % 17 * 13; };
    auto norm = [=](LL x) { if (!mod) return x; x %= mod; return x < 0 ? x + mod : x; };
    for (int shape = 0; shape < 4; ++shape)
    {
        WDSU ds(n, mod);
        if (shape == 2)
            for (int len = 1; len < n; len *= 2)
                for (int l = 1; l + len <= n; l += 2 * len)
                    ds.merge(l, l + len, potential(l) - potential(l + len));
        else
            for (int i = 2; i <= n; ++i)
            {
                if (shape == 3 && (i - 1) % 257 == 0) continue;
                int x = shape == 1 ? 1 : i, y = shape == 1 ? i : i - 1;
                ds.merge(x, y, potential(x) - potential(y));
            }
        // 先查最早的点, 确保退化链在尚未压缩时真正经过 find
        assert(ds.query(1, shape == 3 ? 257 : n) == norm(potential(1) - potential(shape == 3 ? 257 : n)));
        for (int op = 1; op <= n; ++op)
        {
            int u = op, v = 1 + rng() % n;
            bool connected = shape != 3 || (u - 1) / 257 == (v - 1) / 257;
            assert(ds.same(u, v) == connected);
            assert(ds.query(u, v) == (connected ? norm(potential(u) - potential(v)) : WDSU::INF));
            assert(ds.query(u, u) == 0);
            if (connected)
            {
                ds.merge(u, v, 123); // 既有约束不被新声明覆盖
                assert(ds.query(v, u) == norm(potential(v) - potential(u)));
            }
        }
    }
}

static void test_weight_reset(mt19937& rng)
{
    constexpr int N = 200000;
    WDSU ds(N);
    auto fa = ds.fa.data();
    auto dist = ds.d.data();
    auto fc = ds.fa.capacity(), dc = ds.d.capacity();
    for (int n : {N, 0, 1, 17, N})
        for (LL mod : {0LL, 1LL, 3LL, 1000000007LL})
        {
            ds.init(n, mod);
            assert(ds.mod == mod && ds.fa.data() == fa && ds.d.data() == dist);
            assert(ds.fa.capacity() == fc && ds.d.capacity() == dc);
            for (int i = 1; i <= n; ++i)
                assert(ds.find(i) == i && ds.d[i] == 0 && ds.query(i, i) == 0);
            for (int i = 2; i <= n; ++i) ds.merge(i, i - 1, 1);
            for (int i = 1; i <= n; ++i)
            {
                LL expected = i - n;
                if (mod) { expected %= mod; if (expected < 0) expected += mod; }
                assert(ds.query(i, n) == expected);
            }
        }
    for (int tc = 0; tc < 300; ++tc)
    {
        int n = 1 + rng() % 20;
        ds.init(n, 3);
        ds.merge(1, n, 2);
        ds.init(n); // 省略模数须重置为普通距离模式
        assert(ds.mod == 0 && ds.query(1, n) == (n == 1 ? 0 : WDSU::INF));
        ds.merge(1, n, -7);
        assert(ds.query(1, n) == (n == 1 ? 0 : -7));
    }
    WDSU empty;
    empty.init(0, 1);
    empty.init(0);
    assert(empty.mod == 0 && empty.fa.size() == 1 && empty.d[0] == 0);
}

static void test_weight_edges()
{
    for (LL mod : {0LL, 1LL, 2LL, 3LL, 1000000007LL, LLONG_MAX - 1, LLONG_MAX})
    {
        WDSU ds(3, mod);
        assert(!ds.same(1, 3) && ds.query(1, 3) == WDSU::INF);
        ds.merge(1, 2, 1);
        assert(ds.query(1, 2) == (mod == 1 ? 0 : 1));
        assert(ds.query(2, 1) == (mod ? mod - 1 : -1));
        ds.merge(1, 2, 99);
        ds.merge(1, 1, 99);
        assert(ds.query(1, 1) == 0);
        assert(ds.query(1, 2) == (mod == 1 ? 0 : 1));
    }
    for (LL w : {WDSU::INF, LLONG_MAX, -LLONG_MAX})
    {
        WDSU ds(2);
        ds.merge(1, 2, w);
        assert(ds.same(1, 2) && ds.query(1, 2) == w && ds.query(2, 1) == -w);
    }
}

int main()
{
    mt19937 rng(42);
    test_dsu(rng);
    wdsu_case(rng, 0);
    wdsu_case(rng, 2);
    wdsu_case(rng, 3);
    wdsu_case(rng, 5);
    for (LL mod : {0LL, 1LL, 2LL, 3LL, 5LL, 1000000007LL}) test_constraints(rng, mod);
    test_weight_edges();
    test_dsu_large(rng);
    for (LL mod : {0LL, 1LL, 3LL, 1000000007LL}) test_weight_large(rng, mod);
    test_weight_reset(rng);
    cout << "dsu_check passed: DSU / WDSU all tests ok\n";
    return 0;
}
