#include <iostream>
#include <vector>

using namespace std;
int n, m, s;
const int N = 1e4 + 10, M = 5e5 + 10, INF = 2147483647;
typedef long long LL;
typedef pair<int, int> PII;
LL dist[N];
vector<PII> edges[N];

void bf()
{
    for(int i = 0; i <= n; i++)
    {
        dist[i] = INF;
    }

    dist[s] = 0;
    for(int i = 1; i <= n - 1; i++)
    {
        bool flag = false;
        for(int j = 1; j <= n; j++)//    bf算法每一轮要松弛所有边
        {
            flag = false;
            if(dist[j] == INF) continue;
            for(auto& e : edges[j])
            {
                int u = e.first, w = e.second;
                if(dist[j] + w < dist[u])
                {
                    dist[u] = dist[j] + w;
                    flag = true;
                }
            }
        }
        if(!flag) break;
    }

    for(int i = 1; i <= n; i++)
    {
        cout << dist[i] << " ";
    }
}
int main()
{
    cin >> n >> m >> s;
    
    for(int i = 1; i <= m; i++)
    {
        int u, v, w; cin >> u >> v >> w;
        edges[u].push_back({v, w});
    }
    bf();
    return 0;
}