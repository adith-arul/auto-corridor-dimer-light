/*
 * GccApplication1.c
 *
 * Created: 10/2/2026 11:13:00 AM
 * Author : adith
 */ 

#include <avr/io.h>


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


const uint16_t DEFAULT_THRESHOLD PROGMEM = 512;  // Default Threshold value stored only in FLASH


uint16_t threshold;   // Running copy of threshold

void init(){
	
	DDRA &= ~(1 << PORTA6);			// A6 as input, leave other bits untouched
	PORTA &= ~(1 << PORTA6);	// disable pull up in A6, since A6 is LDR, leave other bits untouched
	ADMUX = (1 << REFS0) | 6; // Select reference voltage (REFS0 == aVCC), 6 is ADC channel (last 0:4 bits to choose ADC channel)
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1);  // /64 prescaling, voltage needs time to get to full value, thus require prescaling
}

uint16_t read_adc(void)
{
	ADCSRA |= (1 << ADSC);          // start a conversion
	while (ADCSRA & (1 << ADSC));   // wait till conversion is done.
	return ADC;
}

int main(void)
{
	init();
	threshold = pgm_read_word(&DEFAULT_THRESHOLD);  // get value from FLASh and store in SRAM variable threshold
	
	while (1)
	{
	}
}


