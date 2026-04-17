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
#include <bitset>

using namespace std;
using LL = long long;
using ULL = unsigned long long;
#define endl '\n'
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
const int inf = 0x3f3f3f3f;
const LL INF = 0x3f3f3f3f3f3f3f3f;

VI edges[N];
int n, m, w[N];
int sz[N], dfn[N], rnk[N], idx;
VVI f;

void dfs(int x)
{
    sz[x] = 1;
    dfn[x] = ++idx;
    rnk[idx] = x;
    for(int y : edges[x])
    {
        dfs(y);
        sz[x] += sz[y];
    }
}


void solve()
{
    cin >> n >> m;
    m++;
    for(int i = 1; i <= n; i++)
    {
        int k; cin >> k >> w[i];
        edges[k].push_back(i);
    }    
    dfs(0);
    f.resize(n + 10, VI(m + 10));
    for(int i = idx; i >= 1; i--)
    {
        int x = rnk[i];
        for(int j = 1; j <= m; j++)
        {
            f[i][j] = max(f[i + sz[x]][j], f[i + 1][j - 1] + w[x]);
        }
    }
    cout << f[1][m] << endl;
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