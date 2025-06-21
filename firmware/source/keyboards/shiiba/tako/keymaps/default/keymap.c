#include QMK_KEYBOARD_H
#include "pointing_device.h"

enum custom_keycodes {
    DRAG_SCROLL = SAFE_RANGE,
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT_direct(
        KC_BTN1,        // GP14 - 左クリック
        KC_BTN2,        // GP15 - 右クリック
        KC_BTN3,        // GP26 - 中クリック
        KC_BTN4,        // GP27 - 戻る
        KC_BTN5,        // GP28 - 進む
        DRAG_SCROLL     // GP29 - ドラッグスクロール
    ),
};

// ドラッグスクロール状態管理
static bool drag_scroll_enabled     = false;
static bool cpi_low                 = false;    // false=デフォルト, true=低感度
static bool primary_clicked_in_drag = false;

static const uint16_t CPI_DEFAULT = PMW33XX_CPI; // config.h で定義されている
static const uint16_t CPI_ALT     = CPI_DEFAULT / 3;

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
	/* --- Drag-Scroll ボタン ----------------------------------------- */ 
        case DRAG_SCROLL:
            if (record->event.pressed) {
                // GP29が押されたときドラッグスクロールを有効化
                drag_scroll_enabled = true;
		primary_clicked_in_drag = false;        // 毎回リセット
                pointing_device_set_cpi(100);           // スクロール用に低いCPIに変更
            } else { /* DRAG_SCROLL released */
                // GP29が離されたときドラッグスクロールを無効化
                drag_scroll_enabled = false;

		/* ホールド中に主ボタンが押されていたら CPI をトグル */
                if (primary_clicked_in_drag) {
                    cpi_low = !cpi_low;
                }
                pointing_device_set_cpi(cpi_low ? CPI_ALT : CPI_DEFAULT);
            }
            return false;       // イベントを他へ流さない
        
        /* --- 主クリック -------------------------------------------------- */
        case KC_BTN1:
            if (drag_scroll_enabled) {
                /* ホールド中はクリックを OS に送らずフラグだけ */
                if (record->event.pressed) {
                    primary_clicked_in_drag = true;         // 予約フラグを立てる
                }
                return false;   // ここでストップ
            }
            return true;        // 通常時はそのまま送る
    }
    return true;
}

// ポインティングデバイスの動作をカスタマイズ
report_mouse_t pointing_device_task_user(report_mouse_t m) {
    if (drag_scroll_enabled) {
        // ドラッグスクロールモード時は、X/Y移動をスクロールに変換
        m.h = m.x;      // 水平スクロール
        m.v = -m.y;     // 垂直スクロール
        m.x = 0;        // マウス移動を無効化
        m.y = 0;
    }
    return m;
}
