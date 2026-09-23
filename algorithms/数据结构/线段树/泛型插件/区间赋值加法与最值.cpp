#ifndef Z_OI_SEGASSIGNADD_PLUGIN
#define Z_OI_SEGASSIGNADD_PLUGIN
#include "../../../杂项/utils/utils.cpp"

// 区间赋值/加法, 查询和/最值; 中间值须在 LL 内
// 合并/作用 O(1); 普通树 O(n) 空间, n = 2e5 约 45 MB
namespace SegAssignAdd
{
struct Tag
{
    LL value = 0, add = 0;
    bool has_set = false; // 先赋值再加; 赋值 0 也有效
    static Tag assign(LL x) { return {x, 0, true}; }
    static Tag increase(LL x) { return {0, x, false}; }
    void apply(const Tag& t)
    {
        if (t.has_set) *this = t; // 新赋值抹掉旧操作
        else add += t.add;
    }
    void clear() { *this = {}; }
    bool has_tag() const { return has_set || add != 0; }
};
struct Info
{
    LL len = 0, sum = 0, mn = 0, mx = 0;
    Info() = default;
    explicit Info(LL x) : len(1), sum(x), mn(x), mx(x) {}
    bool break_cond(const Tag&) const { return false; }
    bool tag_cond(const Tag&) const { return true; }
    void apply(const Tag& t)
    {
        if (!len) return;
        if (t.has_set) { sum = t.value * len; mn = mx = t.value; }
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
