#include <vector>
using namespace std;
using VI = vector<int>;

template <bool Directed = false, typename W = int>
struct Graph
{
    struct Edge { int to, nxt; W w; };
    VI head;
    vector<Edge> edges;
    int ecnt = 0;

    Graph(int n) : head(n + 1, -1) { edges.reserve(1); }

    void add_edge(int u, int v, W w = 1)
    {
        edges.push_back({v, head[u], w}); head[u] = ecnt++;
        if constexpr (!Directed)
        {
            edges.push_back({u, head[v], w}); head[v] = ecnt++;
        }
    }

    struct Adj
    {
        const Graph& g; int u;
        struct Iter
        {
            const Graph& g; int e;
            const Edge& operator*() const { return g.edges[e]; }
            Iter& operator++() { e = g.edges[e].nxt; return *this; }
            bool operator!=(const Iter& o) const { return e != o.e; }
        };
        Iter begin() const { return {g, g.head[u]}; }
        Iter end()   const { return {g, -1}; }
    };
    Adj operator[](int u) const { return {*this, u}; }
};