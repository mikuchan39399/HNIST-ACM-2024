#include <iostream>
#include <cstring>
using namespace std;
int n, m;
const int N = 110, M = 4510;
int dp[N][N];
int main()
{
    cin >> n >> m;
    memset(dp, 0x3f, sizeof(dp));
    for(int i = 1; i <= n; i++) dp[i][i] = 0;
    for(int i = 1; i <= m; i++)
    {
        int u, v, w; cin >> u >> v >> w;
        dp[u][v] = dp[v][u] = min(dp[u][v], w);
    }

    for(int k = 1; k <= n; k++)
    {
        for(int i = 1; i <= n; i++)
        {
            for(int j = 1; j <= n; j++)
            {
                dp[i][j] = min(dp[i][j], 
                    dp[i][k] + dp[k][j]);
            }
        }
    }
    for(int i = 1; i <= n; i++)
        {
            for(int j = 1; j <= n; j++)
            {
                cout << dp[i][j] << " ";
            }
            cout << endl;
        }
    return 0;
}