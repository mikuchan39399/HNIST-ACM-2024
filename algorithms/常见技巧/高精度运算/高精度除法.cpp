#include <iostream>
#include <string>

using namespace std;

const int N = 1e6 + 10;
int a[N], c[N];
int la, lc;
int b;
void sub(int c[], int a[], int b)
{
    long long t = 0;
    for(int i = la - 1; i >= 0; i--)
    {
        t = t * 10 + a[i];//子被除数为上一次的余数*10加上被除数的一位
        c[i] = t / b;
        t %= b;//余数
    }
    //处理前导零
    while(lc > 1 && c[lc - 1] == 0) lc--;
}
int main()
{
    string x;
    cin >> x >> b;
    la = x.size();
    lc = la;

    for(int i = 0; i < la; i++) a[la - 1 - i] = x[i] - '0';

    sub(c, a, b);

    for(int i = lc - 1; i >= 0; i--)
    {
        cout << c[i];
    }
    return 0;
}