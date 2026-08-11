#pragma once

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <utility>

namespace aur
{
/** Minimal in-place iterative radix-2 FFT (pure C++, no dependencies).
    N must be a power of two. Used for spectral analysis/detection only. */
struct SimpleFFT
{
    static void forward (double* re, double* im, int N)
    {
        // Bit-reversal permutation.
        for (int i = 1, j = 0; i < N; ++i)
        {
            int bit = N >> 1;
            for (; j & bit; bit >>= 1) j ^= bit;
            j ^= bit;
            if (i < j) { std::swap (re[i], re[j]); std::swap (im[i], im[j]); }
        }

        for (int len = 2; len <= N; len <<= 1)
        {
            const double ang = -2.0 * M_PI / (double) len;
            const double wr = std::cos (ang), wi = std::sin (ang);
            for (int i = 0; i < N; i += len)
            {
                double cwr = 1.0, cwi = 0.0;
                for (int k = 0; k < len / 2; ++k)
                {
                    const int a = i + k, b = i + k + len / 2;
                    const double ur = re[a],           ui = im[a];
                    const double vr = re[b] * cwr - im[b] * cwi;
                    const double vi = re[b] * cwi + im[b] * cwr;
                    re[a] = ur + vr; im[a] = ui + vi;
                    re[b] = ur - vr; im[b] = ui - vi;
                    const double ncwr = cwr * wr - cwi * wi;
                    cwi = cwr * wi + cwi * wr;
                    cwr = ncwr;
                }
            }
        }
    }

    /** Inverse FFT (conjugate → forward → conjugate → scale by 1/N). */
    static void inverse (double* re, double* im, int N)
    {
        for (int i = 0; i < N; ++i) im[i] = -im[i];
        forward (re, im, N);
        const double inv = 1.0 / (double) N;
        for (int i = 0; i < N; ++i) { re[i] *= inv; im[i] = -im[i] * inv; }
    }
};
} // namespace aur
