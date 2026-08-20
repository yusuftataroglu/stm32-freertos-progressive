#include "lcd.h"
#include "cmsis_os2.h"
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
  osDelay(LCD_DELAY_SHORT_MS); // Enable pulse width >= 450ns, using 1ms for
                               // simplicity
  HAL_GPIO_WritePin(lcdEnable_GPIO_Port, lcdEnable_Pin, GPIO_PIN_RESET);
  osDelay(LCD_DELAY_SHORT_MS);
}

void LCD_CursorShift(uint8_t isDirectionRight) {
  if (isDirectionRight == 1U) {
    LCD_WriteCommand(LCD_SHIFT_CURSOR_OR_DISPLAY | LCD_SHIFT_CURSOR |
                     LCD_SHIFT_RIGHT);
  } else {
    LCD_WriteCommand(LCD_SHIFT_CURSOR_OR_DISPLAY | LCD_SHIFT_CURSOR |
                     LCD_SHIFT_LEFT);
  }
}

int8_t LCD_Cursor(uint8_t row, uint8_t column) {
  if (row > 1 || column > 15) {
    return -1; // Invalid row or column
  }
  uint8_t address = (row == 0) ? LCD_LINE1_ADDR : LCD_LINE2_ADDR;
  address += column;
  LCD_WriteCommand(LCD_SET_DDRAM_ADDR | address);
  return 0; // Success
}

void LCD_Init(void) {
  HAL_GPIO_WritePin(lcdRegisterSelect_GPIO_Port, lcdRegisterSelect_Pin,
                    GPIO_PIN_RESET);
  HAL_GPIO_WritePin(lcdReadWrite_GPIO_Port, lcdReadWrite_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(lcdEnable_GPIO_Port, lcdEnable_Pin, GPIO_PIN_RESET);

  osDelay(50);

  // 4 bit moduna geç
  LCD_Write4Bits(0x03U);
  osDelay(5);
  LCD_Write4Bits(0x03U);
  osDelay(1);
  LCD_Write4Bits(0x03U);
  osDelay(1);
  LCD_Write4Bits(0x02U);
  osDelay(1);

  // Function Set: 4-bit, 2 satır, 5x8 font
  LCD_WriteCommand(LCD_FUNC_SET | LCD_FUNC_4BIT | LCD_FUNC_2LINE |
                   LCD_FUNC_5x8);
  // = 0x28

  // Display Control: ekranı kapat (init sırasında kapalı tutmak standart
  // pratik)
  LCD_WriteCommand(LCD_DISPLAY_CONTROL | LCD_DISPLAY_OFF);
  // = 0x08

  // Clear Display
  LCD_WriteCommand(LCD_CLEAR_DISPLAY);
  // = 0x01 - bu komut ~1.52ms sürüyor

  // Entry Mode Set: imleç sağa gitsin, ekran kaymasın
  LCD_WriteCommand(LCD_ENTRY_MODE_SET | LCD_ENTRY_INCREMENT |
                   LCD_ENTRY_SHIFT_OFF);
  // = 0x06

  // Display Control: ekranı aç, imleç kapalı, blink kapalı
  LCD_WriteCommand(LCD_DISPLAY_CONTROL | LCD_DISPLAY_ON |
                   LCD_DISPLAY_CURSOR_OFF | LCD_DISPLAY_BLINK_OFF);
  // = 0x0C
}

void LCD_WriteCommand(uint8_t cmd) {
  HAL_GPIO_WritePin(lcdRegisterSelect_GPIO_Port, lcdRegisterSelect_Pin,
                    GPIO_PIN_RESET);
  HAL_GPIO_WritePin(lcdReadWrite_GPIO_Port, lcdReadWrite_Pin, GPIO_PIN_RESET);
  osDelay(
      LCD_DELAY_SHORT_MS); // Ensure RS and RW are stable before sending command
  LCD_Write4Bits(cmd >> 4);
  LCD_Write4Bits(cmd & 0x0FU);

  if (cmd == LCD_CLEAR_DISPLAY || cmd == LCD_RETURN_HOME) {
    osDelay(
        LCD_DELAY_LONG_MS); // 1.52ms delay clear ve return home komutları için
  } else {
    osDelay(LCD_DELAY_SHORT_MS); // 37us delay diğer komutlar için (daha sonra
                                 // timer ile optimize edilebilir)
  }
}

void LCD_Print(const uint8_t *data, uint8_t len) {
  HAL_GPIO_WritePin(lcdRegisterSelect_GPIO_Port, lcdRegisterSelect_Pin,
                    GPIO_PIN_SET);
  HAL_GPIO_WritePin(lcdReadWrite_GPIO_Port, lcdReadWrite_Pin, GPIO_PIN_RESET);
  osDelay(
      LCD_DELAY_SHORT_MS); // Ensure RS and RW are stable before sending data
  for (size_t i = 0; i < len; i++) {
    if (i == 16) {
      LCD_Cursor(1, 0); // İkinci satıra geç
      HAL_GPIO_WritePin(lcdRegisterSelect_GPIO_Port, lcdRegisterSelect_Pin,
                        GPIO_PIN_SET);
      HAL_GPIO_WritePin(lcdReadWrite_GPIO_Port, lcdReadWrite_Pin,
                        GPIO_PIN_RESET); // LCD_Cursor RS ve RW pinlerini
      // değiştirdiği için tekrar ayarlıyoruz
      osDelay(LCD_DELAY_SHORT_MS); // Ensure RS and RW are stable before sending
                                   // data
    } else if (i == 32) {
      break; // Stop writing if we reach the end of the second line
    }
    LCD_Write4Bits(data[i] >> 4);
    LCD_Write4Bits(data[i] & 0x0FU);
    osDelay(LCD_DELAY_SHORT_MS); // Execution time for writing a character is
                                 // ~43us, using 1ms for simplicity
  }
}
void LCD_Clear(void) { LCD_WriteCommand(LCD_CLEAR_DISPLAY); }
