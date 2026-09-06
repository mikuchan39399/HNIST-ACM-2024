// ============ 有序表与平衡树家族对拍套件 ============
// 覆盖: AVL/Treap/FHQ_Treap/SGTree/Splay vs std::multiset, SkipList vs std::set 独立比对
//       笛卡尔树: 随机数组建树三性质自证(中序=下标/小根堆/子树根=区间最小值)
//       (插/删/rank/kth/pre/suf/size 300 组, 窄/宽值域交替, kth 全序走查,
//        Treap/FHQ_Treap 笛卡尔树线性建树域: sort 后 build 全量对账) |
//       FHQ_Seq 双域: 仿射域(单批插/删/加/乘/翻/移/和/RMQ最值) + 覆盖域
//       (P2042 口径: 批插/删/覆盖/翻/和/最大子段和(非空段)/移) vs vector 暴力 |
//       同场共编全家族(守卫幂等) | clear 复用 | ±INF 边界
// 纪律: 改动任一模板, 必重跑本套件
// 跑法: g++ -std=c++20 -Wall -Wextra -O2 oset_check.cpp -o oset_check && ./oset_check
#include "../AVL.cpp"
#include "../Treap.cpp"
#include "../FHQ_Treap.cpp"
#include "../替罪羊树.cpp"
#include "../Splay.cpp"
#include "../FHQ_Treap_序列.cpp"
#include "../跳表.cpp"
#include "../笛卡尔树.cpp"
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <random>
#include <set>
#include <vector>

// 笛卡尔树自证: 递归一遍, 查四件事 —— 每点至多一左一右(按编号分侧),
// 中序位置对得上自身下号, 堆方向不破, 子树根是子树那段的最/大值
static int ct_verify(int u, int fa, int lo, const VLL& a, Cartesian& ct, bool mn)
{
    int lch = 0, rch = 0;
    for (auto& e : ct.tree[u])
    {
        if (e.v == fa) continue;
        if (e.v < u) { assert(!lch); lch = e.v; }
        else { assert(!rch); rch = e.v; }
    }
    int ls = lch ? ct_verify(lch, u, lo, a, ct, mn) : 0;
    int rs = rch ? ct_verify(rch, u, lo + ls + 1, a, ct, mn) : 0;
    int l = lo, r = lo + ls + rs;
    LL key = a[l];
    for (int j = l; j <= r; j++) key = mn ? min(key, a[j]) : max(key, a[j]);
    assert(lo + ls == u);
    assert(a[u] == key);
    for (auto& e : ct.tree[u])
    {
        if (e.v == fa) continue;
        assert(mn ? a[u] <= a[e.v] : a[u] >= a[e.v]);
    }
    return ls + rs + 1;
}

// 顺序插入 BST 参照: 等值往右(false: x < v 走左) / 等值往左(true: x <= v 走左)
static void bst_ref(const VLL& a, VI& lcp, VI& rcp, bool eq_left)
{
    int n = (int)a.size() - 1;
    lcp.assign(n + 1, 0);
    rcp.assign(n + 1, 0);
    for (int i = 2; i <= n; i++)
    {
        int cur = 1;
        while (true)
        {
            bool goleft = eq_left ? (a[i] <= a[cur]) : (a[i] < a[cur]);
            int& slot = goleft ? lcp[cur] : rcp[cur];
            if (!slot) { slot = i; break; }
            cur = slot;
        }
    }
}

