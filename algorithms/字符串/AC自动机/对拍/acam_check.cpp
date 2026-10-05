#include "../acam.cpp"
#include "../acam.cpp"

mt19937 rng(42);

VLL brute(const vector<string>& p, const string& t)
{
    VLL ans(max(size_t(1), p.size()), 0);
    for (size_t i = 1; i < p.size(); i++)
        for (size_t j = 0; j <= t.size(); j++)
        {
            if (p[i].size() > t.size() - j) continue;
            bool ok = true;
            for (size_t k = 0; k < p[i].size(); k++)
                if (p[i][k] != t[j + k]) { ok = false; break; }
            ans[i] += ok;
        }
    return ans;
}

template<int K>
void verify_states(const ACAM<K>& ac, const vector<string>& p, const string& alphabet)
{
    map<string, int> id;
    id[""] = 0;
    for (size_t i = 1; i < p.size(); i++)
    {
        int u = 0;
        string prefix;
        for (char c : p[i])
        {
            u = ac.tr[u].ch[alphabet.find(c)];
            prefix += c;
            if (id.count(prefix)) assert(id[prefix] == u);
            else id[prefix] = u;
        }
        assert(ac.pos[i] == u);
    }
    assert(id.size() == ac.tr.size());
    vector<string> label(ac.tr.size());
    vector<bool> seen(ac.tr.size());
    for (const auto& x : id)
    {
        assert(x.second >= 0 && size_t(x.second) < ac.tr.size());
        assert(!seen[x.second]);
        seen[x.second] = true;
        label[x.second] = x.first;
    }
    for (const auto& x : id)
    {
        string suffix = x.first.empty() ? "" : x.first.substr(1);
        while (!id.count(suffix)) suffix.erase(0, 1);
        assert(ac.tr[x.second].fail == id[suffix]);
        for (int c = 0; c < K; c++)
        {
            suffix = x.first + alphabet[c];
            while (!id.count(suffix)) suffix.erase(0, 1);
            assert(ac.tr[x.second].ch[c] == id[suffix]);
        }
    }
    assert(ac.q.size() == ac.tr.size() && ac.q[0] == 0);
    fill(seen.begin(), seen.end(), false);
    size_t depth = 0;
    for (int u : ac.q)
    {
        assert(!seen[u]);
        assert(label[u].size() >= depth);
        depth = label[u].size();
        if (u) assert(seen[ac.tr[u].fail]);
        seen[u] = true;
    }
}

template<int K>
void random_cases(const string& alphabet)
{
    ACAM<K> ac(400);
    auto data = ac.tr.data();
    auto capacity = ac.tr.capacity();
    auto q_capacity = ac.q.capacity();
    assert(ac.query("") == VLL{0});
    auto word = [&](int n)
    {
        string s;
        while (n--) s += alphabet[rng() % K];
        return s;
    };
    for (int rep = 0; rep < 1000; rep++)
    {
        vector<string> p(1, "ignored placeholder");
        int m = int(rng() % 25);
        for (int i = 0; i < m; i++) p.push_back(word(int(rng() % 13)));
        if (m > 1) p[m] = p[1];
        ac.build(p);
        verify_states(ac, p, alphabet);
        const auto& ro = ac;
        for (int j = 0; j < 4; j++)
        {
            string t = word(int(rng() % 65));
            auto want = brute(p, t);
            assert(ro.query(t) == want);
            assert(ro.query(t) == want);
        }
        ac.build(p);
        assert(ac.query("") == brute(p, ""));
        ac.clear();
        assert(ac.query(word(10)) == VLL{0});
        assert(ac.pos == VI{0} && ac.q == VI{0} && ac.tr.size() == 1);
        assert(ac.tr.data() == data && ac.tr.capacity() == capacity);
        assert(ac.q.capacity() == q_capacity);
    }
}

