struct Tag 
{ 
    LL ow = -INF; 
    void apply(const Tag& t) 
    { 
        if (t.ow != -INF) ow = t.ow;
    } 
};
struct Info 
{
    LL len = 0;
    LL sum = 0;
    void apply(const Tag& t) { if (t.ow != -INF) sum = t.ow; }
    friend Info operator+(const Info& a, const Info& b) 
    {
        if (a.len == 0) return b;
        if (b.len == 0) return a;
        Info r;
        r.len = a.len + b.len;
        r.sum = a.sum + b.sum;
        return r;
    }
};