#pragma once

#include <atomic>

namespace aur
{
/** Lock-free audio→UI meter bridge. Audio thread stores, UI thread loads. */
class MeterState
{
public:
    void pushInputPeak     (float v) noexcept { inPeak.store  (v, std::memory_order_relaxed); }
    void pushOutputPeak    (float v) noexcept { outPeak.store (v, std::memory_order_relaxed); }
    void pushGainReduction (float dB) noexcept { gr.store (dB, std::memory_order_relaxed); }
    void pushCorrelation   (float c)  noexcept { corr.store (c, std::memory_order_relaxed); }

    float getInputPeak()     const noexcept { return inPeak.load  (std::memory_order_relaxed); }
    float getOutputPeak()    const noexcept { return outPeak.load (std::memory_order_relaxed); }
    float getGainReductionDb() const noexcept { return gr.load (std::memory_order_relaxed); }
    float getCorrelation()     const noexcept { return corr.load (std::memory_order_relaxed); }

private:
    std::atomic<float> inPeak  { 0.0f };
    std::atomic<float> outPeak { 0.0f };
    std::atomic<float> gr      { 0.0f };
    std::atomic<float> corr    { 0.0f };
};
} // namespace aur
