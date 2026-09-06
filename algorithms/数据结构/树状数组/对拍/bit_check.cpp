// ============ bit_check BIT/BITR/BIT2D 回归套件 ============
// 覆盖: BIT 区间加+前缀/区间查 | BITR 区间加+后缀查(坐标镜像) |
//       BIT2D 矩阵加+矩阵查; 全部对拍朴素数组, static 实例跨组 init 复用
// 默认另跑 20 万长度分块参照及 2000x2000 在线矩形交集/离线前缀和压力
// 纪律: 改动 树状数组/二维树状数组 模板, 必重跑本套件
// 跑法: g++ -std=c++20 -Wall -Wextra -O2 bit_check.cpp -o bit_check && ./bit_check
#include <iostream>
#include <vector>
#include <cassert>
#include <random>
#include <algorithm>
#include "../树状数组.cpp"
#include "../二维树状数组.cpp"

using namespace std;
using LL = long long;

// 模式 A: BIT 前缀 + BITR 后缀双引擎同场, 300 轮
void test_bit_1d()
{
    mt19937 rng(42);
    static BIT bit{61};
    static BITR br{61};
    for (int tc = 0; tc < 300; tc++)
    {
        int n = 1 + rng() % 60;
        vector<LL> ref(n + 2, 0);
        bit.init(n);
        br.init(n);
        for (int t = 0; t < 80; t++)
        {
            int l = 1 + rng() % n, r = 1 + rng() % n;
            if (l > r) swap(l, r);
            LL k = (LL)(rng() % 11) - 5;
            if (rng() % 3 == 0)
            {
                bit.add(l, r, k);
                br.add(l, r, k);
                for (int i = l; i <= r; i++) ref[i] += k;
            }
            else
            {
                LL s = 0;
                for (int i = l; i <= r; i++) s += ref[i];
                assert(bit.query(l, r) == s);
                assert(br.query(l, r) == s);
            }
        }
        // 尾对账: 全部前缀/后缀点
        for (int x = 1; x <= n; x++)
        {
            LL pre = 0, suf = 0;
            for (int i = 1; i <= x; i++) pre += ref[i];
            for (int i = x; i <= n; i++) suf += ref[i];
            assert(bit.pre(x) == pre);
            assert(br.suf(x) == suf);
        }
        assert(br.suf(n + 1) == 0);
    }
}

// 模式 B: BIT2D 矩阵域, 300 轮
void test_bit_2d()
{
    mt19937 rng(4242);
    static BIT2D t{31, 31};
    for (int tc = 0; tc < 300; tc++)
    {
        int n = 1 + rng() % 30, m = 1 + rng() % 30;
        vector<vector<LL>> ref(n + 1, vector<LL>(m + 1, 0));
        t.init(n, m);
        for (int q = 0; q < 60; q++)
        {
            int x1 = 1 + rng() % n, x2 = 1 + rng() % n;
            int y1 = 1 + rng() % m, y2 = 1 + rng() % m;
            if (x1 > x2) swap(x1, x2);
            if (y1 > y2) swap(y1, y2);
            LL k = (LL)(rng() % 11) - 5;
            if (rng() % 3 == 0)
            {
                t.add(x1, y1, x2, y2, k);
                for (int i = x1; i <= x2; i++)
                    for (int j = y1; j <= y2; j++) ref[i][j] += k;
            }
            else
            {
                LL s = 0;
                for (int i = x1; i <= x2; i++)
                    for (int j = y1; j <= y2; j++) s += ref[i][j];
                assert(t.query(x1, y1, x2, y2) == s);
            }
        }
        // 尾对账: DP 二维前缀和全格点
        vector<vector<LL>> ps(n + 1, vector<LL>(m + 1, 0));
        for (int i = 1; i <= n; i++)
            for (int j = 1; j <= m; j++)
                ps[i][j] = ref[i][j] + ps[i - 1][j] + ps[i][j - 1] - ps[i - 1][j - 1];
        for (int x = 1; x <= n; x++)
            for (int y = 1; y <= m; y++)
                assert(t.query(1, 1, x, y) == ps[x][y]);
    }
}

