// zoi: pbsPointer
#ifndef Z_OI_PBS_POINTER
#define Z_OI_PBS_POINTER

#include "../../utils/utils.cpp"
#include "../../../数据结构/树状数组/树状数组.cpp"
#include "../../离散化/离散化.cpp"

// 整体二分静态第 k 小: 保留事件与值域递归, E 与 add/count 由调用方提供
namespace PBSPointer
{
// 一维默认事件, 也可自定义 E; type=0 用 pos/val, type=1 用 l/r/k/id, 未用字段填 0
struct Event
{
    int type;           // 0: 插入, 1: 询问
    int pos, val;       // 原数组位置, d 中的离散排名
    int l, r, k, id;    // 原数组区间, 原始排名, 询问编号
};

// 静态第 k 小: 返回 ans[id] 的离散排名, q 按值接收, 可 move(q); q[0] 占位
// 前 n 个事件为插入, 其后 m 个为询问; n>=1, V>=1, val 在 1..V, 查询 k 合法, id 为 1..m 的排列
// E 可默认构造/复制/赋值, 有 int 字段 type/val/k/id; type=0 插入, type=1 询问, 其他字段由调用方定义
// add(const E&, int delta) 对插入增减 1, count(const E&) 返回查询集合内当前元素数, 二者不能修改事件
// 外部统计初始为空, 正常返回时恢复为空; 统计须可加可撤销, 查询集合固定, 只支持先插入后询问
// 时间: O(n log n+(n+m) log(V+1)*(1+A+C)), A/C 为 add/count 代价 | 辅助空间: O(n+m+log(V+1))
// 含形参 q 的工作数组约 sizeof(E)*(3n+2m)+4*(n+2m) 字节, 不含外部统计、E 持有的堆内存与调用方保留的 q
template<class E = Event, class Add, class Count>
VI kth(int n, int V, vector<E> q, Add add, Count count)
{
    int tot = (int)q.size() - 1, m = tot - n;
    vector<E> tmp(tot + 1);
    VI ans(m + 1, 0), goLeft(tot + 1, 0);
    vector<E> ins(q.begin(), q.begin() + n + 1);
    sort(ins.begin() + 1, ins.end(), [](const E& a, const E& b)
    {
        return a.val < b.val;
    });
    int used = 0;
    auto divide = [&](auto&& self, int L, int R, int ql, int qr) -> void
    {
        if (ql > qr) return;
        if (L == R)
        {
            for (int i = ql; i <= qr; i++)
                if (q[i].type == 1) ans[q[i].id] = L;
            return;
        }
        int mid = L + (R - L) / 2;
        // ins 固定有序, used 数元素而 mid 数不同值, 每次判定前调到全部 val <= mid
        while (used < n && ins[used + 1].val <= mid)
        {
            used++;
            add(as_const(ins[used]), 1);
        }
        while (used > 0 && ins[used].val > mid)
        {
            add(as_const(ins[used]), -1);
            used--;
        }
        int left_cnt = 0;
        for (int i = ql; i <= qr; i++)
        {
            auto& e = q[i];
            if (e.type == 0)
                goLeft[i] = (e.val <= mid);
            else
                goLeft[i] = (e.k <= count(as_const(e))); // 全局计数, k 不扣减
            left_cnt += goLeft[i];
        }
        // 不回滚, 下一递归自己调整 used; 保留插入事件分组以便与回滚版对照
        int p = ql, t = ql + left_cnt;
        for (int i = ql; i <= qr; i++)
        {
            if (goLeft[i]) tmp[p++] = q[i];
            else tmp[t++] = q[i];
        }
        for (int i = ql; i <= qr; i++) q[i] = tmp[i];
        self(self, L, mid, ql, ql + left_cnt - 1);
        self(self, mid + 1, R, ql + left_cnt, qr);
    };
    divide(divide, 1, V, 1, tot);
    // 外部统计可复用, 最终撤销仍留在前缀中的插入
    while (used > 0)
    {
        add(as_const(ins[used]), -1);
        used--;
    }
    return ans;
}
// 一维静态第 k 小, 返回 ans[id] 原值; q 的公共字段同 kth, 另需 int pos/l/r
// 前 n 项各插入一个 1..n 位置, 查询区间合法; d 已 build 且含全部插入值, val=d(原值)
// q 按值接收, 可 move(q); 内部创建 BIT, 时间 O(n log n+(n+m) log(n+1) log(V+1)), V=d.size()
// 空间为 kth 工作区加 O(n) BIT, 默认 Event 合计约 104n+64m 字节, 不含调用方保留的 q/d
template<class E = Event>
VI range_kth(int n, vector<E> q, const Dcr<int>& d)
{
    BIT bit(n);
    auto add = [&](const E& e, int delta) { bit.add(e.pos, e.pos, delta); };
    auto count = [&](const E& e) { return bit.query(e.l, e.r); };
    auto ans = kth(n, d.size(), move(q), add, count);
    for (int i = 1; i < (int)ans.size(); i++) ans[i] = d[ans[i]];
    return ans;
}
}
#endif
/* Usage: P3834 单组输入, 两版选一份; 改题时展开后修改事件、统计与分流
本版用静态 ins 的值域前缀作统计, k 始终为原排名; 不能直接接入时间交错的修改事件
#include "pbsPointer.h"
int main()
{
    fast_io();
    int n, m; cin >> n >> m;
    VI a(n + 1);
    for (int i = 1; i <= n; i++) cin >> a[i];
    Dcr<int> d;
    d.reserve(n);
    for (int i = 1; i <= n; i++) d.add(a[i]);
    d.build();
    vector<PBSPointer::Event> q(n + m + 1);
    for (int i = 1; i <= n; i++)
        q[i] = {0, i, d(a[i]), 0, 0, 0, 0};
    for (int i = 1; i <= m; i++)
    {
        int l, r, k; cin >> l >> r >> k;
        q[n + i] = {1, 0, 0, l, r, k, i};
    }
    auto ans = PBSPointer::range_kth(n, move(q), d); // 不再需要 q 时转移, 保留原表则传 q
    for (int i = 1; i <= m; i++) cout << ans[i] << endl;
}
*/

