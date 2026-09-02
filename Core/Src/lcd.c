#include "lcd.h"
#include "cmsis_os2.h"
#include "main.h"
#include "core_cm3.h"
#include "string.h"

static void LCD_DWT_Init(void)
{
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0U;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static void LCD_DelayUs(uint32_t microseconds)
{
  uint32_t cycles = microseconds * (SystemCoreClock / 1000000U);
  uint32_t start = DWT->CYCCNT;

  while ((uint32_t)(DWT->CYCCNT - start) < cycles)
  {
  }
}

static void LCD_SetDataPinsMode(uint32_t mode)
{
  GPIO_InitTypeDef gpioInit = {0};

  gpioInit.Pin = lcdData0_Pin | lcdData1_Pin | lcdData2_Pin | lcdData3_Pin;
  gpioInit.Mode = mode;
  gpioInit.Pull = GPIO_NOPULL;
  gpioInit.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &gpioInit);
}

static uint8_t LCD_Read4Bits(void)
{
  uint8_t nibble = 0U;

  HAL_GPIO_WritePin(lcdEnable_GPIO_Port, lcdEnable_Pin, GPIO_PIN_SET);
  LCD_DelayUs(LCD_DELAY_ENABLE_US);

  if (HAL_GPIO_ReadPin(lcdData0_GPIO_Port, lcdData0_Pin) == GPIO_PIN_SET)
    nibble |= 0x01U;
  if (HAL_GPIO_ReadPin(lcdData1_GPIO_Port, lcdData1_Pin) == GPIO_PIN_SET)
    nibble |= 0x02U;
  if (HAL_GPIO_ReadPin(lcdData2_GPIO_Port, lcdData2_Pin) == GPIO_PIN_SET)
    nibble |= 0x04U;
  if (HAL_GPIO_ReadPin(lcdData3_GPIO_Port, lcdData3_Pin) == GPIO_PIN_SET)
    nibble |= 0x08U;

  HAL_GPIO_WritePin(lcdEnable_GPIO_Port, lcdEnable_Pin, GPIO_PIN_RESET);
  LCD_DelayUs(LCD_DELAY_ENABLE_US);

  return nibble;
}

static void LCD_WaitUntilReady(void)
{
  uint32_t start = DWT->CYCCNT;
  uint8_t highNibble;

  LCD_SetDataPinsMode(GPIO_MODE_INPUT);
  HAL_GPIO_WritePin(lcdRegisterSelect_GPIO_Port, lcdRegisterSelect_Pin,
                    GPIO_PIN_RESET);
  HAL_GPIO_WritePin(lcdReadWrite_GPIO_Port, lcdReadWrite_Pin, GPIO_PIN_SET);
  LCD_DelayUs(LCD_DELAY_ENABLE_US);

  do
  {
    highNibble = LCD_Read4Bits();
    (void)LCD_Read4Bits();
  } while ((highNibble & 0x08U) != 0U &&
           (uint32_t)(DWT->CYCCNT - start) <
               LCD_BUSY_TIMEOUT_US * (SystemCoreClock / 1000000U));

  HAL_GPIO_WritePin(lcdReadWrite_GPIO_Port, lcdReadWrite_Pin, GPIO_PIN_RESET);
  LCD_SetDataPinsMode(GPIO_MODE_OUTPUT_PP);
}

static void LCD_Write4Bits(uint8_t nibble)
{
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
  LCD_DelayUs(LCD_DELAY_ENABLE_US);
  HAL_GPIO_WritePin(lcdEnable_GPIO_Port, lcdEnable_Pin, GPIO_PIN_RESET);
  LCD_DelayUs(LCD_DELAY_ENABLE_US);
}

void LCD_CursorShift(uint8_t isDirectionRight)
{
  if (isDirectionRight == 1U)
  {
    LCD_WriteCommand(LCD_SHIFT_CURSOR_OR_DISPLAY | LCD_SHIFT_CURSOR |
                     LCD_SHIFT_RIGHT);
  }
  else
  {
    LCD_WriteCommand(LCD_SHIFT_CURSOR_OR_DISPLAY | LCD_SHIFT_CURSOR |
                     LCD_SHIFT_LEFT);
  }
}

int8_t LCD_Cursor(uint8_t row, uint8_t column)
{
  if (row > 1 || column > 15)
  {
    return -1; // Invalid row or column
  }
  uint8_t address = (row == 0) ? LCD_LINE1_ADDR : LCD_LINE2_ADDR;
  address += column;
  LCD_WriteCommand(LCD_SET_DDRAM_ADDR | address);
  return 0; // Success
}

