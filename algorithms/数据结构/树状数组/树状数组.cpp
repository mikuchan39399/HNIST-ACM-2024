// zoi: bit
#ifndef Z_OI_BIT
#define Z_OI_BIT

#include <vector>
#include <algorithm>
#include "../../杂项/utils/utils.cpp"

using namespace std;

// ============ 树状数组 区间加 + 区间和 (差分双 BIT) ============
// 1-based; 区间加 [l,r] 走差分 d[l]+=k, d[r+1]-=k, 前缀和
//   pre(x) = Σ d[i]*(x+1-i) = (x+1)*Σd[i] - Σd[i]*i,
//   b1/b2 两棵 BIT 分别维护 d 与 d*i
// 内存: 两棵 LL BIT 共 16B/位; 预算 = max_n, 2e5 ≈ 3.2MB
struct BIT
{
    int n;
    VLL b1, b2;
    // 创建长度为 max_n 的全零数组, 一次分配两张表, 合计约 16*(max_n+10) 字节
    // 时间: O(max_n) | 空间: 16B/位
    BIT(int max_n = 0) : n(max_n), b1(max_n + 10, 0), b2(max_n + 10, 0) {}
    // 清空数组并把本轮长度设为 _n, _n 不能超过构造时的 max_n
    // 时间: O(n) | 空间: O(1)
    void init(int _n)
    {
        n = _n;
        z_fill_n(n, 0, b1, b2);
    }
    // [l, r] 整体加 k (r = n 时差分右端越界, 内部循环自动丢弃)
    // 时间: O(log n) | 空间: O(1)
    void add(int l, int r, LL k)
    {
        upd(l, k);
        upd(r + 1, -k);
    }
    // 返回 a[1..x] 之和 (x = 0 返回 0)
    // 时间: O(log n) | 空间: O(1)
    LL pre(int x)
    {
        LL s1 = 0, s2 = 0;
        for (int i = x; i > 0; i -= i & -i) { s1 += b1[i]; s2 += b2[i]; }
        return (LL)(x + 1) * s1 - s2;
    }
    // 返回 a[l..r] 之和
    // 时间: O(log n) | 空间: O(1)
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

// 后缀树状数组: 支持区间加、后缀和与区间和, 下标为 1..n, 内部复用 BIT
struct BITR
{
    BIT t;
    // 创建长度为 max_n 的全零数组, 合计约 16*(max_n+10) 字节
    // 时间: O(max_n) | 空间: O(max_n)
    BITR(int max_n = 0) : t(max_n) {}
    // 清空数组并设置本轮长度 _n, _n 不能超过构造时的 max_n
    // 时间: O(n) | 空间: O(1)
    void init(int _n) { t.init(_n); }
    // [l, r] 整体加 k
    // 时间: O(log n) | 空间: O(1)
    void add(int l, int r, LL k) { t.add(t.n + 1 - r, t.n + 1 - l, k); }
    // 返回 a[x..n] 之和 (x 越上界返回 0)
    // 时间: O(log n) | 空间: O(1)
    LL suf(int x) { return t.pre(t.n + 1 - x); }
    // 返回 a[l..r] 之和
    // 时间: O(log n) | 空间: O(1)
    LL query(int l, int r) { return suf(l) - suf(r + 1); }
};
#endif
/*
 * Usage:
 * BIT bit(n);              // 预算 n
 * bit.init(n);             // 多测复位
 * bit.add(l, r, k);        // [l,r] 整体加 k
 * bit.pre(x);              // a[1..x] 之和
 * bit.query(l, r);         // a[l..r] 之和
 * BITR br(n);              // 后缀版(同款 add/init)
 * br.suf(x);               // a[x..n] 之和
 * br.query(l, r);          // a[l..r] 之和
 */
