#include <sys/qos.h>
#define _DARWIN_C_SOURCE
#include "systick.h"
#include <dispatch/dispatch.h>
#include <mach/mach_time.h>
#include <stdatomic.h>
#include <stdint.h>

static dispatch_source_t g_timer;
static systick_cb_t g_cb;
static atomic_uint_least64_t g_ticks;
static uint64_t g_t0_abs, g_numer, g_denom;

static inline uint64_t ns_from_abs(uint64_t t) {
  return (t * g_numer) / g_denom;
}

#if defined(__cplusplus)
extern "C" {
#endif

int systick_init(uint32_t hz, systick_cb_t cb) {
  if (hz == 0 || cb == NULL)
    return -1;
  mach_timebase_info_data_t tb;
  mach_timebase_info(&tb);
  g_numer = tb.numer;
  g_denom = tb.denom;
  g_t0_abs = mach_absolute_time();
  g_cb = cb;
  atomic_store(&g_ticks, 0);

  uint64_t interval = 1000000000ull / hz;
  g_timer = dispatch_source_create(
      DISPATCH_SOURCE_TYPE_TIMER, 0, 0,
      dispatch_get_global_queue(QOS_CLASS_USER_INTERACTIVE, 0));
  if (!g_timer)
    return -1;
  dispatch_source_set_timer(g_timer, dispatch_time(DISPATCH_TIME_NOW, interval),
                            interval, 0);
  dispatch_source_set_event_handler(g_timer, ^{
    atomic_fetch_add_explicit(&g_ticks, 1, memory_order_relaxed);
    if (g_cb)
      g_cb();
  });
  dispatch_resume(g_timer);
  return 0;
}

void systick_stop(void) {
  if (g_timer) {
    dispatch_source_cancel(g_timer);
    g_timer = NULL;
  }
}

uint64_t systick_ticks(void) {
  return atomic_load_explicit(&g_ticks, memory_order_relaxed);
}

uint64_t systick_ms(void) {
  uint64_t now = mach_absolute_time();
  uint64_t ns = ns_from_abs(now - g_t0_abs);
  return ns / 1000000ull;
}

#if defined(__cplusplus)
}
#endif
