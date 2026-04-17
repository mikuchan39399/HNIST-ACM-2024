#include <iostream>
#include <vector>
#include <queue>
#include <cstring>
using namespace std;
int n, m, s;
const int N = 1e5 + 10, M = 5e5 + 10, INF = 2147483647;
typedef long long LL;
typedef pair<int, int> PII;
LL dist[N];
vector<PII> edges[N];
queue<int> q;
bool st[N];//标记正在队列里的点

void spfa()
{
    for(int i = 0; i <= n; i++) dist[i] = INF;

    dist[s] = 0;
    q.push(s);
    st[s] = true;
    while(q.size())
    {
        int j = q.front(); q.pop();
        st[j] = false;
        for(auto& e : edges[j])
        {
            int u = e.first, w = e.second;
            if(dist[j] + w < dist[u]) //只有上一轮被松弛过的点，这一轮才有可能松弛，用队列优化bf算法
            {
                dist[u] = dist[j] + w;
                if(!st[u])//队列里不要出现重复的点
                {
                    q.push(u);
                    st[u] = true;
                }

            }
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
    spfa();
    return 0;
}