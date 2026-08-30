// zoi: trie
#ifndef Z_OI_TRIE
#define Z_OI_TRIE

#include <array>
#include <cassert>
#include <string>
#include <vector>
#include "../../杂项/utils/utils.cpp"

using namespace std;

// ============ 字典树 (前缀计数 + 单词计数) ============
// 字符集 K: 26 = 小写 | 62 = a-z A-Z 0-9 | <=10 = 数字(含 01-Trie)
// 内存: 每结点 (4K + 8) 字节; max_nodes = 结点总数上限(含根) = 插入串总长 + 1
// 契约: 输入字符必须落在声明的字符集内, 越界行为未定义
template <int K = 26>
struct Trie
{
    struct Node
    {
        array<int, K> ch{};
        int p_cnt = 0, w_cnt = 0;
    };
    vector<Node> tr;
    int cap;
    Trie(int max_nodes = 1000010) : cap(max_nodes)
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
    int new_node()
    {
        assert((int)tr.size() + 1 <= cap && "max_nodes 开小了");
        tr.push_back(Node{});
        return (int)tr.size() - 1;
    }
    int walk(const string& s)
    {
        int cur = 0;
        for (char c : s)
        {
            cur = tr[cur].ch[to_id(c)];
            if (!cur) return -1;
        }
        return cur;
    }
public:
    // 插入单词 s
    // 时间: O(|s|) | 空间: 至多 |s| 个新结点
    void insert(const string& s)
    {
        tr[0].p_cnt++;
        int cur = 0;
        for (char c : s)
        {
            int id = to_id(c);
            if (!tr[cur].ch[id]) tr[cur].ch[id] = new_node();
            cur = tr[cur].ch[id];
            tr[cur].p_cnt++;
        }
        tr[cur].w_cnt++;
    }
    // 插入非负整数 x (按 LL 64 位定宽, 高位在前) —— 01-Trie 专用, 仅 K ∈ [2, 10] 编译
    // 时间: O(64) | 空间: 至多 64 个新结点
    void insert_num(LL x)
    {
        static_assert(K >= 2 && K <= 10, "insert_num 仅数字字符集(K<=10)可用");
        insert_walk(x);
    }
private:
    void insert_walk(LL x)
    {
        tr[0].p_cnt++;
        int cur = 0;
        for (int i = 63; i >= 0; i--)
        {
            int id = (x >> i) & 1;
            if (!tr[cur].ch[id]) tr[cur].ch[id] = new_node();
            cur = tr[cur].ch[id];
            tr[cur].p_cnt++;
        }
        tr[cur].w_cnt++;
    }
public:
    // 查询以 s 为前缀的已插入单词个数
    // 时间: O(|s|) | 空间: O(1)
    int count_prefix(const string& s)
    {
        int u = walk(s);
        return u == -1 ? 0 : tr[u].p_cnt;
    }
    // 查询单词 s 的重复插入次数
    // 时间: O(|s|) | 空间: O(1)
    int count_word(const string& s)
    {
        int u = walk(s);
        return u == -1 ? 0 : tr[u].w_cnt;
    }
    // 查询已插入整数中与 x 异或的最大值 —— 01-Trie 专用, 仅 K ∈ [2, 10] 编译; 空树返回 -1
    // 时间: O(64) | 空间: O(1)
    LL max_xor(LL x)
    {
        static_assert(K >= 2 && K <= 10, "max_xor 仅 K <= 10 可用");
        if (tr[0].p_cnt == 0) return -1;
        LL res = 0;
        int cur = 0;
        for (int i = 63; i >= 0; i--)
        {
            int want = ((x >> i) & 1) ^ 1;
            if (tr[cur].ch[want]) { res |= 1LL << i; cur = tr[cur].ch[want]; }
            else cur = tr[cur].ch[want ^ 1];
        }
        return res;
    }
    // 多测清空
    // 时间: O(Used) | 空间: O(1)
    void clear()
    {
        for (auto& nd : tr) nd = Node{};
        tr.resize(1);
    }
};
#endif

/*
 * Usage:
 * int T; cin >> T;
 * while (T--)
 * {
 *     int n, q; cin >> n >> q;
 *     static Trie<62> trie(600010);    // 预算 = 插入串总长 + 1
 *     trie.clear();                    // 多测清空
 *     for (int i = 1; i <= n; i++) { string s; cin >> s; trie.insert(s); }
 *     while (q--) { string s; cin >> s; cout << trie.count_prefix(s) << '\n'; }
 * }
 * static Trie<2> bt(64 * N + 10);      // 01-Trie: 预算 = 个数 * 64
 * bt.insert_num(x);
 * bt.max_xor(y);                       // 与 y 异或的最大值; 空树 -1
 */
