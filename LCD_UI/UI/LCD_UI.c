#include "LCD_UI.h"
// Define our Host_ID
#include "Host_ID.h"


// Globals ==========================================================

// CG_RAM Data á,é,ó,ö,õ,í,ú,ü -- note û is missing.
__code const unsigned char CGRAM_DATA[64] = {02,04,14,01,15,17,15,00,// á
                                        02,04,14,17,31,16,14,00,// é
                                        02,04,00,14,17,17,14,00,// ó
                                        10,00,14,17,17,17,14,00,// ö
                                        05,10,00,14,17,17,14,00,// õ
                                        02,04,00,12,04,04,14,00,// í
                                        02,04,17,17,17,19,13,00,// ú
                                        10,00,17,17,17,19,13,00// ü
};

volatile unsigned int  DELAY;                   // Global delaytime
volatile unsigned char serial_received_char;
volatile __bit serial_char_needs_processing;
volatile __bit serial_send_complete;

struct temperatures_struct temps;

struct buffer_struct comm_buffer;

unsigned char state;
unsigned char display_mode;

// Buffer for assembling display content
unsigned char display_line_buffer[16];

__code const unsigned char* NAPPALI_LABEL = "Nappali:";
__code const unsigned char* EXTERNAL_LABEL = "Külsõ:";

// END Globals ======================================================

/********************************************************************/
/* Function    : Timer0_ISR()                                   ISR */
/*------------------------------------------------------------------*/
/* Description : Interrupt-Service-Routine on vector #1 for         */
/*               handling Timer0 interrupts. Register set #2 is     */
/*               used to store important registers.                 */
/*               The routine is called every 200s.                 */
/*------------------------------------------------------------------*/
/* Author      : Thorsten Godau  NT8                                */
/*------------------------------------------------------------------*/
/* Input       : none                                               */
/*------------------------------------------------------------------*/
/* Returnvalue : none                                               */
/*------------------------------------------------------------------*/
/* History     : 06/99    V1.0 Basic routine                        */
/*                                                                  */
/********************************************************************/
void Timer0_ISR(void) __interrupt 1 __using 0
{

if ( DELAY > 0 ) DELAY--;    // Timebase ticks for delay-routine

return;
}

void External_ISR(void)  __interrupt 0 __using 0
{

return;
}

void Serial_ISR(void)  __interrupt 4 __using 0
{
if(RI==1)
  {
    RI=0;
    serial_received_char=SBUF;
    serial_char_needs_processing = 1;
  }else if(TI==1){
    TI=0;
    serial_send_complete = 1;
  }
return;
}


/********************************************************************/
/* Function    : delay(delaytime)                               SUB */
/*------------------------------------------------------------------*/
/* Description : Routine waits (dependening on Timer 0) for about   */
/*               delaytime * 100s. Best for delaytime > 10         */
/*------------------------------------------------------------------*/
/* Author      : Thorsten Godau  NT8                                */
/*------------------------------------------------------------------*/
/* Input       : unsigned int time_100s                            */
/*------------------------------------------------------------------*/
/* Returnvalue : none                                               */
/*------------------------------------------------------------------*/
/* History     : 06/99    V1.0 Basic routine                        */
/*               09/99    V1.1 nop inserted in delay-loop           */
/*                                                                  */
/********************************************************************/
void delay(unsigned int time_100us)
{

if ( time_100us == 0 ) return;   // Return if no delaytime is assigned

DELAY = time_100us*2;              // Set global delaytime *2 is a Hack to vercome wrong oscillator speed
TL0  = 0x37;    // Start from 37
TH0  = 0x37;    // Tick every 0.1 msec (200 machine cycles)
TMOD = (TMOD&0xF0)|0x02;    // Autoreload for Timer 0
TR0  = 1;       // Start Timer 0
 
ET0  = 1;       // Enable Timer0 interrupt
while ( DELAY > 0 )
  {
__asm nop __endasm;
  }             // Wait delaytime
ET0  = 0;       // Disable Timer0 interrupt
TR0  = 0;       // Stop Timer 0
return;
}

