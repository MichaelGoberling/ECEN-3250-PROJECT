/*
 * Comm_Systems_Final_Project.c
 *
 * Created : 1/17/2018 2:15:51 PM
 * Authors : Max Hoobler, Michael Goberling, Will Picken, James White
 * Project : Communication Systems Final Project 
 */ 

#include "capi324v221.h"
#define FOSC 20000000
#define BAUD 9600

//===================== Prototypes =====================
void init_adc( void );
void sample_adc( void );
void init_USART( int UBRR );
void tx_USART( unsigned char d );
unsigned char rx_USART( void );

//===================== defines =====================
ADC_SAMPLE sample;
double temp;
unsigned int MYUBRR;
unsigned int temp2;
unsigned int transmit_temp;
unsigned int remote_temp;
unsigned int adc_array[10];
unsigned int degree = 223;


//===================== Main =====================
void CBOT_main( void )
{
	//Free up CEENBoT resources for use
	LCD_open(); 
	STEPPER_open(); 
	ADC_open();
	
	//Initialize ADC subsystem to Channel 3 and set ref voltage & USART subsystem
	init_adc();
	
	init_USART( 129 );
	
	STEPPER_run(STEPPER_BOTH, STEPPER_FWD, 150);
			
	
	while(1)
	{
	
		sample_adc();
		
		tx_USART( transmit_temp );
		remote_temp = rx_USART();
		
		LCD_printf_RC(3, 0, "This Temp(C): %d\n" , transmit_temp);
		LCD_printf_RC(2, 0, "Other Temp(C): %d\n", remote_temp);
		
		TMRSRVC_delay( 50 );
	}
} 

void init_adc()
{
	ADC_set_VREF(ADC_VREF_AVCC);
	ADC_set_channel(ADC_CHAN4);
}

void sample_adc()
{	
	unsigned int sum = 0;

	//Fill an array with 10 values
	for(int i = 0; i < 9; i++)
	{
		TMRSRVC_delay( 30 );
		sample = ADC_sample();
		temp = sample * (5.0 / 1024);
	
		//TMP36 does 750mV at 25 celsius
		//Subtract 500mV to provide linear readings
		//TMP36 scale = 10mV/degree celsius
		transmit_temp = (int)(100*(temp - 0.5));
		adc_array[i] = transmit_temp;
	}

			//Sum that array up
	for(int i = 0; i < 9; i++)
	{
		sum += adc_array[i];
	}

	//That's the new value
	transmit_temp = (sum/10);
	
}

void init_USART( int UBRR )
{
	UBRR1H = (unsigned char)(UBRR >> 8); //set high usart1
	UBRR1L = (unsigned char)UBRR;  //set low usart1
	UCSR1B = (1 <<RXEN0) | (1 << TXEN0); //enabled rx and tx modules for usart1
	
	//Frame format
	//sets 8 bit
	UCSR1C = (1 << USBS1) | (3 << UCSZ01); 
	//set 2 stop bit
	//UCSR1C |= (3 << UCSZ01);
}

void tx_USART( unsigned char d )
{	

	static BOOL timer_started = FALSE;
				
	static TIMEROBJ sense_timer;
				
	if ( timer_started == FALSE )
	{
			
		TMRSRVC_new( &sense_timer, TMRFLG_NOTIFY_FLAG, TMRTCM_RESTART,
					500 );
					
		timer_started = TRUE;
					
	} // end if()
				
	// Otherwise, just do the usual thing and just 'sense'.
	else
	{

		if ( TIMER_ALARM( sense_timer) )
		{

			LED_toggle( LED_Green );
						
			while(!(UCSR1A & (1 << UDRE1)));	//wait until the transmit buffer is empty
			UDR1 = d; //then place the data in the transmit buffer

			TIMER_SNOOZE( sense_timer );
						
		} // end if()

	} // end else.
				
}

unsigned char rx_USART( void )
{ 

	static BOOL timer_started = FALSE;
					
	static TIMEROBJ sense_timer_2;
					
	if ( timer_started == FALSE )
	{
						
		TMRSRVC_new( &sense_timer_2, TMRFLG_NOTIFY_FLAG, TMRTCM_RESTART,
		500 );
						
		timer_started = TRUE;
						
	} // end if()
					
	else
	{

		if ( TIMER_ALARM( sense_timer_2) )
		{

			LED_toggle( LED_Green );
							
			while(!(UCSR1A & (1 << RXC1)));
			return UDR1;
							
			TIMER_SNOOZE( sense_timer_2 );
							
		} // end if()

	} // end else.
}
