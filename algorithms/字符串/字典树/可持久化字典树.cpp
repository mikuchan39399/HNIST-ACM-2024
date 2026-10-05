// zoi: persistentTrie
#ifndef Z_OI_PERS_TRIE
#define Z_OI_PERS_TRIE

#include "../../杂项/utils/utils.cpp"

using namespace std;

// 可持久化 Trie, 根自行保存, 0=空版本; 可重复/空串, 不删除, clear 使旧根失效
// K=26 小写 / 62 大小写及数字 / <=10 数字; 字符合法, 字符串与整数接口不混用
// 整数接口仅 2<=K<=10, 非负 LL 且放入 HB+1 位; 单版本计数不超 int, 多根单侧和不超 LL
// 差集按次数相减, 每个值的剩余次数须非负; 总数非负不够
// 点池不回收, 每次整数插入 HB+2 点, 字符串 |s|+1 点; 每点 4K+4 字节, 另有空根
template <int K = 2, int HB = 63>
struct PersTrie
{
    static_assert(K > 0 && HB >= 0 && HB <= 63);
    struct Node
    {
        array<int, K> ch{};
        int cnt = 0;
    };
    int cap;
    int tot = 0;
    vector<Node> tr;
    // 预留 max_nodes 个可分配点及空根. 时间 O(1), 空间 O(K*max_nodes)
    PersTrie(int max_nodes = 4000010) : cap(max_nodes)
    {
        assert(max_nodes >= 0);
        tr.reserve((size_t)max_nodes + 1);
        tr.push_back(Node{});
    }
    // 清空并保留容量, 旧根全失效. 时间 O(已用点数) 上界
    void clear()
    {
        tot = 0;
        tr.clear();
        tr.push_back(Node{});
    }
    // 在 rt 插入 x, 返回新根. 时间 O(HB+1), 新增 HB+2 点
    int insert(int rt, LL x)
    {
        static_assert(K >= 2 && K <= 10, "insert(数值) 仅 K <= 10 可用");
        return insert(rt, x, HB);
    }
    // 在 rt 插入 s, 返回新根. 时间 O(|s|), 新增 |s|+1 点
    int insert(int rt, const string& s)
    {
        int root = fork(rt), p = root;
        tr[p].cnt++;
        for (char c : s)
        {
            int id = to_id(c);
            int nxt = fork(tr[p].ch[id]);
            tr[p].ch[id] = nxt;
            p = nxt;
            tr[p].cnt++;
        }
        return root;
    }
    // rt 中与 x 异或的最大值, 空版本 -1. 时间 O(HB+1), 空间 O(1)
    LL max_xor(int rt, LL x) const
    {
        static_assert(K >= 2 && K <= 10, "max_xor 仅 K <= 10 可用");
        if (!tr[rt].cnt) return -1;
        LL res = 0;
        int p = rt;
        for (int i = HB; i >= 0; i--)
        {
            int want = ((x >> i) & 1) ^ 1;
            int nxt = tr[p].ch[want];
            if (tr[nxt].cnt) { res |= 1LL << i; p = nxt; }
            else p = tr[p].ch[want ^ 1];
        }
        return res;
    }
    // 多根差集 sum(plus)-sum(minus) 的最大异或, 空集 -1
    // 时间 O(R(HB+1)), 空间 O(R), R=两侧根数之和
    LL max_xor(VI plus, VI minus, LL x) const
    {
        static_assert(K >= 2 && K <= 10, "max_xor 仅 K <= 10 可用");
        LL total = 0;
        for (int p : plus) total += tr[p].cnt;
        for (int p : minus) total -= tr[p].cnt;
        if (total == 0) return -1;
        LL res = 0;
        for (int i = HB; i >= 0; i--)
        {
            int want = ((x >> i) & 1) ^ 1;
            LL cw = 0;
            for (int p : plus) cw += tr[tr[p].ch[want]].cnt;
            for (int p : minus) cw -= tr[tr[p].ch[want]].cnt;
            int d = cw > 0 ? want : (want ^ 1);
            if (d == want) res |= 1LL << i;
            for (int& p : plus) p = tr[p].ch[d];
            for (int& p : minus) p = tr[p].ch[d];
        }
        return res;
    }
    // xs 与差集 p-q 的所有配对异或值中第 k 大(含重复); 空集/空 xs/越界返回 -1
    // 时间 O(|xs|(HB+1)), 空间 O(|xs|)
    LL kth_xor(int p, int q, const VLL& xs, LL k) const
    {
        static_assert(K >= 2 && K <= 10, "kth_xor 仅 K <= 10 可用");
        int n = (int)xs.size();
        LL total = (LL)tr[p].cnt - tr[q].cnt;
        if (n == 0 || k < 1 || k > total * n) return -1;
        VI cp(n, p), cq(n, q);
        LL res = 0;
        for (int i = HB; i >= 0; i--)
        {
            LL one = 0;
            for (int t = 0; t < n; t++)
            {
                int want = (int)((xs[t] >> i) & 1) ^ 1;
                one += tr[tr[cp[t]].ch[want]].cnt - tr[tr[cq[t]].ch[want]].cnt;
            }
            int d = one >= k;
            if (d) res |= 1LL << i;
            else k -= one;
            for (int t = 0; t < n; t++)
            {
                int dir = (int)((xs[t] >> i) & 1) ^ d;
                cp[t] = tr[cp[t]].ch[dir];
                cq[t] = tr[cq[t]].ch[dir];
            }
        }
        return res;
    }
    // 差集中前缀为 s 的单词数. 时间 O(R|s|), 空间 O(R)
    LL count_prefix(const VI& plus, const VI& minus, const string& s) const
    {
        VI p = plus, m = minus;
        for (char c : s)
        {
            int id = to_id(c);
            for (int& t : p) t = tr[t].ch[id];
            for (int& t : m) t = tr[t].ch[id];
            LL cw = 0;
            for (int t : p) cw += tr[t].cnt;
            for (int t : m) cw -= tr[t].cnt;
            if (cw <= 0) return 0;
        }
        LL cw = 0;
        for (int t : p) cw += tr[t].cnt;
        for (int t : m) cw -= tr[t].cnt;
        return max(cw, 0LL);
    }
    // rt 中前缀为 s 的单词数. 时间 O(|s|), 空间 O(1)
    LL count_prefix(int rt, const string& s) const
    {
        for (char c : s)
        {
            rt = tr[rt].ch[to_id(c)];
            if (!rt) return 0;
        }
        return tr[rt].cnt;
    }
    // s 与差集中某个单词的最大 LCP, 空差集 -1. 时间 O(R|s|), 空间 O(R)
    int lcp_len(const VI& plus, const VI& minus, const string& s) const
    {
        LL total = 0;
        for (int t : plus) total += tr[t].cnt;
        for (int t : minus) total -= tr[t].cnt;
        if (total == 0) return -1;
        VI p = plus, m = minus;
        int len = 0;
        for (char c : s)
        {
            int id = to_id(c);
            LL cw = 0;
            for (int t : p) cw += tr[tr[t].ch[id]].cnt;
            for (int t : m) cw -= tr[tr[t].ch[id]].cnt;
            if (cw <= 0) break;
            for (int& t : p) t = tr[t].ch[id];
            for (int& t : m) t = tr[t].ch[id];
            len++;
        }
        return len;
    }
    // 版本元素数, 含重复. O(1)
    int size(int rt) const { return tr[rt].cnt; }
private:
    static int to_id(char c)
    {
        if constexpr (K == 62)
        {
            if (c >= '0' && c <= '9') return c - '0' + 52;
            if (c >= 'A' && c <= 'Z') return c - 'A' + 26;
            return c - 'a';
        }
        else if constexpr (K <= 10)
            return c - '0';
        else
            return c - 'a';
    }
    int fork(int p)
    {
        assert(tot < cap && "max_nodes 开小了");
        Node tmp = tr[p];
        tr.push_back(tmp);
        return ++tot;
    }
    int insert(int p, LL x, int i)
    {
        p = fork(p);
        tr[p].cnt++;
        if (i < 0) return p;
        int id = (x >> i) & 1;
        tr[p].ch[id] = insert(tr[p].ch[id], x, i - 1);
        return p;
    }
};
#endif

