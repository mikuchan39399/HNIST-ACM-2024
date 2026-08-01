/**
 * a[N] -- 原始数组
 * struct node -- 线段树节点结构体，包含{l, r, max_sum, lmax_sum, rmax_sum, sum}
 *      l -- 节点管理原始数组区间左边界
 *      r -- 节点管理原始数组区间右边界
 *      max_sum -- 节点内最大子段和
 *      lmax_sum -- 节点左端为起点的最大子段和
 *      rmax_sum -- 节点右端为起点的最大子段和
 *      sum -- 节点区间和
 * N -- 数据数量边界，考虑离散化
 * node tr[N << 2] -- struct node数组，顺序存储线段树
 * lc -- define p << 1
 * rc -- define p << 1 | 1
 * LL -> long long
 */

/**
 * @brief 根据子节点更新节点管理信息的辅助函数
 * @param p 目标节点
 * @param l 目标节点左节点
 * @param r 目标节点右节点
 */
void push_up(node& p, node& l, node& r)
{
    p.sum = l.sum + r.sum;
    p.lmax_sum = max(l.lmax_sum, l.sum + r.lmax_sum);
    p.rmax_sum = max(r.rmax_sum, r.sum + l.rmax_sum);
    p.max_sum = max({l.max_sum, r.max_sum, l.rmax_sum + r.lmax_sum});
}

/**
 * @brief 以p节点为根节点递归构造一棵管理目标区间的线段树
 * @param p 要作为根节点的目标节点编号
 * @param l 目标区间左边界
 * @param r 目标区间右边界
 */
void build(int p, int l, int r)
{
    tr[p] = {l, r, 0, 0, 0, 0};
    if(l == r)
    {
        tr[p].max_sum = tr[p].lmax_sum = tr[p].rmax_sum = tr[p].sum = a[l];
        return;
    }
    int mid = (l + r) >> 1;
    build(lc, l, mid);
    build(rc, mid + 1, r);
    push_up(tr[p], tr[lc], tr[rc]);
}



/**
 * @brief 在线段树中区间修改原始数组对应位置的值
 * @param p 线段树根节点编号
 * @param x 目标在原始数组中的下标位置
 * @param k 往区间上修改的值
 */
void modify(int p, int x, int k)
{
    int l = tr[p].l, r = tr[p].r;
    if(l == r)
    {
        tr[p].max_sum = tr[p].lmax_sum = tr[p].rmax_sum = tr[p].sum = k;
        return;
    }
    int mid = (l + r) >> 1;
    if(x <= mid) modify(lc, x, k);
    else modify(rc, x, k);
    push_up(tr[p], tr[lc], tr[rc]);
}

/**
 * @brief 在线段树中查询原始数组目标区间和
 * @param p 线段树根节点编号
 * @param x 目标原始区间左边界
 * @param y 目标原始区间右边界
 * 
 * @return 返回整个经由子节点更新后的节点结构体
 */
node query(int p, int x, int y)
{
    int l = tr[p].l, r = tr[p].r;
    if(l >= x && r <= y)
    {
        return tr[p];
    }
    int mid = (l + r) >> 1;
    if(y <= mid) return query(lc, x, y);
    if(x > mid) return query(rc, x, y);
    node lp = query(lc, x, y);
    node rp = query(rc, x, y);
    node ret;
    push_up(ret, lp, rp);
    return ret;
}