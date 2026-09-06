// zoi: avl
#ifndef Z_OI_AVL
#define Z_OI_AVL

#include "../../杂项/utils/utils.cpp"

// 升序维护 LL 集合, 插/删/排名/第k小/前驱/后继 O(log n)
// 值域约定: 元素取值在 (-INF, INF) 内, 前驱/后继无解返回 ∓INF, 第k小越界返回 INF
// 内存: 每结点 32B; 预算 = 总插入次数(删除不回收), 4e6 结点 ≈ 128MB
struct AVL
{
    struct node
    {
        int lc = 0, rc = 0, cnt = 0, h = 0, sz = 0;
        LL val = 0;
    };
    vector<node> tr;
    int idx, root, budget;
    // 构造: 预算 max_nodes 结点(按累计插入计, 删除不回收), 哨兵 0 号就位
    // 时间: O(1) | 空间: O(预算)
    AVL(int max_nodes = 4000010) : idx(0), root(0), budget(max_nodes)
    {
        tr.reserve(budget + 1);
        tr.push_back(node());
    }
    // 插入 v (允许重复)
    // 时间: O(log n) | 额外空间: O(log n)
    void insert(LL v)
    {
        insert(root, v);
    }
    // 删除一个 v, 返回是否存在并删除
    // 时间: O(log n) | 额外空间: O(log n)
    bool erase(LL v)
    {
        int b = tr[root].sz;
        erase(root, v);
        return tr[root].sz < b;
    }
    // 返回 < v 的元素个数 (含重复)
    // 时间: O(log n) | 额外空间: O(log n)
    int get_rank(LL v)
    {
        return rank_of(root, v);
    }
    // 返回第 k 小 (1-based 含重复), k 越界返回 INF
    // 时间: O(log n) | 额外空间: O(log n)
    LL get_kth(int k)
    {
        if (k < 1 || k > tr[root].sz) return INF;
        return kth_of(root, k);
    }
    // 返回 < v 的最大值, 无前驱返回 -INF
    // 时间: O(log n) | 额外空间: O(log n)
    LL get_pre(LL v)
    {
        return pre_of(root, v);
    }
    // 返回 > v 的最小值, 无后继返回 INF
    // 时间: O(log n) | 额外空间: O(log n)
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
    // 时间: O(idx) | 空间: O(1)
    void clear()
    {
        idx = 0;
        root = 0;
        tr.clear();
        tr.push_back(node());
    }
private:
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
};
#endif

/* Usage:
int main()
{
    AVL s(16);
    for (LL v : {2LL, 2LL, 5LL}) s.insert(v);
    cout << s.get_rank(5) << " " << s.get_kth(2) << "\n"; // 2 2
    s.erase(2);                      // 只删一个 2
    cout << s.get_pre(5) << " " << s.get_suf(2) << "\n"; // 2 5
    s.clear();
    cout << s.size() << "\n"; // 0
}
*/
