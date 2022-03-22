/*
 * Project_1.c
 *
 * Created: 13/03/2022 13:15
 * Author : Marine Camba
 * Module : EE4524 Digital System 3
 * Brief : interrupt driven program, display ADC indicator
 * or cylon eye patterns on LED display depending
 * on push buttons pressed and the value of a potentiometer
 * (both on chip)
 *
 */ 

#include <avr/io.h>
#include <avr/iom328p.h>
#include <avr/interrupt.h>

#define ONE_EIGHTH_VOLTAGE 128
#define ONE_QUARTER_VOLTAGE 256
#define THREE_EIGHTH_VOLTAGE 384
#define HALF_VOLTAGE 512
#define FIVE_EIGHTH_VOLTAGE 640
#define THREE_QUARTER_VOLTAGE 768
#define SEVEN_EIGHTH_VOLTAGE 896

// Prototype function
void adc_indicator_display(bool output_shape);
void cylon_eye(bool output_shape);

// Global variables
uint16_t timecount0;		// For overflow of counter 0 purpose
uint16_t adc_reading;		// Value read from potentiometer to be converted by ADC
uint8_t new_adc_data_flag;	// Flag when a new value arise
uint8_t delay;				// Timer value
uint8_t full_scale = 5;	// Full scale = 5V	

int main(void)
{
	new_adc_data_flag = 0;
	
	// All bits have a reset value of 0, if want to be sure all bits are disabled except the one we use use binary form to force bits at 0 (0b...)
	// PORT B - switch inputs
	DDRB = (0<< PORTB4)|(0<<PORTB5);		//PORTB PINB 4 and 5 as inputs
	PORTB = (1<<PINB4)|(1<<PINB5);			// as pull-ups
	
	// PORT D - LED output
	DDRD = 0xFF;							// PORTD all outputs
    PORTD = 0x0;							// Initialize to all off, not mandatory as reset default state =0
		
	// TIMER0
	TCCR0A = 0x00;							// All outputs disabled, can not write this line as default value = 0, here for readability
	TCCR0B = (5<<CS00);						//Or =(1<<CA02)|(1<<CS00) 
	TIMSK0 = (1<<TOIE0);					// Overflow interrupt
	// Start value to add ? TCNT0
	
	// ADC
	ADMUX = (1<<REFS0);						// All other bits = 0 
	ADCSRA = (1<<ADEN)|(1<<ADSC)|(1<<ADATE)|(1<<ADIE)|(7<<ADPS0);		//Or (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)
	ADCSRB = 0x00;							// Not mandatory as default at reset =0, here for readability as in Free Running mode and ACME=0	
		
	sei();									// Global interrupt enable 
		
    while(1)
		{
			if (new_adc_data_flag)
			{
				if (PORTB5)
				{
					if (PORTB4)			// Cylon pattern
					{
						cylon_eye(PORTB5);
					}
					else				// ADC indicator display
					{
						adc_indicator_display(PORTB5);
					}
				}
				else //	PORTB5==0, ADC indicator bit 3-0, Cylon pattern bit 7-4
				{
					adc_indicator_display(PORTB5);
					cylon_eye(PORTB5);
				}
				new_adc_data_flag = 0;
			}
		}			
}

