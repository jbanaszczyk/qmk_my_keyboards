/* Copyright 2024 ~ 2025 @ Keychron (https://www.keychron.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include "quantum.h"
#include "print.h"

#include "eeprom.h"
#include "eeconfig.h"
#include "nvm_eeprom_eeconfig_internal.h"

#include "led_states.h"

#define POWER_ON_LED_DURATION 3000
static uint32_t power_on_indicator_timer;

/*
bool dip_switch_update_user(uint8_t index, bool active) {
    if (index == 0) {
        default_layer_set(1UL << (active ? 0 : 2));
    }
    dip_switch_update_user(index, active);

    return true;
}
*/


void keyboard_post_init_kb(void) {
    uprintf("___ postinit\n");
    initialize_leds();

    power_on_indicator_timer = timer_read32();

    keyboard_post_init_user();
}

void keyboard_post_init_user(void) {
    debug_enable = true;
}

void housekeeping_task_kb(void) {
    if (power_on_indicator_timer) {
        if (timer_elapsed32(power_on_indicator_timer) <= POWER_ON_LED_DURATION) {
            // gpio_write_pin(LED_CAPS_LOCK_PIN, LED_PIN_ON_STATE);

        } else {
            power_on_indicator_timer = 0;

            if (!host_keyboard_led_state().caps_lock) {
                // gpio_write_pin(LED_CAPS_LOCK_PIN, !LED_PIN_ON_STATE);
            }

            uprintf("Power-on duration finished.\n");
        }
    }
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    uprintf("___ process_record_user\n");
    if (record->event.pressed) {
        led_id_t led = LED_CYAN;
        if (keycode == KC_F9) {
            uprintf("_______ led off\n");
            ledOff(led_get(led));
            return false;
        }
        if (keycode == KC_F10) {
            uprintf("_______ ledOn\n");
            ledOn(led_get(led));
            return false;
        }
        if (keycode == KC_F11) {
            uprintf("_______ ledStartFlashing\n");
            ledStartFlashing(led_get(led));
            return false;
        }
        if (keycode == KC_F12) {
            uprintf("_______ ledFlashOnce\n");
            ledStartFlashingNTimes(led_get(led), 3);
            return false;
        }
    }

    return true;
}

bool pre_process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        uprintf("___ pre_process_record_user\n");
        if (keycode == DM_REC1) {
            uprintf("_______ DM_REC1\n");
        }
        if (keycode == DM_RSTP) {
            uprintf("_______ DM_RSTP\n");
        }
    }
    return true;
}
