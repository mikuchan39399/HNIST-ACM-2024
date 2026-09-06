// zoi: skiplist
#ifndef Z_OI_SKIPLIST
#define Z_OI_SKIPLIST

#include "../../杂项/随机数/z_rnd.cpp"
#include "../../杂项/utils/utils.cpp"

// 升序维护 LL 集合, 插/删/排名/第k小/前驱/后继期望 O(log n); 结点删除后回收复用
// K = 层数上限, 需 >= log2(规模), 默认 20 支撑 1e6
// 值域约定: 元素取值在 (-INF, INF) 内, 前驱/后继无解返回 ∓INF, 第k小越界返回 INF
// 内存: 每结点 (16+8K)B, K=20 即 176B; 预算 = 峰值元素数, 1e6 ≈ 176MB
template <int K = 20>
struct SkipList
{
    struct node
    {
        LL val;
        int lvl;
        array<int, K> nxt, span;
        node(LL v = 0, int k = 1) : val(v), lvl(k)
        {
            nxt.fill(-1);
            span.fill(0);
        }
    };
    vector<node> buf;
    VI rub;
    int tot;
    int n;
    int budget;
    // 构造: 预算 max_nodes 结点(回收复用), 哨兵 0 号就位
    // 时间: O(1) | 空间: O(预算)
    SkipList(int max_nodes = 1000010) : tot(0), n(0), budget(max_nodes)
    {
        buf.reserve(budget + 1);
        rub.reserve(budget);
        buf.push_back(node(-INF, K));
    }
    // 插入 v (去重: 已存在不重复插), 返回该元素结点 id (>0)
    // 时间: 期望 O(log n) | 空间: O(1)
    int insert(LL v)
    {
        int cur = 0;
        int update[K];
        int rnk[K];
        for (int i = K - 1; i >= 0; i--)
        {
            rnk[i] = (i == K - 1) ? 0 : rnk[i + 1];
            while (buf[cur].nxt[i] != -1 && buf[buf[cur].nxt[i]].val < v)
            {
                rnk[i] += buf[cur].span[i];
                cur = buf[cur].nxt[i];
            }
            if (buf[cur].nxt[i] != -1 && buf[buf[cur].nxt[i]].val == v) // 允许重复就删掉这个 if 块
                return buf[cur].nxt[i];
            update[i] = cur;
        }
        int id = new_node(v);
        int nxt_sz = buf[id].lvl;
        for (int i = 0; i < nxt_sz; i++)
        {
            buf[id].nxt[i] = buf[update[i]].nxt[i];
            buf[update[i]].nxt[i] = id;
            buf[id].span[i] = buf[update[i]].span[i] - (rnk[0] - rnk[i]);
            buf[update[i]].span[i] = rnk[0] - rnk[i] + 1;
        }
        for (int i = nxt_sz; i < K; i++)
            buf[update[i]].span[i]++;
        n++;
        return id;
    }
    // 删除 v, 返回被删结点 id; 不存在返回 -1
    // 时间: 期望 O(log n) | 空间: O(1)
    int erase(LL v)
    {
        int cur = 0;
        int update[K];
        for (int i = K - 1; i >= 0; i--)
        {
            while (buf[cur].nxt[i] != -1 && buf[buf[cur].nxt[i]].val < v)
                cur = buf[cur].nxt[i];
            update[i] = cur;
        }
        int id = buf[cur].nxt[0];
        if (id == -1 || buf[id].val != v)
            return -1;
        int nxt_sz = buf[id].lvl;
        for (int i = 0; i < K; i++)
        {
            if (i < nxt_sz)
            {
                buf[update[i]].span[i] += buf[id].span[i] - 1;
                buf[update[i]].nxt[i] = buf[id].nxt[i];
            }
            else buf[update[i]].span[i]--;
        }
        rub.push_back(id);
        n--;
        return id;
    }
    // 返回 < v 的最大值, 无前驱返回 -INF
    // 时间: 期望 O(log n) | 空间: O(1)
    LL get_pre(LL v)
    {
        int cur = 0;
        for (int i = K - 1; i >= 0; i--)
            while (buf[cur].nxt[i] != -1 && buf[buf[cur].nxt[i]].val < v)
                cur = buf[cur].nxt[i];
        return buf[cur].val;
    }
    // 返回 > v 的最小值, 无后继返回 INF
    // 时间: 期望 O(log n) | 空间: O(1)
    LL get_suf(LL v)
    {
        int cur = 0;
        for (int i = K - 1; i >= 0; i--)
            while (buf[cur].nxt[i] != -1 && buf[buf[cur].nxt[i]].val <= v)
                cur = buf[cur].nxt[i];
        if (buf[cur].nxt[0] == -1) return INF;
        return buf[buf[cur].nxt[0]].val;
    }
    // 返回 < v 的元素个数
    // 时间: 期望 O(log n) | 空间: O(1)
    int get_rank(LL v)
    {
        int cur = 0;
        int ans = 0;
        for (int i = K - 1; i >= 0; i--)
        {
            while (buf[cur].nxt[i] != -1 && buf[buf[cur].nxt[i]].val < v)
            {
                ans += buf[cur].span[i];
                cur = buf[cur].nxt[i];
            }
        }
        return ans;
    }
    // 返回第 k 小 (1-based), k 越界返回 INF
    // 时间: 期望 O(log n) | 空间: O(1)
    LL get_kth(int k)
    {
        if (k < 1 || k > n) return INF;
        int cur = 0;
        for (int i = K - 1; i >= 0; i--)
        {
            while (buf[cur].nxt[i] != -1 && buf[cur].span[i] <= k)
            {
                k -= buf[cur].span[i];
                cur = buf[cur].nxt[i];
            }
        }
        return buf[cur].val;
    }
    // 返回元素个数
    // 时间: O(1) | 空间: O(1)
    int size()
    {
        return n;
    }
    // 多测复位
    // 时间: O(tot) | 空间: O(1)
    void clear()
    {
        tot = 0;
        n = 0;
        rub.clear();
        buf.clear();
        buf.push_back(node(-INF, K));
    }
private:
    int new_node(LL v)
    {
        int lvl = 1;
        while (lvl < K && z_rnd(1)) lvl++;
        if (rub.size())
        {
            int id = rub.back(); rub.pop_back();
            buf[id] = node(v, lvl);
            return id;
        }
        assert(tot < budget);
        tot++;
        buf.push_back(node(v, lvl));
        return tot;
    }
};
#endif

/* Usage:
int main()
{
    SkipList<20> s(16);
    int id = s.insert(2);
    cout << (s.insert(2) == id) << "\n"; // 1, 去重
    s.insert(5);
    cout << s.get_rank(5) << " " << s.get_kth(2) << "\n"; // 1 5
    cout << s.get_pre(5) << " " << s.get_suf(2) << "\n"; // 2 5
    cout << (s.erase(2) == id) << "\n"; // 1, 删除后 id 可被复用
    s.clear();
    cout << s.size() << "\n"; // 0
}
*/
