// 独立快读写验收: 原生格式化/解析作参照, 真正重定向文件并逐字节比对
// 默认覆盖所有整数类型、浮点与文本、4 MiB 边界、百万整数、EOF 与复位
#include "../快读快写/快读快写.cpp"
#include <cassert>
#include <charconv>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <limits>
#include <random>
#include <sstream>
#include <vector>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

template<class F>
static string capture(const string& input, F f)
{
#ifdef _WIN32
    auto dup_fd = _dup; auto dup2_fd = _dup2; auto close_fd = _close; auto file_no = _fileno;
#else
    auto dup_fd = dup; auto dup2_fd = dup2; auto close_fd = close; auto file_no = fileno;
#endif
    const char* ip = "rw_check_input.bin", *op = "rw_check_output.bin";
    FILE* in = fopen(ip, "wb");
    assert(in && fwrite(input.data(), 1, input.size(), in) == input.size());
    fclose(in);
    utils_io::flush_io(); fflush(stdout);
    int si = dup_fd(file_no(stdin)), so = dup_fd(file_no(stdout));
    assert(si >= 0 && so >= 0);
    assert(freopen(ip, "rb", stdin) && freopen(op, "wb", stdout));
    // 测试更换底层文件后清自有缓存, 不是让调用方在正常读写时重置这些成员
    utils_io::ipos = utils_io::ilen = utils_io::olen = 0;
    f();
    utils_io::flush_io(); fflush(stdout);
    assert(dup2_fd(si, file_no(stdin)) >= 0 && dup2_fd(so, file_no(stdout)) >= 0);
    close_fd(si); close_fd(so); clearerr(stdin); clearerr(stdout);
    FILE* out = fopen(op, "rb"); assert(out);
    string result; char buf[8192]; size_t n;
    while ((n = fread(buf, 1, sizeof buf, out))) result.append(buf, n);
    fclose(out); remove(ip); remove(op);
    return result;
}

template<class T>
static string decimal(T v)
{
    char buf[64]; auto result = to_chars(buf, buf + sizeof buf, v);
    assert(result.ec == errc()); return string(buf, result.ptr);
}

template<class T>
static void integer_type(mt19937_64& rng)
{
    vector<T> values; values.reserve(3005);
    values.push_back(0); values.push_back(1);
    values.push_back(numeric_limits<T>::min()); values.push_back(numeric_limits<T>::max());
    if constexpr (numeric_limits<T>::is_signed) values.push_back(-1);
    for (int i = 0; i < 3000; ++i) values.push_back((T)rng());
    string input, expected;
    for (T v : values) { input += decimal(v) + "\r\n"; expected += decimal(v) + '\n'; }
    assert(capture(input, [&] {
        for (T v : values) { T got{}; assert(read(got) && got == v); write(got); }
        T eof = 1; assert(!read(eof) && eof == 1);
    }) == expected);
}

static void integer_edges()
{
    mt19937_64 rng(20260908);
    integer_type<signed char>(rng); integer_type<unsigned char>(rng);
    integer_type<short>(rng); integer_type<unsigned short>(rng);
    integer_type<int>(rng); integer_type<unsigned>(rng);
    integer_type<long>(rng); integer_type<unsigned long>(rng);
    integer_type<long long>(rng); integer_type<unsigned long long>(rng);
    using I = __int128; using U = unsigned __int128;
    I lo = -((I)1 << 126) - ((I)1 << 126), hi = -(lo + 1);
    string extremes = "-170141183460469231731687303715884105728 170141183460469231731687303715884105727 340282366920938463463374607431768211455";
    for (int split = 0; split <= 40; ++split)
    {
        string input(utils_io::BUFSZ - split, ' '); input += extremes;
        assert(capture(input, [&] {
            I a, b; U c;
            assert(read(a) && a == lo && read(b) && b == hi && read(c) && c == ~U(0));
            write(a, ' '); write(b, ' '); write(c, '\0');
        }) == extremes);
    }
    assert(capture("0 1\v\fZ", [] {
        bool a = true, b = false; char c = '?';
        assert(read(a) && !a && read(b) && b && read(c) && c == 'Z');
        write(a, ' '); write(b, ' '); write(c);
        assert(!read(c) && c == 'Z');
    }) == "0 1 Z\n");
}

static void floats()
{
    vector<string> tokens{"0", "-0", "+.5", "-.125", "1.", "1e-300", "-2.5E+30",
        "1.7976931348623157e308", "-1.7976931348623157e308", "2.2250738585072014e-308",
        "4.9406564584124654e-324", string(4096, '0') + "123.5", "1." + string(utils_io::BUFSZ + 17, '0')};
    mt19937_64 rng(7182);
    for (int i = 0; i < 10000; ++i)
    {
        unsigned long long bits = rng(); double d;
        static_assert(sizeof d == sizeof bits); memcpy(&d, &bits, sizeof d);
        if (!isfinite(d)) continue;
        ostringstream s; s << setprecision(17) << d; tokens.push_back(s.str());
    }
    string input(utils_io::BUFSZ - 1, ' '), expected;
    vector<double> values;
    for (const string& token : tokens)
    {
        input += token + '\n';
        double d = strtod(token.c_str(), nullptr); values.push_back(d);
        ostringstream s; s << fixed << setprecision(6) << d; expected += s.str() + '\n';
    }
    assert(capture(input, [&] {
        for (double want : values) {
            double got = 7; assert(read(got) && got == want && signbit(got) == signbit(want)); write(got);
        }
        double eof = 7; assert(!read(eof) && eof == 7);
    }) == expected);
}

static void text_and_bulk()
{
    for (int n : {0, 1, utils_io::BUFSZ - 1, utils_io::BUFSZ, utils_io::BUFSZ + 1, 2 * utils_io::BUFSZ + 17, 1})
    {
        string word(n, 'x'); if (n) word[n / 2] = (char)0xff;
        assert(capture(" \r\n\t\v\f" + word, [&] {
            string got = "unchanged";
            if (n) { assert(read(got) && got == word); write(got, '\0'); }
            else assert(!read(got) && got == "unchanged");
            assert(!read(got));
        }) == word);
    }
    assert(capture("\v\f Q\t\r\nR", [] {
        char c; assert(read(c) && c == 'Q'); write(c, ' ');
        assert(read(c) && c == 'R'); write(c, '\0');
        write("a", '\0'); write(string("b\0c", 3), '\0');
        utils_io::flush_io(); utils_io::flush_io();
    }) == string("Q Rab\0c", 7));
    constexpr int N = 1000000;
    mt19937_64 rng(813); vector<long long> values; values.reserve(N);
    string input, expected;
    for (int i = 0; i < N; ++i) {
        long long v = (long long)rng(); values.push_back(v);
        input += decimal(v) + (i % 2 ? "\r\n" : " "); expected += decimal(v) + '\n';
    }
    assert(capture(input, [&] {
        for (long long v : values) { long long got; assert(read(got) && got == v); write(got); }
        long long eof = 9; assert(!read(eof) && eof == 9);
    }) == expected);
}

int main(int argc, char** argv)
{
    if (argc == 2 && string(argv[1]) == "--exit-flush") {
        write(string(2 * utils_io::BUFSZ + 17, 'E'), '\0');
        return 0; // 外部测试逐字节核对析构前仍在缓存里的尾部
    }
    integer_edges(); floats(); text_and_bulk();
    puts("rw_check passed: integer types / i128 edges / floats / buffer boundaries / million integers / EOF");
}
