#include <vector>
#include <algorithm>
#include <iostream>

using namespace std;

using VI = vector<int>;
using VVI = vector<vector<int>>;

template<typename... Cs>
void z_fill_n(int n, int val, Cs&... containers) 
{
    (fill(containers.begin(), containers.begin() + min((size_t)(n + 10), containers.size()), val), ...);
}
struct VBCC
{
    int n, m;
    int dfn_idx, vbcc_cnt, edge_cnt, t_edge_cnt;
    
    VI head, to, nxt;
    VI t_head, t_to, t_nxt;
    VI dfn, low, sta;
    VI cut;             // 标记节点是否为割点
    VVI vbcc_cir;       // 存储每个 VBCC 所包含的所有圆点。ans[i] 为第 i 个 VBCC 的节点集合
    VBCC(int _n, int _m) : n(_n), m(_m), 
        head(n + 10, 0), to(m + 10, 0), nxt(m + 10, 0), 
        // 圆方树最多有 n 个圆点 + n 个方点，无向边数量翻倍
        t_head(n * 2 + 10, 0), t_to(n * 4 + 10, 0), t_nxt(n * 4 + 10, 0), 
        dfn(n + 10, 0), low(n + 10, 0), cut(n + 10, 0), 
        vbcc_cir(1, VI{})
    {
        dfn_idx = vbcc_cnt = t_edge_cnt = 0;
        edge_cnt = 1;
    }

    void init(int _n)
    {
        n = _n;
        z_fill_n(n * 2, 0, t_head); // t_head 范围是 2n
        z_fill_n(n, 0, head, dfn, low, cut);
        dfn_idx = vbcc_cnt = t_edge_cnt = 0;
        edge_cnt = 1;
        
        sta.clear();
        vbcc_cir.assign(1, VI{});
    }
    
    void add_edge(int u, int v)
    {
        edge_cnt++;
        to[edge_cnt] = v;
        nxt[edge_cnt] = head[u];
        head[u] = edge_cnt;
    }

    void t_add_edge(int u, int v)
    {
        t_edge_cnt++;
        t_to[t_edge_cnt] = v;
        t_nxt[t_edge_cnt] = t_head[u];
        t_head[u] = t_edge_cnt;
    }

    void tarjan(int u, int root)
    {
        dfn_idx++;
        dfn[u] = low[u] = dfn_idx;
        sta.push_back(u);
        int child_cnt = 0;
        for (int i = head[u]; i; i = nxt[i])
        {
            int v = to[i];
            if (!dfn[v])
            {
                child_cnt++;
                tarjan(v, root);
                low[u] = min(low[u], low[v]);
                if (low[v] >= dfn[u])
                {
                    if (u != root)
                    {
                        cut[u] = 1; 
                    }
                    vbcc_cnt++;
                    vbcc_cir.push_back(VI{});
                    int t;
                    do
                    {
                        t = sta.back(); 
                        sta.pop_back();
                        vbcc_cir[vbcc_cnt].push_back(t);
                    } while (t != v); 
                    vbcc_cir[vbcc_cnt].push_back(u); 
                }
            }
            else
            {
                low[u] = min(low[u], dfn[v]);
            }
        }
        
        if (u == root && child_cnt >= 2)
        {
            cut[u] = 1; 
        }
        // 处理没有任何子节点的根节点（孤立点 / 单点自环）
        if (u == root && child_cnt == 0)
        {
            vbcc_cnt++;
            vbcc_cir.push_back(VI{u});
        }
    }

    void build(int root = -1)
    {
        if (root != -1)
        {
            tarjan(root, root);
            return;
        }
        for (int i = 1; i <= n; i++)
        {
            if (!dfn[i])
            {
                tarjan(i, i);
            }
        }
    }

    // 构建圆方树
    void build_tree()
    {
        for (int i = 1; i <= vbcc_cnt; i++)
        {
            int u = n + i;
            for (int v : vbcc_cir[i])
            {
                t_add_edge(u, v);
                t_add_edge(v, u);
            }
        }
    }

    // 返回单个割点参与的 VBCC 列表
    VI get_bel_vbccs(int u)
    {
        VI res;
        for (int i = t_head[u]; i; i = t_nxt[i])
        {
            int v = t_to[i];        // v 是方点，编号在 n + 1 到 n + vbcc_cnt 之间
            res.push_back(v - n);   // 减去 n 就是真正的 VBCC 编号
        }
        return res;
    }

    // 返回单个 VBCC 包含的割点列表
    VI get_cuts_vbcc(int i)
    {
        VI res;
        if (i < 1 || i > vbcc_cnt) return res;
        for (int v : vbcc_cir[i]) 
        {
            if (cut[v]) res.push_back(v);
        }
        return res;
    }
};

/*
 * =====================================================================================
 * Block-Cut Tree (圆方树) 
 * 
 * [拓扑结构]
 * - 圆点 (Round) : 原图节点，编号 1 ~ n。
 * - 方点 (Square): VBCC，编号 n+1 ~ n+vbcc_cnt。
 * - 边权性质     : 圆方树为二分图。连边必为 (圆, 方)，无 (圆, 圆) 或 (方, 方)。
 * - 空间警告     : 节点总数极值 2N-1 (原图为树时)，所有树上数组(head, sz, fa) 必开 2 倍！
 * 
 * [连通与转化]
 * - 割点判定 : 圆点度数 >= 2  <=> 该点为割点。
 * - 必经点   : 原图 u->v 路径上的所有必经点 == 圆方树 u->v 简单路径上的所有【圆点】。
 * - 割边性质 : 原图中的孤立割边，对应大小为 2 的 VBCC (2个圆点 + 1个方点)。
 * - 内部路径 : 大小 >=3 的 VBCC 内，任取三点 a,b,c，必存在简单路径 a -> b -> c。
 * 
 * [树上维护高频 Trick (树剖 / LCT)]
 * - 菊花图退化 : 若用方点维护其包含的所有圆点，改圆点时更新周围方点会导致 O(N) 退化。
 * - 【标准解法】: 方点仅维护其在圆方树上的【子节点(圆点)】信息。
 * - 【LCA 特判】: 树上查询 u->v 时，若 LCA 为方点，必须额外并入 LCA 的父节点(圆点)贡献！
 * =====================================================================================
 */