struct CountRef
{
    int n; VI bit,cnt;
    CountRef(int n):n(n),bit(n+1),cnt(n+1){}
    void add(int x,int d){cnt[x]+=d;for(int p=x;p<=n;p+=p&-p)bit[p]+=d;}
    int sum(int x){int v=0;for(;x;x-=x&-x)v+=bit[x];return v;}
    int kth(int k){int p=0;for(int b=1<<18;b;b>>=1)if(p+b<=n&&bit[p+b]<k){p+=b;k-=bit[p];}return p+1;}
};
template<class T, bool Unique=false>
void large_set(const char* name)
{
    constexpr int n=200000;
    T s(400010); mt19937 rng(42);
    for(int mode=0;mode<3;mode++)
    {
        s.clear(); CountRef ref(n);
        for(int i=1;i<=n;i++)
        {
            int p=mode==0?i:mode==1?n+1-i:1+(i%64);
            s.insert(p);
            if(!Unique||!ref.cnt[p])ref.add(p,1);
        }
        for(int q=0;q<100000;q++)
        {
            int p=1+rng()%n;
            if(q%3==0){bool had=ref.cnt[p]>0;auto got=s.erase(p);if constexpr(Unique)assert((got!=-1)==had);else assert(got==had);if(had)ref.add(p,-1);}
            else if(q%3==1){s.insert(p);if(!Unique||!ref.cnt[p])ref.add(p,1);}
            int rank=ref.sum(p-1),all=ref.sum(n),upto=ref.sum(p);
            assert(s.size()==all && s.get_rank(p)==rank);
            assert(s.get_pre(p)==(rank?ref.kth(rank):-INF));
            assert(s.get_suf(p)==(upto<all?ref.kth(upto+1):INF));
            if(all){int k=1+rng()%all;assert(s.get_kth(k)==ref.kth(k));}
            assert(s.get_kth(0)==INF && s.get_kth(all+1)==INF);
        }
    }
    s.clear();
    for(LL v:{-INF+1,0LL,INF-1})s.insert(v);
    assert(s.get_pre(-INF+1)==-INF&&s.get_suf(INF-1)==INF);
    assert(s.get_kth(1)==-INF+1&&s.get_kth(3)==INF-1);
    s.clear();assert(!s.size()&&s.get_pre(0)==-INF&&s.get_suf(0)==INF);
    printf("large set %s passed\n",name);
}
void extra_sets()
{
    large_set<AVL>("AVL");large_set<Treap>("Treap");large_set<FHQ_Treap>("FHQ");
    large_set<SGTree>("SGTree");large_set<Splay>("Splay");large_set<SkipList<20>,true>("SkipList");
    constexpr int n=200000;
    Splay sp(n);
    for(int mode=0;mode<2;mode++)
    {
        sp.clear();for(int i=1;i<=n;i++)sp.insert(mode?n-i+1:i);
        for(int q=0;q<100000;q++)assert(mode?sp.get_suf(n+1)==INF:sp.get_pre(0)==-INF);
    }
    Treap tr(n);FHQ_Treap fh(n);
    for(int m:{n,1,0,65537,n})
    {
        VLL a(m+1);for(int i=1;i<=m;i++)a[i]=(i-1)/3;
        tr.build(a);fh.build(a);sp.build(a);
        for(int k=1;k<=m;k++)assert(tr.get_kth(k)==a[k]&&fh.get_kth(k)==a[k]&&sp.get_kth(k)==a[k]);
        for(LL v:{-INF,0LL,12345LL,INF})
        {
            int x,y;fh.split(fh.root,v,x,y);
            int k=upper_bound(a.begin()+1,a.end(),v)-(a.begin()+1);
            assert(fh.tr[x].sz==k&&fh.tr[y].sz==m-k);
            if(k)assert(fh.kth_of(x,k)==a[k]);
            fh.root=fh.merge(x,y);assert(fh.size()==m);
        }
    }
    SGTree sg(100);SkipList<20> sl(100);FHQ_Seq sq(100);
    for(int cycle=0;cycle<300;cycle++)
    {
        for(int i=1;i<=100;i++){sg.insert(i);sl.insert(i);sq.insert(i,i);}
        for(int i=1;i<=100;i++){assert(sg.erase(i));assert(sl.erase(i)>0);}
        sq.erase(1,100);assert(!sq.size()&&sg.idx<=100&&sl.tot<=100&&sq.idx<=100);
    }
}
void extra_cartesian()
{
    constexpr int n=200000;Cartesian ct;mt19937 rng(42);
    for(int mode=0;mode<4;mode++)for(bool mn:{true,false})
    {
        VLL a(n+1);for(int i=1;i<=n;i++)a[i]=mode==0?i:mode==1?-i:mode==2?7:(LL)rng();
        int rt=ct.build(a,mn);VI parent(n+1),order{rt};
        for(size_t k=0;k<order.size();k++)for(auto e:ct.tree[order[k]])if(e.v!=parent[order[k]]){assert(!parent[e.v]&&e.v!=rt);parent[e.v]=order[k];order.push_back(e.v);}
        assert(order.size()==n && ct.tree.edge_cnt()==n-1);
        VI lo(n+1),hi(n+1),sz(n+1);for(int i=1;i<=n;i++)lo[i]=hi[i]=i;
        for(auto it=order.rbegin();it!=order.rend();++it)
        {
            int u=*it;sz[u]++;assert(hi[u]-lo[u]+1==sz[u]);
            if(parent[u]){int p=parent[u];assert(mn?a[p]<=a[u]:a[p]>=a[u]);lo[p]=min(lo[p],lo[u]);hi[p]=max(hi[p],hi[u]);sz[p]+=sz[u];}
        }
        LL extreme=mn?*min_element(a.begin()+1,a.end()):*max_element(a.begin()+1,a.end());assert(a[rt]==extreme);
    }
    for(int m:{n,1,0,2,n})for(int mode=0;mode<3;mode++)for(bool eq:{false,true})
    {
        VLL a(m+1);for(int i=1;i<=m;i++)a[i]=mode==0?i:mode==1?-i:7;
        int rt=eq?ct.build_bst<true>(a):ct.build_bst<false>(a);
        assert(ct.n==m&&ct.tree.edge_cnt()==max(0,m-1));
        if(m)assert(ct.orig[rt]==1);else assert(!rt);
        for(size_t i=0;i<ct.tree.edges.size();i+=2){int x=ct.orig[ct.tree.edges[i].v],y=ct.orig[ct.tree.edges[i+1].v];assert(abs(x-y)==1);}
        for(int i=1;i<=m;i++)assert(ct.key[i]==a[ct.orig[i]]);
    }
}
void extra_seq()
{
    FHQ_Seq s(200000);mt19937 rng(42);
    for(LL value:{LLONG_MIN,LLONG_MIN/2,LLONG_MAX})
    {
        s.build(VLL{0,value});assert(s.get_max_sum()==value&&s.get_sum(1,1)==value);
        s.assign(1,1,value);assert(s.get_max_sum(1,1)==value);
    }
    constexpr int n=200000;VLL a(n+1);for(int i=1;i<=n;i++)a[i]=i;
    s.build(a);LL shift=0,dir=1,sign=1,bias=0;
    auto mod=[](LL x){x%=n;if(x<0)x+=n;return x;};
    for(int q=0;q<200000;q++)
    {
        int op=q%4;
        if(op==0){s.reverse(1,n);shift=mod(shift+dir*(n-1));dir=-dir;}
        if(op==1){int k=1+rng()%(n-1);s.move_interval(1,k,n-k);shift=mod(shift+dir*k);}
        if(op==2){s.mul(1,n,-1);sign=-sign;bias=-bias;}
        if(op==3){s.modify(1,n,3);bias+=3;}
        int l=1+rng()%n,r=1+rng()%n;if(l>r)swap(l,r);
        LL len=r-l+1,start=mod(shift+dir*(l-1));if(dir<0)start=mod(start-len+1);
        LL take=min(len,n-start),rest=len-take;
        LL sum=(start+1+start+take)*take/2+rest*(rest+1)/2;
        LL low=rest?1:start+1,high=rest?n:start+len;
        assert(s.get_sum(l,r)==sign*sum+bias*len);
        assert(s.get_min(l,r)==min(sign*low+bias,sign*high+bias));
        assert(s.get_max(l,r)==max(sign*low+bias,sign*high+bias));
    }
    s.assign(1,n,-3);assert(s.get_max_sum()==-3);s.assign(2,3,7);assert(s.get_max_sum()==14);
    for(int cycle=0;cycle<3;cycle++)
    {
        s.erase(1,s.size());s.insert(1,VLL(n,2));assert(s.size()==n&&s.idx<=n&&s.get_max_sum()==2LL*n);
        int x,y;s.split_rank(s.root,n/2,x,y);VLL out{-99};s.walk(x,out);assert(out.size()==n/2+1&&out.front()==-99&&out.back()==2);s.root=s.merge(x,y);
    }
    for(int m:{1,0,65537,n}){s.build(VLL(m+1,-2));assert(s.size()==m);if(m)assert(s.get_max_sum()==-2);}
}

