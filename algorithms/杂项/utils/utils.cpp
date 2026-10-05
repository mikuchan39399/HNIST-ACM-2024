// zoi: utils
#ifndef Z_OI_UTILS
#define Z_OI_UTILS

#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <climits>
#include <limits>
#include <cfloat>
#include <cstring>
#include <cstdint>
#include <iostream>
#include <string>
#if __cplusplus >= 201703L
#include <string_view>
#endif
#include <initializer_list>
#include <vector>
#include <utility>
#include <tuple>
#include <type_traits>
#include <algorithm>
#include <array>
#include <bitset>
#if __cplusplus >= 202002L
#include <bit>
#include <concepts>
#include <ranges>
#include <span>
#endif
#include <cmath>
#include <functional>
#include <map>
#include <numeric>
#include <queue>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <stack>
#include <chrono>
#include <random>

using namespace std;

using LL = long long;
using ll = long long;
using ULL = unsigned long long;
using VI = vector<int>;
using VLL = vector<LL>;
using PII = pair<int, int>;
using PLL = pair<LL, LL>;
using PIL = pair<int, LL>;
using PLI = pair<LL, int>;
using TIII = tuple<int, int, int>;
using TLLL = tuple<LL, LL, LL>;
using VVI = vector<VI>;
using VVLL = vector<VLL>;
using VPII = vector<PII>;
using VPLL = vector<PLL>;
using VVPII = vector<VPII>;

#define endl '\n'

const int inf = 0x3f3f3f3f;
const LL INF = 0x3f3f3f3f3f3f3f3f;
constexpr int MAX_INT = INT_MAX;
constexpr int MIN_INT = INT_MIN;
constexpr LL MAX_LL = LLONG_MAX;
constexpr LL MIN_LL = LLONG_MIN;
constexpr ULL MAX_ULL = ULLONG_MAX;
constexpr double MAX_DBL = DBL_MAX;
constexpr double MIN_DBL = -DBL_MAX;

// memset 0x3f: int=0x3f3f3f3f, LL=0x3f3f3f3f3f3f3f3f; -1: 整数各位为 1
// 浮点最值用 fill, 不用 memset

#if __cplusplus >= 201703L
inline int dx4[4] = {0, 0, -1, 1};
inline int dy4[4] = {1, -1, 0, 0};
inline int dx8[8] = {-1, -1, -1, 0, 1, 1, 1, 0};
inline int dy8[8] = {-1, 0, 1, 1, 1, 0, -1, -1};
#else

[[gnu::unused]] static int dx4[4] = {0, 0, -1, 1};
[[gnu::unused]] static int dy4[4] = {1, -1, 0, 0};
[[gnu::unused]] static int dx8[8] = {-1, -1, -1, 0, 1, 1, 1, 0};
[[gnu::unused]] static int dy8[8] = {-1, 0, 1, 1, 1, 0, -1, -1};
#endif

// 用 b 更新 a 的最大/最小值, 更新返回 true; 相等保留 a, 两参同型. 数值型 O(1)
template <class T>
bool cmax(T& a, const T& b)
{
    if (a < b)
    {
        a = b;
        return true;
    }
    return false;
}

template <class T>
bool cmin(T& a, const T& b)
{
    if (b < a)
    {
        a = b;
        return true;
    }
    return false;
}

// 填充各容器 [0,min(n+10,size)), size>=n>=0, 不扩容
// 时间 O(实际填充量), 额外空间 O(1); 更远的旧数据不清空
template <class V, typename... CS>
void z_fill_n(int n, V val, CS&... cs)
{
#if __cplusplus >= 201703L
    assert(n >= 0 && ((cs.size() >= (size_t)n) && ...));
    (fill(cs.begin(), cs.begin() + min((size_t)n + 10, cs.size()), val), ...);
#else
    assert(n >= 0);
    (void)initializer_list<int>{(assert(cs.size() >= (size_t)n), 0)...};
    (void)initializer_list<int>{(fill(cs.begin(), cs.begin() + min((size_t)n + 10, cs.size()), val), 0)...};
#endif
}

// 标准流首次读写前调用; 之后不混用 scanf/printf 或 rw. O(1)
inline void fast_io()
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);
}

// sqrt(x) 向下取整, x<=0 返回 0. O(1)
inline LL floor_isqrt(LL x)
{
    if (x <= 0) return 0;
    LL r = sqrt(x);
    while (r + 1 <= x / (r + 1)) r++;
    while (r > x / r) r--;
    return r;
}
// sqrt(x) 向上取整, x<=0 返回 0. O(1)
inline LL ceil_isqrt(LL x)
{
    if (x <= 0) return 0;
    LL r = floor_isqrt(x);
    return r + (r * r != x);
}

// a/b 向下取整; b!=0 且不能为 LLONG_MIN/-1. O(1)
inline LL floor_div(LL a, LL b)
{
    assert(b != 0 && !(a == LLONG_MIN && b == -1));
    LL res = a / b;
    LL rem = a % b;
    if (rem != 0 && ((a < 0) ^ (b < 0)))
    {
        res--;
    }
    return res;
}

// a/b 向上取整, 限制同 floor_div. O(1)
inline LL ceil_div(LL a, LL b)
{
    assert(b != 0 && !(a == LLONG_MIN && b == -1));
    LL res = a / b;
    LL rem = a % b;
    if (rem != 0 && ((a > 0) == (b > 0)))
    {
        res++;
    }
    return res;
}

#ifdef LOCAL
template <class T>
void debug_out(const T& x) { cerr << x; }
template <class H, class... T>
void debug_out(const H& h, const T&... t)
{
    cerr << h << ", ";
    debug_out(t...);
}
#define debug(...) cerr << #__VA_ARGS__ << " = ", debug_out(__VA_ARGS__), cerr << "\n"
#define debug_array(a, n) cerr << #a << ": "; for (int _i = 1; _i <= (n); _i++) cerr << a[_i] << " "; cerr << "\n"
#endif

/* Usage
#include "utils.h"

int main()
{
    fast_io();
    int n;
    if (!(cin >> n)) return 0;          // 输入示例: 3
    VI a(n + 11, -1), b(n + 1, -1);
    z_fill_n(n, 0, a, b);              // 下标 0 也会填充, a[n+10] 保留原值
    cout << a[0] << ' ' << b[n] << ' ' << a[n + 10] << '\n'; // 0 0 -1
    cout << (MAX_LL == LLONG_MAX) << '\n'; // 1, INF 是哨兵而非类型最大值
    int mx = 3, mn = 3;
    cmax(mx, 7);
    cmin(mn, 1);
    cout << mx << ' ' << mn << ' ' << cmax(mx, 7) << '\n'; // 7 1 0
#ifdef LOCAL
    debug(n, b.size());                // 仅 LOCAL 时定义调试宏, 写入 cerr
    debug_array(b, n);                 // 打印 b[1..n]
#endif
    cout << floor_isqrt(10) << ' ' << ceil_isqrt(10) << '\n'; // 3 4
    cout << floor_div(-7, 3) << ' ' << ceil_div(-7, 3) << '\n'; // -3 -2
    cout << ceil_div(LLONG_MAX, 2) << '\n'; // 4611686018427387904
    // endl 在本库为 '\n', 不刷新; 交互题显式 flush.
}
*/

#endif
