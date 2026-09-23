#include "utils.h"
map<string, int> mp;
void solve()
{
    int n, p; cin >> n >> p;
    int sum = 0;
    for (int i = 1; i <= n; i++)
    {
        string s; cin >> s;
        if (s == "UnreasonableProblemArrangement") sum += 10;
        else if (s.empty()) { }
        else if (s.back() <= 'L' && s.back() >= 'A')
        {
            s.pop_back();
            if (mp.count(s)) sum += mp[s];
        }
        else
        {

        }
    }
    if (sum > p) cout << "Joker" << endl;
    else cout << "Judger" << endl;
}

int main()
{
    fast_io();
    mp.insert({"WrongProblem", 100});
    mp.insert({"SameProblem", 30});
    mp.insert({"UnreasonableLimitForProblem", 5});
    mp.insert({"WeakTestsForProblem", 3});
    mp.insert({"BadProblem", 1});
    int t = 1;
    cin >> t;
    while (t--)
    {
        solve();
    }
    return 0;
}