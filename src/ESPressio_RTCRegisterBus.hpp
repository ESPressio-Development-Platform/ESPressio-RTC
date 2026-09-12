#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

#include "ESPressio_RTCDateTime.hpp"

namespace ESPressio::RTC {

namespace Detail {
template <typename T, typename = void>
struct IsRegisterBus : std::false_type {};

template <typename T>
struct IsRegisterBus<T, std::void_t<
    decltype(std::declval<T&>().Read(std::declval<std::uint8_t>(),
                                    std::declval<std::uint8_t>(),
                                    std::declval<std::uint8_t*>(),
                                    std::declval<std::size_t>())),
    decltype(std::declval<T&>().Write(std::declval<std::uint8_t>(),
                                     std::declval<std::uint8_t>(),
                                     std::declval<const std::uint8_t*>(),
                                     std::declval<std::size_t>()))>>
    : std::bool_constant<
          std::is_same_v<decltype(std::declval<T&>().Read(0, 0, nullptr, 0)), bool> &&
          std::is_same_v<decltype(std::declval<T&>().Write(0, 0, nullptr, 0)), bool> &&
          noexcept(std::declval<T&>().Read(0, 0, nullptr, 0)) &&
          noexcept(std::declval<T&>().Write(0, 0, nullptr, 0))> {};
} // namespace Detail

template <typename T>
inline constexpr bool IsRegisterBusV = Detail::IsRegisterBus<T>::value;

/**
 * Zero-allocation register-device helper used by RTC chip drivers.
 *
 * TBus is intentionally structural. It may be backed by Arduino Wire, ESP-IDF,
 * Linux I2C, a mock bus, or any later Platform I2C abstraction without changing
 * the RTC device implementation.
 */
template <typename TBus, std::uint8_t TAddress>
class RegisterDevice {
    static_assert(IsRegisterBusV<TBus>,
                  "RTC register bus must provide noexcept bool Read/Write(address, register, data, size)");
public:
    explicit constexpr RegisterDevice(TBus& bus) noexcept : bus_(bus) {}

    Result Read(std::uint8_t reg, std::uint8_t* data, std::size_t size) noexcept {
        return bus_.Read(TAddress, reg, data, size) ? Result::Ok : Result::BusError;
    }

    Result Write(std::uint8_t reg, const std::uint8_t* data, std::size_t size) noexcept {
        return bus_.Write(TAddress, reg, data, size) ? Result::Ok : Result::BusError;
    }

    Result ReadByte(std::uint8_t reg, std::uint8_t& value) noexcept {
        return Read(reg, &value, 1U);
    }

    Result WriteByte(std::uint8_t reg, std::uint8_t value) noexcept {
        return Write(reg, &value, 1U);
    }

    Result UpdateBits(std::uint8_t reg, std::uint8_t mask, std::uint8_t value) noexcept {
        std::uint8_t current{};
        auto result = ReadByte(reg, current);
        if (result != Result::Ok) return result;
        current = static_cast<std::uint8_t>((current & static_cast<std::uint8_t>(~mask)) | (value & mask));
        return WriteByte(reg, current);
    }

private:
    TBus& bus_;
};

} // namespace ESPressio::RTC
