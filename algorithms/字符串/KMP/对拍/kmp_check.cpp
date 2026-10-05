#include "../kmp.cpp"
#include "../kmp.cpp"

template<class Seq>
VI brute_pi(const Seq& p)
{
    int m = int(p.size());
    VI pi(m + 1);
    for (int len = 1; len <= m; len++)
        for (int k = 1; k < len; k++)
        {
            bool equal = true;
            for (int i = 0; i < k; i++)
                if (p[i] != p[len - k + i]) equal = false;
            if (equal) pi[len] = k;
        }
    return pi;
}

template<class Seq>
VI brute_matches(const Seq& s, const Seq& p)
{
    int n = int(s.size()), m = int(p.size());
    VI pos;
    for (int start = 0; start + m <= n; start++)
    {
        bool equal = true;
        for (int j = 0; j < m; j++)
            if (s[start + j] != p[j]) equal = false;
        if (equal) pos.push_back(start + 1);
    }
    return pos;
}

void check_queries(const KMP& kmp, const string& s, const VI& expected)
{
    assert(kmp.find_first(s) == (expected.empty() ? -1 : expected.front()));
    assert(kmp.find_all(s) == expected);
    assert(kmp.find_first(s) == (expected.empty() ? -1 : expected.front()));
}

void check_case(KMP& kmp, const string& s, const string& p)
{
    kmp.build(p);
    assert(kmp.m == int(p.size()));
    assert(kmp.p.substr(1) == p);
    VI expected_pi = brute_pi(p);
    assert(kmp.pi == expected_pi);
    check_queries(kmp, s, brute_matches(s, p));
    check_queries(kmp, "", brute_matches(string(), p));
    assert(kmp.pi == expected_pi && kmp.p.substr(1) == p);
}

vector<string> binary_strings(int max_len)
{
    vector<string> all;
    for (int len = 0; len <= max_len; len++)
        for (int mask = 0; mask < (1 << len); mask++)
        {
            string s(len, 'a');
            for (int i = 0; i < len; i++)
                if ((mask >> i) & 1) s[i] = 'b';
            all.push_back(s);
        }
    return all;
}

void small_checks()
{
    KMP kmp;
    check_queries(kmp, "abc", {1, 2, 3, 4});
    check_case(kmp, "aaaab", "aaab");
    check_case(kmp, "abababacababaca", "ababaca");
    check_case(kmp, "aaaaa", "aaa");
    check_case(kmp, "x", "x");
    check_case(kmp, "x", "y");
    check_case(kmp, "aba", "ababa");
    check_case(kmp, "", "");
    check_case(kmp, "a a\na a", "a a");
    check_case(kmp, string("a\0a\0a", 5), string("\0a", 2));

    auto texts = binary_strings(8), patterns = binary_strings(6);
    for (const string& p : patterns)
        for (const string& s : texts)
            check_case(kmp, s, p);

    mt19937 rng(42);
    const string alphabet = string("aAbB0 #\n\0", 9) + char(0x80) + char(0xff);
    for (int t = 0; t < 2000; t++)
    {
        int n = int(rng() % 100), m = int(rng() % 40);
        int kinds = t % 2 ? 2 : int(alphabet.size());
        string s(n, 'a'), p(m, 'a');
        for (char& c : s) c = alphabet[rng() % kinds];
        for (char& c : p) c = alphabet[rng() % kinds];
        if (m <= n && t % 3 == 0)
            s.replace(rng() % (n - m + 1), m, p);
        check_case(kmp, s, p);
    }

    string bytes;
    for (int i = 0; i < 256; i++) bytes += char(i);
    for (int i = 0; i < 256; i++)
        check_case(kmp, bytes + bytes, string(1, char(i)));

    string storage = "!aba?";
    kmp.build(storage.substr(1, 3));
    storage.assign(100, 'x');
    const string text_storage = "!ababa?";
    assert(kmp.find_first(text_storage.substr(1, 5)) == 1);
    assert((kmp.find_all(text_storage.substr(1, 5)) == VI{1, 3}));
    KMP copy = kmp;
    kmp.build("z");
    check_queries(copy, "ababa", {1, 3});
    copy.build(copy.p.substr(1));
    check_queries(copy, "ababa", {1, 3});
    string old_pattern = copy.p;
    copy.build(copy.p); // 输入引用成员时, 先完成新串构造再替换旧串
    assert(copy.p.substr(1) == old_pattern && copy.pi == brute_pi(old_pattern));
    KMP temporary(string("aba"));
    check_queries(temporary, "ababa", {1, 3});
}

