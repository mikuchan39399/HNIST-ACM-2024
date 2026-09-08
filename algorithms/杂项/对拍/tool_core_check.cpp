#include <sstream>
#include "../utils/utils.cpp"
#include "../128位整数/128int.cpp"
#include "../离散化/离散化.cpp"
#include "../主元素问题/Misra-Gries.cpp"
#include "../utils/utils.cpp"
#include "../128位整数/128int.cpp"
#include "../离散化/离散化.cpp"
#include "../主元素问题/Misra-Gries.cpp"

static mt19937_64 rng(20260908);
static string decimal_ref(i128 x)
{
    bool neg = x < 0;
    u128 bits = neg ? ~u128(x) + 1 : u128(x);
    string digits = "0";
    for (int bit = 127; bit >= 0; bit--)
    {
        int carry = (bits >> bit) & 1;
        for (char& c : digits)
        {
            int v = (c - '0') * 2 + carry;
            c = (char)('0' + v % 10); carry = v / 10;
        }
        if (carry) digits += (char)('0' + carry);
    }
    reverse(digits.begin(), digits.end());
    return neg ? "-" + digits : digits;
}
static void i128_tests()
{
    const i128 low = -((i128)1 << 126) - ((i128)1 << 126);
    const i128 high = - (low + 1);
    vector<pair<string, i128>> fixed{
        {"-170141183460469231731687303715884105728", low},
        {"170141183460469231731687303715884105727", high},
        {"-9223372036854775808", LLONG_MIN}, {"18446744073709551615", (i128)ULLONG_MAX},
        {"+000000", 0}, {"-00000", 0}, {"+1234567890123456789", 1234567890123456789LL}
    };
    for (auto& [s, value] : fixed)
    {
        istringstream in(" \t\n\v\f" + s);
        i128 got = 17;
        assert(in >> got);
        assert(got == value);
        ostringstream out;
        out << got;
        assert(out.str() == decimal_ref(value));
        assert(!(in >> got) && got == value);
    }
    for (int rep = 0; rep < 10000; rep++)
    {
        u128 bits = (u128)rng() << 64 | rng();
        i128 value = bits <= (u128)high ? (i128)bits : -(i128)(~bits) - 1;
        string s = decimal_ref(value);
        istringstream in(s);
        i128 got;
        assert(in >> got);
        assert(got == value);
        ostringstream out;
        out << value;
        assert(out.str() == s);
    }
    istringstream long_input("-" + string(100000, '0') + "170141183460469231731687303715884105728");
    i128 value;
    assert(long_input >> value);
    assert(value == low);
    string text;
    for (int i = 0; i < 200000; i++) text += to_string((LL)i * 1234567 - 99999999999LL) + ' ';
    istringstream in(text);
    ostringstream out;
    for (int i = 0; i < 200000; i++)
    {
        assert(in >> value);
        assert(value == (LL)i * 1234567 - 99999999999LL);
        out << value << ' ';
    }
    assert(out.str() == text);
    assert(!(in >> value));
}
static void utils_tests()
{
    static_assert(is_same_v<LL, long long> && is_same_v<ULL, unsigned long long>);
    static_assert(is_same_v<VI, vector<int>> && is_same_v<PLL, pair<LL, LL>>);
    assert(inf == 0x3f3f3f3f && INF == 0x3f3f3f3f3f3f3f3fLL);
    assert(MAX_INT == INT_MAX && MIN_INT == INT_MIN);
    assert(MAX_LL == LLONG_MAX && MIN_LL == LLONG_MIN && MAX_ULL == ULLONG_MAX);
    assert(MAX_DBL == DBL_MAX && MIN_DBL == -DBL_MAX);
    set<PII> four, eight;
    for (int i = 0; i < 4; i++) four.insert({dx4[i], dy4[i]});
    for (int i = 0; i < 8; i++) eight.insert({dx8[i], dy8[i]});
    assert((four == set<PII>{{0,1},{0,-1},{1,0},{-1,0}}));
    assert(eight.size() == 8 && !eight.contains({0,0}));
    for (int x = -1; x <= 1; x++) for (int y = -1; y <= 1; y++)
        if (x || y) assert(eight.contains({x,y}));
    for (int n : {200000, 0, 1, 257, 200000})
    {
        VI a(n + 20, 7);
        VLL b(n + 1, 9);
        vector<bool> bits(n + 11, true);
        z_fill_n(n, 0, a, b, bits);
        for (size_t i = 0; i < a.size(); i++) assert(a[i] == ((int)i < n + 10 ? 0 : 7));
        for (LL x : b) assert(x == 0);
        for (size_t i = 0; i < bits.size(); i++) assert(bits[i] == ((int)i >= n + 10));
    }
    vector<string> strings(12, "old");
    z_fill_n(1, string("new"), strings);
    assert(strings[10] == "new" && strings[11] == "old");
    VI empty;
    z_fill_n(0, 0, empty);
    fast_io();
    assert(cin.tie() == nullptr && cout.tie() == nullptr);
}
template<class T>
static void discrete_case(const vector<T>& values)
{
    set<T> unique(values.begin(), values.end());
    map<T, int> rank;
    int id = 0;
    for (const auto& x : unique) rank[x] = ++id;
    Dcr<T> d;
    d.reserve(values.size());
    for (auto& x : values) d.add(x);
    d.build();
    assert(d.size() == (int)rank.size());
    for (auto& [x, r] : rank) assert(d(x) == r && d[r] == x);
    Dcr<T> constructor(values);
    assert(constructor.v == d.v);
    auto copied = d;
    d.clear();
    assert(d.size() == 0);
    for (auto& [x, r] : rank) assert(copied(x) == r);
    if (!values.empty())
    {
        d.add(values.back());
        assert(d(values.back()) == 1 && d.size() == 1);
    }
}
static void discrete_tests()
{
    for (int n : {200000, 0, 1, 257, 200000})
        for (int shape = 0; shape < 4; shape++)
        {
            VLL v;
            for (int i = 0; i < n; i++)
                v.push_back(shape == 0 ? LLONG_MIN + i : shape == 1 ? LLONG_MAX - i :
                            shape == 2 ? 7 : (LL)(rng() % 1001) - 500);
            discrete_case(v);
        }
    discrete_case(vector<string>{"", "aa", "a", "aa", string("a\0b",3), "中文"});
    discrete_case(vector<PII>{{1,2},{1,1},{1,2},{INT_MIN,INT_MAX}});
    Dcr<int> d(VI{20, 40});
    assert(d(40) == 2);
    d.add(10); d.add(20);
    assert(d(40) == 3 && d[1] == 10);
    d.build();
    assert(d.size() == 3);
}
static VI majority_ref(const VI& a, int k)
{
    if (k < 2) return {};
    map<int,int> counts;
    for (int x : a) counts[x]++;
    VI result;
    for (auto [x, n] : counts) if (n > (int)a.size() / k) result.push_back(x);
    return result;
}
static void majority_case(const VI& a, int k)
{
    VI old = a;
    auto got = majority_element_k(a, k);
    sort(got.begin(), got.end());
    assert(got == majority_ref(a, k) && a == old);
}
static void majority_tests()
{
    for (int n = 0, power = 1; n <= 8; n++, power *= 3)
        for (int mask = 0; mask < power; mask++)
        {
            VI a(n);
            int bits = mask;
            for (int& x : a) { x = bits % 3 - 1; bits /= 3; }
            for (int k = 0; k <= n + 2; k++) majority_case(a, k);
        }
    for (int rep = 0; rep < 1000; rep++)
    {
        VI a(rng() % 101);
        for (int& x : a) x = (int)(rng() % 21) - 10;
        majority_case(a, (int)(rng() % 110));
    }
    for (int n : {200000, 0, 1, 257, 200000})
        for (int k : {2, 3, 8, 32})
            for (int shape = 0; shape < 4; shape++)
            {
                VI a(n);
                for (int i = 0; i < n; i++)
                    a[i] = shape == 0 ? INT_MIN : shape == 1 ? i % k :
                           shape == 2 ? i : (i % 2 ? INT_MAX : INT_MIN);
                majority_case(a, k);
            }
    majority_case(VI{1,2,3,1}, INT_MAX);
    majority_case(VI{1,2}, INT_MIN);
    majority_case(VI{1,1,2,2,3,4}, 3); // 恰好阈值不算, 最后必须二次核验
}
int main()
{
    utils_tests(); i128_tests(); discrete_tests(); majority_tests();
    cout << "tool_core_check passed: utils/i128/discrete/Misra-Gries\n";
}
