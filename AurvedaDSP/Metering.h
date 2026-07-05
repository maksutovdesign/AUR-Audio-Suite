#pragma once

#include <atomic>

namespace aur
{
/** Lock-free audio→UI meter bridge. Audio thread stores, UI thread loads. */
class MeterState
{
public:
    void pushInputPeak  (float v) noexcept { inPeak.store  (v, std::memory_order_relaxed); }
    void pushOutputPeak (float v) noexcept { outPeak.store (v, std::memory_order_relaxed); }

    float getInputPeak()  const noexcept { return inPeak.load  (std::memory_order_relaxed); }
    float getOutputPeak() const noexcept { return outPeak.load (std::memory_order_relaxed); }

private:
    std::atomic<float> inPeak  { 0.0f };
    std::atomic<float> outPeak { 0.0f };
};
} // namespace aur
