#ifndef Z_OI_I128
#define Z_OI_I128
using i128 = __int128_t;
using u128 = __uint128_t;
inline istream& operator>>(istream& is, i128& x)
{
    string s;
    if (!(is >> s)) return is;                  
    bool neg = (s[0] == '-');
    size_t i = (s[0] == '-') || (s[0] == '+');
    x = 0;
    for (; i < s.size(); i++) x = x * 10 + (s[i] - '0');
    if (neg) x = -x;
    return is;
}
inline ostream& operator<<(ostream& os, i128 x)
{
    if (x < 0) os << '-';
    u128 v = x < 0 ? (u128)(-(x + 1)) + 1 : (u128)x; 
    char buf[45];
    int n = 0;
    do { buf[n++] = char('0' + int(v % 10)); } while (v /= 10);
    while (n) os << buf[--n];
    return os;
}
#endif