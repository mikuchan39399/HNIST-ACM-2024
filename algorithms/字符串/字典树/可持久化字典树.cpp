// zoi: persistentTrie
#ifndef Z_OI_PERS_TRIE
#define Z_OI_PERS_TRIE

#include "../../杂项/utils/utils.cpp"

using namespace std;

// 可持久化字典树: 每次插入返回新根, 旧版本仍可查询, 根句柄由调用方保存
// 模板参数 K:
//   - K = 2  : 01-Trie
//   - K = 26 : 小写字母字符串
//   - K = 62 : 大小写字母 + 数字
// 模板参数 HB: 整数位深(最高位下标), 默认 63;
//   值 < 2^31 时传 30, 结点数与 cap 近似减半
// 预算按累计分配算: 整数插入次数*(HB+2), 字符串总长+插入次数, 删除版本不回收
// 每结点 (4K+4)B; K=2 时 4e6 个结点约 48MB, 另留一个空根哨兵
// 差集查询按出现次数相减, 每个值的剩余次数必须非负; 字符串须落在 K 对应字符集内
// 整数 x 与所有查询参数均须非负且能放进 HB+1 位; 同一对象不混用字符串和整数接口
// 单版本元素数 <= INT_MAX; 多根正/负侧计数和分别 <= LLONG_MAX, xs.size() <= INT_MAX
// 支持重复值与空串, 不支持删除; clear 后所有旧根失效, 0 始终代表空版本
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
    // 预留 max_nodes 个可分配结点和一个空根, 初始没有任何版本内容
    // 时间: O(1) | 空间: (max_nodes+1)*sizeof(Node) 字节预留
    PersTrie(int max_nodes = 4000010) : cap(max_nodes)
    {
        assert(max_nodes >= 0);
        tr.reserve((size_t)max_nodes + 1);
        tr.push_back(Node{});
    }
    // 多测清空, 复用已分配内存
    // 时间: O(Used) 上界 | 空间: O(1); Node 析构平凡, 不逐个清零旧结点
    void clear()
    {
        tot = 0;
        tr.clear();
        tr.push_back(Node{});
    }
    // 在版本 rt 上插入非负整数 x, 返回新版本根句柄 —— 仅 K ∈ [2, 10] 编译
    // 时间: O(HB + 1) | 空间: HB + 2 个新结点
    int insert(int rt, LL x)
    {
        static_assert(K >= 2 && K <= 10, "insert(数值) 仅 K <= 10 可用");
        return insert(rt, x, HB);
    }
    // 在版本 rt 上插入单词 s, 返回新版本根句柄
    // 时间: O(|s|) | 空间: |s| + 1 个新结点
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
    // 版本 rt 中以 s 为前缀的单词个数; 空版本返回 0
    // 时间: O(|s|) | 空间: O(1)
    LL count_prefix(int rt, const string& s) const
    {
        for (char c : s)
        {
            rt = tr[rt].ch[to_id(c)];
            if (!rt) return 0;
        }
        return tr[rt].cnt;
    }
    // max LCP(s,t), t 遍历差集中各单词; 不是所有单词共同的前缀, 空差集返回 -1
    // 时间: O((|plus|+|minus|) * |s|) | 空间: O(|plus|+|minus|)
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
    // 返回版本 rt 中插入的元素个数, 重复整数或单词分别计数
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
    // 输入及查询数均 <2^31, 取 HB=30; 更大非负 LL 用默认 HB=63
    // 预算不含哨兵: 插入次数*(HB+2), 每次连根一起复制; 不存 root 数组的内存
    PersTrie<2, 30> pt(4 * 32);
    VLL a{3, 5, 7};
    VI rt(4, 0);                          // rt[i] 是前 i 个数, rt[0]=空版本
    for (int i = 1; i <= 3; ++i) rt[i] = pt.insert(rt[i - 1], a[i - 1]);
    cout << pt.max_xor(rt[3], 2) << '\n';  // 7
    int l = 2, r = 3;
    cout << pt.max_xor({rt[r]}, {rt[l - 1]}, 2) << '\n'; // 7, 区间 [2,3]
    int branch = pt.insert(rt[1], 0);      // 从历史版本分叉: {3,0}, 旧版本不变
    cout << pt.size(branch) << ' ' << pt.size(rt[3]) << '\n'; // 2 3
    // 多根按重复次数相加减; 必须保证每个值的最终次数非负, 只看总数不够
    cout << pt.max_xor({rt[3], branch}, {rt[1]}, 2) << '\n'; // {3,5,7,0} -> 7
    VLL xs{0, 2};
    cout << pt.kth_xor(rt[3], rt[1], xs, 2) << '\n'; // {0^5,0^7,2^5,2^7} 第2大=7
    cout << pt.kth_xor(rt[3], rt[1], xs, 5) << '\n'; // -1, k 越界
    // 树路径: 若 root[u] 存根到 u 的点值, 则 +root[u]+root[v]-root[lca]-root[fa(lca)]
    // 根的父亲使用空版本 0; 建树和 LCA 由调用方完成, 重复点值仍按出现次数计算

    // 字符串另建对象: 预算为总长度+插入次数, 空串也复制一个根
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
