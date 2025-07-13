#ifndef PINMUX_H
#define PINMUX_H

#include <stddef.h>  // For NULL definition

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Pin multiplexing macro
 * @param pin Pin name
 * @param func Function name
 */
#define PINMUX(pin, func) \
    do { \
        /* Configure pin multiplexing */ \
        /* Implementation depends on specific hardware */ \
    } while(0)

// Pin function types
typedef enum {
    PIN_FUNC_GPIO = 0,
    PIN_FUNC_UART,
    PIN_FUNC_I2C,
    PIN_FUNC_SPI,
    PIN_FUNC_PWM,
    PIN_FUNC_ADC,
    PIN_FUNC_TIMER,
    PIN_FUNC_CAM,
    PIN_FUNC_AUX,
    PIN_FUNC_DBG,
    PIN_FUNC_XGPIO
} pin_function_type_t;

// Specific pin functions for each chip pin
// Format: PIN_<position>_<function>

// C4 pin functions
#define C4_CAM_MCLK0    1000
#define C4_AUX1         1001
#define C4_XGPIOA_0     1002

// M5 pin functions  
#define M5_UART0_TX     2000
#define M5_CAM_MCLK1    2001
#define M5_PWM_4        2002
#define M5_XGPIOA_16    2003
#define M5_UART1_TX     2004
#define M5_AUX1         2005
#define M5_DBG_6        2006

// Common pin functions for different pins
// A2 pin functions
#define A2_PAD_MIPI_TXM4     3000
#define A2_VI0_D_15          3001
#define A2_SD1_CLK           3002
#define A2_VO_D_24           3003
#define A2_XGPIOC_18         3004
#define A2_CAM_MCLK1         3005
#define A2_PWM_12            3006
#define A2_IIC1_SDA          3007
#define A2_DBG_18            3008

// B1 pin functions
#define B1_XGPIOA_3     3200
#define B1_IIC1_SDA     3201
#define B1_UART1_RX     3202
#define B1_SPI0_MISO    3203

// B2 pin functions
#define B2_XGPIOA_4     3300
#define B2_IIC1_SCL     3301
#define B2_UART1_TX     3302
#define B2_SPI0_MOSI    3303

// More pin functions (expandable for all chip pins)
// Pin 1 functions (QFN numbering)
#define PIN1_XGPIOA_0   4000
#define PIN1_UART0_TX   4001
#define PIN1_PWM_0      4002
#define PIN1_IIC0_SDA   4003

// Pin 2 functions
#define PIN2_XGPIOA_1   4100
#define PIN2_UART0_RX   4101
#define PIN2_PWM_1      4102
#define PIN2_IIC0_SCL   4103

// ADC specific pins
#define ADC0_PIN        5000
#define ADC1_PIN        5001
#define ADC2_PIN        5002
#define ADC3_PIN        5003

// Camera interface pins
#define CAM_MCLK0       6000
#define CAM_MCLK1       6001
#define CAM_PCLK        6002
#define CAM_VSYNC       6003
#define CAM_HSYNC       6004
#define CAM_D0          6005
#define CAM_D1          6006
#define CAM_D2          6007
#define CAM_D3          6008
#define CAM_D4          6009
#define CAM_D5          6010
#define CAM_D6          6011
#define CAM_D7          6012

// Debug interface pins
#define DBG_0           7000
#define DBG_1           7001
#define DBG_2           7002
#define DBG_3           7003
#define DBG_4           7004
#define DBG_5           7005
#define DBG_6           7006
#define DBG_7           7007

// Auxiliary pins
#define AUX0            8000
#define AUX1            8001
#define AUX2            8002
#define AUX3            8003

// Pin function mapping structure
typedef struct {
    const char* pin_name;
    const char* function_name;
    int function_code;
    int is_default;  // 1 if this is the default function, 0 otherwise
} pin_function_map_t;

// Pin function mapping tables
// A2 pin function options (Pin name: PAD_MIPI_TXM4)
static const pin_function_map_t a2_functions[] = {
    {"PAD_MIPI_TXM4", "XGPIOC_18", A2_XGPIOC_18, 1},  // Default function (function select 3)
    {"PAD_MIPI_TXM4", "VI0_D_15", A2_VI0_D_15, 0},    // function select 0
    {"PAD_MIPI_TXM4", "SD1_CLK", A2_SD1_CLK, 0},      // function select 1
    {"PAD_MIPI_TXM4", "VO_D_24", A2_VO_D_24, 0},      // function select 2
    {"PAD_MIPI_TXM4", "CAM_MCLK1", A2_CAM_MCLK1, 0},  // function select 4
    {"PAD_MIPI_TXM4", "PWM_12", A2_PWM_12, 0},        // function select 5
    {"PAD_MIPI_TXM4", "IIC1_SDA", A2_IIC1_SDA, 0},    // function select 6
    {"PAD_MIPI_TXM4", "DBG_18", A2_DBG_18, 0},        // function select 7
    {NULL, NULL, 0, 0}  // End marker
};

