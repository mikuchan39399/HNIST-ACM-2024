// ============ seg_check SegTree/DySegTree 回归套件 ============
// 覆盖: SegTree(区间加/单点/双通道查询/find_first/last 二分+覆盖 Tag 变体
//       含 {0,false} 空标记 no-op, static 实例 init 多测复用) | DySegTree
//       (build 常规域 + 1e9 大值域稀疏域不 build, 虚点 len/零值语义, map 暴力)
// 纪律: 改动 泛型线段树/泛型动态线段树 模板, 必重跑本套件
// 跑法: g++ -std=c++20 -Wall -Wextra -O2 seg_check.cpp -o seg_check && ./seg_check
#include <iostream>
#include <vector>
#include <cassert>
#include <algorithm>
#include <climits>
#include <map>
#include <random>
#include "../泛型动态线段树.cpp"
#include "../泛型线段树.cpp"
#include "../../ST表/ST表.cpp"

using namespace std;
using LL = long long;

// 注: C++ 局部类不能定义 friend operator+, Info/Tag 须放函数外
// 组 1: 区间加, 维护 sum/mx 双查询通道
struct TagS
{
    LL add = 0;
    void apply(const TagS& t) { add += t.add; }
    void clear() { add = 0; }
    bool has_tag() const { return add != 0; }
};
struct InfoS
{
    LL len = 0, sum = 0, mx = 0;
    bool break_cond(const TagS&) const { return false; }
    bool tag_cond(const TagS&) const { return true; }
    void apply(const TagS& t) { sum += t.add * len; mx += t.add; }
    friend InfoS operator+(const InfoS& a, const InfoS& b)
    {
        if (a.len == 0) return b;
        if (b.len == 0) return a;
        InfoS r;
        r.len = a.len + b.len;
        r.sum = a.sum + b.sum;
        r.mx = max(a.mx, b.mx);
        return r;
    }
};

// 组 2: 区间覆盖 (assign 型 Tag, 验证 Info/Tag 代数解耦; has=false 空标记)
struct TagC
{
    LL v = 0;
    bool has = false;
    void apply(const TagC& t)
    {
        if (t.has)
        {
            v = t.v;
            has = true;
        }
    }
    void clear() { v = 0; has = false; }
    bool has_tag() const { return has; }
};
struct InfoC
{
    LL len = 0, sum = 0, mx = 0;
    bool break_cond(const TagC&) const { return false; }
    bool tag_cond(const TagC&) const { return true; }
    void apply(const TagC& t)
    {
        if (t.has) { sum = t.v * len; mx = t.v; }
    }
    friend InfoC operator+(const InfoC& a, const InfoC& b)
    {
        if (a.len == 0) return b;
        if (b.len == 0) return a;
        InfoC r;
        r.len = a.len + b.len;
        r.sum = a.sum + b.sum;
        r.mx = max(a.mx, b.mx);
        return r;
    }
};

