#ifndef Z_OI_LEFTIST_TREE
#define Z_OI_LEFTIST_TREE

#include <vector>
#include <algorithm>
#include <set>
#include <iostream>
#include <functional>
#include <cassert>
#include "../../../杂项/utils/utils.cpp"

using namespace std;

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
    // max_n = n + insert; max_ops = set_val + add_val + insert
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
private:
    int find_root(int p)
    {
        if (!p || fa_dsu[p] == p) return p;
        return fa_dsu[p] = find_root(fa_dsu[p]);
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
public:
    // ===== 外部接口传参皆为逻辑节点 =====
    // --- 状态判定 API ---
    // 查询逻辑点 x 是否存活
    // 时间: O(1) | 空间: O(1)
    bool alive(int x) { int p = pos[x]; return p && !deleted[p]; }
    // 查询 x, y 是否存活且同堆
    // 时间: O(α(N)) | 空间: O(1)
    bool same(int x, int y) { return alive(x) && alive(y) && find_root(pos[x]) == find_root(pos[y]); }
    // --- 结构变更 API ---
    // 合并 x, y 所在堆，返回新堆顶逻辑编号
    // 时间: O(log N) | 空间: O(1)
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
    // 往 x 所在堆插入值为 v 的新节点，返回新节点逻辑编号(x 失效则独立成堆)
    // 时间: O(log N) | 空间: O(1)
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
    // 删除 x (懒惰删除, 死点清理由后续操作分摊)
    // 时间: 均摊 O(log N) | 空间: O(1)
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
    // 删除 x 所在堆的堆顶 (死点清理由后续操作分摊)
    // 时间: 均摊 O(log N) | 空间: O(1)
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
    // 覆盖修改 x 的真实值 (废弃旧点开新点, 死点清理由后续操作分摊)
    // 时间: 均摊 O(log N) | 空间: O(1)
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
    // 点 x 增加 k
    // 时间: 均摊 O(log N) | 空间: O(1)
    int add_val(int x, T k)
    {
        int p = pos[x];
        if (!p || deleted[p]) return -1;
        return set_val(x, val[p] + gadd + k);
    }
    // --- 懒标记 API ---
    // 点 x 所在整堆 + k
    // 时间: O(α(N)) | 空间: O(1)
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
    // 逻辑点 x 所在整堆真实值 * m (m > 0)
    // 时间: O(α(N)) | 空间: O(1)
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
    // --- 查询 API (入参全为逻辑编号，出口全为含全局偏移的真实值) ---
    int get_top_id(int x)  { int p = pos[x]; return (!p || deleted[p]) ? -1 : id[find_root(p)]; }
    T   get_top_val(int x) { int p = pos[x]; return (!p || deleted[p]) ? T() : val[find_root(p)] + gadd; }
    T   get_val(int x)     { int p = pos[x]; return (!p || deleted[p]) ? T() : val[p] + gadd; }
    int get_size(int x)    { int p = pos[x]; return (!p || deleted[p]) ? 0 : sz[find_root(p)]; }
    int get_heap_count() const { return (int)roots.size(); }
    T get_heap_sum(int x)
    {
        int p = pos[x];
        if (!p || deleted[p]) return T();
        int r = find_root(p);
        return hsum[r] + gadd * (T)sz[r];
    }
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
};
#endif
/*
 * Usage:
 * // 1. 初始化
 * LeftistTree<LL, greater<LL>> lt(N, Q); // 例: 大根堆
 * lt.init(n, a); // a 为 1-base 的 vector，初始赋值 a[1] ~ a[n]
 *
 * // 2. 核心操作 (x, y 均为初始生成的逻辑编号 1~n)
 * lt.merge(x, y);       // 合并逻辑点 x 和 y 所在的堆
 * lt.pop(x);            // 弹出逻辑点 x 所在堆的堆顶
 * lt.erase(x);          // 删除逻辑点 x (将懒惰删除, 并在到根时清理)
 * lt.set_val(x, v);     // 单点覆盖: 将逻辑点 x 的值设为 v (开新物理节点)
 * lt.add_val(x, k);     // 单点修改: 将逻辑点 x 的值增加 k
 *
 * // 3. 懒标记操作 (注意: 调用后，非堆顶元素的单点 get_val 会失效)
 * lt.heap_add(x, k);    // x 所在堆的所有元素 +k
 * lt.heap_mul(x, m);    // x 所在堆的所有元素 *m (m必须为正数)
 * lt.add_all(k);        // 全局所有堆的所有元素 +k
 *
 * // 4. 查询操作
 * bool ok = lt.alive(x) && lt.same(x, y); // 判断 x 是否存活且与 y 同堆
 * int rt_id = lt.get_top_id(x);           // 获取 x 所在堆顶的逻辑编号
 * LL rt_val = lt.get_top_val(x);          // 获取 x 所在堆顶的最值
 * LL h_sum = lt.get_heap_sum(x);          // 获取 x 所在堆的元素总和
 * int h_sz = lt.get_size(x);              // 获取 x 所在堆的存活节点数
 *
 * // 5. 全局查最值 (P3273 棘手的操作)
 * // 需在模板中解封含有 [RV] 的代码行。卡常时需将 root_vals 改为对顶优先队列。
 * // cout << lt.get_max_top() << endl;
 */
