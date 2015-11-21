#ifndef SIDELIGHT2_H_
#define SIDELIGHT2_H_
// number of WS2812 to control
#define LED_COUNT 5

// maxmimum brightness, keep logarithmic scale in
// mind, 240 and 255 don't make a big difference
// except in current consumption.
#define MAX_BRIGHTNESS 200

// minimum brightness
#define MIN_BRIGHTNESS 32

// bit mask per sample generating fuzzyness
#define NOISE_AMOUNT 15
#define BATT_THRESHOLD 810

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "light_ws2812.h"
#include "ws2812_config.h"

uint8_t step;

#endif //SIDELIGHT2_H_

