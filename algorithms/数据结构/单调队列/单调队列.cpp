// zoi: monoQueue
#ifndef Z_OI_MONO_QUEUE
#define Z_OI_MONO_QUEUE

#include "../../杂项/utils/utils.cpp"

// 单调队列存候选下标, 队头控制有效范围, 队尾比较决定最值与平手取舍
// 返回 a[1..n] 各完整 k 窗口的最值下标, 默认最小且同值取最右, 未满窗口为 0; k >= 1
// 时间 O(n), 空间 O(n), 比较视作 O(1)
template<class T, class Bad = greater_equal<T>>
VI mono_window(const vector<T>& a, int k, Bad bad = {})
{
    int n = (int)a.size() - 1;
    VI q(n + 1), ans(n + 1);
    int hh = 1, tt = 0;
    for (int i = 1; i <= n; ++i)
    {
        while (hh <= tt && q[hh] < i - k + 1) ++hh; // 改窗口左端点
        while (hh <= tt && bad(a[q[tt]], a[i])) --tt; // 改支配关系, 含平手取舍
        q[++tt] = i;
        if (i >= k) ans[i] = q[hh]; // 当前点也属于窗口, 先入队再取答案
    }
    return ans;
}

// 计算 f[i] = cost[i] + min(f[j]), max(0, i-k) <= j < i, 返回 f[0..n] 且 f[0] = 0; k >= 1
// 时间 O(n), 空间 O(n), cost 为 1-based 且所有加法在 LL 内
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
        f[i] = f[q[hh]] + cost[i]; // 改转移式, 必须在 i 入队前查询
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
