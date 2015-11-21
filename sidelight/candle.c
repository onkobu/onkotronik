#include <util/delay.h>
#include "sidelight2.h"

uint8_t effect(struct cRGB *ledarray, uint8_t number_of_leds, uint8_t brig) {
	ledarray[0].r=brig;
	ledarray[0].g=brig>>1;
	ledarray[0].b=brig>>4; 

	for(uint8_t i=0;i<number_of_leds;i++) {
		ws2812_sendarray((uint8_t *)&ledarray[0],3);
	}
	_delay_ms(50);
	uint8_t k=rand()&7;
	for(uint8_t i=0;i<k;i++) {
		_delay_ms(10);
	}
	if ((rand() & 1 && 
			brig < MAX_BRIGHTNESS) || 
			brig < MIN_BRIGHTNESS) {
		brig=brig+(rand() & NOISE_AMOUNT);
	} else {
		brig=brig-(rand() & NOISE_AMOUNT);
	}
	return brig;
}