/********************************************************************/
/* Function    : init_MCU()                                     SUB */
/*------------------------------------------------------------------*/
/* Description : Routine initializes the used MCU registers,        */
/*               the timers and the global variables.               */
/*               Baudrate calculation :                             */
/*               RCAP2 = 65536 - ( Quarz / 32 * Baud )              */
/*               Timer 0/1 autoreload-register THx calculation :    */
/*               THx = Timebase * Quarz / 12                        */
/*------------------------------------------------------------------*/
/* Author      : Thorsten Godau  NT8                                */
/*------------------------------------------------------------------*/
/* Input       : none                                               */
/*------------------------------------------------------------------*/
/* Returnvalue : none                                               */
/*------------------------------------------------------------------*/
/* History     : 06/99    V1.0 Basic routine                        */
/*                                                                  */
/********************************************************************/
void init_MCU(void)
{
EA   = 0;     // Disable all interrupts
ET0  = 0;     // Disable Timer 0 interrupt
ET1  = 0;     // Disable Timer 1 interrupt
EX0  = 0;     // Disable External 0 interrupt
EX1  = 0;     // Disable External 1 interrupt
ES   = 0;     // Disable Serial interrupt
TR0  = 0;     // Stop Timer 0
TR1  = 0;     // Stop Timer 1

// Initialize globals
DELAY = 0;

EA   = 1;       // Enable all interrupts

return;
}


/********************************************************************/
/* Function    : init_LCD()                                     SUB */
/*------------------------------------------------------------------*/
/* Description : Routine initializes the LCD display recommended    */
/*               in the datasheet (4 Bit initialization)            */
/*------------------------------------------------------------------*/
/* Author      : Thorsten Godau  NT8                                */
/*------------------------------------------------------------------*/
/* Input       : none                                               */
/*------------------------------------------------------------------*/
/* Returnvalue : none                                               */
/*------------------------------------------------------------------*/
/* History     : 06/99    V1.0 Basic routine                        */
/*               09/99    V1.1 Timing correction of init            */
/*                                                                  */
/********************************************************************/
void init_LCD(void)
{
delay(200);             // Wait 20ms
LCD_E  = 0;
LCD_RS = CTRL_REG;      // Switch to inruction register
LCD_RW = WRITE;         // Write to LCD

//Set LCD_DATA to high nibble of Software Reset
LCD_DATA = (LCD_DATA&0x0F)|0x20;
LCD_E = 1; LCD_E = 0;   // Write data to display
delay(50);              // Wait 5ms

LCD_E = 1; LCD_E = 0;   // Write data to display again (SW Reset)
delay(50);              // Wait 5ms

// Set LCD_DATA to high nibble of Function Set : 4 bit, 2 lines, 5*7 font
LCD_DATA = (LCD_DATA&0x0F)|0x80;
LCD_E = 1; LCD_E = 0;   // Write data to display
delay(100);               // Wait 400s

// Set LCD_DATA to high nibble of Display On/Off Control : display off, cursor off, dont blink
LCD_DATA = (LCD_DATA&0x0F)|0x00;
LCD_E = 1; LCD_E = 0;   // Write data to display
// Set LCD_DATA to lower nibble of Display On/Off Control : display off, cursor off, dont blink
LCD_DATA = (LCD_DATA&0x0F)|0xf0;
LCD_E = 1; LCD_E = 0;   // Write data to display
delay(4);               // Wait 400s

// Set LCD_DATA to high nibble of Clear Display
LCD_DATA = (LCD_DATA&0x0F)|0x00;
LCD_E = 1; LCD_E = 0;   // Write data to display
// Set LCD_DATA to lower nibble of Clear Display
LCD_DATA = (LCD_DATA&0x0F)|0x10;
LCD_E = 1; LCD_E = 0;   // Write data to display
delay(50);              // Wait 5ms

// Set LCD_DATA to high nibble of Entry Mode Set : increment DD-RAM address, move cursor
LCD_DATA = (LCD_DATA&0x0F)|0x00;
LCD_E = 1; LCD_E = 0;   // Write data to display
// Set LCD_DATA to lower nibble of Entry Mode Set : increment DD-RAM address, move cursor
LCD_DATA = (LCD_DATA&0x0F)|0x60;
LCD_E = 1; LCD_E = 0;   // Write data to display
delay(4);               // Wait 400s

// Setup the character generator ram of the LCD
setup_cgram(CGRAM_DATA);

return;
}