void test_seg_tree()
{
    mt19937 rng(1926);
    // 模式 A+B: 区间加/单点/查询/二分, static 实例跨组 init 复用
    static SegTree<InfoS, TagS> seg(70);
    for (int tc = 0; tc < 300; tc++)
    {
        int n = 1 + rng() % 60;
        vector<InfoS> a(n + 1);
        vector<LL> ref(n + 1);
        for (int i = 1; i <= n; i++)
        {
            LL v = (LL)(rng() % 41) - 20;
            a[i] = {1, v, v};
            ref[i] = v;
        }
        seg.init(n);
        seg.build(a);
        for (int t = 0, ops = 100; t < ops; t++)
        {
            int op = rng() % 5;
            int l = 1 + rng() % n, r = 1 + rng() % n;
            if (l > r) swap(l, r);
            if (op == 0)
            {
                LL d = (LL)(rng() % 11) - 5;
                seg.modify(l, r, {d});
                for (int i = l; i <= r; i++) ref[i] += d;
            }
            else if (op == 1)
            {
                LL d = (LL)(rng() % 11) - 5;
                seg.modify(l, l, {d});
                ref[l] += d;
            }
            else if (op == 2)
            {
                LL s = 0, m = LLONG_MIN;
                for (int i = l; i <= r; i++)
                {
                    s += ref[i];
                    m = max(m, ref[i]);
                }
                InfoS res = seg.query(l, r);
                assert(res.sum == s && res.mx == m);
            }
            else if (op == 3)
            {
                LL x = (LL)(rng() % 41) - 20;
                int exp = -1;
                for (int i = l; i <= n; i++)
                    if (ref[i] > x) { exp = i; break; }
                assert(seg.find_first(l, [x](const InfoS& in) { return in.mx > x; }) == exp);
            }
            else
            {
                LL x = (LL)(rng() % 41) - 20;
                int exp = -1;
                for (int i = r; i >= 1; i--)
                    if (ref[i] > x) { exp = i; break; }
                assert(seg.find_last(r, [x](const InfoS& in) { return in.mx > x; }) == exp);
            }
        }
        LL s = 0, m = LLONG_MIN;
        for (int i = 1; i <= n; i++)
        {
            s += ref[i];
            m = max(m, ref[i]);
        }
        InfoS res = seg.query(1, n);
        assert(res.len == n && res.sum == s && res.mx == m);
    }
    // 模式 C: 覆盖型 Tag + 空标记 ({0,false} 必须是 no-op)
    for (int tc = 0; tc < 150; tc++)
    {
        int n = 1 + rng() % 60;
        SegTree<InfoC, TagC> seg(n);
        vector<InfoC> a(n + 1);
        vector<LL> ref(n + 1);
        for (int i = 1; i <= n; i++)
        {
            LL v = (LL)(rng() % 21) - 10;
            a[i] = {1, v, v};
            ref[i] = v;
        }
        seg.init(n);
        seg.build(a);
        for (int t = 0, ops = 60; t < ops; t++)
        {
            int l = 1 + rng() % n, r = 1 + rng() % n;
            if (l > r) swap(l, r);
            int op = rng() % 3;
            if (op == 0)
            {
                LL v = (LL)(rng() % 21) - 10;
                seg.modify(l, r, {v, true});
                for (int i = l; i <= r; i++) ref[i] = v;
            }
            else if (op == 1)
            {
                seg.modify(l, r, {0, false});      // 空标记 no-op
            }
            else
            {
                LL s = 0, m = LLONG_MIN;
                for (int i = l; i <= r; i++)
                {
                    s += ref[i];
                    m = max(m, ref[i]);
                }
                InfoC res = seg.query(l, r);
                assert(res.sum == s && res.mx == m);
            }
        }
    }
}

