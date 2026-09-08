#include "../防卡/哈希防卡.cpp"
#include "../防卡/哈希防卡.cpp"
#include "reference/siphash/vectors64.h"

#define siphash ref_sip24
#include "reference/siphash/siphash_ref.h"
#undef siphash
#undef cROUNDS
#undef dROUNDS
#define cROUNDS 4
#define dROUNDS 8
#define siphash ref_sip48
#include "reference/siphash/siphash_ref.h"
#undef siphash
#undef cROUNDS
#undef dROUNDS
#undef ROTL
#undef U32TO8_LE
#undef U64TO8_LE
#undef U8TO64_LE
#undef SIPROUND
#undef TRACE

static mt19937_64 rng(20260908);
static void store_ref(uint64_t x, unsigned char* p)
{
    for (int i = 0; i < 8; i++, x /= 256) p[i] = x % 256;
}
static uint64_t ref_hash(string_view s, uint64_t a, uint64_t b, bool standard = false)
{
    unsigned char key[16], out[8];
    store_ref(a, key); store_ref(b, key + 8);
    // 官方 C 实现对空输入也做指针算术, 给空串提供真实地址
    const char* bytes = s.empty() ? "" : s.data();
    int rc = standard ? ref_sip24(bytes, s.size(), key, out, 8)
                      : ref_sip48(bytes, s.size(), key, out, 8);
    assert(rc == 0);
    uint64_t x = 0;
    for (int i = 7; i >= 0; i--) x = x * 256 + out[i];
    return x;
}
static string encode(uint64_t x)
{
    unsigned char p[8];
    store_ref(x, p);
    return string((char*)p, 8);
}
static void reference_tests()
{
    constexpr uint64_t a = 0x0706050403020100ULL, b = 0x0f0e0d0c0b0a0908ULL;
    string s;
    custom_hash h(a, b);
    for (int n = 0; n < 64; n++)
    {
        assert(ref_hash(s, a, b, true) == sip_vectors[n]);
        assert(h(s) == ref_hash(s, a, b));
        s += (char)n;
    }
    assert(h(string_view{}) == ref_hash("", a, b));
    for (int rep = 0; rep < 4000; rep++)
    {
        uint64_t k0 = rng(), k1 = rng(), x = rng(), y = rng();
        custom_hash f(k0, k1);
        s.resize(rep < 300 ? rep : rng() % 4097);
        for (char& c : s) c = (char)rng();
        assert(f(s) == ref_hash(s, k0, k1));
        assert(f(x) == ref_hash(encode(x), k0, k1));
        assert(f(pair{x, y}) == ref_hash(encode(x) + encode(y), k0, k1));
        const auto copy = f;
        assert(copy(s) == f(s) && copy(x) == f(x));
    }
    for (size_t n : {0U, 1U, 7U, 8U, 9U, 15U, 16U, 17U, 255U, 256U, 257U, 1000000U})
    {
        s.assign(n, '\xff');
        assert(h(s) == ref_hash(s, a, b));
    }
    for (LL x : {LLONG_MIN, LLONG_MIN + 1, -1LL, 0LL, 1LL, LLONG_MAX})
        assert(h(x) == ref_hash(encode((uint64_t)x), a, b));
    assert(h(false) == h(0) && h(true) == h(1));
    for (int x = -128; x <= 127; x++) assert(h((signed char)x) == h((LL)x));
    for (int x = 0; x <= 255; x++) assert(h((unsigned char)x) == h((ULL)x));
    assert(h(PII{INT_MIN, INT_MAX}) == ref_hash(encode((uint64_t)(LL)INT_MIN) + encode(INT_MAX), a, b));
    custom_hash random_a, random_b;
    assert(random_a(42) == random_a(42));
    bool different = false;
    for (int i = 0; i < 16; i++) different |= random_a(i) != random_b(i);
    assert(different); // 随机源冒烟, 不把有限样本当密钥熵证明
}
static void container_tests()
{
    unordered_map<LL, LL, custom_hash> m(0, custom_hash(123, 456));
    m.max_load_factor(0.7f);
    map<LL, LL> ref;
    for (int n : {200000, 0, 1, 257, 200000})
    {
        m.clear(); ref.clear(); m.reserve(n);
        for (int i = 0; i < n; i++)
        {
            LL x = i % 3 ? (LL)(rng() & LLONG_MAX) : (LL)i * 107897;
            LL value = (LL)(rng() % 1000001) - 500000;
            m[x] += value;
            ref[x] += value;
        }
        for (auto [x, value] : ref) assert(m.at(x) == value);
        auto saved = m;
        m.rehash(m.bucket_count() * 2 + 1);
        for (auto [x, value] : ref) assert(m.at(x) == value && saved.at(x) == value);
        for (int rep = 0; rep < n; rep++)
        {
            LL x = (LL)(rng() % (n + 1)) * 107897;
            if (rep & 1) assert(m.erase(x) == ref.erase(x));
            else { m[x]++; ref[x]++; }
        }
        assert(m.size() == ref.size());
        for (auto [x, value] : ref) assert(m.at(x) == value);
        auto moved = move(saved);
        assert(!moved.empty() || !n);
    }
    unordered_map<string, int, custom_hash> strings(0, custom_hash(10, 20));
    map<string, int> sr;
    for (int i = 0; i < 20000; i++)
    {
        string s(40, 'x');
        s += encode(i);
        strings[s]++; sr[s]++;
    }
    for (auto& [s, n] : sr) assert(strings.at(s) == n);
    unordered_set<PLL, custom_hash> pairs(0, custom_hash(9, 8));
    for (int i = 0; i < 10000; i++)
    {
        pairs.insert({i, -i}); pairs.insert({-i, i});
    }
    assert(pairs.size() == 19999);
    for (int i = 0; i < 10000; i++) assert(pairs.contains({i, -i}) && pairs.contains({-i, i}));
}
static uint64_t splitmix(uint64_t x)
{
    x += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}
