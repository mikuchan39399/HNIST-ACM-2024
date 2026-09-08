// zoi: discrete
#ifndef Z_OI_DCR
#define Z_OI_DCR

#include "../utils/utils.cpp"

// 离散排名为 1-based, 输入 vector 的所有元素都参与, 不自动跳过下标 0
// T 的 < 与 == 应一致, 不支持含 NaN 的浮点序列; 返回索引须能放入 int
template<class T>
struct Dcr
{
    Dcr() {}
    vector<T> v;
    bool built = true;
    Dcr(const vector<T>& _v) : v(_v) { build(); }
    // 追加候选值, 使旧排名失效; 摊还 O(1), 容量不足时 O(n) 搬移
    void add(const T& x) { v.push_back(x); built = false; }
    // 只预留容量不添加值, 至多 O(n) 搬移, 按 T 的大小分配
    void reserve(size_t n) { v.reserve(n); }
    // 清空全部值和排名, 保留容量; O(n) 析构, 旧引用失效
    void clear() { v.clear(); built = true; }
    // 排序去重并重建全部排名, O(n log n) 时间; n 个 T 的存储加排序栈
    void build()
    {
        sort(v.begin(), v.end());
        v.erase(unique(v.begin(), v.end()), v.end());
        assert(v.size() <= INT_MAX);
        built = true;
    }
    // 返回已添加值 x 的排名; 未 build 时先重建, 平时 O(log n)
    int operator()(const T& x)
    {
        if (!built) build();
        auto it = lower_bound(v.begin(), v.end(), x);
        assert(it != v.end() && *it == x);
        return it - v.begin() + 1;
    }
    // 返回不同值数目, 要求已 build; O(1)
    int size() const { assert(built); return v.size(); }
    // 按 1-based 排名还原原值, 要求已 build 且 1<=idx<=size; O(1)
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
