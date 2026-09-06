// ============ leftist_check 左偏树双件回归 ============
// 覆盖: LeftistTree 域A(merge/insert/pop/erase/set_val/add_val/add_all, 逐操作
//       全量对账 alive/size/sum/top_id/top_val/heap_count/roots) | 域B(heap_add/
//       heap_mul/add_all/merge/insert/pop, 契约域: 仅堆级+堆顶查询) |
//       PersistentLeftist(版本链 insert/merge/pop + 历史版本随机回访 vs 快照)
// 纪律: 改动 左偏树/可持久化左偏树, 必重跑本套件
// 跑法: g++ -std=c++20 -Wall -Wextra -O2 leftist_check.cpp -o leftist_check && ./leftist_check
#include <iostream>
#include <vector>
#include <cassert>
#include <algorithm>
#include <set>
#include <random>
#include "../左偏树.cpp"
#include "../可持久化左偏树.cpp"

using namespace std;

// 域A: 无懒标记, 全接口对账; ops 后逐堆校验
static void leftist_domain_a(mt19937& rng)
{
    static LeftistTree<LL, less<LL>> lt(400, 900);
    for (int tc = 0; tc < 300; tc++)
    {
        int n = 1 + rng() % 6;
        vector<LL> vals(n + 1);
        for (int i = 1; i <= n; i++) vals[i] = (LL)(rng() % 41) - 20;
        lt.init(n, vals);

        // 暴力: heaps[h] = 存活成员 (val, id); heap_of[id] = 堆号; 死堆留空壳
        vector<vector<pair<LL, int>>> heaps(n + 1);
        VI heap_of(n + 1);
        vector<char> alivef(n + 1, 1);
        for (int i = 1; i <= n; i++)
        {
            heaps[i] = {{vals[i], i}};
            heap_of[i] = i;
        }
        int total = n;
        LL g = 0;

        auto member_val = [&](int x) -> LL
        {
            for (auto& pr : heaps[heap_of[x]])
                if (pr.second == x) return pr.first;
            return 0;
        };
        auto reconcile = [&]()
        {
            int live_heaps = 0;
            VI tops;
            for (size_t h = 0; h < heaps.size(); h++)
            {
                if (heaps[h].empty()) continue;
                live_heaps++;
                int x = heaps[h][0].second;
                assert(lt.alive(x));
                assert(lt.get_size(x) == (int)heaps[h].size());
                auto mn = *min_element(heaps[h].begin(), heaps[h].end());
                assert(lt.get_top_val(x) == mn.first + g);
                assert(lt.get_top_id(x) == mn.second);
                tops.push_back(mn.second);
                LL s = 0;
                for (auto& pr : heaps[h]) s += pr.first;
                assert(lt.get_heap_sum(x) == s + g * (LL)heaps[h].size());
            }
            assert(lt.get_heap_count() == live_heaps);
            VI got_tops = lt.get_roots_id();
            sort(tops.begin(), tops.end());
            sort(got_tops.begin(), got_tops.end());
            assert(got_tops == tops);
        };

        for (int op = 0, ops = 60; op < ops; op++)
        {
            int live_cnt = 0;
            for (int i = 1; i <= total; i++) live_cnt += alivef[i];
            int x = 1 + rng() % total, y = 1 + rng() % total;
            int act = rng() % 8;
            if (act == 0) // merge
            {
                if (!alivef[x] || !alivef[y] || heap_of[x] == heap_of[y]) continue;
                assert(lt.merge(x, y) != -1);
                int hx = heap_of[x], hy = heap_of[y];
                for (auto& pr : heaps[hy])
                {
                    heaps[hx].push_back(pr);
                    heap_of[pr.second] = hx;
                }
                heaps[hy].clear();
            }
            else if (act == 1) // insert (活点并入所在堆, 死点/死堆则独立成堆)
            {
                LL v = (LL)(rng() % 41) - 20;
                int nid = lt.insert(x, v);
                assert(nid == total + 1);
                total++;
                alivef.push_back(1);
                heap_of.push_back(0);
                if (alivef[x] && !heaps[heap_of[x]].empty())
                {
                    int hx = heap_of[x];
                    heaps[hx].push_back({v - g, total});
                    heap_of[total] = hx;
                }
                else
                {
                    heap_of[total] = (int)heaps.size();
                    heaps.push_back({{v - g, total}});
                }
            }
            else if (act == 2) // pop
            {
                if (!alivef[x]) continue;
                int h = heap_of[x];
                auto mn = *min_element(heaps[h].begin(), heaps[h].end());
                lt.pop(x);
                for (auto it = heaps[h].begin(); it != heaps[h].end(); it++)
                    if (it->second == mn.second) { heaps[h].erase(it); break; }
                alivef[mn.second] = 0;
            }
            else if (act == 3) // erase
            {
                if (!alivef[x]) { assert(lt.erase(x) == -1); continue; }
                int h = heap_of[x];
                lt.erase(x);
                for (auto it = heaps[h].begin(); it != heaps[h].end(); it++)
                    if (it->second == x) { heaps[h].erase(it); break; }
                alivef[x] = 0;
            }
            else if (act == 4) // set_val
            {
                if (!alivef[x]) continue;
                LL v = (LL)(rng() % 41) - 20;
                lt.set_val(x, v);
                for (auto& pr : heaps[heap_of[x]])
                    if (pr.second == x) pr.first = v - g;
            }
            else if (act == 5) // add_val
            {
                if (!alivef[x]) continue;
                LL k = (LL)(rng() % 11) - 5;
                lt.add_val(x, k);
                for (auto& pr : heaps[heap_of[x]])
                    if (pr.second == x) pr.first += k;
            }
            else if (act == 6) // add_all
            {
                LL k = (LL)(rng() % 7) - 3;
                lt.add_all(k);
                g += k;
            }
            else // 单点值抽检
            {
                if (!alivef[x]) continue;
                assert(lt.get_val(x) == member_val(x) + g);
            }
            reconcile();
        }
    }
}

