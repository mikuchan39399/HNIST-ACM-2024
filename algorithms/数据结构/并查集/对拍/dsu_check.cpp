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
    for (int tc = 0; tc < 200; tc++)
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

int main()
{
    mt19937 rng(42);
    test_dsu(rng);
    wdsu_case(rng, 0);
    wdsu_case(rng, 2);
    wdsu_case(rng, 3);
    wdsu_case(rng, 5);
    cout << "dsu_check passed: DSU / WDSU all tests ok\n";
    return 0;
}