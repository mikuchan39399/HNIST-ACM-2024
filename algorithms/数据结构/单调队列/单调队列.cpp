// zoi: monoQueue
#ifndef Z_OI_MONO_QUEUE
#define Z_OI_MONO_QUEUE

#include "../../杂项/utils/utils.cpp"

// 滑窗最值下标, 默认最小值且同值取最右; a 为 1-based, k>=1
// 返回 1..n 的窗口答案, 窗口不足 k 时为 0. 比较 O(1) 时, 总时空 O(n)
template<class T, class Bad = greater_equal<T>>
VI mono_window(const vector<T>& a, int k, Bad bad = {})
{
    int n = (int)a.size() - 1;
    VI q(n + 1), ans(n + 1);
    int hh = 1, tt = 0;
    for (int i = 1; i <= n; ++i)
    {
        while (hh <= tt && q[hh] < i - k + 1) ++hh; // 窗口左边界
        while (hh <= tt && bad(a[q[tt]], a[i])) --tt; // 弹出被支配项, >= 时同值取右
        q[++tt] = i;
        if (i >= k) ans[i] = q[hh];
    }
    return ans;
}

// f[i]=cost[i]+min f[j], max(0,i-k)<=j<i; f[0]=0, k>=1
// cost 为 1-based, 返回 f[0..n]; 加法不溢出 LL. 时空 O(n)
VLL mono_dp(const VLL& cost, int k)
{
    int n = (int)cost.size() - 1;
    VI q(n + 2);
    VLL f(n + 1);
    int hh = 1, tt = 1;
    q[1] = 0;
    for (int i = 1; i <= n; ++i)
    {
        while (hh <= tt && q[hh] < i - k) ++hh;
        f[i] = f[q[hh]] + cost[i]; // 先转移再入队, 排除 i 自身
        while (hh <= tt && f[q[tt]] >= f[i]) --tt;
        q[++tt] = i;
    }
    return f;
}
#endif

/* Usage:
#include "monoQueue.h"

int main()
{
    VLL a{0, 3, 1, 1, 4};
    auto lo = mono_window(a, 3); // 最小值, 同值取最右
    auto hi = mono_window(a, 3, less_equal<LL>{});
    auto first = mono_window(a, 3, greater<LL>{}); // 最小值, 同值取最左
    cout << lo[3] << ' ' << a[lo[3]] << ' ' << first[3] << '\n';
    cout << hi[4] << ' ' << a[hi[4]] << '\n';
    auto f = mono_dp(VLL{0, 5, -2, 4, 1}, 2);
    cout << f[4] << '\n';
}
*/