void test_dy_seg_tree()
{
    mt19937 rng(810);
    // 模式 A: build 常规域
    for (int tc = 0; tc < 300; tc++)
    {
        int n = 1 + rng() % 60;
        vector<InfoS> a(n + 1);
        vector<LL> ref(n + 1);
        for (int i = 1; i <= n; i++)
        {
            LL v = (LL)(rng() % 41) - 20;
            a[i] = {1, v, v};
            ref[i] = v;
        }
        DySegTree<InfoS, TagS> s(n, 100000);
        s.build(a);
        for (int t = 0, ops = 80; t < ops; t++)
        {
            int op = rng() % 5;
            int l = 1 + rng() % n, r = 1 + rng() % n;
            if (l > r) swap(l, r);
            if (op == 0)
            {
                LL d = (LL)(rng() % 11) - 5;
                s.modify(l, r, {d});
                for (int i = l; i <= r; i++) ref[i] += d;
            }
            else if (op == 1)
            {
                LL d = (LL)(rng() % 11) - 5;
                s.modify(l, l, {d});
                ref[l] += d;
            }
            else if (op == 2)
            {
                LL sm = 0, m = LLONG_MIN;
                for (int i = l; i <= r; i++)
                {
                    sm += ref[i];
                    m = max(m, ref[i]);
                }
                InfoS res = s.query(l, r);
                assert(res.len == r - l + 1 && res.sum == sm && res.mx == m);
            }
            else if (op == 3)
            {
                LL x = (LL)(rng() % 41) - 20;
                LL exp = -1;
                for (int i = l; i <= n; i++)
                    if (ref[i] > x) { exp = i; break; }
                assert(s.find_first(l, [x](const InfoS& in) { return in.mx > x; }) == exp);
            }
            else
            {
                LL x = (LL)(rng() % 41) - 20;
                LL exp = -1;
                for (int i = r; i >= 1; i--)
                    if (ref[i] > x) { exp = i; break; }
                assert(s.find_last(r, [x](const InfoS& in) { return in.mx > x; }) == exp);
            }
        }
        LL sm = 0, m = LLONG_MIN;
        for (int i = 1; i <= n; i++)
        {
            sm += ref[i];
            m = max(m, ref[i]);
        }
        InfoS res = s.query(1, n);
        assert(res.len == n && res.sum == sm && res.mx == m);
    }
    // 模式 B: 1e9 大值域稀疏域, 不 build; 活动小窗 [1,200] + 远点, map 暴力
    // 语义: 未触碰下标恒为 0 (虚点零值), find 的 pred 取 x >= 0 仅实点可命中
    for (int tc = 0; tc < 300; tc++)
    {
        const LL N = 1000000000;
        DySegTree<InfoS, TagS> s(N, 100000);
        map<LL, LL> mp;
        vector<LL> far(5);
        for (auto& f : far) f = 201 + rng() % (N - 200);
        for (int t = 0, ops = 80; t < ops; t++)
        {
            int op = rng() % 4;
            if (op == 0) // 窗内区间加
            {
                int l = 1 + rng() % 200, r = 1 + rng() % 200;
                if (l > r) swap(l, r);
                LL d = (LL)(rng() % 11) - 5;
                s.modify(l, r, {d});
                for (int i = l; i <= r; i++) mp[i] += d;
            }
            else if (op == 1) // 单点: 窗内或远点
            {
                LL x = rng() % 2 ? 1 + rng() % 200 : far[rng() % 5];
                LL d = (LL)(rng() % 11) - 5;
                s.modify(x, x, {d});
                mp[x] += d;
            }
            else if (op == 2) // 区间查: 窗内 or 跨远点, 虚点 len/零值推导
            {
                LL l, r;
                if (rng() % 2)
                {
                    l = 1 + rng() % 200;
                    r = 1 + rng() % 200;
                    if (l > r) swap(l, r);
                }
                else
                {
                    l = 1;
                    r = far[rng() % 5];   // 全窗 + 大段虚点
                }
                LL sm = 0, m;
                if (r <= 200)
                {
                    m = LLONG_MIN;
                    for (LL k = l; k <= r; k++)   // 精确逐位: 未触碰位值 0
                    {
                        LL v = mp.count(k) ? mp[k] : 0;
                        sm += v;
                        m = max(m, v);
                    }
                }
                else
                {
                    m = 0;   // 范围含海量未触碰位(值 0)
                    for (auto& [k, v] : mp)
                    {
                        if (k >= l && k <= r)
                        {
                            sm += v;
                            m = max(m, v);
                        }
                    }
                }
                InfoS res = s.query(l, r);
                assert(res.len == r - l + 1 && res.sum == sm && res.mx == m);
            }
            else // find_first(start): x ∈ [0,2], 虚点(0)不满足
            {
                LL start = 1 + rng() % 200;
                LL x = rng() % 3;
                LL exp = -1;
                for (auto& [k, v] : mp)
                    if (k >= start && v > x) { exp = k; break; }
                assert(s.find_first(start, [x](const InfoS& in) { return in.mx > x; }) == exp);
            }
        }
        // 全值域尾对账
        LL sm = 0, m = 0;
        for (auto& [k, v] : mp)
        {
            sm += v;
            m = max(m, v);
        }
        InfoS res = s.query(1, N);
        assert(res.len == N && res.sum == sm && res.mx == m);
    }
}

