#ifndef Z_OI_RW
#define Z_OI_RW

// 快读快写: fread/fwrite 手动缓冲 read/write 全家 + utils_int/utils_unsigned
// 自包含(不依赖 utils/128int); 库内引擎零使用, 刷题时题文件按需 include
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

#endif