#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>
using namespace std;

vector<int> smallest_factor;
vector<bool> prime;
vector<int> primes;

void sieve(int maximum) {
    maximum = max(maximum, 1);
    smallest_factor.assign(maximum + 1, 0);
    prime.assign(maximum + 1, true);
    prime[0] = prime[1] = false;
    primes = {};

    for (int p = 2; p <= maximum; p++)
        if (prime[p]) {
            smallest_factor[p] = p;
            primes.push_back(p);

            for (int64_t i = int64_t(p) * p; i <= maximum; i += p)
                if (prime[i]) {
                    prime[i] = false;
                    smallest_factor[i] = p;
                }
        }
}

// Determines whether n is prime in worst case O(sqrt n / log n). Requires having run `sieve` up to at least sqrt(n).
// If we've run `sieve` up to at least n, takes O(1) time.
bool is_prime(int64_t n) {
    int64_t sieve_max = int64_t(smallest_factor.size()) - 1;
    assert(1 <= n && n <= sieve_max * sieve_max);

    if (n <= sieve_max)
        return prime[n];

    for (int64_t p : primes) {
        if (p * p > n)
            break;

        if (n % p == 0)
            return false;
    }

    return true;
}

// Prime factorizes n in worst case O(sqrt n / log n). Requires having run `sieve` up to at least sqrt(n).
// If we've run `sieve` up to at least n, takes O(log n) time.
vector<pair<int64_t, int>> prime_factorize(int64_t n) {
    int64_t sieve_max = int64_t(smallest_factor.size()) - 1;
    assert(1 <= n && n <= sieve_max * sieve_max);
    vector<pair<int64_t, int>> result;

    if (n <= sieve_max) {
        while (n != 1) {
            int64_t p = smallest_factor[n];
            int exponent = 0;

            do {
                n /= p;
                exponent++;
            } while (n % p == 0);

            result.emplace_back(p, exponent);
        }

        return result;
    }

    for (int64_t p : primes) {
        if (p * p > n)
            break;

        if (n % p == 0) {
            result.emplace_back(p, 0);

            do {
                n /= p;
                result.back().second++;
            } while (n % p == 0);
        }
    }

    if (n > 1)
        result.emplace_back(n, 1);

    return result;
}

vector<int64_t> generate_factors(const vector<pair<int64_t, int>> &prime_factors, bool sorted = false) {
    // See http://oeis.org/A066150 and http://oeis.org/A036451 for upper bounds on number of factors.
    static vector<int64_t> buffer;
    int product = 1;

    for (auto &pf : prime_factors)
        product *= pf.second + 1;

    vector<int64_t> factors = {1};
    factors.reserve(product);

    if (sorted)
        buffer.resize(product);

    for (auto &pf : prime_factors) {
        int64_t p = pf.first;
        int exponent = pf.second;
        int before_size = int(factors.size());

        for (int i = 0; i < exponent * before_size; i++)
            factors.push_back(factors[factors.size() - before_size] * p);

        if (sorted && factors[before_size - 1] > p)
            for (int section = before_size; section < int(factors.size()); section *= 2)
                for (int i = 0; i + section < int(factors.size()); i += 2 * section) {
                    int length = min(2 * section, int(factors.size()) - i);
                    merge(factors.begin() + i, factors.begin() + i + section,
                          factors.begin() + i + section, factors.begin() + i + length,
                          buffer.begin());
                    copy(buffer.begin(), buffer.begin() + length, factors.begin() + i);
                }
    }

    assert(int(factors.size()) == product);
    return factors;
}


void test_factoring_number(int64_t n, const vector<int64_t> &expected, bool sorted = false) {
    vector<int64_t> factors = generate_factors(prime_factorize(n), sorted);
    assert(factors == expected);
}

