// zoi: seg
#ifndef Z_OI_SEG
#define Z_OI_SEG

#include "../../杂项/utils/utils.cpp"

// 线段树维护 a[1..n], Info 决定区间存什么, Tag 决定怎样修改, 支持区间修改、查询和找位置
// 每结点 sizeof(Info)+sizeof(Tag) 字节; 预留 4*max_n+10 个, 每结点 32B 时 n=2e5 约 25.6MB
// 区间非空且在 [1, n] 内; find 的 pred 必须等价于区间内存在满足条件的点, 不累积前缀
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
    // 返回 [start, n] 内满足 pred 的最左位置, 不存在或 start 越界返回 -1
    // 时间: O(log n) | 空间: O(log n)
    template<class Pred>
    int find_first(int start, Pred pred)
    {
        if (start < 1 || start > n) return -1;
        return find_first(1, 1, n, start, pred);
    }
    // 返回 [1, end] 内满足 pred 的最右位置, 不存在或 end 越界返回 -1
    // 时间: O(log n) | 空间: O(log n)
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

/* Usage: 区间加、区间和与最大值, 按题改 Info/Tag
struct Tag
{
    LL add = 0;
    void apply(const Tag& t) { add += t.add; }
    void clear() { add = 0; }
    bool has_tag() const { return add != 0; }
};
struct Info
{
    LL len = 0, sum = 0, mx = 0;
    bool break_cond(const Tag&) const { return false; }
    bool tag_cond(const Tag&) const { return true; }
    void apply(const Tag& t) { sum += len * t.add; mx += t.add; }
    friend Info operator+(const Info& a, const Info& b)
    {
        if (!a.len) return b;
        if (!b.len) return a;
        return {a.len + b.len, a.sum + b.sum, max(a.mx, b.mx)};
    }
};
int main()
{
    SegTree<Info, Tag> seg(5);
    vector<Info> a = {{}, {1, 2, 2}, {1, -1, -1}, {1, 4, 4}};
    seg.init(3);
    seg.build(a);
    seg.modify(1, 2, {3});
    cout << seg.query(1, 3).sum << "\n"; // 11
    auto pred = [](const Info& v) { return v.mx >= 4; };
    cout << seg.find_first(2, pred) << " " << seg.find_last(3, pred) << "\n"; // 3 3
}
*/