// 明确覆盖单格、完整容量、贴边更新、64位和、大-小-大复位
void test_boundaries()
{
    BIT bit(60);
    BITR br(60);
    const LL k = 1000000000000LL;
    for (int n : {60, 1, 60})
    {
        bit.init(n); br.init(n);
        assert(bit.pre(0) == 0 && br.suf(n + 1) == 0);
        assert(bit.query(1, n) == 0 && br.query(1, n) == 0);
        bit.add(1, n, k); br.add(1, n, k);
        bit.add(n, n, -2 * k); br.add(n, n, -2 * k);
        for (int l = 1; l <= n; l++)
            for (int r = l; r <= n; r++)
            {
                LL ref = (r - l + 1) * k - (r == n ? 2 * k : 0);
                assert(bit.query(l, r) == ref && br.query(l, r) == ref);
            }
    }
    BIT2D grid(30, 30);
    for (auto [n, m] : vector<pair<int,int>>{{30,30},{1,1},{1,30},{30,1},{30,30}})
    {
        grid.init(n, m);
        assert(grid.query(1, 1, n, m) == 0);
        grid.add(1, 1, n, m, k);
        grid.add(n, m, n, m, -2 * k);
        for (int x = 1; x <= n; x++)
            for (int y = 1; y <= m; y++)
            {
                assert(grid.query(x, y, x, y) == ((x == n && y == m) ? -k : k));
                assert(grid.query(1, 1, x, y) == (LL)x * y * k - ((x == n && y == m) ? 2 * k : 0));
            }
    }
}

// 独立分块参照, 不使用树状数组差分或 lowbit
struct BlockRef
{
    static constexpr int B = 700;
    int n;
    vector<LL> a, sum, tag;
    BlockRef(int n) : n(n), a(n), sum((n + B - 1) / B), tag(sum.size()) {}
    int end(int b) { return min(n, (b + 1) * B); }
    void add(int l, int r, LL v)
    {
        --l;
        while (l < r)
        {
            int b = l / B;
            if (l % B == 0 && end(b) <= r)
            {
                tag[b] += v;
                l = end(b);
            }
            else
            {
                a[l] += v;
                sum[b] += v;
                ++l;
            }
        }
    }
    LL query(int l, int r)
    {
        LL ans = 0;
        --l;
        while (l < r)
        {
            int b = l / B;
            if (l % B == 0 && end(b) <= r)
            {
                ans += sum[b] + tag[b] * (end(b) - l);
                l = end(b);
            }
            else ans += a[l++] + tag[b];
        }
        return ans;
    }
};

void test_large_1d()
{
    constexpr int N = 200000;
    BIT bit(N);
    BITR br(N);
    mt19937 rng(42);
    for (int n : {N, 1, 31, 32, 33, N})
    {
        bit.init(n); br.init(n);
        BlockRef ref(n);
        int q = n == N ? 200000 : 400;
        for (int i = 0; i < q; ++i)
        {
            int l = 1 + rng() % n, r = 1 + rng() % n;
            if (l > r) swap(l, r);
            if (i % 7 == 0) l = 1;
            if (i % 7 == 1) r = n;
            if (i % 7 == 2) r = l;
            LL k = (int)(rng() % 2000001) - 1000000;
            bit.add(l, r, k); br.add(l, r, k); ref.add(l, r, k);
            l = 1 + rng() % n; r = 1 + rng() % n;
            if (l > r) swap(l, r);
            LL want = ref.query(l, r);
            assert(bit.query(l, r) == want && br.query(l, r) == want);
            if (i % 1000 == 0)
            {
                assert(bit.pre(r) == ref.query(1, r));
                assert(br.suf(l) == ref.query(l, n));
            }
        }
        LL pre = 0, total = ref.query(1, n);
        for (int i = 1; i <= n; ++i)
        {
            LL v = ref.query(i, i);
            assert(br.suf(i) == total - pre);
            pre += v;
            assert(bit.pre(i) == pre);
        }
        assert(bit.pre(0) == 0 && br.suf(n + 1) == 0);
    }
    cout << "large 1D: n=200000, two rounds of 200000 updates + queries passed\n";
}

struct Rect
{
    int x1, y1, x2, y2;
};
Rect random_rect(mt19937& rng, int n, int m)
{
    Rect r{1 + (int)(rng() % n), 1 + (int)(rng() % m),
           1 + (int)(rng() % n), 1 + (int)(rng() % m)};
    if (r.x1 > r.x2) swap(r.x1, r.x2);
    if (r.y1 > r.y2) swap(r.y1, r.y2);
    return r;
}
LL overlap(Rect a, Rect b)
{
    return (LL)max(0, min(a.x2, b.x2) - max(a.x1, b.x1) + 1)
             * max(0, min(a.y2, b.y2) - max(a.y1, b.y1) + 1);
}

