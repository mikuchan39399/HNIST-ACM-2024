/**
 * a[N] -- 原始数组
 * struct node -- 线段树节点结构体，包含{l, r, sum}
 *      l -- 节点管理原始数组区间左边界
 *      r -- 节点管理原始数组区间右边界
 *      sum -- 节点管理的区间和信息
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
    tr[p].sum = tr[lc].sum + tr[rc].sum;
}

/**
 * @brief 以p节点为根节点递归构造一棵管理目标区间的线段树
 * @param p 要作为根节点的目标节点编号
 * @param l 目标区间左边界
 * @param r 目标区间右边界
 */
void build(int p, int l, int r)
{
    tr[p] = {l, r, 0};
    if(l == r)
    {
        tr[p].sum = a[l];
        return;
    }
    int mid = (l + r) >> 1;
    build(lc, l, mid);
    build(rc, mid + 1, r);
    push_up(p);
}

/**
 * @brief 在线段树中单点修改原始数组对应位置的值
 * @param p 线段树根节点编号
 * @param x 目标在原始数组中的位置
 * @param k 往线段树中被管理的目标加上此参数的值
 */
void modify(int p, int x, int k)
{
    int l = tr[p].l, r = tr[p].r;
    if(l == r)
    {
        tr[p].sum += k;
        return;
    }
    int mid = (l + r) >> 1;
    if(x <= mid) modify(lc, x, k);
    else modify(rc, x, k);
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
        return tr[p].sum;
    }
    int mid = (l + r) >> 1;
    LL sum = 0;
    if(x <= mid) sum += query(lc, x, y);
    if(y > mid) sum += query(rc, x, y);
    return sum;
}