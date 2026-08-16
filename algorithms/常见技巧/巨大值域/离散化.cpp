#include <vector>
#include <algorithm>
using namespace std;

template<class T> // 1-base
struct Dcr
{
    Dcr() {}
    vector<T> v;
    Dcr(const vector<T>& _v) : v(_v) { build(); }
    void add(const T& x) { v.push_back(x); }
    void build()
    {
        sort(v.begin(), v.end());
        v.erase(unique(v.begin(), v.end()), v.end());
    }
    int operator()(const T& x) const { return lower_bound(v.begin(), v.end(), x) - v.begin() + 1; }
    int size() const { return v.size(); }
    const T& operator[](int idx) const { return v[idx - 1]; }
};