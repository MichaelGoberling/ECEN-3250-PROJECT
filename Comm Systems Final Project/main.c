/*
 * Comm_Systems_Final_Project.c
 *
 * Created: 1/17/2018 2:15:51 PM
 * Author : Michael Goberling
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
	
	//ubrr value = (((fosc/(16*baudrate)) - 1)
	//(20,000,000/(16*9,600)) - 1 = 129.2
	
	//MYUBRR = (FOSC/(16*BAUD) - 1);
	//LCD_printf("%d", MYUBRR);
	
	init_USART( 129 );
	
	while(1)
	{
		//STEPPER_move_rn(STEPPER_BOTH, 
		//	STEPPER_FWD, 200, 400, 
		//	STEPPER_FWD, 100, 400);
			
		sample_adc();
		
		LCD_printf_RC(3, 0, "This Temp(C): %d\n" , transmit_temp);
		LCD_printf_RC(2, 0, "Other Temp(C): %d\n", remote_temp);
		
		tx_USART( transmit_temp );
		//remote_temp = rx_USART();
		
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
		TMRSRVC_delay( 50 );
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
	LCD_printf_RC(1, 0, "Status: TX");
	while(!(UCSR1A & (1 << UDRE1)));	//wait until the transmit buffer is empty
	LCD_clear();
	UDR1 = d; //then place the data in the transmit buffer
}

unsigned char rx_USART( void )
{
	LCD_printf_RC(1, 0, "Status: RX");
	while(!(UCSR1A & (1 << RXC1)));	//wait until data has been received
	LCD_clear();
	return UDR1; //return that data
}


