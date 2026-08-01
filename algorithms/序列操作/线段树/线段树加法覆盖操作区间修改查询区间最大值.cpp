/**
 * a[N] -- 原始数组
 * struct node -- 线段树节点结构体，包含{l, r, max, add, change, st}
 *      l -- 节点管理原始数组区间左边界
 *      r -- 节点管理原始数组区间右边界
 *      add -- 加法懒标记
 *      change -- 覆盖懒标记
 *      st -- 覆盖懒标记是否生效的布尔值
 * N -- 数据数量边界，考虑离散化
 * node tr[N << 2] -- struct node数组，顺序存储线段树
 * lc -- define p << 1
 * rc -- define p << 1 | 1
 * LL -> long long
 */

/**
 * @brief 根据子节点更新节点管理信息的辅助函数
 * @param p 目标节点编号
 */
void push_up(int p)
{
    tr[p].max = max(tr[lc].max, tr[rc].max);
}

/**
 * @brief 以p节点为根节点递归构造一棵管理目标区间的线段树
 * @param p 要作为根节点的目标节点编号
 * @param l 目标区间左边界
 * @param r 目标区间右边界
 */
void build(int p, int l, int r)
{
    tr[p] = {l, r, 0, 0, 0, false};
    if(l == r)
    {
        tr[p].max = a[l];
        return;
    }
    int mid = (l + r) >> 1;
    build(lc, l, mid);
    build(rc, mid + 1, r);
    push_up(p);
}



/**
 * @brief 修改线段树节点管理的信息并添加或叠加懒标记
 * @param p 线段树根节点编号
 * @param add 加法懒标记
 * @param change 覆盖懒标记
 * @param st 覆盖懒标记是否启用的标记
 */
void lazy(int p, LL add, LL change, bool st)
{
    if(st)
    {
        tr[p].max = change;
        tr[p].add = 0;
        tr[p].st = true;
        tr[p].change = change;
    }
    tr[p].max += add;
    tr[p].add += add;
}


/**
 * @brief 下放线段树节点懒标记
 * @param p 线段树根节点编号
 */
void push_down(int p)
{
    lazy(lc, tr[p].add, tr[p].change, tr[p].st);
    lazy(rc, tr[p].add, tr[p].change, tr[p].st);
    tr[p].st = tr[p].add = tr[p].change = 0;
}


/**
 * @brief 在线段树中区间修改原始数组对应位置的值
 * @param p 线段树根节点编号
 * @param x 目标区间在原始数组中的左边界
 * @param y 目标区间在原始数组中的右边界
 * @param add 往区间上加的值
 * @param change 往区间上覆盖的值
 * @param st 覆盖操作启用标记
 */
void modify(int p, int x, int y, LL add, LL change, bool st)
{
    int l = tr[p].l, r = tr[p].r;
    if(l >= x && r <= y)
    {
        lazy(p, add, change, st);
        return;
    }
    push_down(p);
    int mid = (l + r) >> 1;
    if(x <= mid) modify(lc, x, y, add, change, st);
    if(y > mid) modify(rc, x, y, add, change, st);
    push_up(p);
}

/**
 * @brief 在线段树中查询原始数组目标区间和
 * @param p 线段树根节点编号
 * @param x 目标原始区间左边界
 * @param y 目标原始区间右边界
 */
LL query(int p, int x, int y)
{
    int l = tr[p].l, r = tr[p].r;
    if(l >= x && r <= y)
    {
        return tr[p].max;
    }
    push_down(p);
    LL ret = -INF;
    int mid = (l + r) >> 1;
    if(x <= mid) ret = max(ret, query(lc, x, y));
    if(y > mid) ret = max(ret, query(rc, x, y));
    return ret;
}