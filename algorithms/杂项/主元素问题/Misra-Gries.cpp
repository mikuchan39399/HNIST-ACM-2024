// zoi: misraGries
#ifndef Z_OI_MISRA_GRIES
#define Z_OI_MISRA_GRIES

#include "../utils/utils.cpp"

// 至多 k-1 个候选, 满员遇到新值时全减一, 再按原序列计数排除假候选
// 返回出现次数严格大于 floor(n/k) 的不同值, 顺序不保证; k<2 返回空, 所有输入元素参与
// O(n min(k,n)) 时间, O(min(k,n)) 空间, n<=INT_MAX; 适合小 k, 不把候选计数当真实频次
inline VI majority_element_k(const VI& nums, int k)
{
    assert(nums.size() <= INT_MAX);
    int n = nums.size();
    if (k < 2) return {};
    VPII cands;
    for (const int x : nums)
    {
        bool matched = false;
        for (auto& p : cands)
        {
            if (p.first == x)
            {
                p.second++;
                matched = true;
                break;
            }
        }
        if (matched) continue;
        if ((int)cands.size() < k - 1)
        {
            cands.push_back({x, 1});
            continue;
        }
        for (auto& p : cands) p.second--;
        cands.erase(remove_if(cands.begin(), cands.end(), 
            [](const PII& p) { return p.second == 0; }), 
            cands.end());
    }
    VI res;
    for (auto& p : cands)
    {
        int cand = p.first;
        int cnt = 0;
        for (int x : nums)
            if (x == cand) cnt++;
        if (cnt > n / k) res.push_back(cand);
    }
    return res;
}
#endif

/* Usage
#include "misraGries.h"

int main()
{
    VI a{1, 2, 1, 3, 1, 2, 2}; // 所有元素参与, 没有下标 0 的占位值
    auto ans = majority_element_k(a, 3); // 找出现次数 > 7/3 的值
    sort(ans.begin(), ans.end());
    for (int x : ans) cout << x << ' '; // 1 2
    cout << '\n';
    // 按题修改阈值前先确认候选上限; 若只有流式单遍, 本实现的二次核验不可省略
    // k 很大时改用排序计数等方法, 该向量候选实现不承诺对数时间
}
*/
