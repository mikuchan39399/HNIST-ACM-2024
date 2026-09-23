#include "utils.h"

void solve()
{
    int n; cin >> n;
    VLL a(n + 1, 0);
    for (int i = 1; i <= n; i++) cin >> a[i];
    VLL c(n + 1, 0);
    LL ans = INF;
    if (n == 1)
    {
        cout << a[1] << endl;
        return;
    }
    for (int i = 1; i < n; i++)
    {
        c[i] = a[i] + a[i + 1];
    }
    LL sum = 0;
    for (int i = 1; i < n; i++)
    {
        sum += c[i];
        LL tmp = sum + max(0, (n - 1 - i)) * c[i];
        ans = min(ans, tmp);
    }
    sum = 0;
    c[1] = a[n] + a[1];
    for (int i = 2; i < n; i++)
    {
        c[i] = a[n - i + 1] + a[n - i + 2];
    }
    for (int i = 1; i < n; i++)
    {
        sum += c[i];
        LL tmp = sum + max(0, (n - 1 - i)) * c[i];
        ans = min(ans, tmp);

    }
    cout << ans + a[1] << endl;
}

int main()
{
    fast_io();
    int t = 1;
    cin >> t;
    while (t--)
    {
        solve();
    }
    return 0;
}