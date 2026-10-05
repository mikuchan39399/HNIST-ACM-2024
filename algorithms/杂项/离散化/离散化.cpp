// zoi: discrete
#ifndef Z_OI_DCR
#define Z_OI_DCR

#include "../utils/utils.cpp"

// 离散排名 1-based; 输入 vector 全部元素参与, 不跳过 [0]. 空间 O(n)
// T 的 < 与 == 须一致, 浮点不能含 NaN
template<class T>
struct Dcr
{
    Dcr() {}
    vector<T> v;
    bool built = true;
    Dcr(const vector<T>& _v) : v(_v) { build(); }
    // 追加候选, 旧排名失效. 均摊 O(1)
    void add(const T& x) { v.push_back(x); built = false; }
    // 仅预留容量. 至多 O(n) 搬移
    void reserve(size_t n) { v.reserve(n); }
    // 清空并保留容量. O(n) 析构
    void clear() { v.clear(); built = true; }
    // 排序去重并重建排名. 时间 O(n log n), 栈 O(log n)
    void build()
    {
        sort(v.begin(), v.end());
        v.erase(unique(v.begin(), v.end()), v.end());
        assert(v.size() <= INT_MAX);
        built = true;
    }
    // 已添加值 x 的排名; 未 build 会自动重建, 否则 O(log n)
    int operator()(const T& x)
    {
        if (!built) build();
        auto it = lower_bound(v.begin(), v.end(), x);
        assert(it != v.end() && *it == x);
        return it - v.begin() + 1;
    }
    // 不同值数, 须先 build. O(1)
    int size() const { assert(built); return v.size(); }
    // 按排名还原原值, 须先 build, 1<=idx<=size(). O(1)
    const T& operator[](int idx) const
    {
        assert(built && idx >= 1 && idx <= (int)v.size());
        return v[idx - 1];
    }
};
#endif

/* Usage
#include "discrete.h"

int main()
{
    VLL a{40, -7, 40, LLONG_MAX}; // 没有占位哨兵, 四个元素全部参与离散化
    Dcr<LL> d(a);
    cout << d.size() << ' ' << d(40) << ' ' << d[1] << '\n'; // 3 2 -7
    d.add(-10);
    d.build();                  // 新值可能移动旧排名, 外部保存的排名须重新计算
    cout << d(40) << '\n';       // 3
    d.clear();
    d.reserve(200000);
    d.add(8);
    cout << d(8) << '\n';        // 1, 排名查询会自动 build
}
*/
