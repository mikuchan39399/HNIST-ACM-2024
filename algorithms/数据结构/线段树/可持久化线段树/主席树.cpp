// zoi: persistentSeg
#ifndef Z_OI_PERS_SEG_TREE
#define Z_OI_PERS_SEG_TREE

#include <vector>
#include <cassert>
#include "../../../杂项/utils/utils.cpp"

using namespace std;


template<class Info, class Tag>
struct PersSegTree
{
    struct Node
    {
        int lc = 0, rc = 0;
        Info info;
        Tag tag;
    };
    LL n;
    int tot = 0;
    int cap = 2;
    vector<Node> tr;
    // 预算 = (n+1)(log2 m+1) 结点; n=2e5 静态第k小 ≈ 3.8e6, 更大规模显式传
    PersSegTree(LL max_n = 1, int max_nodes = 4000010) : n(max_n)
    {
        cap = max_nodes + 1;
        tr.reserve(max_nodes + 1);
        tr.push_back(Node{});
    }
    void clear()
    {
        tot = 0;
        tr.clear();
        tr.push_back(Node{});
    }
    void set_n(LL _n) { n = _n; }
public:
    // 由一维数组 a[1..n] 直接生成版本 0 的根结点
    // 返回值: 生成的新版本根结点编号
    // 时间: O(n) | 空间: O(n)
    int build(const vector<Info>& a)
    {
        assert((int)a.size() >= 2);
        n = (LL)a.size() - 1;
        return build(1, n, a);
    }
    // 在版本 rt 的区间 [x, y] 应用增量 v (Tag), 单点修改令 x == y 即可
    // 基于 Tag 永久化, 范围修改仅支持同时满足交换律与结合律的操作
    // 返回值: 生成的新版本根结点编号
    // 时间: O(log V) | 空间: O(log V)
    int modify(int rt, LL x, LL y, const Tag& v)
    {
        assert(1 <= x && x <= y && y <= n);
        return modify(rt, 1, n, x, y, v);
    }
    // 查询版本 rt 中区间 [x, y] 的聚合信息, 单点查询令 x == y 即可, 不能查最值
    // 返回值: 区间聚合后的 Info
    // 时间: O(log V) | 空间: O(log V)
    Info query(int rt, LL x, LL y) { return query(rt, 1, n, x, y, Tag{}); }
    // 在版本 rt 中寻找 >= start 且满足谓词 pred 的第一个叶子位置, pred 必须具备单调性
    // 返回值: 符合条件的位置，无解返回 -1
    // 时间: O(log V) | 空间: O(log V) 递归栈，零结点分配
    template<class Pred>
    LL find_first(int rt, LL start, Pred pred)
    {
        if (start < 1 || start > n) return -1;
        return find_first(rt, 1, n, start, Tag{}, pred);
    }
    // 在版本 rt 中寻找 <= end 且满足谓词 pred 的最后一个叶子位置
    // 返回值: 符合条件的位置，无解返回 -1
    // 时间: O(log V) | 空间: O(log V)
    template<class Pred>
    LL find_last(int rt, LL end, Pred pred)
    {
        if (end < 1 || end > n) return -1;
        return find_last(rt, 1, n, end, Tag{}, pred);
    }
    // 在 Σplus − Σminus 的线段树叠加结构上定位第 k 小, 返回值域下标。
    // 要求: Info 结构体中包含 cnt 字段，且 k ∈ [1, 总计数]
    // 限制: 仅用于点修改 (x == y) 构建的版本, 范围修改与第 k 小不要混用
    // 时间: O((|plus|+|minus|) * log V) | 空间: O(|plus|+|minus|)
    LL find_kth(VI plus, VI minus, LL k)
    {
        LL l = 1, r = n;
        while (l < r)
        {
            LL mid = l + (r - l) / 2, cntL = 0;
            for (int p : plus)  cntL += get_info(tr[p].lc, l, mid).cnt;
            for (int p : minus) cntL -= get_info(tr[p].lc, l, mid).cnt;
            if (k <= cntL)
            {
                r = mid;
                for (int &p : plus)  p = tr[p].lc;
                for (int &p : minus) p = tr[p].lc;
            }
            else
            {
                k -= cntL;
                l = mid + 1;
                for (int &p : plus)  p = tr[p].rc;
                for (int &p : minus) p = tr[p].rc;
            }
        }
        return l;
    }
private:
    int fork(int p, LL len)
    {
        assert(tot + 1 < cap && "max_nodes 开小了");
        int q = ++tot;
        if (p)
        {
            Node tmp = tr[p];
            tr.push_back(move(tmp));
        }
        else
        {
            tr.push_back(Node{});
            tr.back().info.len = len;
        }
        return q;
    }
    Info get_info(int p, LL l, LL r)
    {
        Info ret = p ? tr[p].info : Info{};
        if (!p) ret.len = r - l + 1;
        return ret;
    }
    void pushup(int p, LL l, LL r)
    {
        LL mid = l + (r - l) / 2;
        tr[p].info = get_info(tr[p].lc, l, mid) + get_info(tr[p].rc, mid + 1, r);
        tr[p].info.apply(tr[p].tag);
    }
    int modify(int p, LL l, LL r, LL x, LL y, const Tag& v)
    {
        if (l > y || r < x) return p;
        p = fork(p, r - l + 1);
        if (l >= x && r <= y)
        {
            tr[p].info.apply(v);
            tr[p].tag.apply(v);
            return p;
        }
        LL mid = l + (r - l) / 2;
        tr[p].lc = modify(tr[p].lc, l, mid, x, y, v);
        tr[p].rc = modify(tr[p].rc, mid + 1, r, x, y, v);
        pushup(p, l, r);
        return p;
    }
    Info query(int p, LL l, LL r, LL x, LL y, const Tag& acc)
    {
        if (l > y || r < x) return Info{};
        if (l >= x && r <= y)
        {
            Info ret = get_info(p, l, r);
            ret.apply(acc);
            return ret;
        }
        LL mid = l + (r - l) / 2;
        Tag down = acc;
        down.apply(tr[p].tag);
        return query(tr[p].lc, l, mid, x, y, down) + query(tr[p].rc, mid + 1, r, x, y, down);
    }
    template<class Pred>
    LL find_first(int p, LL l, LL r, LL start, const Tag& acc, Pred pred)
    {
        if (r < start) return -1;
        Info cur = get_info(p, l, r);
        cur.apply(acc);
        if (!pred(cur)) return -1;
        if (l == r) return l;
        LL mid = l + (r - l) / 2;
        Tag down = acc;
        down.apply(tr[p].tag);
        LL res = find_first(tr[p].lc, l, mid, start, down, pred);
        if (res == -1) res = find_first(tr[p].rc, mid + 1, r, start, down, pred);
        return res;
    }
    template<class Pred>
    LL find_last(int p, LL l, LL r, LL end, const Tag& acc, Pred pred)
    {
        if (l > end) return -1;
        Info cur = get_info(p, l, r);
        cur.apply(acc);
        if (!pred(cur)) return -1;
        if (l == r) return l;
        LL mid = l + (r - l) / 2;
        Tag down = acc;
        down.apply(tr[p].tag);
        LL res = find_last(tr[p].rc, mid + 1, r, end, down, pred);
        if (res == -1) res = find_last(tr[p].lc, l, mid, end, down, pred);
        return res;
    }
    int build(LL l, LL r, const vector<Info>& a)
    {
        int p = fork(0, r - l + 1);
        if (l == r)
        {
            tr[p].info = a[l];
            tr[p].info.len = 1;
            return p;
        }
        LL mid = l + (r - l) / 2;
        tr[p].lc = build(l, mid, a);
        tr[p].rc = build(mid + 1, r, a);
        pushup(p, l, r);
        return p;
    }
};
#endif

