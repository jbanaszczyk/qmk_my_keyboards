# QMK LED State Machines (Deferred Execution)

A small module for controlling multiple independent LEDs in QMK using the Deferred Execution subsystem.

Key properties:

- One LED = one `led_sm_t` instance (opaque type to callers)
- At most **one deferred token scheduled per LED instance** at any time
- Operations are mutually exclusive: `ledOn/ledOff/ledFlashOnce*/ledStartFlashing*` cancel any previous automation for
  that LED
- Flashing is implemented as a ping-pong of two callbacks

---

## Requirements

Enable deferred execution in `rules.mk`:

```make
DEFERRED_EXEC_ENABLE = yes
````

If you hit token allocation failures, increase the limit in `config.h`:

```c
#define MAX_DEFERRED_EXECUTORS 16
```

---

## Files

* `led_states.h` — public API
* `led_states.c` — implementation + hardware mapping (GPIO init and `set_led_*` functions)

---

## LED IDs

LEDs are identified by `led_id_t`:

```c
typedef enum {
    LED_GREEN = 0,
    LED_BLUE,
    LED_CYAN,
    LED_CAPS_LOCK,
    LED_COUNT
} led_id_t;
```

Use `led_get(id)` to access a particular LED state machine:

```c
led_sm_t *led_get(led_id_t id);
```

---

## Initialization

Call `initialize_leds()` once during firmware startup (e.g. in `keyboard_post_init_user()`):

```c
#include "led_states.h"

void keyboard_post_init_user(void) {
    initialize_leds();
}
```

`initialize_leds()`:

* configures GPIO pins (`initialize_led_pins()` inside `led_states.c`)
* initializes each `led_sm_t` with the corresponding hardware on/off functions

---

## Operations

### Steady ON / OFF

```c
ledOn(led_get(LED_BLUE));
ledOff(led_get(LED_BLUE));
```

### Flash once (default duration)

```c
ledFlashOnce(led_get(LED_GREEN));
```

### Flash once (custom duration)

```c
ledFlashOnceFor(led_get(LED_GREEN), 200);
```

### Flash indefinitely (default on/off times)

```c
ledStartFlashing(led_get(LED_CYAN));
```

### Flash indefinitely (custom on/off times)

```c
ledStartFlashingWith(led_get(LED_CYAN), 80, 120);
```

### Flash N times (default on/off times)

`n_times` is the number of ON→OFF pulses:

* `n_times = 0` => no flashing, ends OFF
* `n_times = 1` => one pulse (ON then OFF)

```c
ledStartFlashingNTimes(led_get(LED_CAPS_LOCK), 3);
```

### Flash N times (custom on/off times)

```c
ledStartFlashingNTimesWith(led_get(LED_CAPS_LOCK), 60, 90, 5);
```

---

## Timing defaults

Default timings are defined in `led_states.h`:

```c
static const uint32_t LED_FLASH_ON_TIME_MS = 120;
static const uint32_t LED_FLASHING_ON_TIME_MS = 80;
static const uint32_t LED_FLASHING_OFF_TIME_MS = 120;
```

---

## Notes

* `led_sm_t` is opaque in the header: callers can store pointers but cannot access internal fields.
* The module uses `defer_exec()` and `cancel_deferred_exec()` from QMK deferred execution.
* Each LED instance maintains its own token; tokens are never shared across LEDs.
