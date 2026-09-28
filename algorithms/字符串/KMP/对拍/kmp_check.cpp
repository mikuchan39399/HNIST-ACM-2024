#include "../kmp.cpp"
#include "../kmp.cpp"

VI brute_pi(const string& p)
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

VI brute_matches(const string& s, const string& p)
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
    assert(kmp.p == " " + p);
    VI expected_pi = brute_pi(p);
    assert(kmp.pi == expected_pi);
    check_queries(kmp, s, brute_matches(s, p));
    check_queries(kmp, "", brute_matches("", p));
    assert(kmp.pi == expected_pi && kmp.p == " " + p);
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
    assert(copy.p == " " + old_pattern && copy.pi == brute_pi(old_pattern));
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

int main()
{
    small_checks();
    large_checks();
    cout << "kmp: PASS (64897 exhaustive pairs, 2000 random cases, byte boundaries, million-length rebuilds)\n";
}
