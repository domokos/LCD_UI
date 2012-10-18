#include <at89x051.h>
#include "LCD_Base.h"

/********************************************************************/
/* Function    : control_LCD(dsp,blink,cursor)                  SUB */
/*------------------------------------------------------------------*/
/* Description : Routine controls the screen                        */
/*------------------------------------------------------------------*/
/* Author      : Thorsten Godau  NT8                                */
/*------------------------------------------------------------------*/
/* Input       : unsigned char dsp    = ON     -> Display on        */
/*                                      OFF    -> Display off       */
/*               unsigned char blink  = BLINK  -> Cursor blinks     */
/*                                      NOBLINK-> Cursor not blinks */
/*               unsigned char cursor = SHOW   -> Cursor visible    */
/*                                      HIDE   -> Cursor invisible  */
/*------------------------------------------------------------------*/
/* Returnvalue : none                                               */
/*------------------------------------------------------------------*/
/* History     : 06/99    V1.0 Basic routine                        */
/*               09/99    V1.1 Calculation of control modified      */
/*                             (because of compiler otim. bug)      */
/*                                                                  */
/********************************************************************/
void control_LCD(unsigned char dsp,unsigned char blink,unsigned char cursor)
{
unsigned char control;  // variable to generate instruction byte

control = (0x08 + blink + cursor + dsp); // Cursor control

EA = 0;                 // Disable all interrupts

LCD_RS = CTRL_REG;      // Switch to instruction register
// Set LCD_DATA to high nibble of Display On/Off Control
LCD_DATA = (LCD_DATA&0x0F)|0x00;
LCD_RW = WRITE;
LCD_E = 1; LCD_E = 0;   // Write data to display

// Set LCD_DATA to lower nibble of Display On/Off Control
LCD_DATA = (LCD_DATA&0x0F)|((control<<4)&0xF0);
LCD_RW = WRITE;
LCD_E = 1; LCD_E = 0;   // Write data to display
EA = 1;                 // Enable all interrupts

return;
}

/********************************************************************/
/* Function    : gotoxy(x,y)                                    SUB */
/*------------------------------------------------------------------*/
/* Description : Sets the write position of the LCD display         */
/*                                                                  */
/*                 (1,1)         (16,1)                             */
/*                   |              |                               */
/*                   ################   -> line 1                   */
/*                   ################   -> line 2                   */
/*                   |              |                               */
/*                 (1,2)         (16,2)                             */
/*------------------------------------------------------------------*/
/* Author      : Thorsten Godau  NT8                                */
/*------------------------------------------------------------------*/
/* Input       : unsigned char x    -> x position (horizontal)      */
/*               unsigned char y    -> y position (vertical)        */
/*------------------------------------------------------------------*/
/* Returnvalue : none                                               */
/*------------------------------------------------------------------*/
/* History     : 06/99    V1.0 Basic routine                        */
/*                                                                  */
/********************************************************************/
void gotoxy(unsigned char x,unsigned char y)
{

// Table to select DD-RAM address (including instruction and address)
// 0x00..0x0F -> Line 1, 0x40..0x4F -> Line 2
__code const unsigned char LOCATION[2][16] = { {0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,
                                               0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F},
                                              {0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,
                                               0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF} };

EA = 0;                         // Disable all interrupts
LCD_RS = CTRL_REG;              // Switch to instruction register
// Set LCD_DATA to high nibble of position table value
LCD_DATA = (LCD_DATA&0x0F)|((LOCATION[y-1][x-1])&0xF0);
LCD_E = 1; LCD_E = 0;           // Write data to display
// Set LCD_DATA to lower nibble of position table value
LCD_DATA = (LCD_DATA&0x0F)|(((LOCATION[y-1][x-1])<<4)&0xF0);
LCD_E = 1; LCD_E = 0;           // Write data to display
EA = 1;                         // Enable all interrupts

return;
}

/********************************************************************/
/* Function    : setup_cgram                                    SUB */
/*------------------------------------------------------------------*/
/* Description : Sets up the LCD Display's CGRAM to display         */
/*               accented characters                                */
/*------------------------------------------------------------------*/
/* Author      : Molnár Domokos                                     */
/*------------------------------------------------------------------*/
/* Input       : none - uses static hard coded character data       */
/*------------------------------------------------------------------*/
/* Returnvalue : none                                               */
/*------------------------------------------------------------------*/
/* History     : 02/09    V1.0 Basic routine                        */
/*                                                                  */
/********************************************************************/
void setup_cgram(unsigned char* cgram_data)
{

unsigned char counter;

EA = 0;                         // Disable all interrupts
LCD_RS = CTRL_REG;              // Switch to instruction register
LCD_RW = WRITE;                  // Write
// Set CG_RAM address to start of CGRAM buffer
LCD_DATA = (LCD_DATA&0x0F)|(0x40&0xF0);
LCD_E = 1; LCD_E = 0;           // Write data to display
// Set LCD_DATA to lower nibble of position table value
LCD_DATA = (LCD_DATA&0x0F)|((0x40<<4)&0xF0);
LCD_E = 1; LCD_E = 0;           // Write data to display
EA = 1;                         // Enable all interrupts

for(counter=0;counter<64;counter++)
{
  LCD_putchar(cgram_data[counter]);
}

return;
}