/*
Usage:
// 1. 结构定义示例 (区间计数)
struct Tag
{
    LL c = 0;
    void apply(const Tag& t)
    {
        c += t.c;
    }
};
struct Info
{
    LL len = 0;
    LL cnt = 0;
    void apply(const Tag& t) { cnt += t.c * len; }
    friend Info operator+(const Info& a, const Info& b)
    {
        if (a.len == 0) return b;
        if (b.len == 0) return a;
        Info r;
        r.len = a.len + b.len;
        r.cnt = a.cnt + b.cnt;
        return r;
    }
};

// 2. 用法 A: 权值模式 (经典静态区间/树上第 k 小)
// 版本 0 是空树，通过外置版本数组 rt 管理每个时刻的根。
//
// 预处理离散化后，设 m 为离散化后值域大小:
// PersSegTree<Info, Tag> seg(m, (n + 1) * 20);
// VI rt(n + 1, 0);
//
// for (int i = 1; i <= n; i++)
// {
//     int pos = id(a[i]);
//     rt[i] = seg.modify(rt[i - 1], pos, pos, {1});
// }
//
// 查询静态区间 [l, r] 第 k 小对应的离散化前原值:
// LL pos = seg.find_kth({rt[r]}, {rt[l - 1]}, k);
// ans = vals[pos - 1];
//
// 树上路径 [u, v] 第 k 小:
// seg.find_kth({rt[u], rt[v]}, {rt[lca], rt[fa_lca]}, k);

// 3. 用法 B: 数组模式 (可持久化数组 / 历史版本访问)
// 传入原数组一键建树，并保存基准版本号。
//
// int v0 = seg.build(a);                    // 根据 a[1..n] 建立初始版本 0
// int v1 = seg.modify(v0, pos, pos, {v});   // 在版本 v0 基础上修改，产生新版本 v1, v0 不受影响
// Info res = seg.query(v0, l, r);           // 穿梭时空，访问任意历史版本

// 4. 用法 C: 范围修改模式 (标记永久化, 只支持加法类可交换标记)
// int v2 = seg.modify(v1, L, R, {k});       // 版本 v1 上 [L,R] 整体加 k, 生成新版本 v2
// Info r1 = seg.query(v1, L, R);            // 查询任意历史版本, v1 时代不受 v2 影响
// 二分接口 pred 判定的已是含标记的真实信息, 用法与普通线段树一致
================================================================================
*/