void test_large_2d()
{
    constexpr int N = 2000;
    BIT2D t(N, N);
    mt19937 rng(42);
    // 在线交错修改/查询, 独立答案为 12 个矩形与查询范围的交集面积
    for (auto [n, m] : vector<pair<int, int>>{{N,N},{1,N},{N,1},{1,1},{N,N}})
    {
        t.init(n, m);
        vector<Rect> basis{{1,1,n,m}, {1,1,1,m}, {n,1,n,m},
                           {1,1,n,1}, {1,m,n,m}, {n,m,n,m}};
        while (basis.size() < 12) basis.push_back(random_rect(rng, n, m));
        vector<LL> weight(12);
        int q = n == N && m == N ? 100000 : 1000;
        for (int i = 0; i < q; ++i)
        {
            int id = rng() % basis.size();
            LL v = (int)(rng() % 2001) - 1000;
            auto a = basis[id];
            t.add(a.x1, a.y1, a.x2, a.y2, v);
            weight[id] += v;
            auto b = random_rect(rng, n, m);
            if (i % 7 == 0) b = basis[i % 12];
            LL want = 0;
            for (int j = 0; j < 12; ++j) want += weight[j] * overlap(basis[j], b);
            assert(t.query(b.x1, b.y1, b.x2, b.y2) == want);
        }
    }
    // 任意矩形批量更新, 用普通二维差分还原所有格子, 再建静态前缀和
    t.init(N, N);
    const int stride = N + 2;
    vector<LL> ps((size_t)stride * stride);
    auto at = [&](int x, int y) -> LL& { return ps[(size_t)x * stride + y]; };
    for (int i = 0; i < 100000; ++i)
    {
        auto r = random_rect(rng, N, N);
        LL k = (int)(rng() % 2001) - 1000;
        t.add(r.x1, r.y1, r.x2, r.y2, k);
        at(r.x1,r.y1) += k; at(r.x2+1,r.y1) -= k;
        at(r.x1,r.y2+1) -= k; at(r.x2+1,r.y2+1) += k;
    }
    for (int pass = 0; pass < 2; ++pass)
        for (int x = 1; x <= N; ++x)
            for (int y = 1; y <= N; ++y)
                at(x,y) += at(x-1,y) + at(x,y-1) - at(x-1,y-1);
    for (int i = 0; i < 100000; ++i)
    {
        auto r = random_rect(rng, N, N);
        LL want = at(r.x2,r.y2) - at(r.x1-1,r.y2) - at(r.x2,r.y1-1) + at(r.x1-1,r.y1-1);
        assert(t.query(r.x1,r.y1,r.x2,r.y2) == want);
    }
    assert(t.query(1,1,N,N) == at(N,N));
    cout << "large 2D: 2000x2000, online basis + arbitrary offline rectangles passed\n";
}

void test_numeric_and_empty()
{
    BIT empty; BITR reverse_empty; BIT2D grid_empty;
    empty.init(0); reverse_empty.init(0); grid_empty.init(0, 0);
    assert(empty.pre(0) == 0 && reverse_empty.suf(1) == 0);
    // 差分取负和坐标乘积也必须在 LL 内, 不把 LLONG_MIN 当合法增量
    const LL k = numeric_limits<LL>::max() / 32;
    BIT a(2); BITR b(2); BIT2D c(2, 2);
    a.add(1,2,3); b.add(1,2,3); c.add(1,1,2,2,3);
    assert(a.query(1,2) == 6 && b.query(1,2) == 6 && c.query(1,1,2,2) == 12);
    for (LL v : {k, -k})
    {
        a.init(2); b.init(2); c.init(2,2);
        a.add(2,2,v); b.add(2,2,v); c.add(2,2,2,2,v);
        assert(a.query(2,2) == v && b.query(2,2) == v && c.query(2,2,2,2) == v);
        a.add(2,2,-v); b.add(2,2,-v); c.add(2,2,2,2,-v);
        assert(a.query(1,2) == 0 && b.query(1,2) == 0 && c.query(1,1,2,2) == 0);
    }
}

int main()
{
    test_boundaries();
    test_bit_1d();
    test_bit_2d();
    test_numeric_and_empty();
    test_large_1d();
    test_large_2d();
    cout << "bit_check passed: BIT pre/suf + BIT2D matrix all tests ok\n";
    return 0;
}
