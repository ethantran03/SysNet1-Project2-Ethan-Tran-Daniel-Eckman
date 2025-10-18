#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <cstring>
#include <iomanip>
#include <ctime>

// Global histogram of stopping times for k in [0, 1000]
static std::vector<unsigned long long> HIST(1001, 0ULL);

// Global counter for the next number to process (starts at 1)
static std::atomic<unsigned long long> COUNTER{1ULL};

// Mutexes to protect shared state when locks are enabled
static std::mutex counter_mtx;
static std::mutex hist_mtx;

// Compute the Collatz stopping time for n (steps to reach 1). n>=1.
static inline unsigned int collatz_stopping_time(unsigned long long start) {
    if (start <= 1ULL) return 0U;
    unsigned int steps = 0U;

    // Use 128-bit for intermediate math to reduce overflow risk on 3n+1.
    __uint128_t n = start;
    while (n != 1) {
        if ((n & 1) == 0) {
            n >>= 1; // n/2
        } else {
            n = 3 * n + 1; // 3n + 1
        }
        ++steps;
    }
    return steps;
}

int main(int argc, char* argv[]) {
    if (argc < 3 || argc > 4) {
        std::cerr << "Usage: " << argv[0] << " N T [-nolock]\n";
        return 1;
    }

    // Parse N and T
    unsigned long long N = 0ULL;
    unsigned int T = 0U;
    try {
        N = std::stoull(argv[1]);
        T = static_cast<unsigned int>(std::stoul(argv[2]));
    } catch (...) {
        std::cerr << "Error: N and T must be positive integers.\n";
        return 1;
    }

    if (N < 1ULL || T < 1U) {
        std::cerr << "Error: N>=1 and T>=1 are required.\n";
        return 1;
    }

    // Optional -nolock flag
    bool use_locks = true;
    if (argc == 4) {
        if (std::strcmp(argv[3], "-nolock") == 0) {
            use_locks = false;
        } else {
            std::cerr << "Warning: Unrecognized option '" << argv[3]
                      << "' (did you mean -nolock?). Locks will be used by default.\n";
        }
    }

    // Reset shared state
    std::fill(HIST.begin(), HIST.end(), 0ULL);
    COUNTER.store(1ULL, std::memory_order_relaxed);

    // Start timer (real-time clock)
    timespec ts_start{}, ts_end{};
    if (clock_gettime(CLOCK_REALTIME, &ts_start) != 0) {
        std::perror("clock_gettime start");
        return 1;
    }

    // Worker function
    auto worker = [N, use_locks]() {
        for (;;) {
            unsigned long long my_n;
            if (use_locks) {
                std::lock_guard<std::mutex> lg(counter_mtx);
                my_n = COUNTER++;
            } else {
                // Intentionally unsynchronized to allow race conditions when -nolock
                my_n = COUNTER++;
            }

            if (my_n > N) break;

            unsigned int steps = collatz_stopping_time(my_n);
            unsigned int bin = (steps > 1000U) ? 1000U : steps; // clamp to [0,1000]

            if (use_locks) {
                std::lock_guard<std::mutex> lg(hist_mtx);
                ++HIST[bin];
            } else {
                // Intentionally unsynchronized when -nolock
                ++HIST[bin];
            }
        }
    };

    // Launch T threads
    std::vector<std::thread> threads;
    threads.reserve(T);
    for (unsigned int i = 0; i < T; ++i) {
        threads.emplace_back(worker);
    }

    // Join all threads
    for (auto &th : threads) th.join();

    // Stop timer
    if (clock_gettime(CLOCK_REALTIME, &ts_end) != 0) {
        std::perror("clock_gettime end");
        return 1;
    }

    // Compute elapsed time with proper borrow for nanoseconds
    long long sec = static_cast<long long>(ts_end.tv_sec) - static_cast<long long>(ts_start.tv_sec);
    long long nsec = static_cast<long long>(ts_end.tv_nsec) - static_cast<long long>(ts_start.tv_nsec);
    if (nsec < 0) { nsec += 1000000000LL; --sec; }

    // Print timing to stderr: N,T,seconds.nanoseconds
    std::cerr << N << "," << T << "," << sec << "." << std::setw(9) << std::setfill('0') << nsec << "\n";

    // Print histogram to stdout: k,frequency for k=0..1000
    for (size_t k = 0; k < HIST.size(); ++k) {
        std::cout << k << "," << HIST[k] << "\n";
    }

    return 0;
}
