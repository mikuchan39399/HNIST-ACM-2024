/**
 * a[N] -- 原始数组
 * struct node -- 线段树节点结构体，包含{l, r, sum, mul, add}
 *      l -- 节点管理原始数组区间左边界
 *      r -- 节点管理原始数组区间右边界
 *      sum -- 节点管理的区间和信息
 *      mul -- 乘法懒标记
 *      add -- 加法懒标记
 * N -- 数据数量边界，考虑离散化
 * m -- 结果对m取模
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
    tr[p].sum = (tr[lc].sum + tr[rc].sum) % m;
}

/**
 * @brief 以p节点为根节点递归构造一棵管理目标区间的线段树
 * @param p 要作为根节点的目标节点编号
 * @param l 目标区间左边界
 * @param r 目标区间右边界
 */
void build(int p, int l, int r)
{
    tr[p] = {l, r, 0, 1, 0};
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
 * @brief 修改线段树节点管理的信息并采取“先乘后加”叠加懒标记
 * @param p 线段树根节点编号
 * @param mul 乘法懒标记
 * @param add 加法懒标记
 */
void lazy(int p, LL mul, LL add)
{
    int l = tr[p].l, r = tr[p].r;
    tr[p].add = ((tr[p].add * mul) % m + add) % m;
    tr[p].mul = (tr[p].mul * mul) % m;
    tr[p].sum  = ((tr[p].sum * mul) % m + add * (r - l + 1)) % m;
}


/**
 * @brief 下放线段树节点懒标记
 * @param p 线段树根节点编号
 */
void push_down(int p)
{
    if (tr[p].mul == 1 && tr[p].add == 0) 
    {
        return;
    }
    lazy(lc, tr[p].mul, tr[p].add);
    lazy(rc, tr[p].mul, tr[p].add);
    tr[p].add = 0;
    tr[p].mul = 1;
}


/**
 * @brief 在线段树中区间修改原始数组对应位置的值
 * @param p 线段树根节点编号
 * @param x 目标区间在原始数组中的左边界
 * @param y 目标区间在原始数组中的右边界
 * @param mul 要往区间上乘的值
 * @param add 要往区间上加的值
 */
void modify(int p, int x, int y, LL mul, LL add)
{
    int l = tr[p].l, r = tr[p].r;
    if(l >= x && r <= y)
    {
        lazy(p, mul, add);
        return;
    }
    push_down(p);
    int mid = (l + r) >> 1;
    if(x <= mid) modify(lc, x, y, mul, add);
    if(y > mid) modify(rc, x, y, mul, add);
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
    push_down(p);
    LL sum = 0;
    int mid = (l + r) >> 1;
    if(x <= mid) sum = (sum + query(lc, x, y)) % m;
    if(y > mid) sum = (sum + query(rc, x, y)) % m;
    return sum;
}

