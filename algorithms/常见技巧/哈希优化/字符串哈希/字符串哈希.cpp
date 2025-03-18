#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long ULL;
const int N = 1e4 + 10;
int P = 131;

ULL a[N];
ULL f[N];

ULL get_hash(string s)
{
    ULL ret = 0;
    for(int i = 0; i < s.size(); i++)
    {
        ret = ret * P + s[i];
    }
    return ret;
}
int main()
{
    int n; cin >> n;
    for(int i = 1; i <= n; i++)
    {
        string s; cin >> s;
        a[i] = get_hash(s);
    }

    ULL ret = 1;
    sort(a + 1, a + 1 + n);
    for(int i = 2; i <= n; i++)
    {
        if(a[i] != a[i - 1]) ret++;
    }
    cout << ret << endl;
    return 0;
}