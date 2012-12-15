#ifndef LCD_UI_H_
#define LCD_UI_H_

#include <at89x051.h>     // Definitions of registers, SFRs and Bits
#include "LCD_Base.h"
#include "Commands.h"
#include "Furnace_modes.h"
#include "Delay.h"

// ISR-Prototypes ===================================================
void External_ISR(void)    __interrupt 0 __using 0; // ISR for debugging
void Timer0_ISR(void)    __interrupt 1 __using 0; // ISR for Timer0 overflow
void Serial_ISR(void)    __interrupt 4 __using 0; // ISR for Serial interrupts
// END ISR-Prototypes ===============================================


// FuncPrototypes =======================================================
// Initialize the LCD display
void init_LCD(void);

// Initialize the MCU
void init_MCU(void);

// Send a character via UART
void UART_putchar(unsigned char value);

// Sends a string to the UART
void UART_puts(unsigned char *text);

//Format and display a temperature value passed to its as temp*100
void display_temp(unsigned char row_id,unsigned char* label,int temp);

// Acknowledge a command
void ack_command(unsigned char host_id,unsigned char seq,unsigned char command);

// Receive characters
void receive_command( void );

// Process a received commands
void process_command( void );

// END FuncPrototypes ===================================================


// StructPrototypes =======================================================

struct temperatures_struct
{
	int	external_temp;
	int	living_temp;
	int	basement_temp;
	int	upstairs_temp;
	unsigned char 	heating_mode;
	
	int	target_living_temp;
	int	target_basement_temp;
	int	target_upstairs_temp;	
};

struct buffer_struct
{
	unsigned char 	content[15];
	unsigned char 	index;	
};

// END StructPrototypes ===================================================


#endif /*LCD_UI_H_*/
