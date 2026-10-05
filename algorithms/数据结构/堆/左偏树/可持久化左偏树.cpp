// zoi: persistentLeftist
#ifndef Z_OI_LEFTIST
#define Z_OI_LEFTIST

#include "../../../杂项/utils/utils.cpp"

// 可持久化左偏堆; less 小根 / greater 大根, 同值顺序不定; N=逻辑大小
// 版本根自行保存, 0=空; 共享/自身合并会重复计数; 值、和及大小须不溢出
// 点池预算=新单点数+merge/pop 复制路径总长, 每点约 36B(LL+int)
template <class T = LL, class Comp = less<T>, class Pay = int>
struct PersistentLeftist
{
    int tot = 0;
    VI lc, rc, dist, sz;
    vector<T> val, hsum;
    vector<Pay> pay;
    // 预留 max_nodes 个物理点. 时空 O(max_nodes)
    PersistentLeftist(int max_nodes = 0) :
        lc(max_nodes + 10), rc(max_nodes + 10), dist(max_nodes + 10, -1),
        sz(max_nodes + 10), val(max_nodes + 10), hsum(max_nodes + 10),
        pay(max_nodes + 10)
    {
        sz[0] = 0;
        hsum[0] = T();
    }
    // 清空点池, 所有旧根失效; 保留容量. O(1)
    void clear() { tot = 0; }
    // 新建单点堆, 返回根. O(1)
    int new_node(T v, Pay p = Pay())
    {
        int q = ++tot;
        assert(q < (int)val.size() && "max_nodes 估算不足");
        lc[q] = rc[q] = 0; dist[q] = 0; sz[q] = 1;
        val[q] = v; hsum[q] = v; pay[q] = p;
        return q;
    }
    // 合并并返回新根, 保留旧版本. 时空 O(log N)
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
    // 原地合并, 仅用于独占且互不相交的两堆. 时间/栈 O(log N), 不开点
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
    // 插入并返回新根. 时空 O(log N)
    int insert(int rt, T v, Pay p = Pay()) { return merge(rt, new_node(v, p)); }
    // 弹顶并返回新根, 空堆返回 0. 时空 O(log N)
    int pop(int rt) { return merge(lc[rt], rc[rt]); }
    // 判空. O(1)
    bool empty  (int rt) const { return rt == 0; }
    // 逻辑元素数. O(1)
    int  size   (int rt) const { return sz[rt]; }
    // 堆顶值, 要求非空. O(1)
    T    top    (int rt) const { return val[rt]; }
    // 元素和, 空堆为 T(). O(1)
    T    sum    (int rt) const { return hsum[rt]; }
    // 堆顶附带数据, 要求非空. O(1)
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
 * int main()
 * {
 *     PersistentLeftist<> t(100);
 *     int a = t.insert(0, 5, 50);
 *     int b = t.insert(a, 2, 20);
 *     int c = t.pop(b);
 *     cout << t.top(a) << ' ' << t.top(b) << ' ' << t.top(c) << endl; // 5 2 5
 *     cout << t.top_pay(b) << endl; // 20
 *     int d = t.merge(a, b); // 共享的 5 在新版本中出现两次
 *     cout << t.size(d) << ' ' << t.sum(d) << endl; // 3 12
 *     t.clear(); // a/b/c/d 全部失效
 *     int e = t.new_node(7);
 *     cout << t.top(e) << endl; // 7
 * }
 */
#endif
