#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include "../字典树.cpp"
#include "../可持久化字典树.cpp"
using namespace std;

// 独立暴力: 扫描串表逐个比对
int brute_prefix(const vector<string>& a, const string& s)
{
    int res = 0;
    for (auto& t : a)
        if (t.size() >= s.size() && t.compare(0, s.size(), s) == 0) res++;
    return res;
}
int brute_word(const vector<string>& a, const string& s)
{
    int res = 0;
    for (auto& t : a)
        if (t == s) res++;
    return res;
}

template <int K>
void run(Trie<K>& tr, const string& cs, mt19937& rng)
{
    tr.clear();
    int n = 1 + rng() % 40;
    vector<string> a(n);
    for (int i = 0; i < n; i++)
    {
        int len = 1 + rng() % 8;
        size_t w = (rng() % 2) ? cs.size() : min<size_t>(2, cs.size()); // 半数小字符集制造大量重复串
        for (int j = 0; j < len; j++)
            a[i] += cs[rng() % w];
    }
    for (auto& s : a) tr.insert(s);
    for (int t = 0; t < 40; t++)
    {
        string q;
        int len = 1 + rng() % 8;
        for (int j = 0; j < len; j++)
            q += cs[rng() % cs.size()];
        assert(tr.count_prefix(q) == brute_prefix(a, q));
        assert(tr.count_word(q) == brute_word(a, q));
    }
    for (auto& s : a)
    {
        assert(tr.count_prefix(s) == brute_prefix(a, s));
        assert(tr.count_word(s) == brute_word(a, s));
        string pre = s.substr(0, s.size() / 2);   // 真前缀
        assert(tr.count_prefix(pre) == brute_prefix(a, pre));
    }
}

void test_trie()
{
    mt19937 rng(42);
    static Trie<26> t26(200010);
    static Trie<62> t62(400010);
    static Trie<2> t2(200010);
    for (int tc = 0; tc < 300; tc++)
    {
        run(t26, "abcz", rng);
        run(t62, "aZ09", rng);
        run(t2, "01", rng);
    }
    cout << "All tests passed flawlessly!\n";
}

// 独立暴力: 全数组扫 max(x ^ v)
void test_trie_xor()
{
    mt19937 rng(42);
    static Trie<2> t(200010);
    auto rnd_ll = [&]() -> LL
    {
        int c = rng() % 3;
        if (c == 0) return rng() % 16;            // 小值
        if (c == 1) return rng() % 1000000;       // 中值
        return ((LL)rng() << 31) + rng();         // 大值(62 位)
    };
    for (int tc = 0; tc < 300; tc++)
    {
        t.clear();
        assert(t.max_xor(0) == -1);  // 空树哨兵
        vector<LL> a;
        int n = 1 + rng() % 25;
        for (int i = 0; i < n; i++)
        {
            LL v = (!a.empty() && rng() % 3 == 0) ? a[rng() % a.size()] : rnd_ll(); // 重复制造重串
            a.push_back(v);
            t.insert_num(v);
        }
        for (int q = 0; q < 30; q++)
        {
            LL x = (q % 3 == 0) ? a[rng() % a.size()] : rnd_ll();
            LL best = 0;
            for (LL v : a) best = max(best, x ^ v);
            assert(t.max_xor(x) == best);
        }
    }
}

