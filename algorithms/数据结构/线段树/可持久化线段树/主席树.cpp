// zoi: persistentSeg
#ifndef Z_OI_PERS_SEG_TREE
#define Z_OI_PERS_SEG_TREE

#include "../../../杂项/utils/utils.cpp"

// 值域 [1,n], 根外置, 0 为全零树; Info{} 补 len, 仅支持加法类可交换 Tag
// pred 表示区间内存在合格点; 查询/find 不开点
// 池按累计新建计: build 2n-1, 每次点改至多 ceil(log2 n)+1, 区间改至多 4ceil(log2 n)+1
template<class Info, class Tag>
struct PersSegTree
{
    struct Node
    {
        int lc = 0, rc = 0;
        Info info;
        Tag tag;
    };
    LL n;
    int tot = 0;
    int cap = 2;
    vector<Node> tr;
    // 预留 max_nodes 个结点, 值域 [1,max_n]; 时间 O(1), 空间 O(max_nodes)
    PersSegTree(LL max_n = 1, int max_nodes = 4000010) : n(max_n)
    {
        cap = max_nodes + 1;
        tr.reserve(max_nodes + 1);
        tr.push_back(Node{});
    }
    // 清空全部版本, 旧根失效; 时间 O(tot), 空间 O(1)
    void clear()
    {
        tot = 0;
        tr.clear();
        tr.push_back(Node{});
    }
    // 只改值域, 改后停用旧根; 时间/空间 O(1)
    void set_n(LL _n) { n = _n; }
    // 非空 a[1..m] 建新根, 设 n=m, 不清池; 时间/新增空间 O(m)
    int build(const vector<Info>& a)
    {
        assert((int)a.size() >= 2);
        n = (LL)a.size() - 1;
        return build(1, n, a);
    }
    // 修改 rt 的 [x,y], 返回新根; 时间/新增空间 O(log n)
    int modify(int rt, LL x, LL y, const Tag& v)
    {
        assert(1 <= x && x <= y && y <= n);
        return modify(rt, 1, n, x, y, v);
    }
    // 查询 rt 的 [x,y]; 时间/栈 O(log n)
    Info query(int rt, LL x, LL y) { return query(rt, 1, n, x, y, Tag{}); }
    // [start,n] 最左合格位置, 无解/越界 -1; 时间/栈 O(log n)
    template<class Pred>
    LL find_first(int rt, LL start, Pred pred)
    {
        if (start < 1 || start > n) return -1;
        return find_first(rt, 1, n, start, Tag{}, pred);
    }
    // [1,end] 最右合格位置, 无解/越界 -1; 时间/栈 O(log n)
    template<class Pred>
    LL find_last(int rt, LL end, Pred pred)
    {
        if (end < 1 || end > n) return -1;
        return find_last(rt, 1, n, end, Tag{}, pred);
    }
    // Σplus-Σminus 的第 k 小值域下标; 仅点改计数版本, 逐点差非负, k 合法
    // R=根数; 时间 O(R log n), 额外空间 O(R)
    LL find_kth(VI plus, VI minus, LL k)
    {
        LL l = 1, r = n;
        while (l < r)
        {
            LL mid = l + (r - l) / 2, cntL = 0;
            for (int p : plus)  cntL += get_info(tr[p].lc, l, mid).cnt;
            for (int p : minus) cntL -= get_info(tr[p].lc, l, mid).cnt;
            if (k <= cntL)
            {
                r = mid;
                for (int &p : plus)  p = tr[p].lc;
                for (int &p : minus) p = tr[p].lc;
            }
            else
            {
                k -= cntL;
                l = mid + 1;
                for (int &p : plus)  p = tr[p].rc;
                for (int &p : minus) p = tr[p].rc;
            }
        }
        return l;
    }
private:
    int fork(int p, LL len)
    {
        assert(tot + 1 < cap && "max_nodes 开小了");
        int q = ++tot;
        if (p)
        {
            Node tmp = tr[p];
            tr.push_back(move(tmp));
        }
        else
        {
            tr.push_back(Node{});
            tr.back().info.len = len;
        }
        return q;
    }
    Info get_info(int p, LL l, LL r)
    {
        Info ret = p ? tr[p].info : Info{};
        if (!p) ret.len = r - l + 1;
        return ret;
    }
    void pushup(int p, LL l, LL r)
    {
        LL mid = l + (r - l) / 2;
        tr[p].info = get_info(tr[p].lc, l, mid) + get_info(tr[p].rc, mid + 1, r);
        tr[p].info.apply(tr[p].tag);
    }
    int modify(int p, LL l, LL r, LL x, LL y, const Tag& v)
    {
        if (l > y || r < x) return p;
        p = fork(p, r - l + 1);
        if (l >= x && r <= y)
        {
            tr[p].info.apply(v);
            tr[p].tag.apply(v);
            return p;
        }
        LL mid = l + (r - l) / 2;
        tr[p].lc = modify(tr[p].lc, l, mid, x, y, v);
        tr[p].rc = modify(tr[p].rc, mid + 1, r, x, y, v);
        pushup(p, l, r);
        return p;
    }
    Info query(int p, LL l, LL r, LL x, LL y, const Tag& acc)
    {
        if (l > y || r < x) return Info{};
        if (l >= x && r <= y)
        {
            Info ret = get_info(p, l, r);
            ret.apply(acc);
            return ret;
        }
        LL mid = l + (r - l) / 2;
        Tag down = acc;
        down.apply(tr[p].tag);
        return query(tr[p].lc, l, mid, x, y, down) + query(tr[p].rc, mid + 1, r, x, y, down);
    }
    template<class Pred>
    LL find_first(int p, LL l, LL r, LL start, const Tag& acc, Pred pred)
    {
        if (r < start) return -1;
        Info cur = get_info(p, l, r);
        cur.apply(acc);
        if (!pred(cur)) return -1;
        if (l == r) return l;
        LL mid = l + (r - l) / 2;
        Tag down = acc;
        down.apply(tr[p].tag);
        LL res = find_first(tr[p].lc, l, mid, start, down, pred);
        if (res == -1) res = find_first(tr[p].rc, mid + 1, r, start, down, pred);
        return res;
    }
    template<class Pred>
    LL find_last(int p, LL l, LL r, LL end, const Tag& acc, Pred pred)
    {
        if (l > end) return -1;
        Info cur = get_info(p, l, r);
        cur.apply(acc);
        if (!pred(cur)) return -1;
        if (l == r) return l;
        LL mid = l + (r - l) / 2;
        Tag down = acc;
        down.apply(tr[p].tag);
        LL res = find_last(tr[p].rc, mid + 1, r, end, down, pred);
        if (res == -1) res = find_last(tr[p].lc, l, mid, end, down, pred);
        return res;
    }
    int build(LL l, LL r, const vector<Info>& a)
    {
        int p = fork(0, r - l + 1);
        if (l == r)
        {
            tr[p].info = a[l];
            tr[p].info.len = 1;
            return p;
        }
        LL mid = l + (r - l) / 2;
        tr[p].lc = build(l, mid, a);
        tr[p].rc = build(mid + 1, r, a);
        pushup(p, l, r);
        return p;
    }
};
#endif

