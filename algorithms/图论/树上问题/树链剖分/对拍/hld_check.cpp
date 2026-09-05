// ============ hld_check HLD+泛型线段树 回归套件 ============
// 覆盖: HLD 剖链(单树 build(g,root) + 森林 build(g) 全扫含孤立点) ×
//       SegTree<Info,Tag> 路径加/查 + 子树加/查, 对拍朴素 parent-walk;
//       static 实例跨组 init 复用(多测路径); 跨分量路径不属契约, 同根才测
// 纪律: 改动 HLD / 泛型线段树 模板, 必重跑本套件
// 跑法: g++ -std=c++20 -Wall -Wextra -O2 hld_check.cpp -o hld_check && ./hld_check
#include <iostream>
#include <vector>
#include <cassert>
#include <random>
#include <algorithm>
#include "../HLD.cpp"

using namespace std;
using LL = long long;

// 注: C++ 局部类不能定义 friend operator+, Info/Tag 须放函数外
struct TagH
{
    LL add = 0;
    void apply(const TagH& t) { add += t.add; }
    void clear() { add = 0; }
    bool has_tag() const { return add != 0; }
};
struct InfoH
{
    LL len = 0, sum = 0;
    bool break_cond(const TagH&) const { return false; }
    bool tag_cond(const TagH&) const { return true; }
    void apply(const TagH& t) { sum += t.add * len; }
    friend InfoH operator+(const InfoH& a, const InfoH& b)
    {
        if (a.len == 0) return b;
        if (b.len == 0) return a;
        InfoH r;
        r.len = a.len + b.len;
        r.sum = a.sum + b.sum;
        return r;
    }
};

// 模式 A: 单树 300 轮, 每轮 60 混合操作
void test_hld_single()
{
    mt19937 rng(20260903);
    static HLD hld{61};
    static SegTree<InfoH, TagH> tr{61};
    for (int tc = 0; tc < 300; tc++)
    {
        int n = tc == 0 ? 1 : (tc <= 3 ? 61 : 1 + rng() % 60);
        Graph<false, Empty> g{n, n};
        vector<LL> ref(n + 1);
        VI par(n + 1, 0), depn(n + 1, 0);
        vector<VI> ch(n + 1);
        for (int i = 1; i <= n; i++) ref[i] = (LL)(rng() % 41) - 20;
        for (int v = 2; v <= n; v++)
        {
            int u = tc == 1 ? v - 1 : (tc == 2 ? 1 : (tc == 3 ? v / 2 : 1 + rng() % (v - 1)));
            par[v] = u;
            depn[v] = depn[u] + 1;
            ch[u].push_back(v);
            g.add(u, v);
        }
        hld.init(n);
        hld.build(g, 1);
        vector<InfoH> b(n + 1);
        for (int i = 1; i <= n; i++) b[i] = {1, ref[hld.seg[i]]};
        tr.init(n);
        tr.build(b);
        auto sub_apply = [&](int r, LL k)
        {
            VI stk{r};
            while (!stk.empty())
            {
                int x = stk.back(); stk.pop_back();
                ref[x] += k;
                for (int y : ch[x]) stk.push_back(y);
            }
        };
        auto sub_sum = [&](int r)
        {
            LL s = 0;
            VI stk{r};
            while (!stk.empty())
            {
                int x = stk.back(); stk.pop_back();
                s += ref[x];
                for (int y : ch[x]) stk.push_back(y);
            }
            return s;
        };
        for (int t = 0; t < 60; t++)
        {
            int op = rng() % 4;
            int u = 1 + rng() % n, v = 1 + rng() % n;
            LL k = (LL)(rng() % 11) - 5;
            if (op == 0) // 路径加
            {
                modify_path(hld, tr, u, v, {k});
                while (u != v)
                {
                    if (depn[u] < depn[v]) swap(u, v);
                    ref[u] += k;
                    u = par[u];
                }
                ref[u] += k;   // LCA
            }
            else if (op == 1) // 路径查
            {
                int a = u, b2 = v;
                LL s = 0;
                while (a != b2)
                {
                    if (depn[a] < depn[b2]) swap(a, b2);
                    s += ref[a];
                    a = par[a];
                }
                s += ref[a];
                assert(query_path(hld, tr, u, v).sum == s);
            }
            else if (op == 2) // 子树加
            {
                modify_subtree(hld, tr, u, {k});
                sub_apply(u, k);
            }
            else // 子树查
            {
                assert(query_subtree(hld, tr, u).sum == sub_sum(u));
            }
        }
        // 尾对账: 根 1 子树 = 全树
        assert(query_subtree(hld, tr, 1).sum == sub_sum(1));
    }
}