// C4 pin function options
static const pin_function_map_t c4_functions[] = {
    {"C4", "CAM_MCLK0", C4_CAM_MCLK0, 1},  // Default function
    {"C4", "AUX1", C4_AUX1, 0},
    {"C4", "XGPIOA_0", C4_XGPIOA_0, 0},
    {NULL, NULL, 0, 0}  // End marker
};

// M5 pin function options
static const pin_function_map_t m5_functions[] = {
    {"M5", "UART0_TX", M5_UART0_TX, 1},    // Default function
    {"M5", "CAM_MCLK1", M5_CAM_MCLK1, 0},
    {"M5", "PWM_4", M5_PWM_4, 0},
    {"M5", "XGPIOA_16", M5_XGPIOA_16, 0},
    {"M5", "UART1_TX", M5_UART1_TX, 0},
    {"M5", "AUX1", M5_AUX1, 0},
    {"M5", "DBG_6", M5_DBG_6, 0},
    {NULL, NULL, 0, 0}  // End marker
};

// Function to get available functions for a pin
const pin_function_map_t* get_pin_functions(const char* pin_name);

// Function to get default function for a pin
const pin_function_map_t* get_default_pin_function(const char* pin_name);

// GPIO pin definitions
#define GPIOA0   0
#define GPIOA1   1
#define GPIOA2   2
#define GPIOA3   3
#define GPIOA4   4
#define GPIOA5   5
#define GPIOA6   6
#define GPIOA7   7
#define GPIOA8   8
#define GPIOA9   9
#define GPIOA10  10
#define GPIOA11  11
#define GPIOA12  12
#define GPIOA13  13
#define GPIOA14  14
#define GPIOA15  15
#define GPIOA16  16
#define GPIOA17  17
#define GPIOA18  18
#define GPIOA19  19
#define GPIOA20  20
#define GPIOA21  21
#define GPIOA22  22
#define GPIOA23  23
#define GPIOA24  24
#define GPIOA25  25
#define GPIOA26  26
#define GPIOA27  27
#define GPIOA28  28
#define GPIOA29  29
#define GPIOA30  30
#define GPIOA31  31

#define GPIOB0   32
#define GPIOB1   33
#define GPIOB2   34
#define GPIOB3   35
#define GPIOB4   36
#define GPIOB5   37
#define GPIOB6   38
#define GPIOB7   39
#define GPIOB8   40
#define GPIOB9   41
#define GPIOB10  42
#define GPIOB11  43
#define GPIOB12  44
#define GPIOB13  45
#define GPIOB14  46
#define GPIOB15  47
#define GPIOB16  48
#define GPIOB17  49
#define GPIOB18  50
#define GPIOB19  51
#define GPIOB20  52
#define GPIOB21  53
#define GPIOB22  54
#define GPIOB23  55
#define GPIOB24  56
#define GPIOB25  57
#define GPIOB26  58
#define GPIOB27  59
#define GPIOB28  60
#define GPIOB29  61
#define GPIOB30  62
#define GPIOB31  63

// Function definitions
#define IIC0_SDA     100
#define IIC0_SCL     101
#define IIC1_SDA     102
#define IIC1_SCL     103
#define IIC2_SDA     104
#define IIC2_SCL     105

#define UART0_TX     200
#define UART0_RX     201
#define UART1_TX     202
#define UART1_RX     203
#define UART2_TX     204
#define UART2_RX     205

#define SPI0_CLK     300
#define SPI0_MOSI    301
#define SPI0_MISO    302
#define SPI0_CS      303
#define SPI1_CLK     304
#define SPI1_MOSI    305
#define SPI1_MISO    306
#define SPI1_CS      307

#define ADC0         400
#define ADC1         401
#define ADC2         402
#define ADC3         403

#define PWM0         500
#define PWM1         501
#define PWM2         502
#define PWM3         503

#define TIMER0       600
#define TIMER1       601
#define TIMER2       602
#define TIMER3       603

#ifdef __cplusplus
}
#endif

#endif // PINMUX_H
