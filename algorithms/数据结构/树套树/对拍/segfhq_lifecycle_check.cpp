#include "../../../杂项/utils/utils.cpp"
namespace Legacy {
#include "../线段树套FHQ_Treap.cpp"
}

int main()
{
    mt19937 rng(42);
    Legacy::rnd.seed(42);
    for (int tc = 0; tc < 350; ++tc)
    {
        int n = 1 + rng() % 60;
        vector<int> a(n + 1);
        for (int i = 1; i <= n; ++i) Legacy::a[i] = a[i] = rng() % 101;
        Legacy::build(n);
        int initial_nodes = Legacy::idx;
        Legacy::build(n);
        assert(Legacy::idx == initial_nodes);
        for (int op = 0; op < 100; ++op)
        {
            if (rng() % 3 == 0)
            {
                int x = 1 + rng() % n, v = rng() % 101;
                Legacy::modify(1, 1, n, x, v);
                Legacy::a[x] = a[x] = v;
            }
            else
            {
                int l = 1 + rng() % n, r = 1 + rng() % n;
                if (l > r) swap(l, r);
                vector<int> b(a.begin() + l, a.begin() + r + 1);
                sort(b.begin(), b.end());
                int k = 1 + rng() % b.size(), x = rng() % 103;
                assert(Legacy::query_kth(l, r, k) == b[k - 1]);
                auto lo = lower_bound(b.begin(), b.end(), x), hi = upper_bound(b.begin(), b.end(), x);
                assert(Legacy::query_rank(1, 1, n, l, r, x) == lo - b.begin());
                assert(Legacy::query_pre(1, 1, n, l, r, x) == (lo == b.begin() ? -Legacy::inf : *prev(lo)));
                assert(Legacy::query_suf(1, 1, n, l, r, x) == (hi == b.end() ? Legacy::inf : *hi));
            }
        }
        Legacy::build(n);
        for (int i = 1; i <= n; ++i) assert(Legacy::query_kth(i, i, 1) == a[i]);
    }
    for (int n : {200000, 1, 0, 257, 200000})
    {
        for (int i = 1; i <= n; ++i) Legacy::a[i] = i;
        Legacy::build(n);
        assert(Legacy::n == n && Legacy::idx < 40 * Legacy::N);
        if (!n) { assert(Legacy::idx == 0 && Legacy::root[1] == 0); continue; }
        for (int k : {1, (n + 1) / 2, n}) assert(Legacy::query_kth(1, n, k) == k);
        Legacy::modify(1, 1, n, n, 0); Legacy::a[n] = 0;
        assert(Legacy::query_kth(1, n, 1) == 0);
    }
    cout << "segfhq_lifecycle_check passed: 350x100 independent sorted-array cases, 200000-1-0-257-200000 rebuild\n";
}