void adc_indicator_display(bool output_shape){
	// Function to display in thermometer mode from indicator value
	// To change the value = change the potentiometer values
	// Method : calculi right comparison side of input voltage range, name if Vin
	//			x (value read by ADC) = Vin*1023/5 with 1023 = 2^10-1 with 10 number of bits of ADC and 5 = Vref
	//			Work with integer to round x to smaller value (x=127.9 becomes x=127)
	
	if (output_shape)
	{
		if (adc_reading>0 && adc_reading<ONE_EIGHTH_VOLTAGE)
		{
			PORTD=0b00000000;
		}
		if (adc_reading>=ONE_EIGHTH_VOLTAGE && adc_reading<ONE_QUARTER_VOLTAGE)
		{
			PORTD=0b00000001;
		}
		if (adc_reading>=ONE_QUARTER_VOLTAGE && adc_reading<THREE_EIGHTH_VOLTAGE)
		{
			PORTD=0b00000011;
		}
		if (adc_reading>=THREE_EIGHTH_VOLTAGE && adc_reading<HALF_VOLTAGE)
		{
			PORTD=0b00000111;
		}
		if (adc_reading>=HALF_VOLTAGE && adc_reading<FIVE_EIGHTH_VOLTAGE)
		{
			PORTD=0b00001111;
		}
		if (adc_reading>=FIVE_EIGHTH_VOLTAGE && adc_reading<THREE_QUARTER_VOLTAGE)
		{
			PORTD=0b00011111;
		}
		if (adc_reading>=THREE_QUARTER_VOLTAGE && adc_reading<SEVEN_EIGHTH_VOLTAGE)
		{
			PORTD=0b00111111;
		}
		if (adc_reading>=SEVEN_EIGHTH_VOLTAGE && adc_reading<1023)
		{
			PORTD=0b01111111;
		}
		if (adc_reading>=1023)
		{
			PORTD=0b11111111;
		}
	}
	else 
	{
		if (adc_reading>0 && adc_reading<ONE_QUARTER_VOLTAGE)
		{
			PORTD |= 0b00000000;
		}
		if (adc_reading>=ONE_QUARTER_VOLTAGE && adc_reading<HALF_VOLTAGE)
		{
			PORTD |= 0b00000001;
		}
		if (adc_reading>=HALF_VOLTAGE && adc_reading<THREE_QUARTER_VOLTAGE)
		{
			PORTD |= 0b00000011;
		}
		if (adc_reading>=THREE_QUARTER_VOLTAGE && adc_reading<1023)
		{
			PORTD |= 0b00000111;
		}
		if (adc_reading>=1023)
		{
			PORTD |= 0b00001111;
		}
	}
	
}
void cylon_eye(bool output_shape)
{
	if (output_shape)
	{
		// Full cylon pattern
		for (i=1; i < 128; i = i*2)
		{
			PORTD = i;
			_delay_loop_2(delay);		//!!!VALEUR DE L'ADC
		}
		for (i=128; i > 1; i = i/2)
		{
			PORTD = i;
			_delay_loop_2(delay);		//!!!VALEUR DE L'ADC
		}
	}
	else 
	{
		// Cylon on bits 4 to 7
		// Bit 7 corresponds to 128 so bit 4 corresponds to 16 (divide by 2 each time)
		for (i=16; i < 128; i = i*2)
		{
			PORTD = i;
			_delay_loop_2(delay);		//!!!VALEUR DE L'ADC
		}
		for (i=128; i > 16; i = i/2)
		{
			PORTD = i;
			_delay_loop_2(delay);		//!!!VALEUR DE L'ADC
		}
	}
}

ISR(TIMER0_OVF_vect)
{
	// !!!!! Classic counter, values to change regarding 1sec and 0.25 sec
	/*TCNT0 = 61;		//TCNT0 needs to be set to the start point each time
	++timecount0;	// count the number of times the interrupt has been reached
	
	if (timecount0 >= 40)	// 40 * 12.5ms = 500ms = 0.5s
	{
		PORTD = ~PORTD;		// Toggle all the bits
		timecount0 = 0;		// Restart the overflow counter
	}
	timecount0 = 0;		// Restart the overflow counter	
	*/
}

ISR(ADC_vect)
{
	adc_reading = ADC;	// Read value from ADC
	
	// !!! Need to calculate timer values and overflow etc
	if (adc_reading>=0 && adc_reading<=HALF_VOLTAGE)
	{
		// Input voltage inf 2.5V, on/off time = 0.25sec
		delay = 9999; //!!!!!!!! to change regarding preset values and time count
	}
	else
	{
		// on/off time = 1sec
		delay = 9999; //!!!!!!!! to change regarding preset values and time count
	}	
	new_adc_data_flag = 1;	// Can handle a new ADC value
}