VI progression(int first, int last, int step = 1)
{
    VI result;
    for (int i = first; i <= last; i += step) result.push_back(i);
    return result;
}

void large_checks()
{
    KMP kmp;
    for (int n : {1000000, 1, 0, 257, 1000000})
    {
        string s(n, 'a');
        int m = n ? max(1, n / 2) : 0;
        string p(m, 'a');
        kmp.build(p);
        for (int i = 1; i <= m; i++) assert(kmp.pi[i] == i - 1);
        check_queries(kmp, s, progression(1, n - m + 1));
        kmp.build("");
        assert(kmp.pi == VI{0});
        check_queries(kmp, s, progression(1, n + 1));
        if (!n) continue;

        kmp.build(s);
        check_queries(kmp, s, {1});
        check_queries(kmp, s.substr(1), {});
        for (int i = 1; i <= n; i++) assert(kmp.pi[i] == i - 1);

        p.back() = 'b';
        kmp.build(p);
        assert(kmp.pi[m] == 0);
        for (int i = 1; i < m; i++) assert(kmp.pi[i] == i - 1);
        check_queries(kmp, s, {});
        s.back() = 'b';
        check_queries(kmp, s, {n - m + 1});

        for (int i = 0; i < n; i++) s[i] = i % 2 ? 'b' : 'a';
        p = s.substr(0, m);
        kmp.build(p);
        for (int i = 1; i <= m; i++) assert(kmp.pi[i] == max(0, i - 2));
        check_queries(kmp, s, progression(1, n - m + 1, 2));

        if (n >= 257)
        {
            // 长串末尾失配沿整条 border 链回退
            p = string(n - 1, 'a') + 'b';
            kmp.build(p);
            for (int i = 1; i < n; i++) assert(kmp.pi[i] == i - 1);
            assert(kmp.pi[n] == 0);
            check_queries(kmp, p, {1});
        }
        check_case(kmp, "aaaab", "aaab");
        check_case(kmp, "", "");
    }
}

template<class T>
void check_sequence(KMPSeq<vector<T>>& kmp, const vector<T>& s, const vector<T>& p)
{
    vector<T> text(1), pattern(1);
    text.insert(text.end(), s.begin(), s.end());
    pattern.insert(pattern.end(), p.begin(), p.end());
    kmp.build(pattern);
    const KMPSeq<vector<T>>& view = kmp;
    VI expected = brute_matches(s, p);
    assert(view.m == int(p.size()) && view.p == pattern);
    assert(view.pi == brute_pi(p));
    assert(view.find_all(text) == expected);
    assert(view.find_first(text) == (expected.empty() ? -1 : expected.front()));
    assert(view.find_all(text) == expected);
    assert(view.find_first(vector<T>()) == (p.empty() ? 1 : -1));
    assert(view.find_all(vector<T>()) == (p.empty() ? VI{1} : VI{}));
#if __cplusplus >= 202002L
    kmp.build(span(pattern).subspan(1));
    assert(view.pi == brute_pi(p));
    assert(view.find_all(span<const T>(text).subspan(1)) == expected);
    assert(view.find_first(span(text).subspan(1)) == (expected.empty() ? -1 : expected.front()));
#endif
}

