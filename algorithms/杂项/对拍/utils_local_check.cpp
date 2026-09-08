#define LOCAL
#include <sstream>
#include "../utils/utils.cpp"
#include "../utils/utils.cpp"

int main()
{
    ostringstream log;
    auto* saved = cerr.rdbuf(log.rdbuf());
    int x = 7;
    VI a{0, 1, -2, 3};
    debug(x, a.size());
    debug_array(a, 3);
    cerr.rdbuf(saved);
    assert(log.str() == "x, a.size() = 7, 4\na: 1 -2 3 \n");
    cout << "utils_local_check passed\n";
}
