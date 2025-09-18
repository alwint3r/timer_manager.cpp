#pragma once

#include <cstdint>
#include <functional>
#include <utility>

namespace timer {
using TimerID = int32_t;

template <size_t Capacity, size_t TicksFrequency = 1000> class TimerManager {
public:
  using TimeoutCallback = std::function<void(TimerID id)>;

  struct State {
    uint64_t ticksLeft[Capacity]{};
    uint32_t timeout[Capacity]{};
    bool autoReload[Capacity]{};
  };

  TimerID addNew(const uint32_t timeoutMs, const bool autoReload) {
    auto id = getFreeTimerID();
    if (id < 0) {
      return -1;
    }

    size_t ix = static_cast<size_t>(id);
    state_.timeout[ix] = timeoutMs;
    state_.autoReload[ix] = autoReload;
    state_.ticksLeft[ix] = computeTicks(timeoutMs);

    return id;
  }

  bool cancelTimer(const TimerID id) {
    if (id < 0) {
      return false;
    }

    size_t ix = static_cast<size_t>(id);
    state_.timeout[ix] = 0;
    state_.autoReload[ix] = false;
    state_.ticksLeft[ix] = 0;

    return true;
  }

  bool changeTimeout(const TimerID id, uint32_t newTimeoutMs) {
    if (id < 0) {
      return false;
    }

    size_t ix = static_cast<size_t>(id);
    state_.timeout[ix] = newTimeoutMs;
    state_.ticksLeft[ix] = computeTicks(newTimeoutMs);

    return true;
  }

  void processTick() {
    for (size_t ix = 0; ix < Capacity; ix++) {
      if (state_.timeout[ix] > 0) {
        state_.ticksLeft[ix] -= 1;
        if (state_.ticksLeft[ix] <= 0) {
          if (timeoutCb_) {
            std::invoke(timeoutCb_, static_cast<TimerID>(ix));
          }

          if (state_.autoReload[ix]) {
            state_.ticksLeft[ix] = computeTicks(state_.timeout[ix]);
          }
        }
      }
    }
  }

  void onTimeoutEvent(TimeoutCallback callback) {
    timeoutCb_ = std::move(callback);
  }

private:
  TimerID getFreeTimerID() const {
    for (size_t ix = 0; ix < Capacity; ix++) {
      auto id = static_cast<TimerID>(ix);
      if (isTimerFree(ix)) {
        return id;
      }
    }

    return -1;
  }

  uint64_t computeTicks(uint32_t timeoutMs) {
    return (TicksFrequency * timeoutMs) / 1000;
  }

  bool isTimerFree(const size_t idx) const { return state_.timeout[idx] == 0; }

private:
  State state_{};
  TimeoutCallback timeoutCb_{nullptr};
};
} // namespace timer

