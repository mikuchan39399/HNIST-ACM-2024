#include <iostream>
#include <string>
#include <algorithm>

using namespace std;

const int N = 1e6 + 10;
int a[N], b[N], c[N];
int la, lb, lc;
string s, q;

bool cmp(string &s, string &q)
{
    if(s.size() != q.size()) return s.size() < q.size();
    return s < q;
}

void sub(int c[], int a[], int b[])
{
    for(int i = 0; i < lc; i++)
    {
        c[i] += a[i] - b[i];
        if(c[i] < 0)
        {
            c[i + 1] -= 1;
            c[i] += 10;
        }
    }
    while(lc > 1 && c[lc - 1] == 0) lc--;
}

int main()
{
    cin >> s >> q;
    if(cmp(s, q))
    {
        swap(s, q);
        cout << '-';
    }
    la = s.size();
    lb = q.size();
    lc = max(la, lb);
    for(int i = 0; i < la; i++) a[la - 1 - i] = s[i] - '0';
    for(int i = 0; i < lb; i++) b[lb - 1 - i] = q[i] - '0';//两个数逆序存储进数组
    
    sub(c, a, b);

    for(int i = lc - 1; i >= 0; i--)
    {
        cout << c[i];
    }
    return 0;
}