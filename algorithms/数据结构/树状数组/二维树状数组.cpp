// zoi: bit2d
#ifndef Z_OI_BIT2D
#define Z_OI_BIT2D

#include "../../杂项/utils/utils.cpp"

// ============ 二维树状数组 矩阵加 + 矩阵和 (差分四 BIT) ============
// 1-based; 矩阵加走二维差分, 前缀和 pre(x,y) = Σ D[i][j]*(x+1-i)*(y+1-j)
//   展开成 (x+1)(y+1)*D - (y+1)*D*i - (x+1)*D*j + D*i*j,
//   t1/t2/t3/t4 四棵 BIT 分别维护 D / D*i / D*j / D*i*j
// 内存: 四棵 LL BIT 共 32B/格; 预算 = n*m, 2000×2000 ≈ 128MB
// 矩形合法非空; 增量取负、坐标乘积、表内累积与查询中间值均须在 LL 内
struct BIT2D
{
    int n, m;
    vector<VLL> t1, t2, t3, t4;
    // 创建 max_n 行 max_m 列的全零矩阵, 四张表的数据约占 32*(max_n+2)*(max_m+2) 字节
    // 时间: O(max_n*max_m) | 空间: 32B/格
    BIT2D(int max_n = 0, int max_m = 0) : n(max_n), m(max_m),
        t1(max_n + 2, VLL(max_m + 2, 0)), t2(max_n + 2, VLL(max_m + 2, 0)),
        t3(max_n + 2, VLL(max_m + 2, 0)), t4(max_n + 2, VLL(max_m + 2, 0))
    {}
    // 清空矩阵并设置本轮行列数, _n 和 _m 分别不能超过构造时的 max_n 和 max_m
    // 时间: O(n*m) | 空间: O(1)
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
    // 把矩形 [x1,x2] × [y1,y2] 整体加 k, 坐标在 1..n × 1..m 内
    // 时间: O(log n·log m) | 空间: O(1)
    void add(int x1, int y1, int x2, int y2, LL k)
    {
        upd(x1, y1, k);
        upd(x1, y2 + 1, -k);
        upd(x2 + 1, y1, -k);
        upd(x2 + 1, y2 + 1, k);
    }
    // 返回矩形 [x1,x2] × [y1,y2] 内的元素和
    // 时间: O(log n·log m) | 空间: O(1)
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
