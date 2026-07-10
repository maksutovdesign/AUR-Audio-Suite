#pragma once

#include <array>
#include <atomic>
#include <cstdint>

namespace aur
{
/**
    Lock-free single-producer/single-consumer ring buffer for feeding an FFT
    analyzer. The audio thread pushes mono samples; the UI thread copies the
    most recent N. Realtime-safe: no allocation, no locks.  Size must be a
    power of two.
*/
template <uint32_t Size>
class AnalyzerFifo
{
public:
    static_assert ((Size & (Size - 1)) == 0, "Size must be a power of two");

    void push (float x) noexcept
    {
        const auto w = writePos.load (std::memory_order_relaxed);
        buffer[w & (Size - 1)] = x;
        writePos.store (w + 1, std::memory_order_release);
    }

    /** Copy the latest `n` samples (n <= Size) into dest, oldest first. */
    void readLatest (float* dest, uint32_t n) const noexcept
    {
        const auto w = writePos.load (std::memory_order_acquire);
        const auto base = w - n;
        for (uint32_t i = 0; i < n; ++i)
            dest[i] = buffer[(base + i) & (Size - 1)];
    }

    static constexpr uint32_t size() { return Size; }

private:
    std::array<float, Size> buffer {};
    std::atomic<uint32_t> writePos { 0 };
};
} // namespace aur
