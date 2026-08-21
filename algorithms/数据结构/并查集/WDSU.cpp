#include <vector>
#include <numeric>
#include <cassert>

using LL = long long;
using VI = std::vector<int>;
using VLL = std::vector<LL>;

#ifndef Z_OI_WDSU
#define Z_OI_WDSU
struct WDSU 
{
    static constexpr LL INF = 0x3f3f3f3f3f3f3f3fLL;
    LL mod;
    VI fa;
    VLL d;
    WDSU(int n = 0, LL mod_ = 0) : mod(mod_), fa(n + 1), d(n + 1, 0) 
    {
        assert(mod >= 0);
        std::iota(fa.begin(), fa.end(), 0);
    }
    LL norm(LL v) const { return mod ? (v % mod + mod) % mod : v; }
    // 查找 x 所在集合的根，并压缩路径，维护 x 到根的有向距离 d(x -> root)
    // 返回值: 根节点编号
    // 时间: 均摊 O(log n) | 空间: O(递归深度)
    int find(int x) 
    {
        if (fa[x] == x) return x;
        int root = find(fa[x]);
        d[x] = norm(d[x] + d[fa[x]]);
        return fa[x] = root;
    }
    // 声明有向距离 d(x -> y) = w 并合并集合（若已连通则忽略，不校验矛盾）
    // 时间: 均摊 O(log n) | 空间: O(1)
    void merge(int x, int y, LL w) {
        int fx = find(x), fy = find(y);
        if (fx == fy) return;
        d[fy] = norm(d[x] - w - d[y]);
        fa[fy] = fx;
    }
    // 判断 x 和 y 是否属于同一个集合
    // 返回值: true 表示连通，false 表示未连通
    // 时间: 均摊 O(log n) | 空间: O(1)
    bool same(int x, int y) { return find(x) == find(y); }
    // 查询有向距离 d(x -> y)
    // 返回值: 连通时返回距离值，未连通时返回 INF
    // 时间: 均摊 O(log n) | 空间: O(1)
    LL query(int x, int y) 
    {
        if (find(x) != find(y)) return INF;
        return norm(d[x] - d[y]);
    }
};
#endif

/*
================================================================================
Usage:
================================================================================
一、 初始化模式选择：
  1. 真实距离模式: 
     WDSU ds(n) 或 WDSU ds(n, 0)
     - 适用场景：链合并、距离累加等（如：银河英雄传说）。
     - 注意：累加距离的绝对值上限为 Σ|w|，需确保不会超出 long long 范围。
  2. 逻辑归一模式: 
     WDSU ds(n, mod)
     - 适用场景：种类并查集、奇偶校验等（如：食物链选 mod=3，关押罪犯选 mod=2）。
     - 注意：距离会被自动约束在 [0, mod) 内，可直接读取 ds.d[i] 作为分类编号使用
       （读取前务必调用 ds.find(i) 更新状态）。
二、 基础操作说明：
  1. ds.merge(x, y, w)
     - 强声明 x 到 y 的向量距离 d(x -> y) = w 并执行合并。
  2. ds.query(x, y)
     - 查询 x 到 y 的距离 d(x -> y)。
     - 若返回 INF 代表两点当前互不连通。
     - 满足反对称性：query(x, y) == -query(y, x)（真实模式下）。
  3. ds.same(x, y)
     - 仅验证连通性。
================================================================================
*/