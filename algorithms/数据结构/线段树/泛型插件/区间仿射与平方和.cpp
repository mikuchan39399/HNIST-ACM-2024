#ifndef Z_OI_SEGAFFINE_PLUGIN
#define Z_OI_SEGAFFINE_PLUGIN
#include "../../../杂项/utils/utils.cpp"

// x <- mul*x+add, 查询和/平方和; Mod 在 [2,INT_MAX], 无须素数
// 乘 0 即赋值; 构造归一化负数; 合并/作用 O(1)
// Info 24B, Tag 16B; 普通树 n=2e5 约 32MB
namespace SegAffine
{
template<int Mod = 998244353>
struct Tag
{
    static_assert(Mod >= 2);
    LL mul, add;
    Tag(LL m = 1, LL a = 0) : mul((m % Mod + Mod) % Mod), add((a % Mod + Mod) % Mod) {}
    void apply(const Tag& t) // 先旧后新: t.mul*(mul*x+add)+t.add
    {
        mul = mul * t.mul % Mod;
        add = (add * t.mul + t.add) % Mod;
    }
    void clear() { *this = {}; }
    bool has_tag() const { return mul != 1 || add != 0; }
};
template<int Mod = 998244353>
struct Info
{
    LL len = 0, sum = 0, sq = 0;
    Info() = default;
    explicit Info(LL x) : len(1), sum((x % Mod + Mod) % Mod), sq(sum * sum % Mod) {}
    bool break_cond(const Tag<Mod>&) const { return false; }
    bool tag_cond(const Tag<Mod>&) const { return true; }
    void apply(const Tag<Mod>& t)
    {
        LL a = sq * t.mul % Mod * t.mul % Mod;
        LL b = 2 * t.mul % Mod * t.add % Mod * sum % Mod;
        LL c = t.add * t.add % Mod * (len % Mod) % Mod;
        sq = (a + b + c) % Mod; // 先用旧 sum 更新平方和
        sum = (sum * t.mul + t.add * (len % Mod)) % Mod;
    }
    friend Info operator+(const Info& a, const Info& b)
    {
        Info c;
        c.len = a.len + b.len; c.sum = (a.sum + b.sum) % Mod; c.sq = (a.sq + b.sq) % Mod;
        return c;
    }
};
}
#endif

/* Usage:
#include "../泛型线段树.cpp"
using Info = SegAffine::Info<>;
using Tag = SegAffine::Tag<>;
int main()
{
    vector<Info> a = {{}, Info(1), Info(2), Info(3)};
    SegTree<Info, Tag> seg(3); seg.build(a);
    seg.modify(1, 3, Tag(2, 1)); // 3,5,7
    cout << seg.query(1, 3).sum << ' ' << seg.query(1, 3).sq << '\n'; // 15 83
    seg.modify(2, 3, Tag(0, -1)); // 赋成 -1 的模意义值
}
*/
