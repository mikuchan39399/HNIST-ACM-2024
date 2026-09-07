// zoi: monoStack
#ifndef Z_OI_MONO_STACK
#define Z_OI_MONO_STACK

#include "../../杂项/utils/utils.cpp"

// 单调栈存候选下标, 弹栈比较决定大小关系, 剩余栈顶给出左侧最近位置
// 返回 a[1..n] 左侧最近满足关系的下标, 默认严格小于当前值, 不存在为 0
// 时间 O(n), 空间 O(n), 比较视作 O(1)
template<class T, class Bad = greater_equal<T>>
VI mono_stack(const vector<T>& a, Bad bad = {})
{
    int n = (int)a.size() - 1;
    VI stk(n + 1), ans(n + 1);
    int tt = 0;
    for (int i = 1; i <= n; ++i)
    {
        while (tt && bad(a[stk[tt]], a[i])) --tt; // 改弹栈关系, >= 弹同值而 > 留同值
        ans[i] = tt ? stk[tt] : 0; // 先取答案再入栈, 排除自己
        stk[++tt] = i;
    }
    return ans;
}

#endif

/* Usage:
#include "monoStack.h"

int main()
{
    VLL a{0, 3, 1, 1, 4};
    auto lt = mono_stack(a); // 左侧最近严格较小
    auto le = mono_stack(a, greater<LL>{}); // 左侧最近小于等于
    auto gt = mono_stack(a, less_equal<LL>{}); // 左侧最近严格较大
    cout << lt[3] << ' ' << le[3] << ' ' << gt[3] << '\n';
    cout << lt[4] << '\n';
}
*/