void test_st_table()
{
    mt19937 rng(20260901);
    for (int tc = 0; tc < 200; tc++)
    {
        int n = 1 + rng() % 80;
        vector<LL> a(n + 1);
        for (int j = 1; j <= n; j++) a[j] = (LL)(rng() % 201) - 100;
        for (int mode = 0; mode < 2; mode++)
        {
            ST st(1);              // 故意开小, 验自动扩容
            st.build(a, mode == 0);
            int q = 30;
            while (q--)
            {
                int l = 1 + rng() % n, r = l + rng() % (n - l + 1);
                LL ref = a[l];
                for (int j = l; j <= r; j++) ref = mode == 0 ? max(ref, a[j]) : min(ref, a[j]);
                assert(st.query(l, r) == ref);
            }
            assert(st.query(1, 1) == a[1] && st.query(n, n) == a[n]);
            assert(st.query(1, n) == (mode == 0 ? *max_element(a.begin() + 1, a.end())
                                                : *min_element(a.begin() + 1, a.end())));
        }
    }
}

// Affine composition includes assignment (mul=0) and sign reversal.
struct TagA
{
    LL mul=1, add=0;
    void apply(const TagA& t) { mul*=t.mul; add=add*t.mul+t.add; }
    void clear() { *this={}; }
    bool has_tag() const { return mul!=1 || add!=0; }
};
struct InfoA
{
    LL len=0, sum=0, lo=0, hi=0;
    bool break_cond(const TagA&) const { return false; }
    bool tag_cond(const TagA&) const { return true; }
    void apply(const TagA& t)
    {
        sum=sum*t.mul+len*t.add;
        LL x=lo*t.mul+t.add, y=hi*t.mul+t.add;
        lo=min(x,y); hi=max(x,y);
    }
    friend InfoA operator+(const InfoA& a,const InfoA& b)
    {
        if (!a.len) return b;
        if (!b.len) return a;
        return {a.len+b.len,a.sum+b.sum,min(a.lo,b.lo),max(a.hi,b.hi)};
    }
};
// Local arithmetic progression, split_tag must shift the right child's origin.
struct TagP
{
    LL first=0, step=0;
    void apply(const TagP& t) { first+=t.first; step+=t.step; }
    void clear() { *this={}; }
    bool has_tag() const { return first || step; }
};
struct InfoP
{
    LL len=0, sum=0;
    bool break_cond(const TagP&) const { return false; }
    bool tag_cond(const TagP&) const { return true; }
    void apply(const TagP& t) { sum+=len*t.first+len*(len-1)/2*t.step; }
    pair<TagP,TagP> split_tag(const TagP& t,const InfoP& l,const InfoP&) const
    { return {t,{t.first+l.len*t.step,t.step}}; }
    friend InfoP operator+(const InfoP& a,const InfoP& b) { return {a.len+b.len,a.sum+b.sum}; }
};
// chmin reaches leaves when mixed; get_real_tag converts cap to a lazy delta.
struct TagB
{
    LL value=0;
    void apply(const TagB& t) { value+=t.value; }
    void clear() { value=0; }
    bool has_tag() const { return value!=0; }
};
struct InfoB
{
    LL len=0, sum=0, lo=0, hi=0;
    bool break_cond(const TagB& t) const { return hi<=t.value; }
    bool tag_cond(const TagB&) const { return lo==hi; }
    TagB get_real_tag(const TagB& t) const { return {t.value-hi}; }
    void apply(const TagB& t) { sum+=len*t.value; lo+=t.value; hi+=t.value; }
    friend InfoB operator+(const InfoB& a,const InfoB& b)
    {
        if (!a.len) return b;
        if (!b.len) return a;
        return {a.len+b.len,a.sum+b.sum,min(a.lo,b.lo),max(a.hi,b.hi)};
    }
};

