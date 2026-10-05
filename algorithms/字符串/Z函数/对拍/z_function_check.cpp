#include "../z_function.cpp"
#include "../z_function.cpp"

template<class Seq>
void verify(const ZFunction& zf, const Seq& s, int first = 0)
{
    int n = max(0, int(s.size()) - first);
    assert(zf.n == n && zf.z.size() == size_t(n + 1) && zf.z[0] == 0);
    for (int start = 0; start < n; start++)
    {
        int common = 0;
        for (int j = start; j < n; j++)
        {
            if (!(s[first + j] == s[first + j - start])) break;
            common++;
        }
        assert(zf.z[start + 1] == common);
    }
    for (int p = 1; p <= n; p++)
    {
        bool period = true;
        for (int i = p; i < n; i++)
            if (!(s[first + i] == s[first + i - p])) period = false;
        assert((p == n || zf.z[p + 1] >= n - p) == period);
    }
}

void string_checks()
{
    ZFunction zf;
    verify(zf, string());
    for (const string& s : {string("a"), string("aaaaa"), string("abcde"),
                           string("abacaba"), string("aabcaabxaaaz"),
                           string("aabaaab"), string("aaaabaaaaac"),
                           string(" \n#\0#\n ", 7)})
    {
        zf.build(s);
        verify(zf, s);
    }
    int total = 0;
    for (int n = 0, ways = 1; n <= 8; n++, ways *= 3)
        for (int mask = 0; mask < ways; mask++)
        {
            string s(n, 'a');
            for (int i = 0, code = mask; i < n; i++, code /= 3) s[i] += code % 3;
            zf.build(s);
            verify(zf, s);
            total++;
        }
    assert(total == 9841);
    mt19937 rng(42);
    for (int test = 0; test < 2000; test++)
    {
        string s(rng() % 81, '\0');
        for (char& c : s) c = char(rng() % (test & 1 ? 256 : 3));
        zf.build(s);
        verify(zf, s);
    }
    string bytes;
    for (int i = 0; i < 256; i++) bytes += char(i);
    bytes += bytes;
    verify(ZFunction(bytes), bytes);
    string s = "abacaba";
    zf.build(s);
    s.assign(100, 'x');
    verify(zf, string("abacaba"));
    ZFunction copy = zf;
    zf.build(string("xabay").substr(1, 3));
    verify(zf, string("aba"));
    verify(copy, string("abacaba"));
    zf.build({});
    verify(zf, string());
}

template<class T>
void sequence_checks()
{
    ZFunction zf;
    int total = 0;
    for (int n = 0, ways = 1; n <= 8; n++, ways *= 3)
        for (int mask = 0; mask < ways; mask++)
        {
            vector<T> a(n + 1, T(99));
            for (int i = 1, code = mask; i <= n; i++, code /= 3) a[i] = T(code % 3 - 1);
            zf.build(a);
            verify(zf, a, 1);
            total++;
        }
    assert(total == 9841);
    const T values[] = {numeric_limits<T>::min(), numeric_limits<T>::max(),
                        T(-1), T(0), T(44), T(300)};
    mt19937 rng(42);
    for (int test = 0; test < 2000; test++)
    {
        vector<T> a(rng() % 81 + 1, T(99));
        for (size_t i = 1; i < a.size(); i++) a[i] = values[rng() % 6];
        zf.build(a);
        verify(zf, a, 1);
    }
    vector<T> empty;
    verify(ZFunction(empty), empty, 1);
    verify(ZFunction(vector<T>(1, T(99))), vector<T>(1, T(99)), 1);
    vector<T> a = {T(99), T(44), T(300), T(44)};
    vector<T> before = a;
    zf.build(a);
    a.assign(50, T(4));
    verify(zf, before, 1);
    verify(ZFunction(vector<T>{T(8), T(-1), T(-1)}), vector<T>{T(8), T(-1), T(-1)}, 1);
}

LL equality_calls = 0;
struct Token
{
    int value;
    explicit Token(int v) : value(v) {}
    Token(Token&&) = default;
    Token(const Token&) = delete;
    bool operator==(const Token& other) const
    {
        equality_calls++;
        return value == other.value;
    }
};

