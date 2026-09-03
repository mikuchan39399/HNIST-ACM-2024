// ============ bit_check BIT/BITR/BIT2D 回归套件 ============
// 覆盖: BIT 区间加+前缀/区间查 | BITR 区间加+后缀查(坐标镜像) |
//       BIT2D 矩阵加+矩阵查; 全部对拍朴素数组, static 实例跨组 init 复用
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

int main()
{
    test_bit_1d();
    test_bit_2d();
    cout << "bit_check passed: BIT pre/suf + BIT2D matrix all tests ok\n";
    return 0;
}
