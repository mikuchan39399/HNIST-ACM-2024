#ifndef Z_OI_SEGAFFINE_PLUGIN
#define Z_OI_SEGAFFINE_PLUGIN
#include "../../../杂项/utils/utils.cpp"

// x <- mul*x+add, 查询和/平方和; 2<=Mod<=INT_MAX, 可合数, 自动归一化负数
// Tag(m,a) 表示乘 m 加 a, m=0 即赋值. 合并/作用 O(1), 普通树空间 O(n)
namespace SegAffine
{
template<int Mod = 998244353>
struct Tag
{
    static_assert(Mod >= 2);
    LL mul, add;
    Tag(LL m = 1, LL a = 0) : mul((m % Mod + Mod) % Mod), add((a % Mod + Mod) % Mod) {}
    void apply(const Tag& t)
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
        sq = (a + b + c) % Mod;
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
