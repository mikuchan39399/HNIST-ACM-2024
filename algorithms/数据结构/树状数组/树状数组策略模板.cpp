#include <vector>

template <class Policy>
struct BIT
{
    using T = typename Policy::T;
    int n;
    std::vector<T> s;

    #define lowbit(x) ((x) & -(x))

    BIT(int size) 
    : 
    n(size), 
    s(size + 1, Policy::identity()) 
    {}

    //1-based
    void build(const std::vector<T>& a) 
    {
        for (int i = 1; i <= n; ++i) s[i] = a[i];
        for (int i = 1; i <= n; ++i) 
        {
            int j = i + lowbit(i);
            if (j <= n) Policy::merge(s[j], s[i]);
        }
    }
    void modify(int x, T k) 
    {
        for (int i = x; i <= n; i += lowbit(i)) 
        {
            Policy::merge(s[i], k);
        }
    }

    T query(int x) 
    {
        T res = Policy::identity();
        for (int i = x; i; i -= lowbit(i)) 
        {
            Policy::merge(res, s[i]);
        }
        return res;
    }

    T query(int l, int r) 
    {
        T res_r = query(r);
        T res_l_1 = query(l - 1);
        Policy::merge(res_r, Policy::inverse(res_l_1));
        return res_r;
    }
    #undef lowbit
};



template <class Policy>
struct BIT2D 
{
    using T = typename Policy::T;
    int n, m;
    std::vector<std::vector<T>> s;

    #define lowbit(x) ((x) & -(x))

    BIT2D(int n, int m) 
    : 
    n(n), 
    m(m), 
    s(n + 1, std::vector<T>(m + 1, Policy::identity())) 
    {}

    //(1, 1) - based
    void build(const std::vector<std::vector<T>>& a) 
    {
        for (int i = 1; i <= n; ++i)
            for (int j = 1; j <= m; ++j)
                s[i][j] = a[i][j];
        
        for (int i = 1; i <= n; ++i) 
        {
            for (int j = 1; j <= m; ++j) 
            {
                int next_j = j + lowbit(j);
                if (next_j <= m) Policy::merge(s[i][next_j], s[i][j]);
            }
        }

        for (int i = 1; i <= n; ++i) 
        {
            int next_i = i + lowbit(i);
            if (next_i <= n) 
            {
                for (int j = 1; j <= m; ++j) 
                {
                    Policy::merge(s[next_i][j], s[i][j]);
                }
            }
        }
    }

    void modify(int x, int y, T k) 
    {
        for (int i = x; i <= n; i += lowbit(i)) 
        {
            for (int j = y; j <= m; j += lowbit(j)) 
            {
                Policy::merge(s[i][j], k);
            }
        }
    }

    T query(int x, int y) 
    {
        T res = Policy::identity();
        for (int i = x; i; i -= lowbit(i)) 
        {
            for (int j = y; j; j -= lowbit(j)) 
            {
                Policy::merge(res, s[i][j]);
            }
        }
        return res;
    }

 
    T query(int x1, int y1, int x2, int y2) 
    {
        // res = S(x2, y2) - S(x1-1, y2) - S(x2, y1-1) + S(x1-1, y1-1)
        T res = query(x2, y2);
        T sub1 = query(x1 - 1, y2);
        T sub2 = query(x2, y1 - 1);
        T add1 = query(x1 - 1, y1 - 1);

        Policy::merge(res, Policy::inverse(sub1));
        Policy::merge(res, Policy::inverse(sub2));
        Policy::merge(res, add1); // 减去两次的部分要加回来
        return res;
    } 
    #undef lowbit
};


/**
 * 树状数组策略需要提供四个接口
 * T 管理的数据类型
 * static void merge(T& data, const T& val) 将val合并至data
 * static constexpr T identity() 返回合并操作的幺元
 * static T inverse(const T& val) 返回val的 合并操作对应的逆操作值
 */
struct AddSumPolicy 
{
    using T = long long; 
    
    static void merge(T& data, const T& val) 
    {
        data += val;
    }

    static constexpr T identity() 
    { 
        return 0; 
    }

    static T inverse(const T& val) 
    {
        return -val;
    }
};


struct XorPolicy 
{
    using T = int;

    static void merge(T& data, const T& val) 
    {
        data ^= val;
    }


    static constexpr T identity() 
    { 
        return 0; 
    }

    static T inverse(const T& val) 
    {
        return val;
    }
};