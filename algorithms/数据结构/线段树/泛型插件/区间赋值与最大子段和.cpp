#ifndef Z_OI_SEGMAXSUBARRAY_PLUGIN
#define Z_OI_SEGMAXSUBARRAY_PLUGIN
#include "../../../杂项/utils/utils.cpp"

// 区间赋值, 查询非空最大子段和/前缀和/后缀和; 不支持区间加
// 全负段返回最大元素; 空单位元 best=0 不是非空段答案; 中间值须在 LL 内
// 合并/作用 O(1); Info 40B, Tag 16B; 普通树 n=2e5 约 44.8MB
namespace SegMaxSubarray
{
struct Tag
{
    LL value = 0;
    bool has_set = false;
    static Tag assign(LL x) { return {x, true}; }
    void apply(const Tag& t) { if (t.has_set) *this = t; }
    void clear() { *this = {}; }
    bool has_tag() const { return has_set; }
};
struct Info
{
    LL len = 0, sum = 0, pre = 0, suf = 0, best = 0;
    Info() = default;
    explicit Info(LL x) : len(1), sum(x), pre(x), suf(x), best(x) {}
    bool break_cond(const Tag&) const { return false; }
    bool tag_cond(const Tag&) const { return true; }
    void apply(const Tag& t)
    {
        if (!len || !t.has_set) return;
        sum = t.value * len;
        pre = suf = best = max(t.value, sum);
    }
    friend Info operator+(const Info& a, const Info& b)
    {
        if (!a.len) return b;
        if (!b.len) return a;
        Info c;
        c.len = a.len + b.len; c.sum = a.sum + b.sum;
        c.pre = max(a.pre, a.sum + b.pre); c.suf = max(b.suf, b.sum + a.suf);
        c.best = max({a.best, b.best, a.suf + b.pre});
        return c;
    }
};
}
#endif

/* Usage:
#include "../泛型线段树.cpp"
using Info = SegMaxSubarray::Info;
using Tag = SegMaxSubarray::Tag;
int main()
{
    vector<Info> a = {{}, Info(-2), Info(3), Info(-1), Info(4)};
    SegTree<Info, Tag> seg(int(a.size()) - 1); seg.build(a);
    cout << seg.query(1, 4).best << '\n'; // 6
    seg.modify(1, 4, Tag::assign(-5));
    cout << seg.query(1, 4).best << '\n'; // -5
}
*/
