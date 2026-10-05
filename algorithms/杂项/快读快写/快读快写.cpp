// zoi: rw
#ifndef Z_OI_RW
#define Z_OI_RW

// 缓冲读写, 不与 cin/cout、scanf/printf 混用; 输入为 ASCII 空白分隔的合法 token
// 整数须在目标类型内; char 读非空白字符, bool 用 0/1; token 前 EOF 返回 false 且不改参数
// write 默认换行, end=0 不追加; double 固定 6 位小数. 正常退出自动 flush, 不用于交互输入
// 时间 O(token 长度), 输入/输出缓存各 4MiB, 浮点/字符串另用 O(token 长度) 空间
#include <cstdio>
#include <cstdlib>
#include <string>
#include <type_traits>

using namespace std;

template <class T>
concept utils_int = is_integral_v<T> || is_same_v<T, __int128>
    || is_same_v<T, unsigned __int128>;

template <class T>
struct utils_unsigned
{
    using type = make_unsigned_t<T>;
};
template <>
struct utils_unsigned<bool>
{
    using type = unsigned;
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
    string buf;
    while ((ch >= '0' && ch <= '9') || ch == '-' || ch == '+'
           || ch == '.' || ch == 'e' || ch == 'E')
    {
        buf.push_back((char)ch);
        ch = utils_io::gc();
    }
    x = strtod(buf.c_str(), nullptr);
    return true;
}

inline bool read(string& s)
{
    int ch = utils_io::gc();
    while (ch == ' ' || (ch >= '\t' && ch <= '\r'))
        ch = utils_io::gc();
    if (ch == -1) return false;
    s.clear();
    while (ch != -1 && ch != ' ' && !(ch >= '\t' && ch <= '\r'))
    {
        s.push_back((char)ch);
        ch = utils_io::gc();
    }
    return true;
}

inline bool read(char& c)
{
    int ch = utils_io::gc();
    while (ch == ' ' || (ch >= '\t' && ch <= '\r'))
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
    char s[384];
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

#endif

/* Usage
#include "rw.h"
#include <vector>

int main()
{
    // 输入: n, 接着 n 个整数, 然后一个浮点数、一个单词和一个非空白字符
    int n;
    if (!read(n)) return 0;               // 空输入直接结束, 正常读写不用初始化缓存
    vector<long long> a(n + 1);
    long long sum = 0;
    for (int i = 1; i <= n; ++i) {
        read(a[i]);
        sum += a[i];                     // 总和由题目保证在 long long 内
    }
    write(sum);
    for (int i = 1; i <= n; ++i) write(a[i], i == n ? '\n' : ' ');

    double x; string word; char c;
    if (!read(x) || !read(word) || !read(c)) return 0;
    write(x);                            // 固定六位小数, 如 1.25e3 输出 1250.000000
    write(word, ' '); write(c);
    write("answer=", '\0'); write(sum);   // C 字符串与 string 均可直接输出

    __int128 lo = -((__int128)1 << 126) - ((__int128)1 << 126);
    unsigned __int128 hi = ~(unsigned __int128)0;
    write(lo); write(hi);                 // 覆盖有/无符号 128 位整数全部范围
    utils_io::flush_io();                 // 正常退出自动刷写
}
*/