template<class Tree>
void affine_case(Tree& s, vector<LL> a, mt19937& rng)
{
    int n=(int)a.size()-1;
    for(int op=0;op<100;op++)
    {
        int l=1+rng()%n, r=1+rng()%n;
        if(l>r) swap(l,r);
        if(op%7==0) { l=1; r=n; }
        TagA t{(int)(rng()%3)-1,(int)(rng()%41)-20};
        s.modify(l,r,t);
        for(int i=l;i<=r;i++) a[i]=a[i]*t.mul+t.add;
        l=1+rng()%n; r=1+rng()%n;
        if(l>r) swap(l,r);
        LL sum=0,lo=LLONG_MAX,hi=LLONG_MIN;
        for(int i=l;i<=r;i++) { sum+=a[i]; lo=min(lo,a[i]); hi=max(hi,a[i]); }
        auto v=s.query(l,r);
        assert(v.len==r-l+1 && v.sum==sum && v.lo==lo && v.hi==hi);
        LL k=(int)(rng()%81)-40;
        int first=-1,last=-1;
        for(int i=l;i<=n;i++) if(a[i]>=k) { first=i; break; }
        for(int i=r;i>=1;i--) if(a[i]>=k) { last=i; break; }
        auto pred=[k](const InfoA& x){return x.hi>=k;};
        assert(s.find_first(l,pred)==first && s.find_last(r,pred)==last);
        assert(s.find_first(0,pred)==-1 && s.find_first(n+1,pred)==-1);
        assert(s.find_last(0,pred)==-1 && s.find_last(n+1,pred)==-1);
    }
}
void test_advanced()
{
    mt19937 rng(42);
    SegTree<InfoA,TagA> fixed(65);
    for(int tc=0;tc<400;tc++)
    {
        int n=tc<4 ? (1<<tc) : 1+rng()%65;
        vector<LL> a(n+1);
        vector<InfoA> info(n+1);
        for(int i=1;i<=n;i++) { a[i]=(int)(rng()%101)-50; info[i]={1,a[i],a[i],a[i]}; }
        fixed.init(n); fixed.build(info);
        affine_case(fixed,a,rng);
        DySegTree<InfoA,TagA> dyn(n,2*n-1);
        if(tc%2) dyn.build(info); else fill(a.begin(),a.end(),0);
        affine_case(dyn,a,rng);
    }
    for(int tc=0;tc<400;tc++)
    {
        int n=1+rng()%65;
        SegTree<InfoP,TagP> s(n);
        DySegTree<InfoP,TagP> d(n,2*n-1);
        vector<InfoP> info(n+1); s.build(info);
        vector<LL> ref(n+1);
        for(int op=0;op<60;op++)
        {
            TagP t{(int)(rng()%21)-10,(int)(rng()%7)-3};
            // Whole-domain AP; arbitrary queries force repeated lazy splitting.
            s.modify(1,n,t); d.modify(1,n,t);
            for(int i=1;i<=n;i++) ref[i]+=t.first+(i-1)*t.step;
            int l=1+rng()%n,r=1+rng()%n; if(l>r) swap(l,r);
            LL sum=0; for(int i=l;i<=r;i++) sum+=ref[i];
            assert(s.query(l,r).sum==sum && d.query(l,r).sum==sum);
        }
        SegTree<InfoB,TagB> b(n);
        DySegTree<InfoB,TagB> db(n,2*n-1);
        vector<InfoB> bi(n+1);
        for(int i=1;i<=n;i++) { ref[i]=(int)(rng()%101)-50; bi[i]={1,ref[i],ref[i],ref[i]}; }
        b.build(bi); db.build(bi);
        for(int op=0;op<60;op++)
        {
            int l=1+rng()%n,r=1+rng()%n; if(l>r) swap(l,r);
            LL cap=(int)(rng()%161)-80;
            b.modify(l,r,{cap}); db.modify(l,r,{cap});
            for(int i=l;i<=r;i++) ref[i]=min(ref[i],cap);
            l=1+rng()%n; r=1+rng()%n; if(l>r) swap(l,r);
            LL sum=0,lo=LLONG_MAX,hi=LLONG_MIN;
            for(int i=l;i<=r;i++) { sum+=ref[i]; lo=min(lo,ref[i]); hi=max(hi,ref[i]); }
            for(auto v:{b.query(l,r),db.query(l,r)}) assert(v.sum==sum && v.lo==lo && v.hi==hi && v.len==r-l+1);
        }
    }
}
// Independent difference Fenwick: range additions and prefix integrals.
struct RefBIT
{
    int n; vector<LL> a,b;
    RefBIT(int n):n(n),a(n+2),b(n+2) {}
    void point(int p,LL v) { for(int i=p;i<=n;i+=i&-i) {a[i]+=v;b[i]+=v*(p-1);} }
    void add(int l,int r,LL v) {point(l,v);point(r+1,-v);}
    LL prefix(int p) { LL x=0,y=0;for(int i=p;i;i-=i&-i){x+=a[i];y+=b[i];}return x*p-y; }
};
void test_target_scale()
{
    constexpr int n=200000;
    mt19937 rng(42);
    SegTree<InfoS,TagS> s(n);
    DySegTree<InfoS,TagS> d(n,2*n-1);
    vector<InfoS> a(n+1); s.build(a); d.build(a);
    assert(d.idx==2*n-1);
    RefBIT ref(n);
    for(int op=0;op<200000;op++)
    {
        int l=1+rng()%n,r=1+rng()%n; if(l>r) swap(l,r);
        if(op%13==0) {l=1;r=n;}
        LL v=(int)(rng()%2001)-1000;
        s.modify(l,r,{v}); d.modify(l,r,{v}); ref.add(l,r,v);
        l=1+rng()%n; r=1+rng()%n; if(l>r) swap(l,r);
        LL sum=ref.prefix(r)-ref.prefix(l-1);
        auto x=s.query(l,r),y=d.query(l,r);
        assert(x.sum==sum && y.sum==sum && x.len==r-l+1 && y.len==r-l+1);
    }
    for(int m:{1,2,65537,n})
    {
        s.init(m); a.assign(m+1,{});
        for(int i=1;i<=m;i++) a[i]={1,i,i};
        s.build(a); s.modify(1,m,{-1000000000000LL});
        for(int q=0;q<25000;q++)
        {
            int k=1+rng()%m;
            auto pred=[k](const InfoS& x){return x.mx>=k-1000000000000LL;};
            assert(s.find_first(1,pred)==k && s.find_last(m,pred)==m);
            assert(s.find_last(k,pred)==k);
        }
    }
}
void test_st_extreme()
{
    mt19937 rng(42); ST st;
    for(int tc=0;tc<400;tc++)
    {
        int n=1+rng()%65; VLL a(n+1);
        for(int i=1;i<=n;i++) a[i]=tc%3 ? (int)(rng()%101)-50 : -7;
        a[1]=LLONG_MIN; a[n]=LLONG_MAX;
        for(bool mode:{false,true})
        {
            st.build(a,mode);
            for(int l=1;l<=n;l++)
            {
                LL v=a[l];
                for(int r=l;r<=n;r++) {v=mode?max(v,a[r]):min(v,a[r]);assert(st.query(l,r)==v);}
            }
        }
    }
    for(int n:{200000,1,2,65537,200000})
    {
        VLL a(n+1); for(int i=1;i<=n;i++) a[i]=i-1000000000000LL;
        for(bool mode:{false,true})
        {
            st.build(a,mode);
            for(int q=0;q<100000;q++)
            {
                int l=1+rng()%n,r=1+rng()%n; if(l>r) swap(l,r);
                assert(st.query(l,r)==a[mode?r:l]);
            }
        }
    }
}
void test_wide_domain()
{
    // LL coordinate arithmetic at its supported endpoint, no nonzero huge sum.
    DySegTree<InfoS,TagS> s(LLONG_MAX,400);
    auto nonnegative=[](const InfoS& x){return x.mx>=0;};
    assert(s.query(2,LLONG_MAX-1).len==LLONG_MAX-2 && s.idx==0);
    assert(s.find_first(LLONG_MAX,nonnegative)==LLONG_MAX);
    assert(s.find_last(LLONG_MAX,nonnegative)==LLONG_MAX && s.idx==0);
    s.modify(LLONG_MAX,LLONG_MAX,{-7});
    assert(s.query(LLONG_MAX,LLONG_MAX).sum==-7);
    assert(s.find_last(LLONG_MAX,nonnegative)==LLONG_MAX-1);
    assert(s.query(1,LLONG_MAX).sum==-7 && s.tr[0].lc==0 && s.tr[0].rc==0);
    // Entire huge range tagged, then descend into previously virtual nodes.
    DySegTree<InfoS,TagS> t(1000000000000LL,2000);
    t.modify(1,t.n,{-2}); t.modify(t.n,t.n,{5});
    assert(t.query(1,t.n).sum==-2*t.n+5 && t.query(1,t.n).mx==3);
    auto positive=[](const InfoS& x){return x.mx>0;};
    assert(t.find_first(1,positive)==t.n && t.find_last(t.n,positive)==t.n);
    assert(t.query(2,t.n-1).sum==-2*(t.n-2));
}

