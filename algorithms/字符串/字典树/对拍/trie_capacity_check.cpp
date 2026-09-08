#include <cassert>
#include <iostream>
#include "../可持久化字典树.cpp"
#include "../字典树.cpp"
#include "../可持久化字典树.cpp"
#include "../字典树.cpp"

template<int HB>
void exact_bits()
{
    PersTrie<2, HB> t(2 * (HB + 2));
    LL x = HB == 63 ? 0x7fffffffffffffffLL : (LL)((1ULL << (HB + 1)) - 1);
    for (int i = 0; i < 10; ++i)
    {
        t.clear();
        auto data = t.tr.data();
        int a = t.insert(0, x), b = t.insert(a, 0LL);
        assert(t.tot == 2 * (HB + 2) && t.tr.data() == data);
        assert(t.max_xor(a, 0) == x && t.max_xor(b, x) == x);
        assert(t.size(a) == 1 && t.size(b) == 2);
    }
    if constexpr (HB < 63) exact_bits<HB + 1>();
}

int main()
{
    exact_bits<0>();
    Trie<26> ordinary(4);
    Trie<2> bits(65);
    Trie<26> empty(1);
    PersTrie<26> zero(0);
    for (int i = 0; i < 300; ++i)
    {
        ordinary.clear(); bits.clear(); empty.clear(); zero.clear();
        auto a = ordinary.tr.data(); auto b = bits.tr.data();
        ordinary.insert("abc"); ordinary.insert("ab"); ordinary.insert("abc");
        bits.insert_num(0); bits.insert_num(0); empty.insert("");
        assert(ordinary.tr.size() == 4 && ordinary.tr.data() == a);
        assert(ordinary.count_prefix("ab") == 3 && ordinary.count_word("ab") == 1);
        assert(bits.tr.size() == 65 && bits.tr.data() == b && bits.max_xor(7) == 7);
        assert(empty.count_word("") == 1 && empty.tr.size() == 1);
        assert(zero.size(0) == 0 && zero.count_prefix(0, "a") == 0);
    }
    // 空串也要复制根; 刚好用满预算时, 哨兵不能挤占可用结点或触发扩容
    PersTrie<26> strings(3);
    PersTrie<2, 3> numbers(5);
    for (int t = 0; t < 300; t++)
    {
        strings.clear();
        auto p = strings.tr.data();
        int a = strings.insert(0, "");
        int b = strings.insert(a, "a");
        assert(strings.tot == 3 && strings.tr.data() == p);
        assert(strings.size(a) == 1 && strings.size(b) == 2);
        assert(strings.count_prefix(a, "a") == 0);
        assert(strings.count_prefix(b, "a") == 1);
        numbers.clear();
        auto q = numbers.tr.data();
        int rt = numbers.insert(0, 15);
        assert(numbers.tot == 5 && numbers.tr.data() == q);
        assert(numbers.size(0) == 0 && numbers.max_xor(rt, 0) == 15);
    }
    std::cout << "trie capacity: PASS\n";
}
