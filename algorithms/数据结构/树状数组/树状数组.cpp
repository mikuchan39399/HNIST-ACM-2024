// zoi: bit
#ifndef Z_OI_BIT
#define Z_OI_BIT

#include "../../杂项/utils/utils.cpp"

// 区间加/区间和树状数组, 1-based, 合法非空区间. 空间 O(n)
// 增量取负、坐标乘积及累加须在 LL 内; add/pre/query 时间 O(log n)
struct BIT
{
    int n;
    VLL b1, b2;
    // 预留并清零 n 个位置. 时空 O(n)
    BIT(int max_n = 0) : n(max_n), b1(max_n + 10, 0), b2(max_n + 10, 0) {}
    // 重置 n<=容量 个位置. 时间 O(n)
    void init(int _n)
    {
        n = _n;
        z_fill_n(n, 0, b1, b2);
    }
    // 给 [l,r] 加 v
    void add(int l, int r, LL k)
    {
        upd(l, k);
        upd(r + 1, -k);
    }
    // [1,x] 的和, x=0 返回 0
    LL pre(int x)
    {
        LL s1 = 0, s2 = 0;
        for (int i = x; i > 0; i -= i & -i) { s1 += b1[i]; s2 += b2[i]; }
        return (LL)(x + 1) * s1 - s2;
    }
    // [l,r] 的和
    LL query(int l, int r) { return pre(r) - pre(l - 1); }
private:
    void upd(int p, LL k)
    {
        for (int i = p; i <= n; i += i & -i)
        {
            b1[i] += k;
            b2[i] += (LL)p * k;
        }
    }
};

// 后缀版树状数组; 同 BIT 的限制与复杂度
struct BITR
{
    BIT t;
    // 预留并清零 n 个位置. 时空 O(n)
    BITR(int max_n = 0) : t(max_n) {}
    // 重置 n<=容量 个位置. 时间 O(n)
    void init(int _n) { t.init(_n); }
    // 给 [l,r] 加 v
    void add(int l, int r, LL k) { t.add(t.n + 1 - r, t.n + 1 - l, k); }
    // [x,n] 的和, 1<=x<=n+1, x=n+1 返回 0
    LL suf(int x) { return t.pre(t.n + 1 - x); }
    // [l,r] 的和
    LL query(int l, int r) { return suf(l) - suf(r + 1); }
};
#endif
/*
 * Usage:
 * int main()
 * {
 *     BIT bit(5);
 *     BITR br(5);
 *     bit.add(2, 4, 3);
 *     br.add(2, 4, 3);
 *     cout << bit.pre(3) << ' ' << br.suf(3) << endl; // 6 6
 *     cout << bit.query(3, 5) << ' ' << br.query(3, 5) << endl; // 6 6
 *     bit.init(3); // 新一轮长度为 3, 原数据清空
 *     br.init(3);
 *     cout << bit.query(1, 3) << ' ' << br.query(1, 3) << endl; // 0 0
 * }
 */