// 域B: 懒标记契约域, 仅堆级+堆顶查询 (单点 get_val/set_val/erase 失效, 禁用)
static void leftist_domain_b(mt19937& rng)
{
    static LeftistTree<LL, less<LL>> lt(400, 900);
    for (int tc = 0; tc < 300; tc++)
    {
        int n = 1 + rng() % 6;
        vector<LL> vals(n + 1);
        for (int i = 1; i <= n; i++) vals[i] = (LL)(rng() % 41) - 20;
        lt.init(n, vals);

        vector<vector<pair<LL, int>>> heaps(n + 1);
        VI heap_of(n + 1);
        vector<char> alivef(n + 1, 1);
        for (int i = 1; i <= n; i++)
        {
            heaps[i] = {{vals[i], i}};
            heap_of[i] = i;
        }
        int total = n;
        LL g = 0;

        auto reconcile = [&]()
        {
            int live_heaps = 0;
            for (size_t h = 0; h < heaps.size(); h++)
            {
                if (heaps[h].empty()) continue;
                live_heaps++;
                int x = heaps[h][0].second;
                assert(lt.alive(x));
                assert(lt.get_size(x) == (int)heaps[h].size());
                auto mn = *min_element(heaps[h].begin(), heaps[h].end());
                assert(lt.get_top_val(x) == mn.first + g);
                assert(lt.get_top_id(x) == mn.second);
                LL s = 0;
                for (auto& pr : heaps[h]) s += pr.first;
                assert(lt.get_heap_sum(x) == s + g * (LL)heaps[h].size());
            }
            assert(lt.get_heap_count() == live_heaps);
        };

        for (int op = 0, ops = 60; op < ops; op++)
        {
            int x = 1 + rng() % total, y = 1 + rng() % total;
            int act = rng() % 6;
            if (act == 0) // merge
            {
                if (!alivef[x] || !alivef[y] || heap_of[x] == heap_of[y]) continue;
                assert(lt.merge(x, y) != -1);
                int hx = heap_of[x], hy = heap_of[y];
                for (auto& pr : heaps[hy])
                {
                    heaps[hx].push_back(pr);
                    heap_of[pr.second] = hx;
                }
                heaps[hy].clear();
            }
            else if (act == 1) // insert
            {
                LL v = (LL)(rng() % 41) - 20;
                int nid = lt.insert(x, v);
                assert(nid == total + 1);
                total++;
                alivef.push_back(1);
                heap_of.push_back(0);
                if (alivef[x] && !heaps[heap_of[x]].empty())
                {
                    int hx = heap_of[x];
                    heaps[hx].push_back({v - g, total});
                    heap_of[total] = hx;
                }
                else
                {
                    heap_of[total] = (int)heaps.size();
                    heaps.push_back({{v - g, total}});
                }
            }
            else if (act == 2) // pop
            {
                if (!alivef[x]) continue;
                int h = heap_of[x];
                auto mn = *min_element(heaps[h].begin(), heaps[h].end());
                lt.pop(x);
                for (auto it = heaps[h].begin(); it != heaps[h].end(); it++)
                    if (it->second == mn.second) { heaps[h].erase(it); break; }
                alivef[mn.second] = 0;
            }
            else if (act == 3) // heap_add
            {
                if (!alivef[x]) continue;
                LL k = (LL)(rng() % 11) - 5;
                lt.heap_add(x, k);
                for (auto& pr : heaps[heap_of[x]]) pr.first += k;
            }
            else if (act == 4) // heap_mul (m > 0)
            {
                if (!alivef[x]) continue;
                LL m = 1 + rng() % 3;
                lt.heap_mul(x, m);
                for (auto& pr : heaps[heap_of[x]]) pr.first = m * pr.first + (m - 1) * g;
            }
            else // add_all
            {
                LL k = (LL)(rng() % 7) - 3;
                lt.add_all(k);
                g += k;
            }
            reconcile();
        }
    }
}

