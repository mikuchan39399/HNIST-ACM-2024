// zoi: leftist
#ifndef Z_OI_LEFTIST_TREE
#define Z_OI_LEFTIST_TREE

#include "../../../杂项/utils/utils.cpp"

// 左偏堆, 逻辑编号稳定; less 小根 / greater 大根, 同值取小编号
// heap_add/heap_mul 后禁用 get_val/set_val/add_val/erase; heap_mul 要求正数; 运算不溢出 T
// h=查根路径长, P=累计物理点, d=本次清理死点数; 查根单次不保证对数
// 点池 P<=初始点+insert/set_val/add_val 次数; LL 每点约 65B, 另每逻辑点 4B
template <class T = LL, class Comp = less<T>>
struct LeftistTree
{
    int n, tot;
    VI pos;              // 逻辑编号 -> 物理点
    VI id;               // 物理点 -> 逻辑编号
    VI lc, rc, dist, fa_dsu, sz;
    vector<T> val;
    vector<T> hsum;      // 仅堆根的有效元素和
    vector<T> tmul;
    vector<T> tadd;
    T gadd;
    vector<bool> deleted;
    VI roots;
    VI root_idx;
    multiset<T> root_vals;  // 未启用的全局最值索引
    T root_sum;
    // 预留 max_n 个逻辑点、max_n+max_ops 个物理点. 时空 O(max_n+max_ops)
    LeftistTree(int max_n = 0, int max_ops = 0) : n(0), tot(0),
        pos(max_n + 10, 0), id(max_n + max_ops + 10, 0),
        lc(max_n + max_ops + 10, 0), rc(max_n + max_ops + 10, 0),
        dist(max_n + max_ops + 10, -1), fa_dsu(max_n + max_ops + 10, 0),
        sz(max_n + max_ops + 10, 0), val(max_n + max_ops + 10, T()),
        hsum(max_n + max_ops + 10, T()),
        tmul(max_n + max_ops + 10, T(1)), tadd(max_n + max_ops + 10, T()),
        gadd(T()),
        deleted(max_n + max_ops + 10, false), root_idx(max_n + max_ops + 10, -1),
        root_sum(T())
    {
        roots.reserve(max_n + max_ops + 10);
    }
    // 重置为 _n 个单点堆, vals 为 1-based(空则全 0), _n<=max_n. 时间 O(_n+旧堆数)
    void init(int _n, const vector<T>& init_vals = {})
    {
        n = tot = _n;
        roots.clear();
        root_vals.clear();
        root_sum = T();
        gadd = T();
        dist[0] = -1;
        sz[0] = lc[0] = rc[0] = fa_dsu[0] = 0;
        hsum[0] = T(); tmul[0] = T(1); tadd[0] = T();
        for (int i = 1; i <= n; i++)
        {
            pos[i] = i; id[i] = i;
            lc[i] = rc[i] = 0;
            dist[i] = 0;
            sz[i] = 1;
            fa_dsu[i] = i;
            deleted[i] = false;
            root_idx[i] = -1;
            val[i] = init_vals.empty() ? T() : init_vals[i];
            hsum[i] = val[i];
            tmul[i] = T(1); tadd[i] = T();
        }
        for (int i = 1; i <= n; i++) { add_root(i); }
    }
    // 编号是否存活. O(1)
    bool alive(int x) { int p = pos[x]; return p && !deleted[p]; }
    // 两存活编号是否同堆. 时间 O(h)
    bool same(int x, int y) { return alive(x) && alive(y) && find_root(pos[x]) == find_root(pos[y]); }
    // 合并并返回堆顶编号, 已同堆/含失效编号返回 -1. 时间 O(h+log P), 栈 O(log P)
    int merge(int x, int y)
    {
        int px = pos[x], py = pos[y];
        if (!px || !py || deleted[px] || deleted[py]) return -1;
        int rx = find_root(px), ry = find_root(py);
        if (rx == ry) return -1;
        remove_root(rx); remove_root(ry);
        int rt = merge_trees(rx, ry);
        fa_dsu[rx] = fa_dsu[ry] = rt;
        sz[rt] = sz[rx] + sz[ry];
        hsum[rt] = hsum[rx] + hsum[ry];
        add_root(rt);
        return to_logical(rt);
    }
    // 插入并返回新编号; x=0/失效时独立成堆. 时间 O(h+log P), 新增 1 点, 栈 O(log P)
    int insert(int x, T v)
    {
        assert(n + 1 < (int)pos.size() && "max_n 需覆盖 insert 总次数");
        v -= gadd;
        int nid = ++n;
        int q = pos[x];
        int rt = (q && !deleted[q]) ? find_root(q) : 0;
        if (rt) remove_root(rt);
        int new_p = ++tot;
        assert(tot < (int)val.size() && "max_ops 需覆盖 insert 次数");
        lc[new_p] = rc[new_p] = dist[new_p] = 0;
        deleted[new_p] = false;
        fa_dsu[new_p] = new_p;
        sz[new_p] = 1;
        val[new_p] = v;  hsum[new_p] = v;
        tmul[new_p] = T(1); tadd[new_p] = T();
        root_idx[new_p] = -1;
        pos[nid] = new_p;  id[new_p] = nid;
        if (rt)
        {
            int nrt = merge_trees(rt, new_p);
            fa_dsu[rt] = fa_dsu[new_p] = nrt;
            sz[nrt] = sz[rt] + 1;
            hsum[nrt] = hsum[rt] + v;
            add_root(nrt);
        }
        else add_root(new_p);
        return nid;
    }
    // 删除 x, 返回剩余堆顶(空堆 0, x 失效 -1). 时间 O(h+(d+1)log P), 栈 O(log P)
    int erase(int x)
    {
        int p = pos[x];
        if (!p || deleted[p]) return -1;
        int rt = find_root(p);
        remove_root(rt);
        deleted[p] = true;
        sz[rt]--;
        hsum[rt] -= val[p];
        if (p == rt)
        {
            int nrt = normalize(p);
            if (nrt) { sz[nrt] = sz[rt]; hsum[nrt] = hsum[rt]; }
            rt = nrt;
        }
        if (rt) add_root(rt);
        return to_logical(rt);
    }
    // 删除 x 所在堆的顶, 返回剩余堆顶(空堆 0, x 失效 -1). 时间 O(h+(d+1)log P), 栈 O(log P)
    int pop(int x)
    {
        int p = pos[x];
        if (!p || deleted[p]) return -1;
        int rt = find_root(p);
        remove_root(rt);
        deleted[rt] = true;
        sz[rt]--;
        hsum[rt] -= val[rt];
        int nrt = normalize(rt);
        if (nrt) { sz[nrt] = sz[rt]; hsum[nrt] = hsum[rt]; add_root(nrt); }
        return to_logical(nrt);
    }
    // 将 x 改为 v, 返回堆顶(x 失效 -1). 时间 O(h+(d+1)log P), 新增 1 点, 栈 O(log P)
    int set_val(int x, T v)
    {
        int p = pos[x];
        if (!p || deleted[p]) return -1;
        v -= gadd;
        int rt = find_root(p);
        remove_root(rt);
        T old = val[p];
        deleted[p] = true;
        if (p == rt)
        {
            int nrt = normalize(p);
            if (nrt) { sz[nrt] = sz[rt]; hsum[nrt] = hsum[rt] - old; }
            rt = nrt;
        }
        else hsum[rt] -= old;
        int new_p = ++tot;
        assert(tot < (int)val.size() && "max_ops 估算不足");
        lc[new_p] = rc[new_p] = dist[new_p] = 0;
        deleted[new_p] = false;
        fa_dsu[new_p] = new_p;
        val[new_p] = v;
        tmul[new_p] = T(1); tadd[new_p] = T();
        root_idx[new_p] = -1;
        pos[x] = new_p;
        id[new_p] = x;
        if (rt)
        {
            int nrt = merge_trees(rt, new_p);
            fa_dsu[rt] = fa_dsu[new_p] = nrt;
            sz[nrt] = sz[rt];
            hsum[nrt] = hsum[rt] + v;
            add_root(nrt);
            return to_logical(nrt);
        }
        else
        {
            sz[new_p] = 1;
            hsum[new_p] = v;
            add_root(new_p);
            return to_logical(new_p);
        }
    }
    // 给 x 加 k, 返回堆顶(x 失效 -1). 时间 O(h+(d+1)log P), 新增 1 点, 栈 O(log P)
    int add_val(int x, T k)
    {
        int p = pos[x];
        if (!p || deleted[p]) return -1;
        return set_val(x, val[p] + gadd + k);
    }
    // 整堆加 k, 返回堆顶(x 失效 -1). 时间 O(h)
    int heap_add(int x, T k)
    {
        int p = pos[x];
        if (!p || deleted[p]) return -1;
        int rt = find_root(p);
        remove_root(rt);
        val[rt] += k;
        tadd[rt] += k;
        hsum[rt] += (T)sz[rt] * k;
        add_root(rt);
        return to_logical(rt);
    }
    // 整堆乘 m>0, 返回堆顶(x 失效 -1). 时间 O(h)
    int heap_mul(int x, T m)
    {
        assert(m > 0);
        int p = pos[x];
        if (!p || deleted[p]) return -1;
        int rt = find_root(p);
        remove_root(rt);
        T c = (m - T(1)) * gadd;
        val[rt]  = m * val[rt] + c;
        tmul[rt] *= m;
        tadd[rt]  = m * tadd[rt] + c;
        hsum[rt]  = m * hsum[rt] + c * (T)sz[rt];
        add_root(rt);
        return to_logical(rt);
    }
    // 所有堆加 k. O(1)
    void add_all(T k) { gadd += k; }
    // 所在堆顶编号, x 失效返回 -1. 时间 O(h)
    int get_top_id(int x)  { int p = pos[x]; return (!p || deleted[p]) ? -1 : id[find_root(p)]; }
    // 所在堆顶值, x 失效返回 T(). 时间 O(h)
    T   get_top_val(int x) { int p = pos[x]; return (!p || deleted[p]) ? T() : val[find_root(p)] + gadd; }
    // x 的值, 失效返回 T(). O(1)
    T   get_val(int x)     { int p = pos[x]; return (!p || deleted[p]) ? T() : val[p] + gadd; }
    // 所在堆大小, x 失效返回 0. 时间 O(h)
    int get_size(int x)    { int p = pos[x]; return (!p || deleted[p]) ? 0 : sz[find_root(p)]; }
    // 非空堆数. O(1)
    int get_heap_count() const { return (int)roots.size(); }
    // 所在堆元素和, x 失效返回 T(). 时间 O(h)
    T get_heap_sum(int x)
    {
        int p = pos[x];
        if (!p || deleted[p]) return T();
        int r = find_root(p);
        return hsum[r] + gadd * (T)sz[r];
    }
    // 各堆顶编号, 无序. 时空 O(堆数)
    VI get_roots_id() const
    {
        VI res; res.reserve(roots.size());
        for (int p : roots) res.push_back(id[p]);
        return res;
    }

private:
    int find_root(int p)
    {
        int r = p;
        while (fa_dsu[r] != r) r = fa_dsu[r];
        while (p != r)
        {
            int q = fa_dsu[p];
            fa_dsu[p] = r;
            p = q;
        }
        return r;
    }
    int to_logical(int p) const { return p ? id[p] : 0; }
    void pushdown(int p)
    {
        if (!p) return;
        T m = tmul[p], a = tadd[p];
        if (m == T(1) && a == T()) return;
        if (lc[p])
        {
            val[lc[p]] = m * val[lc[p]] + a;
            tmul[lc[p]] *= m;
            tadd[lc[p]] = m * tadd[lc[p]] + a;
        }
        if (rc[p])
        {
            val[rc[p]] = m * val[rc[p]] + a;
            tmul[rc[p]] *= m;
            tadd[rc[p]] = m * tadd[rc[p]] + a;
        }
        tmul[p] = T(1); tadd[p] = T();
    }
    void add_root(int p)
    {
        if (!p || deleted[p]) return;
        root_idx[p] = roots.size();
        roots.push_back(p);

    }
    void remove_root(int p)
    {
        if (!p || root_idx[p] == -1) return;
        int idx = root_idx[p];
        int last_p = roots.back();
        roots[idx] = last_p;
        root_idx[last_p] = idx;
        roots.pop_back();
        root_idx[p] = -1;

    }
    int merge_trees(int x, int y)
    {
        if (!x || !y) return x | y;
        if (Comp()(val[y], val[x]) || (val[x] == val[y] && id[x] > id[y])) swap(x, y);
        pushdown(x);
        rc[x] = merge_trees(rc[x], y);
        if (dist[rc[x]] > dist[lc[x]]) swap(lc[x], rc[x]);
        dist[x] = dist[rc[x]] + 1;
        return x;
    }
    int normalize(int p)
    {
        while (p && deleted[p])
        {
            pushdown(p);
            int nrt = merge_trees(lc[p], rc[p]);
            if (nrt)
            {
                fa_dsu[nrt] = nrt;
                fa_dsu[p] = nrt;
                p = nrt;
            }
            else p = 0;
        }
        return p;
    }
};
#endif
/*
 * Usage:
 * int main()
 * {
 *     LeftistTree<> t(4, 2);
 *     t.init(3, VLL{0, 5, 2, 8});
 *     t.merge(1, 2);
 *     cout << t.get_top_id(1) << endl; // 2
 *     t.set_val(1, 1);
 *     cout << t.get_top_val(2) << endl; // 1
 *     t.pop(2);
 *     cout << t.alive(1) << ' ' << t.get_top_id(2) << endl; // 0 2
 *     t.heap_add(2, 3); // 从此只用堆级操作
 *     int x = t.insert(2, 4);
 *     cout << t.get_top_id(2) << ' ' << t.get_heap_sum(x) << endl; // 4 9
 *     t.init(1);
 *     cout << t.get_top_val(1) << endl; // 0
 * }
 */
