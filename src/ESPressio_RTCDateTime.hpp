#pragma once

#include <cstdint>
#include <type_traits>
#include <utility>

#include <ESPressio_Platform.hpp>

namespace ESPressio::RTC {

/** Result codes shared by platform-independent RTC implementations. */
enum class Result : std::uint8_t {
    Ok = 0,
    BusError,
    InvalidArgument,
    InvalidDateTime,
    DeviceBusy,
    WriteProtected,
    Unsupported
};

/** Reason an RTC reading should not be trusted as authoritative civil time. */
enum class TimeValidity : std::uint8_t {
    Valid = 0,
    OscillatorStopped,
    VoltageLow,
    PowerOnReset,
    Unknown
};

/** Calendar/time value represented without timezone or daylight-saving semantics. */
struct DateTime {
    std::uint16_t Year{2000};
    std::uint8_t Month{1};
    std::uint8_t Day{1};
    std::uint8_t Hour{0};
    std::uint8_t Minute{0};
    std::uint8_t Second{0};
    std::uint8_t Weekday{0}; // 0..6; mapping is application-defined unless a device documents one.
};

/** One RTC sample plus the device's current confidence in the stored civil time. */
struct Reading {
    DateTime Value{};
    TimeValidity Validity{TimeValidity::Unknown};
};

constexpr bool IsLeapYear(std::uint16_t year) noexcept {
    return (year % 4U == 0U) && ((year % 100U != 0U) || (year % 400U == 0U));
}

constexpr std::uint8_t DaysInMonth(std::uint16_t year, std::uint8_t month) noexcept {
    switch (month) {
        case 1: case 3: case 5: case 7: case 8: case 10: case 12: return 31;
        case 4: case 6: case 9: case 11: return 30;
        case 2: return IsLeapYear(year) ? 29 : 28;
        default: return 0;
    }
}

constexpr bool IsValid(const DateTime& value) noexcept {
    const auto days = DaysInMonth(value.Year, value.Month);
    return value.Year >= 1900U && value.Year <= 2199U &&
           days != 0U && value.Day >= 1U && value.Day <= days &&
           value.Hour <= 23U && value.Minute <= 59U && value.Second <= 59U &&
           value.Weekday <= 6U;
}

constexpr std::uint8_t ToBcd(std::uint8_t value) noexcept {
    return static_cast<std::uint8_t>(((value / 10U) << 4U) | (value % 10U));
}

constexpr std::uint8_t FromBcd(std::uint8_t value) noexcept {
    return static_cast<std::uint8_t>(((value >> 4U) * 10U) + (value & 0x0FU));
}

namespace Capability {
/** Persistent civil-time/calendar service. */
struct RealTimeClock final : ESPressio::Platform::ExclusiveCapability {};
/** Device exposes at least one programmable alarm. */
struct Alarm final : ESPressio::Platform::SharedCapability {};
/** Device exposes a calibrated or raw temperature reading. */
struct Temperature final : ESPressio::Platform::SharedCapability {};
/** Device exposes a hardware-maintained Unix-time counter. */
struct UnixTime final : ESPressio::Platform::SharedCapability {};
/** Device exposes a programmable hardware clock output. */
struct ClockOutput final : ESPressio::Platform::SharedCapability {};
} // namespace Capability

/** Convenience declaration for a device that owns the RTC service capability. */
template <typename TOrigin,
          typename TExtraCapabilities = ESPressio::Platform::CapabilitySet<>,
          typename TRequirements = ESPressio::Platform::RequirementSet<>>
struct DeviceProviderDeclaration;

template <typename TOrigin, typename... TExtra, typename TRequirements>
struct DeviceProviderDeclaration<TOrigin,
                                 ESPressio::Platform::CapabilitySet<TExtra...>,
                                 TRequirements>
    : ESPressio::Platform::ProviderDeclaration<
          TOrigin,
          ESPressio::Platform::CapabilitySet<Capability::RealTimeClock, TExtra...>,
          TRequirements> {};

namespace Detail {
template <typename T, typename = void>
struct IsRealTimeClock : std::false_type {};

template <typename T>
struct IsRealTimeClock<T,
    std::void_t<decltype(std::declval<T&>().Read(std::declval<Reading&>())),
                decltype(std::declval<T&>().Write(std::declval<const DateTime&>()))>>
    : std::bool_constant<
          ESPressio::Platform::IsProviderV<T> &&
          T::PlatformCapabilities::template Contains<Capability::RealTimeClock> &&
          std::is_same_v<decltype(std::declval<T&>().Read(std::declval<Reading&>())), Result> &&
          std::is_same_v<decltype(std::declval<T&>().Write(std::declval<const DateTime&>())), Result> &&
          noexcept(std::declval<T&>().Read(std::declval<Reading&>())) &&
          noexcept(std::declval<T&>().Write(std::declval<const DateTime&>()))> {};
} // namespace Detail

template <typename T>
inline constexpr bool IsRealTimeClockV = Detail::IsRealTimeClock<T>::value;

} // namespace ESPressio::RTC
