#ifndef Z_OI_UTILS
#define Z_OI_UTILS

#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <climits>
#include <cfloat>
#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <tuple>
#include <type_traits>

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

template<typename... CS>
void z_fill_n(int n, int val, CS&... cs)
{
    assert(((((int)cs.size()) >= n) && ...));
    (fill(cs.begin(), cs.begin() + min((size_t)(n + 10), cs.size()), val), ...);
}

void fast_io()
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);
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

#endif