void LCD_Init(void)
{
  LCD_DWT_Init();

  HAL_GPIO_WritePin(lcdRegisterSelect_GPIO_Port, lcdRegisterSelect_Pin,
                    GPIO_PIN_RESET);
  HAL_GPIO_WritePin(lcdReadWrite_GPIO_Port, lcdReadWrite_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(lcdEnable_GPIO_Port, lcdEnable_Pin, GPIO_PIN_RESET);

  osDelay(50);

  // 4 bit moduna geç
  LCD_Write4Bits(0x03U);
  osDelay(5);
  LCD_Write4Bits(0x03U);
  LCD_DelayUs(LCD_DELAY_INIT_US);
  LCD_Write4Bits(0x03U);
  LCD_DelayUs(LCD_DELAY_INIT_US);
  LCD_Write4Bits(0x02U);
  LCD_DelayUs(LCD_DELAY_INIT_US);

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

void LCD_WriteCommand(uint8_t cmd)
{
  LCD_WaitUntilReady();
  HAL_GPIO_WritePin(lcdRegisterSelect_GPIO_Port, lcdRegisterSelect_Pin,
                    GPIO_PIN_RESET);
  HAL_GPIO_WritePin(lcdReadWrite_GPIO_Port, lcdReadWrite_Pin, GPIO_PIN_RESET);
  LCD_Write4Bits(cmd >> 4);
  LCD_Write4Bits(cmd & 0x0FU);
}

void LCD_Print(const uint8_t *data, uint8_t len)
{
  HAL_GPIO_WritePin(lcdRegisterSelect_GPIO_Port, lcdRegisterSelect_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(lcdReadWrite_GPIO_Port, lcdReadWrite_Pin, GPIO_PIN_RESET);

  for (size_t i = 0; i < len; i++)
  {
    if (i == 16)
    {
      LCD_Cursor(1, 0); // İkinci satıra geç
      HAL_GPIO_WritePin(lcdRegisterSelect_GPIO_Port, lcdRegisterSelect_Pin, GPIO_PIN_SET);
      HAL_GPIO_WritePin(lcdReadWrite_GPIO_Port, lcdReadWrite_Pin, GPIO_PIN_RESET); // LCD_Cursor RS ve RW pinlerini değiştirdiği için tekrar ayarlıyoruz
    }
    else if (i == 32)
    {
      break; // Stop writing if we reach the end of the second line
    }
    LCD_WaitUntilReady();
    HAL_GPIO_WritePin(lcdRegisterSelect_GPIO_Port, lcdRegisterSelect_Pin,
                      GPIO_PIN_SET);
    LCD_Write4Bits(data[i] >> 4);
    LCD_Write4Bits(data[i] & 0x0FU);
  }
}
void LCD_Clear(void)
{
  LCD_WriteCommand(LCD_CLEAR_DISPLAY);
}

void LCD_HandleCommand(uint8_t *cmd)
{
  if (strncmp((char *)cmd, "clear", 5) == 0)
  {
    LCD_Clear();
  }
  else if (strncmp((char *)cmd, "home", 4) == 0)
  {
    LCD_WriteCommand(LCD_RETURN_HOME);
  }
  else if (strncmp((char *)cmd, "display_on", 10) == 0)
  {
    LCD_WriteCommand(LCD_DISPLAY_CONTROL | LCD_DISPLAY_ON | LCD_DISPLAY_CURSOR_OFF | LCD_DISPLAY_BLINK_OFF);
  }
  else if (strncmp((char *)cmd, "display_off", 11) == 0)
  {
    LCD_WriteCommand(LCD_DISPLAY_CONTROL | LCD_DISPLAY_OFF | LCD_DISPLAY_CURSOR_OFF | LCD_DISPLAY_BLINK_OFF);
  }
  else if (strncmp((char *)cmd, "cursor_on", 9) == 0)
  {
    LCD_WriteCommand(LCD_DISPLAY_CONTROL | LCD_DISPLAY_ON | LCD_DISPLAY_CURSOR_ON | LCD_DISPLAY_BLINK_OFF);
  }
  else if (strncmp((char *)cmd, "cursor_off", 10) == 0)
  {
    LCD_WriteCommand(LCD_DISPLAY_CONTROL | LCD_DISPLAY_ON | LCD_DISPLAY_CURSOR_OFF | LCD_DISPLAY_BLINK_OFF);
  }
  else if (strncmp((char *)cmd, "blink_on", 8) == 0)
  {
    LCD_WriteCommand(LCD_DISPLAY_CONTROL | LCD_DISPLAY_ON | LCD_DISPLAY_CURSOR_OFF | LCD_DISPLAY_BLINK_ON);
  }
  else if (strncmp((char *)cmd, "blink_off", 9) == 0)
  {
    LCD_WriteCommand(LCD_DISPLAY_CONTROL | LCD_DISPLAY_ON | LCD_DISPLAY_CURSOR_OFF | LCD_DISPLAY_BLINK_OFF);
  }
  else if (strncmp((char *)cmd, "shift_left", 10) == 0)
  {
    LCD_CursorShift(0);
  }
  else if (strncmp((char *)cmd, "shift_right", 11) == 0)
  {
    LCD_CursorShift(1);
  }
  else if (strncmp((char *)cmd, "func_1line", 9) == 0)
  {
    LCD_WriteCommand(LCD_FUNC_SET | LCD_FUNC_4BIT | LCD_FUNC_1LINE | LCD_FUNC_5x8);
  }
  else if (strncmp((char *)cmd, "func_2line", 10) == 0)
  {
    LCD_WriteCommand(LCD_FUNC_SET | LCD_FUNC_4BIT | LCD_FUNC_2LINE | LCD_FUNC_5x8);
  }
  else if (strncmp((char *)cmd, "func_5x8", 8) == 0)
  {
    LCD_WriteCommand(LCD_FUNC_SET | LCD_FUNC_4BIT | LCD_FUNC_2LINE | LCD_FUNC_5x8);
  }
  else if (strncmp((char *)cmd, "func_5x10", 9) == 0)
  {
    LCD_WriteCommand(LCD_FUNC_SET | LCD_FUNC_4BIT | LCD_FUNC_2LINE | LCD_FUNC_5x10);
  }
  else
  {
    return; // geçersiz komut, hiçbir şey yapma
  }
}
