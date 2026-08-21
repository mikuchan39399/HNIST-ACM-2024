#include <cassert>

#ifndef Z_OI_ZFILLN
#define Z_OI_ZFILLN
template<typename... CS>
void z_fill_n(int n, int val, CS&... cs) 
{
    assert(((((int)cs.size()) >= n) && ...));
    (fill(cs.begin(), cs.begin() + min((size_t)(n + 10), cs.size()), val), ...);
}
#endif
/* Usage:
    z_fill_n(n, 0, head, t_head, dfn, low, bel, ...);
*/