/********************************************************************/
/* Function    : clrscr()                                       SUB */
/*------------------------------------------------------------------*/
/* Description : Clears LCD display, and sets position to (1,1)     */
/*------------------------------------------------------------------*/
/* Author      : Thorsten Godau  NT8                                */
/*------------------------------------------------------------------*/
/* Input       : none                                               */
/*------------------------------------------------------------------*/
/* Returnvalue : none                                               */
/*------------------------------------------------------------------*/
/* History     : 06/99    V1.0 Basic routine                        */
/*               09/99    V1.1 Timing correction                    */
/*                                                                  */
/********************************************************************/
void clrscr(void)
{

EA = 0;                         // Disable all interrupts
LCD_RS = CTRL_REG;              // Switch to instruction register
// Set LCD_DATA to high nibble of Clear Screen
LCD_DATA = (LCD_DATA&0x0F)|0x00;
LCD_E = 1; LCD_E = 0;           // Write data to display
// Set LCD_DATA to lower nibble of Clear Screen
LCD_DATA = (LCD_DATA&0x0F)|0x10;
LCD_E = 1; LCD_E = 0;           // Write data to display
EA = 1;                         // Enable all interrupts

return;
}


/********************************************************************/
/* Function    : LCD_putchar(value)                             SUB */
/*------------------------------------------------------------------*/
/* Description : Writes the character value to the display          */
/*------------------------------------------------------------------*/
/* Author      : Thorsten Godau  NT8                                */
/*------------------------------------------------------------------*/
/* Input       : unsigned char value    -> character to be written  */
/*------------------------------------------------------------------*/
/* Returnvalue : none                                               */
/*------------------------------------------------------------------*/
/* History     : 06/99    V1.0 Basic routine                        */
/*                                                                  */
/********************************************************************/
void LCD_putchar(char value)
{
// Decode accented characters
switch(value)
{
    case 'á': value = 0; break;
    case 'é': value = 1; break;
    case 'ó': value = 2; break;
    case 'ö': value = 3; break;
    case 'õ': value = 4; break;
    case 'í': value = 5; break;
    case 'ú': value = 6; break;
    case 'ü': value = 7; break;
}

EA = 0;                         // Disable all interrupts
LCD_RS = DATA_REG;              // Switch to data register
// Set LCD_DATA to high nibble of value
LCD_DATA = (LCD_DATA&0x0F)|(value&0xF0);
LCD_E = 1; LCD_E = 0;           // Write data to display
// Set LCD_DATA to lower nibble of value
LCD_DATA = (LCD_DATA&0x0F)|((value<<4)&0xF0);
LCD_E = 1; LCD_E = 0;           // Write data to display
EA = 1;                         // Enable all interrupts
if (value<8) {delay(40);}
return;
}



/********************************************************************/
/* Function    : LCD_printxy(x,y,*text)                         SUB */
/*------------------------------------------------------------------*/
/* Description : Prints text to position x/y of the display         */
/*------------------------------------------------------------------*/
/* Author      : Thorsten Godau  NT8                                */
/*------------------------------------------------------------------*/
/* Input       : unsigned char x     -> x position of the display   */
/*               unsigned char y     -> y position of the display   */
/*               unsigned char *text -> pointer to text             */
/*------------------------------------------------------------------*/
/* Returnvalue : none                                               */
/*------------------------------------------------------------------*/
/* History     : 06/99    V1.0 Basic routine                        */
/*                                                                  */
/********************************************************************/
void LCD_printxy(unsigned char x,unsigned char y, unsigned char *text)
{

gotoxy(x,y);            // Set cursor position

while( *text )          // while not end of text
  {
  LCD_putchar(*text++); // Write character and increment position
  } 

return;
}

/********************************************************************/
/* Function    : LCD_print(*text)                               SUB */
/*------------------------------------------------------------------*/
/* Description : Prints text to the current position of the display */
/*------------------------------------------------------------------*/
/* Author      : Domokos Molnar                                     */
/*------------------------------------------------------------------*/
/* Input       : unsigned char *text -> pointer to text             */
/*------------------------------------------------------------------*/
/* Returnvalue : none                                               */
/*------------------------------------------------------------------*/
/* History     : 02/09    V1.0 Basic routine                        */
/*                                                                  */
/********************************************************************/
void LCD_print(unsigned char *text)
{

while( *text )          // while not end of text
  {
  LCD_putchar(*text++); // Write character and increment position
  } 

return;
}
