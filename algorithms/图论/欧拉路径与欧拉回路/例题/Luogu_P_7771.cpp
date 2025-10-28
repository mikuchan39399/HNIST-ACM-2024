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


const int N = 1e5 + 10;
VI edges[N];
int n, m;
int in[N], out[N];
VI ans;

void dfs(int u)
{
    while(edges[u].size())
    {
        int v = edges[u].back();
        edges[u].pop_back();
        dfs(v);
    }
    ans.push_back(u);
}

void solve()
{
    cin >> n >> m;
    for(int i = 1; i <= m; i++)
    {
        int u, v; cin >> u >> v;
        edges[u].push_back(v);
        out[u]++;
        in[v]++;
    }
    int s = 0;
    for(int i = 1; i <= n; i++)
    {
        if(out[i] - in[i] > 1)
        {
            cout << "No" << endl;
            return;
        }
        if(out[i] - in[i] == 1)
        {
            if(s)
            {
                cout << "No" << endl;
                return;
            }
            s = i;
        }
        sort(edges[i].begin(), edges[i].end(), greater<int>());
    }
    if(!s) s = 1;
    dfs(s);
    reverse(ans.begin(), ans.end());
    for(int u : ans)
    {
        cout << u << " ";
    }
    cout << endl;
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