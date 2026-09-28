// zoi: kmp
#ifndef Z_OI_KMP
#define Z_OI_KMP

#include "../../杂项/utils/utils.cpp"

// C++11 起; 输入原串, 无需补位, 长度 < INT_MAX; p[1..m], pi[i] 为前 i 个字符的最长相等真前后缀长度
// 空模式匹配每个间隙; 存储 O(m), m=1e6 时约 5 MB, 另计匹配结果
struct KMP
{
    int m = 0;
    string p;
    VI pi;

    KMP(const string& pattern = "") { build(pattern); }

    // 重建模式串及前缀函数, 覆盖旧结果
    // 时间: O(m) | 空间: O(m)
    void build(const string& pattern)
    {
        m = int(pattern.size());
        p = " " + pattern;
        pi.assign(p.size(), 0);
        for (int i = 2, j = 0; i <= m; i++)
        {
            while (j && p[i] != p[j + 1])
                j = pi[j];
            if (p[i] == p[j + 1])
                j++;
            pi[i] = j;
        }
    }

    // 返回首次匹配的 1-based 起点, 不存在返回 -1, 空模式返回 1
    // 时间: O(n) | 额外空间: O(1), n 为文本长度
    int find_first(const string& s) const
    {
        if (!m) return 1;
        int n = int(s.size());
        for (int i = 1, j = 0; i <= n; i++)
        {
            j = advance(j, s[i - 1]);
            if (j == m) return i - m + 1;
        }
        return -1;
    }

    // 返回升序的全部 1-based 起点, 包含重叠; 空模式返回 1..n+1, 无匹配返回空表
    // 时间: O(n + k) | 额外空间: O(k), k 为匹配数
    VI find_all(const string& s) const
    {
        VI pos;
        int n = int(s.size());
        if (!m)
        {
            for (int i = 0; i <= n; i++) pos.push_back(i + 1);
            return pos;
        }
        for (int i = 1, j = 0; i <= n; i++)
        {
            j = advance(j, s[i - 1]);
            if (j == m) pos.push_back(i - m + 1);
        }
        return pos;
    }

private:
    int advance(int j, char c) const
    {
        if (j == m) j = pi[j];
        while (j && p[j + 1] != c)
            j = pi[j];
        return j + (p[j + 1] == c);
    }
};
#endif

/* Usage
#include <kmp.h>
int main()
{
    string s = "abababa";
    KMP kmp("aba");
    cout << kmp.find_first(s) << '\n'; // 1
    for (int pos : kmp.find_all(s)) cout << pos << ' '; // 1 3 5
    cout << '\n';
    for (int i = 1; i <= kmp.m; i++) cout << kmp.pi[i] << ' '; // 0 0 1
    cout << '\n';
    kmp.build("ba");
    cout << kmp.find_first(s) << '\n'; // 2
}
*/
