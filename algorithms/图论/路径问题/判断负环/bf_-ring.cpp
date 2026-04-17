#include <iostream>
#include <cstring>
using namespace std;
const int N = 2e3 + 10, M = 3e3 + 10, INF = 0x3f3f3f3f;
int n, m;
int cnt;
struct node
{
    int x, y ,z;
}arr[2 * M];
int dist[N];

bool bf()
{
    memset(dist, 0x3f, sizeof(dist));
    dist[1] = 0;
    bool flag;
    for(int i = 1; i <= n; i++)
    {
        flag = false;
        for(int j = 1; j <= cnt; j++)
        {
            int pre = arr[j].x, next = arr[j].y, len = arr[j].z;
            if(dist[pre] == INF) continue;
            if(dist[pre] + len < dist[next])
            {
                flag = true;
                dist[next] = dist[pre] + len;
            }
        }
        if(flag == false) return false;
    }
    return true;
}
int main()
{
    int t; cin >> t;
    while(t--)
    {
        memset(arr, 0, sizeof(arr));
        cnt = 0;
        cin >> n >> m;
        for(int i = 1; i <= m; i++)
        {
            int u, v, w; cin >> u >> v >> w;
            if(w < 0)
            {
                cnt++;
                arr[cnt].x = u, arr[cnt].y = v, arr[cnt].z = w;
            }
            else
            {
                cnt++;
                arr[cnt].x = u, arr[cnt].y = v, arr[cnt].z = w;
                cnt++;
                arr[cnt].x = v, arr[cnt].y = u, arr[cnt].z = w;
            }
        }

        if(bf()) cout << "YES" << endl;
        else cout << "NO" << endl;
    }
    return 0;
}