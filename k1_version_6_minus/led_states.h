#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

    void set_led_caps_lock_on(void);
    void set_led_caps_lock_off(void);

    typedef enum {
        LED_GREEN = 0,
        LED_BLUE,
        LED_CYAN,
        LED_COUNT
    } led_id_t;

    typedef struct led_sm led_sm_t;

    typedef void (*led_hw_fn_t)(void);

    static const uint32_t LED_FLASH_ON_TIME_MS = 120;
    static const uint32_t LED_FLASHING_ON_TIME_MS = 80;
    static const uint32_t LED_FLASHING_OFF_TIME_MS = 120;

    void initialize_leds(void);

    led_sm_t *led_get(led_id_t id);

    void ledInit(led_sm_t *led, led_hw_fn_t on, led_hw_fn_t off);
    void ledOn(led_sm_t *led);
    void ledOff(led_sm_t *led);

    void ledFlashOnce(led_sm_t *led);
    void ledFlashOnceFor(led_sm_t *led, uint32_t flash_time_ms);

    void ledStartFlashing(led_sm_t *led);
    void ledStartFlashingWith(led_sm_t *led, uint32_t flashing_on_time_ms, uint32_t flashing_off_time_ms);

    void ledStartFlashingNTimes(led_sm_t *led, uint16_t n_times);
    void ledStartFlashingNTimesWith(led_sm_t *led, uint32_t flashing_on_time_ms, uint32_t flashing_off_time_ms, uint16_t n_times);

#ifdef __cplusplus
}
#endif
