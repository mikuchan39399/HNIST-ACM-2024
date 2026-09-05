// zoi: seg
#ifndef Z_OI_SEG
#define Z_OI_SEG

#include <vector>
#include <algorithm>
#include "../../杂项/utils/utils.cpp"

using namespace std;

// 线段树维护 a[1..n], Info 决定区间存什么, Tag 决定怎样修改, 支持区间修改、查询和找位置
// 每结点 sizeof(Info)+sizeof(Tag) 字节; 预留 4*max_n+10 个, 每结点 32B 时 n=2e5 约 25.6MB
// find_first/find_last 的 pred 判断区间内是否存在答案, 无答案的整段必须返回 false
template<class Info, class Tag>
struct SegTree
{
    int n;
    vector<Info> info;
    vector<Tag> tag;
    // 一次分配长度上限 max_n 所需的表; 使用前 build(a), a[0] 不参与建树
    // 时间: O(max_n) | 空间: O(max_n)
    SegTree(int max_n) :
        n(max_n), info(4 * max_n + 10), tag(4 * max_n + 10)
    {}
    // 清空信息和标记并设置本轮长度 _n, _n 不超过构造上限; 随后重新 build
    // 时间: O(n) | 空间: O(1)
    void init(int _n)
    {
        n = _n;
        for (int i = 0; i <= 4 * n; i++)
        {
            info[i] = Info{};
            tag[i] = Tag{};
        }
    }
    // 把标记 v 应用到闭区间 [x,y], 单点修改传 x==y
    // 时间: 普通懒标记 O(log n), 势能修改按题分析 | 空间: O(log n)
    void modify(int x, int y, const Tag& v) { modify(1, 1, n, x, y, v); }
    // 返回闭区间 [x,y] 合并后的 Info
    // 时间: O(log n) | 空间: O(log n)
    Info query(int x, int y) { return query(1, 1, n, x, y); }
    // 用 a[1..n] 建树, a[0] 不用; 重建前先 init(n) 清除旧标记
    // 时间: O(n) | 空间: O(log n)
    void build(const vector<Info>& a) { build(1, 1, n, a); }
    // 返回 [start,n] 内满足 pred 的最左位置, 不存在或 start 越界返回 -1
    // 时间: 可正确剪枝时 O(log n) | 空间: O(log n)
    template<class Pred>
    int find_first(int start, Pred pred)
    {
        if (start < 1 || start > n) return -1;
        return find_first(1, 1, n, start, pred);
    }
    // 返回 [1,end] 内满足 pred 的最右位置, 不存在或 end 越界返回 -1
    // 时间: 可正确剪枝时 O(log n) | 空间: O(log n)
    template<class Pred>
    int find_last(int end, Pred pred)
    {
        if (end < 1 || end > n) return -1;
        return find_last(1, 1, n, end, pred);
    }
private:
    void pushup(int p) { info[p] = info[p << 1] + info[p << 1 | 1]; }
    void lazy(int p, const Tag& v)
    {
        info[p].apply(v);
        tag[p].apply(v);
    }
    void pushdown(int p)
    {
        if (tag[p].has_tag())
        {
            if constexpr (requires { info[p].split_tag(tag[p], info[p << 1], info[p << 1 | 1]); })
            {
                auto [tagl, tagr] = info[p].split_tag(tag[p], info[p << 1], info[p << 1 | 1]);
                lazy(p << 1, tagl);
                lazy(p << 1 | 1, tagr);
            }
            else
            {
                lazy(p << 1, tag[p]);
                lazy(p << 1 | 1, tag[p]);
            }
            tag[p].clear();
        }
    }
    void build(int p, int l, int r, const vector<Info>& a)
    {
        if (l == r)
        {
            info[p] = a[l];
            info[p].len = 1;
            return;
        }
        int mid = (l + r) >> 1;
        build(p << 1, l, mid, a);
        build(p << 1 | 1, mid + 1, r, a);
        pushup(p);
    }
    void modify(int p, int l, int r, int x, int y, const Tag& v)
    {
        if (l > y || r < x || info[p].break_cond(v)) return;
        if (l >= x && r <= y && info[p].tag_cond(v))
        {
            if constexpr (requires { info[p].get_real_tag(v); })
            {
                lazy(p, info[p].get_real_tag(v));
            }
            else
            {
                lazy(p, v);
            }
            return;
        }
        pushdown(p);
        int mid = (l + r) >> 1;
        modify(p << 1, l, mid, x, y, v);
        modify(p << 1 | 1, mid + 1, r, x, y, v);
        pushup(p);
    }
    Info query(int p, int l, int r, int x, int y)
    {
        if (l > y || r < x) return Info{};
        if (l >= x && r <= y) return info[p];
        pushdown(p);
        int mid = (l + r) >> 1;
        return query(p << 1, l, mid, x, y) + query(p << 1 | 1, mid + 1, r, x, y);
    }
    template<class Pred>
    int find_first(int p, int l, int r, int start, Pred pred)
    {
        if (r < start || !pred(info[p])) return -1;
        if (l == r) return l;
        pushdown(p);
        int mid = (l + r) >> 1;
        int res = find_first(p << 1, l, mid, start, pred);
        if (res == -1)
            res = find_first(p << 1 | 1, mid + 1, r, start, pred);
        return res;
    }
    template<class Pred>
    int find_last(int p, int l, int r, int end, Pred pred)
    {
        if (l > end || !pred(info[p])) return -1;
        if (l == r) return l;
        pushdown(p);
        int mid = (l + r) >> 1;
        int res = find_last(p << 1 | 1, mid + 1, r, end, pred);
        if (res == -1)
            res = find_last(p << 1, l, mid, end, pred);
        return res;
    }
};
#endif

/*
 * Usage:
 * // 1. 初始化
 * // N 为最大可能区间长度
 * SegTree<Info, Tag> seg(N);
 * vector<Info> a(n + 1);
 * for (int i = 1; i <= n; i++)
 * {
 *     a[i] = {w[i], 1};  // w 数组为初始权值，len 固定设为 1
 * }
 * seg.init(n);
 * seg.build(a);
 *
 * // 2. 核心操作 (x, y 均为 1-base 的逻辑区间下标)
 * seg.modify(x, y, {k}); // 区间 [x, y] 统一加上 k (Tag的初始化列表)
 * Info res = seg.query(x, y); // 查询区间 [x, y] 的合并信息
 * LL ans = res.sum; // 从返回的 Info 结构体中提取所需属性
 *
 * // 3. 线段树上二分
 * // 例: 查找下标 >= 1 且区间和 >= k 的第一个叶子节点位置
 * auto pred = [&](const Info& info)
 * {
 *     return info.sum >= k;
 * };
 * int pos = seg.find_first(1, pred);
 *
 * // 4. 定制化 (应对不同题目)
 * // - 代数层自备: 现成件抄 数据结构/线段树/泛型插件/ (区间加区间和、
 *   区间历史最值), 按题改字段; Info 管信息合并(operator+), Tag 管标记叠加(apply)。
 */
