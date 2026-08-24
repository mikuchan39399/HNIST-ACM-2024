#include <iostream>

using namespace std;

using LL = long long;

const int N = 2050; // 根据题意修改矩阵最大边长

// 二维树状数组核心：利用四个树状数组维护差分数组的不同项
// 假设原数组为 A，其差分数组为 D
// t1 维护：D[i][j]
// t2 维护：D[i][j] * i
// t3 维护：D[i][j] * j
// t4 维护：D[i][j] * i * j
LL t1[N][N], t2[N][N], t3[N][N], t4[N][N];
int n, m;

#define lowbit(x) ((x) & -(x))

// 单点修改辅助函数：更新差分数组对应的四个项
void add(int x, int y, LL k) 
{
    for (int i = x; i <= n; i += lowbit(i)) 
    {
        for (int j = y; j <= m; j += lowbit(j)) 
        {
            t1[i][j] += k;
            t2[i][j] += x * k;
            t3[i][j] += y * k;
            t4[i][j] += 1ll * x * y * k; // 加上 1ll 防止坐标相乘提前溢出 int
        }
    }
}

// 二维区间修改：利用二维差分性质
// 将左上角 (x1, y1) 到右下角 (x2, y2) 范围内的所有元素加上 k
void range_add(int x1, int y1, int x2, int y2, LL k) 
{
    add(x1, y1, k);
    add(x1, y2 + 1, -k);
    add(x2 + 1, y1, -k);
    add(x2 + 1, y2 + 1, k);
}

// 二维前缀和查询辅助函数
// 数学推导：通过四个差分维护数组，求出 (1, 1) 到 (x, y) 的前缀和
LL query(int x, int y) 
{
    LL res = 0;
    for (int i = x; i > 0; i -= lowbit(i)) 
    {
        for (int j = y; j > 0; j -= lowbit(j)) 
        {
            res += 1ll * (x + 1) * (y + 1) * t1[i][j]
                 - 1ll * (y + 1) * t2[i][j]
                 - 1ll * (x + 1) * t3[i][j]
                 + t4[i][j];
        }
    }
    return res;
}

// 二维区间查询：利用二维容斥原理
// 查询左上角 (x1, y1) 到右下角 (x2, y2) 范围内的所有元素的和
LL range_query(int x1, int y1, int x2, int y2) 
{
    return query(x2, y2) 
         - query(x1 - 1, y2) 
         - query(x2, y1 - 1) 
         + query(x1 - 1, y1 - 1);
}