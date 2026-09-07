#ifndef Z_OI_SEGASSIGNADD_PLUGIN
#define Z_OI_SEGASSIGNADD_PLUGIN
#include "../../../杂项/utils/utils.cpp"

// 赋值、加法, 查询和/最小值/最大值; 赋值 0 也是有效操作
// 合并/作用 O(1); Info 32B, Tag 24B; 普通树 n=2e5 约 44.8MB
namespace SegAssignAdd
{
struct Tag
{
    LL value = 0, add = 0;
    bool has_set = false; // 先赋值(若有), 再加; 不用特殊数值代表未赋值
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

/* Usage:
#include "../泛型线段树.cpp"
using Info = SegAssignAdd::Info;
using Tag = SegAssignAdd::Tag;
int main()
{
    vector<Info> a(5, Info(0));
    SegTree<Info, Tag> seg(int(a.size()) - 1); seg.build(a);
    seg.modify(1, 4, Tag::assign(-3));
    seg.modify(2, 3, Tag::increase(5));
    cout << seg.query(1, 4).sum << '\n'; // -2
    seg.modify(1, 4, Tag::assign(0));
}
*/
