// zoi: leftist
#ifndef Z_OI_LEFTIST_TREE
#define Z_OI_LEFTIST_TREE

#include "../../../杂项/utils/utils.cpp"


// 逻辑编号保持不变, 点修改废弃旧物理点; less 为小根堆, greater 为大根堆, 同值取小编号
// 调用 heap_add/heap_mul 后只用堆级操作, 不再调用 get_val/set_val/add_val/erase; add_all 不受此限
// h 为本次查根的父链长度, P 为累计物理点数, d 为本次清理死点数; 查根迭代压缩, 不保证单次对数
// LL 物理点约 64 B 加 deleted 位表, 逻辑点另 4 B; P <= 初始点数 + insert + set_val/add_val 次数
// 所有值、偏移、标记复合与求和中间值须在 T 内, heap_mul 只接受正数
template <class T = LL, class Comp = less<T>>
struct LeftistTree
{
    int n, tot;
    VI pos;              // 逻辑节点 -> 物理节点
    VI id;               // 物理节点 -> 逻辑节点
    VI lc, rc, dist, fa_dsu, sz;
    vector<T> val;
    vector<T> hsum;      // hsum[堆根] = 该堆存活元素值之和, 只在根上维护
    vector<T> tmul;      // 仿射标记: 乘
    vector<T> tadd;      // 仿射标记: 加
    T gadd;              // 全局加偏移
    vector<bool> deleted;
    VI roots;            // 所有堆的物理堆顶序列
    VI root_idx;         // 物理点在 roots 里的下标, -1 = 非堆顶
    multiset<T> root_vals;  // 所有堆顶的值(不含 gadd)   [RV] 需全局查询时解封
    T root_sum;             // 全部堆顶之和(不含 gadd)   [RV] 需全局查询时解封
    // 预算: max_n = n + insert 次数; max_ops = set_val + add_val + insert 次数
    // 时间: O(max_n+max_ops) | 空间: O(max_n+max_ops)
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
    // 清空并建立 _n 个独立堆, 初值取 1-based init_vals[1.._n], 空表取零, _n <= max_n
    // 时间: O(_n+旧堆数) | 空间: O(1)
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
    // 查询逻辑点 x 是否存活
    // 时间: O(1) | 空间: O(1)
    bool alive(int x) { int p = pos[x]; return p && !deleted[p]; }
    // 查询 x, y 是否存活且同堆
    // 时间: O(h) | 空间: O(1)
    bool same(int x, int y) { return alive(x) && alive(y) && find_root(pos[x]) == find_root(pos[y]); }
    // 合并 x, y 所在堆, 返回堆顶逻辑编号, 死点或同堆返回 -1
    // 时间: O(h+log P) | 递归空间: O(log P)
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
    // 向 x 所在堆插入真实值 v, 返回新逻辑编号; x 为 0 或死点则独立成堆
    // 时间: O(h+log P) | 新物理点: 1, 递归空间: O(log P)
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
    // 删除 x, 返回剩余堆顶逻辑编号, 空堆为 0, 死点为 -1; 不用于整堆懒标记之后
    // 时间: O(h+(d+1)log P) | 递归空间: O(log P)
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
    // 删除 x 所在堆的堆顶, 返回剩余堆顶逻辑编号, 空堆为 0, 死点为 -1
    // 时间: O(h+(d+1)log P) | 递归空间: O(log P)
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
    // 把 x 的真实值改为 v, 返回新堆顶逻辑编号, 死点为 -1; 不用于整堆懒标记之后
    // 时间: O(h+(d+1)log P) | 新物理点: 1, 递归空间: O(log P)
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
    // 把 x 的值增加 k, 返回新堆顶逻辑编号, 死点为 -1; 不用于整堆懒标记之后
    // 时间: O(h+(d+1)log P) | 新物理点: 1, 递归空间: O(log P)
    int add_val(int x, T k)
    {
        int p = pos[x];
        if (!p || deleted[p]) return -1;
        return set_val(x, val[p] + gadd + k);
    }
    // 把 x 所在堆整体加 k, 返回堆顶逻辑编号, 死点为 -1
    // 时间: O(h) | 空间: O(1)
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
    // 把 x 所在堆的真实值乘 m, m > 0, 返回堆顶逻辑编号, 死点为 -1
    // 时间: O(h) | 空间: O(1)
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
    // 全体存活堆 + k
    // 时间: O(1) | 空间: O(1)
    void add_all(T k) { gadd += k; }
    // 返回 x 所在堆顶的逻辑编号, 死点为 -1
    // 时间: O(h) | 空间: O(1)
    int get_top_id(int x)  { int p = pos[x]; return (!p || deleted[p]) ? -1 : id[find_root(p)]; }
    // 返回 x 所在堆顶的真实值, 死点为 T()
    // 时间: O(h) | 空间: O(1)
    T   get_top_val(int x) { int p = pos[x]; return (!p || deleted[p]) ? T() : val[find_root(p)] + gadd; }
    // 返回 x 的真实值, 死点为 T(); 不用于整堆懒标记之后
    // 时间: O(1) | 空间: O(1)
    T   get_val(int x)     { int p = pos[x]; return (!p || deleted[p]) ? T() : val[p] + gadd; }
    // 返回 x 所在堆的存活元素数, 死点为 0
    // 时间: O(h) | 空间: O(1)
    int get_size(int x)    { int p = pos[x]; return (!p || deleted[p]) ? 0 : sz[find_root(p)]; }
    // 返回非空堆数
    // 时间: O(1) | 空间: O(1)
    int get_heap_count() const { return (int)roots.size(); }
    // 返回 x 所在堆的真实值之和, 死点为 T()
    // 时间: O(h) | 空间: O(1)
    T get_heap_sum(int x)
    {
        int p = pos[x];
        if (!p || deleted[p]) return T();
        int r = find_root(p);
        return hsum[r] + gadd * (T)sz[r];
    }
    // 返回所有堆顶逻辑编号, 顺序不固定
    // 时间: O(堆数) | 空间: O(堆数)
    VI get_roots_id() const
    {
        VI res; res.reserve(roots.size());
        for (int p : roots) res.push_back(id[p]);
        return res;
    }
    // --- 全局最值查询 API ---
    // T get_max_top() const { return root_vals.empty() ? T() : *root_vals.rbegin() + gadd; }   // [RV]
    // T get_min_top() const { return root_vals.empty() ? T() : *root_vals.begin() + gadd; }    // [RV]
    // T get_sum_tops() const { return root_sum + gadd * (T)roots.size(); }                     // [RV]
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
        // root_vals.insert(val[p]);   // [RV]
        // root_sum += val[p];         // [RV]
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
        // root_vals.erase(root_vals.find(val[p]));     // [RV]
        // root_sum -= val[p];                          // [RV]
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
