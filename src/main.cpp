#include "systick.h"
#include "timer_manager.h"
#include <cstdio>
#include <print>
#include <queue>
#include <unistd.h>

static constexpr size_t TimerCapacity = 8;
static timer::TimerManager<TimerCapacity> manager;
using EventQueue = std::queue<timer::TimerID>;
static EventQueue events;

static void on_tick() { manager.processTick(); }

int main() {
  if (systick_init(1000, on_tick) != 0) {
    fprintf(stderr, "Failed to initialize systick\n");
    return -1;
  }
  manager.onTimeoutEvent([](timer::TimerID id) { events.push(id); });
  manager.addNew(100, false);
  manager.addNew(300, true);
  manager.addNew(1000, true);

  while (systick_ms() < 33 * 1000) {
    if (events.empty() == false) {
      auto timerId = events.front();
      events.pop();
      fprintf(stdout, "Timer ID %d fired at %llu\n", timerId, systick_ms());
    }
  }

  systick_stop();

  return 0;
}
