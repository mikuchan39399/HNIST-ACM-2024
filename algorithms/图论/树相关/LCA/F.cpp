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
#include <random>
#include <chrono>

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

struct custom_hash
{
    static uint64_t splitmix64(uint64_t x)
    {
        x += 0x9e3779b97f4a7c15;
        x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
        x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
        return x ^ (x >> 31);
    }

    size_t operator()(uint64_t x) const
    {
        static const uint64_t FIXED_RANDOM = chrono::steady_clock::now().time_since_epoch().count();
        return splitmix64(x + FIXED_RANDOM);
    }
};

inline LL floor_div(LL a, LL b)
{
    LL res = a / b;
    LL rem = a % b;
    if (rem != 0 && ((a < 0) ^ (b < 0)))
    {
        res--;
    }
    return res;
}

inline LL ceil_div(LL a, LL b)
{
    LL res = a / b;
    LL rem = a % b;
    if (rem != 0 && ((a > 0) == (b > 0)))
    {
        res++;
    }
    return res;
}

#define debug(x) cerr << #x << " = " << (x) << endl
#define debug_array(a, n) cerr << #a << ": "; for(int i=1; i<=(n); ++i) cerr << a[i] << " "; cerr << endl

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
#define VVPII vector<vector<pair<int, int>>>

const int N = 1e5 + 10;
const int inf = 0x3f3f3f3f;
const LL INF = 0x3f3f3f3f3f3f3f3f;


int max_w = 0;
int n;
VI f[N];
VPII edges[N];
LL sum[N];
int ans[N];

void init(int n)
{
    max_w = 0;
    for (int i = 1; i <= n; i++)
    {
        edges[i].clear();
        sum[i] = 0;
        ans[i] = 0;
        f[i].clear();
    }
}

int safe_f(int u, int x)
{
    if (x < 0) return inf;
    if (x >= f[u].size()) return f[u].back();
    return f[u][x];
}

void dfs(int u, int fa)
{
    for (auto& [v, w] : edges[u])
    {
        if (v == fa) continue;
        dfs(v, u);
        max_w = max(max_w, w);
        sum[u] += sum[v] + w;
    }

    LL M = min(sum[u], 2ll * max_w);
    f[u].assign(M + 10, 0);
    for (auto& [v, w] : edges[u])
    {
        if (v == fa) continue;
        for (int x = 0; x <= M; x++)
        {
            int t1 = safe_f(v, x + w) + w;
            int t2 = inf;
            if (x >= w)
            {
                t2 = safe_f(v, x - w) - w;
            }
            f[u][x] = max(f[u][x], min(t1, t2));
            
        }
    }
    ans[u] = inf;
    for (int x = 0; x <= M; x++)
    {
        ans[u] = min(ans[u], x + f[u][x]);
    }
}

void solve()
{
    cin >> n;
    init(n);
    for (int i = 1; i < n; i++)
    {
        int u, v, w; cin >> u >> v >> w;
        edges[u].push_back({v, w});
        edges[v].push_back({u, w});
        max_w = max(max_w, w);
    }
    dfs(1, 0);
    for (int i = 1; i <= n; i++)
    {
        cout << ans[i] << " ";
    }
    cout << endl;
}

int main()
{
    fast_io();
    int t = 1;
    cin >> t;
    while (t--)
    {
        solve();
    }
    return 0;
}