void test_generate_factors() {
    sieve(100);

    for (int n = 1; n <= 10000; n++) {
        vector<pair<int64_t, int>> prime_factors = prime_factorize(n);
        assert(is_prime(n) == (prime_factors.size() == 1 && prime_factors.front().second == 1));
    }

    sieve(1e5);

    test_factoring_number(1, {1});
    test_factoring_number(2, {1, 2});
    test_factoring_number(3, {1, 3});
    test_factoring_number(4, {1, 2, 4});
    test_factoring_number(60, {1, 2, 4, 3, 6, 12, 5, 10, 20, 15, 30, 60});
    test_factoring_number(36, {1, 2, 4, 3, 6, 12, 9, 18, 36});

    test_factoring_number(5000000029LL, {1, 5000000029LL});
    test_factoring_number(4802300273LL, {1, 60013, 80021, 4802300273LL});
    test_factoring_number(6276787200LL, {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 3, 6, 12, 24, 48, 96, 192, 384, 768, 1536, 3072, 9, 18, 36, 72, 144, 288, 576, 1152, 2304, 4608, 9216, 27, 54, 108, 216, 432, 864, 1728, 3456, 6912, 13824, 27648, 81, 162, 324, 648, 1296, 2592, 5184, 10368, 20736, 41472, 82944, 243, 486, 972, 1944, 3888, 7776, 15552, 31104, 62208, 124416, 248832, 5, 10, 20, 40, 80, 160, 320, 640, 1280, 2560, 5120, 15, 30, 60, 120, 240, 480, 960, 1920, 3840, 7680, 15360, 45, 90, 180, 360, 720, 1440, 2880, 5760, 11520, 23040, 46080, 135, 270, 540, 1080, 2160, 4320, 8640, 17280, 34560, 69120, 138240, 405, 810, 1620, 3240, 6480, 12960, 25920, 51840, 103680, 207360, 414720, 1215, 2430, 4860, 9720, 19440, 38880, 77760, 155520, 311040, 622080, 1244160, 25, 50, 100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600, 75, 150, 300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 76800, 225, 450, 900, 1800, 3600, 7200, 14400, 28800, 57600, 115200, 230400, 675, 1350, 2700, 5400, 10800, 21600, 43200, 86400, 172800, 345600, 691200, 2025, 4050, 8100, 16200, 32400, 64800, 129600, 259200, 518400, 1036800, 2073600, 6075, 12150, 24300, 48600, 97200, 194400, 388800, 777600, 1555200, 3110400, 6220800, 1009, 2018, 4036, 8072, 16144, 32288, 64576, 129152, 258304, 516608, 1033216, 3027, 6054, 12108, 24216, 48432, 96864, 193728, 387456, 774912, 1549824, 3099648, 9081, 18162, 36324, 72648, 145296, 290592, 581184, 1162368, 2324736, 4649472, 9298944, 27243, 54486, 108972, 217944, 435888, 871776, 1743552, 3487104, 6974208, 13948416, 27896832, 81729, 163458, 326916, 653832, 1307664, 2615328, 5230656, 10461312, 20922624, 41845248, 83690496, 245187, 490374, 980748, 1961496, 3922992, 7845984, 15691968, 31383936, 62767872, 125535744, 251071488, 5045, 10090, 20180, 40360, 80720, 161440, 322880, 645760, 1291520, 2583040, 5166080, 15135, 30270, 60540, 121080, 242160, 484320, 968640, 1937280, 3874560, 7749120, 15498240, 45405, 90810, 181620, 363240, 726480, 1452960, 2905920, 5811840, 11623680, 23247360, 46494720, 136215, 272430, 544860, 1089720, 2179440, 4358880, 8717760, 17435520, 34871040, 69742080, 139484160, 408645, 817290, 1634580, 3269160, 6538320, 13076640, 26153280, 52306560, 104613120, 209226240, 418452480, 1225935, 2451870, 4903740, 9807480, 19614960, 39229920, 78459840, 156919680, 313839360, 627678720, 1255357440, 25225, 50450, 100900, 201800, 403600, 807200, 1614400, 3228800, 6457600, 12915200, 25830400, 75675, 151350, 302700, 605400, 1210800, 2421600, 4843200, 9686400, 19372800, 38745600, 77491200, 227025, 454050, 908100, 1816200, 3632400, 7264800, 14529600, 29059200, 58118400, 116236800, 232473600, 681075, 1362150, 2724300, 5448600, 10897200, 21794400, 43588800, 87177600, 174355200, 348710400, 697420800, 2043225, 4086450, 8172900, 16345800, 32691600, 65383200, 130766400, 261532800, 523065600, 1046131200, 2092262400, 6129675, 12259350, 24518700, 49037400, 98074800, 196149600, 392299200, 784598400, 1569196800, 3138393600, 6276787200});

    // Test sorted.
    test_factoring_number(4, {1, 2, 4}, true);
    test_factoring_number(60, {1, 2, 3, 4, 5, 6, 10, 12, 15, 20, 30, 60}, true);
    test_factoring_number(36, {1, 2, 3, 4, 6, 9, 12, 18, 36}, true);

    test_factoring_number(5000000029LL, {1, 5000000029LL}, true);
    test_factoring_number(4802300273LL, {1, 60013, 80021, 4802300273LL}, true);
    test_factoring_number(6276787200LL, {1, 2, 3, 4, 5, 6, 8, 9, 10, 12, 15, 16, 18, 20, 24, 25, 27, 30, 32, 36, 40, 45, 48, 50, 54, 60, 64, 72, 75, 80, 81, 90, 96, 100, 108, 120, 128, 135, 144, 150, 160, 162, 180, 192, 200, 216, 225, 240, 243, 256, 270, 288, 300, 320, 324, 360, 384, 400, 405, 432, 450, 480, 486, 512, 540, 576, 600, 640, 648, 675, 720, 768, 800, 810, 864, 900, 960, 972, 1009, 1024, 1080, 1152, 1200, 1215, 1280, 1296, 1350, 1440, 1536, 1600, 1620, 1728, 1800, 1920, 1944, 2018, 2025, 2160, 2304, 2400, 2430, 2560, 2592, 2700, 2880, 3027, 3072, 3200, 3240, 3456, 3600, 3840, 3888, 4036, 4050, 4320, 4608, 4800, 4860, 5045, 5120, 5184, 5400, 5760, 6054, 6075, 6400, 6480, 6912, 7200, 7680, 7776, 8072, 8100, 8640, 9081, 9216, 9600, 9720, 10090, 10368, 10800, 11520, 12108, 12150, 12800, 12960, 13824, 14400, 15135, 15360, 15552, 16144, 16200, 17280, 18162, 19200, 19440, 20180, 20736, 21600, 23040, 24216, 24300, 25225, 25600, 25920, 27243, 27648, 28800, 30270, 31104, 32288, 32400, 34560, 36324, 38400, 38880, 40360, 41472, 43200, 45405, 46080, 48432, 48600, 50450, 51840, 54486, 57600, 60540, 62208, 64576, 64800, 69120, 72648, 75675, 76800, 77760, 80720, 81729, 82944, 86400, 90810, 96864, 97200, 100900, 103680, 108972, 115200, 121080, 124416, 129152, 129600, 136215, 138240, 145296, 151350, 155520, 161440, 163458, 172800, 181620, 193728, 194400, 201800, 207360, 217944, 227025, 230400, 242160, 245187, 248832, 258304, 259200, 272430, 290592, 302700, 311040, 322880, 326916, 345600, 363240, 387456, 388800, 403600, 408645, 414720, 435888, 454050, 484320, 490374, 516608, 518400, 544860, 581184, 605400, 622080, 645760, 653832, 681075, 691200, 726480, 774912, 777600, 807200, 817290, 871776, 908100, 968640, 980748, 1033216, 1036800, 1089720, 1162368, 1210800, 1225935, 1244160, 1291520, 1307664, 1362150, 1452960, 1549824, 1555200, 1614400, 1634580, 1743552, 1816200, 1937280, 1961496, 2043225, 2073600, 2179440, 2324736, 2421600, 2451870, 2583040, 2615328, 2724300, 2905920, 3099648, 3110400, 3228800, 3269160, 3487104, 3632400, 3874560, 3922992, 4086450, 4358880, 4649472, 4843200, 4903740, 5166080, 5230656, 5448600, 5811840, 6129675, 6220800, 6457600, 6538320, 6974208, 7264800, 7749120, 7845984, 8172900, 8717760, 9298944, 9686400, 9807480, 10461312, 10897200, 11623680, 12259350, 12915200, 13076640, 13948416, 14529600, 15498240, 15691968, 16345800, 17435520, 19372800, 19614960, 20922624, 21794400, 23247360, 24518700, 25830400, 26153280, 27896832, 29059200, 31383936, 32691600, 34871040, 38745600, 39229920, 41845248, 43588800, 46494720, 49037400, 52306560, 58118400, 62767872, 65383200, 69742080, 77491200, 78459840, 83690496, 87177600, 98074800, 104613120, 116236800, 125535744, 130766400, 139484160, 156919680, 174355200, 196149600, 209226240, 232473600, 251071488, 261532800, 313839360, 348710400, 392299200, 418452480, 523065600, 627678720, 697420800, 784598400, 1046131200, 1255357440, 1569196800, 2092262400, 3138393600, 6276787200}, true);

    cerr << "Tests passed!" << endl;
    sieve(0);
}

