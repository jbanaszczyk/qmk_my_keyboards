#include "quantum.h"
#include "led_states.h"

#include "deferred_exec.h"

typedef enum {
    LED_STEADY_OFF = 0,
    LED_STEADY_ON,
    LED_FLASH_ONCE_ON,
    LED_FLASHING_ON,
    LED_FLASHING_OFF,
} led_state_t;

struct led_sm {
    deferred_token token;
    uint8_t state;
    led_hw_fn_t hw_on;
    led_hw_fn_t hw_off;
    uint32_t flashing_on_time_ms;
    uint32_t flashing_off_time_ms;
    uint16_t flashing_remaining;
};

static void led_cancel_token(led_sm_t *led) {
    if (led->token != INVALID_DEFERRED_TOKEN) {
        cancel_deferred_exec(led->token);
        led->token = INVALID_DEFERRED_TOKEN;
    }
}

static uint32_t led_cb_flash_once_off(uint32_t trigger_time __attribute__((unused)), void *cb_arg);

static uint32_t led_cb_flashing_on(uint32_t trigger_time __attribute__((unused)), void *cb_arg);

static uint32_t led_cb_flashing_off(uint32_t trigger_time __attribute__((unused)), void *cb_arg);

static uint32_t led_cb_flash_once_off(uint32_t trigger_time __attribute__((unused)), void *cb_arg) {
    led_sm_t *led = cb_arg;

    led->hw_off();
    led->state = (uint8_t) LED_STEADY_OFF;
    led->token = INVALID_DEFERRED_TOKEN;
    return 0;
}

static uint32_t led_cb_flashing_on(uint32_t trigger_time __attribute__((unused)), void *cb_arg) {
    led_sm_t *led = cb_arg;

    led->hw_on();
    led->state = (uint8_t) LED_FLASHING_ON;

    led->token = defer_exec(led->flashing_on_time_ms, led_cb_flashing_off, led);
    if (led->token == INVALID_DEFERRED_TOKEN) {
        led->state = (uint8_t) LED_STEADY_OFF;
        led->flashing_remaining = 0;
        led->hw_off();
    }
    return 0;
}

static uint32_t led_cb_flashing_off(uint32_t trigger_time __attribute__((unused)), void *cb_arg) {
    led_sm_t *led = cb_arg;

    led->hw_off();
    led->state = (uint8_t)LED_FLASHING_OFF;

    if (led->flashing_remaining > 0) {
        led->flashing_remaining--;
        if (led->flashing_remaining == 0) {
            led->state = (uint8_t)LED_STEADY_OFF;
            led->token = INVALID_DEFERRED_TOKEN;
            return 0;
        }
    }

    led->token = defer_exec(led->flashing_off_time_ms, led_cb_flashing_on, led);
    if (led->token == INVALID_DEFERRED_TOKEN) {
        led->state = (uint8_t)LED_STEADY_OFF;
        led->flashing_remaining = 0;
        led->hw_off();
    }
    return 0;
}

void ledInit(led_sm_t *led, const led_hw_fn_t on, const led_hw_fn_t off) {
    led->hw_on = on;
    led->hw_off = off;
    led->token = INVALID_DEFERRED_TOKEN;
    led->state = (uint8_t) LED_STEADY_OFF;
    led->flashing_on_time_ms = LED_FLASHING_ON_TIME_MS;
    led->flashing_off_time_ms = LED_FLASHING_OFF_TIME_MS;
    led->flashing_remaining = 0;
    led->hw_off();
}

void ledOn(led_sm_t *led) {
    led_cancel_token(led);
    led->state = (uint8_t) LED_STEADY_ON;
    led->flashing_remaining = 0;
    led->hw_on();
}

void ledOff(led_sm_t *led) {
    led_cancel_token(led);
    led->state = (uint8_t) LED_STEADY_OFF;
    led->flashing_remaining = 0;
    led->hw_off();
}

void ledFlashOnce(led_sm_t *led) {
    ledFlashOnceFor(led, LED_FLASH_ON_TIME_MS);
}

void ledFlashOnceFor(led_sm_t *led, const uint32_t flash_time_ms) {
    led_cancel_token(led);
    led->state = (uint8_t) LED_FLASH_ONCE_ON;
    led->flashing_remaining = 0;
    led->hw_on();

    led->token = defer_exec(flash_time_ms, led_cb_flash_once_off, led);
    if (led->token == INVALID_DEFERRED_TOKEN) {
        led->state = (uint8_t) LED_STEADY_OFF;
        led->hw_off();
    }
}

