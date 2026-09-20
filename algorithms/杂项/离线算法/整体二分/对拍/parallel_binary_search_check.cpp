#include "../整体二分_回滚.cpp"
#include "../整体二分_指针.cpp"

using Ask = array<int, 3>;
int case_cnt = 0;
LL query_cnt = 0;

void check(const VI& a, const vector<Ask>& ask, const VI& expected)
{
    int n = (int)a.size() - 1, m = (int)ask.size() - 1;
    Dcr<int> d;
    for (int i = 1; i <= n; i++) d.add(a[i]);
    d.build();
    vector<PBSRollback::Event> rollback(n + m + 1);
    vector<PBSPointer::Event> pointer(n + m + 1);
    for (int i = 1; i <= n; i++)
    {
        rollback[i] = {0, i, d(a[i]), 0, 0, 0, 0};
        pointer[i] = {0, i, d(a[i]), 0, 0, 0, 0};
    }
    for (int i = 1; i <= m; i++)
    {
        auto [l, r, k] = ask[i];
        rollback[n + i] = {1, 0, 0, l, r, k, i};
        pointer[n + i] = {1, 0, 0, l, r, k, i};
    }
    if (case_cnt % 2)
    {
        reverse(rollback.begin() + 1, rollback.begin() + n + 1);
        reverse(pointer.begin() + 1, pointer.begin() + n + 1);
        reverse(rollback.begin() + n + 1, rollback.end());
        reverse(pointer.begin() + n + 1, pointer.end());
    }
    auto x = PBSRollback::range_kth(n, rollback, d);
    auto y = PBSPointer::range_kth(n, pointer, d);
    for (int i = 1; i <= n + m; i++)
    {
        auto verify = [&](const auto& e)
        {
            if (i <= n) assert(e.type == 0 && d[e.val] == a[e.pos]);
            else
            {
                assert(e.type == 1);
                assert(e.l == ask[e.id][0] && e.r == ask[e.id][1] && e.k == ask[e.id][2]);
            }
        };
        verify(rollback[i]);
        verify(pointer[i]);
    }
    assert(x == expected && y == expected);
    case_cnt++;
    query_cnt += m;
}

void brute(const VI& a, const vector<Ask>& ask)
{
    VI expected(ask.size(), 0);
    for (int i = 1; i < (int)ask.size(); i++)
    {
        auto [l, r, k] = ask[i];
        VI sub(a.begin() + l, a.begin() + r + 1);
        sort(sub.begin(), sub.end());
        expected[i] = sub[k - 1];
    }
    check(a, ask, expected);
}

vector<Ask> all_queries(int n)
{
    vector<Ask> ask(1);
    for (int l = 1; l <= n; l++)
        for (int r = l; r <= n; r++)
            for (int k = 1; k <= r - l + 1; k++) ask.push_back({l, r, k});
    return ask;
}