template<class T>
void integer_checks(const vector<T>& values)
{
    KMPSeq<vector<T>> kmp;
    assert((kmp.find_all(vector<T>{T(), values[0], values[1]}) == VI{1, 2, 3}));
    auto texts = binary_strings(8), patterns = binary_strings(6);
    for (const string& p : patterns)
        for (const string& s : texts)
        {
            vector<T> a, b;
            for (char c : s) a.push_back(values[c - 'a']);
            for (char c : p) b.push_back(values[c - 'a']);
            check_sequence(kmp, a, b);
        }
    mt19937 rng(42);
    for (int rep = 0; rep < 2000; rep++)
    {
        int n = rng() % 100, m = rng() % 40;
        vector<T> a(n), b(m);
        for (T& v : a) v = values[rng() % values.size()];
        for (T& v : b) v = values[rng() % values.size()];
        if (m <= n && rep % 3 == 0)
            copy(b.begin(), b.end(), a.begin() + rng() % (n - m + 1));
        check_sequence(kmp, a, b);
    }
    // 占位值不参与匹配, 模式独立持有, 复制/自身引用重建保持一致
    vector<T> pattern = {values[1], values[0], values[1], values[0]};
    vector<T> text = {values[0], values[0], values[1], values[0], values[1], values[0]};
    kmp.build(pattern);
    pattern.assign(20, values[2]);
    assert((kmp.find_all(text) == VI{1, 3}));
    KMPSeq<vector<T>> clone = kmp;
    kmp.build({});
    clone.build(clone.p);
    assert((clone.find_all(text) == VI{1, 3}));
    KMPSeq<vector<T>> temporary(vector<T>{T(), values[0]});
    assert((temporary.find_all(text) == VI{1, 3, 5}));

    for (int n : {1000000, 1, 0, 257, 1000000})
    {
        vector<T> a(n + 1, values[0]);
        int m = n ? max(1, n / 2) : 0;
        vector<T> b(m + 1, values[0]);
        kmp.build(b);
        for (int i = 1; i <= m; i++) assert(kmp.pi[i] == i - 1);
        assert(kmp.find_all(a) == progression(1, n - m + 1));
        assert(kmp.find_first(a) == 1);
        kmp.build({});
        assert(kmp.pi == VI{0});
        assert(kmp.find_all(a) == progression(1, n + 1));
        if (!n) continue;

        kmp.build(a);
        assert(kmp.find_all(a) == VI{1});
        assert(kmp.find_first(vector<T>(a.begin(), a.end() - 1)) == -1);
        b.back() = values[1];
        kmp.build(b);
        assert(kmp.pi[m] == 0 && kmp.find_all(a).empty());
        a.back() = values[1];
        assert(kmp.find_all(a) == VI{n - m + 1});
        for (int i = 1; i <= n; i++) a[i] = values[i % 2];
        b.assign(a.begin(), a.begin() + m + 1);
        kmp.build(b);
        for (int i = 1; i <= m; i++) assert(kmp.pi[i] == max(0, i - 2));
        assert(kmp.find_all(a) == progression(1, n - m + 1, 2));
        if (n >= 257)
        {
            b.assign(n + 1, values[0]); b.back() = values[1];
            kmp.build(b);
            assert(kmp.pi[n] == 0);
            for (int i = 1; i < n; i++) assert(kmp.pi[i] == i - 1);
            assert(kmp.find_all(b) == VI{1});
        }
    }
}

void sequence_checks()
{
    KMPSeq<VI> empty(VI{123}); // 只有占位, 等价于空容器, 不解引用尾后指针
    assert(empty.m == 0 && empty.pi == VI{0});
    assert(empty.find_first(VI{456}) == 1 && empty.find_all(VI{}) == VI{1});
    empty.build(VI{99, 7});
    assert(empty.find_first(VI{}) == -1 && empty.find_all(VI{456}).empty());
    empty.build(VI{99, 3, 3, 3});
    VI old_pi = empty.pi;
    empty.build(empty.pi); // 输入也可引用将被重建的前缀函数数组
    assert(empty.p == old_pi && empty.find_all(old_pi) == VI{1});
    integer_checks<int>({INT_MIN, INT_MAX, -300, 0, 300});
    integer_checks<LL>({LLONG_MIN, LLONG_MAX, -3000000000LL, 0, 3000000000LL});
    KMPSeq<VPII> pairs;
    check_sequence(pairs, VPII{{1, 2}, {3, 4}, {1, 2}}, VPII{{1, 2}});
    KMPSeq<vector<string>> words;
    check_sequence(words, vector<string>{"ab", "c", "ab"}, vector<string>{"ab"});

    // 元素值不同但转成 char 会碰撞, 不能靠窄化字符来匹配
    KMPSeq<VI> narrow(VI{0, 300});
    assert(narrow.find_first(VI{0, 44}) == -1);
    // 两段高度序列整体平移后的匹配: 暴力逐项比较平移量
    mt19937 rng(42);
    for (int rep = 0; rep < 600; rep++)
    {
        int n = 2 + rng() % 40, m = 2 + rng() % 15;
        VI a(n + 1), b(m + 1);
        for (int i = 1; i <= n; i++) a[i] = int(rng() % 21) - 10;
        for (int i = 1; i <= m; i++) b[i] = int(rng() % 21) - 10;
        if (m <= n && rep % 2 == 0)
        {
            int at = 1 + rng() % (n - m + 1);
            for (int i = 1; i <= m; i++) a[at + i - 1] = b[i] + 7;
        }
        VLL da(n), db(m);
        for (int i = 1; i < n; i++) da[i] = LL(a[i + 1]) - a[i];
        for (int i = 1; i < m; i++) db[i] = LL(b[i + 1]) - b[i];
        VI expected;
        for (int at = 1; at + m - 1 <= n; at++)
        {
            bool ok = true;
            for (int j = 1; j <= m; j++)
                if (LL(a[at + j - 1]) - b[j] != LL(a[at]) - b[1]) ok = false;
            if (ok) expected.push_back(at);
        }
        KMPSeq<VLL> kmp(db);
        assert(kmp.find_all(da) == expected);
    }
}