// Send a character to the UART
void UART_putchar(unsigned char value)
{
  serial_send_complete = 0;
  SBUF = value;
  do{
/* The serial interrupt handler will set the send_complete 
	flag so it is reasonable to wait for this.
	Optimizer will let us do this as the variable is declared as volatile */  
    }while(!serial_send_complete);
}

// Sends a string to the UART
void UART_puts(unsigned char *text)
{
do{
  UART_putchar(*text++);
}while(*text != MESSAGE_TERMINATOR);
}

void ack_command(unsigned char host_id,unsigned char seq,unsigned char command)
{
	unsigned char message[6];
  
	message[0]=host_id;
	message[1]=seq/100;
	message[2]=(seq%100)/10;
	message[3]=seq%10;
	message[4]=command;
	message[5]=MESSAGE_TERMINATOR;
	UART_puts(message);
}

void receive_command( void )
{
	unsigned char ch_received=0;
	__bit process_char;
	
	ES = 0; //Disable serial interrupt to make sure we are not interrupted
	process_char = serial_char_needs_processing;
	if(process_char==1) 
	{
		serial_char_needs_processing=0;
		ch_received = serial_received_char;
	}
	ES = 1; //Enable serial interrupt
	
	if(process_char==1)
	{
	  if(state == AWAITING_COMMAND)
	  {
	    if(ch_received == MESSAGE_HEAD)
	      {
			comm_buffer.index=0;
			comm_buffer.content[0]= MESSAGE_TERMINATOR;
			state = RECEIVING_COMMAND;
	      }
	  }else{
		comm_buffer.content[comm_buffer.index]=ch_received;
		comm_buffer.index++;
	  if(ch_received == MESSAGE_TERMINATOR) {state = PROCESSING_COMMAND;}
	  }
	}
}

void process_command( void )
{
	unsigned char host_id=comm_buffer.content[0];
	unsigned char seq=comm_buffer.content[1]*100+comm_buffer.content[2]*10+comm_buffer.content[3];
	unsigned char command=comm_buffer.content[4];
	unsigned char *parameter=comm_buffer.content+5;

	int potential_temp_value;
	
	// Temp format: ABCD A:sign ('+' or '-'), BC: digits ('0'-'9'), D: decimal digit ('0'-'9')
	potential_temp_value = (parameter[1]-48)*100+(parameter[2]-48)*10+(parameter[3]-48);
	if(parameter[0]==(unsigned char)'-') {potential_temp_value = -1*potential_temp_value;}
	
	switch (command)
	  { 
	  case COMMUNICATE_LIVING_TEMP:
	    temps.living_temp = potential_temp_value;
	    ack_command(host_id,seq,ACK_COMMUNICATE_LIVING_TEMP);
	    break;
	  case COMMUNICATE_UPSTAIRS_TEMP:
	    temps.upstairs_temp = potential_temp_value;
	    ack_command(host_id,seq,ACK_COMMUNICATE_UPSTAIRS_TEMP);
	    break;
	  case COMMUNICATE_BASEMENT_TEMP:
	    temps.basement_temp = potential_temp_value;
	    ack_command(host_id,seq,ACK_COMMUNICATE_BASEMENT_TEMP);
	    break;
	  case COMMUNICATE_EXTERNAL_TEMP:
	    temps.external_temp = potential_temp_value;
	    ack_command(host_id,seq,ACK_COMMUNICATE_EXTERNAL_TEMP);
	    break;
	  case COMMUNICATE_HEATING_MODE:
	    temps.heating_mode = parameter[0];
	    ack_command(host_id,seq,ACK_COMMUNICATE_HEATING_MODE);
	    break;
	
	/*
	int target_living_temp;
	int target_basement_temp;
	int target_upstairs_temp;
	
	    
	#define ACK_COMMUNICATE_LIVING_TEMP 24
	#define ACK_COMMUNICATE_UPSTAIRS_TEMP 25
	#define ACK_COMMUNICATE_BASEMENT_TEMP 26
	#define ACK_COMMUNICATE_EXTERNAL_TEMP 27
	#define ACK_COMMUNICATE_HEATING_MODE 28
	*/
	
	  }
	
	//Reset state
	state = AWAITING_COMMAND;
}


