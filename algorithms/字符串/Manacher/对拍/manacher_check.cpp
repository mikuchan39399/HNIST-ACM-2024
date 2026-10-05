#include "../manacher.cpp"
#include "../manacher.cpp"

template<class Seq>
bool brute_palindrome(const Seq& s, int l, int r)
{
    while (l < r)
        if (!(s[l++] == s[r--])) return false;
    return true;
}

template<class Seq>
void verify_seq(const Manacher& man, const Seq& s, int first = 0)
{
    int n = max(0, int(s.size()) - first);
    VI expected(2 * n + 2, 0);
    PII best(0, 0);
    int best_len = 0;
    LL count = 0;
    for (int l = 0; l < n; l++)
        for (int r = l; r < n; r++)
        {
            bool palindrome = brute_palindrome(s, l + first, r + first);
            assert(man.is_palindrome(l + 1, r + 1) == palindrome);
            if (!palindrome) continue;
            count++;
            int len = r - l + 1;
            expected[l + r + 2] = max(expected[l + r + 2], len);
            if (len > best_len)
            {
                best_len = len;
                best = {l + 1, r + 1};
            }
        }
    assert(man.n == n && man.p == expected);
    assert(man.longest() == best && man.count() == count);
    assert(man.longest() == best && man.count() == count);
}

void verify(const Manacher& man, const string& s) { verify_seq(man, s); }

void small_checks()
{
    Manacher man;
    verify(man, "");
    int exhaustive = 0;
    for (int n = 0, ways = 1; n <= 8; n++, ways *= 3)
        for (int mask = 0; mask < ways; mask++)
        {
            string s(n, 'a');
            for (int i = 0, code = mask; i < n; i++, code /= 3)
                s[i] += char(code % 3);
            man.build(s);
            verify(man, s);
            exhaustive++;
        }
    assert(exhaustive == 9841);
    mt19937 rng(42);
    for (int test = 0; test < 2000; test++)
    {
        string s(rng() % 81, '\0');
        for (char& c : s) c = char(rng() % (test % 2 ? 256 : 4));
        man.build(s);
        verify(man, s);
    }
    for (const string& s : {string("a"), string("aa"), string("ab"), string("abba"),
                           string("abacaba"), string("babad"), string("bananas"),
                           string("^#$##$^"), string("a a\na a"), string("a\0a\0a", 5)})
        verify(Manacher(s), s);
    string bytes;
    for (int c = 0; c < 256; c++) bytes += char(c);
    verify(Manacher(bytes), bytes);
    bytes += string(bytes.rbegin(), bytes.rend());
    verify(Manacher(bytes), bytes);
    for (int c = 0; c < 256; c++)
    {
        string repeated(9, char(c));
        verify(Manacher(repeated), repeated);
    }
    string input = "abacaba";
    man.build(input);
    input.assign(1000, 'z');
    verify(man, "abacaba");
    Manacher copy = man;
    man.build("abba");
    verify(copy, "abacaba");
    verify(man, "abba");
    man.build(string("xxabacabayy").substr(2, 7));
    verify(man, "abacaba");
}

