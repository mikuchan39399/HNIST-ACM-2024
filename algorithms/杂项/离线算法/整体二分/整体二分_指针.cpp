// zoi: pbsPointer
#ifndef Z_OI_PBS_POINTER
#define Z_OI_PBS_POINTER

#include "../../utils/utils.cpp"
#include "../../../数据结构/树状数组/树状数组.cpp"
#include "../../离散化/离散化.cpp"

// 整体二分的静态区间第 k 小骨架, 值域递归与稳定划分固定, 事件/统计/分流按题修改
namespace PBSPointer
{
// type=0 用 pos/val, type=1 用 l/r/k/id, 未用字段填 0
struct Event
{
    int type;           // 0: 插入, 1: 询问
    int pos, val;       // 原数组位置, d 中的离散排名
    int l, r, k, id;    // 原数组区间, 原始排名, 询问编号
};

// q[0] 占位, q[1..n] 各插入一个不同位置, 其后为 m 个询问, id 为 1..m 的排列
// d 已 build 且含全部插入值, val 为 d 的 1-based 排名; n >= 1, m >= 0, pos 在 1..n, 查询区间及 k 合法
// 返回 ans[id] 的原值, q 按值接收且不改调用方; 可传 move(q) 省副本, 每次调用重建状态
// 时间: O(n log n + (n+m) log(n+1) log(V+1)), V=d.size() | 空间: O(n+m+log(V+1))
// 每个 Event 为 7 个 int, 含形参 q 的工作数组约 104n+64m 字节, 不含调用方保留的 q/d
inline VI range_kth(int n, vector<Event> q, const Dcr<int>& d)
{
    int tot = (int)q.size() - 1, m = tot - n;
    vector<Event> tmp(tot + 1);
    VI ans(m + 1, 0), goLeft(tot + 1, 0);
    vector<Event> ins(q.begin(), q.begin() + n + 1);
    sort(ins.begin() + 1, ins.end(), [](const Event& a, const Event& b)
    {
        return a.val < b.val;
    });
    int used = 0;
    BIT bit(n);
    auto divide = [&](auto&& self, int L, int R, int ql, int qr) -> void
    {
        if (ql > qr) return;
        if (L == R)
        {
            for (int i = ql; i <= qr; i++)
                if (q[i].type == 1) ans[q[i].id] = d[L];
            return;
        }
        int mid = L + (R - L) / 2;
        // ins 固定有序, used 数元素而 mid 数不同值, 每次判定前调到全部 val <= mid
        while (used < n && ins[used + 1].val <= mid)
        {
            used++;
            bit.add(ins[used].pos, ins[used].pos, 1);
        }
        while (used > 0 && ins[used].val > mid)
        {
            bit.add(ins[used].pos, ins[used].pos, -1);
            used--;
        }
        int left_cnt = 0;
        for (int i = ql; i <= qr; i++)
        {
            auto& e = q[i];
            if (e.type == 0)
                goLeft[i] = (e.val <= mid);
            else
                goLeft[i] = (e.k <= bit.query(e.l, e.r)); // 全局计数, k 不扣减
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
    divide(divide, 1, d.size(), 1, tot);
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
