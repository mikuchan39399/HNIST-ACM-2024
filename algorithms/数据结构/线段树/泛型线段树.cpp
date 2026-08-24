#include <vector>
#include <algorithm>

using namespace std;
using LL = long long;
const LL INF = 0x3f3f3f3f3f3f3f3f;

#define lc (p << 1)
#define rc (p << 1 | 1)
// ============ 线段树 (支持懒标记、线段树上二分) ============
//   1. 结构解耦: 基于 Info (区间信息) 和 Tag (懒标记) 的代数结构解耦，只需修改两者的结构体即可适配不同题目。
//   2. 编译期优化: 使用 if constexpr 自动推导并适配是否需要拆分标记 (split_tag) 或萃取有效标记 (get_real_tag)。
//   3. 二分查找: 内置 find_first 和 find_last，支持传入 Pred 谓词进行 O(log N) 的线段树上二分定位。
template<class Info, class Tag>
struct SegTree
{
    int n;
    vector<Info> info;
    vector<Tag> tag;
    SegTree(int max_n) : 
        n(max_n), info(4 * max_n + 10), tag(4 * max_n + 10) 
    {
         tr.reserve(max_nodes + 1);
        tr.push_back(Node{});
    }
    void init(int _n)
    {
        n = _n;
        for (int i = 0; i <= 4 * n; i++)
        {
            info[i] = Info{};
            tag[i] = Tag{};
        }
    }
    void pushup(int p) { info[p] = info[lc] + info[rc]; }
    void lazy(int p, const Tag& v)
    {
        info[p].apply(v);
        tag[p].apply(v);
    }
    void pushdown(int p)
    {
        if (tag[p].has_tag())
        {
            if constexpr (requires { info[p].split_tag(tag[p], info[lc], info[rc]); })
            {
                auto [tagl, tagr] = info[p].split_tag(tag[p], info[lc], info[rc]);
                lazy(lc, tagl);
                lazy(rc, tagr);
            }
            else
            {
                lazy(lc, tag[p]);
                lazy(rc, tag[p]);
            }
            tag[p].clear();
        }
    }
    void build(int p, int l, int r, const vector<Info>& a)
    {
        if (l == r)
        {
            info[p] = a[l];
            info[p].len = 1;
            return;
        }
        int mid = (l + r) >> 1;
        build(lc, l, mid, a);
        build(rc, mid + 1, r, a);
        pushup(p);
    }
    void modify(int p, int l, int r, int x, int y, const Tag& v)
    {
        if (l > y || r < x || info[p].break_cond(v)) return;
        if (l >= x && r <= y && info[p].tag_cond(v))
        {
            if constexpr (requires { info[p].get_real_tag(v); })
            {
                lazy(p, info[p].get_real_tag(v));
            }
            else
            {
                lazy(p, v);
            }
            return;
        }
        pushdown(p);
        int mid = (l + r) >> 1;
        modify(lc, l, mid, x, y, v);
        modify(rc, mid + 1, r, x, y, v);
        pushup(p);
    }
    Info query(int p, int l, int r, int x, int y)
    {
        if (l > y || r < x) return Info{};
        if (l >= x && r <= y) return info[p];
        pushdown(p);
        int mid = (l + r) >> 1;
        return query(lc, l, mid, x, y) + query(rc, mid + 1, r, x, y);
    }
    template<class Pred>
    int find_first(int p, int l, int r, int start, Pred pred)
    {
        if (r < start || !pred(info[p])) return -1;
        if (l == r) return l;
        pushdown(p);
        int mid = (l + r) >> 1;
        int res = find_first(lc, l, mid, start, pred);
        if (res == -1)
            res = find_first(rc, mid + 1, r, start, pred);
        return res;
    }
    template<class Pred>
    int find_last(int p, int l, int r, int end, Pred pred)
    {
        if (l > end || !pred(info[p])) return -1;
        if (l == r) return l;
        pushdown(p);
        int mid = (l + r) >> 1;
        int res = find_last(rc, mid + 1, r, end, pred);
        if (res == -1)
            res = find_last(lc, l, mid, end, pred);
        return res;
    }
    // --- 外部 API ---
    // 区间 [x, y] 应用标记 v
    void modify(int x, int y, const Tag& v) { modify(1, 1, n, x, y, v); }
    // 查询区间 [x, y] 的信息
    Info query(int x, int y) { return query(1, 1, n, x, y); }
    // 根据 a 数组构建线段树 (a 为 1-base)
    void build(const vector<Info>& a) { build(1, 1, n, a); }
    // 查找下标 >= start 且满足 pred 条件的第一个位置
    template<class Pred>
    int find_first(int start, Pred pred)
    {
        if (start < 1 || start > n) return -1;
        return find_first(1, 1, n, start, pred);
    }
    // 查找下标 <= end 且满足 pred 条件的最后一个位置
    template<class Pred>
    int find_last(int end, Pred pred)
    {
        if (end < 1 || end > n) return -1;
        return find_last(1, 1, n, end, pred);
    }
};
// ==================== 区间加法区间求和 ====================
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
    bool break_cond(const Tag& t)   // 区间剪枝条件, 用于势能线段树, 非势能场景则恒返回 false
    { 
        return false; 
    }
    bool tag_cond(const Tag& t)     // 用于判断势能线段树中是否能区间修改, 非势能场景恒返回 true
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

/*
 * Usage:
 * // 1. 初始化
 * // N 为最大可能区间长度
 * SegTree<Info, Tag> seg(N);
 * vector<Info> a(n + 1);
 * for (int i = 1; i <= n; i++)
 * {
 *     a[i] = {w[i], 1};  // w 数组为初始权值，len 固定设为 1
 * }
 * seg.init(n);
 * seg.build(a);
 * 
 * // 2. 核心操作 (x, y 均为 1-base 的逻辑区间下标)
 * seg.modify(x, y, {k}); // 区间 [x, y] 统一加上 k (Tag的初始化列表)
 * Info res = seg.query(x, y); // 查询区间 [x, y] 的合并信息
 * LL ans = res.sum; // 从返回的 Info 结构体中提取所需属性
 * 
 * // 3. 线段树上二分
 * // 例: 查找下标 >= 1 且区间和 >= k 的第一个叶子节点位置
 * auto pred = [&](const Info& info) 
 * {
 *     return info.sum >= k; 
 * };
 * int pos = seg.find_first(1, pred);
 * 
 * // 4. 定制化 (应对不同题目)
 * // - 仅需修改底部的 Tag 和 Info 结构体内部逻辑。
 * // - Info 负责维护数据维度及合并 (operator+)。
 * // - Tag 负责标记维度的叠加 (apply)。
 */