#include QMK_KEYBOARD_H

/* ───── レイヤー定義 ───── */
enum layer_names { _BASE, _SCROLL };

/* ───── 独自キーコード ───── */
enum custom_keycodes {
    CPI_TGL = SAFE_RANGE,   // 左クリック置換（既存）
    SCRL_MODE               // スクロール方向切替
};

/* ───── キーマップ ───── */
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /* 基本レイヤー（物理 KC_BTN4 は Back ボタンのまま） */
    [_BASE] = LAYOUT_direct(
        KC_BTN1, KC_BTN2, KC_BTN3, KC_BTN4, KC_BTN5,
        MO(_SCROLL)                 // DRAG_SCROLL
    ),

    /* Drag-Scroll レイヤー */
    [_SCROLL] = LAYOUT_direct(
        CPI_TGL,  KC_TRNS,  KC_TRNS, SCRL_MODE, KC_TRNS,
        KC_TRNS                    // DRAG_SCROLL キー自身（レイヤー維持）
    ),
};

/* ───── 状態フラグ ───── */
static bool drag_scroll = false;
static bool cpi_low    = false;

/* スクロールモード（0:両,1:Y のみ,2:X のみ） */
enum { SCROLL_BOTH = 0, SCROLL_YONLY, SCROLL_XONLY };
static uint8_t scroll_mode = SCROLL_BOTH;

/* ───── CPI 定数 ───── */
static const uint16_t CPI_DEF    = PMW33XX_CPI;   // 例: 600 dpi
static const uint16_t CPI_ALT    = CPI_DEF / 3;   // 例: 200 dpi
static const uint16_t CPI_SCROLL = 50;            // Drag-Scroll 中固定

/* ───── キー処理 ───── */
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {

        /* 左クリック → CPI_TGL に置換済み */
        case CPI_TGL:
            if (record->event.pressed)
                cpi_low = !cpi_low;
            return false;

        /* SCRL_MODE：スクロール方向を循環 */
        case SCRL_MODE:
            if (drag_scroll && record->event.pressed) {
                scroll_mode = (scroll_mode + 1) % 3;   // 0→1→2→0…
            }
            return false;           // OS へ送らない（クリック無し）

    }
    return true;
}

/* ───── レイヤー遷移で CPI 制御 ───── */
layer_state_t layer_state_set_user(layer_state_t state) {
    bool now_scroll = layer_state_cmp(state, _SCROLL);
    if (now_scroll != drag_scroll) {
        drag_scroll = now_scroll;
        pointing_device_set_cpi(
            drag_scroll ? CPI_SCROLL
                        : (cpi_low ? CPI_ALT : CPI_DEF)
        );
    }
    return state;
}

/* ───── ポインタ → スクロール変換 ───── */
report_mouse_t pointing_device_task_user(report_mouse_t m) {
    if (drag_scroll) {
        switch (scroll_mode) {
            case SCROLL_YONLY:
                m.h = 0;
                m.v = -m.y;
                break;
            case SCROLL_XONLY:
                m.h =  m.x;
                m.v = 0;
                break;
            default:               // SCROLL_BOTH
                m.h =  m.x;
                m.v = -m.y;
        }
        m.x = m.y = 0;             // マウス移動は常に無効化
    }
    return m;
}

