// zoi: monoQueue
#include <iostream>
#include <vector>

using namespace std;
using LL = long long;
using VLL = vector<LL>;

struct MonotonicQueue
{
    vector<LL> q;

    MonotonicQueue(int max_n)
    {
        q.resize(max_n + 10, 0);
    }

    void init()
    {
        q.clear();
    }

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

/*
 * 使用示例:
 * MonotonicQueue mq(n + 1);        // 预算 n + 1
 * VLL a(n + 1);                    // 1-based 原数组
 * vector<LL> mn = mq.get_min(a, n, k);   // 滑窗最小
 * vector<LL> mx = mq.get_max(a, n, k);   // 滑窗最大
 * 多测: mq.init() 复用
 */