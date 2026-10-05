#ifndef Z_OI_SEGMAXSUBARRAY_PLUGIN
#define Z_OI_SEGMAXSUBARRAY_PLUGIN
#include "../../../杂项/utils/utils.cpp"

// 区间赋值, 查询非空最大子段/前缀/后缀和; 不支持区间加, 中间值在 LL 内
// 全负取最大元素, Info{} 为空单位元. 合并/作用 O(1), 普通树空间 O(n)
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
