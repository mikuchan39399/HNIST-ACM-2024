// Test-only normalization: run the existing electronic-API suites on printed span engines.
#pragma once
#include "utils.h"

inline auto booklet_view(const string& s) { return span(s); }
template<class T, class A>
auto booklet_view(const vector<T, A>& s) { return span(s).subspan(!s.empty()); }
template<class T, size_t N>
auto booklet_view(span<T, N> s) { return s; }

template<class Seq, class F>
decltype(auto) booklet_call(const Seq& s, F f) { return f(booklet_view(s)); }
template<class A, class F>
decltype(auto) booklet_call(const vector<bool, A>& s, F f)
{
    size_t n = s.empty() ? 0 : s.size() - 1;
    unique_ptr<bool[]> data(new bool[n]);
    for (size_t i = 0; i < n; i++) data[i] = s[i + 1];
    return f(span<const bool>(data.get(), n));
}

#define BOOKLET_BUILD_ADAPTER(Name, Base) \
    Name(const string& s = "") { build(s); } \
    template<class Seq> Name(const Seq& s) { build(s); } \
    void build(const string& s) { Base::build(span(s)); } \
    template<class T, class A> void build(const vector<T, A>& s) \
    { booklet_call(s, [&](auto v) { Base::build(v); }); } \
    template<class T, size_t N> void build(span<T, N> s) { Base::build(s); }

#if defined(BOOKLET_CHECK_KMP)
#define KMPSeq BookletKMPSeq
#define KMP BookletKMP
#include "kmp.h"
#undef KMP
#undef KMPSeq
template<class Seq>
struct KMPSeq : BookletKMPSeq<Seq>
{
    using Base = BookletKMPSeq<Seq>;
    KMPSeq(const Seq& s = Seq()) { build(s); }
    template<class T, size_t N> KMPSeq(span<T, N> s) { build(s); }
    void build(const Seq& s) { Base::build(booklet_view(s)); }
    template<class T, size_t N> void build(span<T, N> s) { Base::build(s); }
    int find_first(const Seq& s) const { return Base::find_first(booklet_view(s)); }
    template<class T, size_t N>
    int find_first(span<T, N> s) const { return Base::find_first(s); }
    VI find_all(const Seq& s) const { return Base::find_all(booklet_view(s)); }
    template<class T, size_t N>
    VI find_all(span<T, N> s) const { return Base::find_all(s); }
};
using KMP = KMPSeq<string>;
#elif defined(BOOKLET_CHECK_MANACHER)
#define Manacher BookletManacher
#include "manacher.h"
#undef Manacher
struct Manacher : BookletManacher
{
    BOOKLET_BUILD_ADAPTER(Manacher, BookletManacher)
};
#elif defined(BOOKLET_CHECK_Z)
#define ZFunction BookletZFunction
#include "zFunction.h"
#undef ZFunction
struct ZFunction : BookletZFunction
{
    BOOKLET_BUILD_ADAPTER(ZFunction, BookletZFunction)
    static VI extend(const string& s, const string& p)
    { return BookletZFunction::extend(span(s), span(p)); }
    template<class T, class A, class B>
    static VI extend(const vector<T, A>& s, const vector<T, B>& p)
    {
        return booklet_call(s, [&](auto a) {
            return booklet_call(p, [&](auto b) { return BookletZFunction::extend(a, b); });
        });
    }
    template<class T, size_t N, class U, size_t M>
    static VI extend(span<T, N> s, span<U, M> p) { return BookletZFunction::extend(s, p); }
};
#else
#error Select one printed engine to test
#endif
#undef BOOKLET_BUILD_ADAPTER
