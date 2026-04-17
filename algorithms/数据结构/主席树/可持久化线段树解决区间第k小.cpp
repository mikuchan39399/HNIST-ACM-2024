// ---------------------------------------------------------
// 主席树 (可持久化线段树) - 静态区间第 k 小/大 核心模板
// 空间复杂度: O(N * log(值域))，通常开 30 * N ~ 40 * N
// ---------------------------------------------------------

const int N = 2e5 + 10;
int root[N]; // 记录每个历史版本的根节点
int idx;     // 内存池分配指针

#define lc(p) tr[p].lc
#define rc(p) tr[p].rc

struct Node 
{
    int lc, rc;
    int cnt; // 区间内元素个数
} tr[30 * N];

// 离散化去重后的值域数组，用于建立权值线段树
vector<int> vals;

// 获取离散化后的离线索引 (1-based)
int get_id(int x) 
{
    return lower_bound(vals.begin(), vals.end(), x) - vals.begin() + 1;
}

void pushup(int p) 
{
    tr[p].cnt = tr[lc(p)].cnt + tr[rc(p)].cnt;
}

// 核心：在版本 v 的基础上，在位置 x 插入值 k，生成新版本 p
// 外部调用示例: modify(root[i], 1, vals.size(), root[i - 1], get_id(a[i]), 1);
void modify(int& p, int l, int r, int v, int x, int k) 
{
    p = ++idx;       // 动态分配新节点
    tr[p] = tr[v];   // 核心：直接复制上一个版本的节点状态 (共享未修改的子树)
    if (l == r) 
    {
        tr[p].cnt += k;
        return;
    }
    int mid = l + (r - l) / 2;
    // 只有发生修改的路径才需要新建节点，另一半直接沿用 tr[v] 的指针
    if (x <= mid) modify(lc(p), l, mid, lc(v), x, k);
    else modify(rc(p), mid + 1, r, rc(v), x, k);
    
    pushup(p);
}

// 核心：查询区间 [L, R] 的第 k 小
// 利用前缀和思想: 版本 R 的树 减去 版本 L-1 的树
// 外部调用示例: int id = query(root[R], 1, vals.size(), root[L - 1], k); cout << vals[id - 1] << endl;
int query(int p, int l, int r, int v, int k) 
{
    if (l == r) return l;
    
    int mid = l + (r - l) / 2;
    // 计算左子树实际新增的元素个数
    int d = tr[lc(p)].cnt - tr[lc(v)].cnt; 
    
    if (k <= d) return query(lc(p), l, mid, lc(v), k);
    else return query(rc(p), mid + 1, r, rc(v), k - d);
}

// 多组测试数据时的初始化清理
void init() 
{
    idx = 0;
    vals.clear();
    root[0] = 0;
    tr[0] = {0, 0, 0}; // 0号节点作为空节点哨兵，一切皆从虚无开始
}