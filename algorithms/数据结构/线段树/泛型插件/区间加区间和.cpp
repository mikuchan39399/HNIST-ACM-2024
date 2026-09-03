#include <algorithm>
using namespace std;

using LL = long long;

// |=======================================================================================|
// 区间加法 + 区间求和 (SegTree<Info, Tag> 直接组装)
struct Tag
{
    LL add = 0;
    void apply(const Tag& t)    // 负责懒标记叠加
    {
        add += t.add;
    }
    void clear()                // 清空懒标记, 只用于 pushdown
    {
        add = 0;
    }
    bool has_tag()              // 仅在 pushdown 中判断是否存在有效懒标记, 不用懒标记则恒返回 false
    {
        return add != 0;
    }
};
struct Info
{
    LL sum = 0;
    int len = 0;
    bool break_cond(const Tag&)   // 区间剪枝条件, 用于势能线段树, 非势能场景则恒返回 false
    {
        return false;
    }
    bool tag_cond(const Tag&)     // 用于判断势能线段树中是否能区间修改, 非势能场景恒返回 true
    {
        return true;
    }
    void apply(const Tag& t)        // 懒标记应用上具体区间信息
    {
        sum += t.add * len;
    }
    friend Info operator+(const Info& a, const Info& b)     // 维护信息间的合并
    {
        if (a.len == 0) return b;
        if (b.len == 0) return a;
        Info c;
        c.sum = a.sum + b.sum;
        c.len = a.len + b.len;
        return c;
    }
};
