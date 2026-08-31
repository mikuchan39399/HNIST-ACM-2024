// zoi: persistentTrie
#ifndef Z_OI_PERS_TRIE
#define Z_OI_PERS_TRIE

#include <array>
#include <cassert>
#include <string>
#include <vector>
#include "../../杂项/utils/utils.cpp"

using namespace std;

// ==========================================
// 模板参数 K:
//   - K = 2  : 01-Trie
//   - K = 26 : 小写字母字符串
//   - K = 62 : 大小写字母 + 数字
// 模板参数 HB: 整数位深(最高位下标), 默认 63;
//   值 < 2^31 时传 30, 结点数与 cap 近似减半
// max_nodes 开多大:
//   - 存整数时: 插入次数 * (HB + 2) + 10
//   - 存字符串时: 所有字符串的最大长度和 + 10
// ==========================================
template <int K = 2, int HB = 63>
struct PersTrie
{
    struct Node
    {
        array<int, K> ch{};
        int cnt = 0;
    };
    int cap;
    int tot = 0;
    vector<Node> tr;
    PersTrie(int max_nodes = 4000010) : cap(max_nodes)
    {
        tr.reserve(max_nodes);
        tr.push_back(Node{});
    }
    // 多测清空, 复用已分配内存
    // 时间: O(1) | 空间: O(1)
    void clear()
    {
        tot = 0;
        tr.clear();
        tr.push_back(Node{});
    }
    // 在版本 rt 上插入非负整数 x, 返回新版本根句柄 —— 仅 K ∈ [2, 10] 编译
    // 时间: O(HB + 1) | 空间: 至多 HB + 1 个新结点
    int insert(int rt, LL x)
    {
        static_assert(K >= 2 && K <= 10, "insert(数值) 仅 K <= 10 可用");
        return insert(rt, x, HB);
    }
    // 在版本 rt 上插入单词 s, 返回新版本根句柄
    // 时间: O(|s|) | 空间: 至多 |s| 个新结点
    int insert(int rt, const string& s) { return insert(rt, s, 0); }
    // 版本 rt 中与 x 异或的最大值; 空版本返回 -1
    // 时间: O(HB + 1) | 空间: O(1)
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
    // 版本差集 (Σplus 并集 − Σminus 并集) 中与 x 异或的最大值; 空差集返回 -1
    // 时间: O((|plus|+|minus|) * (HB + 1)) | 空间: O(|plus|+|minus|)
    LL max_xor(VI plus, VI minus, LL x) const
    {
        static_assert(K >= 2 && K <= 10, "max_xor 仅 K <= 10 可用");
        int total = 0;
        for (int p : plus) total += tr[p].cnt;
        for (int p : minus) total -= tr[p].cnt;
        if (total == 0) return -1;
        LL res = 0;
        for (int i = HB; i >= 0; i--)
        {
            int want = ((x >> i) & 1) ^ 1;
            int cw = 0;
            for (int p : plus) cw += tr[tr[p].ch[want]].cnt;
            for (int p : minus) cw -= tr[tr[p].ch[want]].cnt;
            int d = cw > 0 ? want : (want ^ 1);
            if (d == want) res |= 1LL << i;
            for (int& p : plus) p = tr[p].ch[d];
            for (int& p : minus) p = tr[p].ch[d];
        }
        return res;
    }
    // 版本差集 (p − q) 中, {xs 每个值与差集全体数的异或值} 的第 k 大(含重复);
    // 差集空 / xs 空 / k 越界返回 -1 —— 仅 K ∈ [2, 10] 编译
    // 时间: O(|xs| * (HB + 1)) | 空间: O(|xs|)
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
    // 版本差集中以 s 为前缀的单词个数;
    // 时间: O((|plus|+|minus|) * |s|) | 空间: O(|plus|+|minus|)
    LL count_prefix(const VI& plus, const VI& minus, const string& s) const
    {
        VI p = plus, m = minus;
        for (char c : s)
        {
            int id = to_id(c);
            for (int& t : p) t = tr[t].ch[id];
            for (int& t : m) t = tr[t].ch[id];
            int cw = 0;
            for (int t : p) cw += tr[t].cnt;
            for (int t : m) cw -= tr[t].cnt;
            if (cw <= 0) return 0;
        }
        int cw = 0;
        for (int t : p) cw += tr[t].cnt;
        for (int t : m) cw -= tr[t].cnt;
        return max(cw, 0);
    }
    // 版本 rt 中以 s 为前缀的单词个数; 空版本返回 0
    // 时间: O(|s|) | 空间: O(1)
    LL count_prefix(int rt, const string& s) const { return count_prefix({rt}, {0}, s); }
    // 版本差集中与 s 的最长公共前缀长度; 空差集返回 -1
    // 时间: O((|plus|+|minus|) * |s|) | 空间: O(|plus|+|minus|)
    int lcp_len(const VI& plus, const VI& minus, const string& s) const
    {
        int total = 0;
        for (int t : plus) total += tr[t].cnt;
        for (int t : minus) total -= tr[t].cnt;
        if (total == 0) return -1;
        VI p = plus, m = minus;
        int len = 0;
        for (char c : s)
        {
            int id = to_id(c);
            int cw = 0;
            for (int t : p) cw += tr[tr[t].ch[id]].cnt;
            for (int t : m) cw -= tr[tr[t].ch[id]].cnt;
            if (cw <= 0) break;
            for (int& t : p) t = tr[t].ch[id];
            for (int& t : m) t = tr[t].ch[id];
            len++;
        }
        return len;
    }
    // 版本 rt 中的整数个数
    // 时间: O(1) | 空间: O(1)
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
        assert(tot + 1 <= cap && "max_nodes 开小了");
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
    int insert(int p, const string& s, size_t i)
    {
        p = fork(p);
        tr[p].cnt++;
        if (i == s.size()) return p;
        int id = to_id(s[i]);
        tr[p].ch[id] = insert(tr[p].ch[id], s, i + 1);
        return p;
    }
};
#endif

