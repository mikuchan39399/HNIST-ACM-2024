#include <iostream>
#include <vector>

using namespace std;
using LL = long long;

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

    vector<int> get_min(VLL& a, int n, int k)
    {
        vector<int> res;
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

    vector<int> get_max(VLL& a, int n, int k)
    {
        vector<int> res;
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
 * 使用示例：
 * const int MAXN = 1000005; // 根据题目要求设定极限值
 * MonotonicQueue mq(MAXN);
 * void solve()
 * {
    * int n, k;
    * cin >> n >> k;
    * mq.init(n, k);
    * for (int i = 1; i <= n; i++)
    * {
        * cin >> mq.a[i]; 
    * }
    * vector<int> min_ans = mq.get_min();
    * for (int i = 0; i < min_ans.size(); i++)
    * {
        * cout << min_ans[i] << (i == min_ans.size() - 1 ? "" : " ");
    * }
    * cout << "\n";
    * vector<int> max_ans = mq.get_max();
    * for (int i = 0; i < max_ans.size(); i++)
    * {
        * cout << max_ans[i] << (i == max_ans.size() - 1 ? "" : " ");
    * }
    * cout << "\n";
 * }
 */