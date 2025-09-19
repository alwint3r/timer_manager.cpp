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
    bool allocated[Capacity]{};
    bool active[Capacity]{};
  };

  TimerID addNew(const uint32_t timeoutMs, const bool autoReload) {
    auto id = getFreeTimerID();
    if (id < 0) {
      return -1;
    }

    size_t ix = static_cast<size_t>(id);
    state_.allocated[ix] = true;
    state_.active[ix] = true;
    state_.timeout[ix] = timeoutMs;
    state_.autoReload[ix] = autoReload;
    state_.ticksLeft[ix] = computeTicks(timeoutMs);

    return id;
  }

  bool cancelTimer(const TimerID id) {
    if (!isValidAllocated(id)) {
      return false;
    }

    size_t ix = static_cast<size_t>(id);
    state_.active[ix] = false;
    state_.ticksLeft[ix] = 0;
    state_.autoReload[ix] = false;
    return true;
  }

  bool removeTimer(const TimerID id) {
    if (!isValidAllocated(id)) {
      return false;
    }

    size_t ix = static_cast<size_t>(id);
    state_.allocated[ix] = false;
    state_.active[ix] = false;
    state_.timeout[ix] = 0;
    state_.autoReload[ix] = false;
    state_.ticksLeft[ix] = 0;
    return true;
  }

  bool changeTimeout(const TimerID id, uint32_t newTimeoutMs) {
    if (!isValidAllocated(id)) {
      return false;
    }

    size_t ix = static_cast<size_t>(id);
    state_.timeout[ix] = newTimeoutMs;
    state_.ticksLeft[ix] = computeTicks(newTimeoutMs);
    return true;
  }

  bool resume(const TimerID id) {
    if (!isValidAllocated(id)) {
      return false;
    }
    size_t ix = static_cast<size_t>(id);
    state_.ticksLeft[ix] = computeTicks(state_.timeout[ix]);
    state_.active[ix] = true;
    return true;
  }

  void processTick() {
    for (size_t ix = 0; ix < Capacity; ix++) {
      if (!state_.allocated[ix] || !state_.active[ix]) {
        continue;
      }

      if (state_.ticksLeft[ix] > 0) {
        state_.ticksLeft[ix] -= 1;
      }

      if (state_.ticksLeft[ix] == 0) {
          if (timeoutCb_) {
            std::invoke(timeoutCb_, static_cast<TimerID>(ix));
          }

          if (state_.autoReload[ix]) {
            state_.ticksLeft[ix] = computeTicks(state_.timeout[ix]);
        } else {
          state_.active[ix] = false; // one-shot stops but stays allocated
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

  bool isTimerFree(const size_t idx) const { return !state_.allocated[idx]; }

  bool isValidAllocated(TimerID id) const {
    if (id < 0) return false;
    size_t ix = static_cast<size_t>(id);
    if (ix >= Capacity) return false;
    return state_.allocated[ix];
  }

private:
  State state_{};
  TimeoutCallback timeoutCb_{nullptr};
};
} // namespace timer
