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
    for (int tc = 0; tc < 200; tc++)
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

int main()
{
    mt19937 rng(42);
    leftist_domain_a(rng);
    leftist_domain_b(rng);
    persistent_test(rng);
    cout << "leftist_check passed: LeftistTree / PersistentLeftist all tests ok\n";
    return 0;
}