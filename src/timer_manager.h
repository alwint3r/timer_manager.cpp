#pragma once

#include <cstdint>
#include <functional>
#include <utility>
#ifdef TIMER_ENABLE_STD_MUTEX
#include <mutex>
#endif

namespace timer {

using TimerID = int32_t;

struct NoLockPolicy {
  struct Guard {
    Guard() noexcept {}
    ~Guard() = default;
  };
};

#ifdef TIMER_ENABLE_STD_MUTEX
struct StdMutexPolicy {
  struct Guard {
    Guard() { mutex().lock(); }
    ~Guard() { mutex().unlock(); }

  private:
    static std::mutex &mutex() {
      static std::mutex m;
      return m;
    }
  };
};
#endif

template <size_t Capacity, size_t TicksFrequency = 1000,
          typename SyncPolicy = NoLockPolicy,
          typename CallbackT = std::function<void(TimerID)>>
class TimerManager {
public:
  using TimeoutCallback = CallbackT;

  struct State {
    uint64_t ticksLeft[Capacity]{};
    uint32_t timeout[Capacity]{};
    bool autoReload[Capacity]{};
    bool allocated[Capacity]{};
    bool active[Capacity]{};
  };

  TimerID addNew(const uint32_t timeoutMs, const bool autoReload) {
    typename SyncPolicy::Guard guard{};
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
    typename SyncPolicy::Guard guard{};
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
    typename SyncPolicy::Guard guard{};
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
    typename SyncPolicy::Guard guard{};
    if (!isValidAllocated(id)) {
      return false;
    }

    size_t ix = static_cast<size_t>(id);
    state_.timeout[ix] = newTimeoutMs;
    state_.ticksLeft[ix] = computeTicks(newTimeoutMs);
    return true;
  }

  bool resume(const TimerID id) {
    typename SyncPolicy::Guard guard{};
    if (!isValidAllocated(id)) {
      return false;
    }
    size_t ix = static_cast<size_t>(id);
    state_.ticksLeft[ix] = computeTicks(state_.timeout[ix]);
    state_.active[ix] = true;
    return true;
  }

  void processTick() {
    TimerID expired[Capacity];
    size_t expiredCount = 0;
    TimeoutCallback cbCopy{};

    {
      typename SyncPolicy::Guard guard{};
      cbCopy = timeoutCb_;

      for (size_t ix = 0; ix < Capacity; ix++) {
        if (!state_.allocated[ix] || !state_.active[ix]) {
          continue;
        }

        if (state_.ticksLeft[ix] > 0) {
          state_.ticksLeft[ix] -= 1;
        }

        if (state_.ticksLeft[ix] == 0) {
          expired[expiredCount++] = static_cast<TimerID>(ix);

          if (state_.autoReload[ix]) {
            state_.ticksLeft[ix] = computeTicks(state_.timeout[ix]);
          } else {
            state_.active[ix] = false;
          }
        }
      }
    }

    if (cbCopy) {
      for (size_t i = 0; i < expiredCount; ++i) {
        std::invoke(cbCopy, expired[i]);
      }
    }
  }

  void onTimeoutEvent(TimeoutCallback callback) {
    typename SyncPolicy::Guard guard{};
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

  static constexpr uint64_t computeTicks(uint32_t timeoutMs) {
    return (static_cast<uint64_t>(TicksFrequency) *
            static_cast<uint64_t>(timeoutMs)) /
           1000u;
  }

  bool isTimerFree(const size_t idx) const { return !state_.allocated[idx]; }

  bool isValidAllocated(TimerID id) const {
    if (id < 0)
      return false;
    size_t ix = static_cast<size_t>(id);
    if (ix >= Capacity)
      return false;
    return state_.allocated[ix];
  }

private:
  State state_{};
  TimeoutCallback timeoutCb_{nullptr};
};

} // namespace timer