void ledStartFlashing(led_sm_t *led) {
    ledStartFlashingWith(led, LED_FLASHING_ON_TIME_MS, LED_FLASHING_OFF_TIME_MS);
}

void ledStartFlashingWith(led_sm_t *led, const uint32_t flashing_on_time_ms, const uint32_t flashing_off_time_ms) {
    led_cancel_token(led);

    led->flashing_on_time_ms = flashing_on_time_ms;
    led->flashing_off_time_ms = flashing_off_time_ms;
    led->flashing_remaining = 0;

    led->state = (uint8_t) LED_FLASHING_ON;
    led->hw_on();

    led->token = defer_exec(led->flashing_on_time_ms, led_cb_flashing_off, led);
    if (led->token == INVALID_DEFERRED_TOKEN) {
        led->state = (uint8_t) LED_STEADY_OFF;
        led->hw_off();
    }
}

void ledStartFlashingNTimes(led_sm_t *led, const uint16_t n_times) {
    ledStartFlashingNTimesWith(led, LED_FLASHING_ON_TIME_MS, LED_FLASHING_OFF_TIME_MS, n_times);
}

void ledStartFlashingNTimesWith(led_sm_t *led, const uint32_t flashing_on_time_ms, const uint32_t flashing_off_time_ms,
                                const uint16_t n_times) {
    led_cancel_token(led);

    if (n_times == 0) {
        led->state = (uint8_t) LED_STEADY_OFF;
        led->flashing_remaining = 0;
        led->hw_off();
        return;
    }

    led->flashing_on_time_ms = flashing_on_time_ms;
    led->flashing_off_time_ms = flashing_off_time_ms;
    led->flashing_remaining = n_times;

    led->state = (uint8_t) LED_FLASHING_ON;
    led->hw_on();

    led->token = defer_exec(led->flashing_on_time_ms, led_cb_flashing_off, led);
    if (led->token == INVALID_DEFERRED_TOKEN) {
        led->state = (uint8_t) LED_STEADY_OFF;
        led->flashing_remaining = 0;
        led->hw_off();
    }
}

static led_sm_t leds[LED_COUNT];

static void initialize_led_pins(void) {
    gpio_set_pin_output_push_pull(LED_CAPS_LOCK_PIN);
    gpio_write_pin(LED_CAPS_LOCK_PIN, !LED_PIN_ON_STATE);

    gpio_set_pin_output_push_pull(LED_BLUE_PIN);
    gpio_write_pin(LED_BLUE_PIN, !LED_BLUE_ON_STATE);

    gpio_set_pin_output_push_pull(LED_GREEN_PIN);
    gpio_write_pin(LED_GREEN_PIN, !LED_GREEN_ON_STATE);
}

static void set_led_blue_on(void) { gpio_write_pin(LED_BLUE_PIN, LED_BLUE_ON_STATE); }
static void set_led_blue_off(void) { gpio_write_pin(LED_BLUE_PIN, !LED_BLUE_ON_STATE); }
static void set_led_green_on(void) { gpio_write_pin(LED_GREEN_PIN, LED_GREEN_ON_STATE); }
static void set_led_green_off(void) { gpio_write_pin(LED_GREEN_PIN, !LED_GREEN_ON_STATE); }

static void set_led_cyan_on(void) {
    set_led_blue_on();
    set_led_green_on();
}

static void set_led_cyan_off(void) {
    set_led_blue_off();
    set_led_green_off();
}

void set_led_caps_lock_on(void) { gpio_write_pin(LED_CAPS_LOCK_PIN, LED_PIN_ON_STATE); }
void set_led_caps_lock_off(void) { gpio_write_pin(LED_CAPS_LOCK_PIN, !LED_PIN_ON_STATE); }

void initialize_leds(void) {
    initialize_led_pins();
    ledInit(&leds[LED_GREEN], set_led_green_on, set_led_green_off);
    ledInit(&leds[LED_BLUE], set_led_blue_on, set_led_blue_off);
    ledInit(&leds[LED_CYAN], set_led_cyan_on, set_led_cyan_off);
}

led_sm_t *led_get(led_id_t id) {
    if (id >= LED_COUNT) {
        id = LED_GREEN;
    }
    return &leds[(uint8_t) id];
}
