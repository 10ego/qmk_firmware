#include "matrix.h"
#include "quantum.h"

#define COL_SHIFTER ((uint16_t)1)

static bool colABpressed = false;
static bool encoderPressed = false;
static bool lgui_held = false;

void clicked(void) {
    if (IS_LAYER_ON(1)) {
        // media play / pause
        tap_code(KC_MPLY);
    } else if (IS_LAYER_ON(2)) {
        // releaseq LGUI key and switch to selected window
        unregister_code(KC_LGUI);
        lgui_held = false;
    } else if (IS_LAYER_ON(4)) {
        // move window down
        tap_code16(LAG(LSFT(KC_DOWN)));
    } else {
        // mute
        tap_code(KC_MUTE);
    }
}

void turned(bool clockwise) {
    if (IS_LAYER_ON(1)) {
        // next / prev media
        tap_code16(clockwise ? KC_MNXT : LSFT(KC_MPRV));
    } else if (IS_LAYER_ON(2)) {
        // hold LGUI and press tab / shift+tab until encoder is clicked
        if (!lgui_held) {
            register_code(KC_LGUI);
            lgui_held=true;
        }
        if (clockwise) {
            tap_code(KC_TAB);
        } else {
            register_code(KC_LSFT);
            tap_code(KC_TAB);
            unregister_code(KC_LSFT);
        }
    } else if (IS_LAYER_ON(4)) {
        // move window left / right
        tap_code16(clockwise ? LAG(LSFT(KC_RIGHT)) : LAG(LSFT(KC_LEFT)));
    } else {
        // volume up / down
        tap_code(clockwise ? KC_VOLU : KC_VOLD);
    }
}

void blank_column(matrix_row_t current_matrix[], uint8_t col) {
    uint16_t column_index_bitmask = COL_SHIFTER << col;
    for (uint8_t row_index = 0; row_index < MATRIX_ROWS-1; row_index++) {
        current_matrix[row_index] &= ~column_index_bitmask;
    }
}

bool is_entire_column_held(matrix_row_t current_matrix[], uint8_t col) {
    uint16_t column_index_bitmask = COL_SHIFTER << col;
    for (uint8_t row_index = 0; row_index < MATRIX_ROWS-1; row_index++) {
        if (!(current_matrix[row_index] & column_index_bitmask)) return false;
    }
    return true;
}

// Because of a bug in the routing of the cheapino, encoder action
// triggers entire columns... fix it in software here, assumption is that
// you never press an entire column, sounds safe?
void fix_encoder_action(matrix_row_t current_matrix[]) {

    // 7th column means encoder was pressed
    if (is_entire_column_held(current_matrix, 7)) {
        encoderPressed = true;
        blank_column(current_matrix, 7);
        current_matrix[3] |= COL_SHIFTER;
    } else {
        current_matrix[3] &= ~COL_SHIFTER;
        // Only trigger click on release
        if (encoderPressed) {
            encoderPressed = false;
            clicked();
        }
    }

    // Now check rotary action:
    bool colA = is_entire_column_held(current_matrix, 9);
    bool colB = is_entire_column_held(current_matrix, 11);

    if (colA && colB) {
        colABpressed = true;
        blank_column(current_matrix, 9);
        blank_column(current_matrix, 11);
    } else if (colA) {
        if (colABpressed) {
            colABpressed = false;
            turned(true);
        }
        blank_column(current_matrix, 9);
    } else if (colB) {
        if (colABpressed) {
            colABpressed = false;
            turned(false);
        }
        blank_column(current_matrix, 11);
    }
}