void generic_checks()
{
    vector<PII> pairs = {{9, 9}, {1, 2}, {-1, -3}, {1, 2}};
    vector<string> words = {"pad", "red", "blue", "red"};
    vector<bool> bits = {false, true, false, true, true};
    verify(ZFunction(pairs), pairs, 1);
    verify(ZFunction(words), words, 1);
    verify(ZFunction(bits), bits, 1);
    vector<Token> tokens;
    for (int x : {99, 7, 2, 7}) tokens.emplace_back(x);
    verify(ZFunction(tokens), tokens, 1);
    ZFunction zf("abacaba");
    for (int test = 0; test < 5; test++)
    {
        VI before = zf.z;
        zf.build(zf.z);
        verify(zf, before, 1);
    }
    zf.build(VLL{0, 3000000000LL, -3000000000LL, 3000000000LL});
    verify(zf, VLL{0, 3000000000LL, -3000000000LL, 3000000000LL}, 1);
    zf.build("aba");
    verify(zf, string("aba"));

    mt19937 rng(42);
    for (int test = 0; test < 600; test++)
    {
        int n = int(rng() % 40) + 1, m = int(rng() % 15) + 1;
        VI text(n + 1), pattern(m + 1), joined(1, 0);
        for (int i = 1; i <= n; i++) text[i] = int(rng() % 3);
        for (int i = 1; i <= m; i++) pattern[i] = int(rng() % 3);
        joined.insert(joined.end(), pattern.begin() + 1, pattern.end());
        joined.push_back(-1);
        joined.insert(joined.end(), text.begin() + 1, text.end());
        zf.build(joined);
        for (int i = 1; i <= n; i++)
        {
            int common = 0;
            while (common < m && i + common <= n && text[i + common] == pattern[common + 1]) common++;
            assert(zf.z[m + 1 + i] == common);
        }
    }
}

template<class Text, class Pattern>
void verify_extend(const Text& text, const Pattern& pattern, int first = 0)
{
    int n = max(0, int(text.size()) - first), m = max(0, int(pattern.size()) - first);
    VI e = ZFunction::extend(text, pattern);
    assert(e.size() == size_t(n + 1) && e[0] == 0);
    for (int start = 0; start < n; start++)
    {
        int common = 0;
        for (int j = start; j < n && j - start < m; j++)
        {
            if (!(text[first + j] == pattern[first + j - start])) break;
            common++;
        }
        assert(e[start + 1] == common);
    }
}

template<class T>
void extend_sequence_checks()
{
    const T values[] = {numeric_limits<T>::min(), numeric_limits<T>::max(), T(-1), T(0), T(44), T(300)};
    mt19937 rng(20260930);
    for (int test = 0; test < 2000; test++)
    {
        vector<T> text(rng() % 81 + 1, T(99)), pattern(rng() % 81 + 1, T(-99));
        for (size_t i = 1; i < text.size(); i++) text[i] = values[rng() % 6];
        for (size_t i = 1; i < pattern.size(); i++) pattern[i] = values[rng() % 6];
        verify_extend(text, pattern, 1);
    }
    for (const vector<T>& a : {vector<T>(), vector<T>{T(99)}, vector<T>{T(99), T(44), T(300)}})
        for (const vector<T>& b : {vector<T>(), vector<T>{T(-99)}, vector<T>{T(-99), T(44), T(300)}})
            verify_extend(a, b, 1);
}