// 独立暴力: 快照数组全扫 + 历史版本随机回访
void test_pers_trie()
{
    mt19937 rng(42);
    static PersTrie pt(300 * 80 * 64 + 100);
    auto rnd_ll = [&]() -> LL
    {
        int c = rng() % 3;
        if (c == 0) return rng() % 16;            // 小值
        if (c == 1) return rng() % 1000000;       // 中值
        return ((LL)rng() << 31) + rng();         // 大值(62 位)
    };
    for (int tc = 0; tc < 300; tc++)
    {
        pt.clear();
        vector<int> roots(1, 0);
        vector<vector<LL>> snaps(1, vector<LL>{});
        assert(pt.max_xor(roots[0], 0) == -1);  // 空版本哨兵
        assert(pt.size(roots[0]) == 0);
        int ops = 40 + rng() % 40;
        for (int t = 0; t < ops; t++)
        {
            LL v = (!snaps.back().empty() && rng() % 3 == 0)
                ? snaps.back()[rng() % snaps.back().size()] : rnd_ll(); // 重复制造共享链
            roots.push_back(pt.insert(roots.back(), v));
            snaps.push_back(snaps.back());
            snaps.back().push_back(v);
            assert(pt.size(roots.back()) == (int)snaps.back().size());
        }
        for (int t = 0; t < 40; t++)
        {
            int vi = rng() % roots.size();       // 任意历史版本
            if (snaps[vi].empty())               // 空版本: 哨兵 -1
            {
                assert(pt.max_xor(roots[vi], rnd_ll()) == -1);
                continue;
            }
            LL x = (rng() % 3 == 0) ? snaps[vi][rng() % snaps[vi].size()] : rnd_ll();
            LL best = 0;
            for (LL v : snaps[vi]) best = max(best, x ^ v);
            assert(pt.max_xor(roots[vi], x) == best);
        }
    }
}

// 独立暴力: 区间切片全扫
void test_pers_trie_range()
{
    mt19937 rng(42);
    static PersTrie pt(300 * 60 * 64 + 100);
    auto rnd_ll = [&]() -> LL
    {
        int c = rng() % 3;
        if (c == 0) return rng() % 16;
        if (c == 1) return rng() % 1000000;
        return ((LL)rng() << 31) + rng();
    };
    for (int tc = 0; tc < 300; tc++)
    {
        pt.clear();
        int n = 1 + rng() % 40;
        vector<LL> a(n + 1);
        VI rt(n + 1, 0);
        for (int i = 1; i <= n; i++)
        {
            a[i] = (!a.empty() && i > 1 && rng() % 3 == 0) ? a[1 + rng() % (i - 1)] : rnd_ll();
            rt[i] = pt.insert(rt[i - 1], a[i]);
        }
        int k = 1 + rng() % n;
        assert(pt.max_xor({rt[k]}, {rt[k]}, 0) == -1);  // 空差集哨兵
        for (int q = 0; q < 40; q++)
        {
            int l = 1 + rng() % n, r = l + rng() % (n - l + 1);
            LL x = (rng() % 3 == 0) ? a[l + rng() % (r - l + 1)] : rnd_ll();
            LL best = 0;
            for (int i = l; i <= r; i++) best = max(best, x ^ a[i]);
            assert(pt.max_xor({rt[r]}, {rt[l - 1]}, x) == best);
        }
    }
}

// 独立暴力: 切片内全扫前缀匹配
void test_pers_trie_str()
{
    mt19937 rng(42);
    static PersTrie<26> pt(300 * 40 * 8 + 100);
    auto rnd_str = [&](int len)
    {
        string s;
        for (int i = 0; i < len; i++) s += (char)('a' + rng() % 26);
        return s;
    };
    for (int tc = 0; tc < 300; tc++)
    {
        pt.clear();
        int n = 1 + rng() % 30;
        vector<string> a(n + 1);
        VI rt(n + 1, 0);
        for (int i = 1; i <= n; i++)
        {
            if (i > 1 && rng() % 3 == 0) a[i] = a[1 + rng() % (i - 1)]; // 重复串
            else a[i] = rnd_str(1 + rng() % 6);
            rt[i] = pt.insert(rt[i - 1], a[i]);
        }
        for (int q = 0; q < 40; q++)
        {
            int l = 1 + rng() % n, r = l + rng() % (n - l + 1);
            string s = (rng() % 3 == 0)
                ? a[l + rng() % (r - l + 1)].substr(0, 1 + rng() % 3)   // 成员前缀
                : rnd_str(1 + rng() % 6);
            LL bc = 0;
            int bl = 0;
            for (int i = l; i <= r; i++)
            {
                if (a[i].size() >= s.size() && a[i].compare(0, s.size(), s) == 0) bc++;
                int L = 0;
                while (L < (int)s.size() && L < (int)a[i].size() && a[i][L] == s[L]) L++;
                bl = max(bl, L);
            }
            assert(pt.count_prefix({rt[r]}, {rt[l - 1]}, s) == bc);
            assert(pt.lcp_len({rt[r]}, {rt[l - 1]}, s) == bl);
        }
        assert(pt.lcp_len({rt[1]}, {rt[1]}, "a") == -1);   // 空差集哨兵
        string s = a[1 + rng() % n];                        // 单版本重载
        LL bc = 0;
        for (int i = 1; i <= n; i++)
            if (a[i].size() >= s.size() && a[i].compare(0, s.size(), s) == 0) bc++;
        assert(pt.count_prefix(rt[n], s) == bc);
    }
}

