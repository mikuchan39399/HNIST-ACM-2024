#include <vector>
#include <algorithm>
#include <set>
#include <iostream>
#include <functional>
#include <cassert>

using namespace std;
using LL = long long;
using VI = vector<int>;

// T: 节点权值类型(需支持 + 、* 、与 T()/T(1) 的相等判断)
// Comp: 默认 less<T> 为小根堆, 传 greater<T> 为大根堆
#include <vector>
#include <algorithm>
#include <set>
#include <iostream>
#include <functional>
#include <cassert>

using namespace std;
using LL = long long;
using VI = vector<int>;

// T: 节点权值类型(需支持 + 、* 、与 T()/T(1) 的相等判断)
// Comp: 默认 less<T> 为小根堆, 传 greater<T> 为大根堆
//
// 仿射懒标记约定(加法/乘法统一) 
//   每个物理节点挂 (tmul, tadd), 语义: 孩子的真实值 = tmul * val[孩子] + tadd (堆内坐标)
//   - val[p] 恒已含自身 tag; 堆根无严格祖先 → 真实值 = val[根], 免 push 直读
//   - 非根真实值 = 严格祖先的 tag 由浅到深复合后作用于 val[] (mode-1 军规:
//     heap_add/heap_mul 后对非堆顶点的 get_val/add_val/set_val/erase 的数值读取受限)
//   - heap_add(x,k) ≡ 仿射 (1,k);  heap_mul(x,m) ≡ 仿射 (m,0), m 必须 > 0(保堆序前提)
//     一般 v->m*v+a ≡ heap_mul 后紧跟 heap_add, 组合律自动复合成 (m,a)
//   - 组合律(父盖子外): (m2,a2)∘(m1,a1) = (m2*m1, m2*a1 + a2)
//   - hsum 线性可分解: hsum' = m*hsum + a*sz, 全程 O(1) 精确
//   gadd: 全局加偏移(纯加), 叠加在一切堆坐标之上, 查询出口统一补偿
template <class T = LL, class Comp = less<T>>
struct LeftistTree
{
    int n, tot;
    VI pos;              // 逻辑节点 -> 物理节点
    VI id;               // 物理节点 -> 逻辑节点
    VI lc, rc, dist, fa_dsu, sz;
    vector<T> val;
    vector<T> hsum;      // hsum[当前物理堆根] = 该堆存活元素真实值之和(堆内坐标, 只在根上增量)
    vector<T> tmul;
    vector<T> tadd;
    T gadd;              // 全局加偏移
    vector<bool> deleted;
    VI roots;
    VI root_idx;
    multiset<T> root_vals; // 所有堆顶的值(不含 gadd); 懒更新 + 全局查询场景必须启用
    T root_sum;           // 全部堆顶之和(不含 gadd)

    // max_n: 逻辑元素数上限, max_ops: 单测试点内 set_val/add_val 调用总次数上限
    LeftistTree(int max_n = 0, int max_ops = 0) : n(0), tot(0),
        pos(max_n + 10, 0), id(max_n + max_ops + 10, 0),
        lc(max_n + max_ops + 10, 0), rc(max_n + max_ops + 10, 0),
        dist(max_n + max_ops + 10, -1), fa_dsu(max_n + max_ops + 10, 0),
        sz(max_n + max_ops + 10, 0), val(max_n + max_ops + 10, T()),
        hsum(max_n + max_ops + 10, T()),
        tmul(max_n + max_ops + 10, T(1)), tadd(max_n + max_ops + 10, T()),  // ★ 恒等元起步
        gadd(T()),
        deleted(max_n + max_ops + 10, false), root_idx(max_n + max_ops + 10, -1),
        root_sum(T())
    {
        roots.reserve(max_n + max_ops + 10);
    }

