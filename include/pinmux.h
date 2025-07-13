#ifndef PINMUX_H
#define PINMUX_H

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
