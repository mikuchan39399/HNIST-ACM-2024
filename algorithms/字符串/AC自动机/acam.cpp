// zoi: acam
#ifndef Z_OI_ACAM
#define Z_OI_ACAM
#include "../../杂项/utils/utils.cpp"

// ACAM, K=26 小写 / 62 大小写及数字 / 1..10 前 K 个数字, 字符须合法
// 模式表 1-based, string 不补位; tr 根为 0, pos[i] 为模式终点, build 后只读
// 文本长度 < INT_MAX, 次数用 LL; 模式与点数须在 int 容量内
// 每轮点池含根, 按总模式长度+1 预算; 每点 4K+4 字节, BFS 表另占 4 字节/点
template<int K = 26>
struct ACAM
{
    static_assert((K >= 1 && K <= 10) || K == 26 || K == 62, "unsupported alphabet");
    struct Node
    {
        array<int, K> ch{};
        int fail = 0;
    };
    vector<Node> tr;
    VI pos, q;
    int cap;
    // 默认空模式表; 预留 max_nodes 点, 时间 O(1), 空间 O(K*max_nodes)
    ACAM(int max_nodes = 1000010) : cap(max_nodes)
    {
        assert(max_nodes >= 1);
        tr.reserve(max_nodes);
        q.reserve(max_nodes);
        clear();
    }
    // 丢弃模式并保留容量, 旧点号失效; 时间 O(已用点数+模式数) 上界
    void clear()
    {
        tr.clear();
        tr.push_back(Node{});
        pos.assign(1, 0);
        q.clear();
        q.push_back(0);
    }
    // 重建模式表, s[0] 占位, {} 也表示无模式; 总长 L/点数 S, 时间 O(L+K*S+模式数), 空间 O(S+模式数)
    void build(const vector<string>& s)
    {
        tr.reserve(cap);
        q.reserve(cap);
        clear();
        int m = int(s.size());
        pos.resize(max(1, m));
        for (int i = 1; i < m; i++)
        {
            int u = 0;
            for (char c : s[i])
            {
                int id = to_id(c);
                if (!tr[u].ch[id]) tr[u].ch[id] = new_node();
                u = tr[u].ch[id];
            }
            pos[i] = u;
        }
        int n = int(tr.size());
        for (int i = 0; i < n; i++)
        {
            int u = q[i];
            for (int c = 0; c < K; c++)
            {
                int v = tr[u].ch[c];
                if (v)
                {
                    tr[v].fail = u ? tr[tr[u].fail].ch[c] : 0;
                    q.push_back(v);
                }
                else tr[u].ch[c] = tr[tr[u].fail].ch[c];
            }
        }
    }
    // 返回各模式出现次数, [0]=0, 含重叠/重复, 空模式 |s|+1; 时间 O(|s|+S+模式数), 额外空间 O(S+模式数)
    VLL query(const string& s) const
    {
        VLL cnt(tr.size(), 0), ans(pos.size(), 0);
        cnt[0] = 1;
        int u = 0;
        for (char c : s)
        {
            u = tr[u].ch[to_id(c)];
            cnt[u]++;
        }
        for (int i = int(q.size()); i > 1; i--)
        {
            int v = q[i - 1];
            cnt[tr[v].fail] += cnt[v];
        }
        int m = int(pos.size());
        for (int i = 1; i < m; i++) ans[i] = cnt[pos[i]];
        return ans;
    }
private:
    static int to_id(char c)
    {
        if (K == 62)
        {
            if (c >= '0' && c <= '9') return c - '0' + 52;
            if (c >= 'A' && c <= 'Z') return c - 'A' + 26;
        }
        return c - (K <= 10 ? '0' : 'a');
    }
    int new_node()
    {
        assert(tr.size() < size_t(cap));
        tr.push_back(Node{});
        return int(tr.size()) - 1;
    }
};
#endif

/* Usage
#include <acam.h>
int main()
{
    vector<string> p = {"", "a", "aa", "a", ""}; // p[0] 占位, p[4] 是空模式
    ACAM<> ac(1 + 1 + 2 + 1);
    ac.build(p);
    auto ans = ac.query("aaa");
    for (int i = 1, m = int(ans.size()); i < m; i++) cout << ans[i] << ' '; // 3 2 3 4
    cout << endl;
    cout << ac.query("b")[2] << '\n'; // 0, 查询不累积
    ac.build({"", "ba"});             // 多测直接重建, 不先 clear
    cout << ac.query("baba")[1] << '\n'; // 2
}
*/
