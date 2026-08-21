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
#include <cassert>
#include <array>
#include <type_traits>
#include <tuple>

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
const double INF = 1e15;

template <class T = LL, class Comp = less<T>, class Pay = int>
struct PersistentLeftist
{
    int tot = 0;
    VI lc, rc, dist;
    vector<T> val;
    vector<Pay> pay;
    PersistentLeftist(int max_nodes = 0) :
        lc(max_nodes + 10), rc(max_nodes + 10), dist(max_nodes + 10, -1),
        val(max_nodes + 10),
        pay(max_nodes + 10)
    { 
    }
    void init() { tot = 0; }
private:
    int clone(int p)
    {
        int q = ++tot;
        assert(q < (int)val.size() && "max_nodes 估算不足");
        lc[q] = lc[p]; rc[q] = rc[p]; dist[q] = dist[p];
         val[q] = val[p]; pay[q] = pay[p];
        return q;
    }
public:
    // 生成一个只含单点的新堆，返回物理节点编号
    // 时间: O(1) | 空间: O(1)
    int new_node(T v, Pay p = Pay())
    {
        int q = ++tot;
        assert(q < (int)val.size() && "max_nodes 估算不足");
        lc[q] = rc[q] = 0; dist[q] = 0;
        val[q] = v; pay[q] = p;
        return q;
    }
    // 合并堆 x 和 y，返回新版本堆根编号
    // 时间: O(log N) | 空间: O(log N)
    int merge(int x, int y) 
    {
        if (!x || !y) return x | y;
        if (Comp()(val[y], val[x])) swap(x, y);
        int c = clone(x);
        rc[c] = merge(rc[c], y);
        if (dist[rc[c]] > dist[lc[c]]) swap(lc[c], rc[c]);
        dist[c] = dist[rc[c]] + 1;
        return c;
    }
    // 直接合并 x 和 y, 覆盖 x 堆
    // 时间: O(log N) | 空间: O(1)
    int merge_raw(int x, int y)
    {
        if (!x || !y) return x | y;
        if (Comp()(val[y], val[x])) swap(x, y);
        rc[x] = merge_raw(rc[x], y);
        if (dist[rc[x]] > dist[lc[x]]) swap(lc[x], rc[x]);
        dist[x] = dist[rc[x]] + 1;
        return x;
    }
    // 在堆 rt 的基础上插入 v，返回新版本堆根编号
    // 时间: O(log N) | 空间: O(log N)
    int insert(int rt, T v, Pay p = Pay()) { return merge(rt, new_node(v, p)); }
    // 弹出堆 rt 的根节点，返回新版本堆根编号
    // 时间: O(log N) | 空间: O(log N)
    int pop(int rt) { return merge(lc[rt], rc[rt]); }
    // --- 查询接口 (入参全为某版本的堆根 rt) ---
    // 时间: O(1) | 空间: O(1)
    bool empty  (int rt) const { return rt == 0; }
    T    top    (int rt) const { return val[rt]; }
    Pay  top_pay(int rt) const { return pay[rt]; }
};
struct Empty {};
template <bool Dir = false, class W = Empty>
struct Graph 
{
    struct Edge 
    {
        int v, nxt;                 
        [[no_unique_address]] W w;  
    };
    VI head, used;
    VI deg;     // 无向图的度数 / 有向图的出度
    VI in_deg;  // 有向图的入度(仅 Dir = true 时有效) 
    vector<Edge> edges;     
    Graph(int max_n = 0, int max_m = 0) : 
        head(max_n + 10, -1), deg(max_n + 10, 0)
    {
        if constexpr (Dir) in_deg.assign(max_n + 10, 0);
        used.reserve(max_n + 10);
        edges.reserve(max_m * (Dir ? 1 : 2) + 10); 
    }
    void clear() // O(used) 擦除, 多测复用
    {
        for (size_t i = 0; i < used.size(); i++)
        {
            int u = used[i];
            head[u] = -1;
            deg[u] = 0;
            if constexpr (Dir) in_deg[u] = 0;
        }
        used.clear();
        edges.clear();
    }
    int add(int u, int v, const W& w = W()) 
    {
        auto mark = [&](int x)
        {
            bool first = (head[x] == -1) && (deg[x] == 0);
            if constexpr (Dir) first = first && (in_deg[x] == 0);
            if (first) used.push_back(x);
        };
        mark(u);
        if (u != v) mark(v);
        int idx = edges.size();
        edges.push_back({v, head[u], w});
        head[u] = idx;
        deg[u]++;
        if constexpr (!Dir) 
        {
            edges.push_back({u, head[v], w});
            head[v] = idx + 1;
            deg[v]++;
        }
        else in_deg[v]++;
        return idx;
    }
    int node_cnt() const { return used.size(); } // 触碰过的点数 
    int edge_cnt() const { return (int)edges.size() / (Dir ? 1 : 2); } // 逻辑边数
    int rev(int i) const { return i ^ 1; } // 无向半边 i 的对偶半边 ( Dir = true 时无意义)
    int id(const Edge& e) const { return &e - edges.data(); } // 只能对遍历中的活引用调用
    struct Iter 
    {
        Graph& g; int e;
        Edge& operator*() { return g.edges[e]; } 
        Edge* operator->() { return &g.edges[e]; }
        Iter& operator++() { e = g.edges[e].nxt; return *this; } 
        bool operator!=(const Iter& o) const { return e != o.e; }
    };
    struct Adj 
    {
        Graph& g; int u; 
        Iter begin() { return {g, g.head[u]}; } 
        Iter end() { return {g, -1}; } 
    };
    Adj operator[](int u) { return {*this, u}; }
};
VI tree_edge;
struct Dijkstra
    {
        struct node
        {};
        vector<double> dist;
        Dijkstra(int max_n = 0) : 
            dist(max_n + 10, INF)
        {}
        void init(int _n) 
        { 
            fill(dist.begin(), dist.begin() + _n + 10, INF); 
        }
        template <class G>
        void build(const VI& nodes, G& g)
        {
            priority_queue<pair<double, int>, vector<pair<double, int>>, greater<pair<double, int>>> heap;
            for (int s : nodes)
            {
                dist[s] = 0;
                heap.push({0, s});
            }
            while (heap.size())
            {
                auto [d, u] = heap.top();
                heap.pop();
                if (d > dist[u]) continue;
                for (auto& e : g[u])
                {
                    int v = e.v; double w = e.w;
                    if (dist[u] + w < dist[v])
                    {
                        dist[v] = dist[u] + w;
                        heap.push({dist[v], v});
                        tree_edge[v] = g.id(e);
                    }
                }
            }
        }
        template <class G>
        void build(int s, G& g) { build(VI{s}, g); }
    };