void display_temp(unsigned char row_id,unsigned char* label,int temp)
{
// Line structure is as follows: "Label:   -99.9 C"
unsigned char i;
for(i=0;i<16;i++){display_line_buffer[i]=' ';}
i=0;
while(*label)
{
  display_line_buffer[i] = *(label++);
  i+=1;
}
// Add negative sign in case of negative temperature values
if(temp<0)
{
  display_line_buffer[9]='-';
  temp = -1 * temp;
}

if (temp>100)
{
  display_line_buffer[10]=temp/100+48;
  temp = temp % 100;
}else if (display_line_buffer[9] == (unsigned char)'-'){
  display_line_buffer[9]=' ';
  display_line_buffer[10]='-';
}

display_line_buffer[11]=temp/10+48;
display_line_buffer[12]='.';
display_line_buffer[13]=temp%10+48;
display_line_buffer[14]=' ';
display_line_buffer[15]='C';

LCD_printxy(1,row_id,display_line_buffer);
}


// MAIN =============================================================
void main( void )
{
int prev_living_temp;
int prev_external_temp;

init_MCU();                                            // Initialize MCU

init_LCD();                                            // Initialize the LCD display

control_LCD(ON,NOBLINK,HIDE);

// Setup the serial port operation mode
RI  = 0;TI  = 0;REN = 1;
SM2 = 0;SM1 = 1;SM0 = 0;

// Setup the serial port timer Timer1
TL1  = 0xfa;    // Start from 250
TH1  = 0xfa;    // Reload 250 4800 baud @ 10.0592 MHz Crystal
TMOD = (TMOD&0x0f)|0x20;    // Autoreload for Timer 1 hellpers
TR1  = 1;       // Start Timer 1

//Clear receive_buffer
comm_buffer.content[0] = MESSAGE_TERMINATOR;
comm_buffer.index = 0;
serial_char_needs_processing=0;

state=AWAITING_COMMAND; // Set the initial state
display_mode = DISPLAY_MODE_BASE; // Set the initial displaymode

temps.living_temp = 234;
temps.external_temp= 81;

prev_living_temp=temps.living_temp;
prev_external_temp=temps.external_temp;


ES   = 1;     // Enable Serial interrupt

while(1) // Main loop
{

// Do command processing
    switch(state)
    {
	    case AWAITING_COMMAND:
	      receive_command();
	      break;
	    case RECEIVING_COMMAND:
	      receive_command();
	      break;
	    case PROCESSING_COMMAND:
	      process_command();
	      break;
    }

// Do loop activities    
    switch(display_mode)
    {
    	case DISPLAY_MODE_BASE:
			if (temps.external_temp!=prev_external_temp || temps.living_temp!= prev_living_temp)
			{
			  display_temp(DISPLAY_TOP_ROW,NAPPALI_LABEL,temps.living_temp);
			  display_temp(DISPLAY_BOTTOM_ROW,EXTERNAL_LABEL,temps.external_temp);
			  prev_living_temp=temps.living_temp;
			  prev_external_temp=temps.external_temp;
			}
		break;
		case DISPLAY_MODE_EXTERNAL:
			display_temp(DISPLAY_TOP_ROW,NAPPALI_LABEL,temps.living_temp);
			display_temp(DISPLAY_BOTTOM_ROW,EXTERNAL_LABEL,temps.external_temp);
		break;
    }

  }

}
// END MAIN =========================================================
