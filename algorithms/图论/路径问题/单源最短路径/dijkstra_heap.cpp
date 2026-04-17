#include <iostream>
#include <vector>
#include <queue>
#include <cstring>
using namespace std;
const int N = 1e5 + 10, M = 2e5 + 10, INF = 0x3f3f3f3f;
typedef long long LL;
typedef pair<LL, LL> PII;
struct Compare 
{
    bool operator()(PII& a, PII& b) 
    {
        return a.second > b.second;
    }
};
int n, m, s;
vector<PII> edges[N];
bool st[N];//标记已经确认最短路的点
LL dist[N];
priority_queue<PII, vector<PII>, Compare> heap;
void dijkstra()
{
    memset(dist, 0x3f, sizeof(dist));
    dist[s] = 0;
    heap.push({s, 0});

    while(heap.size())//用小根堆优化朴素算法"寻找没被确认最短路的dist_min点"这一步
    {
        int t = (heap.top()).first; heap.pop();
        if(st[t]) continue; // 如果堆顶元素已确定最短路则忽略
        st[t] = true;//t最短路已确认

        for(auto& e : edges[t])
        {
            int pos = e.first, len = e.second;
            if(dist[t] + len < dist[pos])
            {
                dist[pos] = dist[t] + len;
                heap.push({pos, dist[pos]});
            }
        }
    }
    for(int i = 1; i <= n; i++) cout << dist[i] << " ";
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