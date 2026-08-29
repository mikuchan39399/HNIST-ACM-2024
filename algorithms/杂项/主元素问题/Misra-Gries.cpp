#include <vector>
#include <algorithm>
using namespace std;

using LL = long long;
using PII = pair<int, int>;
using VPII = vector<pair<int, int>>;
using VI = vector<int>;

// 返回输入序列中出现次数 > n / k 的序列
VI majority_element_k(const VI& nums, int k)
{
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