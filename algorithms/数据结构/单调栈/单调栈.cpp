// zoi: monoStack
#ifndef Z_OI_MONO_STACK
#define Z_OI_MONO_STACK

#include "../../杂项/utils/utils.cpp"

// 返回每个位置左侧最近的严格更小值下标, 无则 0; a 为 1-based
// 比较 O(1) 时, 总时空 O(n). 改比较器可求其他关系
template<class T, class Bad = greater_equal<T>>
VI mono_stack(const vector<T>& a, Bad bad = {})
{
    int n = (int)a.size() - 1;
    VI stk(n + 1), ans(n + 1);
    int tt = 0;
    for (int i = 1; i <= n; ++i)
    {
        while (tt && bad(a[stk[tt]], a[i])) --tt; // >= 弹出同值, > 保留同值
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