void small_cases()
{
    ACAM<> ac(100);
    for (auto p : vector<vector<string>>{{}, {""}, {"ignored", "", ""},
        {"", "a", "aa", "aaa", "aa", "b", ""},
        {"", "he", "she", "hers", "his", "s", "ers"},
        {"", "abcd", "bcd", "cd", "d", "abc", "bc", "c"},
        {"", "ab", "bab", "bc", "bca", "c", "caa"}})
    {
        ac.build(p);
        verify_states(ac, p, "abcdefghijklmnopqrstuvwxyz");
        for (string t : {"", "a", "aaaaaa", "ushershishe", "abcdabcd", "abccab", "zzzz"})
            assert(ac.query(t) == brute(p, t));
    }
    vector<string> p(1);
    for (int len = 0; len <= 4; len++)
        for (int mask = 0; mask < (1 << len); mask++)
        {
            string s;
            for (int i = 0; i < len; i++) s += char('0' + ((mask >> i) & 1));
            p.push_back(s);
        }
    ACAM<2> binary(31);
    binary.build(p);
    verify_states(binary, p, "01");
    for (int len = 0; len <= 8; len++)
        for (int mask = 0; mask < (1 << len); mask++)
        {
            string t;
            for (int i = 0; i < len; i++) t += char('0' + ((mask >> i) & 1));
            assert(binary.query(t) == brute(p, t));
        }
    p[1] = "111";
    assert(binary.query("")[1] == 1); // 输入独立持有
    auto copy = binary;
    copy.build({"", "1"});
    assert(binary.query("0")[1] == 2 && copy.query("0")[1] == 0);
    ACAM<> roomy(100);
    roomy.build({"", "a"});
    auto copied = roomy;
    copied.build({"", string(99, 'a')});
    assert(copied.tr.size() == 100 && copied.tr.capacity() >= 100);
    assert(copied.q.capacity() >= 100 && copied.query(string(100, 'a'))[1] == 2);
    assert(roomy.query("aa")[1] == 2);
    ACAM<> assigned(1);
    assigned = roomy;
    assigned.build({"", string(99, 'b')});
    assert(assigned.tr.size() == 100 && assigned.tr.capacity() >= 100);
    assert(assigned.q.capacity() >= 100 && assigned.query(string(100, 'b'))[1] == 2);
    ACAM<> exact(4);
    exact.build({"", "abc", "abc", ""});
    assert(exact.tr.size() == 4 && exact.pos[1] == exact.pos[2]);
    ACAM<> root_only(1);
    root_only.build({"", "", ""});
    assert((root_only.query("abc") == VLL{0, 4, 4}));
}

void stress()
{
    const int n = 1000000;
    ACAM<> ac(n + 1);
    auto data = ac.tr.data();
    auto capacity = ac.tr.capacity();
    auto q_capacity = ac.q.capacity();
    for (int len : {n, 1, 0, 257, n})
    {
        vector<string> p = {"", "", string(len, 'a'), string(len, 'a')};
        auto begin = chrono::steady_clock::now();
        ac.build(p);
        auto built = chrono::steady_clock::now();
        string t(n, 'a');
        auto ans = ac.query(t);
        auto done = chrono::steady_clock::now();
        assert((ans == VLL{0, n + 1, n - len + 1, n - len + 1}));
        assert(ac.query("b")[2] == (len ? 0 : 2));
        assert(ac.tr.data() == data && ac.tr.capacity() == capacity);
        assert(ac.q.capacity() == q_capacity && ac.tr.size() == size_t(len + 1));
        cout << "chain=" << len << " build_ms="
             << chrono::duration<double, milli>(built - begin).count() << " query_ms="
             << chrono::duration<double, milli>(done - built).count()
             << " pool_bytes=" << ac.tr.capacity() * sizeof(ACAM<>::Node) + ac.q.capacity() * sizeof(int)
             << " query_workspace_bytes=" << ac.tr.size() * sizeof(LL) + ac.pos.size() * sizeof(LL) << '\n';
    }
    vector<string> p(50001, "a");
    ac.build(p);
    auto ans = ac.query(string(100000, 'a'));
    assert(accumulate(ans.begin(), ans.end(), 0LL) == 5000000000LL);
    ac.build({});
    assert(ac.query(string(n, 'z')) == VLL{0});
    p.assign(1, "");
    const int words = 26 * 26 * 26 * 26;
    for (int v = 0; v < words; v++)
    {
        string s(4, 'a');
        int x = v;
        for (int j = 3; j >= 0; j--) { s[j] += x % 26; x /= 26; }
        p.push_back(s);
    }
    auto begin = chrono::steady_clock::now();
    ac.build(p);
    auto built = chrono::steady_clock::now();
    string text(n, 'a');
    for (int i = 0; i < n; i++) text[i] += i % 26;
    ans = ac.query(text);
    auto done = chrono::steady_clock::now();
    for (int i = 1; i <= words; i++)
    {
        int start = p[i][0] - 'a';
        bool cyclic = true;
        for (int j = 1; j < 4; j++) cyclic &= p[i][j] - 'a' == (start + j) % 26;
        LL want = cyclic ? (n - 4 - start) / 26 + 1 : 0;
        assert(ans[i] == want);
    }
    assert(ac.tr.size() == size_t(1 + 26 + 676 + 17576 + words));
    assert(accumulate(ans.begin(), ans.end(), 0LL) == n - 3);
    assert(ac.tr.data() == data && ac.tr.capacity() == capacity);
    cout << "broad_patterns=" << words << " states=" << ac.tr.size() << " build_ms="
         << chrono::duration<double, milli>(built - begin).count() << " query_ms="
         << chrono::duration<double, milli>(done - built).count() << '\n';
}

int main()
{
    small_cases();
    random_cases<1>("0");
    random_cases<2>("01");
    random_cases<10>("0123456789");
    random_cases<26>("abcdefghijklmnopqrstuvwxyz");
    random_cases<62>("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
    stress();
    cout << "ACAM PASS: 5000 random dictionaries, 511 exhaustive texts, boundaries and default stress\n";
}