// 可持久化左偏树: 版本链 + 历史版本随机回访 vs multiset 快照
static void persistent_test(mt19937& rng)
{
    static PersistentLeftist<LL, less<LL>> pl(2000000);
    for (int tc = 0; tc < 300; tc++)
    {
        pl.init();
        vector<int> rts{0};
        vector<multiset<LL>> snaps(1);
        for (int op = 0, ops = 120; op < ops; op++)
        {
            int act = rng() % 3;
            if (act == 0) // insert
            {
                int v0 = rng() % rts.size();
                LL v = (LL)(rng() % 101) - 50;
                rts.push_back(pl.insert(rts[v0], v));
                snaps.push_back(snaps[v0]);
                snaps.back().insert(v);
            }
            else if (act == 1) // merge 任意两版本
            {
                int a = rng() % rts.size(), b = rng() % rts.size();
                rts.push_back(pl.merge(rts[a], rts[b]));
                snaps.push_back(snaps[a]);
                snaps.back().insert(snaps[b].begin(), snaps[b].end());
            }
            else // pop (空堆跳过)
            {
                int v0 = rng() % rts.size();
                if (rts[v0] == 0) continue;
                rts.push_back(pl.pop(rts[v0]));
                snaps.push_back(snaps[v0]);
                snaps.back().erase(snaps.back().begin());
            }
            // 随机回访历史版本
            int q = rng() % rts.size();
            assert(pl.empty(rts[q]) == snaps[q].empty());
            if (!snaps[q].empty())
            {
                assert(pl.size(rts[q]) == (int)snaps[q].size());
                assert(pl.top(rts[q]) == *snaps[q].begin());
                LL s = 0;
                for (LL e : snaps[q]) s += e;
                assert(pl.sum(rts[q]) == s);
            }
        }
    }
}

template <bool Max>
void large_mutable()
{
    constexpr int N = 200000, Q = 100000;
    using Cmp = conditional_t<Max, greater<LL>, less<LL>>;
    LeftistTree<LL, Cmp> t(N, Q);
    mt19937 rng(42);
    VLL vals(N + 1);
    auto order = [](pair<LL,int> a, pair<LL,int> b)
    {
        if (a.first != b.first) return Max ? a.first > b.first : a.first < b.first;
        return a.second < b.second;
    };
    for (int n : {N, 1, N})
    {
        set<pair<LL,int>, decltype(order)> ref(order);
        LL sum = 0, offset = 0;
        for (int i = 1; i <= n; ++i)
        {
            vals[i] = (int)(rng() % 1001) - 500;
            ref.insert({vals[i],i}); sum += vals[i];
        }
        t.init(n, vals);
        for (int step = 1; step < n; step *= 2)
            for (int i = 1; i + step <= n; i += step * 2) t.merge(i,i+step);
        assert(t.get_heap_count() == 1);
        int q = n == N ? Q : 100;
        for (int i = 0; i < q; ++i)
        {
            int x = 1 + rng() % n;
            LL k = (int)(rng() % 2001) - 1000;
            if (i % 3 == 0) { t.add_all(k); offset += k; }
            else
            {
                ref.erase({vals[x],x}); sum -= vals[x];
                if (i % 3 == 1) { t.set_val(x,k); vals[x] = k - offset; }
                else { t.add_val(x,k); vals[x] += k; }
                ref.insert({vals[x],x}); sum += vals[x];
            }
            assert(t.get_size(x) == n && t.get_heap_sum(x) == sum + offset*n);
            assert(t.get_val(x) == vals[x] + offset);
            assert(t.get_top_id(x) == ref.begin()->second);
            assert(t.get_top_val(x) == ref.begin()->first + offset);
            assert(t.same(x,1) && t.merge(x,1) == -1);
        }
        // 非堆顶惰删大量点, 再依独立有序集合弹空
        for (int x = 2; x <= n; x += 2)
        {
            t.erase(x); sum -= vals[x]; ref.erase({vals[x],x});
            assert(!t.alive(x) && t.erase(x) == -1);
        }
        while (!ref.empty())
        {
            auto [v,x] = *ref.begin();
            assert(t.get_top_id(x) == x && t.get_top_val(x) == v+offset);
            assert(t.get_heap_sum(x) == sum + offset*(LL)ref.size());
            t.pop(x); ref.erase(ref.begin()); sum -= v;
            assert(!t.alive(x));
        }
        assert(t.get_heap_count() == 0 && t.get_roots_id().empty());
    }
}

