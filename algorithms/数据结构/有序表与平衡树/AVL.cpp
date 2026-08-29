#ifndef Z_OI_AVL
#define Z_OI_AVL

#include <vector>
#include <cassert>
#include "../../杂项/utils/utils.cpp"

using namespace std;

// ============ AVL 平衡二叉搜索树 (可重复集合) ============
// 升序维护 LL 集合, 插/删/排名/第k小/前驱/后继 O(log n)
// 值域约定: 元素取值在 (-INF, INF) 内, 前驱/后继无解返回 ∓INF, 第k小越界返回 INF
// 内存账: 每结点 32B, 默认预算 4000010 结点(按累计插入计, 删除不回收)
//   ≈ 128MB, 超预算插入触发 assert
struct AVL
{
    static constexpr LL INF = 0x3f3f3f3f3f3f3f3f;
    struct node
    {
        int lc = 0, rc = 0, cnt = 0, h = 0, sz = 0;
        LL val = 0;
    };
private:
    vector<node> tr;
    int idx, root, budget;
    void pushup(int x)
    {
        if (!x) return;
        tr[x].h = max(tr[tr[x].lc].h, tr[tr[x].rc].h) + 1;
        tr[x].sz = tr[tr[x].lc].sz + tr[tr[x].rc].sz + tr[x].cnt;
    }
    void ro_left(int& x)
    {
        int y = tr[x].rc;
        tr[x].rc = tr[y].lc;
        tr[y].lc = x;
        pushup(x);
        pushup(y);
        x = y;
    }
    void ro_right(int& x)
    {
        int y = tr[x].lc;
        tr[x].lc = tr[y].rc;
        tr[y].rc = x;
        pushup(x);
        pushup(y);
        x = y;
    }
    void rotate(int& x)
    {
        if (tr[tr[x].lc].h - tr[tr[x].rc].h > 1)
        {
            if (tr[tr[tr[x].lc].lc].h >= tr[tr[tr[x].lc].rc].h) ro_right(x);
            else
            {
                ro_left(tr[x].lc);
                ro_right(x);
            }
        }
        else if (tr[tr[x].lc].h - tr[tr[x].rc].h < -1)
        {
            if (tr[tr[tr[x].rc].rc].h >= tr[tr[tr[x].rc].lc].h) ro_left(x);
            else
            {
                ro_right(tr[x].rc);
                ro_left(x);
            }
        }
    }
    void insert(int& x, LL v)
    {
        if (!x)
        {
            assert(idx < budget);
            tr.push_back(node());
            x = ++idx;
            tr[x].cnt = tr[x].h = tr[x].sz = 1;
            tr[x].val = v;
            return;
        }
        if (tr[x].val == v) tr[x].cnt++;
        else if (tr[x].val < v) insert(tr[x].rc, v);
        else insert(tr[x].lc, v);
        pushup(x);
        rotate(x);
    }
    void erase(int& x, LL v)
    {
        if (!x) return;
        if (tr[x].val == v)
        {
            if (tr[x].cnt > 1) tr[x].cnt--;
            else
            {
                if (!tr[x].lc || !tr[x].rc) x = tr[x].lc + tr[x].rc;
                else
                {
                    int y = tr[x].lc;
                    while (tr[y].rc) y = tr[y].rc;
                    tr[x].cnt = tr[y].cnt;
                    tr[x].val = tr[y].val;
                    tr[y].cnt = 0;
                    erase(tr[x].lc, tr[y].val);
                }
            }
        }
        else
        {
            if (tr[x].val < v) erase(tr[x].rc, v);
            else erase(tr[x].lc, v);
        }
        pushup(x);
        rotate(x);
    }
    int rank_of(int x, LL v)
    {
        if (!x) return 0;
        if (tr[x].val >= v) return rank_of(tr[x].lc, v);
        return tr[x].cnt + tr[tr[x].lc].sz + rank_of(tr[x].rc, v);
    }
    LL kth_of(int x, int k)
    {
        if (tr[tr[x].lc].sz >= k) return kth_of(tr[x].lc, k);
        if (tr[tr[x].lc].sz + tr[x].cnt >= k) return tr[x].val;
        return kth_of(tr[x].rc, k - tr[tr[x].lc].sz - tr[x].cnt);
    }
    LL pre_of(int x, LL v)
    {
        if (!x) return -INF;
        if (tr[x].val >= v) return pre_of(tr[x].lc, v);
        return max(tr[x].val, pre_of(tr[x].rc, v));
    }
    LL suf_of(int x, LL v)
    {
        if (!x) return INF;
        if (tr[x].val <= v) return suf_of(tr[x].rc, v);
        return min(tr[x].val, suf_of(tr[x].lc, v));
    }
public:
    // 构造: 预算 max_nodes 结点(按累计插入计, 删除不回收), 哨兵 0 号就位
    // 时间: O(1) | 空间: O(预算) (账目见类头)
    AVL(int max_nodes = 4000010) : idx(0), root(0), budget(max_nodes)
    {
        tr.reserve(budget + 1);
        tr.push_back(node());
    }
    // 插入 v (允许重复)
    // 时间: O(log n) | 空间: O(1)
    void insert(LL v)
    {
        insert(root, v);
    }
    // 删除一个 v, 返回是否存在并删除
    // 时间: O(log n) | 空间: O(1)
    bool erase(LL v)
    {
        int b = tr[root].sz;
        erase(root, v);
        return tr[root].sz < b;
    }
    // 返回 < v 的元素个数 (含重复)
    // 时间: O(log n) | 空间: O(1)
    int get_rank(LL v)
    {
        return rank_of(root, v);
    }
    // 返回第 k 小 (1-based 含重复), k 越界返回 INF
    // 时间: O(log n) | 空间: O(1)
    LL get_kth(int k)
    {
        if (k < 1 || k > tr[root].sz) return INF;
        return kth_of(root, k);
    }
    // 返回 < v 的最大值, 无前驱返回 -INF
    // 时间: O(log n) | 空间: O(1)
    LL get_pre(LL v)
    {
        return pre_of(root, v);
    }
    // 返回 > v 的最小值, 无后继返回 INF
    // 时间: O(log n) | 空间: O(1)
    LL get_suf(LL v)
    {
        return suf_of(root, v);
    }
    // 返回元素个数 (含重复)
    // 时间: O(1) | 空间: O(1)
    int size()
    {
        return tr[root].sz;
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
};
#endif
/* Usage:
    AVL avl;                       // 默认预算 4e6 结点
    avl.insert(x);                 // 允许重复
    avl.erase(x);                  // 删一个, 返回 bool
    avl.get_rank(x);               // <x 的元素个数(含重复)
    avl.get_kth(k);                // 第 k 小, 越界返回 INF
    avl.get_pre(x);                // 严格前驱, 无则 -INF
    avl.get_suf(x);                // 严格后继, 无则 INF
    avl.size();                    // 元素个数(含重复)
    avl.clear();                   // 多测复位, 容量保留
*/
