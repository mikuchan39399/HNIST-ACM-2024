// ============ pst_check PersSegTree 回归套件 ============
// 覆盖: 静态区间第k小(static 单实例+clear 多测复用) | 区间加历史版本随机回访 |
//       find_first/find_last 二分 | 区点修改混用
// 纪律: 改动 PersSegTree 模板, 必重跑本套件; SegTree/DySegTree 聚合于 seg_check.cpp
// 跑法: g++ -std=c++20 -Wall -Wextra -O2 pst_check.cpp -o pst_check && ./pst_check
#include <iostream>
#include <vector>
#include <cassert>
#include <algorithm>
#include <random>
#include "../可持久化线段树/主席树.cpp"

using namespace std;
using LL = long long;

// 注: C++ 局部类(函数体内定义的 struct)中不能定义 friend operator+,
//     因此 Info/Tag 必须放在函数外定义, 与模板配套使用
struct TagA
{
    LL c = 0;
    void apply(const TagA& t) { c += t.c; }
};
struct InfoA
{
    LL len = 0, cnt = 0;
    void apply(const TagA& t) { cnt += t.c; }
    friend InfoA operator+(const InfoA& a, const InfoA& b)
    {
        if (a.len == 0) return b;
        if (b.len == 0) return a;
        InfoA r;
        r.len = a.len + b.len;
        r.cnt = a.cnt + b.cnt;
        return r;
    }
};

struct TagB
{
    LL add = 0;
    void apply(const TagB& t) { add += t.add; }
};
struct InfoB
{
    LL len = 0, sum = 0, mx = 0;
    void apply(const TagB& t) { sum += t.add * len; mx += t.add; }
    friend InfoB operator+(const InfoB& a, const InfoB& b)
    {
        if (a.len == 0) return b;
        if (b.len == 0) return a;
        InfoB r;
        r.len = a.len + b.len;
        r.sum = a.sum + b.sum;
        r.mx = max(a.mx, b.mx);
        return r;
    }
};

void test_pers_seg_tree()
{
    mt19937 rng(42);
    int test_cases = 300;

    for (int tc = 0; tc < test_cases; tc++)
    {
        // 模式 A: 静态区间第 k 小 (static + clear 复用池子, 覆盖多测重置路径)
        {
            static PersSegTree<InfoA, TagA> s(1, 2000);
            int sn = 1 + rng() % 60;
            vector<LL> b(sn + 1);
            for (int i = 1; i <= sn; i++) b[i] = rng() % 17;

            vector<LL> vs(b.begin() + 1, b.end());
            sort(vs.begin(), vs.end());
            vs.erase(unique(vs.begin(), vs.end()), vs.end());
            int mm = vs.size();

            auto id2 = [&](LL x)
            {
                return (int)(lower_bound(vs.begin(), vs.end(), x) - vs.begin()) + 1;
            };

            s.set_n(mm);
            s.clear();
            vector<int> rt(sn + 1, 0);

            for (int i = 1; i <= sn; i++)
            {
                int p = id2(b[i]);
                rt[i] = s.modify(rt[i - 1], p, p, {1});
            }

            for (int t = 0; t < 20; t++)
            {
                int l = 1 + rng() % sn, r = 1 + rng() % sn;
                if (l > r) swap(l, r);
                int k = 1 + rng() % (r - l + 1);

                vector<LL> c(b.begin() + l, b.begin() + r + 1);
                nth_element(c.begin(), c.begin() + k - 1, c.end());
                assert(vs[s.find_kth({rt[r]}, {rt[l - 1]}, k) - 1] == c[k - 1]);
            }
        }

        // 模式 B: 区间加, 历史版本随机回访与二分
        {
            int sn = 1 + rng() % 60;
            vector<InfoB> a(sn + 1);
            for (int i = 1; i <= sn; i++)
            {
                a[i].len = 1;
                a[i].sum = a[i].mx = rng() % 11;
            }

            PersSegTree<InfoB, TagB> s(sn, 2 * sn + 80 * 20);
            vector<int> rts;
            vector<vector<LL>> snaps;

            {
                vector<LL> base(sn + 1);
                for (int i = 1; i <= sn; i++) base[i] = a[i].sum;
                rts.push_back(s.build(a));
                snaps.push_back(base);
            }

            int ops = 80;
            for (int t = 0; t < ops; t++)
            {
                int op = rng() % 4;
                if (op == 0 || op == 1)
                {
                    int l = 1 + rng() % sn, r = 1 + rng() % sn;
                    if (l > r) swap(l, r);
                    LL v = (int)(rng() % 9) - 4;
                    int base = rng() % rts.size(); // 从任意旧版本分叉, 不只延长最新版本
                    vector<LL> cur = snaps[base];

                    if (op == 0)
                    {
                        rts.push_back(s.modify(rts[base], l, r, {v}));
                        for (int i = l; i <= r; i++) cur[i] += v;
                    }
                    else
                    {
                        rts.push_back(s.modify(rts[base], l, l, {v}));
                        cur[l] += v;
                    }
                    snaps.push_back(cur);
                }
                else
                {
                    int vi = rng() % rts.size();
                    const vector<LL>& cur = snaps[vi];
                    int l = 1 + rng() % sn, r = 1 + rng() % sn;
                    if (l > r) swap(l, r);

                    LL es = 0, em = cur[l];
                    for (int i = l; i <= r; i++)
                    {
                        es += cur[i];
                        em = max(em, cur[i]);
                    }

                    InfoB res = s.query(rts[vi], l, r);
                    assert(res.sum == es && res.mx == em);

                    LL x = (int)(rng() % 15) - 4;

                    LL got = s.find_first(rts[vi], l, [x](const InfoB& info) { return info.mx > x; });
                    LL exp = -1;
                    for (LL i = l; i <= sn; i++)
                    {
                        if (cur[i] > x) { exp = i; break; }
                    }
                    assert(got == exp);

                    got = s.find_last(rts[vi], r, [x](const InfoB& info) { return info.mx > x; });
                    exp = -1;
                    for (LL i = r; i >= 1; i--)
                    {
                        if (cur[i] > x) { exp = i; break; }
                    }
                    assert(got == exp);
                }
            }
        }
    }
    cout << "All tests passed flawlessly!\n";
}

int main()
{
    test_pers_seg_tree();
    return 0;
}
