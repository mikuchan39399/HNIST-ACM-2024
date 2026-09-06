// zoi: st
#ifndef Z_OI_ST
#define Z_OI_ST

#include "../../杂项/utils/utils.cpp"

// ST 表按二次幂长度预存静态区间最值, 查询用两段覆盖; max/min 重复合并不改变结果
// build 后修改原数组不影响表; max/min 同时查询需两个实例, query 要求 1 <= l <= r <= n
// 每点 8 * (floor(log2 n) + 1) + 4 B, n=2e5 约 29.6 MB; 缩小重建保留原容量
struct ST
{
    int n;
    bool is_max;
    VVLL st;   // st[k][i] = [i, i + 2^k) 的最值
    VI lg;     // lg[x] = floor(log2 x)
    // 预分配 max_n 点所需的表, build 更大数组时自动扩容
    // 时间: O(max_n log max_n) | 空间: O(max_n log max_n)
    ST(int max_n = 0) : n(0), is_max(true)
    {
        int lv = max_n > 1 ? __lg(max_n) + 1 : 1;
        st.assign(lv, VLL(max_n + 10, 0));
        lg.assign(max_n + 10, 0);
    }
    // 用非空 a[1..m] 替换旧表, max_mode 为 true 求最大值, false 求最小值
    // 时间: O(m log m) | 空间: O(m log m), 原容量足够时复用
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

/* Usage:
int main()
{
    VLL a = {0, 5, -2, 7};
    ST mx, mn;
    mx.build(a);
    mn.build(a, false);
    cout << mx.query(1, 3) << " " << mn.query(1, 3) << "\n"; // 7 -2
    mx.build(VLL{0, -9});            // 同一实例重建
    cout << mx.query(1, 1) << "\n"; // -9
}
*/
