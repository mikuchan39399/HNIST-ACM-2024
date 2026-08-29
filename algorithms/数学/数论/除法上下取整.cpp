using LL = long long;

inline LL floor_div(LL a, LL b)
{
    LL res = a / b;
    LL rem = a % b;
    if (rem != 0 && ((a < 0) ^ (b < 0)))
    {
        res--;
    }
    return res;
}

inline LL ceil_div(LL a, LL b)
{
    LL res = a / b;
    LL rem = a % b;
    if (rem != 0 && ((a > 0) == (b > 0)))
    {
        res++;
    }
    return res;
}
