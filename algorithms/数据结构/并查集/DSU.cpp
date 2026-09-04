// zoi: dsu
#ifndef Z_OI_DSU
#define Z_OI_DSU

#include <vector>
#include "../../杂项/utils/utils.cpp"

using namespace std;

struct DSU
{
    int n;
    VI fa, sz;
    DSU(int max_n = 0) :
        n(max_n), fa(max_n + 10), sz(max_n + 10, 1)
    {
        for (int i = 0; i <= n; i++)
            fa[i] = i;
    }
    void init(int _n)
    {
        n = _n;
        for (int i = 0; i <= n; i++)
        {
            fa[i] = i;
            sz[i] = 1;
        }
    }
    int find(int x)
    {
        if (x == fa[x]) return x;
        return fa[x] = find(fa[x]);
    }
    bool merge(int u, int v)
    {
        int fu = find(u);
        int fv = find(v);
        if (fu == fv) return false;
        fa[fv] = fu;
        sz[fu] += sz[fv];
        return true;
    }
    bool same(int u, int v) { return find(u) == find(v); }
    int size(int x) { return sz[find(x)]; }
};
#endif