void extend_checks()
{
    assert(ZFunction::extend("aaabaac", "aab") == VI({0, 2, 3, 1, 0, 2, 1, 0}));
    int total = 0;
    for (int n = 0; n <= 8; n++)
        for (int a = 0; a < (1 << n); a++)
        {
            string text(n, 'a');
            for (int i = 0; i < n; i++) text[i] += (a >> i) & 1;
            for (int m = 0; m <= 6; m++)
                for (int b = 0; b < (1 << m); b++)
                {
                    string pattern(m, 'a');
                    for (int i = 0; i < m; i++) pattern[i] += (b >> i) & 1;
                    verify_extend(text, pattern);
                    total++;
                }
        }
    assert(total == 64897);
    mt19937 rng(20260930);
    for (int test = 0; test < 2000; test++)
    {
        string text(rng() % 81, '\0'), pattern(rng() % 81, '\0');
        for (char& c : text) c = char(rng() % (test & 1 ? 256 : 3));
        for (char& c : pattern) c = char(rng() % (test & 1 ? 256 : 3));
        verify_extend(text, pattern);
    }
    string bytes;
    for (int i = 0; i < 256; i++) bytes += char(i);
    verify_extend(bytes + bytes, bytes);
    extend_sequence_checks<int>();
    extend_sequence_checks<LL>();
    verify_extend(vector<bool>{false, true, false, true, false}, vector<bool>{true, true, false}, 1);
    verify_extend(vector<PII>{{9, 9}, {1, 2}, {-1, -3}, {1, 2}}, vector<PII>{{8, 8}, {1, 2}}, 1);
    verify_extend(vector<string>{"pad", "red", "blue", "red"}, vector<string>{"ignored", "red", "blue"}, 1);
    ZFunction zf("abacaba");
    VI before = zf.z;
    verify_extend(zf.z, zf.z, 1);
    assert(zf.z == before && zf.n == 7);
    string text = "aaabaac", pattern = "aab";
    VI e = ZFunction::extend(text, pattern);
    assert(text == "aaabaac" && pattern == "aab");
    text.clear(); pattern.clear();
    assert(e == VI({0, 2, 3, 1, 0, 2, 1, 0}));

    const int n = 1000000, m = 500001;
    vector<Token> tokens, key;
    tokens.reserve(n + 1); key.reserve(m + 1);
    for (int i = 0; i <= n; i++) tokens.emplace_back(1);
    for (int i = 0; i <= m; i++) key.emplace_back(1);
    for (int mode = 0; mode < 4; mode++)
    {
        for (int i = 1; i <= n; i++) tokens[i].value = mode == 2 ? i % 2 : 1;
        for (int i = 1; i <= m; i++)
            key[i].value = mode == 0 ? 1 : mode == 1 ? (i == m ? 2 : 1) : mode == 2 ? i % 2 : 2;
        equality_calls = 0;
        e = ZFunction::extend(tokens, key);
        assert(equality_calls <= 2LL * (n + m));
        assert(e.size() == size_t(n + 1) && e[0] == 0);
        for (int i = 1; i <= n; i++)
        {
            int expected = mode == 0 ? min(m, n - i + 1) : mode == 1 ? min(m - 1, n - i + 1)
                         : mode == 2 && i % 2 ? min(m, n - i + 1) : 0;
            assert(e[i] == expected);
        }
    }
    cout << "extend: PASS (64897 pairs, string/int/LL each 2000 random, empty/long pattern, generic elements, million lengths, linear comparison bound)\n";
}

void large_checks()
{
    ZFunction zf;
    for (int n : {1000000, 1, 0, 257, 1000000})
    {
        string s(n, 'a');
        zf.build(s);
        assert(zf.n == n && zf.z.size() == size_t(n + 1) && zf.z[0] == 0);
        for (int i = 1; i <= n; i++) assert(zf.z[i] == n - i + 1);
        VI ints(n + 1, INT_MIN);
        VLL longs(n + 1, LLONG_MIN);
        zf.build(ints);
        for (int i = 1; i <= n; i++) assert(zf.z[i] == n - i + 1);
        zf.build(longs);
        assert(zf.n == n && zf.z.size() == size_t(n + 1) && zf.z[0] == 0);
        for (int i = 1; i <= n; i++) assert(zf.z[i] == n - i + 1);
        if (!n) continue;
        s.back() = 'b';
        zf.build(s);
        assert(zf.z[1] == n);
        for (int i = 2; i <= n; i++) assert(zf.z[i] == n - i);
        for (int i = 0; i < n; i++) s[i] = char('a' + i % 2);
        zf.build(s);
        for (int i = 1; i <= n; i++) assert(zf.z[i] == (i % 2 ? n - i + 1 : 0));
        ints.back() = INT_MAX;
        zf.build(ints);
        assert(zf.z[1] == n);
        for (int i = 2; i <= n; i++) assert(zf.z[i] == n - i);
        longs.back() = LLONG_MAX;
        zf.build(longs);
        assert(zf.z[1] == n);
        for (int i = 2; i <= n; i++) assert(zf.z[i] == n - i);
    }
    const int n = 1000000;
    vector<Token> tokens;
    tokens.reserve(n + 1);
    for (int i = 0; i <= n; i++) tokens.emplace_back(1);
    for (int mode = 0; mode < 4; mode++)
    {
        for (int i = 1; i <= n; i++)
            tokens[i].value = mode == 0 ? 1 : mode == 1 ? (i == n ? 2 : 1) : mode == 2 ? i % 2 : i;
        equality_calls = 0;
        zf.build(tokens);
        assert(equality_calls <= 2LL * n && zf.z[1] == n);
        for (int i = 2; i <= n; i++)
        {
            int expected = mode == 0 ? n - i + 1 : mode == 1 ? n - i : mode == 2 && i % 2 ? n - i + 1 : 0;
            assert(zf.z[i] == expected);
        }
    }
}

