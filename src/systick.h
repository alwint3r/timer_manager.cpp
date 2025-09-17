#pragma once

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef void (*systick_cb_t)(void);
int systick_init(uint32_t hz, systick_cb_t cb);
void systick_stop(void);
uint64_t systick_ticks(void);
uint64_t systick_ms(void);

#if defined(__cplusplus)
}
#endif
