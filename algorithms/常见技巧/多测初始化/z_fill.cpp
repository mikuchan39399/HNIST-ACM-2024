template<typename... Cs>
void z_fill_n(int n, int val, Cs&... containers) 
{
    (fill(containers.begin(), containers.begin() + min((size_t)(n + 10), containers.size()), val), ...);
}

/* Usage:
    z_fill_n(n, 0, head, t_head, dfn, low, bel, ...);
*/