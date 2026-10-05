// zoi: bit2d
#ifndef Z_OI_BIT2D
#define Z_OI_BIT2D

#include "../../杂项/utils/utils.cpp"

// 二维区间加/区间和树状数组, 1-based, 合法非空矩形. 空间 O(nm)
// 取负、坐标乘积及累加须在 LL 内; add/query 时间 O(log n log m)
struct BIT2D
{
    int n, m;
    vector<VLL> t1, t2, t3, t4;
    // 预留 max_n * max_m 并清零. 时空 O(max_n*max_m)
    BIT2D(int max_n = 0, int max_m = 0) : n(max_n), m(max_m),
        t1(max_n + 2, VLL(max_m + 2, 0)), t2(max_n + 2, VLL(max_m + 2, 0)),
        t3(max_n + 2, VLL(max_m + 2, 0)), t4(max_n + 2, VLL(max_m + 2, 0))
    {}
    // 重置 n*m, 不得超过预留行列数. 时间 O(nm)
    void init(int _n, int _m)
    {
        n = _n;
        m = _m;
        for (int i = 0; i <= n + 1; i++)
        {
            fill(t1[i].begin(), t1[i].begin() + m + 2, 0);
            fill(t2[i].begin(), t2[i].begin() + m + 2, 0);
            fill(t3[i].begin(), t3[i].begin() + m + 2, 0);
            fill(t4[i].begin(), t4[i].begin() + m + 2, 0);
        }
    }
    // 给 [x1,x2] * [y1,y2] 加 v
    void add(int x1, int y1, int x2, int y2, LL k)
    {
        upd(x1, y1, k);
        upd(x1, y2 + 1, -k);
        upd(x2 + 1, y1, -k);
        upd(x2 + 1, y2 + 1, k);
    }
    // 查询 [x1,x2] * [y1,y2] 的和
    LL query(int x1, int y1, int x2, int y2)
    {
        return pre(x2, y2) - pre(x1 - 1, y2) - pre(x2, y1 - 1) + pre(x1 - 1, y1 - 1);
    }
private:
    void upd(int x, int y, LL k)
    {
        for (int i = x; i <= n; i += i & -i)
            for (int j = y; j <= m; j += j & -j)
            {
                t1[i][j] += k;
                t2[i][j] += (LL)x * k;
                t3[i][j] += (LL)y * k;
                t4[i][j] += (LL)x * y * k;
            }
    }
    LL pre(int x, int y)
    {
        LL s = 0;
        for (int i = x; i > 0; i -= i & -i)
            for (int j = y; j > 0; j -= j & -j)
                s += (LL)(x + 1) * (y + 1) * t1[i][j]
                   - (LL)(y + 1) * t2[i][j]
                   - (LL)(x + 1) * t3[i][j]
                   + t4[i][j];
        return s;
    }
};
#endif
/*
 * Usage:
 * int main()
 * {
 *     BIT2D t(3, 4);
 *     t.add(1, 2, 3, 4, 2);
 *     t.add(3, 4, 3, 4, -5);
 *     cout << t.query(1, 1, 3, 4) << endl; // 13
 *     cout << t.query(3, 4, 3, 4) << endl; // -3
 *     t.init(1, 4); // 新一轮 1 行 4 列, 原数据清空
 *     cout << t.query(1, 1, 1, 4) << endl; // 0
 * }
 */
