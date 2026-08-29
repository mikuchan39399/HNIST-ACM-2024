// ============ z_rnd 随机数回归套件 ============
// 覆盖: 值域包含(300组随机区间) | 覆盖+均匀性([0,9] 打1e6发, ±33σ松界) |
//       缺省下界0生效 | long long / 无符号 / 浮点路径
// 纪律: 改动 z_rnd 模板, 必重跑本套件
// 跑法: g++ -std=c++20 -Wall -Wextra -O2 rnd_check.cpp -o rnd_check && ./rnd_check
#include <cassert>
#include <cstdio>
#include <random>
#include "../z_rnd.cpp"
using namespace std;

int main()
{
    mt19937 par(42);
    // 300 组随机区间: 值域包含
    for (int t = 0; t < 300; t++)
    {
        int lo = (int)(par() % 2001) - 1000;
        int hi = lo + (int)(par() % 2001);
        for (int i = 0; i < 100; i++)
        {
            int x = z_rnd(hi, lo);
            assert(lo <= x && x <= hi);
        }
    }
    // [0,9] 打 1e6 发: 全值覆盖 + 均匀性 (期望 1e5, ±1e4 为 33σ 松界)
    // 兼验缺省下界 0 生效 (cnt[0] 命中)
    int cnt[10] = {0};
    for (int i = 0; i < 1000000; i++) cnt[z_rnd(9)]++;
    for (int v = 0; v < 10; v++) assert(90000 <= cnt[v] && cnt[v] <= 110000);
    // long long / 无符号 / 浮点路径
    for (int i = 0; i < 100; i++)
    {
        long long x = z_rnd(1LL << 62, -(1LL << 62));
        assert(-(1LL << 62) <= x && x <= (1LL << 62));
        size_t u = z_rnd((size_t)100);
        assert(u <= 100);
        double d = z_rnd(1.0);
        assert(0.0 <= d && d < 1.0);
    }
    puts("z_rnd check passed");
    return 0;
}
