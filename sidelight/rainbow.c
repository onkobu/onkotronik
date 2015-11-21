#include <util/delay.h>
#include "sidelight2.h"
uint8_t effect(struct cRGB *ledarray, uint8_t number_of_leds, uint8_t brig) {
	switch (step) {
		case 0:	// orange
			ledarray[0].r=brig;
			ledarray[0].g=(brig<<2)/10;
			ledarray[0].b=0; 
			break;
		case 1:	// orange
			ledarray[0].g=brig;
			ledarray[0].b=(brig<<2)/10;
			ledarray[0].r=0; 
			break;
		case 2:	// orange
			ledarray[0].b=brig;
			ledarray[0].r=(brig<<2)/10;
			ledarray[0].g=0; 
			break;
		case 3:	// orange
			ledarray[0].b=brig;
			ledarray[0].r=(brig<<2)/10;
			ledarray[0].g=0; 
			break;
		case 4:	// orange
			ledarray[0].g=brig;
			ledarray[0].r=(brig<<2)/10;
			ledarray[0].b=0; 
			break;
		case 5:	// orange
			ledarray[0].b=brig;
			ledarray[0].g=(brig<<2)/10;
			ledarray[0].r=0; 
			break;
	}

	for(uint8_t i=0;i<LED_COUNT;i++) {
		ws2812_sendarray((uint8_t *)&ledarray[0],3);
	}
	_delay_ms(50);
	uint8_t k=rand()&7;
	for(uint8_t i=0;i<k;i++) {
		_delay_ms(10);
	}
	if (brig<MIN_BRIGHTNESS) {
		// Can be called multiple steps in a sequence,
		// thus fast changing through all the colors.
		// I prefer this randomness instead of counting
		// up brig, too to stay on the safe side
		step=(step+1)%6;
	}
	// Some alternating in adding and subtracting
	// with current limiting (max. 200) and minimal
	// brightness (>32)
	if ((rand() & 1 && brig < MAX_BRIGHTNESS) || brig<MIN_BRIGHTNESS) {
		brig=brig+(rand() & 15);
	} else {
		brig=brig-(rand() & 15);
	}
	return brig;
}
