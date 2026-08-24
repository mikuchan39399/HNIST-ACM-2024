#include <iostream>
#include <vector>
using namespace std;
const int N = 1e4 + 10, M = 5e5 + 10, INF = 2147483647;
int n, m, s;
typedef pair<int, int> PII;
vector<PII> edges[N];
bool st[N];//标记已经确认最短路的点
int dist[N];

void dijkstra()
{
    for(int i = 0; i <= n; i++) dist[i] = INF;
    dist[s] = 0;
    for(int i = 1; i <= n - 1; i++)//确认n-1个点的最短路
    {
        int t = 0;
        for(int j = 1; j <= n; j++)//找到没被确认最短路的点中的dist_min
        {
            if(!st[j] && dist[j] < dist[t])
            {
                t = j;
            }
        }
        st[t] = true;//t最短路已确认

        for(auto& e : edges[t])//只要盯着这一轮确认最短路的点松弛就行了
        {
            int pos = e.first, len = e.second;
            dist[pos] = min(dist[pos], len + dist[t]);
        }
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

    dijkstra();

    return 0;
}