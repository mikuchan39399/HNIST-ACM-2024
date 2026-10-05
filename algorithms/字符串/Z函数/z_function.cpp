// zoi: zFunction
#ifndef Z_OI_Z_FUNCTION
#define Z_OI_Z_FUNCTION
#include "../../杂项/utils/utils.cpp"
#ifdef ZOI_BOOKLET
// 传 span, 每项有效; z/e 为 1-based, 输入长度 < INT_MAX
// z[i]: 后缀与整段的 LCP, z[1]=n; 空态 z={0}
#else
// string/span 不补位, vector[0] 占位; span 需 C++20, n < INT_MAX
// z[1..n]: 各后缀与整段的 LCP 长度, z[1]=n; 空态 z={0}
#endif
struct ZFunction
{
    int n = 0;
    VI z;
#ifdef ZOI_BOOKLET
    ZFunction() : z(1, 0) {}
    ZFunction(auto s) { build(s); }
    // 重建 Z 数组; 时间/空间 O(n)
    void build(auto s)
    {
        int len = int(s.size());
#else
    // 默认空序列, 构造同 build
    ZFunction(const string& s = "") { build(s); }
    template<class Seq>
    ZFunction(const Seq& s) { build(s); }
    // 重建 Z 数组; 时间/空间 O(n)
    void build(const string& s) { build_seq(s, 0); }
    template<class T, class A>
    void build(const vector<T, A>& s) { build_seq(s, 1); }
#if __cplusplus >= 202002L
    template<class T, size_t N>
    void build(span<T, N> s) { build_seq(s, 0); }
#endif
    // e[1..|text|]: 各后缀与 pattern 的 LCP; 空模式全 0, e[0]=0
    // 无需 build; 时间/空间 O(|text|+|pattern|), 不改已有结果
    static VI extend(const string& text, const string& pattern) { return extend_seq(text, pattern, 0); }
    template<class T, class A, class B>
    static VI extend(const vector<T, A>& text, const vector<T, B>& pattern) { return extend_seq(text, pattern, 1); }
#if __cplusplus >= 202002L
    template<class T, size_t N, class U, size_t M>
    static VI extend(span<T, N> text, span<U, M> pattern) { return extend_seq(text, pattern, 0); }
#endif
private:
    template<class Seq>
    void build_seq(const Seq& s, int first)
    {
        int len = max(0, int(s.size()) - first);
#endif
        VI d(len + 1, 0);
        if (len) d[1] = len;
        for (int i = 2, l = 1, r = 0; i <= len; i++)
        {
            if (i <= r) d[i] = min(r - i + 1, d[i - l + 1]);
#ifdef ZOI_BOOKLET
            while (i + d[i] <= len && s[d[i]] == s[i + d[i] - 1])
#else
            while (i + d[i] <= len && s[first + d[i]] == s[first + i + d[i] - 1])
#endif
                d[i]++;
            if (i + d[i] - 1 > r)
            {
                l = i;
                r = i + d[i] - 1;
            }
        }
        z = move(d);
        n = len;
    }
#ifdef ZOI_BOOKLET
    // e[i]: text 第 i 项起与 pattern 的 LCP, 空模式全 0; 时间/空间 O(n+m)
    static VI extend(auto text, auto pattern)
    {
        ZFunction base;
        base.build(pattern);
        int n = int(text.size()), m = base.n;
#else
    template<class Text, class Pattern>
    static VI extend_seq(const Text& text, const Pattern& pattern, int first)
    {
        ZFunction base;
        base.build_seq(pattern, first);
        int n = max(0, int(text.size()) - first), m = base.n;
#endif
        VI e(n + 1, 0);
        for (int i = 1, l = 1, r = 0; i <= n; i++)
        {
            if (i <= r) e[i] = min(r - i + 1, base.z[i - l + 1]);
#ifdef ZOI_BOOKLET
            while (e[i] < m && e[i] <= n - i && text[i + e[i] - 1] == pattern[e[i]])
#else
            while (e[i] < m && e[i] <= n - i && text[first + i + e[i] - 1] == pattern[first + e[i]])
#endif
                e[i]++;
            if (i + e[i] - 1 > r)
            {
                l = i;
                r = i + e[i] - 1;
            }
        }
        return e;
    }
};
#endif
/* Usage
#include <zFunction.h>
#ifdef ZOI_BOOKLET
int main()
{
    string s = "abacaba";
    ZFunction zf{span(s)};
    for (int i = 1; i <= zf.n; i++) cout << zf.z[i] << ' ';
    cout << '\n';
    string a = "aaabaac", b = "aab";
    VI e = ZFunction::extend(span(a), span(b));
    for (int i = 1; i < int(e.size()); i++) cout << e[i] << ' ';
    cout << '\n';
    VI v = {0, 7, 2, 7, 2, 7};
    zf.build(span(v).subspan(1));
    cout << zf.z[3] << '\n'; // 3
}
#else
int main()
{
    ZFunction zf("abacaba");
    for (int i = 1; i <= zf.n; i++) cout << zf.z[i] << ' '; // 7 0 1 0 3 0 1
    cout << '\n';
    for (int p = 1; p <= zf.n; p++)
        if (p == zf.n || zf.z[p + 1] >= zf.n - p) cout << p << ' '; // 周期 4 6 7
    cout << '\n';
    VLL a = {0, -1, 3000000000LL, -1, 3000000000LL, -1}; // [0] 占位
    zf.build(a);
    cout << zf.z[3] << '\n'; // 3
    zf.build("");
    cout << zf.n << ' ' << zf.z.size() << '\n'; // 0 1
    VI e = ZFunction::extend("aaabaac", "aab"); // 文本在前, 模式在后
    for (int i = 1; i < int(e.size()); i++) cout << e[i] << ' '; // 2 3 1 0 2 1 0
    cout << '\n';
#if __cplusplus >= 202002L
    int raw[] = {99, 7, 2, 7, 88};
    zf.build(span(raw).subspan(1, 3)); // 每项有效, 无占位
    cout << zf.z[1] << ' ' << zf.z[3] << '\n'; // 3 1
#endif
}
#endif
*/
