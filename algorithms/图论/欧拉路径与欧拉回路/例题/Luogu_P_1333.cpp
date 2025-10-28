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


const int N = 5e5 + 10;
unordered_map<string, int> mp;
int fa[N];
int n;
int d[N];
using PSS = pair<string, string>;
vector<PSS> arr;
int find(int x)
{
    if(fa[x] == x) return x;
    else return fa[x] = find(fa[x]);
}

void uni(int a, int b)
{
    fa[find(b)] = find(a);
}

void solve()
{
    string u, v;
    while(cin >> u >> v)
    {
        arr.push_back({u, v});
        if(!mp.count(u))
        {
            mp.insert({u, ++n});
        }
        if(!mp.count(v))
        {
            mp.insert({v, ++n});
        }
        d[mp[u]]++;
        d[mp[v]]++;
    }
    int cc{0};
    for(int i = 1; i <= n; i++)
    {
        fa[i] = i;
        if(d[i] % 2 == 1)
        {
            cc++;
        }
    }
    if(!(cc == 2 || cc == 0))
    {
        cout << "Impossible" << endl;
        return;
    }
    for(auto& [u, v] : arr)
    {
        uni(mp[u], mp[v]);
    }
    int uni_cnt = 0;
    for(int i = 1; i <= n; i++)
    {
        if(find(i) == i) uni_cnt++;
    }
    if(uni_cnt++ > 1)
    {
        cout << "Impossible" << endl;
        return;
    }
    cout << "Possible" << endl;

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