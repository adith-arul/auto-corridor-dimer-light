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

int main(void)
{
    /* Replace with your application code */
    while (1)
    {
		// if bbuttton {
			
		}
		
		
    }
}

