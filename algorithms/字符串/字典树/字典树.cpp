// zoi: trie
#ifndef Z_OI_TRIE
#define Z_OI_TRIE

#include "../../杂项/utils/utils.cpp"

using namespace std;

// Trie, K=26 小写 / 62 大小写及数字 / <=10 前 K 个数字; 字符须合法
// 可重复/空串, 不删除, 字符串与整数接口不混用; 整数为非负 LL, 总数不超 INT_MAX
// 点池含根, 按总插入长度+1 预算, 每点 4K+8 字节; clear 后旧编号失效
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
    // 预留 max_nodes 点(含根 0). 时间 O(1), 空间 O(K*max_nodes)
    Trie(int max_nodes = 1000010) : cap(max_nodes)
    {
        assert(max_nodes >= 1);
        tr.reserve(max_nodes);
        tr.push_back(Node{});
    }
    // s 对应的点号, 断链 -1, 空串 0. 时间 O(|s|), 空间 O(1)
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
    // 插入 s. 时间/新增点 O(|s|)
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
    // 插入非负 LL, 仅 2<=K<=10; 时间 O(64), 至多 64 个新点
    void insert_num(LL x)
    {
        static_assert(K >= 2 && K <= 10, "insert_num 仅数字字符集(K<=10)可用");
        insert_walk(x);
    }
    // 前缀为 s 的单词数. 时间 O(|s|), 空间 O(1)
    int count_prefix(const string& s) const
    {
        int u = walk(s);
        return u == -1 ? 0 : tr[u].p_cnt;
    }
    // s 的出现次数. 时间 O(|s|), 空间 O(1)
    int count_word(const string& s) const
    {
        int u = walk(s);
        return u == -1 ? 0 : tr[u].w_cnt;
    }
    // 与 x 异或的最大值(非原数), 空树 -1; 仅 2<=K<=10. 时间 O(64), 空间 O(1)
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
    // 清空并保留容量. 时间 O(已用点数) 上界
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

    Trie<62> words(1 + 3 + 3 + 5);
    for (string s : {"Ab1", "Ab1", "Ab123", ""}) words.insert(s);
    cout << words.count_word("Ab1") << ' ' << words.count_prefix("Ab") << '\n'; // 2 3
    cout << words.count_word("") << ' ' << words.count_prefix("") << '\n';    // 1 4
    int u = words.walk("Ab1");             // 断链 -1, 空串返回根 0
    cout << words.tr[u].w_cnt << '\n';     // 2
    words.clear();                        // 多测复用; 旧结点号不能再使用
    cout << words.count_prefix("") << '\n'; // 0

    // 整数另建对象
    Trie<2> nums(1 + 64 * 3);
    for (LL x : {3LL, 5LL, 5LL}) nums.insert_num(x);
    cout << nums.max_xor(2) << '\n';        // 7, 返回异或结果, 不是被选中的原数
    nums.clear();
    cout << nums.max_xor(2) << '\n';        // -1, 空集合
}
*/
