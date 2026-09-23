#include "../整体二分_回滚.cpp"
#include "../整体二分_指针.cpp"
#include "../../../../数据结构/树状数组/二维树状数组.cpp"
#include "../../../../数据结构/树状数组/树状数组.cpp"
#include "../../../离散化/离散化.cpp"

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
    BIT bit(n);
    auto add = [&](const auto& e, int delta) { bit.add(e.pos, e.pos, delta); };
    auto query = [&](const auto& e) { return bit.query(e.l, e.r); };
    auto x = PBSRollback::find_first(n, d.size(), rollback, add, query);
    for (int i = 1; i <= n; i++) assert(bit.query(i, i) == 0);
    auto y = PBSPointer::find_first(n, d.size(), pointer, add, query);
    for (int i = 1; i <= n; i++) assert(bit.query(i, i) == 0);
    for (int i = 1; i <= m; i++) x[i] = d[x[i]], y[i] = d[y[i]];
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

// 与默认 Event 无继承关系, 使用 pair 坐标并附带不会被内核解释的元数据
struct RectEvent
{
    int type, val, k, id;
    pair<int, int> lo, hi;
    LL stamp;
    bool operator==(const RectEvent&) const = default;
};
using RectAsk = array<int, 5>;
int rect_cases = 0;
LL rect_queries = 0;

void check_rect(BIT2D& bit, const vector<VI>& a, const vector<RectAsk>& ask, const VI& expected)
{
    int rows = (int)a.size() - 1, cols = (int)a[1].size() - 1;
    int n = rows * cols, m = (int)ask.size() - 1;
    bit.init(rows, cols);
    Dcr<int> d;
    for (int x = 1; x <= rows; x++)
        for (int y = 1; y <= cols; y++) d.add(a[x][y]);
    d.build();
    vector<RectEvent> q(n + m + 1);
    for (int x = 1, i = 0; x <= rows; x++)
        for (int y = 1; y <= cols; y++)
        {
            ++i;
            q[i] = {0, d(a[x][y]), 0, 0, {x, y}, {x, y}, i};
        }
    for (int i = 1; i <= m; i++)
    {
        auto [x1, y1, x2, y2, k] = ask[i];
        q[n + i] = {1, 0, k, i, {x1, y1}, {x2, y2}, n + i};
    }
    const auto original = q;
    reverse(q.begin() + 1, q.begin() + n + 1);
    reverse(q.begin() + n + 1, q.end());
    VI active(n + 1, 0);
    auto add = [&](const RectEvent& e, int delta)
    {
        assert(e == original[e.stamp] && e.type == 0);
        active[e.stamp] += delta;
        assert(active[e.stamp] == 0 || active[e.stamp] == 1);
        auto [x, y] = e.lo;
        bit.add(x, y, x, y, delta);
    };
    auto query = [&](const RectEvent& e)
    {
        const auto& before = original[e.stamp];
        assert(e.type == 1 && e.id == before.id && e.lo == before.lo && e.hi == before.hi);
        return bit.query(e.lo.first, e.lo.second, e.hi.first, e.hi.second);
    };
    auto verify = [&](VI ans)
    {
        for (int i = 1; i <= m; i++) ans[i] = d[ans[i]];
        assert(ans == expected);
        assert(all_of(active.begin(), active.end(), [](int v) { return v == 0; }));
        // 不能只验总和为零, 四张表逐格检查, 同一外部 BIT 直接给下一次调用
        for (int x = 0; x <= rows + 1; x++)
            for (int y = 0; y <= cols + 1; y++)
                assert(bit.t1[x][y] == 0 && bit.t2[x][y] == 0 && bit.t3[x][y] == 0 && bit.t4[x][y] == 0);
    };
    const auto saved = q;
    verify(PBSRollback::find_first(n, d.size(), q, add, query));
    assert(q == saved);
    verify(PBSPointer::find_first(n, d.size(), q, add, query));
    assert(q == saved);
    if (n <= 50)
    {
        auto copy = q;
        verify(PBSRollback::find_first(n, d.size(), move(copy), add, query));
        verify(PBSPointer::find_first(n, d.size(), move(q), add, query));
    }
    rect_cases++;
    rect_queries += m;
}

