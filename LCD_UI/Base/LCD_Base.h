//===================================================================
// Basic LCD functions (4bit mode).         =
//                                                                  =
// Used MCU : Atmel AT89C4051 ( 128 Byte RAM, 4 KByte EPROM,        =
//                             2 Timer, UART, @ 24 MHz )            =
//                                                                  =
//===================================================================

//===================================================================
// Port-map of the MCU :                                            =
//                                                                  =
// P1_0 -> backilght control                                        =
// P1_1 -> LCD R/W                                                  =
// P1_2 -> LCD RS                                                   =
// P1_3 -> LCD E                                                    =
// P1_4 -> LCD D4                                                   =
// P1_5 -> LCD D5                                                   =
// P1_6 -> LCD D6                                                   =
// P1_7 -> LCD D7                                                   =
//                                                                  =
// P3_0 -> [RXD]  unused                                            =
// P3_1 -> [TXD]  unused                                            =
// P3_2 -> [INT0] unused                                            =
// P3_3 -> [INT1] unused                                            =
// P3_4 -> [T0]   unused                                            =
// P3_5 -> [T1]   unused                                            =
// P3_6 -> [WR]   unused                                            =
// P3_7 -> unused                                                   =
//                                                                  =
//===================================================================


#ifndef LCD_BASE_H_
#define LCD_BASE_H_

#include <at89x051.h> 
#include "Delay.h"


// DEFINES ==========================================================
#define LCD_DATA  P1      // Dataport of LCD-Display (D4..D7) 
#define LCD_RW    P1_1    // Read-Write select of LCD
#define LCD_RS    P1_2    // Register Select of LCD-Display
#define LCD_E     P1_3    // Enable of LCD-Display
#define CTRL_REG  0       // Select instruction register
#define DATA_REG  1       // Select data register
#define BLINK     0x01    // Alias for blinking cursor
#define NOBLINK   0x00    // Alias for non blinking cursor
#define SHOW      0x02    // Alias for cursor on
#define HIDE      0x00    // Alias for cursor off
#define ON        0x04    // Alias for display on
#define OFF       0x00    // Alias for display off
#define READ      1       // Alias for read
#define WRITE     0       // Alias for write

// The display rows
#define DISPLAY_TOP_ROW 1
#define DISPLAY_BOTTOM_ROW 2

// PROTOTYPES ========================================================
// Clears LCD screen
void clrscr(void);

// Writes a character to display
void LCD_putchar(char value);

// Prints a text to x/y position
void LCD_printxy(unsigned char x,unsigned char y, unsigned char *text);

// Controls the display
void control_LCD(unsigned char dsp,unsigned char blink,unsigned char cursor);

// Sets LCD write position
void gotoxy(unsigned char x,unsigned char y);

// Sets up CG_RAM_DATA
void setup_cgram(unsigned char* cgram_data);

#endif /*LCD_BASE_H_*/