#if __cplusplus >= 202002L
void span_checks()
{
    int raw[] = {99, 7, 2, 7, 88};
    span<const int, 5> whole(raw);
    verify(ZFunction(whole), whole);
    auto part = whole.subspan<1, 3>();
    verify(ZFunction(part), part);
    verify_extend(span(raw), part);
    verify_extend(part, whole);
    verify_extend(part, span<const int>());
    verify_extend(span<const int>(), part);
    ZFunction zf(part);
    assert(zf.z == VI({0, 3, 0, 1}));
    const array<int, 3> fixed = {{44, 300, 44}};
    zf.build(span(fixed));
    verify(zf, fixed);
    zf.build(span<const int>());
    verify(zf, span<const int>());
    char chars[] = "aba";
    verify(ZFunction(span(chars)), span(chars));
    assert(ZFunction(span(chars)).n == 4);
    verify_extend(span(chars), span<const char, 4>(chars));
    mt19937 rng(42);
    for (int test = 0; test < 600; test++)
    {
        VLL a(rng() % 51 + 1);
        for (LL& x : a) x = LL(rng() % 7) - 3;
        size_t l = rng() % (a.size() + 1), len = rng() % (a.size() - l + 1);
        auto part = span<const LL>(a).subspan(l, len);
        zf.build(part);
        verify(zf, part);
        VLL key(rng() % 51 + 1);
        for (LL& x : key) x = LL(rng() % 7) - 3;
        size_t start = rng() % (key.size() + 1), count = rng() % (key.size() - start + 1);
        verify_extend(part, span(key).subspan(start, count));
    }
    zf.build("abacaba");
    VI before = zf.z;
    verify_extend(span(zf.z), span<const int>(zf.z).subspan(1));
    assert(zf.z == before);
    zf.build(span(zf.z));
    verify(zf, before);
    before = zf.z;
    zf.build(span<const int>(zf.z).subspan(2, 5));
    verify(zf, span<const int>(before).subspan(2, 5));
    zf.build(span(zf.z).last(0));
    verify(zf, string());
    {
        VI temporary = {99, 7, 2, 7};
        zf.build(span(temporary).subspan(1));
    }
    verify(zf, VI{7, 2, 7});
    for (int n : {1000000, 1, 0, 257, 1000000})
    {
        VLL a(n + 2, LLONG_MIN);
        a.front() = a.back() = LLONG_MAX;
        zf.build(span(a).subspan(1, n));
        assert(zf.n == n && zf.z.size() == size_t(n + 1) && zf.z[0] == 0);
        for (int i = 1; i <= n; i++) assert(zf.z[i] == n - i + 1);
        VI e = ZFunction::extend(span(a).subspan(1, n), span<const LL>(a).subspan(1, n / 2));
        assert(e.size() == size_t(n + 1) && e[0] == 0);
        for (int i = 1; i <= n; i++) assert(e[i] == min(n / 2, n - i + 1));
    }
    cout << "span z: PASS (static/const/raw, 600 slices, self-input, lifetime, million-length rebuilds)\n";
}
#endif

int main()
{
    string_checks();
    sequence_checks<int>();
    sequence_checks<LL>();
    generic_checks();
    extend_checks();
    large_checks();
#if __cplusplus >= 202002L
    span_checks();
#endif
    cout << "z_function: PASS (string/int/LL each 9841 exhaustive + 2000 random, byte/element boundaries, self-input, 600 pattern matches, million-length rebuilds, linear comparison bound)\n";
}