int main()
{
    mt19937 rng(42);
    Dcr<int> d;
    for (int v : {-100, -5, 0, 7, 9, 100}) d.add(v);
    d.build();
    vector<PBSRollback::Event> r{{}, {0, 3, d(7), 0, 0, 0, 0}, {0, 1, d(-5), 0, 0, 0, 0},
                                {0, 2, d(7), 0, 0, 0, 0}, {1, 0, 0, 1, 3, 3, 2}, {1, 0, 0, 1, 2, 1, 1}};
    vector<PBSPointer::Event> p;
    for (auto e : r) p.push_back({e.type, e.pos, e.val, e.l, e.r, e.k, e.id});
    assert(PBSRollback::range_kth(3, r, d) == VI({0, -5, 7}));
    assert(PBSPointer::range_kth(3, p, d) == VI({0, -5, 7}));
    assert(PBSRollback::range_kth(3, move(r), d) == VI({0, -5, 7}));
    assert(PBSPointer::range_kth(3, move(p), d) == VI({0, -5, 7}));

    check({0, 25957, 6405, 15770, 26287, 26465},
          {{}, {2, 2, 1}, {3, 4, 1}, {4, 5, 1}, {1, 2, 2}, {4, 4, 1}},
          {0, 6405, 15770, 26287, 25957, 26287});
    brute({0, INT_MIN, INT_MAX, 0, INT_MIN, INT_MAX}, all_queries(5));
    brute({0, 7, 7, 7, 7, 7}, all_queries(5));
    brute({0, 3, 1, 2}, vector<Ask>(1));
    for (int n = 1; n <= 6; n++)
    {
        int total = 1;
        for (int i = 1; i <= n; i++) total *= 3;
        auto ask = all_queries(n);
        for (int mask = 0; mask < total; mask++)
        {
            VI a(n + 1);
            int value = mask;
            for (int i = 1; i <= n; i++, value /= 3) a[i] = value % 3 - 1;
            brute(a, ask);
        }
    }
    for (int tc = 0; tc < 600; tc++)
    {
        int n = 1 + rng() % 80;
        VI a(n + 1);
        for (int i = 1; i <= n; i++)
            a[i] = tc % 2 ? (int)(rng() % 7) - 3 : (int)(rng() % 2000000001) - 1000000000;
        vector<Ask> ask(1);
        for (int i = 1; i <= 100; i++)
        {
            int l = 1 + rng() % n, r = 1 + rng() % n;
            if (l > r) swap(l, r);
            int k = 1 + rng() % (r - l + 1);
            ask.push_back({l, r, k});
        }
        brute(a, ask);
    }
    // 每种大数据分别用闭式答案或排序参照, 不把两份整体二分互拍当证明
    const int n = 200000, m = 200000;
    for (int mode = 0; mode < 5; mode++)
    {
        VI a(n + 1), expected(m + 1);
        vector<Ask> ask(m + 1);
        for (int i = 1; i <= n; i++)
        {
            if (mode == 0) a[i] = i - 100001;
            if (mode == 1) a[i] = 100001 - i;
            if (mode == 2) a[i] = -7;
            if (mode == 3) a[i] = i % 2 ? INT_MIN : INT_MAX;
            if (mode == 4) a[i] = (int)(rng() % 2000000001) - 1000000000;
        }
        VI sorted(a.begin() + 1, a.end());
        sort(sorted.begin(), sorted.end());
        for (int i = 1; i <= m; i++)
        {
            int l = 1 + rng() % n, r = 1 + rng() % n;
            if (l > r) swap(l, r);
            if (i % 5 == 0) l = 1, r = n;
            if (mode == 4)
            {
                if (i % 3 == 0) l = 1, r = n;
                else if (i % 3 == 1) r = l;
                else r = min(n, l + 39);
            }
            int k = 1 + rng() % (r - l + 1);
            if (i % 7 == 0) k = 1;
            if (i % 7 == 1) k = r - l + 1;
            ask[i] = {l, r, k};
            if (mode == 0) expected[i] = l + k - 1 - 100001;
            if (mode == 1) expected[i] = 100001 - (r - k + 1);
            if (mode == 2) expected[i] = -7;
            if (mode == 3) expected[i] = k <= (r + 1) / 2 - l / 2 ? INT_MIN : INT_MAX;
            if (mode == 4)
            {
                if (l == 1 && r == n) expected[i] = sorted[k - 1];
                else
                {
                    VI sub(a.begin() + l, a.begin() + r + 1);
                    sort(sub.begin(), sub.end());
                    expected[i] = sub[k - 1];
                }
            }
        }
        check(a, ask, expected);
        // 大-小-空询问-大调用, 验证内部 q/k/BIT/used 不跨调用残留
        brute({0, INT_MIN}, {{}, {1, 1, 1}});
        brute({0, 2, -1, 2}, vector<Ask>(1));
    }
    cout << "parallel_binary_search_check passed: " << case_cnt << " cases, "
         << query_cnt << " queries per variant; exhaustive, random, five n=m=200000 modes\n";
}
