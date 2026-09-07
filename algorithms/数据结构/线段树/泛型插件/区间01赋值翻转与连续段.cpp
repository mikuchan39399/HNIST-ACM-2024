#ifndef Z_OI_SEGBINARY_PLUGIN
#define Z_OI_SEGBINARY_PLUGIN
#include "../../../杂项/utils/utils.cpp"

// 01 赋值/翻转, 查询 1 个数与最长连续 0/1; 左右信息不可交换
// 仅接已 build 的树: 全零段的 pre/suf/best[0] 也等于 len, 不能只补 len
// 合并/作用 O(1); Info 32B, Tag 8B; 普通树 n=2e5 约 32MB
namespace SegBinary
{
struct Tag
{
    int set = -1; // -1 不赋值, 否则只能为 0/1
    bool flip = false; // 先赋值, 再翻转
    void apply(const Tag& t)
    {
        if (t.set != -1) *this = t;
        else flip ^= t.flip;
    }
    void clear() { *this = {}; }
    bool has_tag() const { return set != -1 || flip; }
};
struct Info
{
    int len = 0, ones = 0;
    array<int, 2> pre{}, suf{}, best{};
    Info() = default;
    explicit Info(int x) : len(1), ones(x) { pre[x] = suf[x] = best[x] = 1; }
    bool break_cond(const Tag&) const { return false; }
    bool tag_cond(const Tag&) const { return true; }
    void apply(const Tag& t)
    {
        if (t.set != -1)
        {
            ones = t.set * len;
            pre = suf = best = {};
            pre[t.set] = suf[t.set] = best[t.set] = len;
        }
        if (t.flip)
        {
            ones = len - ones;
            swap(pre[0], pre[1]); swap(suf[0], suf[1]); swap(best[0], best[1]);
        }
    }
    friend Info operator+(const Info& a, const Info& b)
    {
        Info c;
        c.len = a.len + b.len; c.ones = a.ones + b.ones;
        for (int x = 0; x < 2; x++)
        {
            c.pre[x] = a.pre[x] + (a.pre[x] == a.len ? b.pre[x] : 0);
            c.suf[x] = b.suf[x] + (b.suf[x] == b.len ? a.suf[x] : 0);
            c.best[x] = max({a.best[x], b.best[x], a.suf[x] + b.pre[x]});
        }
        return c;
    }
};
}
#endif

/* Usage:
#include "../泛型线段树.cpp"
using Info = SegBinary::Info;
using Tag = SegBinary::Tag;
int main()
{
    vector<Info> a(6, Info(0));
    SegTree<Info, Tag> seg(int(a.size()) - 1); seg.build(a);
    seg.modify(2, 4, {1, false});
    seg.modify(3, 5, {-1, true}); // 0,1,0,0,1
    auto v = seg.query(1, 5);
    cout << v.ones << ' ' << v.best[0] << '\n'; // 2 2
}
*/
