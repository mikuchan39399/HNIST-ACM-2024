// Graph 独立对拍: 逻辑边账本逐点扫描, 不复用前向星链表
#include <cassert>
#include <random>
#include <numeric>
#include <type_traits>
#include "../图的存储/Graph.cpp"

struct RefEdge
{
    int u, v;
    LL w;
};

template <bool Dir, class W>
void verify(Graph<Dir, W>& g, const vector<RefEdge>& ref, int n)
{
    vector<int> touched(n + 1), in(n + 1), out(n + 1);
    for (auto e : ref)
    {
        touched[e.u] = touched[e.v] = 1;
        out[e.u]++;
        in[e.v]++;
        if constexpr (!Dir) out[e.v]++;
    }
    assert(g.node_cnt() == accumulate(touched.begin(), touched.end(), 0));
    assert(g.edge_cnt() == (int)ref.size());
    assert(g.edges.size() == ref.size() * (Dir ? 1 : 2));
    vector<int> seen(n + 1);
    for (int u : g.used)
    {
        assert(1 <= u && u <= n && !seen[u]);
        seen[u] = 1;
    }
    assert(seen == touched);
    for (int u = 1; u <= n; u++)
    {
        assert(g.deg[u] == out[u]);
        if constexpr (Dir) assert(g.in_deg[u] == in[u]);
        vector<pair<int, int>> expected;
        for (int j = (int)ref.size() - 1; j >= 0; j--)
        {
            // 无向自环的反向半边后插入, 应先遍历到
            if constexpr (!Dir)
                if (ref[j].v == u) expected.push_back({ref[j].u, 2 * j + 1});
            if (ref[j].u == u) expected.push_back({ref[j].v, (Dir ? 1 : 2) * j});
        }
        int k = 0;
        for (auto& e : g[u])
        {
            assert(k < (int)expected.size());
            auto [v, id] = expected[k++];
            assert(e.v == v && g.id(e) == id);
            if constexpr (!is_same_v<W, Empty>)
                assert(e.w == ref[id / (Dir ? 1 : 2)].w);
            if constexpr (!Dir)
            {
                int opposite = id % 2 == 0 ? id + 1 : id - 1;
                assert(g.rev(id) == opposite);
                assert(g.edges[opposite].v == u);
            }
        }
        assert(k == (int)expected.size());
        assert(g.head[u] == (expected.empty() ? -1 : expected.front().second));
        if (!expected.empty())
        {
            auto it = g[u].begin();
            assert(it->v == expected.front().first);
        }
    }
}

template <bool Dir, class W>
void append(Graph<Dir, W>& g, vector<RefEdge>& ref, int u, int v, LL w)
{
    int id;
    if constexpr (is_same_v<W, Empty>) id = g.add(u, v);
    else id = g.add(u, v, w);
    assert(id == (int)ref.size() * (Dir ? 1 : 2));
    ref.push_back({u, v, w});
}

template <bool Dir, class W>
void test()
{
    static_assert(is_nothrow_move_constructible_v<Graph<Dir, W>>);
    static_assert(is_nothrow_move_assignable_v<Graph<Dir, W>>);
    mt19937 rng(42);
    static Graph<Dir, W> g(12, 0);
    vector<RefEdge> ref;
    verify(g, ref, 12);
    // 明确超过实际 capacity, 检查扩容后的编号、权值和链序
    size_t capacity = g.edges.capacity();
    int count = (int)capacity + 1;
    for (int i = 0; i < count; i++) append(g, ref, 1, 12, i - 20);
    assert(g.edges.capacity() > capacity);
    verify(g, ref, 12);
    for (int tc = 0; tc < 304; tc++)
    {
        size_t cap = g.edges.capacity();
        g.clear();
        g.clear();
        ref.clear();
        assert(g.edges.capacity() == cap);
        verify(g, ref, 12);
        int n = tc < 4 ? 1 : 1 + (int)(rng() % 12);
        int m = tc % 4 == 0 ? 0 : tc % 4 == 1 ? 1 : 60;
        for (int i = 0; i < m; i++)
        {
            int u = 1 + (int)(rng() % n), v = 1 + (int)(rng() % n);
            LL w = (int)(rng() % 101) - 50;
            append(g, ref, u, v, w);
            verify(g, ref, 12);
        }
        Graph<Dir, W> copy(g), assigned;
        assigned = g;
        verify(copy, ref, 12);
        verify(assigned, ref, 12);
        auto copy_ref = ref;
        append(copy, copy_ref, 12, 12, -7);
        verify(copy, copy_ref, 12);
        verify(g, ref, 12);
        if constexpr (!is_same_v<W, Empty>)
        {
            // 边权可经邻接引用修改; 无向的两条半边独立存权
            auto it = copy[12].begin();
            int id = copy.id(*it);
            it->w = 123;
            assert(copy.edges[id].w == 123);
            if constexpr (!Dir) assert(copy.edges[copy.rev(id)].w == -7);
            verify(g, ref, 12);
        }
        copy.clear();
        verify(g, ref, 12);
        Graph<Dir, W> moved(move(assigned)), moved_assigned;
        moved_assigned = move(moved);
        verify(moved_assigned, ref, 12);
        auto& alias = moved_assigned;
        moved_assigned = alias;
        verify(moved_assigned, ref, 12);
    }
}

int main()
{
    test<true, Empty>();
    test<false, Empty>();
    test<true, LL>();
    test<false, LL>();
    cout << "Graph core checks passed\n";
}