void large_checks()
{
    Manacher man;
    for (int n : {1000000, 1, 0, 257, 1000000})
    {
        man.build(string(n, '\0'));
        assert(man.n == n && man.p.size() == size_t(2 * n + 2));
        assert(man.count() == 1LL * n * (n + 1) / 2);
        assert(man.longest() == (n ? PII(1, n) : PII(0, 0)));
        for (int i = 1; i <= 2 * n + 1; i++)
            assert(man.p[i] == min(i - 1, 2 * n + 1 - i));
        if (n) assert(man.is_palindrome(1, n) && man.is_palindrome(n, n));
    }
    const int n = 1000000;
    string alternating(n, 'a');
    for (int i = 1; i < n; i += 2) alternating[i] = 'b';
    man.build(alternating);
    assert(man.longest() == PII(1, n - 1));
    assert(man.count() == 1LL * ((n + 1) / 2) * ((n + 2) / 2));
    for (int i = 1; i <= 2 * n + 1; i++)
        assert(man.p[i] == (i % 2 ? 0 : min(i - 1, 2 * n + 1 - i)));
    mt19937 rng(42);
    for (int test = 0; test < 100000; test++)
    {
        int l = int(rng() % n) + 1, r = int(rng() % n) + 1;
        if (l > r) swap(l, r);
        assert(man.is_palindrome(l, r) == ((r - l) % 2 == 0));
    }
    string tail(n, 'a');
    tail.back() = 'b';
    man.build(tail);
    assert(man.longest() == PII(1, n - 1));
    assert(man.count() == 1LL * (n - 1) * n / 2 + 1);
    for (int i = 1; i <= 2 * n - 1; i++)
        assert(man.p[i] == min(i - 1, 2 * n - 1 - i));
    assert(man.p[2 * n] == 1 && man.p[2 * n + 1] == 0);
    assert(!man.is_palindrome(1, n) && man.is_palindrome(1, n - 1));
    man.build("");
    verify(man, "");
}

template<class T>
void sequence_checks()
{
    Manacher man;
    int exhaustive = 0;
    for (int n = 0, ways = 1; n <= 8; n++, ways *= 3)
        for (int mask = 0; mask < ways; mask++)
        {
            vector<T> a(n + 1, T(19));
            for (int i = 1, code = mask; i <= n; i++, code /= 3)
                a[i] = T(-1 - code % 3);
            man.build(a);
            verify_seq(man, a, 1);
            exhaustive++;
        }
    assert(exhaustive == 9841);
    const T values[] = {numeric_limits<T>::min(), numeric_limits<T>::max(),
                        T(-1), T(-2), T(-3), T(0), T(44), T(300)};
    mt19937 rng(42);
    for (int test = 0; test < 2000; test++)
    {
        vector<T> a(rng() % 81 + 1, T(19));
        for (size_t i = 1; i < a.size(); i++) a[i] = values[rng() % 8];
        man.build(a);
        verify_seq(man, a, 1);
    }
    vector<T> empty;
    verify_seq(Manacher(empty), empty, 1);
    vector<T> input = {T(99), T(-1), T(300), T(-1)};
    vector<T> before = input;
    man.build(input);
    input.assign(100, T(8));
    verify_seq(man, before, 1);
    Manacher copy = man;
    man.build("abba");
    verify_seq(copy, before, 1);
    man.build(vector<T>{T(99), T(44), T(300), T(-3)});
    verify_seq(man, vector<T>{T(99), T(44), T(300), T(-3)}, 1);

    for (int n : {1000000, 1, 0, 257, 1000000})
    {
        vector<T> a(n + 1, numeric_limits<T>::min());
        man.build(a);
        assert(man.n == n && man.count() == 1LL * n * (n + 1) / 2);
        assert(man.longest() == (n ? PII(1, n) : PII(0, 0)));
        for (int i = 1; i <= 2 * n + 1; i++)
            assert(man.p[i] == min(i - 1, 2 * n + 1 - i));
        if (!n) continue;
        a.back() = numeric_limits<T>::max();
        man.build(a);
        assert(man.count() == 1LL * (n - 1) * n / 2 + 1);
        assert(man.longest() == (n == 1 ? PII(1, 1) : PII(1, n - 1)));
        for (int i = 1; i <= 2 * n - 1; i++)
            assert(man.p[i] == min(i - 1, 2 * n - 1 - i));
        assert(man.p[2 * n] == 1 && man.p[2 * n + 1] == 0);
    }
}

struct Token
{
    int value;
    explicit Token(int v) : value(v) {}
    Token(Token&&) = default;
    Token(const Token&) = delete;
    bool operator==(const Token& other) const { return value == other.value; }
};

