#ifndef Z_OI_SEGLINEAR_PLUGIN
#define Z_OI_SEGLINEAR_PLUGIN
#include "../../../杂项/utils/utils.cpp"

// a[i] += k*i+b, 查询和; i 是建树时的绝对下标, 不随覆盖段重算
// 仅接已 build 的树, 叶子须填 index; 所有乘加中间值须在 LL 内
// 合并/作用 O(1); Info 24B, Tag 16B; 普通树 n=2e5 约 32MB
namespace SegLinear
{
struct Tag
{
    LL k = 0, b = 0;
    // [l,r] 加 first,first+step,...; 范围仍由 modify(l,r,...) 指定
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

/* Usage:
#include "../泛型线段树.cpp"
using Info = SegLinear::Info;
using Tag = SegLinear::Tag;
int main()
{
    vector<Info> a(6);
    for (int i = 1; i <= 5; i++) a[i] = Info(0, i);
    SegTree<Info, Tag> seg(int(a.size()) - 1); seg.build(a);
    seg.modify(2, 5, Tag::progression(2, 3, 2)); // 0,3,5,7,9
    cout << seg.query(3, 4).sum << '\n'; // 12
}
*/
