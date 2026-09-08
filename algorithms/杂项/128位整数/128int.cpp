// zoi: i128
#ifndef Z_OI_I128
#define Z_OI_I128

#include "../utils/utils.cpp"

using i128 = __int128_t;
using u128 = __uint128_t;
// 读取范围内的合法有符号十进制整数, 可带 +/-; EOF 保留 x, d 位 O(d) 时间/临时空间
inline istream& operator>>(istream& is, i128& x)
{
    string s;
    if (!(is >> s)) return is;                  
    bool neg = (s[0] == '-');
    size_t i = (s[0] == '-') || (s[0] == '+');
    u128 v = 0;
    for (; i < s.size(); i++) v = v * 10 + (s[i] - '0');
    x = neg && v ? -i128(v - 1) - 1 : i128(v);
    return is;
}
// 输出有符号十进制, 包括 -2^127; O(位数) 时间, O(1) 额外空间
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

/* Usage
#include "i128.h"

int main()
{
    i128 x;
    if (!(cin >> x)) return 0; // 输入示例: -170141183460469231731687303715884105728
    cout << x << '\n';        // 原样输出最小 i128, 不要先对它取负
    i128 a = (i128)1000000000000LL * 1000000000000LL;
    cout << a << '\n';        // 1000000000000000000000000, 乘之前先提升类型
    // 这里只给 i128 提供流运算符; u128 为类型别名, 读写可用 rw
}
*/
