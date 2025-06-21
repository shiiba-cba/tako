#pragma once

/* Direct pin configuration */
#define DIRECT_PINS { \
    { GP14, GP27, GP15, GP26, GP29, GP28 } \
}

/* Debounce reduces chatter (unintended double-presses) - set 0 if debouncing is not needed */
#define DEBOUNCE 5

/* SPI configuration for PMW3360 */
#define SPI_DRIVER SPID0
#define SPI_SCK_PIN GP2
#define SPI_MOSI_PIN GP3
#define SPI_MISO_PIN GP4
#define PMW33XX_CS_PIN GP5

/* PMW3360 settings */
#define PMW33XX_CPI 600
#define POINTING_DEVICE_INVERT_Y

/* Enable drag scroll feature */
#define POINTING_DEVICE_GESTURES_SCROLL_ENABLE