/*
 * Usage: PersTrie
 * ---------------------------------------------------------
 * [1] 01-Trie: 区间最大异或和
 * static PersTrie pt(64 * (n + Q) + 10); // cap: 插入次数*(HB+2)+10
 * VI rt(n + 1, 0);
 * for (int i = 1; i <= n; i++) rt[i] = pt.insert(rt[i - 1], a[i]);
 * pt.max_xor(rt[i], y);                  // 前i个数与y的最大异或, 空树返回-1
 * pt.max_xor({rt[r]}, {rt[l - 1]}, x);   // 区间a[l..r]与x的最大异或, 空树返回-1
 * pt.size(rt[i]);                        // 版本i包含的元素总数
 * ---------------------------------------------------------
 * [2] Char-Trie: 字符串区间前缀 / LCP
 * static PersTrie<26> ps(100010);        // cap: 插入串总长+10
 * VI rs(n + 1, 0);
 * for (int i = 1; i <= n; i++) rs[i] = ps.insert(rs[i - 1], s[i]);
 * ps.count_prefix({rs[r]}, {rs[l - 1]}, t); // 区间s[l..r]中以t为前缀的个数
 * ps.lcp_len({rs[r]}, {rs[l - 1]}, t);      // 区间s[l..r]中与t的最长公共前缀长度
 * ---------------------------------------------------------
 * [3] Tree-Path: 树上路径查询
 * // rt[u]为根到节点u建出的版本, 路径u->v查差集: (u+v)-(lca+fa_lca)
 * pt.max_xor({rt[u], rt[v]}, {rt[lca], rt[fa_lca]}, x);
 * ---------------------------------------------------------
 * [4] 01-Trie: 子矩阵异或第 k 大 (P5795)
 * static PersTrie<2, 30> pt(32 * (m + 1) + 10); // 值 < 2^31: HB=30, cap 减半
 * VI rt(m + 1, 0);
 * for (int j = 1; j <= m; j++) rt[j] = pt.insert(rt[j - 1], y[j]);
 * vector<LL> xs(x + u, x + d + 1);             // 行切片 x[u..d] (x 为 LL 数组)
 * pt.kth_xor(rt[r], rt[l - 1], xs, k);         // {x_i ^ y_j} 全体第 k 大
 */
