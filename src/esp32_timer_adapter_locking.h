#pragma once

#ifdef ESP_PLATFORM
extern "C" {
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
}
#endif

#include "timer_manager.h"

namespace esp32 {

template <size_t Capacity, size_t TicksFrequency = 1000>
class TimerManagerLockingAdapter {
  using Backend = timer::TimerManager<Capacity, TicksFrequency>;

public:
  using TimerID = timer::TimerID;
  using TimeoutCallback = typename Backend::TimeoutCallback;

  explicit TimerManagerLockingAdapter(Backend &backend)
      : backend_(backend)
#ifdef ESP_PLATFORM
        , mux_(portMUX_INITIALIZER_UNLOCKED)
#endif
  {}

  TimerID addTimer(uint32_t timeoutMs, bool autoReload) {
    CriticalGuard guard(mux_);
    return backend_.addNew(timeoutMs, autoReload);
  }

  bool cancelTimer(TimerID id) {
    CriticalGuard guard(mux_);
    return backend_.cancelTimer(id);
  }

  bool changeTimeout(TimerID id, uint32_t newTimeoutMs) {
    CriticalGuard guard(mux_);
    return backend_.changeTimeout(id, newTimeoutMs);
  }

  void setTimeoutCallback(TimeoutCallback cb) {
    CriticalGuard guard(mux_);
    backend_.onTimeoutEvent(std::move(cb));
  }

  void processTickFromISR() {
    CriticalGuardISR guard(mux_);
    backend_.processTick();
  }

  void processTickFromTask() {
    CriticalGuard guard(mux_);
    backend_.processTick();
  }

private:
  class CriticalGuard {
  public:
#ifdef ESP_PLATFORM
    explicit CriticalGuard(portMUX_TYPE &mux) : mux_(mux) {
      portENTER_CRITICAL(&mux_);
    }

    ~CriticalGuard() { portEXIT_CRITICAL(&mux_); }
#else
    explicit CriticalGuard(int &) {}
#endif

  private:
#ifdef ESP_PLATFORM
    portMUX_TYPE &mux_;
#endif
  };

  class CriticalGuardISR {
  public:
#ifdef ESP_PLATFORM
    explicit CriticalGuardISR(portMUX_TYPE &mux) : mux_(mux) {
      portENTER_CRITICAL_ISR(&mux_);
    }

    ~CriticalGuardISR() { portEXIT_CRITICAL_ISR(&mux_); }
#else
    explicit CriticalGuardISR(int &) {}
#endif

  private:
#ifdef ESP_PLATFORM
    portMUX_TYPE &mux_;
#endif
  };

private:
  Backend &backend_;
#ifdef ESP_PLATFORM
  portMUX_TYPE mux_;
#else
  int mux_{};
#endif
};

} // namespace esp32

