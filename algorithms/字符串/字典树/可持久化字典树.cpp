#include <array>
#include <cassert>
#include <string>
#include <vector>
using namespace std;
using LL = long long;
using VI = vector<int>;

#ifndef Z_OI_PERS_TRIE
#define Z_OI_PERS_TRIE
// ==========================================
// 模板参数 K:
//   - K = 2  : 01-Trie
//   - K = 26 : 小写字母字符串
//   - K = 62 : 大小写字母 + 数字
// max_nodes 开多大:
//   - 存整数时: 插入次数 * 64 + 10(如果数字 <= 1e9，可把源码里的 63 改成 30 进一步省空间)
//   - 存字符串时: 所有字符串的最大长度和 + 10
// ==========================================
template <int K = 2>
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
    // 多测清空, 复用已分配内存
    // 时间: O(1) | 空间: O(1)
    void clear()
    {
        tot = 0;
        tr.clear();
        tr.push_back(Node{});
    }
private:
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
public:
    // 在版本 rt 上插入非负整数 x, 返回新版本根句柄 —— 仅 K ∈ [2, 10] 编译
    // 时间: O(64) | 空间: 至多 64 个新结点
    int insert(int rt, LL x)
    {
        static_assert(K >= 2 && K <= 10, "insert(数值) 仅数字字符集(K<=10)可用");
        return insert(rt, x, 63);
    }
    // 在版本 rt 上插入单词 s, 返回新版本根句柄
    // 时间: O(|s|) | 空间: 至多 |s| 个新结点
    int insert(int rt, const string& s) { return insert(rt, s, 0); }
    // 版本 rt 中与 x 异或的最大值; 空版本返回 -1
    // 时间: O(64) | 空间: O(1)
    LL max_xor(int rt, LL x) const
    {
        static_assert(K >= 2 && K <= 10, "max_xor 仅数字字符集(K<=10)可用");
        if (!tr[rt].cnt) return -1;
        LL res = 0;
        int p = rt;
        for (int i = 63; i >= 0; i--)
        {
            int want = ((x >> i) & 1) ^ 1;
            int nxt = tr[p].ch[want];
            if (tr[nxt].cnt) { res |= 1LL << i; p = nxt; }
            else p = tr[p].ch[want ^ 1];
        }
        return res;
    }
    // 版本差集 (Σplus 并集 − Σminus 并集) 中与 x 异或的最大值; 空差集返回 -1
    // 时间: O((|plus|+|minus|) * 64) | 空间: O(|plus|+|minus|)
    LL max_xor(VI plus, VI minus, LL x) const
    {
        static_assert(K >= 2 && K <= 10, "max_xor 仅数字字符集(K<=10)可用");
        int total = 0;
        for (int p : plus) total += tr[p].cnt;
        for (int p : minus) total -= tr[p].cnt;
        if (total == 0) return -1;
        LL res = 0;
        for (int i = 63; i >= 0; i--)
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
};
#endif

/*
 * Usage: PersTrie
 * ---------------------------------------------------------
 * [1] 01-Trie: 区间最大异或和
 * static PersTrie pt(64 * (n + Q) + 10); // cap: 插入次数*64+10
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
 */