/* Usage
#include <persistentTrie.h>
int main()
{
    // 值<2^31 取 HB=30, 更大非负 LL 用默认 63
    PersTrie<2, 30> pt(4 * 32);
    VLL a{3, 5, 7};
    VI rt(4, 0);                          // rt[i] 是前 i 个数, rt[0]=空版本
    for (int i = 1; i <= 3; ++i) rt[i] = pt.insert(rt[i - 1], a[i - 1]);
    cout << pt.max_xor(rt[3], 2) << '\n';  // 7
    int l = 2, r = 3;
    cout << pt.max_xor({rt[r]}, {rt[l - 1]}, 2) << '\n'; // 7, 区间 [2,3]
    int branch = pt.insert(rt[1], 0);      // 从历史版本分叉: {3,0}, 旧版本不变
    cout << pt.size(branch) << ' ' << pt.size(rt[3]) << '\n'; // 2 3

    cout << pt.max_xor({rt[3], branch}, {rt[1]}, 2) << '\n'; // {3,5,7,0} -> 7
    VLL xs{0, 2};
    cout << pt.kth_xor(rt[3], rt[1], xs, 2) << '\n'; // {0^5,0^7,2^5,2^7} 第2大=7
    cout << pt.kth_xor(rt[3], rt[1], xs, 5) << '\n'; // -1, k 越界
    // 树路径点权差集: +root[u]+root[v]-root[lca]-root[fa(lca)], 根的父版本为 0.
    PersTrie<26> ps(1 + 4 + 4 + 2);
    int s0 = ps.insert(0, ""), s1 = ps.insert(s0, "abc");
    int s2 = ps.insert(s1, "abd"), sb = ps.insert(s0, "z");
    cout << ps.count_prefix(s2, "ab") << '\n'; // 2
    cout << ps.count_prefix({s2}, {s1}, "ab") << '\n'; // 1, 只剩 abd
    cout << ps.lcp_len({s2}, {s0}, "abcd") << '\n'; // 3, 与某个单词 LCP 的最大值
    cout << ps.lcp_len({s2}, {s2}, "a") << '\n'; // -1, 空差集
    cout << ps.count_prefix(sb, "") << '\n';    // 2, 含空串的总个数
    ps.clear();                               // 释放版本内容, 保留容量; 旧根全失效
    cout << ps.size(0) << '\n';                // 0
}
*/
