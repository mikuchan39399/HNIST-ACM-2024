#ifndef Z_OI_SEGLINEAR_PLUGIN
#define Z_OI_SEGLINEAR_PLUGIN
#include "../../../杂项/utils/utils.cpp"

// a[i] += k * i + b, 查询和; i 是建树时的绝对下标
// 必须 build, 叶子 Info(value, index); 所有中间值须在 LL 内
// 合并/作用 O(1); 普通树 O(n) 空间, n = 2e5 约 32 MB
namespace SegLinear
{
struct Tag
{
    LL k = 0, b = 0;
    // [l, r] 加首项 first、公差 step 的数列; 仍须 modify(l, r, ...)
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
