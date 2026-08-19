#ifndef LCD_H
#define LCD_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
                            DB7 DB6 DB5 DB4 DB3 DB2 DB1 DB0
Clear display ->            0   0   0   0   0   0   0   1
Return home ->              0   0   0   0   0   0   1   0
Entry mode set ->           0   0   0   0   0   1   I/D S
Display on/off control ->   0   0   0   0   1   D   C   B
Cursor or display shift ->  0   0   0   1   S/C R/L x   x
Function set ->             0   0   1   DL  N   F   x   x

I/D = 1: Increment
I/D = 0: Decrement
S = 1: Accompanies display shift
S/C = 1: Display shift
S/C = 0: Cursor move
R/L = 1: Shift to the right
R/L = 0: Shift to the left
DL = 1: 8 bits, DL = 0: 4 bits
N = 1: 2 lines, N = 0: 1 line
F = 1: 5 × 10 dots, F = 0: 5 × 8 dots
*/

// Clear Display (0x01, sabit) - parametresi yok, tek başına gönderilir
#define LCD_CMD_CLEAR_DISPLAY 0x01U

// Return Home (0x02, sabit) - parametresi yok, tek başına gönderilir
#define LCD_CMD_RETURN_HOME 0x02U

// Entry Mode Set (0x04 taban) - I/D (bit 1), S (bit 0)
#define LCD_ENTRY_MODE_SET 0x04U
#define LCD_ENTRY_INCREMENT 0x02U // I/D = 1
#define LCD_ENTRY_DECREMENT 0x00U // I/D = 0
#define LCD_ENTRY_SHIFT_ON 0x01U  // S = 1
#define LCD_ENTRY_SHIFT_OFF 0x00U // S = 0

// Display Control (0x08 taban) - D (bit 2), C (bit 1), B (bit 0)
#define LCD_DISPLAY_CONTROL 0x08U
#define LCD_DISPLAY_ON 0x04U // D = 1
#define LCD_DISPLAY_OFF 0x00U
#define LCD_CURSOR_ON 0x02U // C = 1
#define LCD_CURSOR_OFF 0x00U
#define LCD_BLINK_ON 0x01U // B = 1
#define LCD_BLINK_OFF 0x00U

// Cursor or Display Shift (0x10 taban) - S/C (bit 3), R/L (bit 2)
#define LCD_SHIFT_CURSOR_OR_DISPLAY 0x10U
#define LCD_SHIFT_DISPLAY 0x08U // S/C = 1 (ekran kayar)
#define LCD_SHIFT_CURSOR 0x00U  // S/C = 0 (sadece imleç kayar)
#define LCD_SHIFT_RIGHT 0x04U   // R/L = 1
#define LCD_SHIFT_LEFT 0x00U    // R/L = 0

// Function Set (0x20 taban) - DL (bit 4), N (bit 3), F (bit 2)
#define LCD_FUNCTION_SET 0x20U
#define LCD_FUNC_8BIT 0x10U  // DL = 1
#define LCD_FUNC_4BIT 0x00U  // DL = 0
#define LCD_FUNC_2LINE 0x08U // N = 1
#define LCD_FUNC_1LINE 0x00U // N = 0
#define LCD_FUNC_5x10 0x04U  // F = 1
#define LCD_FUNC_5x8 0x00U   // F = 0

void LCD_Init(void);
void LCD_Clear(void);
void LCD_WriteChar(char character);
void LCD_WriteString(const char *string);
void LCD_Cursor(uint8_t row, uint8_t column);
void LCD_CursorShift(uint8_t isDirectionRight);
#ifdef __cplusplus
}
#endif

#endif /* LCD_H */
