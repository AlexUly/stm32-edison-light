#include "stm32f3xx_hal.h"
// Адреса наших устройств A0-A2
#define PCA9555_ADR20   0x20
#define PCA9555_ADR21   0x21
#define PCA9555_ADR22   0x22
#define PCA9555_ADR23   0x23
#define PCA9555_ADR24   0x24
#define PCA9555_ADR25   0x25
#define PCA9555_ADR26   0x26
#define PCA9555_ADR27   0x27

//Команды конфигурации

#define PCA9555_INPUT_PORT_0                  0
#define PCA9555_INPUT_PORT_1                  1
#define PCA9555_OUTPUT_PORT_0                 2
#define PCA9555_OUTPUT_PORT_1                 3
#define PCA9555_POLARITY_INVERSION_PORT_0     4
#define PCA9555_POLARITY_INVERSION_PORT_1     5
#define PCA9555_CONFIG_PORT_0                 6
#define PCA9555_CONFIG_PORT_1                 7

#define PIN_1 0x1
#define PIN_2 0x2
#define PIN_3 0x4
#define PIN_4 0x8
#define PIN_5 0x8
#define PIN_6 0x10
#define PIN_7 0x20
#define PIN_8 0x40