// 独立暴力: 全部异或值排序取第 k 大
template <class PT>
void run_kth(PT& pt, int pick, mt19937& rng)
{
    pt.clear();
    auto rnd_val = [&]() -> LL
    {
        if (pick == 0) return rng() % (1LL << 11);   // HB=10 值域
        if (pick == 1) return rng() % (1LL << 31);   // HB=30 值域
        return ((LL)rng() << 31) + rng();            // 默认 63 全域
    };
    int n = 1 + rng() % 40;
    vector<LL> a(n + 1);
    VI rt(n + 1, 0);
    for (int i = 1; i <= n; i++)
    {
        if (i > 1 && rng() % 3 == 0) a[i] = a[1 + rng() % (i - 1)]; // 重复值
        else a[i] = rnd_val();
        rt[i] = pt.insert(rt[i - 1], a[i]);
    }
    vector<LL> all;
    for (int q = 0; q < 40; q++)
    {
        int l = 1 + rng() % n, r = l + rng() % (n - l + 1);
        int u = 1 + rng() % n, d = min(n, u + (int)(rng() % 5)); // 多 x 并行(1~5 行)
        vector<LL> xs;
        for (int i = u; i <= d; i++)
            xs.push_back((rng() % 3 == 0) ? a[1 + rng() % n] : rnd_val());
        assert(pt.kth_xor(rt[r], rt[r], xs, 1) == -1);        // 空差集哨兵
        assert(pt.kth_xor(rt[r], rt[l - 1], xs, 0) == -1);    // k=0 哨兵
        vector<LL> es;
        assert(pt.kth_xor(rt[n], 0, es, 1) == -1);            // xs 空哨兵
        all.clear();
        for (int i = u; i <= d; i++)
            for (int j = l; j <= r; j++) all.push_back(xs[i - u] ^ a[j]);
        sort(all.begin(), all.end());
        int kmax = (int)all.size();
        assert(pt.kth_xor(rt[r], rt[l - 1], xs, kmax) == all[0]);      // k=max 即最小
        int k = 1 + rng() % kmax;
        assert(pt.kth_xor(rt[r], rt[l - 1], xs, k) == all[kmax - k]);
        assert(pt.kth_xor(rt[r], rt[l - 1], xs, kmax + 1) == -1);      // k 越界哨兵
    }
}

void test_pers_trie_kth()
{
    mt19937 rng(42);
    static PersTrie<2, 10> p10(300 * 40 * 12 + 100);
    static PersTrie<2, 30> p30(300 * 40 * 32 + 100);
    static PersTrie<> p63(300 * 40 * 64 + 100);
    for (int tc = 0; tc < 300; tc++)
    {
        int pick = tc % 3;                    // 三种位深各 100 组
        if (pick == 0) run_kth(p10, 0, rng);
        else if (pick == 1) run_kth(p30, 1, rng);
        else run_kth(p63, 2, rng);
    }
}

int main()
{
    test_trie();
    test_trie_xor();
    test_pers_trie();
    test_pers_trie_range();
    test_pers_trie_str();
    test_pers_trie_kth();
    return 0;
}