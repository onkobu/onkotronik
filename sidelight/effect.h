#ifndef EFFECT_H_
#define EFFECT_H_


/**
  * Definition of a single effect step.
  *
  * ledarray - the LED array definition, always size 1
  * number_of_leds - repetition of RGB struct, all WS2812
  *   ought to have same RGB values
  * brig - current brightness
  *
  * return the new brightness calculated by the effect
  */
uint8_t effect(struct cRGB *ledarray, uint8_t number_of_leds, uint8_t brig);

#endif //EFFECT_H
