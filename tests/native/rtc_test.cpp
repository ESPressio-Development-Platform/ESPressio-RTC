#include <array>
#include <cassert>
#include <cstdint>

#include "ESPressio_RTC.hpp"

using namespace ESPressio;

struct TestOrigin final : Platform::Backend {};

struct FakeBus {
    std::array<std::uint8_t, 256> regs{};
    bool Read(std::uint8_t, std::uint8_t reg, std::uint8_t* data, std::size_t size) noexcept {
        for (std::size_t i = 0; i < size; ++i) data[i] = regs[static_cast<std::uint8_t>(reg + i)];
        return true;
    }
    bool Write(std::uint8_t, std::uint8_t reg, const std::uint8_t* data, std::size_t size) noexcept {
        for (std::size_t i = 0; i < size; ++i) regs[static_cast<std::uint8_t>(reg + i)] = data[i];
        return true;
    }
};

using TickClock = RTC::ExternalTickClock<TestOrigin, 32768U>;
static_assert(Platform::Clock::IsClockSourceV<TickClock>);
static_assert(Platform::Clock::IsInterruptReadableClockSourceV<TickClock>);
static_assert(Platform::Clock::FrequencyHz<TickClock> == 32768U);

int main() {
    static_assert(RTC::IsLeapYear(2000));
    static_assert(!RTC::IsLeapYear(2100));
    static_assert(RTC::DaysInMonth(2024, 2) == 29);
    static_assert(RTC::ToBcd(59) == 0x59);
    static_assert(RTC::FromBcd(0x42) == 42);

    RTC::DateTime valid{2026, 9, 12, 16, 30, 45, 6};
    assert(RTC::IsValid(valid));
    valid.Day = 31;
    valid.Month = 2;
    assert(!RTC::IsValid(valid));

    FakeBus bus;
    RTC::RegisterDevice<FakeBus, 0x68> device(bus);
    assert(device.WriteByte(0x10, 0xAA) == RTC::Result::Ok);
    std::uint8_t read{};
    assert(device.ReadByte(0x10, read) == RTC::Result::Ok);
    assert(read == 0xAA);

    TickClock clock;
    clock.OnTickFromInterrupt();
    clock.OnTickFromInterrupt();
    assert(clock.Now() == 2U);
}
