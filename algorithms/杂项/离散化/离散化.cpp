// zoi: discrete
#include <vector>
#include <algorithm>
#include <cassert>
using namespace std;

template<class T> // 1-base
struct Dcr
{
    Dcr() {}
    vector<T> v;
    bool built = false;
    Dcr(const vector<T>& _v) : v(_v) { build(); }
    void add(const T& x) { v.push_back(x); built = false; }
    void reserve(size_t n) { v.reserve(n); }
    void build()
    {
        sort(v.begin(), v.end());
        v.erase(unique(v.begin(), v.end()), v.end());
        built = true;
    }
    int operator()(const T& x)
    {
        if (!built) build();
        auto it = lower_bound(v.begin(), v.end(), x);
        assert(it != v.end() && *it == x);
        return it - v.begin() + 1;
    }
    int size() const { return v.size(); }
    const T& operator[](int idx) const { return v[idx - 1]; }
};