// zoi: eulerLca
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace std;

using VI = vector<int>;
using VVI = vector<vector<int>>;

struct LCA
{
    int n;
    int idx = 0;
    const VI& head;
    const VI& to;
    const VI& nxt;
    VI dep, pre;
    VVI rmq;
    LCA(int _n, const VI& _head, const VI& _to, const VI& _nxt) : n(_n),
        head(_head), to(_to), nxt(_nxt)
    {
        dep.assign(n + 1, 0);
        pre.assign(n + 1, 0);
        rmq.assign(2 * n + 1, VI(floor(log2(2 * n)) + 10, 0));
    }
    // 从绑定的前向星重建 _n 点树, root 必须显式传入, _n 不超过构造容量
    // 时间 O(_n log _n) | 递归栈 O(h), 查询点须在 root 所在树内
    void build(int _n, int root)
    {
        n = _n;
        fill(dep.begin(), dep.begin() + n + 1, 0);
        fill(pre.begin(), pre.begin() + n + 1, 0);
        idx = 0;
        dfs(root, 0);
        for (int k = 1; (1 << k) <= idx; k++)
        {
            for (int i = 1; i + (1 << k) - 1 <= idx; i++)
            {
                int u = rmq[i][k - 1];
                int v = rmq[i + (1 << (k - 1))][k - 1];
                rmq[i][k] = dep[u] < dep[v] ? u : v;
            }
        }
    }

    int lca(int u, int v)
    {
        if (u == v) return u;
        int l = pre[u], r = pre[v];
        if (l > r) swap(l, r);
        int k = floor(log2(r - l + 1));
        u = rmq[l][k];
        v = rmq[r - (1 << k) + 1][k];
        return dep[u] < dep[v] ? u : v;
    }
private:
    void dfs(int u, int fa)
    {
        dep[u] = dep[fa] + 1;
        idx++;
        rmq[idx][0] = u;
        pre[u] = idx;
        for (int i = head[u]; i; i = nxt[i])
        {
            int v = to[i];
            if (v == fa) continue;
            dfs(v, u);
            idx++;
            rmq[idx][0] = u;
        }
    }
};

/* Usage
int main()
{
    VI head{0, 1, 2}, to{0, 2, 1}, nxt{0, 0, 0};
    LCA lc(2, head, to, nxt);
    lc.build(2, 1); // 点数、根都显式传入
    cout << lc.lca(1, 2) << '\n'; // 1
    lc.build(2, 2); // 换根重建, 无需手动清表
    cout << lc.lca(1, 2) << '\n'; // 2
}
*/