// 模式 B: 森林 150 轮(一半点孤立成单点树), build(g) 全森林扫描
void test_hld_forest()
{
    mt19937 rng(3369);
    static HLD hld{51};
    static SegTree<InfoH, TagH> tr{51};
    for (int tc = 0; tc < 150; tc++)
    {
        int n = 1 + rng() % 50;
        Graph<false, Empty> g{n, n};
        vector<LL> ref(n + 1);
        VI par(n + 1, 0), depn(n + 1, 0), rt(n + 1, 0);
        vector<VI> ch(n + 1);
        for (int i = 1; i <= n; i++) { ref[i] = (LL)(rng() % 41) - 20; rt[i] = i; }
        for (int v = 2; v <= n; v++)
        {
            if (rng() % 2)
            {
                int u = tc == 1 ? v - 1 : (tc == 2 ? 1 : (tc == 3 ? v / 2 : 1 + rng() % (v - 1)));
                par[v] = u;
                depn[v] = depn[u] + 1;
                rt[v] = rt[u];
                ch[u].push_back(v);
                g.add(u, v);
            }
        }
        hld.init(n);
        hld.build(g);
        vector<InfoH> b(n + 1);
        for (int i = 1; i <= n; i++) b[i] = {1, ref[hld.seg[i]]};
        tr.init(n);
        tr.build(b);
        auto sub_apply = [&](int r, LL k)
        {
            VI stk{r};
            while (!stk.empty())
            {
                int x = stk.back(); stk.pop_back();
                ref[x] += k;
                for (int y : ch[x]) stk.push_back(y);
            }
        };
        auto sub_sum = [&](int r)
        {
            LL s = 0;
            VI stk{r};
            while (!stk.empty())
            {
                int x = stk.back(); stk.pop_back();
                s += ref[x];
                for (int y : ch[x]) stk.push_back(y);
            }
            return s;
        };
        for (int t = 0; t < 60; t++)
        {
            int op = rng() % 4;
            int u = 1 + rng() % n, v = 1 + rng() % n;
            LL k = (LL)(rng() % 11) - 5;
            if (op == 0 || op == 1) // 路径对: 同根才测(跨分量无契约)
            {
                if (rt[u] != rt[v]) continue;
                if (op == 0)
                {
                    modify_path(hld, tr, u, v, {k});
                    while (u != v)
                    {
                        if (depn[u] < depn[v]) swap(u, v);
                        ref[u] += k;
                        u = par[u];
                    }
                    ref[u] += k;   // LCA
                }
                else
                {
                    int a = u, b2 = v;
                    LL s = 0;
                    while (a != b2)
                    {
                        if (depn[a] < depn[b2]) swap(a, b2);
                        s += ref[a];
                        a = par[a];
                    }
                    s += ref[a];
                    assert(query_path(hld, tr, u, v).sum == s);
                }
            }
            else if (op == 2) // 子树加(孤立点子树=自身, 天然覆盖)
            {
                modify_subtree(hld, tr, u, {k});
                sub_apply(u, k);
            }
            else // 子树查
            {
                assert(query_subtree(hld, tr, u).sum == sub_sum(u));
            }
        }
        // 尾对账: 每棵树的根子树和 = 分量总和
        for (int i = 1; i <= n; i++)
            if (rt[i] == i) assert(query_subtree(hld, tr, i).sum == sub_sum(i));
    }
}

int main()
{
    test_hld_single();
    test_hld_forest();
    cout << "hld_check passed: HLD/SegTree path+subtree single+forest all tests ok\n";
    return 0;
}
