// zoi: eulerSieve
#ifndef Z_OI_EULER_SIEVE
#define Z_OI_EULER_SIEVE

#include "../../../杂项/utils/utils.cpp"

// 线性筛; lp[x] 为最小质因子, lp[0]=lp[1]=0, primes 为升序质数表(0-based)
struct LinearSieve
{
    int n = 0;
    VI lp, primes;
    LinearSieve(int n = 0) { init(n); }
    // 重建 [0,n], 0<=n<INT_MAX. 时空 O(n)
    void init(int limit)
    {
        assert(limit >= 0 && limit < INT_MAX);
        n = limit;
        lp.assign((size_t)n + 1, 0);
        primes.clear();
        for (int i = 2; i <= n; ++i)
        {
            if (!lp[i]) { lp[i] = i; primes.push_back(i); }
            for (int p : primes)
            {
                if (p > n / i) break;
                lp[i * p] = p;
                if (p == lp[i]) break;
            }
        }
    }
    // x 是否质数, 0<=x<=n. O(1)
    bool is_prime(int x) const { return x >= 2 && lp[x] == x; }
    // 分解 1<=x<=n, 返回升序 {质因子,指数}, 1 为空. 时间 O(log x), 空间 O(质因子种数)
    vector<PII> factorize(int x) const
    {
        vector<PII> factors;
        while (x > 1)
        {
            int p = lp[x], e = 0;
            do { x /= p; ++e; } while (x % p == 0);
            factors.push_back({p, e});
        }
        return factors;
    }
};
#endif

/* Usage
#include <eulerSieve.h>
int main()
{
    LinearSieve sieve(100);
    cout << sieve.is_prime(97) << ' ' << sieve.is_prime(1) << '\n'; // 1 0
    for (auto [p, e] : sieve.factorize(72)) cout << p << '^' << e << ' '; // 2^3 3^2
    cout << '\n';
    cout << sieve.lp[91] << '\n'; // 7, 可直接利用最小质因子递推
    sieve.init(10);
    for (int p : sieve.primes) cout << p << ' '; // 2 3 5 7
    cout << '\n';
}
*/
