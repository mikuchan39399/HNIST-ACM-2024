#include <iostream>
#include <string>

using namespace std;

const int N = 1e6 + 10;
int a[N], b[N], c[N];
int la, lb, lc;
string s, q;

void add(int c[], int a[], int b[])
{
    for(int i = 0; i < lc; i++)
    {
        c[i] += a[i] + b[i];
        c[i + 1] += c[i] / 10;
        c[i] = c[i] % 10;
        if(c[lc])
        {
            lc++;
        }
    }
}

int main()
{
    cin >> s >> q;
    la = s.size();
    lb = q.size();
    lc = max(la, lb);
    for(int i = 0; i < la; i++) a[la - 1 - i] = s[i] - '0';
    for(int i = 0; i < lb; i++) b[lb - 1 - i] = q[i] - '0';//两个数逆序存储进数组
    
    add(c, a, b);
    for(int i = lc - 1; i >= 0; i--)
    {
        cout << c[i];
    }
    return 0;
}