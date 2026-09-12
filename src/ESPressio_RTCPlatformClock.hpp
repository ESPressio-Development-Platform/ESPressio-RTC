#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>

#include <ESPressio_PlatformClock.hpp>

namespace ESPressio::RTC {

/**
 * Platform Clock provider for a hardware clock-output pin counted by an ISR.
 *
 * The application connects the physical RTC CLKOUT/SQW pin to an interrupt and
 * calls OnTickFromInterrupt() once for every selected clock edge. This type has
 * no dependency on GPIO, Arduino, ESP-IDF, FreeRTOS, or a particular RTC chip.
 */
template <typename TOrigin,
          std::uint64_t TFrequencyHz,
          std::size_t TCounterWidthBits = 32U,
          typename TExtraCapabilities = ESPressio::Platform::CapabilitySet<>>
class ExternalTickClock;

template <typename TOrigin,
          std::uint64_t TFrequencyHz,
          std::size_t TCounterWidthBits,
          typename... TExtraCapabilities>
class ExternalTickClock<TOrigin,
                        TFrequencyHz,
                        TCounterWidthBits,
                        ESPressio::Platform::CapabilitySet<TExtraCapabilities...>>
    : public ESPressio::Platform::Clock::MonotonicProviderDeclaration<
          TOrigin,
          TFrequencyHz,
          TCounterWidthBits,
          ESPressio::Platform::PropertySet<
              ESPressio::Platform::PropertyValue<
                  ESPressio::Platform::PropertyKey::ClockResolutionNanoseconds,
                  ESPressio::Platform::Clock::ResolutionNanosecondsForFrequency(TFrequencyHz)>>,
          ESPressio::Platform::CapabilitySet<
              ESPressio::Platform::Capability::InterruptReadableClock,
              TExtraCapabilities...>> {
    static_assert(TCounterWidthBits > 0U && TCounterWidthBits <= 32U,
                  "ExternalTickClock currently supports counter widths from 1 to 32 bits");
    static_assert(std::atomic<std::uint32_t>::is_always_lock_free || sizeof(TOrigin) == 0U,
                  "ExternalTickClock requires a lock-free 32-bit atomic for ISR safety");
public:
    using Tick = ESPressio::Platform::Clock::Tick;

    void OnTickFromInterrupt() noexcept {
        counter_.fetch_add(1U, std::memory_order_relaxed);
    }

    Tick Now() const noexcept {
        return static_cast<Tick>(counter_.load(std::memory_order_relaxed)) &
               ESPressio::Platform::Clock::CounterMask(TCounterWidthBits);
    }

    Tick NowFromInterrupt() const noexcept { return Now(); }

private:
    std::atomic<std::uint32_t> counter_{0U};
};

} // namespace ESPressio::RTC