void test_sparse_scale()
{
    constexpr LL n=1000000000000LL, step=4096;
    constexpr int q=200000;
    DySegTree<InfoS,TagS> s(n,4000010);
    mt19937 rng(42);
    for(int i=1;i<=q;i++)
    {
        LL p=step*i;
        s.modify(p,p,{1});
        LL l=1+(LL)(rng()%((unsigned long long)p+step));
        LL r=1+(LL)(rng()%((unsigned long long)p+step));
        if(l>r) swap(l,r);
        LL left=max(1LL,(l+step-1)/step),right=min<LL>(i,r/step);
        LL count=max(0LL,right-left+1);
        auto v=s.query(l,r);
        assert(v.len==r-l+1 && v.sum==count && v.mx==(count?1:0));
        auto pred=[](const InfoS& x){return x.mx>0;};
        LL first=max(1LL,(l+step-1)/step),last=min<LL>(i,r/step);
        assert(s.find_first(l,pred)==(first<=i?first*step:-1));
        assert(s.find_last(r,pred)==(last>=1?last*step:-1));
    }
    assert(s.query(1,n).sum==q && s.tr[0].info.len==0);
    cout << "sparse scale: nodes=" << s.idx << ", node_bytes=" << sizeof(s.tr[0]) << '\n';
}

