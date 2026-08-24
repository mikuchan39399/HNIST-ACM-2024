#include <iostream>
#include <vector>
#include <queue>
#include <cstring>
using namespace std;
int n, m;
const int N = 2e3 + 10, M = 3e3 + 10, INF = 0x3f3f3f3f;
typedef pair<int, int> PII;
vector<PII> edges[N];
int dist[N];
int cnt[N]; // 关键, 用来标记从起点走了多少步到此点
queue<int> q;

bool spfa()
{
    memset(dist, 0x3f, sizeof(dist));
    memset(cnt, 0, sizeof(cnt));
    while(q.size()) q.pop();
    dist[1] = 0;
    q.push(1);
    while(q.size())
    {
        int t = q.front(); q.pop();
        for(auto& e : edges[t])
        {
            int next = e.first, len = e.second;
            if(dist[t] + len < dist[next])
            {
                dist[next] = dist[t] + len;
                q.push(next);
                cnt[next] = cnt[t] + 1;
                if(cnt[next] >= n) return true; // 边数 大于等于 点数说明有负环
            }
        }
    }
    return false;
}

int main()
{
    int t; cin >> t;
    while(t--)
    {
        memset(edges, 0, sizeof(edges));
        cin >> n >> m;
        for(int i = 1; i <= m; i++)
        {
            int u, v, w; cin >> u >> v >> w;
            edges[u].push_back({v, w});
            if(w >= 0) edges[v].push_back({u, w});
        }

        if(spfa()) cout << "YES" << endl;
        else cout << "NO" << endl;
    }
    return 0;
}