// zoi: manacher
#ifndef Z_OI_MANACHER
#define Z_OI_MANACHER
#include "../../杂项/utils/utils.cpp"
#ifdef ZOI_BOOKLET
// 传 span, 每项有效; 区间 1-based, n <= (INT_MAX-3)/2
// p[1..2n+1]: 回文长度; 第 i 项/右侧空隙中心为 2i/2i+1
#else
// string/span 不补位, vector[0] 占位; 区间 1-based, span 需 C++20
// p[1..2n+1]: 回文长度; 第 i 项/右侧空隙的中心为 2i/2i+1
// n/p 只读, n <= (INT_MAX - 3) / 2
#endif
struct Manacher
{
    int n = 0;
    VI p;
#ifdef ZOI_BOOKLET
    Manacher() : p(2, 0) {}
    Manacher(auto s) { build(s); }
    // 重建半径; 时间/空间 O(n)
    void build(auto s)
    {
        int len = int(s.size()), m = 2 * len + 1;
#else
    // 默认空序列, 构造同 build
    Manacher(const string& s = "") { build(s); }
    template<class Seq>
    Manacher(const Seq& s) { build(s); }
    // 重建所有中心的回文半径; 时间/空间 O(n)
    void build(const string& s) { build_seq(s, 0); }
    template<class T, class A>
    void build(const vector<T, A>& s) { build_seq(s, 1); }
#if __cplusplus >= 202002L
    template<class T, size_t N>
    void build(span<T, N> s) { build_seq(s, 0); }
#endif
private:
    template<class Seq>
    void build_seq(const Seq& s, int first)
    {
        int len = max(0, int(s.size()) - first), m = 2 * len + 1;
#endif
        VI d(m + 1, 0);
        int pos = 0;
        LL sum = 0;
        auto equal = [&](int l, int r)
        {
#ifdef ZOI_BOOKLET
            return (l & 1) || s[l / 2 - 1] == s[r / 2 - 1];
#else
            return (l & 1) || s[l / 2 - 1 + first] == s[r / 2 - 1 + first];
#endif
        };
        for (int i = 1, c = 0, r = 0; i <= m; i++)
        {
            if (i < r)
                d[i] = min(d[c - (i - c)], r - i);
            while (i - d[i] > 1 && i + d[i] < m && equal(i - d[i] - 1, i + d[i] + 1))
                d[i]++;
            if (i + d[i] > r)
            {
                c = i;
                r = i + d[i];
            }
            if (d[i] > d[pos]) pos = i;
            sum += (d[i] + 1) / 2;
        }
        p = move(d);
        n = len;
        best = pos;
        total = sum;
    }
#ifndef ZOI_BOOKLET
public:
#endif
    // 判断 [l,r] 是否回文, 1 <= l <= r <= n; 时间/空间 O(1)
    bool is_palindrome(int l, int r) const
    {
        return p[l + r] >= r - l + 1;
    }
    // 最左最长闭区间, 空序列 {0,0}; 时间/空间 O(1)
    PII longest() const
    {
        if (!n) return {0, 0};
        int l = (best - p[best] + 1) / 2;
        return {l, l + p[best] - 1};
    }
    // 非空回文出现次数 (不去重); 时间/空间 O(1)
    LL count() const { return total; }
private:
    int best = 0;
    LL total = 0;
};
#endif
/* Usage
#include <manacher.h>
#ifdef ZOI_BOOKLET
int main()
{
    string s = "abacaba";
    Manacher man{span(s)};
    auto [l, r] = man.longest();
    cout << l << ' ' << r << '\n'; // 1 7
    cout << man.is_palindrome(2, 6) << ' ' << man.count() << '\n'; // 1 12
    VI a = {0, 7, 2, 7};
    man.build(span(a).subspan(1));
    cout << man.longest().second << '\n'; // 3
}
#else
int main()
{
    string s = "abacaba";
    Manacher man(s);
    PII seg = man.longest();
    cout << seg.first << ' ' << seg.second << '\n'; // 1 7
    cout << s.substr(seg.first - 1, seg.second - seg.first + 1) << '\n'; // abacaba
    cout << man.is_palindrome(2, 6) << ' ' << man.is_palindrome(1, 2) << '\n'; // 1 0
    cout << man.p[8] << ' ' << man.count() << '\n'; // 7 12
    man.build("abba");
    cout << man.p[5] << ' ' << man.count() << '\n'; // 4 6
    man.build("");
    seg = man.longest();
    cout << seg.first << ' ' << seg.second << ' ' << man.count() << '\n'; // 0 0 0
    VLL a = {0, -1, 3000000000LL, -1}; // [0] 占位
    Manacher seq(a);
    cout << seq.is_palindrome(1, 3) << ' ' << seq.count() << '\n'; // 1 4
#if __cplusplus >= 202002L
    int raw[] = {99, 7, 2, 7, 88};
    seq.build(span(raw).subspan(1, 3)); // 下标相对片段
    seg = seq.longest();
    cout << seg.first << ' ' << seg.second << '\n'; // 1 3
#endif
}
#endif
*/
