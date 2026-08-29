#include <algorithm>
using namespace std;

using LL = long long;

struct Tag
{
    LL op_mod = 0;
    void apply(const Tag&)          // 只在叶子节点应用懒标记，永远不会叠加
    {}     
    void clear() 
    {}                              // 不下放就不会清空
    bool has_tag() const            // 永不下放懒标记
    { 
        return false; 
    }
};

struct Info
{
    LL sum = 0;
    LL max_val = 0;
    int len = 0;
    Info() {}
    bool break_cond(const Tag& t) const     
    {
        return max_val < t.op_mod;
    }
    bool tag_cond(const Tag&) const        
    {
        return len == 1; 
    }
    void apply(const Tag& t)                 
    {
        max_val %= t.op_mod;
        sum = max_val;
    }
    friend Info operator+(const Info& a, const Info& b)
    {
        if (a.len == 0) return b;
        if (b.len == 0) return a;
        Info c;
        c.sum = a.sum + b.sum;
        c.max_val = max(a.max_val, b.max_val);
        c.len = a.len + b.len;
        return c;
    }
};