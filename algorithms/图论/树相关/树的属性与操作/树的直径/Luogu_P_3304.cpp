#include <iostream>
#include <vector>
using namespace std;
using PII = pair<int, int>;
const int N = 2e5 + 10;
vector<PII> edges[N];
int pre[N];
int n, r;
using LL = long long;
LL d[N], ret;
bool st[N];

void dfs(int x, int fa)
{
    pre[x] = fa;
    if(d[x] > ret)
    {
        ret = d[x];
        r = x;
    }
    for(auto& [y, cost] : edges[x])
    {
        if(y == fa || st[y]) continue;
        d[y] = d[x] + cost;
        dfs(y, x);
    }
}

int main()
{
    LL L, R;
    cin >> n;
    for(int i = 1; i < n; i++)
    {
        int a, b, c; cin >> a >> b >> c;
        edges[a].push_back({b, c});
        edges[b].push_back({a, c});
    }
    dfs(1, 0); L = r, ret = d[r] = 0;
    dfs(L, 0); R = r;
    LL D = ret;
    cout << D << endl;
    for(int i = R; i; i = pre[i]) st[i] = true;
    for(int i = R; i; i = pre[i])
    {
        LL ld = d[i]; LL rd = D - d[i];
        d[i] = ret = 0;
        dfs(i, pre[i]);
        if(ret == rd) R = i;
        if(ret == ld)
        {
            L = i;
            break;
        }
    }
    int cnt = 0;
    for(int i = R; i != L; i = pre[i])
    {
        cnt++;
    }
    cout << cnt << endl;
    return 0;
}