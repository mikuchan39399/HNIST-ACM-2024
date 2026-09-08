// zoi: customHash
#ifndef Z_OI_HASH
#define Z_OI_HASH

#include "../utils/utils.cpp"

// SipHash-4-8, 128 位随机密钥与 64 位输出; 原算法 Aumasson/Bernstein, CC0
// https://github.com/veorq/SipHash, 防哈希洪泛的保守轮数; 不承诺 unordered_map 最坏 O(1)
// 整数 O(1), 字符串 O(长度), 整数对 O(1); 每个哈希对象 16 B, 默认种子来自 random_device
struct custom_hash
{
    static_assert(sizeof(size_t) == 8, "custom_hash requires a 64-bit target");

    custom_hash() : k0(seed()), k1(seed()) {}
    // 固定密钥仅用于对拍复现, 赛场默认构造; 对象复制保持密钥, 不在存有元素时换密钥
    custom_hash(uint64_t a, uint64_t b) : k0(a), k1(b) {}

    template <integral T> requires (sizeof(T) <= 8)
    size_t operator()(T x) const noexcept
    {
        char bytes[8];
        store(bytes, (uint64_t)x);
        return (*this)(string_view(bytes, 8));
    }
    template <integral A, integral B> requires (sizeof(A) <= 8 && sizeof(B) <= 8)
    size_t operator()(const pair<A, B>& x) const noexcept
    {
        char bytes[16];
        store(bytes, (uint64_t)x.first);
        store(bytes + 8, (uint64_t)x.second);
        return (*this)(string_view(bytes, 16));
    }
    // 字符串按原始字节取哈希, 包含内嵌 NUL; 支持 string/string_view, 不先压成 std::hash
    size_t operator()(string_view s) const noexcept
    {
        uint64_t v0 = 0x736f6d6570736575ULL ^ k0, v1 = 0x646f72616e646f6dULL ^ k1;
        uint64_t v2 = 0x6c7967656e657261ULL ^ k0, v3 = 0x7465646279746573ULL ^ k1;
        auto rounds = [&](int count)
        {
            while (count--)
            {
                v0 += v1; v1 = rotl(v1, 13) ^ v0; v0 = rotl(v0, 32);
                v2 += v3; v3 = rotl(v3, 16) ^ v2;
                v0 += v3; v3 = rotl(v3, 21) ^ v0;
                v2 += v1; v1 = rotl(v1, 17) ^ v2; v2 = rotl(v2, 32);
            }
        };
        size_t pos = 0;
        while (s.size() - pos >= 8)
        {
            uint64_t m = 0;
            for (int j = 0; j < 8; j++) m |= (uint64_t)(unsigned char)s[pos + j] << (8 * j);
            v3 ^= m; rounds(4); v0 ^= m;
            pos += 8;
        }
        uint64_t last = (uint64_t)s.size() << 56;
        for (size_t j = 0; j < s.size() - pos; j++)
            last |= (uint64_t)(unsigned char)s[pos + j] << (8 * j);
        v3 ^= last; rounds(4); v0 ^= last;
        v2 ^= 0xff; rounds(8);
        return v0 ^ v1 ^ v2 ^ v3;
    }
private:
    uint64_t k0, k1;
    static uint64_t seed()
    {
        random_device rd;
        return uniform_int_distribution<uint64_t>{}(rd);
    }
    static void store(char* p, uint64_t x)
    {
        for (int j = 0; j < 8; j++) p[j] = (unsigned char)(x >> (8 * j));
    }
};
#endif

/* Usage
#include "customHash.h"

int main()
{
    unordered_map<LL, int, custom_hash> cnt;
    cnt.max_load_factor(0.7f);          // 先设置负载率, 再按预计不同键数 reserve
    cnt.reserve(200000);
    cnt[-1]++;
    cnt[LLONG_MAX] += 2;
    cout << cnt[-1] << ' ' << cnt[LLONG_MAX] << '\n'; // 1 2

    unordered_set<string, custom_hash> words;
    words.insert(string("a\0b", 3));
    cout << words.contains(string("a\0b", 3)) << '\n'; // 1
    unordered_map<PII, LL, custom_hash> edge;
    edge[{1, 2}] = 7;
    cout << edge[{1, 2}] << '\n';       // 7, 两个整数按顺序编码
    // 保持键和哈希密钥不变; 不把哈希值当成唯一 ID, 不同键仍可能碰撞
    // 需要确定性的最坏复杂度时选 map/set 或离线排序, 而不是增加哈希轮数
}
*/
