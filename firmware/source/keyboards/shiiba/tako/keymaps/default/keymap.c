#include QMK_KEYBOARD_H

/* ───── レイヤー定義 ───── */
enum layer_names { _BASE, _SCROLL };

/* ───── 独自キーコード ───── */
enum custom_keycodes {
    CPI_TGL = SAFE_RANGE,   // CPI 切替
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

/* ---- スクロール平滑化用 ---- */
#define SCROLL_UNIT  40        // Δが ±40cnt たまったら wheel ±1step 送る
static int16_t rem_x = 0;
static int16_t rem_y = 0;

/* ───── CPI 定数 ───── */
static const uint16_t CPI_DEF    = PMW33XX_CPI;   // 例: 600 dpi
static const uint16_t CPI_ALT    = CPI_DEF / 3;   // 例: 200 dpi
static const uint16_t CPI_SCROLL = CPI_DEF;       // Drag-Scroll 中固定

/* ───── キー処理 ───── */
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {

        /* CPI_TGL：CPI 状態を反転 */
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
        /* ① Δを蓄積 */
        rem_x += m.x;
        rem_y += m.y;

        /* ② SCROLL_UNIT ごとに ±1step へ変換 */
        int8_t wheel_h = 0, wheel_v = 0;
        if (scroll_mode != SCROLL_YONLY) {
            while (rem_x >=  SCROLL_UNIT) { wheel_h++; rem_x -= SCROLL_UNIT; }
            while (rem_x <= -SCROLL_UNIT) { wheel_h--; rem_x += SCROLL_UNIT; }
        }
        if (scroll_mode != SCROLL_XONLY) {
            while (rem_y >=  SCROLL_UNIT) { wheel_v--; rem_y -= SCROLL_UNIT; } // Y反転
            while (rem_y <= -SCROLL_UNIT) { wheel_v++; rem_y += SCROLL_UNIT; }
        }

        /* ③ HID レポートを生成 */
        m.x = m.y = 0;   // ポインタ移動は無効
        m.h = wheel_h;
        m.v = wheel_v;
    }
    return m;
}