template <bool Max>
void persistent_large()
{
    constexpr int N = 200000;
    using Cmp = conditional_t<Max, greater<LL>, less<LL>>;
    PersistentLeftist<LL,Cmp> t(8000000);
    for (int shape = 0; shape < 3; ++shape)
    {
        t.init();
        VI roots(N+1);
        LL sign = Max ? -1 : 1;
        for (int i = 1; i <= N; ++i)
        {
            LL v = shape == 2 ? 7 : sign * (shape == 0 ? i : -i);
            roots[i] = t.insert(roots[i-1], v, i);
            int q = i / 2 + 1;
            assert(t.size(roots[q]) == q);
            LL want = shape == 2 ? 7*q : sign*(shape == 0 ? 1 : -1)*q*(q+1)/2;
            assert(t.sum(roots[q]) == want);
        }
        int rt = roots[N];
        for (int i = 1; i <= N; ++i)
        {
            LL want = shape == 2 ? 7 : sign * (shape == 0 ? i : -(N-i+1));
            assert(t.top(rt) == want && t.size(rt) == N-i+1);
            if (shape != 2) assert(t.top_pay(rt) == (shape == 0 ? i : N-i+1));
            rt = t.pop(rt);
            assert(t.size(roots[N]) == N);
        }
        assert(t.empty(rt) && t.pop(0) == 0 && t.sum(0) == 0);
        cout << "persistent large max=" << Max << " shape=" << shape << " nodes=" << t.tot << '\n';
    }
}

void persistent_sharing()
{
    PersistentLeftist<> t(100000);
    mt19937 rng(42);
    for (int tc = 0; tc < 300; ++tc)
    {
        t.init();
        VI roots{0};
        vector<multiset<LL>> ref(1);
        for (int i = 0; i < 60; ++i)
        {
            int a = rng()%roots.size(), b = rng()%roots.size();
            int old = t.tot;
            auto snapshot = [old](const auto& v) { return decay_t<decltype(v)>(v.begin(),v.begin()+old+1); };
            auto lc=snapshot(t.lc), rc=snapshot(t.rc), dist=snapshot(t.dist), sz=snapshot(t.sz), pay=snapshot(t.pay);
            auto val=snapshot(t.val), sum=snapshot(t.hsum);
            multiset<LL> next = ref[a];
            int rt;
            if (i%3 == 0 && ref[a].size()+ref[b].size() < 200)
            {
                rt=t.merge(roots[a],roots[b]);
                next.insert(ref[b].begin(),ref[b].end());
            }
            else if (i%3 == 1 && !next.empty())
            { rt=t.pop(roots[a]); next.erase(next.begin()); }
            else
            { LL v=(int)(rng()%41)-20; rt=t.insert(roots[a],v,i); next.insert(v); }
            for(int p=0;p<=old;++p)
                assert(t.lc[p]==lc[p] && t.rc[p]==rc[p] && t.dist[p]==dist[p] &&
                       t.sz[p]==sz[p] && t.pay[p]==pay[p] && t.val[p]==val[p] && t.hsum[p]==sum[p]);
            roots.push_back(rt); ref.push_back(next);
            int used=t.tot;
            for(size_t q=0;q<roots.size();++q)
            {
                assert(t.size(roots[q]) == (int)ref[q].size());
                assert(t.sum(roots[q]) == accumulate(ref[q].begin(),ref[q].end(),0LL));
                if(!ref[q].empty()) assert(t.top(roots[q]) == *ref[q].begin());
            }
            assert(t.tot == used);
        }
    }
    // merge_raw 只用于独占且互不重叠的新堆, 不与共享版本混用
    t.init();
    int a=t.new_node(4,40), b=t.new_node(-2,20);
    int raw=t.merge_raw(a,b);
    assert(t.top(raw)==-2 && t.top_pay(raw)==20 && t.sum(raw)==2 && t.size(raw)==2);
    int doubled=t.merge(raw,raw);
    assert(t.size(doubled)==4 && t.sum(doubled)==4 && t.size(raw)==2);
}

