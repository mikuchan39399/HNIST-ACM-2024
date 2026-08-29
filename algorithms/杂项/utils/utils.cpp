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
#include "../128位整数/128int.cpp"

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

template <class T>
concept utils_int = is_integral_v<T> || is_same_v<T, __int128>
    || is_same_v<T, unsigned __int128>;

template <class T>
struct utils_unsigned
{
    using type = make_unsigned_t<T>;
};
template <>
struct utils_unsigned<__int128>
{
    using type = unsigned __int128;
};
template <>
struct utils_unsigned<unsigned __int128>
{
    using type = unsigned __int128;
};
template <class T>
using utils_unsigned_t = typename utils_unsigned<T>::type;

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

namespace utils_io
{
    constexpr int BUFSZ = 1 << 22;
    inline char ibuf[BUFSZ];
    inline int ilen = 0, ipos = 0;
    inline char obuf[BUFSZ];
    inline int olen = 0;
    inline int gc()
    {
        if (ipos == ilen)
        {
            ilen = (int)fread(ibuf, 1, BUFSZ, stdin);
            ipos = 0;
            if (ilen == 0) return -1;
        }
        return (unsigned char)ibuf[ipos++];
    }
    inline void flush_io()
    {
        fwrite(obuf, 1, olen, stdout);
        olen = 0;
    }
    inline void pc(char c)
    {
        if (olen == BUFSZ) flush_io();
        obuf[olen++] = c;
    }
    struct IOFlusher
    {
        ~IOFlusher() { flush_io(); }
    };
    inline IOFlusher io_flusher;
}

template <class T>
    requires utils_int<T>
bool read(T& x)
{
    int ch = utils_io::gc();
    while (ch != '-' && (ch < '0' || ch > '9'))
    {
        if (ch == -1) return false;
        ch = utils_io::gc();
    }
    int f = 1;
    if (ch == '-')
    {
        f = -1;
        ch = utils_io::gc();
    }
    using U = utils_unsigned_t<T>;
    U v = 0;
    while (ch >= '0' && ch <= '9')
    {
        v = v * 10 + (U)(ch - '0');
        ch = utils_io::gc();
    }
    x = f == 1 ? (T)v : (T)(0 - v);
    return true;
}

inline bool read(double& x)
{
    int ch = utils_io::gc();
    while (ch != '-' && ch != '+' && ch != '.' && (ch < '0' || ch > '9'))
    {
        if (ch == -1) return false;
        ch = utils_io::gc();
    }
    char buf[64];
    int n = 0;
    while ((ch >= '0' && ch <= '9') || ch == '-' || ch == '+'
           || ch == '.' || ch == 'e' || ch == 'E')
    {
        buf[n++] = (char)ch;
        ch = utils_io::gc();
    }
    buf[n] = 0;
    x = strtod(buf, nullptr);
    return true;
}

inline bool read(string& s)
{
    int ch = utils_io::gc();
    while (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t')
        ch = utils_io::gc();
    if (ch == -1) return false;
    s.clear();
    while (ch != -1 && ch != ' ' && ch != '\n' && ch != '\r' && ch != '\t')
    {
        s.push_back((char)ch);
        ch = utils_io::gc();
    }
    return true;
}

inline bool read(char& c)
{
    int ch = utils_io::gc();
    while (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t')
        ch = utils_io::gc();
    if (ch == -1) return false;
    c = (char)ch;
    return true;
}

template <class T>
    requires utils_int<T>
void write(T x, char end = '\n')
{
    using U = utils_unsigned_t<T>;
    U v = (U)x;
    if constexpr (T(-1) < T(0))
    {
        if (x < 0)
        {
            utils_io::pc('-');
            v = 0 - v;
        }
    }
    char tmp[48];
    int n = 0;
    do
    {
        tmp[n++] = (char)('0' + v % 10);
        v /= 10;
    } while (v);
    while (n) utils_io::pc(tmp[--n]);
    if (end) utils_io::pc(end);
}

inline void write(double x, char end = '\n')
{
    char s[64];
    snprintf(s, sizeof s, "%.6f", x);
    for (char* p = s; *p; p++) utils_io::pc(*p);
    if (end) utils_io::pc(end);
}

inline void write(const string& s, char end = '\n')
{
    for (char c : s) utils_io::pc(c);
    if (end) utils_io::pc(end);
}

inline void write(char c, char end = '\n')
{
    utils_io::pc(c);
    if (end) utils_io::pc(end);
}

inline void write(const char* s, char end = '\n')
{
    for (const char* p = s; *p; p++) utils_io::pc(*p);
    if (end) utils_io::pc(end);
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