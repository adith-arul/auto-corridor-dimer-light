/*
 * GccApplication1.c
 *
 * Created: 10/2/2026 11:13:00 AM
 * Author : adith
 */ 




// 1. Write DDRA as input  ( AND mask(0x40) ) 
// 2. Write 0x00 for PORTA
//   Write 0x00 to PORTB 


// 3. Set threshold value as constant 
// 4. Compare ADC6 and threhold value 

	// 4.1 If value > threshold, DAYTIME (BOOL status = FALSE)
	// 4.2 value < threshold, NIGHTTIME  (BOOL status  = TRUE) 
	
// 5. CHECK status , if true
	// Start Polling PWM cycle till 100% 
	//  HOld for 2 seconds 
	// Decrement PWM cycle till 0


/*

	while (1) {
		
		if key A is pressed
			update threshold 
			
		if # is pressed 
			{
				if (daynight())
					motion();
			}
	}
				
		
		
	}


*/
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#define  HYST 30 
#define F_CPU 12000000UL

#include <util/delay.h>        

#define KEY_NONE  0xFF
#define KEY_HASH  0x0F       // '#'
#define KEY_A     0x0A       // 'A'
#define HOLD_TICKS 1000
#define FADE_MAX   51
#define FADE_TICKS 4
#define A_HOLD_TICKS 200

const uint16_t DEFAULT_THRESHOLD PROGMEM = 512;  // Default Threshold value stored only in FLASH
uint16_t hystLow, hystHigh;

uint8_t isDark = 0;  // 1: 

uint16_t threshold;   // Running copy of threshold

uint16_t savedThreshold EEMEM;

typedef enum { OFF, FADE_UP, HOLD, FADE_DOWN } state_t;
state_t state = OFF;
uint16_t holdCount = 0;
uint16_t motionCount = 0;
uint8_t level = 0;
uint8_t fadeCount = 0;
uint16_t aCount = 0;
uint8_t aDone = 0;

const uint8_t colMask[4] = { 0xEF, 0xDF, 0xBF, 0x7F };

const uint8_t scanTable[256] = {
	[0x00 ... 0xFF] = 0xFF,
	[0xEE] = 0x01, [0xDE] = 0x02, [0xBE] = 0x03, [0x7E] = 0x0A,
	[0xED] = 0x04, [0xDD] = 0x05, [0xBD] = 0x06, [0x7D] = 0x0B,
	[0xEB] = 0x07, [0xDB] = 0x08, [0xBB] = 0x09, [0x7B] = 0x0C,
	[0xE7] = 0x0E, [0xD7] = 0x00, [0xB7] = 0x0F, [0x77] = 0x0D
};

void init(){
	
	DDRA &= ~(1 << PORTA6);			// A6 as input, leave other bits untouched
	PORTA &= ~(1 << PORTA6);	// disable pull up in A6, since A6 is LDR, leave other bits untouched
	ADMUX = (1 << REFS0) | 6; // Select reference voltage (REFS0 == aVCC), 6 is ADC channel (last 0:4 bits to choose ADC channel)
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1);  // /64 prescaling, voltage needs time to get to full value, thus require prescaling

	DDRC = 0xF0;
	PORTC = 0xFF;

	DDRB |= (1 << PB3);
	TCCR0 = (1 << WGM01) | (1 << WGM00) | (1 << CS01);
	OCR0 = 0;
}

uint16_t read_adc()
{
	ADCSRA |= (1 << ADSC);          // start a conversion
	while (ADCSRA & (1 << ADSC));   // wait till conversion is done.
	return ADC;
}


// Function: add_deadzone(): adds a deadzone range where ADC value does not change isDark value
// based on HYST value defined. 
// Input: Threshold value 
// OUTPUT: Active Day/Night Range 

void add_deadzone()
{
	if (threshold > HYST)
	hystLow = threshold - HYST;
	else
	hystLow = 0;            // if below 0, stop at 0

	if (threshold < 1023 - HYST)
	hystHigh = threshold + HYST;  // add values to hy
	else
	hystHigh = 1023;        // if above 1023, stop at 1023
}

