// zoi: floyd
#include <iostream>
#include <algorithm>
using namespace std;
int n, m;
const int N = 110, M = 4510;
int dp[N][N];
int main()
{
    cin >> n >> m;
    // 只初始化本轮 1..n 的矩阵, 预留上限 N 不参与每轮清空。
    for (int i = 1; i <= n; i++) fill(dp[i] + 1, dp[i] + n + 1, 0x3f3f3f3f);
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
