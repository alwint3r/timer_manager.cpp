# timer_manager

A small, header-only timer manager tailored for embedded-style systems that expose a periodic tick. It stores timers in a fixed-size pool, delivers callbacks when timers expire, and can optionally auto-reload timers for periodic events.

## Highlights
- Header-only: include `include/timer_manager.h` and wire it into your tick ISR or scheduler.
- Deterministic storage: compile-time `Capacity` template parameter controls the number of timers.
- Flexible tick rate: `TicksFrequency` template parameter matches your system tick in Hz.
- Optional synchronization: enable `StdMutexPolicy` by defining `TIMER_ENABLE_STD_MUTEX` when you need thread safety.
- Example application for macOS that simulates a systick using Grand Central Dispatch.

## Requirements
- CMake 3.20+
- Ninja (or provide your own generator when configuring CMake)
- A C++23-capable compiler (tested with clang on macOS)
- For the macOS example: macOS 13+ with developer tools that support Blocks (`-fblocks`)

## Building the macOS example
1. Configure and build: `./build.sh`
2. Run the sample: `./run.sh`

`build.sh` configures CMake (default generator: Ninja) and builds the `example_mac` target. `run.sh` executes the resulting binary from `build/example_mac/example_mac`.

## Library overview
The primary entry point is `timer::TimerManager`:

```cpp
#include "timer_manager.h"

static constexpr size_t Capacity = 8;
static constexpr size_t TickHz = 1000; // one tick per millisecond

using TimerManager = timer::TimerManager<Capacity, TickHz>;
static TimerManager manager;

void systick_isr() {
    manager.processTick();
}

int main() {
    manager.onTimeoutEvent([](timer::TimerID id) {
        // react to the timer firing
    });

    auto id = manager.addNew(250, true); // 250 ms periodic timer
    // ... feed ticks via systick_isr()
}
```

Key operations include:
- `addNew(timeout_ms, auto_reload)` allocates a timer and arms it immediately.
- `cancelTimer(id)` stops the timer without freeing its slot.
- `removeTimer(id)` frees the slot for reuse.
- `changeTimeout(id, new_timeout_ms)` updates the period for the next expiration.
- `resume(id)` restarts a cancelled timer using its stored timeout.
- `onTimeoutEvent(cb)` installs a callback that receives each expired timer ID.

`processTick()` is lightweight and intended to be called from a periodic interrupt or scheduler. Expired timer IDs are collected and callbacks are invoked outside the guarded section to keep the critical path short.

## Configuration notes
- **Synchronization**: By default, `TimerManager` uses `NoLockPolicy`. Define `TIMER_ENABLE_STD_MUTEX` before including the header and pass `timer::StdMutexPolicy` as the `SyncPolicy` template argument to guard all state mutations with a `std::mutex`.
- **Custom callback type**: Supply your own functor type via the `CallbackT` template parameter if you want something lighter than `std::function`.
- **Embedded builds**: When `ESP_PLATFORM` is defined (e.g., inside an ESP-IDF project), the root `CMakeLists.txt` registers the component so you can add it directly to your firmware.

## Repository layout
- `include/timer_manager.h` — header-only timer manager implementation.
- `example_mac/` — sample application that drives the manager with a macOS systick shim.
- `build.sh` / `run.sh` — helper scripts for configuring, building, and running the macOS example.

Feel free to drop the header into your embedded project, tune the template parameters to match your system, and adapt the example systick shim to your platform.
