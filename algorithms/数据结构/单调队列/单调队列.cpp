// zoi: monoQueue
#ifndef Z_OI_MONO_QUEUE
#define Z_OI_MONO_QUEUE

#include "../../杂项/utils/utils.cpp"

using namespace std;

// 返回 a[1..n] 每个长度为 k 的窗口最值, 结果从下标 0 开始存
struct MonotonicQueue
{
    vector<LL> q;

    // 一次分配 max_n 的工作空间, 后续查询的 n 不超过 max_n; 每个位置 8B
    // 时间: O(max_n) | 空间: 8*(max_n+10) 字节
    MonotonicQueue(int max_n)
    {
        q.resize(max_n + 10, 0);
    }

    // 多测复用入口; 队头队尾在每次查询中重置, 无需清空工作数组
    // 时间: O(1) | 空间: O(1)
    void init() {}

    // 返回所有长度为 k 的窗口最小值, 要求 1 <= k <= n
    // 时间: O(n) | 空间: O(n-k+1) 返回数组
    vector<LL> get_min(VLL& a, int n, int k)
    {
        vector<LL> res;
        if (n >= k)
        {
            res.reserve(n - k + 1);
        }

        int head = 1;
        int tail = 0;

        for (int i = 1; i <= n; i++)
        {
            while (head <= tail && q[head] <= i - k)
            {
                head++;
            }
            while (head <= tail && a[q[tail]] >= a[i])
            {
                tail--;
            }
            tail++;
            q[tail] = i;

            if (i >= k)
            {
                res.push_back(a[q[head]]);
            }
        }
        return res;
    }

    // 返回所有长度为 k 的窗口最大值, 要求 1 <= k <= n
    // 时间: O(n) | 空间: O(n-k+1) 返回数组
    vector<LL> get_max(VLL& a, int n, int k)
    {
        vector<LL> res;
        if (n >= k)
        {
            res.reserve(n - k + 1);
        }

        int head = 1;
        int tail = 0;

        for (int i = 1; i <= n; i++)
        {
            while (head <= tail && q[head] <= i - k)
            {
                head++;
            }
            while (head <= tail && a[q[tail]] <= a[i])
            {
                tail--;
            }
            tail++;
            q[tail] = i;

            if (i >= k)
            {
                res.push_back(a[q[head]]);
            }
        }
        return res;
    }
};
#endif

/*
 * 使用示例:
 * MonotonicQueue mq(n + 1);        // 预算 n + 1
 * VLL a(n + 1);                    // 1-based 原数组
 * vector<LL> mn = mq.get_min(a, n, k);   // 滑窗最小
 * vector<LL> mx = mq.get_max(a, n, k);   // 滑窗最大
 * 多测: mq.init() 复用
 */