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
#include <string_view>
#include <vector>
#include <utility>
#include <tuple>
#include <type_traits>
#include <algorithm>
#include <array>
#include <bitset>
#include <bit>
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
#include <concepts>
#include <random>

using namespace std;

using LL = long long;
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

/* memset 速查:
 *   int a[N]; memset(a, 0x3f, sizeof a);   // 每元素 0x3f3f3f3f ≈ 1.06e9
 *   LL  d[N]; memset(d, 0x3f, sizeof d);   // 每元素 0x3f3f3f3f3f3f3f3f ≈ 4.6e18
 *   int p[N]; memset(p, -1, sizeof p);     // 每元素 -1
 *   ULL u[N]; memset(u, -1, sizeof u);     // 每元素 18446744073709551615
 *   double 不可 memset 造最值(重复字节非合法浮点), 用 fill/vll 赋值
 */

inline int dx4[4] = {0, 0, -1, 1};
inline int dy4[4] = {1, -1, 0, 0};
inline int dx8[8] = {-1, -1, -1, 0, 1, 1, 1, 0};
inline int dy8[8] = {-1, 0, 1, 1, 1, 0, -1, -1};

// 从下标 0 填到 min(n+10,size)-1, 含 1-based 哨兵与少量余量; 各容器 size>=n>=0
// 时间为实际填充元素数之和, O(1) 额外空间; 不扩容, 不保证清空更远的旧数据
template <class V, typename... CS>
void z_fill_n(int n, V val, CS&... cs)
{
    assert(n >= 0 && ((cs.size() >= (size_t)n) && ...));
    (fill(cs.begin(), cs.begin() + min((size_t)n + 10, cs.size()), val), ...);
}

// 在标准流第一次读写前调用; 后续不混用 scanf/printf 或 rw, O(1) 设置
inline void fast_io()
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);
}

// 返回 sqrt(x) 向下取整的整数; x <= 0 时返回 0
// 时间: O(1) | 空间: O(1)
inline LL floor_isqrt(LL x)
{
    if (x <= 0) return 0;
    LL r = sqrt(x);
    while (r + 1 <= x / (r + 1)) r++;
    while (r > x / r) r--;
    return r;
}
// 返回 sqrt(x) 向上取整的整数; x <= 0 时返回 0
// 时间: O(1) | 空间: O(1)
inline LL ceil_isqrt(LL x)
{
    if (x <= 0) return 0;
    LL r = floor_isqrt(x);
    return r + (r * r != x);
}

// 返回 a/b 向负无穷取整; b!=0 且不能是 LLONG_MIN/-1 (结果超出 LL)
// 时间: O(1) | 空间: O(1)
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

// 返回 a/b 向正无穷取整; 契约同 floor_div, 支持正负分子和分母
// 时间: O(1) | 空间: O(1)
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
#ifdef LOCAL
    debug(n, b.size());                // 仅 LOCAL 时定义调试宏, 写入 cerr
    debug_array(b, n);                 // 打印 b[1..n]
#endif
    cout << floor_isqrt(10) << ' ' << ceil_isqrt(10) << '\n'; // 3 4
    cout << floor_div(-7, 3) << ' ' << ceil_div(-7, 3) << '\n'; // -3 -2
    cout << ceil_div(LLONG_MAX, 2) << '\n'; // 4611686018427387904
    // 不用 (a+b-1)/b: 负数方向与中间溢出都会出错
    // endl 在本库为 '\n', 不主动刷新; 交互题应显式 flush
}
*/

#endif
