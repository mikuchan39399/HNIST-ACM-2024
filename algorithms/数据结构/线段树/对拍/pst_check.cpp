// ============ pst_check PersSegTree 回归套件 ============
// 覆盖: 静态区间第k小(static 单实例+clear 多测复用) | 区间加历史版本随机回访 |
//       find_first/find_last 二分 | 区点修改混用
// 纪律: 改动 PersSegTree 模板, 必重跑本套件; SegTree/DySegTree 聚合于 seg_check.cpp
// 跑法: g++ -std=c++20 -Wall -Wextra -O2 pst_check.cpp -o pst_check && ./pst_check
#include <iostream>
#include <vector>
#include <cassert>
#include <algorithm>
#include <random>
#include "../可持久化线段树/主席树.cpp"

using namespace std;
using LL = long long;

// 注: C++ 局部类(函数体内定义的 struct)中不能定义 friend operator+,
//     因此 Info/Tag 必须放在函数外定义, 与模板配套使用
struct TagA
{
    LL c = 0;
    void apply(const TagA& t) { c += t.c; }
};
struct InfoA
{
    LL len = 0, cnt = 0;
    void apply(const TagA& t) { cnt += t.c; }
    friend InfoA operator+(const InfoA& a, const InfoA& b)
    {
        if (a.len == 0) return b;
        if (b.len == 0) return a;
        InfoA r;
        r.len = a.len + b.len;
        r.cnt = a.cnt + b.cnt;
        return r;
    }
};

struct TagB
{
    LL add = 0;
    void apply(const TagB& t) { add += t.add; }
};
struct InfoB
{
    LL len = 0, sum = 0, mx = 0;
    void apply(const TagB& t) { sum += t.add * len; mx += t.add; }
    friend InfoB operator+(const InfoB& a, const InfoB& b)
    {
        if (a.len == 0) return b;
        if (b.len == 0) return a;
        InfoB r;
        r.len = a.len + b.len;
        r.sum = a.sum + b.sum;
        r.mx = max(a.mx, b.mx);
        return r;
    }
};

void test_pers_seg_tree()
{
    mt19937 rng(42);
    int test_cases = 300;

    for (int tc = 0; tc < test_cases; tc++)
    {
        // 模式 A: 静态区间第 k 小 (static + clear 复用池子, 覆盖多测重置路径)
        {
            static PersSegTree<InfoA, TagA> s(1, 2000);
            int sn = 1 + rng() % 60;
            vector<LL> b(sn + 1);
            for (int i = 1; i <= sn; i++) b[i] = rng() % 17;

            vector<LL> vs(b.begin() + 1, b.end());
            sort(vs.begin(), vs.end());
            vs.erase(unique(vs.begin(), vs.end()), vs.end());
            int mm = vs.size();

            auto id2 = [&](LL x)
            {
                return (int)(lower_bound(vs.begin(), vs.end(), x) - vs.begin()) + 1;
            };

            s.set_n(mm);
            s.clear();
            vector<int> rt(sn + 1, 0);

            for (int i = 1; i <= sn; i++)
            {
                int p = id2(b[i]);
                rt[i] = s.modify(rt[i - 1], p, p, {1});
            }

            for (int t = 0; t < 20; t++)
            {
                int l = 1 + rng() % sn, r = 1 + rng() % sn;
                if (l > r) swap(l, r);
                int k = 1 + rng() % (r - l + 1);

                vector<LL> c(b.begin() + l, b.begin() + r + 1);
                nth_element(c.begin(), c.begin() + k - 1, c.end());
                assert(vs[s.find_kth({rt[r]}, {rt[l - 1]}, k) - 1] == c[k - 1]);
            }
        }

        // 模式 B: 区间加, 历史版本随机回访与二分
        {
            int sn = 1 + rng() % 60;
            vector<InfoB> a(sn + 1);
            for (int i = 1; i <= sn; i++)
            {
                a[i].len = 1;
                a[i].sum = a[i].mx = rng() % 11;
            }

            PersSegTree<InfoB, TagB> s(sn, 2 * sn + 80 * 20);
            vector<int> rts;
            vector<vector<LL>> snaps;

            {
                vector<LL> base(sn + 1);
                for (int i = 1; i <= sn; i++) base[i] = a[i].sum;
                rts.push_back(s.build(a));
                snaps.push_back(base);
            }

            int ops = 80;
            for (int t = 0; t < ops; t++)
            {
                int op = rng() % 4;
                if (op == 0 || op == 1)
                {
                    int l = 1 + rng() % sn, r = 1 + rng() % sn;
                    if (l > r) swap(l, r);
                    LL v = (int)(rng() % 9) - 4;
                    int base = rng() % rts.size(); // 从任意旧版本分叉, 不只延长最新版本
                    vector<LL> cur = snaps[base];

                    if (op == 0)
                    {
                        rts.push_back(s.modify(rts[base], l, r, {v}));
                        for (int i = l; i <= r; i++) cur[i] += v;
                    }
                    else
                    {
                        rts.push_back(s.modify(rts[base], l, l, {v}));
                        cur[l] += v;
                    }
                    snaps.push_back(cur);
                }
                else
                {
                    int vi = rng() % rts.size();
                    const vector<LL>& cur = snaps[vi];
                    int l = 1 + rng() % sn, r = 1 + rng() % sn;
                    if (l > r) swap(l, r);

                    LL es = 0, em = cur[l];
                    for (int i = l; i <= r; i++)
                    {
                        es += cur[i];
                        em = max(em, cur[i]);
                    }

                    InfoB res = s.query(rts[vi], l, r);
                    assert(res.sum == es && res.mx == em);

                    LL x = (int)(rng() % 15) - 4;

                    LL got = s.find_first(rts[vi], l, [x](const InfoB& info) { return info.mx > x; });
                    LL exp = -1;
                    for (LL i = l; i <= sn; i++)
                    {
                        if (cur[i] > x) { exp = i; break; }
                    }
                    assert(got == exp);

                    got = s.find_last(rts[vi], r, [x](const InfoB& info) { return info.mx > x; });
                    exp = -1;
                    for (LL i = r; i >= 1; i--)
                    {
                        if (cur[i] > x) { exp = i; break; }
                    }
                    assert(got == exp);
                }
            }
        }
    }
    cout << "All tests passed flawlessly!\n";
}

