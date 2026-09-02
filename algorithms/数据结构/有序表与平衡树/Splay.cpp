// zoi: splay
#ifndef Z_OI_SPLAY
#define Z_OI_SPLAY

#include <vector>
#include <cassert>
#include "../../杂项/utils/utils.cpp"

using namespace std;

// ============ Splay 伸展树 (可重复集合) ============
// 升序维护 LL 集合, 插/删/排名/第k小/前驱/后继均摊 O(log n)
// 值域约定: 元素取值在 (-INF, INF) 内, 前驱/后继无解返回 ∓INF, 第k小越界返回 INF
// 机制: 访问即伸展 —— 每次操作把触到的结点 splay 到根; 双旋(父子同侧先旋父)
//   把访问路径对折, 均摊 O(log n); erase 用"前驱旋到根旁"接右子的习语
// 内存: 每结点 24B, 预算 = 总插入次数(删除不回收), 4e6 ≈ 96MB
struct Splay
{
    static constexpr LL INF = 0x3f3f3f3f3f3f3f3f;
    struct node
    {
        int ch[2];
        int fa, sz;
        LL val;
    };
    vector<node> tr;
    int idx, root, budget;
    Splay(int max_nodes = 4000010) : idx(0), root(0), budget(max_nodes)
    {
        tr.reserve(budget + 1);
        tr.push_back(node());
    }
    // 插入 v (允许重复)
    // 时间: 均摊 O(log n) | 空间: O(1)
    void insert(LL v)
    {
        int cur = root, f = 0;
        while (cur)
        {
            f = cur;
            cur = tr[cur].ch[v > tr[cur].val];
        }
        int x = newnode(v);
        tr[x].fa = f;
        if (f) tr[f].ch[v > tr[f].val] = x;
        else root = x;
        splay(x);
    }
    // 删除一个 v, 返回是否存在并删除
    // 时间: 均摊 O(log n) | 空间: O(1)
    bool erase(LL v)
    {
        int x = find_val(v);
        if (!x) return false;
        if (!tr[x].ch[0])
        {
            root = tr[x].ch[1];
            tr[root].fa = 0;
            return true;
        }
        int p = tr[x].ch[0];
        while (tr[p].ch[1]) p = tr[p].ch[1];
        splay(p, x);
        int rc = tr[x].ch[1];
        tr[p].ch[1] = rc;
        if (rc) tr[rc].fa = p;
        root = p;
        tr[p].fa = 0;
        pushup(p);
        return true;
    }
    // 返回 < v 的元素个数 (含重复)
    // 时间: 均摊 O(log n) | 空间: O(1)
    int get_rank(LL v)
    {
        int cur = root, last = 0, ret = 0;
        while (cur)
        {
            last = cur;
            if (tr[cur].val >= v) cur = tr[cur].ch[0];
            else
            {
                ret += tr[tr[cur].ch[0]].sz + 1;
                cur = tr[cur].ch[1];
            }
        }
        if (last) splay(last);
        return ret;
    }
    // 返回第 k 小 (1-based 含重复), k 越界返回 INF
    // 时间: 均摊 O(log n) | 空间: O(1)
    LL get_kth(int k)
    {
        if (k < 1 || k > tr[root].sz) return INF;
        int x = find(k);
        splay(x);
        return tr[x].val;
    }
    // 返回 < v 的最大值, 无前驱返回 -INF
    // 时间: 均摊 O(log n) | 空间: O(1)
    LL get_pre(LL v)
    {
        int cur = root, hit = 0;
        while (cur)
        {
            if (tr[cur].val < v) { hit = cur; cur = tr[cur].ch[1]; }
            else cur = tr[cur].ch[0];
        }
        if (!hit) return -INF;
        splay(hit);
        return tr[hit].val;
    }
    // 返回 > v 的最小值, 无后继返回 INF
    // 时间: 均摊 O(log n) | 空间: O(1)
    LL get_suf(LL v)
    {
        int cur = root, hit = 0;
        while (cur)
        {
            if (tr[cur].val > v) { hit = cur; cur = tr[cur].ch[0]; }
            else cur = tr[cur].ch[1];
        }
        if (!hit) return INF;
        splay(hit);
        return tr[hit].val;
    }
    // 返回元素个数 (含重复)
    // 时间: O(1) | 空间: O(1)
    int size() { return tr[root].sz; }
    // 从升序 a[1..m] 完美平衡二分建树, 替换现有集合 (a.size() = m + 1)
    // 契约: a[1..m] 已升序(允许重复), 违约触发 assert
    // 时间: O(m) | 空间: 递归栈 O(log m)
    void build(const VLL& a)
    {
        clear();
        int m = (int)a.size() - 1;
        for (int i = 2; i <= m; i++) assert(a[i - 1] <= a[i]);
        root = build_range(a, 1, m, 0);
    }
    // 多测复位: 清全部元素, 容量保留
    // 时间: O(1) | 空间: O(1)
    void clear()
    {
        idx = 0;
        root = 0;
        tr.clear();
        tr.push_back(node());
    }
private:
    int newnode(LL v)
    {
        assert(idx < budget);
        tr.push_back(node());
        int x = ++idx;
        tr[x].sz = 1;
        tr[x].val = v;
        return x;
    }
    void pushup(int x)
    {
        tr[x].sz = tr[tr[x].ch[0]].sz + tr[tr[x].ch[1]].sz + 1;
    }
    // x 是 fa[x] 的哪个孩子: 0 左 1 右
    bool get(int x) { return x == tr[tr[x].fa].ch[1]; }
    // 把 x 向上转一格, 左旋右旋合一(k = get(x) 定方向)
    void rotate(int x)
    {
        int y = tr[x].fa, z = tr[y].fa;
        int k = get(x);
        tr[y].ch[k] = tr[x].ch[k ^ 1];
        if (tr[x].ch[k ^ 1]) tr[tr[x].ch[k ^ 1]].fa = y;
        tr[x].ch[k ^ 1] = y;
        tr[y].fa = x;
        tr[x].fa = z;
        if (z) tr[z].ch[y == tr[z].ch[1]] = x;
        pushup(y);
        pushup(x);
    }
    // 把 x 双旋到 goal 的孩子位, goal = 0 代表旋到根
    // 同侧(get 相同)先旋父再旋己, 异侧连旋两次己
    void splay(int x, int goal = 0)
    {
        while (tr[x].fa != goal)
        {
            int y = tr[x].fa, z = tr[y].fa;
            if (z != goal)
            {
                if (get(x) == get(y)) rotate(y);
                else rotate(x);
            }
            rotate(x);
        }
        if (!goal) root = x;
    }
    // 中序排名为 rnk 的结点 (1-based), rnk 保证合法
    int find(int rnk)
    {
        int cur = root;
        while (cur)
        {
            if (tr[tr[cur].ch[0]].sz + 1 == rnk) return cur;
            else if (tr[tr[cur].ch[0]].sz >= rnk) cur = tr[cur].ch[0];
            else
            {
                rnk -= tr[tr[cur].ch[0]].sz + 1;
                cur = tr[cur].ch[1];
            }
        }
        return 0;
    }
    // 值 v 的结点: 找到则 splay 到根并返回; 找不到 splay 最后触点返回 0
    int find_val(LL v)
    {
        int cur = root, last = 0;
        while (cur)
        {
            if (tr[cur].val == v)
            {
                splay(cur);
                return cur;
            }
            last = cur;
            cur = tr[cur].ch[v > tr[cur].val];
        }
        if (last) splay(last);
        return 0;
    }
    // [l, r] 完美平衡二分建子树挂 f 下, 返回子树根
    int build_range(const VLL& a, int l, int r, int f)
    {
        if (l > r) return 0;
        int mid = (l + r) >> 1;
        int x = newnode(a[mid]);
        tr[x].fa = f;
        tr[x].ch[0] = build_range(a, l, mid - 1, x);
        tr[x].ch[1] = build_range(a, mid + 1, r, x);
        pushup(x);
        return x;
    }
};
#endif
/*
 * Usage:
 * Splay sp;                        // 默认预算 4e6 结点
 * sp.build(a);                     // 升序 a[1..m] O(n) 建树(替换现有)
 * sp.insert(x);                    // 允许重复
 * sp.erase(x);                     // 删一个, 返回 bool
 * sp.get_rank(x);                  // <x 的元素个数(含重复)
 * sp.get_kth(k);                   // 第 k 小, 越界返回 INF
 * sp.get_pre(x);                   // 严格前驱, 无则 -INF
 * sp.get_suf(x);                   // 严格后继, 无则 INF
 * sp.size();                       // 元素个数(含重复)
 * sp.clear();                      // 多测复位, 容量保留
 * // P_3369: 排名查询输出 get_rank(x) + 1; 第 k 小输出 get_kth(k)
 */

