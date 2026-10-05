#ifndef Z_OI_SEGLINEAR_PLUGIN
#define Z_OI_SEGLINEAR_PLUGIN
#include "../../../杂项/utils/utils.cpp"

// a[i]+=k*i+b, 查询和; i 为建树绝对下标, 中间值在 LL 内
// 须 build, 叶子 Info(value,index). 合并/作用 O(1), 普通树空间 O(n)
namespace SegLinear
{
struct Tag
{
    LL k = 0, b = 0;
    // 生成首项 first、公差 step 的加法标记; 调用 modify(l,r,Tag::progression(l,first,step))
    static Tag progression(LL l, LL first, LL step) { return {step, first - step * l}; }
    void apply(const Tag& t) { k += t.k; b += t.b; }
    void clear() { *this = {}; }
    bool has_tag() const { return k != 0 || b != 0; }
};
struct Info
{
    LL len = 0, sum = 0, index_sum = 0;
    Info() = default;
    Info(LL x, LL index) : len(1), sum(x), index_sum(index) {}
    bool break_cond(const Tag&) const { return false; }
    bool tag_cond(const Tag&) const { return true; }
    void apply(const Tag& t) { sum += t.k * index_sum + t.b * len; }
    friend Info operator+(const Info& a, const Info& b)
    {
        Info c;
        c.len = a.len + b.len; c.sum = a.sum + b.sum; c.index_sum = a.index_sum + b.index_sum;
        return c;
    }
};
}
#endif