/* Usage: 二维静态矩形第 k 小, 与上面的 main 选一份; E 从 vector<Event> 自动推导
#include "pbsPointer.h"
#include "bit2d.h"
struct Event
{
    int type, val, k, id;
    int x1, y1, x2, y2; // 插入时两角相同, 查询时为矩形两角
};
int main()
{
    fast_io();
    int rows, cols, m; cin >> rows >> cols >> m;
    int n = rows * cols;
    vector<Event> q(n + m + 1);
    Dcr<int> d;
    d.reserve(n);
    for (int x = 1, i = 0; x <= rows; x++)
        for (int y = 1; y <= cols; y++)
        {
            int v; cin >> v;
            d.add(v);
            q[++i] = {0, v, 0, 0, x, y, x, y};
        }
    d.build();
    for (int i = 1; i <= n; i++) q[i].val = d(q[i].val);
    for (int i = 1; i <= m; i++)
    {
        auto& e = q[n + i];
        e.type = 1, e.id = i;
        cin >> e.x1 >> e.y1 >> e.x2 >> e.y2 >> e.k;
    }
    BIT2D bit(rows, cols);
    auto add = [&](const Event& e, int delta) { bit.add(e.x1, e.y1, e.x1, e.y1, delta); };
    auto count = [&](const Event& e) { return bit.query(e.x1, e.y1, e.x2, e.y2); };
    auto ans = PBSPointer::kth(n, d.size(), move(q), add, count);
    for (int i = 1; i <= m; i++) cout << d[ans[i]] << endl; // kth 返回排名, 在外部还原
}
*/
