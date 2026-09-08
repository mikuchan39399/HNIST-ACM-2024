// zoi: eratosthenes
#ifndef Z_OI_ERATOSTHENES
#define Z_OI_ERATOSTHENES

#include "../../杂项/utils/utils.cpp"

// 埃氏筛, prime[x] 标记质数, primes 按升序从下标 0 存; 不保存最小质因子
// 0<=n<INT_MAX 且表能放下; 位压缩标记约 (n+1)/8+4*pi(n) B, n=1e7 约 3.9MB 有效数据
struct Eratosthenes
{
    int n = 0;
    vector<bool> prime;
    VI primes;
    Eratosthenes(int n = 0) { init(n); }
    // 重建 [0,n] 的质数标记和质数表, 覆盖旧结果; 不做增量续筛
    // 时间: O(n log log n) | 空间: O(n), 缩表保留已分配容量
    void init(int limit)
    {
        assert(limit >= 0 && limit < INT_MAX);
        n = limit;
        prime.assign((size_t)n + 1, true);
        primes.clear();
        prime[0] = false;
        if (n >= 1) prime[1] = false;
        for (int i = 2; i <= n / i; ++i)
            if (prime[i])
                for (LL j = 1LL * i * i; j <= n; j += i) prime[j] = false;
        for (int i = 2; i <= n; ++i)
            if (prime[i]) primes.push_back(i);
    }
    // 查询 [0,n] 内 x 是否为质数, 0 和 1 返回 false
    // 时间: O(1) | 空间: O(1)
    bool is_prime(int x) const { return prime[x]; }
};
#endif

/* Usage
#include <eratosthenes.h>
int main()
{
    Eratosthenes sieve(30);
    for (int p : sieve.primes) cout << p << ' '; // 2 3 5 7 11 13 17 19 23 29
    cout << '\n' << sieve.is_prime(29) << ' ' << sieve.is_prime(25) << '\n'; // 1 0
    sieve.init(1);
    cout << sieve.primes.size() << '\n'; // 0, 旧表已经清空
}
*/