void generic_checks()
{
    vector<PII> pairs = {{9, 9}, {1, 2}, {-1, -3}, {1, 2}};
    vector<string> words = {"pad", "red", "blue", "red"};
    vector<bool> bits = {false, true, false, true, true};
    verify_seq(Manacher(pairs), pairs, 1);
    verify_seq(Manacher(words), words, 1);
    verify_seq(Manacher(bits), bits, 1);
    vector<Token> tokens;
    for (int x : {99, 7, 2, 7}) tokens.emplace_back(x);
    verify_seq(Manacher(tokens), tokens, 1);
    Manacher man("abacaba");
    for (int test = 0; test < 4; test++)
    {
        VI before = man.p;
        man.build(man.p);
        verify_seq(man, before, 1);
    }
    man.build({});
    verify(man, "");
    mt19937 rng(42);
    for (int test = 0; test < 600; test++)
    {
        int n = int(rng() % 25) + 1;
        VLL a(n + 1), d(1, 0);
        for (int i = 1; i <= n; i++) a[i] = LL(rng() % 201) - 100;
        for (int i = 2; i <= n; i++) d.push_back(a[i] - a[i - 1]);
        man.build(d);
        verify_seq(man, d, 1);
        for (int l = 1; l <= n; l++)
            for (int r = l + 1; r <= n; r++)
            {
                bool symmetric = true;
                for (int k = 0; k <= r - l; k++)
                    if (a[l + k] + a[r - k] != a[l] + a[r]) symmetric = false;
                assert(man.is_palindrome(l, r - 1) == symmetric);
            }
    }
    cout << "sequence manacher: PASS (int/LL each 9841 exhaustive + 2000 random, million-length rebuilds, generic equality, self-input, 600 difference-array cases)\n";
}

#if __cplusplus >= 202002L
void span_checks()
{
    Manacher man;
    int raw[] = {99, 7, 2, 7, 88};
    const array<int, 3> fixed = {{-1, -3, -1}};
    span<const int, 5> whole(raw);
    verify_seq(Manacher(whole), whole);
    auto part = whole.subspan<1, 3>();
    verify_seq(Manacher(part), part);
    assert(Manacher(part).longest() == PII(1, 3));
    man.build(span(fixed));
    verify_seq(man, fixed);
    man.build(span<const int>());
    verify_seq(man, span<const int>());
    char chars[] = "aba";
    verify_seq(Manacher(span(chars)), span(chars)); // 包含末尾零字节
    assert(Manacher(span(chars)).n == 4);
    mt19937 rng(42);
    for (int test = 0; test < 600; test++)
    {
        VLL a(rng() % 50 + 2);
        for (LL& x : a) x = LL(rng() % 9) - 4;
        size_t l = rng() % (a.size() + 1), len = rng() % (a.size() - l + 1);
        auto view = span<const LL>(a).subspan(l, len);
        man.build(view);
        verify_seq(man, view);
    }
    man.build("abacaba");
    VI before = man.p;
    man.build(span(man.p));
    verify_seq(man, before);
    before = man.p;
    man.build(span<const int>(man.p).subspan(2, 5));
    verify_seq(man, span<const int>(before).subspan(2, 5));
    man.build(span(man.p).last(0));
    verify(man, "");
    {
        VI temporary = {99, 7, 2, 7};
        man.build(span(temporary).subspan(1));
    }
    verify_seq(man, VI{7, 2, 7});
    for (int n : {1000000, 1, 0, 257, 1000000})
    {
        VLL a(n + 2, LLONG_MIN);
        a.front() = a.back() = LLONG_MAX;
        man.build(span(a).subspan(1, n));
        assert(man.n == n && man.count() == 1LL * n * (n + 1) / 2);
        assert(man.longest() == (n ? PII(1, n) : PII(0, 0)));
        for (int i = 1; i <= 2 * n + 1; i++)
            assert(man.p[i] == min(i - 1, 2 * n + 1 - i));
    }
    cout << "span manacher: PASS (raw arrays, const/static extent, slices, self-input, lifetime, million-length rebuilds)\n";
}
#endif

int main()
{
    small_checks();
    large_checks();
    sequence_checks<int>();
    sequence_checks<LL>();
    generic_checks();
#if __cplusplus >= 202002L
    span_checks();
#endif
    cout << "manacher: PASS (9841 exhaustive strings, 2000 random cases, full bytes, million-length rebuilds, 100000 queries)\n";
}