static void collision_tests()
{
    const int n = 5000;
    unordered_set<uint64_t> vulnerable;
    vulnerable.reserve(n);
    uint64_t step = vulnerable.bucket_count();
    for (int i = 0; i < n; i++) vulnerable.insert(step * i);
    size_t old_max = 0;
    for (size_t b = 0; b < vulnerable.bucket_count(); b++) old_max = max(old_max, vulnerable.bucket_size(b));
    // 固定种子的构造性分布回归, 不作为任意输入的最大桶长承诺
    for (uint64_t seed : {0ULL, 1ULL, 42ULL, 0xffffffffffffffffULL})
    {
        unordered_set<uint64_t, custom_hash> safe(0, custom_hash(seed, seed ^ 123456789));
        safe.reserve(n);
        for (int i = 0; i < n; i++) safe.insert(step * i);
        size_t largest = 0;
        for (size_t b = 0; b < safe.bucket_count(); b++) largest = max(largest, safe.bucket_size(b));
        assert(largest < 32);
        cout << "collision fixture: standard=" << old_max << " sip48=" << largest << '\n';
    }
    uint64_t checksum = 0;
    const custom_hash h(123, 456);
    auto start = chrono::steady_clock::now();
    for (int i = 0; i < 1000000; i++) checksum ^= splitmix((uint64_t)i + 123);
    auto middle = chrono::steady_clock::now();
    for (int i = 0; i < 1000000; i++) checksum ^= h(i);
    auto end = chrono::steady_clock::now();
    cout << "million integer hashes: splitmix_ms=" << chrono::duration<double, milli>(middle - start).count()
         << " sip48_ms=" << chrono::duration<double, milli>(end - middle).count() << " checksum=" << checksum << '\n';
}
int main()
{
    reference_tests();
    container_tests();
    collision_tests();
    cout << "hash_check passed: SipHash-4-8 reference, keys, containers, collisions\n";
}
