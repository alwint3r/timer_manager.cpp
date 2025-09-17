#include "systick.h"
#include <cstdint>
#include <cstdio>
#include <queue>
#include <unistd.h>

static constexpr size_t TimerCapacity = 8;
struct Timer {
  uint64_t ticksleft[TimerCapacity]{};
  uint32_t timeout[TimerCapacity]{};
  bool autoreload[TimerCapacity]{};
};

static constexpr auto TimerTicksHz = 1000;
inline uint64_t calculate_ticks_left(uint32_t timeout_ms) {
	return (TimerTicksHz * timeout_ms) / 1000;
}

static Timer timers;

using EventQueue = std::queue<size_t>;
static EventQueue events;

static void on_tick() {
  for (size_t id = 0; id < TimerCapacity; id++) {
		//
    if (timers.timeout[id] > 0) {
      timers.ticksleft[id] -= 1;
      if (timers.ticksleft[id] <= 0) {
        events.push(id);
        if (timers.autoreload[id]) {
          timers.ticksleft[id] = calculate_ticks_left(timers.timeout[id]);
        }
      }
    }
  }
}

int main() {
  if (systick_init(1000, on_tick) != 0) {
    fprintf(stderr, "Failed to initialize systick\n");
    return -1;
  }

  size_t currentId = 0;
  timers.timeout[currentId] = 1000;
  timers.autoreload[currentId] = true;
  timers.ticksleft[currentId] =
     calculate_ticks_left(timers.timeout[currentId]); 

  currentId++;
  timers.timeout[currentId] = 100;
  timers.autoreload[currentId] = false;
  timers.ticksleft[currentId] =
     calculate_ticks_left(timers.timeout[currentId]); 

  fprintf(stdout, "Timer 0: timeout = %u, autoreload = %d\n", timers.timeout[0],
          timers.autoreload[0]);
  fprintf(stdout, "Timer 1: timeout = %u, autoreload = %d\n", timers.timeout[1],
          timers.autoreload[1]);

  while (systick_ms() < 30 * 1000) {
    if (events.empty() == false) {
      auto timerId = events.front();
      events.pop();
      fprintf(stdout, "Timer ID %lu fired at %llu\n", timerId, systick_ms());
    }
  }

  systick_stop();

  return 0;
}
