// zoi: persistentLeftist
#ifndef Z_OI_LEFTIST
#define Z_OI_LEFTIST

#include <vector>
#include <algorithm>
#include <cassert>
#include "../../../杂项/utils/utils.cpp"

using namespace std;

// ============ 可持久化左偏树(函数式) ============
// 核心性质:
//   1. 节点只增不改，所有写操作(merge/insert/pop)均生成新版本，旧根句柄永久有效。
//   2. 外部无任何懒标记或并查集，完全依赖调用方自己维护版本根节点数组。
//   3. 预算 = 初始点数 + 操作次数 * 40 结点
template <class T = LL, class Comp = less<T>, class Pay = int>
struct PersistentLeftist
{
    int tot = 0;
    VI lc, rc, dist, sz;
    vector<T> val, hsum;
    vector<Pay> pay;
    PersistentLeftist(int max_nodes = 0) :
        lc(max_nodes + 10), rc(max_nodes + 10), dist(max_nodes + 10, -1),
        sz(max_nodes + 10), val(max_nodes + 10), hsum(max_nodes + 10),
        pay(max_nodes + 10)
    {
        sz[0] = 0;
        hsum[0] = T();
    }
    void init() { tot = 0; }
    // 生成一个只含单点的新堆，返回物理节点编号
    // 时间: O(1) | 空间: O(1)
    int new_node(T v, Pay p = Pay())
    {
        int q = ++tot;
        assert(q < (int)val.size() && "max_nodes 估算不足");
        lc[q] = rc[q] = 0; dist[q] = 0; sz[q] = 1;
        val[q] = v; hsum[q] = v; pay[q] = p;
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
        sz[c]   = sz[lc[c]] + sz[rc[c]] + 1;
        hsum[c] = val[c] + hsum[lc[c]] + hsum[rc[c]];
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
        sz[x]   = sz[lc[x]] + sz[rc[x]] + 1;
        hsum[x] = val[x] + hsum[lc[x]] + hsum[rc[x]];
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
    int  size   (int rt) const { return sz[rt]; }
    T    top    (int rt) const { return val[rt]; }
    T    sum    (int rt) const { return hsum[rt]; }
    Pay  top_pay(int rt) const { return pay[rt]; }
private:
    int clone(int p)
    {
        int q = ++tot;
        assert(q < (int)val.size() && "max_nodes 估算不足");
        lc[q] = lc[p]; rc[q] = rc[p]; dist[q] = dist[p];
        sz[q] = sz[p]; val[q] = val[p]; hsum[q] = hsum[p]; pay[q] = pay[p];
        return q;
    }
};

/*
 * Usage:
 * // 1. 初始化 (例: M次操作, 小根堆)
 * PersistentLeftist<LL> lt(M * 40);
 * VI rt(M + 1, 0); // 版本中心: rt[i] 存第 i 版的堆根
 *
 * // 2. 状态转移
 * rt[1] = lt.insert(rt[0], 5);     // 插入: 0版插5 -> 新版1
 * rt[2] = lt.merge(rt[1], rt[x]);  // 合并: 1版并x版 -> 新版2
 * rt[3] = lt.pop(rt[2]);           // 弹出: 弹2版顶 -> 新版3
 *
 * // 3. 查询
 * if (!lt.empty(rt[i]))
 * {
 *     cout << lt.top(rt[i]) << endl;
 * }
 */
#endif