/* Usage:
struct Tag
{
    LL add = 0;
    void apply(const Tag& t) { add += t.add; }
};
struct Info
{
    LL len = 0, cnt = 0;
    void apply(const Tag& t) { cnt += t.add * len; }
    friend Info operator+(const Info& a, const Info& b)
    {
        return {a.len + b.len, a.cnt + b.cnt};
    }
};
int main()
{
    PersSegTree<Info, Tag> seg(5, 128);
    VI rt(5);
    VI a = {0, 3, 1, 3, 5};          // 值域下标
    for (int i = 1; i <= 4; i++)
        rt[i] = seg.modify(rt[i - 1], a[i], a[i], {1});
    cout << seg.find_kth({rt[4]}, {rt[1]}, 2) << "\n"; // 原数组 [2, 4] 第 2 小为 3
    // 树上路径第 k 小用 {rt[u], rt[v]} 减 {rt[lca], rt[parent_lca]}
    int branch = seg.modify(rt[4], 2, 4, {2}); // 此根不再用于 find_kth
    cout << seg.query(rt[4], 1, 5).cnt << " " << seg.query(branch, 1, 5).cnt << "\n"; // 4 10
    auto pred = [](const Info& v) { return v.cnt > 0; }; // 逐点计数非负
    cout << seg.find_first(rt[4], 2, pred) << " " << seg.find_last(rt[4], 4, pred) << "\n"; // 3 3
    seg.clear();                    // rt 和 branch 全部失效
    vector<Info> b = {{}, {1, 7}, {1, -2}};
    int base = seg.build(b);         // build 改值域为 [1, 2], 返回新根
    int newer = seg.modify(base, 1, 2, {3});
    cout << seg.query(base, 1, 2).cnt << " " << seg.query(newer, 1, 2).cnt << "\n"; // 5 11
}
*/
