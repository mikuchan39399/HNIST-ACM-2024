#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cstring>
#include <unordered_set>
#include <stack>
#include <queue>
#include <deque>
#include <cmath>
#include <map>
#include <set>
#include <list>

using namespace std;
using LL = long long;
using ULL = unsigned long long;
void fast_io()
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);
}
// LL read()
// {
//     LL ret = 0;
//     int flag = 1;
//     char ch = getchar_unlocked(); // 只在Linux里能用这个

//     while (ch < '0' || ch > '9')
//     {
//         if (ch == '-')
//             flag = -1;
//         ch = getchar_unlocked();
//     }

//     while (ch >= '0' && ch <= '9')
//     {
//         ret = ret * 10 + ch - '0';
//         ch = getchar_unlocked();
//     }

//     return ret * flag;
// }
// void print(LL x)
// {
//     if (x < 0)
//     {
//         putchar('-');
//         x = -x;
//     }
//     if (x > 9)
//         print(x / 10);
//     putchar(x % 10 + '0');
// }

int dx4[4] = {0, 0, -1, 1};
int dy4[4] = {1, -1, 0, 0};
int dx8[8] = {-1, -1, -1, 0, 1, 1, 1, 0};
int dy8[8] = {-1, 0, 1, 1, 1, 0, -1, -1};

// 一些常用的宏定义
#define PII pair<int, int>
#define PLL pair<LL, LL>
#define TIII tuple<int, int, int>
#define TLLL tuple <LL, LL, LL>
#define VVI vector<vector<int>>
#define VVLL vector<vector<LL>>
#define VI vector<int>
#define VLL vector<LL>
#define VPII vector<pair<int, int>>
#define VPLL vector<pair<LL, LL>>


const int N = 1e5 + 10, M = 1e6 + 10;
using SI = stack<int>;
bool st[N]; //标记点是否在tmp栈中
int n, m;
VPII edges[N];
int d[N];  //度
bool mark[N]; //标记整个图的节点是否已遍历
bool vis[M]; //标记边是否已经被删除
int cnt;
VI tmp;
VI ret[M];

void dfs(int u)
{
    mark[u] = true;
    while(edges[u].size())
    {
        auto [v, id] = edges[u].back();
        edges[u].pop_back();
        if(vis[id]) continue;
        vis[id] = true;
        dfs(v);
    }
    if(st[u])
    {
        cnt++;
        ret[cnt].push_back(u);
        while(tmp.size() && tmp.back() != u)
        {
            int r = tmp.back();
            tmp.pop_back();
            st[r] = false;
            ret[cnt].push_back(r);
        }
        ret[cnt].push_back(u);
    }
    else
    {
        tmp.push_back(u);
        st[u] = true;
    }
}

void solve()
{
    cin >> n >> m;
    for(int i = 1; i <= m; i++)
    {
        int u, v, s, t; cin >> u >> v >> s >> t;
        if(s == t) continue;
        edges[u].push_back({v, i});
        edges[v].push_back({u, i});
        d[u]++;
        d[v]++;
    }
    for(int i = 1; i <= n; i++)
    {
        if(d[i] & 1)
        {
            cout << "NIE" << endl;
            return;
        }
    }
    for(int i = 1; i <= n; i++)
    {
        if(d[i] && !mark[i])
        {
            dfs(i);
        }
    }
    cout << cnt << endl;
    for(int i = 1; i <= cnt; i++)
    {
        cout << ret[i].size() - 1 << " ";
        while(ret[i].size())
        {
            cout << ret[i].back() << " ";
            ret[i].pop_back();
        }
        cout << endl;
    }
}

int main()
{
    fast_io();
    int t = 1;
    
    while (t--)
    {
        solve();
    }
    return 0;
}