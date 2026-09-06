// zoi: persistentLeftist
#ifndef Z_OI_LEFTIST
#define Z_OI_LEFTIST

#include "../../../杂项/utils/utils.cpp"


// ============ 可持久化左偏树(函数式) ============
// merge/insert/pop 只新增结点, 版本根由调用方保存; init 使全部旧根失效, merge_raw 另按独占契约使用
// less 为小根堆, greater 为大根堆; 同值不保证 payload 顺序, N 为本次堆的逻辑元素数
// LL + int payload 每物理点 36 B; 预算 = 新单点数 + 所有 merge/pop 复制路径长度之和, 8e6 ≈ 288 MB
// 允许合并共享版本和自身, 重叠元素重复计数; 逻辑大小须在 int 内, 值与求和中间值须在 T 内
template <class T = LL, class Comp = less<T>, class Pay = int>
struct PersistentLeftist
{
    int tot = 0;
    VI lc, rc, dist, sz;
    vector<T> val, hsum;
    vector<Pay> pay;
    // 分配 max_nodes 个物理点的空间, 初始为空池
    // 时间: O(max_nodes) | 空间: O(max_nodes)
    PersistentLeftist(int max_nodes = 0) :
        lc(max_nodes + 10), rc(max_nodes + 10), dist(max_nodes + 10, -1),
        sz(max_nodes + 10), val(max_nodes + 10), hsum(max_nodes + 10),
        pay(max_nodes + 10)
    {
        sz[0] = 0;
        hsum[0] = T();
    }
    // 清空结点池并保留容量, 所有旧根停用
    // 时间: O(1) | 空间: O(1)
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
    // 合并 x 和 y 为新版本, 返回物理根编号; 0 为空堆, 共享元素按重数保留
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
    // 原地合并独占且互不重叠的 x 和 y, 返回物理根编号, 两个输入均可能被改写
    // 时间: O(log N) | 递归空间: O(log N), 不开新点
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
    // 弹出 rt 的堆顶并返回新版本物理根编号, 空堆返回 0
    // 时间: O(log N) | 空间: O(log N)
    int pop(int rt) { return merge(lc[rt], rc[rt]); }
    // 判断版本 rt 是否为空
    // 时间: O(1) | 空间: O(1)
    bool empty  (int rt) const { return rt == 0; }
    // 返回版本 rt 的逻辑元素数, 空堆为 0
    // 时间: O(1) | 空间: O(1)
    int  size   (int rt) const { return sz[rt]; }
    // 返回非空版本 rt 的堆顶值
    // 时间: O(1) | 空间: O(1)
    T    top    (int rt) const { return val[rt]; }
    // 返回版本 rt 的元素和, 空堆为 T()
    // 时间: O(1) | 空间: O(1)
    T    sum    (int rt) const { return hsum[rt]; }
    // 返回非空版本 rt 的堆顶载荷, 同值时不保证载荷顺序
    // 时间: O(1) | 空间: O(1)
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
 *     t.init(); // a/b/c/d 全部失效
 *     int e = t.new_node(7);
 *     cout << t.top(e) << endl; // 7
 * }
 */
#endif
