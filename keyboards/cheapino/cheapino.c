#include "wait.h"
#include "quantum.h"

// This is to keep state between callbacks, when it is 0 the
// initial RGB flash is finished
uint8_t _hue_countdown = 50;

// These are to keep track of user selected color, so we
// can restore it after RGB flash
uint8_t _hue;
uint8_t _saturation;
uint8_t _value;

// Do a little 2.5 seconds display of the different colors
// Use the deferred executor so the LED flash dance does not
// stop us from using the keyboard.
// https://docs.qmk.fm/#/custom_quantum_functions?id=deferred-executor-registration
uint32_t flash_led(uint32_t next_trigger_time, void *cb_arg) {
    rgblight_sethsv(_hue_countdown * 5, 230, 70);
    _hue_countdown--;
    if (_hue_countdown == 0) {
        // Finished, reset to user chosen led color
        rgblight_sethsv(_hue, _saturation, _value);
        return 0;
    } else {
        return 50;
    }
}

void keyboard_post_init_user(void) {
    //debug_enable=true;
    //debug_matrix=true;
    //debug_keyboard=true;
    //debug_mouse=true;

    // Store user selected rgb hsv:
    _hue = rgblight_get_hue();
    _saturation = rgblight_get_sat();
    _value = rgblight_get_val();

    // Flash a little on start
    defer_exec(50, flash_led, NULL);
}

// Make the builtin RGB led show different colors per layer:
// This seemed like a good idea but turned out pretty annoying,
// to me at least... Uncomment the lines below to enable
uint8_t get_hue(uint8_t layer) {
    switch (layer) {
        case 6:
            return 169;
        case 5:
            return 43;
        case 4:
            return 85;
        case 3:
            return 120;
        case 2:
            return 180;
        case 1:
            return 220;
        default:
            return 0;
    }
}

layer_state_t layer_state_set_user(layer_state_t state) {
    uint8_t sat = rgblight_get_sat();
    uint8_t val = rgblight_get_val();
    uint8_t hue = get_hue(get_highest_layer(state));
    rgblight_sethsv(hue, sat, val);
    return state;
}

// This tracks if the keyboard has been active in the last 10 seconds
// and turns the rgb light off if inactive. otherwise it does nothing.
uint32_t last_key_pressed_time = 0;
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed){
        last_key_pressed_time = timer_read32();
        if (!rgblight_is_enabled()) {
            rgblight_enable();
        }
    }
    return true;
}
void matrix_scan_user(void) {
    if (timer_elapsed32(last_key_pressed_time) > 10000) {
        rgblight_disable();
    }
}

// manually configure the breathing mode because config.h won't pick it up for some reason
// void matrix_init_user(void) {
//     if (rgblight_is_enabled()) {
//         rgblight_mode(RGBLIGHT_MODE_BREATHING);
//     }
// }

