#ifndef Z_OI_SEGXOR_PLUGIN
#define Z_OI_SEGXOR_PLUGIN
#include "../../../杂项/utils/utils.cpp"

// 区间异或, 查询和/异或和/各位 1 个数; 数据及 mask 在 [0,2^Bits), len<=INT_MAX
// 合并/作用 O(Bits), 树操作 O(Bits*log n); Bits=20 时 Info 104B, Tag 4B, n=2e5 约 86.4MB
namespace SegXor
{
struct Tag
{
    unsigned mask = 0;
    void apply(const Tag& t) { mask ^= t.mask; }
    void clear() { mask = 0; }
    bool has_tag() const { return mask != 0; }
};
template<int Bits = 20>
struct Info
{
    static_assert(1 <= Bits && Bits <= 31);
    LL len = 0, sum = 0;
    array<int, Bits> cnt{};
    unsigned xr = 0;
    Info() = default;
    explicit Info(unsigned x) : len(1), sum(x), xr(x)
    {
        for (int b = 0; b < Bits; b++) cnt[b] = (x >> b) & 1U;
    }
    bool break_cond(const Tag&) const { return false; }
    bool tag_cond(const Tag&) const { return true; }
    void apply(const Tag& t)
    {
        for (int b = 0; b < Bits; b++) if ((t.mask >> b) & 1U)
        {
            sum += (len - 2LL * cnt[b]) * (1LL << b);
            cnt[b] = int(len - cnt[b]);
        }
        if (len & 1) xr ^= t.mask;
    }
    friend Info operator+(const Info& a, const Info& b)
    {
        Info c;
        c.len = a.len + b.len; c.sum = a.sum + b.sum; c.xr = a.xr ^ b.xr;
        for (int i = 0; i < Bits; i++) c.cnt[i] = a.cnt[i] + b.cnt[i];
        return c;
    }
};
}
#endif

/* Usage:
#include "../泛型线段树.cpp"
using Info = SegXor::Info<20>;
using Tag = SegXor::Tag;
int main()
{
    vector<Info> a = {{}, Info(1), Info(2), Info(3)};
    SegTree<Info, Tag> seg(int(a.size()) - 1); seg.build(a);
    seg.modify(1, 2, {3}); // 2,1,3
    auto v = seg.query(1, 3);
    cout << v.sum << ' ' << v.xr << ' ' << v.cnt[0] << '\n'; // 6 0 2
}
*/
