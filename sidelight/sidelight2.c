// -----------------------------------------
// ws2812-Module an einem ATTiny45 kontrollieren
// Benutzt light_ws2812, siehe 
// https://github.com/cpldcpu/light_ws2812/
// -----------------------------------------

#include <util/delay.h>
#include "sidelight2.h"
#include "light_ws2812.h"
#include "effect.h"

struct cRGB led[1];
uint8_t brig=64;

// Init A/D-converter
void ADC_init(void) {
	// Intern 1.1V, PB2 In
	ADMUX |= ( 1 << REFS1) | ( 1 << MUX0);
	// enable, single conversion, 125kHz sample rate@16MHz
	ADCSRA |= ( 1 << ADEN) | ( 1 << ADSC) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
	while(ADCSRA & (1<<ADSC));
	(void) ADC;
	ADCSRA|=(1<<ADIF);
}

// Reads battery voltage once.
unsigned int read_voltage(void) {
	ADCSRA |= ( 1 << ADSC);
	while(ADCSRA & (1<<ADSC));
	unsigned int voltage=ADC;
	ADCSRA|=(1<<ADIF);
	return voltage;
}


int main (void)
{
#ifdef __AVR_ATtiny10__
	CCP=0xD8;	// configuration change protection, write signature
	CLKPSR=0;	// set cpu clock prescaler =1 (8Mhz) (attiny 4/5/9/10)
#else
	CLKPR=_BV(CLKPCE);
	CLKPR=0;	// set clock prescaler to 1 (attiny 25/45/85/24/44/84/13/13A)
#endif

	ADC_init();
	// nur notwendig, wenn sendarray direkt aufgerufen wird.
	// setleds ruft das implizit auf.
	DDRB|=_BV(ws2812_pin);

	uint8_t msr=0;
	uint8_t battOk=read_voltage() > BATT_THRESHOLD;
	while(battOk)
	{
		brig=effect(led, LED_COUNT, brig);
		msr++;
		// read battery voltage only if msr overflowed
		if (!msr) {
			battOk=read_voltage() > BATT_THRESHOLD;
		}
	}
	led[0].r=0;
	led[0].g=0;
	led[0].b=0; 

	for(uint8_t i=0;i<LED_COUNT;i++) {
		ws2812_sendarray((uint8_t *)&led[0],3);
	}
	// If battery is not ok there'll be only a single
	// LED flashing red forever
	while(1) {
		led[0].r=255;
		led[0].g=0;
		led[0].b=0; 
		ws2812_sendarray((uint8_t *)&led[0],3);
		_delay_ms(10);
		led[0].r=0;
		led[0].g=0;
		led[0].b=0; 
		ws2812_sendarray((uint8_t *)&led[0],3);
		_delay_ms(2000);
	}
}