// Function: update_daylight:- updates isDark based on Active Day/Night range
// Input: none (must call add_deadzone() before) 
// Output: Updates isDark 

void update_daylight(void)
{
	uint16_t a = read_adc();
	
	if (a > hystHigh)
		isDark = 1;
	else if (a < hystLow)
		isDark = 0;
	
	// else Keep isDark value
	
	
}

uint8_t ReadKP(void)
{
	for (uint8_t col = 0; col < 4; col++)
	{
		PORTC = colMask[col];            // drive one column low
		_delay_us(10);                   // let the rows settle
		uint8_t reading = PINC;
		PORTC = 0xFF;

		if (reading != colMask[col])     // a row went low: key pressed
		return reading;
	}
	return 0xFF;                         // nothing pressed
}

uint8_t LookupKey(uint8_t scanCode)
{
	return scanTable[scanCode];
}

void wait_tick(void)
{
	TCNT2 = 139;
	TCCR2 = (1 << CS22) | (1 << CS21) | (1 << CS20);
	while (!(TIFR & (1 << TOV2)));
	TCCR2 = 0x00;
	TIFR = (1 << TOV2);
}

void motion_event(void)
{
	if (isDark)
	{
		if (state == OFF || state == FADE_DOWN)
			state = FADE_UP;
		else if (state == HOLD)
			holdCount = HOLD_TICKS;
	}
	else
	{
		motionCount++;
	}
}

void set_level(uint8_t l)
{
	uint8_t duty = (uint32_t)l * l * 255 / (FADE_MAX * FADE_MAX);

	if (duty == 0)
	{
		TCCR0 &= ~(1 << COM01);
		PORTB &= ~(1 << PB3);
	}
	else
	{
		TCCR0 |= (1 << COM01);
		OCR0 = duty;
	}
}

void fade_up(void)
{
	fadeCount++;
	if (fadeCount >= FADE_TICKS)
	{
		fadeCount = 0;
		if (level < FADE_MAX)
			level++;
		set_level(level);

		if (level >= FADE_MAX)
		{
			state = HOLD;
			holdCount = HOLD_TICKS;
		}
	}
}

void hold(void)
{
	if (holdCount > 0)
		holdCount--;

	if (holdCount == 0)
	{
		state = FADE_DOWN;
		fadeCount = 0;
	}
}

void fade_down(void)
{
	fadeCount++;
	if (fadeCount >= FADE_TICKS)
	{
		fadeCount = 0;
		if (level > 0)
			level--;
		set_level(level);

		if (level == 0)
			state = OFF;
	}
}

void calibrate(void)
{
	uint16_t sum = 0;

	for (uint8_t i = 0; i < 8; i++)
		sum += read_adc();

	threshold = sum >> 3;

	if (threshold < HYST + 1)
		threshold = HYST + 1;
	if (threshold > 1022 - HYST)
		threshold = 1022 - HYST;

	add_deadzone();
	eeprom_update_word(&savedThreshold, threshold);
}

void check_calibration(uint8_t key)
{
	if (key == KEY_A)
	{
		if (!aDone)
		{
			aCount++;
			if (aCount >= A_HOLD_TICKS)
			{
				calibrate();
				aDone = 1;
			}
		}
	}
	else
	{
		aCount = 0;
		aDone = 0;
	}
}


int main(void)
{
	init();
	threshold = pgm_read_word(&DEFAULT_THRESHOLD);  // get value from FLASh and store in SRAM variable threshold

	uint16_t saved = eeprom_read_word(&savedThreshold);
	if (saved >= HYST + 1 && saved <= 1022 - HYST)
		threshold = saved;
	
	add_deadzone();                 // add Hystersis to threshold
	uint8_t prevKey = KEY_NONE;
	
	while (1)
	{
		wait_tick();
		update_daylight();
		
		uint8_t key = LookupKey(ReadKP());

		if (key == KEY_HASH && prevKey != KEY_HASH)
		motion_event();                  // only on the press, not while held

		prevKey = key;
		check_calibration(key);


		if (state == FADE_UP)
			fade_up();
		else if (state == HOLD)
			hold();
		else if (state == FADE_DOWN)
			fade_down();
	}
}