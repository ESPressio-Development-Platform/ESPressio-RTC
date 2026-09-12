# ESPressio-RTC

Platform-independent real-time-clock domain contracts for ESPressio.

`ESPressio-RTC` deliberately separates **civil/persistent time** from `ESPressio-Platform::Clock`, which represents monotonic elapsed-time ticks. An RTC device library may provide both capabilities, but changing calendar time must never move a monotonic Platform clock.

The library provides:

- `DateTime`, `Reading` and `TimeValidity`;
- calendar validation and BCD helpers;
- the `RTC::Capability::RealTimeClock` domain capability;
- structural validation through `RTC::IsRealTimeClockV<T>`;
- a platform-neutral register-bus contract used by chip libraries;
- `RTC::ExternalTickClock`, a lock-free ISR-driven adapter that allows an RTC oscillator/clock-output pin to satisfy `Platform::Capability::Clock` without depending on Arduino, ESP-IDF, or FreeRTOS.

A register bus is any type exposing:

```cpp
bool Read(uint8_t address, uint8_t reg, uint8_t* data, size_t size);
bool Write(uint8_t address, uint8_t reg, const uint8_t* data, size_t size);
```

Each call represents one sequential register transaction beginning at `reg`; multi-byte RTC snapshots therefore remain a single bus operation. The operations are required to be `noexcept`. This allows the same chip driver to sit above any present or future I2C implementation.
