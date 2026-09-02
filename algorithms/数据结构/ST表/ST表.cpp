// zoi: st
#ifndef Z_OI_ST
#define Z_OI_ST

#include <vector>
#include <cassert>
#include "../../杂项/utils/utils.cpp"

using namespace std;

// ============ ST表 稀疏表 (静态区间最值, O(1) 查询) ============
// 建表后数组不可改; 同数组既要 max 又要 min, 开两个实例各建一次
// 值域约定: query 契约 1 <= l <= r <= n, 违约触发 assert
// 内存: 每点 8B × (log2 n + 1) ≈ 160B/点, 2e5 ≈ 32MB
struct ST
{
    int n;
    bool is_max;
    VVLL st;   // st[k][i] = [i, i + 2^k) 的最值
    VI lg;     // lg[x] = floor(log2 x)
    ST(int max_n = 0) : n(0), is_max(true)
    {
        int lv = max_n > 1 ? __lg(max_n) + 1 : 1;
        st.assign(lv, VLL(max_n + 10, 0));
        lg.assign(max_n + 10, 0);
    }
    // 用 a[1..m] 建表, max_mode 选问 max 还是 min, 替换现有表
    // 时间: O(m log m) | 空间: 见类头
    void build(const VLL& a, bool max_mode = true)
    {
        n = (int)a.size() - 1;
        is_max = max_mode;
        int lv = n > 1 ? __lg(n) + 1 : 1;
        if ((int)st.size() < lv || (int)st[0].size() < n + 1) st.assign(lv, VLL(n + 10, 0));
        if ((int)lg.size() < n + 1) lg.assign(n + 10, 0);
        lg[1] = 0;
        for (int i = 2; i <= n; i++) lg[i] = lg[i >> 1] + 1;
        for (int i = 1; i <= n; i++) st[0][i] = a[i];
        for (int k = 1; k < lv; k++)
            for (int i = 1; i + (1 << k) - 1 <= n; i++)
                st[k][i] = is_max ? max(st[k - 1][i], st[k - 1][i + (1 << (k - 1))])
                                  : min(st[k - 1][i], st[k - 1][i + (1 << (k - 1))]);
    }
    // 返回 [l, r] 的最值
    // 时间: O(1) | 空间: O(1)
    LL query(int l, int r)
    {
        assert(1 <= l && l <= r && r <= n);
        int k = lg[r - l + 1];
        return is_max ? max(st[k][l], st[k][r - (1 << k) + 1])
                      : min(st[k][l], st[k][r - (1 << k) + 1]);
    }
};
#endif
/*
 * Usage:
 * ST st(n);                       // 预算 n 个点, 不够自动扩
 * st.build(a);                    // a[1..n] 建表, 缺省问 max
 * st.build(a, false);             // 问 min; max+min 都要就开两个实例
 * st.query(l, r);                 // O(1) 区间最值
 * // 静态结构: 建表后改数组无效, 要改就重建
 */