#ifndef Z_OI_SEGADD_PLUGIN
#define Z_OI_SEGADD_PLUGIN
#include "../../../杂项/utils/utils.cpp"

// 区间加, 查询和/最小值/最大值; 所有中间值须在 LL 内
// 合并/作用 O(1); 普通树 O(n) 空间, n = 2e5 约 32 MB
namespace SegAdd
{
struct Tag
{
    LL add = 0;
    void apply(const Tag& t) { add += t.add; }
    void clear() { *this = {}; }
    bool has_tag() const { return add != 0; }
};
struct Info
{
    LL len = 0, sum = 0, mn = 0, mx = 0;
    Info() = default; // len = 0 为空; 只补 len 可表示全零段
    explicit Info(LL x) : len(1), sum(x), mn(x), mx(x) {}
    bool break_cond(const Tag&) const { return false; }
    bool tag_cond(const Tag&) const { return true; }
    void apply(const Tag& t)
    {
        if (!len) return;
        sum += t.add * len; mn += t.add; mx += t.add;
    }
    friend Info operator+(const Info& a, const Info& b)
    {
        if (!a.len) return b;
        if (!b.len) return a;
        Info c;
        c.len = a.len + b.len; c.sum = a.sum + b.sum;
        c.mn = min(a.mn, b.mn); c.mx = max(a.mx, b.mx);
        return c;
    }
};
}
#endif