void deep_and_lazy()
{
    constexpr int N=200000;
    LeftistTree<> t(N,0);
    VLL a(N+1);
    for(int i=1;i<=N;++i) a[i]=-i;
    t.init(N,a);
    for(int i=2;i<=N;++i) t.merge(i-1,i);
    assert(t.get_top_id(1)==N && t.same(1,N));
    LL offset=0, factor=1;
    for(int i=0;i<100000;++i)
    {
        LL k=i%2 ? -3 : 3;
        if(i%3==0) { t.add_all(k); offset+=k; }
        else { t.heap_add(1,k); offset+=k; }
        if(i==300 || i==600) { t.heap_mul(1,2); factor*=2; offset*=2; }
        assert(t.get_top_val(1)==-N*factor+offset);
        assert(t.get_heap_sum(1)==-factor*N*(N+1LL)/2+offset*N);
    }
    for(int i=N;i>=1;--i)
    {
        assert(t.get_top_id(i)==i && t.get_top_val(i)==-i*factor+offset);
        t.pop(i);
    }
    assert(t.get_heap_count()==0);
    for(int n : {1,0,100,N})
    {
        t.init(n);
        assert(t.get_heap_count()==n);
        if(n) assert(t.get_top_val(1)==0);
    }
}

void reset_and_payload()
{
    struct Pay { int id=0; LL stamp=0; };
    PersistentLeftist<LL,greater<LL>,Pay> p(1000);
    LeftistTree<LL,greater<LL>> t(3,2);
    for(int tc=0;tc<300;++tc)
    {
        p.init();
        int r=p.new_node(-1000000000000LL,{1,99});
        assert(p.top_pay(r).stamp==99);
        r=p.insert(r,1000000000000LL,{2,88});
        assert(p.top(r)==1000000000000LL && p.top_pay(r).id==2 && p.sum(r)==0);
        int next=p.pop(r);
        assert(p.top_pay(next).id==1 && p.size(r)==2);
        t.init(2,VLL{0,3,3});
        assert(t.merge(1,2)==1);
        t.add_all(5); t.heap_mul(1,2); t.heap_add(1,-3);
        int id=t.insert(1,20);
        assert(id==3 && t.get_top_id(1)==3 && t.get_heap_sum(1)==46);
        assert(t.pop(1)==1 && !t.alive(3));
        assert(t.pop(1)==2 && t.pop(2)==0);
        assert(t.merge(1,2)==-1 && t.pop(1)==-1 && t.get_top_id(1)==-1);
        assert(t.get_size(1)==0 && t.get_top_val(1)==0 && t.get_heap_sum(1)==0);
        t.init(1); t.set_val(1,4); t.add_val(1,-6);
        assert(t.tot==3 && t.get_val(1)==-2 && t.get_size(1)==1);
    }
    PersistentLeftist<> one(1);
    for(int i=0;i<300;++i) { one.init(); assert(one.new_node(i)==1); assert(one.pop(1)==0); }
    p.init();
    int r=p.new_node(1);
    for(int i=0;i<30;++i) r=p.merge(r,r);
    assert(p.size(r)==(1<<30) && p.sum(r)==(1LL<<30));
}

int main()
{
    mt19937 rng(42);
    leftist_domain_a(rng);
    leftist_domain_b(rng);
    persistent_test(rng);
    deep_and_lazy();
    reset_and_payload();
    persistent_sharing();
    large_mutable<false>();
    large_mutable<true>();
    persistent_large<false>();
    persistent_large<true>();
    cout << "leftist_check passed: LeftistTree / PersistentLeftist all tests ok\n";
    return 0;
}
