// zoi: st
#ifndef Z_OI_ST
#define Z_OI_ST

#include "../../杂项/utils/utils.cpp"

// 静态区间最值; 先 build, query 要求合法非空区间. 空间 O(n log n)
struct ST
{
    int n;
    bool is_max;
    VVLL st;
    VI lg;
    // 预留 max_n, build 可自动扩容. 时空 O(max_n log max_n)
    ST(int max_n = 0) : n(0), is_max(true)
    {
        int lv = max_n > 1 ? __lg(max_n) + 1 : 1;
        st.assign(lv, VLL(max_n + 10, 0));
        lg.assign(max_n + 10, 0);
    }
    // 用 1-based 非空数组构建, true 最大 / false 最小. 时空 O(n log n)
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
    // 查询 [l,r] 最值. O(1)
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
