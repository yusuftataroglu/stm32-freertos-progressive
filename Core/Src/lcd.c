#include "lcd.h"
#include "main.h"

static void LCD_Write4Bits(uint8_t nibble) {
  HAL_GPIO_WritePin(lcdData0_GPIO_Port, lcdData0_Pin,
                    (nibble & 0x01U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(lcdData1_GPIO_Port, lcdData1_Pin,
                    (nibble & 0x02U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(lcdData2_GPIO_Port, lcdData2_Pin,
                    (nibble & 0x04U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(lcdData3_GPIO_Port, lcdData3_Pin,
                    (nibble & 0x08U) ? GPIO_PIN_SET : GPIO_PIN_RESET);

  // The HD44780 latches the nibble on the falling edge of Enable.
  HAL_GPIO_WritePin(lcdEnable_GPIO_Port, lcdEnable_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(lcdEnable_GPIO_Port, lcdEnable_Pin, GPIO_PIN_RESET);
}

static void LCD_WriteCommand(uint8_t command) {
  HAL_GPIO_WritePin(lcdRegisterSelect_GPIO_Port, lcdRegisterSelect_Pin,
                    GPIO_PIN_RESET);
  HAL_GPIO_WritePin(lcdReadWrite_GPIO_Port, lcdReadWrite_Pin, GPIO_PIN_RESET);
  LCD_Write4Bits(command >> 4);
  LCD_Write4Bits(command & 0x0FU);
  HAL_Delay(2);
}

void LCD_Init(void) {
  HAL_GPIO_WritePin(lcdRegisterSelect_GPIO_Port, lcdRegisterSelect_Pin,
                    GPIO_PIN_RESET);
  HAL_GPIO_WritePin(lcdReadWrite_GPIO_Port, lcdReadWrite_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(lcdEnable_GPIO_Port, lcdEnable_Pin, GPIO_PIN_RESET);

  // Allow the LCD controller to finish its power-on reset.
  HAL_Delay(40);

  // HD44780 starts in 8-bit mode; three 0x03 nibbles make that state robust.
  LCD_Write4Bits(0x03U);
  HAL_Delay(5);
  LCD_Write4Bits(0x03U);
  HAL_Delay(1);
  LCD_Write4Bits(0x03U);
  HAL_Delay(1);

  // Select 4-bit transfers. Later commands use two nibbles.
  LCD_Write4Bits(0x02U);
  HAL_Delay(1);

  LCD_WriteCommand(0x28U); // 4-bit, 2 lines, 5x8 character font
  LCD_WriteCommand(0x0CU); // Display on, cursor off, blink off
  LCD_WriteCommand(0x06U); // Increment cursor, no display shift
  LCD_Clear();
}

void LCD_Clear(void) {
  LCD_WriteCommand(0x01U);
  HAL_Delay(2);
}

void LCD_WriteChar(char character) {
  HAL_GPIO_WritePin(lcdRegisterSelect_GPIO_Port, lcdRegisterSelect_Pin,
                    GPIO_PIN_SET);
  HAL_GPIO_WritePin(lcdReadWrite_GPIO_Port, lcdReadWrite_Pin, GPIO_PIN_RESET);
  LCD_Write4Bits((uint8_t)character >> 4);
  LCD_Write4Bits((uint8_t)character & 0x0FU);
}

void LCD_WriteString(const char *string) {
  while (*string != '\0') {
    LCD_WriteChar(*string++);
    LCD_CursorShift(1U); // Move cursor right after each character
  }
}

void LCD_Cursor(uint8_t row, uint8_t column) {
  static const uint8_t rowOffsets[] = {0x00U, 0x40U};

  if (row < 2U && column < 16U) {
    LCD_WriteCommand(0x80U | (rowOffsets[row] + column));
  }
}

void LCD_CursorShift(uint8_t isDirectionRight) {
  if (isDirectionRight == 1U) {
    LCD_WriteCommand(0x18U); // Shift cursor left
  } else {
    LCD_WriteCommand(0x10U); // Shift cursor right
  }
}
