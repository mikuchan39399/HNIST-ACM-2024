// zoi: kmp
#ifndef Z_OI_KMP
#define Z_OI_KMP
#include "../../杂项/utils/utils.cpp"
#ifdef ZOI_BOOKLET
// 传 span, 每项有效; p/pi 与匹配起点 1-based, 模式独立持有
// pi[i]: 前 i 项最长 border; 输入长度 < INT_MAX
#else
// string/span 不补位, vector[0] 占位 (不含 bool); span 需 C++20
// p/pi 为 1-based 只读结果; pi[i] 为前 i 项最长 border 长度; 输入长度 < INT_MAX
#endif
template<class Seq>
struct KMPSeq
{
    using T = typename Seq::value_type;
    int m = 0;
    Seq p;
    VI pi;
#ifdef ZOI_BOOKLET
    KMPSeq() : p(1, T()), pi(1, 0) {}
    KMPSeq(auto s) { build(s); }
    // 复制模式并重建 pi; 时间/空间 O(m)
    void build(auto s)
    {
        int n = int(s.size());
#else
    // 默认空模式, 构造同 build
    KMPSeq(const Seq& pattern = Seq()) { build(pattern); }
    // 复制模式并重建 pi; 时间/空间 O(m)
    void build(const Seq& pattern) { build_raw(data(pattern), length(pattern)); }
    // 首次起点(1-based), 无解 -1, 空模式 1; 时间 O(n), 额外空间 O(1)
    int find_first(const Seq& s) const { return first_raw(data(s), length(s)); }
    // k 个升序起点 (含重叠, 列表不补位), 空模式 1..n+1; 时间 O(n+k), 空间 O(k)
    VI find_all(const Seq& s) const { return all_raw(data(s), length(s)); }
#if __cplusplus >= 202002L
    template<class U, size_t N>
    KMPSeq(span<U, N> pattern) { build(pattern); }
    template<class U, size_t N>
    void build(span<U, N> pattern)
    {
        span<const T> s = pattern;
        build_raw(s.data(), int(s.size()));
    }
    template<class U, size_t N>
    int find_first(span<U, N> text) const
    {
        span<const T> s = text;
        return first_raw(s.data(), int(s.size()));
    }
    template<class U, size_t N>
    VI find_all(span<U, N> text) const
    {
        span<const T> s = text;
        return all_raw(s.data(), int(s.size()));
    }
#endif
private:
    static const int first = is_same<Seq, string>::value ? 0 : 1;
    static int length(const Seq& s) { return max(0, int(s.size()) - first); }
    static const T* data(const Seq& s) { return s.empty() ? nullptr : s.data() + first; }
    void build_raw(const T* s, int n)
    {
#endif
        Seq next(1, T());
#ifdef ZOI_BOOKLET
        if (n) next.insert(next.end(), s.begin(), s.end());
#else
        if (n) next.insert(next.end(), s, s + n);
#endif
        p = move(next);
        m = n;
        pi.assign(m + 1, 0);
        for (int i = 2, j = 0; i <= m; i++)
        {
            while (j && p[i] != p[j + 1])
                j = pi[j];
            if (p[i] == p[j + 1])
                j++;
            pi[i] = j;
        }
    }
#ifdef ZOI_BOOKLET
    // 首次起点, 无解 -1, 空模式 1; 时间 O(n), 额外空间 O(1)
    int find_first(auto s) const
    {
        int n = int(s.size());
#else
    int first_raw(const T* s, int n) const
    {
#endif
        if (!m) return 1;
        for (int i = 1, j = 0; i <= n; i++)
        {
            j = advance(j, s[i - 1]);
            if (j == m) return i - m + 1;
        }
        return -1;
    }
#ifdef ZOI_BOOKLET
    // 升序起点表不补位, 含重叠; 空模式 1..n+1; 时间 O(n+k), 空间 O(k)
    VI find_all(auto s) const
    {
        int n = int(s.size());
#else
    VI all_raw(const T* s, int n) const
    {
#endif
        VI pos;
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
#ifdef ZOI_BOOKLET
private:
#endif
    int advance(int j, const T& c) const
    {
        if (j == m) j = pi[j];
        while (j && p[j + 1] != c)
            j = pi[j];
        return j + (p[j + 1] == c);
    }
};
using KMP = KMPSeq<string>;
#endif
/* Usage
#include <kmp.h>
#ifdef ZOI_BOOKLET
int main()
{
    string s = "abababa", p = "aba";
    KMP kmp{span(p)};
    cout << kmp.find_first(span(s)) << '\n'; // 1
    for (int i = 1; i <= kmp.m; i++) cout << kmp.pi[i] << ' '; // 0 0 1
    cout << '\n';
    for (int x : kmp.find_all(span(s))) cout << x << ' '; // 1 3 5
    cout << '\n';
    VI a = {0, 3, -2, 3, -2, 3}, b = {0, 3, -2, 3};
    KMPSeq<VI> seq{span(b).subspan(1)}; // 跳过占位
    for (int x : seq.find_all(span(a).subspan(1))) cout << x << ' '; // 1 3
}
#else
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
    VI a = {0, 3, -2, 3, -2, 3}, b = {0, 3, -2, 3}; // [0] 占位
    KMPSeq<VI> seq(b);
    for (int pos : seq.find_all(a)) cout << pos << ' '; // 1 3
    cout << '\n';
    seq.build(VI{0, -2});
    cout << seq.find_first(a) << '\n'; // 2
    KMPSeq<VLL> wide(VLL{0, 3000000000LL, -3000000000LL});
    cout << wide.find_first(VLL{0, 7, 3000000000LL, -3000000000LL}) << '\n'; // 2
#if __cplusplus >= 202002L
    // 结果相对片段, 从 1 开始
    seq.build(span(b).subspan(1));
    cout << seq.find_first(span(a).subspan(2)) << '\n'; // 2
    int raw[] = {3, -2, 3};
    seq.build(span(raw));
    cout << seq.find_first(a) << '\n'; // 1
#endif
}
#endif
*/
