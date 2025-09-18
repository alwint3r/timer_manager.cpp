#pragma once

#ifdef ESP_PLATFORM
extern "C" {
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
}
#endif

#include "timer_manager.h"

namespace esp32 {

template <size_t Capacity, size_t QueueDepth = 16, size_t TicksFrequency = 1000>
class TimerManagerCommandQueueAdapter {
  using Backend = timer::TimerManager<Capacity, TicksFrequency>;

public:
  using TimerID = timer::TimerID;
  using TimeoutCallback = typename Backend::TimeoutCallback;

  TimerManagerCommandQueueAdapter(Backend &backend)
      : backend_(backend)
#ifdef ESP_PLATFORM
        , queueStorage_{}, queueBuffer_{}, queue_(createQueue())
#endif
  {}

  ~TimerManagerCommandQueueAdapter() = default;

  bool requestAddTimer(uint32_t timeoutMs, bool autoReload, TimerID *result,
                       TickType_t waitTicks = portMAX_DELAY) {
    Command cmd{};
    cmd.type = Command::Type::Add;
    cmd.payload.add.timeoutMs = timeoutMs;
    cmd.payload.add.autoReload = autoReload;
    cmd.payload.add.result = result;
    return enqueue(cmd, waitTicks);
  }

  bool requestCancelTimer(TimerID id, bool *result,
                          TickType_t waitTicks = portMAX_DELAY) {
    Command cmd{};
    cmd.type = Command::Type::Cancel;
    cmd.payload.cancel.id = id;
    cmd.payload.cancel.result = result;
    return enqueue(cmd, waitTicks);
  }

  bool requestChangeTimeout(TimerID id, uint32_t newTimeoutMs, bool *result,
                            TickType_t waitTicks = portMAX_DELAY) {
    Command cmd{};
    cmd.type = Command::Type::ChangeTimeout;
    cmd.payload.change.id = id;
    cmd.payload.change.timeoutMs = newTimeoutMs;
    cmd.payload.change.result = result;
    return enqueue(cmd, waitTicks);
  }

  bool requestCallbackInstall(TimeoutCallback *callback,
                              TickType_t waitTicks = portMAX_DELAY) {
    Command cmd{};
    cmd.type = Command::Type::InstallCallback;
    cmd.payload.callback.cb = callback;
    return enqueue(cmd, waitTicks);
  }

  bool requestProcessTickFromISR(BaseType_t *woken = nullptr) {
    Command cmd{};
    cmd.type = Command::Type::ProcessTick;
    return enqueueFromISR(cmd, woken);
  }

  void serviceLoop(TickType_t waitTicks = portMAX_DELAY) {
    while (true) {
      Command cmd{};
      if (!dequeue(cmd, waitTicks)) {
        continue;
      }
      dispatch(cmd);
    }
  }

private:
  struct Command {
    enum class Type : uint8_t {
      Add,
      Cancel,
      ChangeTimeout,
      InstallCallback,
      ProcessTick
    } type{};

    struct AddPayload {
      uint32_t timeoutMs{};
      bool autoReload{};
      TimerID *result{};
    };

    struct CancelPayload {
      TimerID id{};
      bool *result{};
    };

    struct ChangePayload {
      TimerID id{};
      uint32_t timeoutMs{};
      bool *result{};
    };

    struct CallbackPayload {
      TimeoutCallback *cb{};
    };

    union Payload {
      AddPayload add;
      CancelPayload cancel;
      ChangePayload change;
      CallbackPayload callback;
    } payload{};
  };

  void dispatch(Command &cmd) {
    switch (cmd.type) {
    case Command::Type::Add: {
      if (cmd.payload.add.result != nullptr) {
        *(cmd.payload.add.result) =
            backend_.addNew(cmd.payload.add.timeoutMs, cmd.payload.add.autoReload);
      } else {
        (void)backend_.addNew(cmd.payload.add.timeoutMs, cmd.payload.add.autoReload);
      }
      break;
    }
    case Command::Type::Cancel: {
      if (cmd.payload.cancel.result != nullptr) {
        *(cmd.payload.cancel.result) = backend_.cancelTimer(cmd.payload.cancel.id);
      } else {
        (void)backend_.cancelTimer(cmd.payload.cancel.id);
      }
      break;
    }
    case Command::Type::ChangeTimeout: {
      if (cmd.payload.change.result != nullptr) {
        *(cmd.payload.change.result) = backend_.changeTimeout(
            cmd.payload.change.id, cmd.payload.change.timeoutMs);
      } else {
        (void)backend_.changeTimeout(cmd.payload.change.id,
                                     cmd.payload.change.timeoutMs);
      }
      break;
    }
    case Command::Type::InstallCallback: {
      if (cmd.payload.callback.cb != nullptr) {
        backend_.onTimeoutEvent(std::move(*(cmd.payload.callback.cb)));
      }
      break;
    }
    case Command::Type::ProcessTick: {
      backend_.processTick();
      break;
    }
    }
  }

  bool enqueue(const Command &cmd, TickType_t waitTicks) {
#ifdef ESP_PLATFORM
    return xQueueSend(queue_, &cmd, waitTicks) == pdTRUE;
#else
    (void)cmd;
    (void)waitTicks;
    return false;
#endif
  }

  bool enqueueFromISR(const Command &cmd, BaseType_t *woken) {
#ifdef ESP_PLATFORM
    return xQueueSendFromISR(queue_, &cmd, woken) == pdTRUE;
#else
    (void)cmd;
    (void)woken;
    return false;
#endif
  }

  bool dequeue(Command &cmd, TickType_t waitTicks) {
#ifdef ESP_PLATFORM
    return xQueueReceive(queue_, &cmd, waitTicks) == pdTRUE;
#else
    (void)cmd;
    (void)waitTicks;
    return false;
#endif
  }

#ifdef ESP_PLATFORM
  QueueHandle_t createQueue() {
    return xQueueCreateStatic(QueueDepth, sizeof(Command), queueStorage_, &queueBuffer_);
  }
#endif

private:
  Backend &backend_;
#ifdef ESP_PLATFORM
  StaticQueue_t queueBuffer_;
  uint8_t queueStorage_[QueueDepth * sizeof(Command)];
  QueueHandle_t queue_;
#endif
};

} // namespace esp32