void test_reset()
{
    constexpr int cap=399999;
    DySegTree<InfoA,TagA> s(200000,cap);
    auto capacity=s.tr.capacity();
    for(int n:{200000,1,2,65537,200000,200000})
    {
        vector<InfoA> a(n+1);
        for(int i=1;i<=n;i++) a[i]={1,-i,-i,-i};
        s.build(a);
        assert(s.n==n && s.idx==2*n-1 && s.tr.capacity()==capacity);
        assert(s.query(1,n).sum==-(LL)n*(n+1)/2);
        s.modify(1,n,{0,7});
        assert(s.query(n,n).sum==7);
        s.clear();
        assert(s.n==n && s.idx==0 && s.root==0 && s.tr.size()==1 && s.tr.capacity()==capacity);
        assert(s.query(1,n).sum==0 && s.query(1,n).len==n && s.idx==0);
        assert(s.find_first(1,[](const InfoA& v){return v.hi>=0;})==1);
        s.modify(1,n,{1,2});
        assert(s.query(1,n).sum==2LL*n);
    }
    s.init(1000000000000LL);
    assert(s.n==1000000000000LL && !s.idx && !s.root);
    s.modify(s.n,s.n,{0,-9});
    assert(s.query(s.n,s.n).sum==-9 && s.query(1,s.n).hi==0);
    s.init(1);
    assert(s.query(1,1).sum==0 && !s.idx);
    DySegTree<InfoS,TagS> exact(1,1);
    for(int i=0;i<100;i++)
    {
        exact.build(vector<InfoS>{{},{1,i,i}});
        assert(exact.idx==1 && exact.query(1,1).sum==i);
    }
}

int main()
{
    test_reset();
    test_sparse_scale();
    test_advanced();
    test_target_scale();
    test_st_extreme();
    test_wide_domain();
    test_seg_tree();
    test_dy_seg_tree();
    test_st_table();
    cout << "seg_check passed: SegTree / DySegTree / ST all tests ok\n";
    return 0;
}