#include <chrono>
#include <iomanip>
#include <random>

void test_runtime() {
    // auto random_address = [] { char *p = new char; delete p; return uint64_t(p); };

    // const uint64_t SEED = chrono::steady_clock::now().time_since_epoch().count() * (random_address() | 1);
    const uint64_t SEED = 0;
    mt19937_64 rng(SEED);

    cerr << setprecision(3);

    sieve(2e5);

{
    const vector<int64_t> OPTIONS = {13071985783, 16510398467, 14387119589, 25092948337, 32149278989};

    long double begin = clock();
    uint64_t sum = 0;

    for (int iter = 0; iter < 500; iter++) {
        int64_t n = OPTIONS[rng() % OPTIONS.size()];
        vector<pair<int64_t, int>> prime_factors = prime_factorize(n);

        vector<int64_t> factors = generate_factors(prime_factors);

        for (int64_t f : factors)
            sum += f;
    }

    cerr << "sum = " << sum << endl;
    cerr << (clock() - begin) / CLOCKS_PER_SEC << 's' << endl;
}

// {
//     const vector<int64_t> OPTIONS = {897612484786617600, 748010403988848000, 673209363589963200, 448806242393308800, 374005201994424000};

//     long double begin = clock();
//     uint64_t sum = 0;

//     for (int iter = 0; iter < 1000; iter++) {
//         int64_t n = OPTIONS[rng() % OPTIONS.size()];
//         vector<pair<int64_t, int>> prime_factors = prime_factorize(n);

//         vector<int64_t> factors = generate_factors(prime_factors);

//         for (int64_t f : factors)
//             sum += f;
//     }

//     cerr << "sum = " << sum << endl;
//     cerr << (clock() - begin) / CLOCKS_PER_SEC << 's' << endl;
// }

    sieve(0);
}

#include <cmath>

int main() {
    ios::sync_with_stdio(false);
#ifndef NEAL_DEBUG
    cin.tie(nullptr);
#endif

    test_generate_factors();

    test_runtime();

    vector<int64_t> inputs;
    int sieve_size = 0;
    int64_t x;

    while (cin >> x) {
        sieve_size = max(sieve_size, int(sqrt(x)) + 1);
        inputs.push_back(x);
    }

    sieve(sieve_size);

    for (int64_t n : inputs) {
        vector<pair<int64_t, int>> prime_factors = prime_factorize(n);
        cout << n << '\n';

        for (pair<int64_t, int> &pf : prime_factors)
            cout << pf.first << ' ' << pf.second << '\n';
    }
}
