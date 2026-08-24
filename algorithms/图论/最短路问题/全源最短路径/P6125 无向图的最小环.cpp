#include <iostream>
#include <cstring>
using namespace std;
int n, m;
const int N = 110, INF = 1e8;

int edges[N][N];
int dp[N][N];
int main()
{
    cin >> n >> m;
    for(int i = 1; i <= n; i++)
    {
        for(int j = 1; j <= n; j++)
        {
            dp[i][j] = edges[i][j] = INF;
        }
    }
    for(int i = 1; i <= n; i++) dp[i][i] = edges[i][i] = 0;

    for(int i = 1; i <= m; i++)
    {
        int u, v, d; cin >> u >> v >> d;
        dp[u][v] = dp[v][u] = edges[u][v] = edges[v][u] = min(dp[u][v], d);
    }

    int ret = INF;
    for(int k = 1; k <= n; k++)//枚举k
    {
        for(int i = 1; i < k; i++)//在一个k下，先判断最小环，再更新最短路
        {
            for(int j = i + 1; j < k; j++)//i和j要小于k且不重复，i和j的桥梁只能是当前k的滞后一步
            {
                ret = min(ret, dp[i][j] + edges[i][k] + edges[k][j]);
            }
        }

        for(int i = 1; i <= n; i++)
        {
            for(int j = 1; j <= n; j++)
            {
                dp[i][j] = min(dp[i][j], dp[i][k] + dp[k][j]);//bug
            }//0x3f累加太多会溢出成负数
        }
    }
    if(ret == INF) cout << "No solution." << endl;
    else cout << ret << endl;
    return 0;
}