void brute_rect(BIT2D& bit, const vector<VI>& a, const vector<RectAsk>& ask)
{
    VI expected(ask.size(), 0);
    for (int i = 1; i < (int)ask.size(); i++)
    {
        auto [x1, y1, x2, y2, k] = ask[i];
        VI sub;
        for (int x = x1; x <= x2; x++)
            for (int y = y1; y <= y2; y++) sub.push_back(a[x][y]);
        sort(sub.begin(), sub.end());
        expected[i] = sub[k - 1];
    }
    check_rect(bit, a, ask, expected);
}

void custom_checks()
{
    // 自定义一维事件可直接用通用入口, 无继承/类型转换; 附加字段可被完整复制
    struct MyEvent
    {
        int type, pos, val, l, r, k, id;
        string note;
        bool operator==(const MyEvent&) const = default;
    };
    Dcr<int> d;
    for (int v : {-100, -7, 0, 12, 100}) d.add(v);
    d.build();
    vector<MyEvent> q{{}, {0, 2, d(-7), 0, 0, 0, 0, "second"}, {0, 1, d(12), 0, 0, 0, 0, "first"},
                      {1, 0, 0, 1, 2, 2, 2, "large"}, {1, 0, 0, 1, 2, 1, 1, "small"}};
    const auto saved = q;
    assert(q == saved);
    BIT one(2);
    auto add = [&](const MyEvent& e, int delta) { one.add(e.pos, e.pos, delta); };
    auto query = [&](const MyEvent& e) { return one.query(e.l, e.r); };
    VI ranked{0, d(-7), d(12)};
    assert(PBSRollback::find_first(2, d.size(), q, add, query) == ranked);
    assert(one.query(1, 1) == 0 && one.query(2, 2) == 0);
    assert(PBSPointer::find_first(2, d.size(), q, add, query) == ranked);
    assert(one.query(1, 1) == 0 && one.query(2, 2) == 0);
    assert(q == saved);
    // 花括号实参无法推导 E, 确认缺省 E=Event 真正可用; V=1 不应触碰统计
    auto unused_add = [](const auto&, int) { assert(false); };
    auto unused_query = [](const auto&) { assert(false); return 0; };
    assert(PBSRollback::find_first(1, 1, {{}, {0, 1, 1, 0, 0, 0, 0}, {1, 0, 0, 1, 1, 1, 1}}, unused_add, unused_query) == VI({0, 1}));
    assert(PBSPointer::find_first(1, 1, {{}, {0, 1, 1, 0, 0, 0, 0}, {1, 0, 0, 1, 1, 1, 1}}, unused_add, unused_query) == VI({0, 1}));


    BIT2D bit(500, 500);
    for (int mask = 0; mask < 81; mask++)
    {
        vector<VI> a(3, VI(3));
        int v = mask;
        for (int x = 1; x <= 2; x++)
            for (int y = 1; y <= 2; y++, v /= 3) a[x][y] = v % 3 - 1;
        vector<RectAsk> ask(1);
        for (int x1 = 1; x1 <= 2; x1++)
            for (int x2 = x1; x2 <= 2; x2++)
                for (int y1 = 1; y1 <= 2; y1++)
                    for (int y2 = y1; y2 <= 2; y2++)
                        for (int k = 1; k <= (x2 - x1 + 1) * (y2 - y1 + 1); k++)
                            ask.push_back({x1, y1, x2, y2, k});
        brute_rect(bit, a, ask);
    }
    mt19937 rng(42);
    for (int tc = 0; tc < 350; tc++)
    {
        int rows = 1 + rng() % 8, cols = 1 + rng() % 8;
        if (tc % 7 == 0) rows = 1;
        if (tc % 7 == 1) cols = 1;
        vector<VI> a(rows + 1, VI(cols + 1));
        for (int x = 1; x <= rows; x++)
            for (int y = 1; y <= cols; y++)
            {
                a[x][y] = (int)(rng() % 21) - 10;
                if (tc % 5 == 0) a[x][y] = (x + y) % 2 ? INT_MIN : INT_MAX;
            }
        vector<RectAsk> ask(1);
        for (int i = 0; i < 80; i++)
        {
            int x1 = 1 + rng() % rows, x2 = 1 + rng() % rows;
            int y1 = 1 + rng() % cols, y2 = 1 + rng() % cols;
            if (x1 > x2) swap(x1, x2);
            if (y1 > y2) swap(y1, y2);
            int k = 1 + rng() % ((x2 - x1 + 1) * (y2 - y1 + 1));
            ask.push_back({x1, y1, x2, y2, k});
        }
        brute_rect(bit, a, ask);
    }
    // 500x500 非方形查询, 行优先单调值域可独立推导矩形内第 k 小, 无逐查询暴力瓶颈
    const int side = 500, m = 60000;
    for (int mode = 0; mode < 2; mode++)
    {
        vector<VI> a(side + 1, VI(side + 1));
        for (int x = 1; x <= side; x++)
            for (int y = 1; y <= side; y++)
                a[x][y] = mode == 0 ? (x - 1) * side + y - 125000 : 125001 - (x - 1) * side - y;
        vector<RectAsk> ask(m + 1);
        VI expected(m + 1);
        for (int i = 1; i <= m; i++)
        {
            int x1 = 1 + rng() % side, x2 = 1 + rng() % side;
            int y1 = 1 + rng() % side, y2 = 1 + rng() % side;
            if (x1 > x2) swap(x1, x2);
            if (y1 > y2) swap(y1, y2);
            int area = (x2 - x1 + 1) * (y2 - y1 + 1);
            int k = 1 + rng() % area;
            if (i % 5 == 0) k = 1;
            if (i % 5 == 1) k = area;
            ask[i] = {x1, y1, x2, y2, k};
            int offset = mode == 0 ? k - 1 : area - k;
            int x = x1 + offset / (y2 - y1 + 1), y = y1 + offset % (y2 - y1 + 1);
            expected[i] = mode == 0 ? (x - 1) * side + y - 125000 : 125001 - (x - 1) * side - y;
        }
        check_rect(bit, a, ask, expected);
        brute_rect(bit, {{0, 0}, {0, INT_MAX}}, {{}, {1, 1, 1, 1, 1}});
        brute_rect(bit, {{0, 0, 0}, {0, -1, -1}}, vector<RectAsk>(1));
    }
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
    BIT bit(3);
    auto add = [&](const auto& e, int delta) { bit.add(e.pos, e.pos, delta); };
    auto query = [&](const auto& e) { return bit.query(e.l, e.r); };
    const VI ranked{0, d(-5), d(7)};
    auto verify = [&](const VI& ans)
    {
        assert(ans == ranked);
        for (int i = 1; i <= 3; i++) assert(bit.query(i, i) == 0);
    };
    verify(PBSRollback::find_first(3, d.size(), r, add, query));
    verify(PBSPointer::find_first(3, d.size(), p, add, query));
    verify(PBSRollback::find_first(3, d.size(), move(r), add, query));
    verify(PBSPointer::find_first(3, d.size(), move(p), add, query));

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
    custom_checks();
    cout << "custom Event/2D: " << rect_cases << " cases, " << rect_queries << " queries per variant; external BIT restored\n";
    cout << "parallel_binary_search_check passed: " << case_cnt << " cases, "
         << query_cnt << " queries per variant; exhaustive, random, five n=m=200000 modes\n";
}