#if __cplusplus >= 202002L
void span_checks()
{
    int raw[] = {7, -2, 7, -2, 7};
    KMPSeq<VI> kmp{span(raw).first<3>()};
    assert((kmp.find_all(span(raw)) == VI{1, 3}));
    assert(kmp.find_first(span(raw).subspan(1)) == 2);
    raw[0] = 42;
    assert(kmp.p[1] == 7);
    kmp.build(span(kmp.p).subspan(1));
    assert((kmp.find_all(VI{0, 7, -2, 7}) == VI{1}));
    const array<int, 3> fixed = {{7, -2, 7}};
    kmp.build(span(fixed));
    assert(kmp.find_first(span(fixed)) == 1);
    kmp.build(span<const int>());
    assert((kmp.find_all(span(raw).first(2)) == VI{1, 2, 3}));
    assert(kmp.find_all(span<const int>()) == VI{1});
    assert(kmp.find_first(span<const int>()) == 1);
    kmp.build(VI{0, 1, 1, 1});
    VI old_pi = kmp.pi;
    kmp.build(span(kmp.pi).subspan(1));
    assert(kmp.p == old_pi && kmp.find_all(old_pi) == VI{1});
    kmp.build(span(kmp.p).last(0));
    assert(kmp.m == 0 && kmp.find_all(span<const int>()) == VI{1});

    char text[] = {'a', '\0', 'a', '\0', 'a'};
    KMP chars{span(text).first<3>()};
    assert((chars.find_all(span(text)) == VI{1, 3}));
    chars.build(span(chars.p).subspan(1));
    assert(chars.find_first(span(text).last<3>()) == 1);
    chars.build("aba"); // 字面量沿用 string 接口, 不把末尾 '\0' 当成模式
    assert(chars.m == 3 && chars.find_first("ababa") == 1);

    for (int n : {1000000, 1, 0, 257, 1000000})
    {
        VLL a(n, LLONG_MIN);
        KMPSeq<VLL> large{span<const LL>(a)};
        assert(large.m == n);
        for (int i = 1; i <= n; i++) assert(large.pi[i] == i - 1);
        assert(large.find_all(span(a)) == VI{1});
        large.build(span(a).first(n ? 1 : 0));
        assert(large.find_all(span(a)) == progression(1, n ? n : 1));
    }
}
#endif

int main()
{
    small_checks();
    large_checks();
    sequence_checks();
#if __cplusplus >= 202002L
    span_checks();
    cout << "span kmp: PASS (subspan, const/static extent, raw arrays, ownership, empty spans, million-length slices)\n";
#endif
    cout << "kmp: PASS (64897 exhaustive pairs, 2000 random cases, byte boundaries, million-length rebuilds)\n";
    cout << "sequence kmp: PASS (int/LL each 64897 exhaustive + 2000 random, million-length rebuilds, pair/string elements, 600 difference-array cases)\n";
}