    void init(int _n, const vector<T>& init_vals = {}) // init_vals 1-base
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
    // 以下全部操作物理编号
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
        // root_vals.insert(val[p]);   // [RV] 卡常, 需F全局堆顶查询时恢复
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
    // 入参一律逻辑编号; 返回值约定: 逻辑编号 ∈ [1,n], -1 = 操作非法, 0 = 堆被删空
    bool alive(int x) { int p = pos[x]; return p && !deleted[p]; }
    bool same(int x, int y) { return alive(x) && alive(y) && find_root(pos[x]) == find_root(pos[y]); }

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
        assert(tot < (int)val.size() && "max_ops 估计不足");
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
    int add_val(int x, T k)
    {
        int p = pos[x];
        if (!p || deleted[p]) return -1;
        return set_val(x, val[p] + gadd + k);
    }
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
    int heap_mul(int x, T m)
    {
        assert(m > 0);
        int p = pos[x];
        if (!p || deleted[p]) return -1;
        int rt = find_root(p);
        remove_root(rt);
        val[rt] *= m;
        tmul[rt] *= m;
        tadd[rt] *= m;
        hsum[rt] *= m;
        add_root(rt);
        return to_logical(rt);
    }
    void add_all(T k) { gadd += k; }
    // 查询接口(入参逻辑编号, 已删点返回哨兵; 出口统一为真实值)
    int get_top_id(int x) { int p = pos[x]; return (!p || deleted[p]) ? -1 : id[find_root(p)]; }
    T get_top_val(int x)  { int p = pos[x]; if (!p || deleted[p]) return T(); return val[find_root(p)] + gadd; }
    T get_val(int x)      { int p = pos[x]; return (!p || deleted[p]) ? T() : val[p] + gadd; }
    int get_size(int x)   { int p = pos[x]; return (!p || deleted[p]) ? 0 : sz[find_root(p)]; }
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
    // T get_max_top() const { return root_vals.empty() ? T() : *root_vals.rbegin() + gadd; }   // [RV]
    // T get_min_top() const { return root_vals.empty() ? T() : *root_vals.begin() + gadd; }    // [RV]
    // T get_sum_tops() const { return root_sum + gadd * (T)roots.size(); }                     // [RV]
};


/**
 * Usage:
 * 
 * // max_n: 逻辑点数上限, max_ops: 单测内 set_val+add_val 总调用次数
 * // 多测换 init 即复位; 大根堆传 greater<LL>
 * LeftistTree<LL> lt(n, 2 * q);
 * vector<LL> w(n + 1);
 * lt.init(n, w);                            // 1-base, 先于一切操作
 * 
 * // 返回值: >=1 新堆顶编号, 0 堆已空, -1 非法(点已删/同堆)
 * lt.merge(x, y);                           // 合 x,y 所在堆
 * lt.pop(x);                                // 删堆顶, 注意删的不是 x
 * lt.erase(x);                              // 删 x 本身
 * lt.set_val(x, v);                         // v 传真实值
 * lt.add_val(x, k);
 * lt.heap_add(x, k);                        // 整堆加
 * lt.heap_mul(x, m);                        // 整堆乘, 只许 m > 0
 * lt.add_all(k);                            // 全体堆加
 * 
 * lt.alive(x); lt.same(x, y);
 * lt.get_top_id(x); lt.get_top_val(x);      // 查询出口一律真实值
 * lt.get_val(x);   lt.get_size(x);          lt.get_heap_sum(x);
 * lt.get_heap_count(); lt.get_roots_id();
 * 
 * // P3377 罗马游戏
 * if (op == 'M') lt.merge(x, y);
 * else 
 * {
 *     if (!lt.alive(x)) puts("0");
 *     else { cout << lt.get_top_val(x) << endl; lt.pop(x); }
 * }
 * 
 * // P1456 猴王, 大根堆
 * ll a = lt.get_top_val(x), b = lt.get_top_val(y);
 * lt.set_val(x, a / 2); lt.set_val(y, b / 2);
 * lt.merge(x, y);
 * 
 * // BZOJ4003 城池攻占: 骑士是逻辑点, rt[u] 记每城代表骑士(-1 无)
 * // 建树时逐城 merge, 后序 dfs 三步:
 * for (int v : son[u])
 *     if (rt[v] != -1) rt[u] = (rt[u] == -1 ? rt[v] : lt.merge(rt[u], rt[v]));
 * while (rt[u] != -1 && lt.get_top_val(rt[u]) < def[u]) 
 * {
 *     ans[u]++;
 *     int t = lt.pop(rt[u]);
 *     rt[u] = t ? t : -1;
 * }
 * if (rt[u] != -1)
 *     opt[u] == 1 ? lt.heap_add(rt[u], c[u]) : lt.heap_mul(rt[u], c[u]);
 * 
 * // 注:
 * // 堆顶类查询/规模/结构操作永远精确;
 * // heap_add/heap_mul/add_all 之后对非堆顶点的 get_val/add_val/erase 会漂移,
 * // 全程只碰堆顶就没事, 城池攻占正是这种
 * // 乘加连打等于 v -> m*v+a, 乘法注意 m*val 溢出
 * // 要查全体堆顶最值/和时解开模板里的 [RV] 行
 */