void solve()
{
    int n, m; double E; cin >> n >> m >> E;
    Graph<true, double> g{n, m}, rev_g{n, m};
    for (int i = 1; i <= m; i++)
    {
        int s, t; double e; cin >> s >> t >> e;
        g.add(s, t, e);
        rev_g.add(t, s, e);
    }
    tree_edge.assign(n + 1, -1);
    Dijkstra dj{n};
    dj.build(n, rev_g);
    VI is_spt(m, 0); 
    for (int i = 1; i <= n; i++)
    {
        if (i != n && dj.dist[i] != INF && tree_edge[i] != -1)
        {
            is_spt[tree_edge[i]] = 1;
        }
    }
    VI p(n + 1, 0);
    iota(p.begin() + 1, p.end(), 1);
    sort(p.begin() + 1, p.end(), [&](int a, int b) {
        return dj.dist[a] < dj.dist[b];
    });
    PersistentLeftist<double, less<double>, int> lt(m * 20);
    VI root(n + 1, 0);
    for (int i = 1; i <= n; i++)
    {
        int u = p[i];
        if (dj.dist[u] == INF) continue;
        if (u != n)
        {
            int nxt = g.edges[tree_edge[u]].v;
            root[u] = root[nxt];
            for (auto& e : g[u])
            {
                int v = e.v;
                if (is_spt[g.id(e)]) continue;
                if (dj.dist[v] == INF) continue;
                double d = e.w + dj.dist[v] - dj.dist[u];
                root[u] = lt.insert(root[u], d, v);
            }
        }
    }
    if (dj.dist[1] == INF || E < dj.dist[1])
    {
        cout << 0 << endl;
        return;
    }
    E -= dj.dist[1];
    LL ans = 1;
    priority_queue<pair<double, int>, vector<pair<double, int>>, greater<pair<double, int>>> heap;
    if (root[1]) heap.push({dj.dist[1] + lt.top(root[1]), root[1]});
    while(heap.size())
    {
        auto [cur_dist, rt] = heap.top(); heap.pop();
        if (E > cur_dist - 1e-8)
        {
            ans++;
            E -= cur_dist;
        }
        else break;
        int l = lt.lc[rt], r = lt.rc[rt];
        if (l) heap.push({cur_dist - lt.top(rt) + lt.top(l), l});
        if (r) heap.push({cur_dist - lt.top(rt) + lt.top(r), r});
        int nxt_rt = root[lt.top_pay(rt)];
        if (nxt_rt) heap.push({cur_dist + lt.top(nxt_rt), nxt_rt});
    }
    cout << ans << endl;
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