#include <cassert>
#include <iostream>
#include "../可持久化字典树.cpp"

int main()
{
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
