# STM32 FreeRTOS Progressive

> **STM32F103RBT6 (Blue Pill)** üzerinde **FreeRTOS** ve gömülü sistem kavramlarını **gerçek uygulamalarla kademeli öğrenme** projesi.

**Tech Stack:** STM32CubeMX • CMake + Ninja • CMSIS-RTOS v2 • FreeRTOS v10.3.1 • arm-none-eabi-gcc

---

## Öğrenilen RTOS Kavramları

### Temel Task Yönetimi
- [x] **Task oluşturma & lifecycle** - `osThreadNew`, `osDelay`
- [x] **Task öncelikleri & preemption** - BelowNormal → Normal → High
- [x] **Stack sizing** - 512B/task (128 words), heap yönetimi

### Senkronizasyon & İletişim
- [x] **Mutex + Priority Inheritance** - LCD paylaşımı (3 task güvenli erişim)
- [x] **Message Queue** - UART ISR → LCD Task (veri taşıma, 32B)
- [x] **Task Notification** - Buton EXTI ISR → Emergency Task (hafif sinyal)

### Interrupt & RTOS Entegrasyonu
- [x] **ISR-safe API** - `osMessageQueuePut(...,0,0)`, `osThreadFlagsSet`
- [x] **NVIC Priority yönetimi** - `configMAX_SYSCALL_INTERRUPT_PRIORITY = 5`
- [x] **HAL timebase** - TIM4 (SysTick FreeRTOS'e ayrıldı)

### Donanım & Periferal
- [x] **UART Idle Interrupt** - `HAL_UARTEx_ReceiveToIdle_IT` (değişken uzunluk)
- [x] **ADC + DMA** - Dahili sıcaklık sensörü + VREFINT (continuous)
- [x] **EXTI Interrupt** - PC13 buton (falling edge, pull-down)
- [x] **HD44780 LCD 4-bit** - GPIO sürücü, komut parsing

---

## Öğrenme Yol Haritası

| Aşama | Kavram | Uygulama | Durum |
|-------|--------|----------|-------|
| 1 | **Task Basics** | LED blink, task create/delay | ✅ |
| 2 | **Queue** | UART → LCD mesaj iletimi | ✅ |
| 3 | **Mutex** | LCD çoklu task paylaşımı | ✅ |
| 4 | **Task Notification** | Buton → Emergency "ACIL!" | ✅ |
| 5 | **Semaphore** | Binary/Counting - Queue farkı | ⏳ |
| 6 | **Software Timer** | LED blink timer callback | ⏳ |
| 7 | **Stack Analysis** | `uxTaskGetStackHighWaterMark` | ⏳ |
| 8 | **Event Flags** | Çoklu koşul bekleme (ADC+BTN) | ⏳ |
| 9 | **Runtime Stats** | `vTaskList`, CPU% per task | ⏳ |
| 10 | **Dynamic Tasks** | Runtime create/delete | ⏳ |

---

## Proje Yapısı (Özet)

```
Core/
  Src/
    app_tasks.c      # Tüm task implementasyonları
    app_callbacks.c  # ISR callbacks (UART, EXTI)
    lcd.c            # HD44780 4-bit driver
  Inc/
    FreeRTOSConfig.h # RTOS konfigürasyonu
```
