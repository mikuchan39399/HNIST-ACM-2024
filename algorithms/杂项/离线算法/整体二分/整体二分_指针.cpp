// zoi: pbsPointer
#ifndef Z_OI_PBS_POINTER
#define Z_OI_PBS_POINTER

#include "../../utils/utils.cpp"

// 整体二分静态第 k 小: 保留事件与值域递归, E 与 add/query 由调用方提供
namespace PBSPointer
{
// 一维默认事件, 未用字段填 0; 自定义 E 时 type/val/k/id 为内核固定字段名, 不可改名或省略
// pos/l/r 仅供 add/query 定位, 可改名、替换或删除, 同步修改回调与事件初始化即可
struct Event
{
    int type;           // 0: 插入, 1: 询问
    int pos, val;       // 原数组位置, d 中的离散排名
    int l, r, k, id;    // 原数组区间, 原始排名, 询问编号
};

// 找计数首次达到 k 的答案坐标, 返回 ans[id]; 当前按静态第 k 小验证, k 须合法
// n 为插入事件数, V 为答案上界, 均 >=1; q[0] 占位, 先 n 个插入后 m 个询问, 按值接收可 move
// E 默认 Event, 须可默认构造/复制/赋值, 含 int type/val/k/id; val 在 1..V, 询问 id 为 1..m 的排列
// add(e,+1/-1) 加入/撤销插入, query(e) 返回固定集合的计数; 不改事件, 外部统计初始及返回时均为空
// 时间 O(n log n+(n+m) log(V+1)*(1+A+C)), A/C 为 add/query 代价; 辅助空间 O(n+m+log(V+1))
// 默认 Event, n=m=20万时工作数组约 30 MB; 另计调用方保留的 q 和外部统计
template<class E = Event, class Add, class Query>
VI find_first(int n, int V, vector<E> q, Add add, Query query)
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
                goLeft[i] = (e.k <= query(as_const(e))); // 全局计数, k 不扣减
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

}
#endif
/* Usage: P3834 单组输入, 两版选一份; 改题时展开后修改事件、统计与分流
本版用静态 ins 的值域前缀作统计, k 始终为原排名; 不能直接接入时间交错的修改事件
#include "pbsPointer.h"
#include "bit.h"
#include "discrete.h"
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
    BIT bit(n);
    auto add = [&](const PBSPointer::Event& e, int delta) { bit.add(e.pos, e.pos, delta); };
    auto query = [&](const PBSPointer::Event& e) { return bit.query(e.l, e.r); };
    auto ans = PBSPointer::find_first(n, d.size(), move(q), add, query);
    for (int i = 1; i <= m; i++) cout << d[ans[i]] << endl; // 返回答案坐标, 原值由调用方还原
}
*/

/* Usage: 二维静态矩形第 k 小, 与上面的 main 选一份; E 从 vector<Event> 自动推导
#include "pbsPointer.h"
#include "bit2d.h"
#include "discrete.h"
struct Event
{
    int type, val, k, id; // 内核固定字段名, 不可改名或省略
    int x1, y1, x2, y2;   // 自定义定位字段, 可改名; 插入时两角相同, 查询时为矩形两角
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
    auto query = [&](const Event& e) { return bit.query(e.x1, e.y1, e.x2, e.y2); };
    auto ans = PBSPointer::find_first(n, d.size(), move(q), add, query);
    for (int i = 1; i <= m; i++) cout << d[ans[i]] << endl; // find_first 返回排名, 在外部还原
}
*/