void test_branch_snapshots()
{
    mt19937 rng(42);PersSegTree<InfoB,TagB> s(1,20000);
    for(int tc=0;tc<400;tc++)
    {
        int n=1+rng()%45;s.clear();s.set_n(n);
        vector<InfoB> a(n+1);vector<LL> init(n+1);
        for(int i=1;i<=n;i++){init[i]=(int)(rng()%101)-50;a[i]={1,init[i],init[i]};}
        vector<int> roots{0,s.build(a)};vector<vector<LL>> snaps{vector<LL>(n+1),init};
        for(int op=0;op<100;op++)
        {
            int base=rng()%roots.size(),l=1+rng()%n,r=1+rng()%n;if(l>r)swap(l,r);
            if(op%8==0){l=1;r=n;}LL delta=(int)(rng()%101)-50;
            int old=s.tot;auto before=s.tr;
            roots.push_back(s.modify(roots[base],l,r,{delta}));
            auto next=snaps[base];for(int i=l;i<=r;i++)next[i]+=delta;snaps.push_back(next);
            for(int i=0;i<=old;i++){const auto& x=before[i];const auto& y=s.tr[i];assert(x.lc==y.lc&&x.rc==y.rc&&x.info.len==y.info.len&&x.info.sum==y.info.sum&&x.info.mx==y.info.mx&&x.tag.add==y.tag.add);}
            for(int j=0;j<3;j++)
            {
                int version=rng()%roots.size();l=1+rng()%n;r=1+rng()%n;if(l>r)swap(l,r);
                LL sum=0,mx=LLONG_MIN;for(int i=l;i<=r;i++){sum+=snaps[version][i];mx=max(mx,snaps[version][i]);}
                old=s.tot;auto got=s.query(roots[version],l,r);assert(got.len==r-l+1&&got.sum==sum&&got.mx==mx);
                LL threshold=(int)(rng()%201)-100;auto pred=[threshold](const InfoB& v){return v.mx>=threshold;};
                int first=-1,last=-1;for(int i=l;i<=n;i++)if(snaps[version][i]>=threshold){first=i;break;}
                for(int i=r;i>=1;i--)if(snaps[version][i]>=threshold){last=i;break;}
                assert(s.find_first(roots[version],l,pred)==first&&s.find_last(roots[version],r,pred)==last);
                assert(s.find_first(roots[version],0,pred)==-1&&s.find_last(roots[version],n+1,pred)==-1&&s.tot==old);
            }
        }
        // build appends another baseline; earlier versions of the same domain remain valid.
        int more=s.build(a);assert(s.query(more,1,n).sum==s.query(roots[1],1,n).sum);
    }
}
void test_path_kth()
{
    mt19937 rng(42);PersSegTree<InfoA,TagA> s(31,4000);
    for(int tc=0;tc<400;tc++)
    {
        s.clear();int n=1+rng()%60;VI par(n+1),depth(n+1),value(n+1),rt(n+1);
        for(int i=1;i<=n;i++){par[i]=i==1?0:1+rng()%(i-1);depth[i]=depth[par[i]]+1;value[i]=1+rng()%31;rt[i]=s.modify(rt[par[i]],value[i],value[i],{1});}
        for(int q=0;q<100;q++)
        {
            int u=1+rng()%n,v=1+rng()%n,x=u,y=v;VI path;
            while(x!=y){if(depth[x]<depth[y])swap(x,y);path.push_back(value[x]);x=par[x];}
            path.push_back(value[x]);sort(path.begin(),path.end());
            int count=s.tot;VI plus{rt[u],rt[v]},minus{rt[x],rt[par[x]]};
            for(int k:{1,(int)path.size(),1+(int)(rng()%path.size())})assert(s.find_kth(plus,minus,k)==path[k-1]);
            assert(s.tot==count&&plus[0]==rt[u]&&minus[0]==rt[x]);
        }
    }
}
void test_large_kth()
{
    constexpr int n=200000;PersSegTree<InfoA,TagA> s(n,4000010);mt19937 rng(42);VI rt(n+1);
    for(int mode=0;mode<3;mode++)
    {
        s.clear();s.set_n(mode==2?1:n);rt[0]=0;
        for(int i=1;i<=n;i++){int p=mode==0?i:mode==1?n+1-i:1;rt[i]=s.modify(rt[i-1],p,p,{1});}
        int used=s.tot;
        for(int q=0;q<200000;q++)
        {
            int l=1+rng()%n,r=1+rng()%n;if(l>r)swap(l,r);int k=1+rng()%(r-l+1);
            LL expected=mode==0?l+k-1:mode==1?n-r+k:1;
            assert(s.find_kth({rt[r]},{rt[l-1]},k)==expected);
            assert(s.query(rt[r],1,s.n).cnt==r&&s.tot==used);
        }
        cout<<"large kth mode="<<mode<<" nodes="<<used<<" bytes="<<sizeof(s.tr[0])<<'\n';
    }
    PersSegTree<InfoA,TagA> exact(1,3);int r=0;
    for(int cycle=0;cycle<100;cycle++){exact.clear();r=0;for(int i=1;i<=3;i++)r=exact.modify(r,1,1,{1});assert(exact.tot==3&&exact.find_kth({r},{},3)==1);}
}
void test_large_ranges()
{
    constexpr int n=200000,q=100000;PersSegTree<InfoB,TagB> s(n,8000010);mt19937 rng(42);
    struct Branch{int root,l,r;LL base,delta;};
    vector<int> uniform(q+1);vector<Branch> branches{{0,1,n,0,0}};
    for(int i=1;i<=q;i++)
    {
        uniform[i]=s.modify(uniform[i-1],1,n,{1});
        int version=rng()%i,l=1+rng()%n,r=1+rng()%n;if(l>r)swap(l,r);LL delta=(int)(rng()%2001)-1000;
        branches.push_back({s.modify(uniform[version],l,r,{delta}),l,r,version,delta});
        const auto& b=branches[rng()%branches.size()];l=1+rng()%n;r=1+rng()%n;if(l>r)swap(l,r);
        LL overlap=max(0,min(r,b.r)-max(l,b.l)+1),len=r-l+1;
        LL mx=overlap==0?b.base:overlap==len?b.base+b.delta:max(b.base,b.base+b.delta);
        int used=s.tot;auto got=s.query(b.root,l,r);assert(got.len==len&&got.sum==b.base*len+b.delta*overlap&&got.mx==mx);
        LL threshold=b.base+(int)(rng()%2001)-1000;auto pred=[threshold](const InfoB& v){return v.mx>=threshold;};
        int first=-1,last=-1;
        for(auto [lo,hi,val]:vector<tuple<int,int,LL>>{{1,b.l-1,b.base},{b.l,b.r,b.base+b.delta},{b.r+1,n,b.base}})
        {
            if(val<threshold||lo>hi)continue;
            int f=max(l,lo),t=min(r,hi);if(f<=hi&&(first==-1||f<first))first=f;if(t>=lo)last=max(last,t);
        }
        assert(s.find_first(b.root,l,pred)==first&&s.find_last(b.root,r,pred)==last&&s.tot==used);
        assert(s.query(uniform[version],1,n).sum==(LL)version*n);
    }
    cout<<"large ranges: versions="<<2*q<<" nodes="<<s.tot<<" bytes="<<sizeof(s.tr[0])<<'\n';
    auto capacity=s.tr.capacity();s.clear();s.set_n(1);int rt=s.modify(0,1,1,{7});assert(s.query(rt,1,1).sum==7&&s.tr.capacity()==capacity);
    s.clear();s.set_n(n);assert(s.query(0,1,n).sum==0&&s.query(0,1,n).len==n);
}
void test_wide_pst()
{
    PersSegTree<InfoB,TagB> s(LLONG_MAX,256);
    auto pred=[](const InfoB& v){return v.mx>=0;};
    assert(s.query(0,1,s.n).len==LLONG_MAX&&s.find_last(0,s.n,pred)==s.n&&!s.tot);
    int rt=s.modify(0,s.n,s.n,{-7});assert(s.query(rt,s.n,s.n).sum==-7);
    assert(s.find_last(rt,s.n,pred)==s.n-1&&s.query(0,s.n,s.n).sum==0);
    s.clear();s.set_n(1);rt=s.modify(0,1,1,{LLONG_MAX});assert(s.query(rt,1,1).sum==LLONG_MAX);
    s.clear();rt=s.modify(0,1,1,{LLONG_MIN});assert(s.query(rt,1,1).sum==LLONG_MIN);
}

int main()
{
    test_branch_snapshots();
    test_path_kth();
    test_large_kth();
    test_large_ranges();
    test_wide_pst();
    test_pers_seg_tree();
    return 0;
}