int main()
{
    extra_sets();
    extra_cartesian();
    extra_seq();
    mt19937 par(42);
    for (int t = 0; t < 300; t++)
    {
        LL mod = (t & 1) ? 41 : 100001;
        LL off = (t & 1) ? -20 : -50000;
        AVL avl(1010);
        Treap trp(1010);
        FHQ_Treap fhq(1010);
        SGTree sgt(1010);
        Splay sp(1010);
        SkipList<5> sl(1010);
        multiset<LL> mref;
        set<LL> sref;
        // 笛卡尔树线性建树域: 随机数组(带重复) sort 后 build, 全量对账
        {
            int m0 = par() % 50;
            vector<LL> b(m0 + 1);
            for (int j = 1; j <= m0; j++) b[j] = (LL)(par() % mod) + off;
            sort(b.begin() + 1, b.end());
            trp.build(b);
            fhq.build(b);
            sp.build(b);
            for (int j = 1; j <= m0; j++)
            {
                avl.insert(b[j]);
                sgt.insert(b[j]);
                sl.insert(b[j]);
                sref.insert(b[j]);
            }
            mref = multiset<LL>(b.begin() + 1, b.end());
            assert(trp.size() == m0 && fhq.size() == m0 && sp.size() == m0);
            for (int j = 1; j <= m0; j++)
            {
                assert(trp.get_kth(j) == b[j]);
                assert(fhq.get_kth(j) == b[j]);
                assert(sp.get_kth(j) == b[j]);
            }
            assert(trp.get_kth(m0 + 1) == INF && fhq.get_kth(m0 + 1) == INF && sp.get_kth(m0 + 1) == INF);
        }
        // 笛卡尔树域: 随机数组(未排序, 窄值域带重复)双堆型建树, 三性质自证 + 图视图边数
        {
            int m0 = par() % 60;
            vector<LL> b(m0 + 1);
            for (int j = 1; j <= m0; j++) b[j] = (LL)(par() % 37) - 18;
            Cartesian ct(70);
            int rt = ct.build(b);
            assert((rt != 0) == (m0 > 0));
            if (m0)
            {
                ct_verify(rt, 0, 1, b, ct, true);
                assert(ct.tree.edge_cnt() == m0 - 1);
                if (m0 > 1) assert(ct.tree.node_cnt() == m0);
                int rt2 = ct.build(b, false);
                ct_verify(rt2, 0, 1, b, ct, false);
                assert(ct.tree.edge_cnt() == m0 - 1);
                if (m0 > 1) assert(ct.tree.node_cnt() == m0);
            }
        }
        // 顺序插入 BST 域: 带重复值, 双等值约定对拍朴素插入
        {
            int m0 = par() % 40;
            vector<LL> c(m0 + 1);
            for (int j = 1; j <= m0; j++) c[j] = (LL)(par() % 11) - 5;
            for (int conv = 0; conv < 2; conv++)
            {
                bool eq_left = (conv == 1);
                VI lcp, rcp;
                bst_ref(c, lcp, rcp, eq_left);
                set<pair<int, int>> e1;
                for (int j = 1; j <= m0; j++)
                {
                    if (lcp[j]) e1.insert({min(j, lcp[j]), max(j, lcp[j])});
                    if (rcp[j]) e1.insert({min(j, rcp[j]), max(j, rcp[j])});
                }
                Cartesian cb(m0 + 1);
                int root = eq_left ? cb.build_bst<true>(c) : cb.build_bst<false>(c);
                assert((root != 0) == (m0 > 0));
                if (m0)
                {
                    assert(cb.orig[root] == 1);   // 根 = 第一个插入的
                    set<pair<int, int>> e2;
                    for (size_t k = 0; k + 1 < cb.tree.edges.size(); k += 2)
                    {
                        int x = cb.tree.edges[k + 1].v, y = cb.tree.edges[k].v;
                        int ox = cb.orig[x], oy = cb.orig[y];
                        e2.insert({min(ox, oy), max(ox, oy)});
                    }
                    assert(e1 == e2);
                }
            }
        }
        int ops = 100 + par() % 400;
        for (int i = 0; i < ops; i++)
        {
            LL x = (LL)(par() % mod) + off;
            int op = par() % 6;
            if (op == 0)
            {
                avl.insert(x);
                trp.insert(x);
                fhq.insert(x);
                sgt.insert(x);
                sp.insert(x);
                mref.insert(x);
                bool had = sref.count(x) > 0;
                int b = sl.size();
                sl.insert(x);
                sref.insert(x);
                if (had) assert(sl.size() == b);
                else assert(sl.size() == b + 1);
            }
            else if (op == 1)
            {
                auto it = mref.find(x);
                bool rm = it != mref.end();
                if (rm) mref.erase(it);
                assert(avl.erase(x) == rm);
                assert(trp.erase(x) == rm);
                assert(fhq.erase(x) == rm);
                assert(sgt.erase(x) == rm);
                assert(sp.erase(x) == rm);
                assert((sl.erase(x) != -1) == (sref.erase(x) > 0));
            }
            else if (op == 2)
            {
                int expect = (int)distance(mref.begin(), mref.lower_bound(x));
                assert(avl.get_rank(x) == expect);
                assert(trp.get_rank(x) == expect);
                assert(fhq.get_rank(x) == expect);
                assert(sgt.get_rank(x) == expect);
                assert(sp.get_rank(x) == expect);
                assert(sl.get_rank(x) == (int)distance(sref.begin(), sref.lower_bound(x)));
            }
            else if (op == 3 && !mref.empty())
            {
                int k = 1 + par() % (int)mref.size();
                LL expect = *next(mref.begin(), k - 1);
                assert(avl.get_kth(k) == expect);
                assert(trp.get_kth(k) == expect);
                assert(fhq.get_kth(k) == expect);
                assert(sgt.get_kth(k) == expect);
                assert(sp.get_kth(k) == expect);
                int k2 = 1 + par() % (int)sref.size();
                assert(sl.get_kth(k2) == *next(sref.begin(), k2 - 1));
            }
            else if (op == 4)
            {
                auto it = mref.lower_bound(x);
                LL expect = (it == mref.begin()) ? -INF : *prev(it);
                assert(avl.get_pre(x) == expect);
                assert(trp.get_pre(x) == expect);
                assert(fhq.get_pre(x) == expect);
                assert(sgt.get_pre(x) == expect);
                assert(sp.get_pre(x) == expect);
                auto it2 = sref.lower_bound(x);
                LL expect2 = (it2 == sref.begin()) ? -INF : *prev(it2);
                assert(sl.get_pre(x) == expect2);
            }
            else
            {
                auto it = mref.upper_bound(x);
                LL expect = (it == mref.end()) ? INF : *it;
                assert(avl.get_suf(x) == expect);
                assert(trp.get_suf(x) == expect);
                assert(fhq.get_suf(x) == expect);
                assert(sgt.get_suf(x) == expect);
                assert(sp.get_suf(x) == expect);
                auto it2 = sref.upper_bound(x);
                LL expect2 = (it2 == sref.end()) ? INF : *it2;
                assert(sl.get_suf(x) == expect2);
            }
            assert(avl.size() == (int)mref.size());
            assert(trp.size() == (int)mref.size());
            assert(fhq.size() == (int)mref.size());
            assert(sgt.size() == (int)mref.size());
            assert(sp.size() == (int)mref.size());
            assert(sl.size() == (int)sref.size());
        }
        int k = 1;
        for (LL e : mref)
        {
            assert(avl.get_kth(k) == e);
            assert(trp.get_kth(k) == e);
            assert(fhq.get_kth(k) == e);
            assert(sgt.get_kth(k) == e);
            assert(sp.get_kth(k) == e);
            k++;
        }
        assert(avl.get_kth(k) == INF);
        assert(trp.get_kth(k) == INF);
        assert(fhq.get_kth(k) == INF);
        assert(sgt.get_kth(k) == INF);
        assert(sp.get_kth(k) == INF);
        int k2 = 1;
        for (LL e : sref)
        {
            assert(sl.get_kth(k2) == e);
            k2++;
        }
        assert(sl.get_kth(k2) == INF && sl.get_kth(0) == INF);
        avl.clear();
        trp.clear();
        fhq.clear();
        sgt.clear();
        sp.clear();
        sl.clear();
        assert(avl.size() == 0 && trp.size() == 0 && fhq.size() == 0 && sgt.size() == 0 && sp.size() == 0 && sl.size() == 0);
        assert(avl.get_pre(0) == -INF && trp.get_suf(0) == INF);
        assert(fhq.get_pre(0) == -INF && sl.get_suf(0) == INF);
        assert(sgt.get_pre(0) == -INF && sp.get_suf(0) == INF);
        assert(sp.get_pre(0) == -INF && sp.get_suf(0) == INF);
        assert(avl.get_kth(1) == INF && trp.get_kth(1) == INF && fhq.get_kth(1) == INF && sgt.get_kth(1) == INF && sp.get_kth(1) == INF && sl.get_kth(1) == INF);
        avl.insert(7);
        trp.insert(7);
        fhq.insert(7);
        sgt.insert(7);
        sp.insert(7);
        sl.insert(7);
        assert(avl.get_kth(1) == 7 && trp.get_kth(1) == 7 && fhq.get_kth(1) == 7 && sgt.get_kth(1) == 7 && sp.get_kth(1) == 7);
        assert(avl.get_rank(7) == 0 && trp.get_rank(7) == 0 && fhq.get_rank(7) == 0 && sgt.get_rank(7) == 0 && sp.get_rank(7) == 0);
    }
    for (int t = 0; t < 300; t++)
    {
        FHQ_Seq s(1010);
        int m0 = par() % 50;
        vector<LL> a(m0 + 1);
        for (int j = 1; j <= m0; j++) a[j] = (LL)(par() % 1001) - 500;
        s.build(a);
        vector<LL> ref(a.begin() + 1, a.end());
        assert(s.size() == m0);
        int ops = 60 + par() % 140;
        for (int i = 0; i < ops; i++)
        {
            int m = (int)ref.size();
            int op = par() % 7;
            if (op == 0 || m == 0)
            {
                if (par() % 2)
                {
                    int pos = 1 + par() % (m + 1);
                    LL v = (LL)(par() % 1001) - 500;
                    s.insert(pos, v);
                    ref.insert(ref.begin() + pos - 1, v);
                }
                else
                {
                    int k = 1 + par() % 5;
                    int pos = 1 + par() % (m + 1);
                    vector<LL> b(k);
                    for (int j = 0; j < k; j++) b[j] = (LL)(par() % 1001) - 500;
                    s.insert(pos, b);
                    ref.insert(ref.begin() + pos - 1, b.begin(), b.end());
                }
            }
            else
            {
                int l = 1 + par() % m;
                int r = l + par() % (m - l + 1);
                if (op == 1)
                {
                    LL d = (LL)(par() % 11) - 5;
                    s.modify(l, r, d);
                    for (int j = l - 1; j < r; j++) ref[j] += d;
                }
                else if (op == 2)
                {
                    LL mx = 0;
                    for (LL e : ref)
                    {
                        LL ae = e < 0 ? -e : e;
                        if (ae > mx) mx = ae;
                    }
                    if (mx < (LL)1e15)
                    {
                        LL mm = (LL)(par() % 5) - 1;
                        s.mul(l, r, mm);
                        for (int j = l - 1; j < r; j++) ref[j] *= mm;
                    }
                    else
                    {
                        LL d = (LL)(par() % 11) - 5;
                        s.modify(l, r, d);
                        for (int j = l - 1; j < r; j++) ref[j] += d;
                    }
                }
                else if (op == 3)
                {
                    LL e = 0, hi = ref[l - 1], lo = ref[l - 1];
                    for (int j = l - 1; j < r; j++)
                    {
                        e += ref[j];
                        if (ref[j] > hi) hi = ref[j];
                        if (ref[j] < lo) lo = ref[j];
                    }
                    assert(s.get_sum(l, r) == e);
                    assert(s.get_max(l, r) == hi);
                    assert(s.get_min(l, r) == lo);
                }
                else if (op == 4)
                {
                    s.reverse(l, r);
                    reverse(ref.begin() + l - 1, ref.begin() + r);
                }
                else if (op == 5)
                {
                    s.erase(l, r);
                    ref.erase(ref.begin() + l - 1, ref.begin() + r);
                }
                else
                {
                    int len = r - l + 1;
                    int pos = par() % (m - len + 1);
                    s.move_interval(l, r, pos);
                    vector<LL> cut(ref.begin() + l - 1, ref.begin() + r);
                    ref.erase(ref.begin() + l - 1, ref.begin() + r);
                    ref.insert(ref.begin() + pos, cut.begin(), cut.end());
                }
            }
            assert(s.size() == (int)ref.size());
        }
        vector<LL> out;
        s.collect(out);
        assert(out == ref);
        if (!ref.empty())
        {
            assert(s.get_max() == *max_element(ref.begin(), ref.end()));
            assert(s.get_min() == *min_element(ref.begin(), ref.end()));
        }
        s.clear();
        assert(s.size() == 0);
    }
    for (int t = 0; t < 300; t++)
    {
        FHQ_Seq s(1010);
        int m0 = 1 + par() % 40;
        vector<LL> a(m0 + 1);
        for (int j = 1; j <= m0; j++) a[j] = (LL)(par() % 41) - 30;
        s.build(a);
        vector<LL> ref(a.begin() + 1, a.end());
        int ops = 60 + par() % 140;
        for (int i = 0; i < ops; i++)
        {
            int m = (int)ref.size();
            int op = par() % 7;
            if (op == 0 || m == 0)
            {
                int k = 1 + par() % 5;
                int pos = par() % (m + 1);
                vector<LL> b(k);
                for (int j = 0; j < k; j++) b[j] = (LL)(par() % 41) - 30;
                s.insert(pos + 1, b);
                ref.insert(ref.begin() + pos, b.begin(), b.end());
            }
            else
            {
                int l = 1 + par() % m;
                int r = l + par() % (m - l + 1);
                if (op == 1)
                {
                    LL v = (LL)(par() % 21) - 10;
                    s.assign(l, r, v);
                    for (int j = l - 1; j < r; j++) ref[j] = v;
                }
                else if (op == 2)
                {
                    s.reverse(l, r);
                    reverse(ref.begin() + l - 1, ref.begin() + r);
                }
                else if (op == 3)
                {
                    s.erase(l, r);
                    ref.erase(ref.begin() + l - 1, ref.begin() + r);
                }
                else if (op == 4)
                {
                    LL e = 0, hi = ref[l - 1], lo = ref[l - 1];
                    for (int j = l - 1; j < r; j++)
                    {
                        e += ref[j];
                        if (ref[j] > hi) hi = ref[j];
                        if (ref[j] < lo) lo = ref[j];
                    }
                    assert(s.get_sum(l, r) == e);
                    assert(s.get_max(l, r) == hi);
                    assert(s.get_min(l, r) == lo);
                }
                else if (op == 5)
                {
                    LL cur = ref[l - 1], mx = ref[l - 1];
                    for (int j = l; j < r; j++)
                    {
                        cur = max(ref[j], cur + ref[j]);
                        if (cur > mx) mx = cur;
                    }
                    assert(s.get_max_sum(l, r) == mx);
                }
                else
                {
                    int len = r - l + 1;
                    int pos = par() % (m - len + 1);
                    s.move_interval(l, r, pos);
                    vector<LL> cut(ref.begin() + l - 1, ref.begin() + r);
                    ref.erase(ref.begin() + l - 1, ref.begin() + r);
                    ref.insert(ref.begin() + pos, cut.begin(), cut.end());
                }
            }
            if (!ref.empty())
            {
                LL cur = ref[0], gmx = ref[0];
                for (size_t j = 1; j < ref.size(); j++)
                {
                    cur = max(ref[j], cur + ref[j]);
                    if (cur > gmx) gmx = cur;
                }
                assert(s.get_max_sum() == gmx);
            }
            assert(s.size() == (int)ref.size());
        }
        vector<LL> out;
        s.collect(out);
        assert(out == ref);
        s.clear();
        assert(s.size() == 0);
    }
    {
        FHQ_Seq s(100);
        vector<LL> a = {0, 5, -3, 2};
        s.build(a);
        s.modify(1, 3, -100);
        s.assign(1, 3, 4);
        assert(s.get_max_sum() == 12);
        s.reverse(1, 3);
        assert(s.get_max_sum() == 12);
        vector<LL> out;
        s.collect(out);
        assert((out == vector<LL>{4, 4, 4}));
    }
    {
        // 非空段口径: 全负时最大子段和 = 最大负值; RMQ 全域有效(含乘 -1)
        FHQ_Seq s(100);
        vector<LL> a = {0, -5, -3, -8};
        s.build(a);
        assert(s.get_max_sum() == -3);
        assert(s.get_max() == -3 && s.get_min() == -8);
        s.mul(1, 3, -1);
        assert(s.get_max() == 8 && s.get_min() == 3);
        s.assign(1, 3, 2);
        assert(s.get_max_sum() == 6);
    }
    {
        FHQ_Seq s(10);
        vector<LL> a(1);
        s.build(a);
        assert(s.size() == 0);
        a.push_back(42);
        s.build(a);
        vector<LL> out;
        s.collect(out);
        assert(out.size() == 1 && out[0] == 42 && s.get_sum(1, 1) == 42);
        vector<LL> b = {7, 8};
        s.insert(1, b);
        s.insert(4, b);
        out.clear();
        s.collect(out);
        assert((out == vector<LL>{7, 8, 42, 7, 8}));
        assert(s.get_max_sum() == 72);
        assert(s.get_max() == 42 && s.get_min() == 7);
        assert(s.get_max(1, 3) == 42 && s.get_min(1, 2) == 7);
    }
    AVL avl(10);
    Treap trp(10);
    FHQ_Treap fhq(10);
    SGTree sgt(10);
    FHQ_Seq seq(10);
    SkipList sl(10);
    avl.insert(-INF + 1);
    trp.insert(-INF + 1);
    fhq.insert(-INF + 1);
    sgt.insert(-INF + 1);
    assert(avl.get_pre(-INF + 1) == -INF);
    assert(trp.get_pre(-INF + 1) == -INF);
    assert(fhq.get_pre(-INF + 1) == -INF);
    assert(sgt.get_pre(-INF + 1) == -INF);
    int id = sl.insert(5);
    assert(id > 0 && sl.insert(5) == id);
    seq.insert(1, INF - 1);
    assert(seq.get_sum(1, 1) == INF - 1);
    puts("有序表与平衡树家族 check passed");
    return 0;
}
