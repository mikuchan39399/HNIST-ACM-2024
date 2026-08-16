#include <iostream>
#include <string>

using namespace std;

typedef __int128_t i128;

istream& operator>>(istream& is, i128& x)
{
    x = 0;
    string s;
    is >> s;
    int f = 1;
    int i = 0;
    if (s[i] == '-')
    {
        f = -1;
        i++;
    }
    while (i < s.length())
    {
        x = x * 10 + (s[i] - '0');
        i++;
    }
    x *= f;
    return is;
}

ostream& operator<<(ostream& os, i128 x)
{
    if (x == 0) return os << 0;
    __uint128_t ux = x;
    if (x < 0)
    {
        os << '-';
        ux = -x;
    }
    int stk[40];
    int top = 0;
    while (ux > 0)
    {
        stk[top] = ux % 10;
        top++;
        ux /= 10;
    }
    while (top > 0)
    {
        top--;
        os << stk[top];
    }
    return os;
}

int main()
{
    i128 a;
    i128 b;
    while (cin >> a >> b)
    {
        cout << a + b << '\n';
    }
    return 0;
}