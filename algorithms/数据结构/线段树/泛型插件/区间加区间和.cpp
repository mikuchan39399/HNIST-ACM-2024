#ifndef Z_OI_SEGADD_PLUGIN
#define Z_OI_SEGADD_PLUGIN
#include "../../../杂项/utils/utils.cpp"

// 区间加, 查询和/最小值/最大值; 所有中间值须在 LL 内
// 合并/作用 O(1); Info 32B, Tag 8B; 普通树 n=2e5 约 32MB
namespace SegAdd
{
struct Tag
{
    LL add = 0;
    void apply(const Tag& t) { add += t.add; } // 旧操作之后再做 t
    void clear() { *this = {}; }
    bool has_tag() const { return add != 0; }
};
struct Info
{
    LL len = 0, sum = 0, mn = 0, mx = 0;
    Info() = default; // len=0 为空; 只补 len 也能表示全零段
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

/* Usage:
#include "../泛型线段树.cpp"
using Info = SegAdd::Info;
using Tag = SegAdd::Tag;
int main()
{
    vector<Info> a = {{}, Info(-2), Info(3), Info(1)};
    SegTree<Info, Tag> seg(int(a.size()) - 1); seg.build(a);
    seg.modify(1, 2, {4});
    cout << seg.query(1, 3).sum << '\n'; // 10
    cout << seg.find_first(1, [](const Info& v) { return v.mx >= 5; }) << '\n'; // 2
}
*/
