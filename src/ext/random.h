/*
   World Class Chaotic Pseudo-Random Number Generator
   Algorithm by David Blackman.

   computers can't produce true unpredictability without hardware entropy or external inputs.
*/

#ifndef RANDOM_H
#define RANDOM_H

#include <stdint.h>

#ifdef __cplusplus
#include <random>
#else
#include <time.h>
#if defined(_WIN32)
#include <windows.h>
#include <process.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct {
        uint64_t a;
        uint64_t b;
        uint64_t c;
        uint64_t d;
    } random_state;

    static inline uint64_t _random_rotl(uint64_t x, int k) {
        return (x << k) | (x >> (64 - k));
    }

    static inline void random_advance(random_state* s) {
        s->b += s->c;
        s->a = _random_rotl(s->a, 32);
        s->c ^= s->b;

        s->d += 0x55aa96a5;

        s->a += s->b;
        s->c = _random_rotl(s->c, 23);
        s->b ^= s->a;

        s->a += s->c;
        s->b = _random_rotl(s->b, 19);
        s->c += s->a;

        s->b += s->d;
    }

    static inline void random_seed(random_state* s, uint64_t seed) {
        s->a = seed;
        s->b = 0;
        s->c = 2000001;
        s->d = 0;
        for (int i = 0; i < 14; ++i) random_advance(s);
    }

    static inline void random_seed2(random_state* s, uint64_t seed1, uint64_t seed2) {
        s->a = seed1;
        s->b = seed2;
        s->c = 2000001;
        s->d = 0;
        for (int i = 0; i < 14; ++i) random_advance(s);
    }

    static inline void random_autoseed(random_state* s) {
        uint64_t seed1 = 0, seed2 = 0;

#ifdef __cplusplus
        try {
            std::random_device rd;
            seed1 = ((uint64_t)rd() << 32) | rd();
            seed2 = ((uint64_t)rd() << 32) | rd();
        }
        catch (...) {
            seed1 = (uint64_t)time(NULL);
            seed2 = (uint64_t)clock();
        }

#elif defined(_WIN32)
        FILETIME ft;
        GetSystemTimeAsFileTime(&ft);
        LARGE_INTEGER pc;
        QueryPerformanceCounter(&pc);

        seed1 = ((uint64_t)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
        seed1 ^= (uint64_t)GetCurrentProcessId() << 32;
        seed1 ^= (uint64_t)GetCurrentThreadId();

        seed2 = (uint64_t)pc.QuadPart;
        seed2 ^= (uint64_t)clock();

#else
        int fd = open("/dev/urandom", O_RDONLY);
        if (fd >= 0) {
            uint64_t buf[2];
            if (read(fd, buf, sizeof(buf)) == sizeof(buf)) {
                seed1 = buf[0];
                seed2 = buf[1];
            }
            close(fd);
        }

        if (seed1 == 0 && seed2 == 0) {
            seed1 = (uint64_t)time(NULL);
            seed2 = (uint64_t)clock();
            seed2 ^= (uint64_t)(uintptr_t)s;
        }
#endif

        random_seed2(s, seed1, seed2);
    }

    static inline uint64_t random_next(random_state* s) {
        random_advance(s);
        return s->a;
    }

    static inline double random_double(random_state* s) {
        return (random_next(s) >> 11) * 0x1.0p-53;
    }

#ifdef __cplusplus
}
#endif

#endif