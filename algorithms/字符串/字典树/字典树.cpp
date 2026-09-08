// zoi: trie
#ifndef Z_OI_TRIE
#define Z_OI_TRIE

#include "../../杂项/utils/utils.cpp"

using namespace std;

// ============ 字典树 (前缀计数 + 单词计数) ============
// 字符集 K: 26 = 小写 | 62 = a-z A-Z 0-9 | <=10 = 数字(含 01-Trie)
// 每结点 (4K+8)B; max_nodes 包含 0 号根, 按累计插入串总长+1 预留, K=26 时 1e6 约 112MB
// 契约: 输入字符必须落在声明的字符集内, 越界行为未定义
// 支持重复值、空串; 不支持删除。同一对象不混用字符串和整数接口
// 整数及查询 x 在 [0, LLONG_MAX], 元素总数 <= INT_MAX; clear 后旧结点号失效
template <int K = 26>
struct Trie
{
    static_assert(K > 0);
    struct Node
    {
        array<int, K> ch{};
        int p_cnt = 0, w_cnt = 0;
    };
    vector<Node> tr;
    int cap;
    // 预留 max_nodes 个结点, 包含已创建的 0 号根
    // 时间: O(1) | 空间: max_nodes*sizeof(Node) 字节预留
    Trie(int max_nodes = 1000010) : cap(max_nodes)
    {
        assert(max_nodes >= 1);
        tr.reserve(max_nodes);
        tr.push_back(Node{});
    }
    // 走字符串 s 对应的结点, 返回结点号; 中途断链返回 -1
    // 时间: O(|s|) | 空间: O(1)
    int walk(const string& s) const
    {
        int cur = 0;
        for (char c : s)
        {
            cur = tr[cur].ch[to_id(c)];
            if (!cur) return -1;
        }
        return cur;
    }
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
    // 查询以 s 为前缀的已插入单词个数
    // 时间: O(|s|) | 空间: O(1)
    int count_prefix(const string& s) const
    {
        int u = walk(s);
        return u == -1 ? 0 : tr[u].p_cnt;
    }
    // 查询单词 s 的重复插入次数
    // 时间: O(|s|) | 空间: O(1)
    int count_word(const string& s) const
    {
        int u = walk(s);
        return u == -1 ? 0 : tr[u].w_cnt;
    }
    // 查询已插入整数中与 x 异或的最大值 —— 01-Trie 专用, 仅 K ∈ [2, 10] 编译; 空树返回 -1
    // 时间: O(64) | 空间: O(1)
    LL max_xor(LL x) const
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
        tr.clear();
        tr.push_back(Node{});
    }
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
    int new_node()
    {
        assert(tr.size() < (size_t)cap && "max_nodes 开小了");
        tr.push_back(Node{});
        return (int)tr.size() - 1;
    }
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
};
#endif


/* Usage
#include <trie.h>
int main()
{
    // 字符串: K=26 小写, K=62 大小写及数字, K<=10 为前 K 个数字字符
    // 预算包含根: 1 + 所有插入串的长度之和; 重复串与公共前缀会省结点
    Trie<62> words(1 + 3 + 3 + 5);
    for (string s : {"Ab1", "Ab1", "Ab123", ""}) words.insert(s);
    cout << words.count_word("Ab1") << ' ' << words.count_prefix("Ab") << '\n'; // 2 3
    cout << words.count_word("") << ' ' << words.count_prefix("") << '\n';    // 1 4
    int u = words.walk("Ab1");             // 断链 -1, 空串返回根 0
    cout << words.tr[u].w_cnt << '\n';     // 2
    words.clear();                        // 多测复用; 旧结点号不能再使用
    cout << words.count_prefix("") << '\n'; // 0

    // 整数另建对象, 不和变长字符串混用; 0<=x<=LLONG_MAX, 每次最多开 64 个点
    Trie<2> nums(1 + 64 * 3);
    for (LL x : {3LL, 5LL, 5LL}) nums.insert_num(x);
    cout << nums.max_xor(2) << '\n';        // 7, 返回异或结果, 不是被选中的原数
    nums.clear();
    cout << nums.max_xor(2) << '\n';        // -1, 空集合
}
*/
