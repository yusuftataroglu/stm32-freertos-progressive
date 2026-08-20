# STM32 FreeRTOS Progressive

STM32F103RBT6 kartı üzerinde FreeRTOS ve gömülü sistemler kavramlarını gerçek
uygulamalarla öğrenmek için geliştirilen kademeli bir projedir. Proje STM32CubeMX,
CMake, CMSIS-RTOS v2 ve FreeRTOS kullanır.

## Mevcut Durum

Şu ana kadar tamamlanan uygulamalar:

- CubeMX ile temel proje iskeleti oluşturuldu.
- FreeRTOS, CMSIS-RTOS v2 arayüzü ile etkinleştirildi.
- HAL timebase kaynağı SysTick yerine TIM4 olarak yapılandırıldı.
- LEDBlinkTask oluşturuldu ve LED, osDelay(500) kullanılarak 500 ms aralıkla toggle ediliyor.
- USART2, asenkron 115200 8-N-1 modunda yapılandırıldı.
- printf() çıktısı USART2 üzerinden terminale yönlendirildi.
- HAL_UARTEx_ReceiveToIdle_IT() ile değişken uzunlukta UART alımı eklendi.
- UART callback ile LCD task arasında CMSIS-RTOS message queue kullanıldı.
- HD44780 uyumlu 16x2 LCD için 4-bit GPIO sürücüsü yazıldı.
- UART üzerinden gelen metin LCD task tarafından LCD ekrana yazdırılıyor.
- LCD komutları terminal üzerinden gönderilebilir hale getirildi.

## Donanım

### USART2

| STM32 pini | Görev |
|---|---|
| PA2 | USART2 TX |
| PA3 | USART2 RX |

Terminal ayarları:

- Baud rate: 115200
- Data bits: 8
- Parity: None
- Stop bits: 1
- Flow control: None

### HD44780 16x2 LCD

LCD, 4-bit modda kullanılır. RW pini yazma işlemleri için düşük seviyede tutulur.

| LCD sinyali | STM32 pini |
|---|---|
| RS | PB10 |
| RW | PB11 |
| D4 | PB12 |
| D5 | PB13 |
| D6 | PB14 |
| D7 | PB15 |
| E | PC6 |

LCD sürücüsünün genel API'si (Core/Inc/lcd.h), uygulaması ise
(Core/Src/lcd.c) içindedir.

## Yazılım Mimarisi

text
Terminal
	|
	v
USART2 RX interrupt
	|
	v
HAL_UARTEx_RxEventCallback
	|
	v
lcdQueue
	|
	v
StartLCDTask
	|
	v
HD44780 LCD


- UART callback'i interrupt context içinde çalışır ve queue'ya bloklamadan mesaj bırakır.
- LCD task queue'dan bloklayarak mesaj bekler.
- LCD'ye ait zamanlama ve GPIO işlemleri task context içinde gerçekleştirilir.
- UART mesajları en fazla 32 byte olacak şekilde buffer'lanır.
- LCD çıktısı 16x2 ekran kapasitesiyle sınırlıdır.

## Desteklenen LCD Komutları

Terminalden aşağıdaki komutlar gönderilebilir:

text
clear
home
display_on
display_off
cursor_on
cursor_off
blink_on
blink_off
shift_left
shift_right
func_1line
func_2line
func_5x8
func_5x10


Komut dışındaki metinler LCD'ye normal karakter verisi olarak yazdırılır.

## Derleme

Proje kök dizininde:

text
cmake --preset Debug
cmake --build --preset Debug


Release derlemesi için:

text
cmake --preset Release
cmake --build --preset Release


CubeMX tarafından yeniden üretilebilen CMake dosyası
cmake/stm32cubemx/CMakeLists.txt içindedir. Kullanıcıya ait lcd.c gibi
dosyalar kalıcı CMake girişi olan kök (CMakeLists.txt) içinde
kaynak listesine eklenmelidir.

## Öğrenme Yol Haritası

Bir sonraki aşamalarda şu konuların eklenmesi planlanmaktadır:

- UART mesaj protokolünün ve komut ayrıştırıcısının iyileştirilmesi
- Queue taşması ve hata durumlarının ele alınması
- Task stack kullanımının uxTaskGetStackHighWaterMark() ile ölçülmesi
- UART RX buffer ve mesaj uzunluğu yönetiminin daha sağlam hale getirilmesi
- FreeRTOS senkronizasyon araçları: semaphore, mutex ve event flags
- Yeni donanımların CubeMX üzerinden eklenmesi ve uygulama task'larıyla birleştirilmesi
