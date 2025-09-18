#include "pinfunction.h"

PinFunction::PinFunction()
{
    initializePinFunctions();
}

QStringList PinFunction::getSupportedFunctions(const QString& pinName) const
{
    return m_pinFunctions.value(pinName, QStringList() << "GPIO");
}

QString PinFunction::getDefaultFunction(const QString& pinName) const
{
    return m_defaultFunctions.value(pinName, "GPIO");
}

QString PinFunction::getFunctionMacroName(const QString& pinName, const QString& function) const
{
    QString key = pinName + "_" + function;
    return m_functionMacros.value(key, function.toUpper());
}

bool PinFunction::isPinFunctionSupported(const QString& pinName, const QString& function) const
{
    return m_pinFunctions.value(pinName).contains(function);
}

void PinFunction::initializePinFunctions()
{
    // A2 引脚功能定义（Pin name: PAD_MIPI_TXM4）
    // 引脚名称使用实际的PAD名称
    m_pinFunctions["PAD_MIPI_TXM4"] = QStringList() << "VI0_D_15" << "SD1_CLK" << "VO_D_24" 
                                        << "XGPIOC_18" << "CAM_MCLK1" << "PWM_12" 
                                        << "IIC1_SDA" << "DBG_18";
    m_defaultFunctions["PAD_MIPI_TXM4"] = "XGPIOC_18";  // 默认功能 (function select 3)
    // 功能宏映射
    m_functionMacros["XGPIOC_18"] = "XGPIOC_18";
    m_functionMacros["VI0_D_15"] = "VI0_D_15";
    m_functionMacros["SD1_CLK"] = "SD1_CLK";
    m_functionMacros["VO_D_24"] = "VO_D_24";
    m_functionMacros["CAM_MCLK1"] = "CAM_MCLK1";
    m_functionMacros["PWM_12"] = "PWM_12";
    m_functionMacros["IIC1_SDA"] = "IIC1_SDA";
    m_functionMacros["DBG_18"] = "DBG_18";
    // A4 引脚功能定义（Pin name: PAD_MIPIRX0N）
    // 引脚名称使用实际的PAD名称
    m_pinFunctions["PAD_MIPIRX0N"] = QStringList() << "VI0_D_7" << "VO_D_6" << "XGPIOC_10" 
                                        << "IIC1_SCL" << "CAM_MCLK1" << "PWM_OFF_0" 
                                        << "DBG_10";
    m_defaultFunctions["PAD_MIPIRX0N"] = "XGPIOC_10";  // 默认功能 (function select 3)
    // 功能宏映射
    m_functionMacros["VI0_D_7"] = "VI0_D_7";
    m_functionMacros["VO_D_6"] = "VO_D_6";
    m_functionMacros["XGPIOC_10"] = "XGPIOC_10";
    m_functionMacros["IIC1_SCL"] = "IIC1_SCL";
    m_functionMacros["CAM_MCLK1"] = "CAM_MCLK1";
    m_functionMacros["PWM_OFF_0"] = "PWM_OFF_0";
    m_functionMacros["DBG_10"] = "DBG_10";

    // A6 引脚功能定义（Pin name: PAD_MIPIRX3P
    // 引脚名称使用实际的PAD名称
    m_pinFunctions["PAD_MIPIRX3P"] = QStringList() << "CV_4WTCK_CR_2WTCK" << "VI0_D_2" << "VI1_D_16" 
                                        << "XGPIOC_5" << "MUX_SPI1_MOSI";
    m_defaultFunctions["PAD_MIPIRX3P"] = "XGPIOC_5";  // 默认功能 (function select 3)
    // 功能宏映射
    m_functionMacros["CV_4WTCK_CR_2WTCK"] = "CV_4WTCK_CR_2WTCK";
    m_functionMacros["VI0_D_2"] = "VI0_D_2";
    m_functionMacros["VI1_D_16"] = "VI1_D_16";
    m_functionMacros["XGPIOC_5"] = "XGPIOC_5";
    m_functionMacros["MUX_SPI1_MOSI"] = "MUX_SPI1_MOSI";

    // A7 引脚功能定义（Pin name: PAD_MIPIRX4P
    // 引脚名称使用实际的PAD名称
    m_pinFunctions["PAD_MIPIRX4P"] = QStringList() << "CV_4WTMS_CR_SDA0" << "VI0_D_0" << "VI1_D_14" 
                                        << "XGPIOC_3" << "IIC1_SCL" << "CAM_MCLK1" 
                                        << "KEY_ROW1"<< "MUX_SPI1_CS";
    m_defaultFunctions["PAD_MIPIRX4P"] = "CV_4WTMS_CR_SDA0";  // 默认功能 (function select 0)
    // 功能宏映射
    m_functionMacros["CV_4WTMS_CR_SDA0"] = "CV_4WTMS_CR_SDA0";
    m_functionMacros["VI0_D_0"] = "VI0_D_0";
    m_functionMacros["VI1_D_14"] = "VI1_D_14";
    m_functionMacros["XGPIOC_3"] = "XGPIOC_3";
    m_functionMacros["IIC1_SCL"] = "IIC1_SCL";
    m_functionMacros["CAM_MCLK1"] = "CAM_MCLK1";
    m_functionMacros["KEY_ROW1"] = "KEY_ROW1";
    m_functionMacros["MUX_SPI1_CS"] = "MUX_SPI1_CS";

   // A9 引脚功能定义（Pin name: VIVO_D2
    // 引脚名称使用实际的PAD名称
    m_pinFunctions["VIVO_D2"] = QStringList() << "VI2_D_2" << "VI1_D_2" << "VO_D_15" 
                                        << "XGPIOB_19" << "RMII0_TXD1" << "CAM_MCLK1" 
                                        << "PWM_2" << "UART2_TX";
    m_defaultFunctions["VIVO_D2"] = "XGPIOB_19";  // 默认功能 (function select 3)
    // 功能宏映射
    m_functionMacros["VI2_D_2"] = "VI2_D_2";
    m_functionMacros["VI1_D_2"] = "VI1_D_2";
    m_functionMacros["VO_D_15"] = "VO_D_15";
    m_functionMacros["XGPIOB_19"] = "XGPIOB_19";
    m_functionMacros["RMII0_TXD1"] = "RMII0_TXD1";
    m_functionMacros["CAM_MCLK1"] = "CAM_MCLK1";
    m_functionMacros["PWM_2"] = "PWM_2";
    m_functionMacros["UART2_TX"] = "UART2_TX";

    // A10 引脚功能定义（Pin name: VIVO_D3
    // 引脚名称使用实际的PAD名称
    m_pinFunctions["VIVO_D3"] = QStringList() << "VI2_D_3" << "VI1_D_3" << "VO_D_16" 
                                        << "XGPIOB_18" << "RMII0_TXD0" << "IIC1_SCL" 
                                        << "UART2_RTS" << "CAM_HS0";
    m_defaultFunctions["VIVO_D3"] = "XGPIOB_18";  // 默认功能 (function select 3)
    // 功能宏映射
    m_functionMacros["VI2_D_3"] = "VI2_D_3";
    m_functionMacros["VI1_D_3"] = "VI1_D_3";
    m_functionMacros["VO_D_16"] = "VO_D_16";
    m_functionMacros["XGPIOB_18"] = "XGPIOB_18";
    m_functionMacros["RMII0_TXD0"] = "RMII0_TXD0";
    m_functionMacros["IIC1_SCL"] = "IIC1_SCL";
    m_functionMacros["UART2_RTS"] = "UART2_RTS";
    m_functionMacros["CAM_HS0"] = "CAM_HS0";

    // A12 引脚功能定义（Pin name: VIVO_D10
    // 引脚名称使用实际的PAD名称
    m_pinFunctions["VIVO_D10"] = QStringList() << "PWM_1" << "VI1_D_10" << "VO_D_23" 
                                        << "XGPIOB_11" << "RMII0_IRQ" << "CAM_MCLK0" 
                                        << "IIC1_SDA" << "UART2_TX";
    m_defaultFunctions["VIVO_D10"] = "XGPIOB_11";  // 默认功能 (function select 3)
    // 功能宏映射
    m_functionMacros["PWM_1"] = "PWM_1";
    m_functionMacros["VI1_D_10"] = "VI1_D_10";
    m_functionMacros["VO_D_23"] = "VO_D_23";
    m_functionMacros["IIC1_SCL"] = "IIC1_SCL";
    m_functionMacros["XGPIOB_11"] = "XGPIOB_11";
    m_functionMacros["RMII0_IRQ"] = "RMII0_IRQ";
    m_functionMacros["CAM_MCLK0"] = "CAM_MCLK0";
    m_functionMacros["IIC1_SDA"] = "IIC1_SDA";
    m_functionMacros["UART2_TX"] = "UART2_TX";

    // A13 引脚功能定义（Pin name: USB_VBUS_DET）
    // 引脚名称使用实际的PAD名称
    m_pinFunctions["USB_VBUS_DET"] = QStringList() << "USB_VBUS_DET" << "XGPIOB_6" << "CAM_MCLK0"  << "CAM_MCLK1";
    m_defaultFunctions["USB_VBUS_DET"] = "USB_VBUS_DET";  // 默认功能 (function select 0)
    // 功能宏映射
    m_functionMacros["XGPIOB_6"] = "XGPIOB_6";
    m_functionMacros["VO_D_6"] = "VO_D_6";
    m_functionMacros["CAM_MCLK0"] = "CAM_MCLK0";
    m_functionMacros["CAM_MCLK1"] = "CAM_MCLK1";

    // B1 引脚功能定义 Pin name: PAD_MIPI_TXP3
    // 引脚名称使用实际的PAD名称
    m_pinFunctions["PAD_MIPI_TXP3"] = QStringList() << "VI0_D_18" << "SD1_D1" << "VO_D_27" 
                                        << "XGPIOC_21" << "IIC2_SCL" << "PWM_15" 
                                        << "IIC1_SCL" << "CAM_HS0";
    m_defaultFunctions["PAD_MIPI_TXP3"] = "XGPIOC_21";  // 默认功能 (function select 3)
    // 功能宏映射
    m_functionMacros["VI0_D_18"] = "VI0_D_18";
    m_functionMacros["SD1_D1"] = "SD1_D1";
    m_functionMacros["VO_D_27"] = "VO_D_27";
    m_functionMacros["XGPIOC_21"] = "XGPIOC_21";
    m_functionMacros["IIC2_SCL"] = "IIC2_SCL";
    m_functionMacros["PWM_15"] = "PWM_15";
    m_functionMacros["IIC1_SCL"] = "IIC1_SCL";
    m_functionMacros["CAM_HS0"] = "CAM_HS0";

    // B2 引脚功能定义 Pin name: PAD_MIPI_TXM3
    // 引脚名称使用实际的PAD名称
    m_pinFunctions["PAD_MIPI_TXM3"] = QStringList() << "VI0_D_17" << "SD1_D0" << "VO_D_26" 
                                        << "XGPIOC_20" << "IIC2_SDA" << "PWM_14" 
                                        << "IIC1_SDA" << "CAM_VS0";
    m_defaultFunctions["PAD_MIPI_TXM3"] = "XGPIOC_20";  // 默认功能 (function select 3)
    // 功能宏映射
    m_functionMacros["VI0_D_17"] = "VI0_D_17";
    m_functionMacros["SD1_D0"] = "SD1_D0";
    m_functionMacros["VO_D_26"] = "VO_D_26";
    m_functionMacros["XGPIOC_20"] = "XGPIOC_20";
    m_functionMacros["IIC2_SDA"] = "IIC2_SDA";
    m_functionMacros["PWM_14"] = "PWM_14";
    m_functionMacros["IIC1_SDA"] = "IIC1_SDA";
    m_functionMacros["CAM_VS0"] = "CAM_VS0";
    
    // B3 引脚功能定义 Pin name: PAD_MIPI_TXP4
    // 引脚名称使用实际的PAD名称
    m_pinFunctions["PAD_MIPI_TXP4"] = QStringList() << "VI0_D_16" << "SD1_CMD " << "VO_D_25" 
                                        << "XGPIOC_19" << "CAM_MCLK0" << "PWM_13" 
                                        << "IIC1_SCL" << "DBG_19";
    m_defaultFunctions["PAD_MIPI_TXP4"] = "XGPIOC_19";  // 默认功能 (function select 3)
    // 功能宏映射
    m_functionMacros["VI0_D_16"] = "VI0_D_16";
    m_functionMacros["SD1_CMD"] = "SD1_CMD";
    m_functionMacros["VO_D_25"] = "VO_D_25";
    m_functionMacros["XGPIOC_19"] = "XGPIOC_19";
    m_functionMacros["CAM_MCLK0"] = "CAM_MCLK0";
    m_functionMacros["PWM_15"] = "PWM_15";
    m_functionMacros["IIC1_SCL"] = "IIC1_SCL";
    m_functionMacros["DBG_19"] = "DBG_19";

    // B4 引脚功能定义 Pin name: PAD_MIPIRX0P
    // 引脚名称使用实际的PAD名称
    m_pinFunctions["PAD_MIPIRX0P"] = QStringList() << "VI0_D_8" << "VO_D_5" << "XGPIOC_11" 
                                        << "CAM_MCLK0" << "DBG_11";
    m_defaultFunctions["PAD_MIPIRX0P"] = "XGPIOC_11";  // 默认功能 (function select 3)
    // 功能宏映射
    m_functionMacros["VI0_D_8"] = "VI0_D_8";
    m_functionMacros["VO_D_5"] = "VO_D_5";
    m_functionMacros["XGPIOC_11"] = "XGPIOC_11";
    m_functionMacros["CAM_MCLK0"] = "CAM_MCLK0";
    m_functionMacros["DBG_11"] = "DBG_11";

    // B5 引脚功能定义 Pin name: PAD_MIPIRX1N
    // 引脚名称使用实际的PAD名称
    m_pinFunctions["PAD_MIPIRX1N"] = QStringList() << "VI0_D_5" << "VO_D_8" << "XGPIOC_8" 
                                        << "KEY_ROW3" << "DBG_8";
    m_defaultFunctions["PAD_MIPIRX1N"] = "XGPIOC_8";  // 默认功能 (function select 3)
    // 功能宏映射
    m_functionMacros["VI0_D_5"] = "VI0_D_5";
    m_functionMacros["VO_D_8"] = "VO_D_8";
    m_functionMacros["XGPIOC_8"] = "XGPIOC_8";
    m_functionMacros["KEY_ROW3"] = "KEY_ROW3";
    m_functionMacros["DBG_8"] = "DBG_8";
    
    // B6 引脚功能定义 Pin name: PAD_MIPIRX2N
    // 引脚名称使用实际的PAD名称
    m_pinFunctions["PAD_MIPIRX2N"] = QStringList() << "VI0_D_3" << "VO_D_10" << "XGPIOC_6" 
                                        << "VI1_D_17" << "IIC4_SCL" << "DBG_6";
    m_defaultFunctions["PAD_MIPIRX2N"] = "XGPIOC_6";  // 默认功能 (function select 3)
    // 功能宏映射
    m_functionMacros["VI0_D_3"] = "VI0_D_3";
    m_functionMacros["VO_D_10"] = "VO_D_10";
    m_functionMacros["XGPIOC_6"] = "XGPIOC_6";
    m_functionMacros["VI1_D_17"] = "VI1_D_17";
    m_functionMacros["IIC4_SCL"] = "IIC4_SCL";
    m_functionMacros["DBG_6"] = "DBG_6";

    // B7 引脚功能定义 Pin name: PAD_MIPIRX4N
    // 引脚名称使用实际的PAD名称
    m_pinFunctions["PAD_MIPIRX4N"] = QStringList() << "CV_4WTDI_CR_SCL0" << "VI0_CLK" << "VI1_D_13" 
                                        << "XGPIOC_2" << "IIC1_SDA" << "CAM_MCLK0"
                                        << "KEY_ROW0" << "MUX_SPI1_SCK";
    m_defaultFunctions["PAD_MIPIRX4N"] = "CV_4WTDI_CR_SCL0";  // 默认功能 (function select 0)
    // 功能宏映射
    m_functionMacros["CV_4WTDI_CR_SCL0"] = "CV_4WTDI_CR_SCL0";
    m_functionMacros["VI0_CLK"] = "VI0_CLK";
    m_functionMacros["VI1_D_13"] = "VI1_D_13";
    m_functionMacros["XGPIOC_2"] = "XGPIOC_2";
    m_functionMacros["IIC1_SDA"] = "IIC1_SDA";
    m_functionMacros["CAM_MCLK0"] = "CAM_MCLK0";
    m_functionMacros["KEY_ROW0"] = "KEY_ROW0";
    m_functionMacros["MUX_SPI1_SCK"] = "MUX_SPI1_SCK";

    // B8 引脚功能定义 Pin name: PAD_MIPIRX5N
    // 引脚名称使用实际的PAD名称
    m_pinFunctions["PAD_MIPIRX5N"] = QStringList() << "VI1_D_11" << "VO_D_12" << "XGPIOC_0" 
                                        << "CAM_MCLK0" << "WG0_D0" << "DBG_0";
    m_defaultFunctions["PAD_MIPIRX5N"] = "XGPIOC_0";  // 默认功能 (function select 3)
    // 功能宏映射
    m_functionMacros["VI1_D_11"] = "VI1_D_11";
    m_functionMacros["VO_D_12"] = "VO_D_12";
    m_functionMacros["XGPIOC_0"] = "XGPIOC_0";
    m_functionMacros["CAM_MCLK0"] = "CAM_MCLK0";
    m_functionMacros["WG0_D0"] = "WG0_D0";
    m_functionMacros["DBG_0"] = "DBG_0";

    // B9 引脚功能定义 Pin name: VIVO_D1
    // 引脚名称使用实际的PAD名称
    m_pinFunctions["VIVO_D1"] = QStringList() << "VI2_D_1" << "VI1_D_1" << "VO_D_14" 
                                        << "XGPIOB_20" << "RMII0_RXDV" << "IIC3_SDA" 
                                        << "PWM_3" << "IIC4_SCL";
    m_defaultFunctions["VIVO_D1"] = "XGPIOB_20";  // 默认功能 (function select 3)
    // 功能宏映射
    m_functionMacros["VI2_D_1"] = "VI2_D_1";
    m_functionMacros["VI1_D_1"] = "VI1_D_1";
    m_functionMacros["VO_D_14"] = "VO_D_14";
    m_functionMacros["XGPIOB_20"] = "XGPIOB_20";
    m_functionMacros["RMII0_RXDV"] = "RMII0_RXDV";
    m_functionMacros["IIC3_SDA"] = "IIC3_SDA";
    m_functionMacros["PWM_3"] = "PWM_3";
    m_functionMacros["IIC4_SCL"] = "IIC4_SCL";

    // B10 引脚功能定义 Pin name: VIVO_D5
    // 引脚名称使用实际的PAD名称
    m_pinFunctions["VIVO_D5"] = QStringList() << "VI2_D_5" << "VI1_D_5" << "VO_D_18" 
                                        << "XGPIOB_16" << "RMII0_RXD0" << "SPI3_CS_X" 
                                        << "UART2_RX" << "CAM_HS0";
    m_defaultFunctions["VIVO_D5"] = "XGPIOB_16";  // 默认功能 (function select 3)
    // 功能宏映射
    m_functionMacros["VI2_D_5"] = "VI2_D_5";
    m_functionMacros["VI1_D_5"] = "VI1_D_5";
    m_functionMacros["VO_D_18"] = "VO_D_18";
    m_functionMacros["XGPIOB_16"] = "XGPIOB_16";
    m_functionMacros["RMII0_RXD0"] = "RMII0_RXD0";
    m_functionMacros["SPI3_CS_X"] = "SPI3_CS_X";
    m_functionMacros["UART2_RX"] = "UART2_RX";
    m_functionMacros["CAM_HS0"] = "CAM_HS0";

    // B11 引脚功能定义 Pin name: VIVO_D7
    // 引脚名称使用实际的PAD名称
    m_pinFunctions["VIVO_D7"] = QStringList() << "VI2_D_7" << "VI1_D_7" << "VO_D_20" 
                                        << "XGPIOB_14" << "RMII0_RXD1" << "SPI3_SDI" 
                                        << "IIC2_SDA" << "CAM_HS0";
    m_defaultFunctions["VIVO_D7"] = "XGPIOB_14";  // 默认功能 (function select 3)
    // 功能宏映射
    m_functionMacros["VI2_D_7"] = "VI2_D_7";
    m_functionMacros["VI1_D_7"] = "VI1_D_7";
    m_functionMacros["VO_D_20"] = "VO_D_20";
    m_functionMacros["XGPIOB_14"] = "XGPIOB_14";
    m_functionMacros["RMII0_RXD1"] = "RMII0_RXD1";
    m_functionMacros["SPI3_SDI"] = "SPI3_SDI";
    m_functionMacros["IIC2_SDA"] = "IIC2_SDA";
    m_functionMacros["CAM_HS0"] = "CAM_HS0";

    // B12 引脚功能定义 Pin name: VIVO_D9
    // 引脚名称使用实际的PAD名称
    m_pinFunctions["VIVO_D9"] = QStringList() << "PWM_2" << "VI1_D_9" << "VO_D_22" 
                                        << "XGPIOB_12" << "CAM_MCLK1" << "IIC1_SCL" 
                                        << "UART2_RX";
    m_defaultFunctions["VIVO_D9"] = "XGPIOB_12";  // 默认功能 (function select 3)
    // 功能宏映射
    m_functionMacros["PWM_2"] = "PWM_2";
    m_functionMacros["VI1_D_9"] = "VI1_D_9";
    m_functionMacros["VO_D_22"] = "VO_D_22";
    m_functionMacros["XGPIOB_12"] = "XGPIOB_12";
    m_functionMacros["CAM_MCLK1"] = "CAM_MCLK1";
    m_functionMacros["IIC1_SCL"] = "IIC1_SCL";
    m_functionMacros["UART2_RX"] = "UART2_RX";


    // B13 引脚功能定义 Pin name: USB_ID
    m_pinFunctions["USB_ID"] = QStringList() << "USB_ID" << "XGPIOB_4" ;
    m_defaultFunctions["USB_ID"] = "USB_ID";
    m_functionMacros["USB_ID"] = "USB_ID";
    m_functionMacros["XGPIOB_4"] = "XGPIOB_4";

    // B15 引脚功能定义 Pin name: PAD_ETH_RXM___EPHY_TXP
    m_pinFunctions["PAD_ETH_RXM___EPHY_TXP"] = QStringList()<< "UART3_CTS" << "CAM_MCLK0" << "XGPIOB_26" 
                                                 << "PWM_14"<< "CAM_VS0" << "SPI1_CS_X" << "IIS2_DI";
    m_defaultFunctions["PAD_ETH_RXM___EPHY_TXP"] = "XGPIOB_26";
    m_functionMacros["UART3_CTS"] = "UART3_CTS";
    m_functionMacros["CAM_MCLK0"] = "CAM_MCLK0";
    m_functionMacros["XGPIOB_26"] = "XGPIOB_26";
    m_functionMacros["PWM_14"] = "PWM_14";
    m_functionMacros["CAM_VS0"] = "CAM_VS0";
    m_functionMacros["SPI1_CS_X"] = "SPI1_CS_X";
    m_functionMacros["IIS2_DI"] = "IIS2_DI";


    // C1 引脚功能定义 Pin name: PAD_MIPI_TXP2
    m_pinFunctions["PAD_MIPI_TXP2"] = QStringList() << "CV_4WTDI_CR_SCL0" << "VI0_D_14" << "VO_CLK0" 
                                         << "XGPIOC_17" << "IIC1_SCL" << "PWM_9" << "SPI0_CS_X" 
                                         << "SD1_D3";
    m_defaultFunctions["PAD_MIPI_TXP2"] = "XGPIOC_17";
    m_functionMacros["CV_4WTDI_CR_SCL0"] = "CV_4WTDI_CR_SCL0";
    m_functionMacros["VI0_D_14"] = "VI0_D_14";
    m_functionMacros["VO_CLK0"] = "VO_CLK0";
    m_functionMacros["XGPIOC_17"] = "XGPIOC_17";
    m_functionMacros["IIC1_SCL"] = "IIC1_SCL";
    m_functionMacros["PWM_9"] = "PWM_9";
    m_functionMacros["SPI0_CS_X"] = "SPI0_CS_X";
    m_functionMacros["SD1_D3"] = "SD1_D3";

    // C2 引脚功能定义 Pin name: PAD_MIPI_TXM2
    m_pinFunctions["PAD_MIPI_TXM2"] = QStringList() << "CV_4WTMS_CR_SDA0" << "VI0_D_13" << "VO_D_0" 
                                         << "XGPIOC_16" << "IIC1_SDA" << "PWM_8" << "SPI0_SCK" 
                                         << "SD1_D2";
    m_defaultFunctions["PAD_MIPI_TXM2"] = "XGPIOC_16";
    m_functionMacros["CV_4WTMS_CR_SDA0"] = "CV_4WTMS_CR_SDA0";
    m_functionMacros["VI0_D_13"] = "VI0_D_13";
    m_functionMacros["VO_D_0"] = "VO_D_0";
    m_functionMacros["XGPIOC_16"] = "XGPIOC_16";
    m_functionMacros["IIC1_SDA"] = "IIC1_SDA";
    m_functionMacros["PWM_8"] = "PWM_8";
    m_functionMacros["SPI0_SCK"] = "SPI0_SCK";
    m_functionMacros["SD1_D2"] = "SD1_D2";

    
    // C3 引脚功能定义 Pin name: CAM_PD0
    m_pinFunctions["CAM_PD0"] = QStringList() << "IIS1_MCLK" << "XGPIOA_1" << "CAM_HS0" ;
    m_defaultFunctions["CAM_PD0"] = "XGPIOA_1";
    m_functionMacros["IIS1_MCLK"] = "IIS1_MCLK";
    m_functionMacros["XGPIOA_1"] = "XGPIOA_1";
    m_functionMacros["CAM_HS0"] = "CAM_HS0";

    
    // C4 引脚功能定义 Pin name: CAM_MCLK0
    m_pinFunctions["CAM_MCLK0"] = QStringList() << "CAM_MCLK0" << "AUX1" << "XGPIOA_0" ;
    m_defaultFunctions["CAM_MCLK0"] = "XGPIOA_0";
    m_functionMacros["CAM_MCLK0"] = "CAM_MCLK0";
    m_functionMacros["AUX1"] = "AUX1";
    m_functionMacros["XGPIOA_0"] = "XGPIOA_0";

    // C5 引脚功能定义 Pin name: PAD_MIPIRX1P
    m_pinFunctions["PAD_MIPIRX1P"] = QStringList() << "VI0_D_6" << "VO_D_7" << "XGPIOC_9" 
                                         << "IIC1_SDA" << "KEY_ROW2" << "  DBG_9";
    m_defaultFunctions["PAD_MIPIRX1P"] = "XGPIOC_9";
    m_functionMacros["VI0_D_6"] = "VI0_D_6";
    m_functionMacros["VO_D_7"] = "VO_D_7";
    m_functionMacros["XGPIOC_9"] = "XGPIOC_9";
    m_functionMacros["IIC1_SDA"] = "IIC1_SDA";
    m_functionMacros["KEY_ROW2"] = "KEY_ROW2";
    m_functionMacros["DBG_9"] = "DBG_9";

    // C6 引脚功能定义 Pin name: PAD_MIPIRX2P
    m_pinFunctions["PAD_MIPIRX2P"] = QStringList() << "VI0_D_4" << "VO_D_9" << "XGPIOC_7" 
                                         << "VI1_D_18" << "IIC4_SDA" << "DBG_7";
    m_defaultFunctions["PAD_MIPIRX2P"] = "XGPIOC_7";
    m_functionMacros["VI0_D_4"] = "VI0_D_4";
    m_functionMacros["VO_D_9"] = "VO_D_9";
    m_functionMacros["XGPIOC_7"] = "XGPIOC_7";
    m_functionMacros["VI1_D_18"] = "VI1_D_18";
    m_functionMacros["IIC4_SDA"] = "IIC4_SDA";
    m_functionMacros["DBG_7"] = "DBG_7";

    // C7 引脚功能定义 Pin name: PAD_MIPIRX3N
    m_pinFunctions["PAD_MIPIRX3N"] = QStringList() << "CV_4WTDO_CR_2WTMS" << "VI0_D_1" << "VI1_D_15" 
                                      << "XGPIOC_4" << "CAM_MCLK0" << "MUX_SPI1_MISO";
    m_defaultFunctions["PAD_MIPIRX3N"] = "XGPIOC_4";
    m_functionMacros["CV_4WTDO_CR_2WTMS"] = "CV_4WTDO_CR_2WTMS";
    m_functionMacros["VI0_D_1"] = "VI0_D_1";
    m_functionMacros["VI1_D_15"] = "VI1_D_15";
    m_functionMacros["XGPIOC_4"] = "XGPIOC_4";
    m_functionMacros["CAM_MCLK0"] = "CAM_MCLK0";
    m_functionMacros["MUX_SPI1_MISO"] = "MUX_SPI1_MISO";

    // C8 引脚功能定义 Pin name: PAD_MIPIRX5P
    m_pinFunctions["PAD_MIPIRX5P"] = QStringList() << "VI1_D_12" << "VO_D_11" << "XGPIOC_1"
                                      << "IIS1_MCLK" << "CAM_MCLK1" << "WG0_D1" << "DBG_1" ;
    m_defaultFunctions["PAD_MIPIRX5P"] = "XGPIOC_1";
    m_functionMacros["VI1_D_12"] = "VI1_D_12";
    m_functionMacros["VO_D_11"] = "VO_D_11";
    m_functionMacros["XGPIOC_1"] = "XGPIOC_1";
    m_functionMacros["IIS1_MCLK"] = "IIS1_MCLK";
    m_functionMacros["CAM_MCLK1"] = "CAM_MCLK1";
    m_functionMacros["WG0_D1"] = "WG0_D1";
    m_functionMacros["DBG_1"] = "DBG_1";

    // C9 引脚功能定义 Pin name: VIVO_CLK
    m_pinFunctions["VIVO_CLK"] = QStringList() << "VI2_CLK" << "VI1_CLK" << "VO_CLK1"
                                 << "XGPIOB_22" << "RMII0_TXEN" << "CAM_MCLK0"
                                  << "WG1_D1" << "UART2_RX";
    m_defaultFunctions["VIVO_CLK"] = "XGPIOB_22";
    m_functionMacros["VI2_CLK"] = "VI2_CLK";
    m_functionMacros["VI1_CLK"] = "VI1_CLK";
    m_functionMacros["VO_CLK1"] = "VO_CLK1";
    m_functionMacros["XGPIOB_22"] = "XGPIOB_22";
    m_functionMacros["RMII0_TXEN"] = "RMII0_TXEN";
    m_functionMacros["CAM_MCLK0"] = "CAM_MCLK0";
    m_functionMacros["WG1_D1"] = "WG1_D1";
    m_functionMacros["UART2_RX"] = "UART2_RX";    

    // C10 引脚功能定义 Pin name: VIVO_D6
    m_pinFunctions["VIVO_D6"] = QStringList() << " VI2_D_6" << "VI1_D_6" << "VO_D_19" 
                                   << "XGPIOB_15" << "RMII0_REFCLKI" << "SPI3_SCK"
                                   << "UART2_TX" << "CAM_VS0";
    m_defaultFunctions["VIVO_D6"] = "XGPIOB_15";
    m_functionMacros["VI2_D_6"] = "VI2_D_6";
    m_functionMacros["VI1_D_6"] = "VI1_D_6";
    m_functionMacros["VO_D_19"] = "VO_D_19";
    m_functionMacros["XGPIOB_15"] = "XGPIOB_15";
    m_functionMacros["RMII0_REFCLKI"] = "RMII0_REFCLKI";
    m_functionMacros["SPI3_SCK"] = "SPI3_SCK";
    m_functionMacros["UART2_TX"] = "UART2_TX";
    m_functionMacros["CAM_VS0"] = "CAM_VS0";

    // C11 引脚功能定义 Pin name: VIVO_D8
    m_pinFunctions["VIVO_D8"] = QStringList() << "PWM_3" << "VI1_D_8" << "VO_D_21" 
                                   << "XGPIOB_13" << "RMII0_MDIO" << "SPI3_SDO"
                                   << "IIC2_SCL" << "CAM_VS0";
    m_defaultFunctions["VIVO_D8"] = "XGPIOB_13";
    m_functionMacros["PWM_3"] = "PWM_3";
    m_functionMacros["VI1_D_8"] = "VI1_D_8";
    m_functionMacros["VO_D_21"] = "VO_D_21";
    m_functionMacros["XGPIOB_13"] = "XGPIOB_13";
    m_functionMacros["RMII0_MDIO"] = "RMII0_MDIO";
    m_functionMacros["SPI3_SDO"] = "SPI3_SDO";
    m_functionMacros["IIC2_SCL"] = "IIC2_SCL";
    m_functionMacros["CAM_VS0"] = "CAM_VS0";

    // C12 引脚功能定义 Pin name: USB_VBUS_EN
    m_pinFunctions["USB_VBUS_EN"] = QStringList() << "USB_VBUS_EN" << "XGPIOB_5";
    m_defaultFunctions["USB_VBUS_EN"] = "USB_VBUS_EN";
    m_functionMacros["USB_VBUS_EN"] = "USB_VBUS_EN";
    m_functionMacros["XGPIOB_5"] = "XGPIOB_5";

    // C14 引脚功能定义 Pin name: PAD_ETH_RXP___EPHY_TXN
    m_pinFunctions["PAD_ETH_RXP___EPHY_TXN"] = QStringList() << "UART3_TX" << "CAM_MCLK1" << "XGPIOB_27"
                                                  << "PWM_15" << "SPI1_SCK" << "IIS2_DO";
    m_defaultFunctions["PAD_ETH_RXP___EPHY_TXN"] = "XGPIOB_27";
    m_functionMacros["USB_VBUS_EN"] = "USB_VBUS_EN";
    m_functionMacros["UART3_TX"] = "UART3_TX";
    m_functionMacros["CAM_MCLK1"] = "CAM_MCLK1";
    m_functionMacros["XGPIOB_27"] = "XGPIOB_27";
    m_functionMacros["PWM_15"] = "PWM_15";
    m_functionMacros["SPI1_SCK"] = "SPI1_SCK";
    m_functionMacros["IIS2_DO"] = "IIS2_DO";

    // C15 引脚功能定义 Pin name: GPIO_RTX___EPHY_RTX
    m_pinFunctions["GPIO_RTX___EPHY_RTX"] = QStringList() << "XGPIOB_23" << "PWM_1" << "CAM_MCLK0";
    m_defaultFunctions["GPIO_RTX___EPHY_RTX"] = "XGPIOB_23";
    m_functionMacros["XGPIOB_23"] = "XGPIOB_23";
    m_functionMacros["PWM_1"] = "PWM_1";
    m_functionMacros["CAM_MCLK0"] = "CAM_MCLK0";

    // D1 引脚功能定义 Pin name: PAD_MIPI_TXP1
    m_pinFunctions["PAD_MIPI_TXP1"] = QStringList() << "CV_4WTCK_CR_2WTCK" << "VI0_D_12" << "VO_D_1"
                                    << "XGPIOC_15" << "IIC2_SCL" << "PWM_11"
                                    << "SPI0_SDI" << "DBG_15";
    m_defaultFunctions["PAD_MIPI_TXP1"] = "XGPIOC_15";  // 默认功能 (function select 3)
    m_functionMacros["CV_4WTCK_CR_2WTCK"] = "CV_4WTCK_CR_2WTCK";
    m_functionMacros["VI0_D_12"] = "VI0_D_12";
    m_functionMacros["VO_D_1"] = "VO_D_1";
    m_functionMacros["XGPIOC_15"] = "XGPIOC_15";
    m_functionMacros["IIC2_SCL"] = "IIC2_SCL";
    m_functionMacros["PWM_11"] = "PWM_11";
    m_functionMacros["SPI0_SDI"] = "SPI0_SDI";
    m_functionMacros["DBG_15"] = "DBG_15";

    // D3 引脚功能定义 Pin name: CAM_MCLK1
    m_pinFunctions["CAM_MCLK1"] = QStringList() << "CAM_MCLK1 " << "AUX2" << "XGPIOA_3" 
                                         << "CAM_HS0" ;
    m_defaultFunctions["CAM_MCLK1"] = "XGPIOA_3";
    m_functionMacros["AUX2"] ="AUX2";
    m_functionMacros["XGPIOA_3"] = "XGPIOA_3";
    m_functionMacros["CAM_HS0"] = "CAM_HS0";


    // D4 引脚功能定义 Pin name: IIC3_SCL
    m_pinFunctions["IIC3_SCL"] = QStringList() << "IIC3_SCL" << "XGPIOA_5";
    m_defaultFunctions["IIC3_SCL"] = "XGPIOA_5";  // 默认功能 (function select 3)
    m_functionMacros["IIC3_SCL"] = "IIC3_SCL";
    m_functionMacros["XGPIOA_5"] = "XGPIOA_5";

    // D10 引脚功能定义 Pin name: VIVO_D4
    m_pinFunctions["VIVO_D4"] = QStringList() << "VI2_D_4" << "VI1_D_4" << "VO_D_17"
                                    << "XGPIOB_17" << "RMII0_MDC" << "IIC1_SDA"
                                    << "UART2_CTS" << "CAM_VS0";
    m_defaultFunctions["VIVO_D4"] = "XGPIOB_17";  // 默认功能 (function select 3)
    m_functionMacros["VI2_D_4"] = "VI2_D_4";
    m_functionMacros["VI1_D_4"] = "VI1_D_4";
    m_functionMacros["VO_D_17"] = "VO_D_17";
    m_functionMacros["XGPIOB_17"] = "XGPIOB_17";
    m_functionMacros["RMII0_MDC"] = "RMII0_MDC";
    m_functionMacros["IIC1_SDA"] = "IIC1_SDA";
    m_functionMacros["UART2_CTS"] = "UART2_CTS";
    m_functionMacros["CAM_VS0"] = "CAM_VS0";

    // D14 引脚功能定义 Pin name: PAD_ETH_TXM___EPHY_RXP
    m_pinFunctions["PAD_ETH_TXM___EPHY_RXP"] = QStringList() << "UART3_RTS" << "IIC1_SDA" << "XGPIOB_24" 
                                                     << "PWM_12" << "CAM_MCLK1" << "SPI1_SDI" << "IIS2_BCLK";
    m_defaultFunctions["PAD_ETH_TXM___EPHY_RXP"] = "XGPIOB_24";  // 默认功能 (function select 3)
    m_functionMacros["UART3_RTS"] = "UART3_RTS";
    m_functionMacros["IIC1_SDA"] = "IIC1_SDA";
    m_functionMacros["XGPIOB_24"] = "XGPIOB_24";
    m_functionMacros["PWM_12"] = "PWM_12";
    m_functionMacros["CAM_MCLK1"] = "CAM_MCLK1";
    m_functionMacros["SPI1_SDI"] = "SPI1_SDI";
    m_functionMacros["IIS2_BCLK"] = "IIS2_BCLK";

    // D15 引脚功能定义 Pin name: PAD_ETH_TXP___EPHY_RXN
    m_pinFunctions["PAD_ETH_TXP___EPHY_RXN"] = QStringList() << "UART3_RX" << "IIC1_SCL"<< "XGPIOB_25"
                                                      << "WM_1P3"<< "CAM_MCLK0"<< "SPI1_SDO"<< "IIS2_LRCK";
    m_defaultFunctions["PAD_ETH_TXP___EPHY_RXN"] = "XGPIOB_25";  // 默认功能 (function select 3)
    m_functionMacros["UART3_RX"] = "UART3_RX";
    m_functionMacros["IIC1_SCL"] = "IIC1_SCL";
    m_functionMacros["WM_1P3"] = "WM_1P3";
    m_functionMacros["CAM_MCLK0"] = "CAM_MCLK0";
    m_functionMacros["SPI1_SDO"] = "SPI1_SDO";
    m_functionMacros["IIS2_LRCK"] = "IIS2_LRCK";

    // E1 引脚功能定义 Pin name: PAD_MIPI_TXP0
    m_pinFunctions["PAD_MIPI_TXP0"] = QStringList() << "VI0_D_10" << "VO_D_3"<<"XGPIOC_13" 
                                    << "CAM_MCLK0" << "PWM_15"<<"CAM_HS0" 
                                    << "DBG_13";
    m_defaultFunctions["PAD_MIPI_TXP0"] = "XGPIOC_13";  // 默认功能 (function select 3)
    m_functionMacros["VI0_D_10"] = "VI0_D_10";
    m_functionMacros["VO_D_3"] = "VO_D_3";
    m_functionMacros["XGPIOC_13"] = "XGPIOC_13";
    m_functionMacros["CAM_MCLK0"] = "CAM_MCLK0";
    m_functionMacros["PWM_15"] = "PWM_15";
    m_functionMacros["CAM_HS0"] = "CAM_HS0";
    m_functionMacros["DBG_13"] = "DBG_13";

    // E2 引脚功能定义 Pin name: PAD_MIPI_TXM0
    m_pinFunctions["PAD_MIPI_TXM0"] = QStringList() << "VI0_D_9" << "VO_D_4"<<"XGPIOC_12" 
                                    << "CAM_MCLK1" << "PWM_14"<<"CAM_VS0" 
                                    << "DBG_12";
    m_defaultFunctions["PAD_MIPI_TXM0"] = "XGPIOC_12";  // 默认功能 (function select 3)
    m_functionMacros["VI0_D_9"] = "VI0_D_9";
    m_functionMacros["VO_D_4"] = "VO_D_4";
    m_functionMacros["XGPIOC_12"] = "XGPIOC_12";
    m_functionMacros["CAM_MCLK1"] = "CAM_MCLK1";
    m_functionMacros["PWM_14"] = "PWM_14";
    m_functionMacros["CAM_VS0"] = "CAM_VS0";
    m_functionMacros["DBG_12"] = "DBG_12";

    // E4 引脚功能定义 Pin name: CAM_PD1
    m_pinFunctions["CAM_PD1"] = QStringList() << "CAM_PD1" << "XGPIOA_4" << "CAM_VS0" 
                                       << "IIC4_SDA";
    m_defaultFunctions["CAM_PD1"] = "XGPIOA_4";  // 默认功能 (function select 3)
    m_functionMacros["XGPIOA_4"] = "XGPIOA_4";
    m_functionMacros["CAM_VS0"] = "CAM_VS0";
    m_functionMacros["IIC4_SDA"] = "IIC4_SDA";

    // E5 引脚功能定义 Pin name: CAM_RST0
    m_pinFunctions["CAM_RST0"] = QStringList() << "XGPIOA_2" << "CAM_VS0"<<"IIC4_SCL";
    m_defaultFunctions["CAM_RST0"] = "XGPIOA_2";  // 默认功能 (function select 3)
    m_functionMacros["XGPIOA_2"] = "XGPIOA_2";
    m_functionMacros["CAM_VS0"] = "CAM_VS0";
    m_functionMacros["IIC4_SCL"] = "IIC4_SCL";

    // E10 引脚功能定义 Pin name: VIVO_D0
    m_pinFunctions["VIVO_D0"] = QStringList() << "VI2_D_0" << "VI1_D_0"<<"VO_D_13" 
                                    << "XGPIOB_21" << "RMII0_TXCLK"<<"IIC3_SCL" 
                                    << "WG1_D0"<< "IIC4_SDA";
    m_defaultFunctions["VIVO_D0"] = "XGPIOB_21";  // 默认功能 (function select 3)
    m_functionMacros["VI2_D_0"] = "VI2_D_0";
    m_functionMacros["VI1_D_0"] = "VI1_D_0";
    m_functionMacros["VO_D_13"] = "VO_D_13";
    m_functionMacros["XGPIOB_21"] = "XGPIOB_21";
    m_functionMacros["RMII0_TXCLK"] = "RMII0_TXCLK";
    m_functionMacros["IIC3_SCL"] = "IIC3_SCL";
    m_functionMacros["WG1_D0"] = "WG1_D0";
    m_functionMacros["IIC4_SDA"] = "IIC4_SDA";

    // E13 引脚功能定义 Pin name: ADC1
    m_pinFunctions["ADC1"] = QStringList() << "XGPIOB_3" << "KEY_COL2";
    m_defaultFunctions["ADC1"] = "XGPIOB_3";  // 默认功能 (function select 3)
    m_functionMacros["XGPIOB_3"] = "XGPIOB_3";
    m_functionMacros["KEY_COL2"] = "KEY_COL2";

    // E14 引脚功能定义 Pin name: ADC2
    m_pinFunctions["ADC2"] = QStringList() << "CAM_MCLK1" << "IIC4_SDA"<<"XGPIOB_2" 
                                    << "PWM_13" << "EPHY_SPD_LED"<<"WG2_D1" 
                                    << "UART3_RX";
    m_defaultFunctions["ADC2"] = "XGPIOB_2";  // 默认功能 (function select 3)
    m_functionMacros["CAM_MCLK1"] = "CAM_MCLK1";
    m_functionMacros["IIC4_SDA"] = "IIC4_SDA";
    m_functionMacros["XGPIOB_2"] = "XGPIOB_2";
    m_functionMacros["PWM_13"] = "PWM_13";
    m_functionMacros["EPHY_SPD_LED"] = "EPHY_SPD_LED";
    m_functionMacros["WG2_D1"] = "WG2_D1";

    // E15 引脚功能定义 Pin name: ADC3
    m_pinFunctions["ADC3"] = QStringList() << "CAM_MCLK0" << "IIC4_SCL"<<" XGPIOB_1" 
                                    << "PWM_12" << "EPHY_LNK_LED"<<"WG2_D0" 
                                    << "UART3_TX";
    m_defaultFunctions["ADC3"] = "XGPIOB_1";  // 默认功能 (function select 3)
    m_functionMacros["CAM_MCLK0"] = "CAM_MCLK0";
    m_functionMacros["CAM_VS0"] = "CAM_VS0";
    m_functionMacros["IIC4_SCL"] = "IIC4_SCL";
    m_functionMacros["XGPIOB_1"] = "XGPIOB_1";
    m_functionMacros["PWM_12"] = "PWM_12";
    m_functionMacros["EPHY_LNK_LED"] = "EPHY_LNK_LED";
    m_functionMacros["WG2_D0"] = "WG2_D0";
    m_functionMacros["UART3_TX"] = "UART3_TX";

    // F2 引脚功能定义 Pin name: PAD_AUD_AOUTL
    m_pinFunctions["PAD_AUD_AOUTL"] = QStringList() << "XGPIOC_25 " << "IIS1_LRCK " << "IIS2_LRCK";
    m_defaultFunctions["PAD_AUD_AOUTL"] = "XGPIOC_25";  // 默认功能 (function select 3)
    m_functionMacros["XGPIOC_25"] = "XGPIOC_25";
    m_functionMacros["IIS1_LRCK"] = "IIS1_LRCK";
    m_functionMacros["IIS2_LRCK"] = "IIS2_LRCK";

    // F4 引脚功能定义 Pin name: IIC3_SDA
    m_pinFunctions["IIC3_SDA"] = QStringList() << "IIC3_SDA " << "XGPIOA_6 ";
    m_defaultFunctions["IIC3_SDA"] = "XGPIOA_6";  // 默认功能 (function select 3)
    m_functionMacros["XGPIOA_6"] = "XGPIOA_6";

    // F14 引脚功能定义 Pin name: SD1_D2
    m_pinFunctions["SD1_D2"] = QStringList() << "PWR_SD1_D2_VO33" << "IIC1_SCL"<< "UART2_TX"
                                    << "PWR_GPIO_19"<< "CAM_MCLK0"<< "UART3_TX"
                                    << "PWR_SPINOR1_HOLD_X"<< "PWM_5";
    m_defaultFunctions["SD1_D2"] = "PWR_SPINOR1_HOLD_X";  // 默认功能 (function select 6)
    m_functionMacros["PWR_SD1_D2_VO33"] = "PWR_SD1_D2_VO33";
    m_functionMacros["IIC1_SCL"] = "IIC1_SCL";
    m_functionMacros["UART2_TX"] = "UART2_TX";
    m_functionMacros["PWR_GPIO_19"] = "PWR_GPIO_19";
    m_functionMacros["CAM_MCLK0"] = "CAM_MCLK0";
    m_functionMacros["UART3_TX"] = "UART3_TX";
    m_functionMacros["PWR_SPINOR1_HOLD_X"] = "PWR_SPINOR1_HOLD_X";
    m_functionMacros["PWM_56"] = "PWM_5";

    // G2 引脚功能定义 Pin name: PAD_AUD_AOUTR
    m_pinFunctions["PAD_AUD_AOUTR"] = QStringList() << "XGPIOC_24" << "IIS1_DI"<< "IIS2_DO"
                                                    << "IIS1_DO";
    m_defaultFunctions["PAD_AUD_AOUTR"] = "XGPIOC_24";  // 默认功能 (function select 3)
    m_functionMacros["XGPIOC_24"] = "XGPIOC_24";
    m_functionMacros["IIS1_DI"] = "IIS1_DI";
    m_functionMacros["IIS2_DO"] = "IIS2_DO";
    m_functionMacros["IIS1_DO"] = "IIS1_DO";

    // G13 引脚功能定义 Pin name: SD1_D3
    m_pinFunctions["SD1_D3"] = QStringList() << "PWR_SD1_D3_VO32" << "SPI2_CS_X"<< "IIC1_SCL"
                                      << "PWR_GPIO_18"<< "CAM_MCLK0"<< "UART3_CTS"
                                      << "PWR_SPINOR1_CS_X"<< "PWM_4";
    m_defaultFunctions["SD1_D3"] = "PWR_SPINOR1_CS_X";  // 默认功能 (function select 3)
    m_functionMacros["PWR_SD1_D3_VO32"] = "PWR_SD1_D3_VO32";
    m_functionMacros["SPI2_CS_X"] = "SPI2_CS_X";
    m_functionMacros["IIC1_SCL"] = "IIC1_SCL";
    m_functionMacros["PWR_GPIO_18"] = "PWR_GPIO_18";
    m_functionMacros["CAM_MCLK0"] = "CAM_MCLK0";
    m_functionMacros["UART3_CTS"] = "UART3_CTS";
    m_functionMacros["PWR_SPINOR1_CS_X"] = "PWR_SPINOR1_CS_X";
    m_functionMacros["PWM_4"] = "PWM_4";

    // G14 引脚功能定义 Pin name: SD1_CLK
    m_pinFunctions["SD1_CLK"] = QStringList() << "PWR_SD1_CLK_VO37" << "SPI2_SCK"<< "IIC3_SDA"
                                       << "PWR_SPINGPIO_23"<< "CAM_HS0"<< "EPHY_SPD_LED"
                                       << "PWR_SPINOR1_SCK"<< "PWM_9";
    m_defaultFunctions["SD1_CLK"] = "PWR_SPINOR1_SCK";  // 默认功能 (function select 3)
    m_functionMacros["PWR_SD1_CLK_VO37"] = "PWR_SD1_CLK_VO37";
    m_functionMacros["SPI2_SCK"] = "SPI2_SCK";
    m_functionMacros["IIC3_SDA"] = "IIC3_SDA";
    m_functionMacros["PWR_GPIO_23"] = "PWR_GPIO_23";
    m_functionMacros["CAM_HS0"] = "CAM_HS0";
    m_functionMacros["EPHY_SPD_LED"] = "EPHY_SPD_LED";
    m_functionMacros["PWR_SPINOR1_SCK"] = "PWR_SPINOR1_SCK";
    m_functionMacros["PWM_9"] = "PWM_9";

    // G15 引脚功能定义 Pin name: SD1_CMD
    m_pinFunctions["SD1_CMD"] = QStringList() << "PWR_SD1_CMD_VO36" << "SPI2_SDO"<< "IIC3_SCL"
                                       << "PWR_GPIO_22"<< "CAM_VS0"<< "EPHY_LNK_LED"
                                       << "PWR_SPINOR1_MOSI"<< "PWM_8";
    m_defaultFunctions["SD1_CMD"] = "PWR_SPINOR1_MOSI";  // 默认功能 (function select 3)
    m_functionMacros["PWR_SD1_CMD_VO36"] = "PWR_SD1_CMD_VO36";
    m_functionMacros["SPI2_SDO"] = "SPI2_SDO";
    m_functionMacros["IIC3_SCL"] = "IIC3_SCL";
    m_functionMacros["PWR_GPIO_22"] = "PWR_GPIO_22";
    m_functionMacros["CAM_VS0"] = "CAM_VS0";
    m_functionMacros["EPHY_LNK_LED"] = "EPHY_LNK_LED";
    m_functionMacros["PWR_SPINOR1_MOSI"] = "PWR_SPINOR1_MOSI";
    m_functionMacros["PWM_8"] = "PWM_8";

    // H1 引脚功能定义 Pin name: PAD_AUD_AINL_MIC
    m_pinFunctions["PAD_AUD_AINL_MIC"] = QStringList() << "XGPIOC_23"  << "IIS1_BCLK" << "IIS2_BCLK";
    m_defaultFunctions["PAD_AUD_AINL_MIC"] = "XGPIOC_23";  // 默认功能 (function select 3)
    m_functionMacros["XGPIOC_23"] = "XGPIOC_23";
    m_functionMacros["IIS1_BCLK"] = "IIS1_BCLK";
    m_functionMacros["IIS2_BCLK"] = "IIS2_BCLK";

    // H12 引脚功能定义 Pin name: RSTN
    m_pinFunctions["RSTN"] = QStringList() << "RSTN" ;
    m_defaultFunctions["RSTN"] = "RSTN";  // 默认功能 (function select 3)
    m_functionMacros["RSTN"] = "RSTN";

     // H13 引脚功能定义 Pin name: PWM0_BUCK
    m_pinFunctions["PWM0_BUCK"] = QStringList() << "PWM_0"<< "XGPIOB_0";
    m_defaultFunctions["PWM0_BUCK"] = "XGPIOB_0";  // 默认功能 (function select 3)
    m_functionMacros["PWM_0"] = "PWM_0";
    m_functionMacros["XGPIOB_0"] = "XGPIOB_0";

    // H14 引脚功能定义 Pin name: SD1_D1
    m_pinFunctions["SD1_D1"] = QStringList() << "PWR_SD1_D1_VO34" << "IIC1_SDA"<< "UART2_RX"
                                      << "PWR_GPIO_20"<< "CAM_MCLK1"<< "UART3_RX"
                                      << "PWR_SPINOR1_WP_X"<< "PWM_6";
    m_defaultFunctions["SD1_D1"] = "PWR_SPINOR1_WP_X";  // 默认功能 (function select 3)
    m_functionMacros["PWR_SD1_D1_VO34"] = "PWR_SD1_D1_VO34";
    m_functionMacros["IIC1_SDA"] = "IIC1_SDA";
    m_functionMacros["UART2_RX"] = "UART2_RX";
    m_functionMacros["PWR_GPIO_20"] = "PWR_GPIO_20";
    m_functionMacros["CAM_MCLK1"] = "CAM_MCLK1";
    m_functionMacros["UART3_RX"] = "UART3_RX";
    m_functionMacros["PWR_SPINOR1_WP_X"] = "PWR_SPINOR1_WP_X";
    m_functionMacros["PWM_6"] = "PWM_6";

    // H15 引脚功能定义 Pin name: SD1_D0
    m_pinFunctions["SD1_D0"] = QStringList() << "PWR_SD1_D0_VO35" << "SPI2_SDI"<< "IIC1_SDA"
                                      << "PWR_GPIO_21"<< "CAM_MCLK1"<< "UART3_RTS"
                                      << "PWR_SPINOR1_MISO"<< "PWM_7";
    m_defaultFunctions["SD1_D0"] = "PWR_SPINOR1_MISO";  // 默认功能 (function select 3)
    m_functionMacros["PWR_SD1_D0_VO35"] = "PWR_SD1_D0_VO35";
    m_functionMacros["SPI2_SDI"] = "SPI2_SDI";
    m_functionMacros["IIC1_SDA"] = "IIC1_SDA";
    m_functionMacros["PWR_GPIO_21"] = "PWR_GPIO_21";
    m_functionMacros["CAM_MCLK1"] = "CAM_MCLK1";
    m_functionMacros["UART3_RTS"] = "UART3_RTS";
    m_functionMacros["PWR_SPINOR1_MISO"] = "PWR_SPINOR1_MISO";
    m_functionMacros["PWM_7"] = "PWM_7";

    // J1 引脚功能定义 Pin name: PAD_AUD_AINR_MIC
    m_pinFunctions["PAD_AUD_AINR_MIC"] = QStringList() << "XGPIOC_22 " << "IIS1_DO "<< "IIS2_DI "
                                               << "IIS1_DI";
    m_defaultFunctions["PAD_AUD_AINR_MIC"] = "XGPIOC_22";  // 默认功能 (function select 3)
    m_functionMacros["XGPIOC_22"] = "XGPIOC_22";
    m_functionMacros["IIS1_DO"] = "IIS1_DO";
    m_functionMacros["IIS2_DI"] = "IIS2_DI";
    m_functionMacros["IIS1_DI"] = "IIS1_DI";
    
    // J13 引脚功能定义 Pin name: IIC2_SCL
    m_pinFunctions["IIC2_SCL"] = QStringList() << "IIC2_SCL " << "PWM_14 "<< "PWR_GPIO_12 "
                                        << "UART2_RX"<< "KEY_COL2";
    m_defaultFunctions["IIC2_SCL"] = "PWR_GPIO_12";  // 默认功能 (function select 3)
    m_functionMacros["IIC2_SCL"] = "IIC2_SCL";
    m_functionMacros["PWM_14"] = "PWM_14";
    m_functionMacros["PWR_GPIO_12"] = "PWR_GPIO_12";
    m_functionMacros["UART2_RX"] = "UART2_RX";
    m_functionMacros["KEY_COL2"] = "KEY_COL2";

     // J14 引脚功能定义 Pin name: IIC2_SDA
    m_pinFunctions["IIC2_SDA"] = QStringList() << "IIC2_SDA " << "PWM_15 "<< "PWR_GPIO_13 "
                                        << "UART2_TX"<< "IIS1_MCLK "<< "IIS2_MCLK "<< "KEY_COL3 ";
    m_defaultFunctions["IIC2_SDA"] = "PWR_GPIO_13";  // 默认功能 (function select 3)
    m_functionMacros["IIC2_SDA"] = "IIC2_SDA";
    m_functionMacros["PWM_15"] = "PWM_15";
    m_functionMacros["PWR_GPIO_13"] = "PWR_GPIO_13";
    m_functionMacros["UART2_TX"] = "UART2_TX";
    m_functionMacros["IIS1_MCLK "] = "IIS1_MCLK ";
    m_functionMacros["IIS2_MCLK"] = "IIS2_MCLK";
    m_functionMacros["KEY_COL3"] = "KEY_COL3";

    // K2 引脚功能定义 Pin name: SD0_CD
    m_pinFunctions["SD0_CD"] = QStringList() << "SD0_CD " << "SDIO0_CD" << "XGPIOA—13";
    m_defaultFunctions["SD0_CD"] = "SDIO0_CD";  // 默认功能 (function select 0)
    m_functionMacros["XGPIOA—13"] = "XGPIOA—13";

    
    // K3 引脚功能定义 Pin name: SD0_D1
    m_pinFunctions["SD0_D1"] = QStringList()  << "SDIO0_D_1 "<< "IIC1_SDA " << "AUX0" 
                                      << "XGPIOA—10"  << "UART1_TX"<< "PWM_12" 
                                      << "WG0_D1" << "DBG_3" ;
    m_defaultFunctions["SD0_D1"] = "SDIO0_D_1";  // 默认功能 (function select 0)
    m_functionMacros["SDIO0_D_1"] = "SDIO0_D_1";
    m_functionMacros["IIC1_SDA"] = "IIC1_SDA ";
    m_functionMacros["AUX0"] = "AUX0";
    m_functionMacros["XGPIOA—10"] = "XGPIOA—10";
    m_functionMacros["UART1_TX"] = "UART1_TX";
    m_functionMacros["PWM_12"] = "PWM_12";
    m_functionMacros["WG0_D1"] = "WG0_D1";
    m_functionMacros["DBG_3"] = "DBG_3";

    // K13 引脚功能定义 Pin name: UART2_RX
    m_pinFunctions["UART2_RX"] = QStringList()  << "UART2_RX"<< "PWM_10 " << "PWR_UART1_RX" 
                                      << "PWR_GPIO_16"  << "KEY_COL3"<< "UART4_RX" 
                                      << "IIS2_DI " << "WG2_D1" ;
    m_defaultFunctions["UART2_RX"] = "PWR_GPIO_16";  // 默认功能 (function select 3)
    m_functionMacros["UART2_RX"] = "UART2_RX";
    m_functionMacros["PWM_10"] = "PWM_10";
    m_functionMacros["PWR_UART1_RX"] = "PWR_UART1_RX";
    m_functionMacros["PWR_GPIO_16"] = "PWR_GPIO_16";
    m_functionMacros["KEY_COL3"] = "KEY_COL3";
    m_functionMacros["UART4_RX"] = "UART4_RX";
    m_functionMacros["IIS2_DI"] = "IIS2_DI";
    m_functionMacros["WG2_D1"] = "WG2_D1";


    // K14 引脚功能定义 Pin name: UART2_CTS
    m_pinFunctions["UART2_CTS"] = QStringList()  << "UART2_CTS"<< "PWM_9 " << "PWR_GPIO_17" 
                                      << "KEY_ROW1"  << "UART4_CTS"<< "IIS2_LRCK" 
                                      << "WG1_D1";
    m_defaultFunctions["UART2_CTS"] = "PWR_GPIO_17";  // 默认功能 (function select 3)
    m_functionMacros["UART2_CTS"] = "UART2_CTS";
    m_functionMacros["PWM_9"] = "PWM_9";
    m_functionMacros["PWR_GPIO_17"] = "PWR_GPIO_17";
    m_functionMacros["KEY_ROW1"] = "KEY_ROW1";
    m_functionMacros["UART4_CTS"] = "UART4_CTS";
    m_functionMacros["IIS2_LRCK"] = "IIS2_LRCK";
    m_functionMacros["WG1_D1"] = "WG1_D1";


    // K15 引脚功能定义 Pin name: UART2_TX
    m_pinFunctions["UART2_TX"] = QStringList()  << "UART2_TX"<< "PWM_11 " << "PWR_UART1_TX" 
                                      << "PWR_GPIO_14"  << "KEY_ROW3"<< "UART4_TX" 
                                      << "IIS2_BCLK " << "WG2_D0" ;
    m_defaultFunctions["UART2_TX"] = "PWR_GPIO_14";  // 默认功能 (function select 3)
    m_functionMacros["UART2_TX"] = "UART2_TX";
    m_functionMacros["PWM_11"] = "PWM_11";
    m_functionMacros["PWR_UART1_TX"] = "PWR_UART1_TX";
    m_functionMacros["KEY_ROW3"] = "KEY_ROW3";
    m_functionMacros["UART4_TX"] = "UART4_TX";
    m_functionMacros["PWR_GPIO_14"] = "PWR_GPIO_14";
    m_functionMacros["IIS2_BCLK"] = "IIS2_BCLK";
    m_functionMacros["WG2_D0"] = "WG2_D0";

    
    // L1 引脚功能定义 Pin name: SD0_CLK
    m_pinFunctions["SD0_CLK"] = QStringList() << "SDIO0_CLK" << "IIC1_SDA" << "SPI0_SCK" 
                                       << "XGPIOA_7" << "PWM_15" << "EPHY_LNK_LED" 
                                       << "DBG_0" ;
    m_defaultFunctions["SD0_CLK"] = "SDIO0_CLK";  // 默认功能 (function select 0)
    m_functionMacros["IIC1_SDA"] = "IIC1_SDA";
    m_functionMacros["SPI0_SCK"] = "SPI0_SCK";
    m_functionMacros["XGPIOA_7"] = "XGPIOA_7";
    m_functionMacros["PWM_15"] = "PWM_15";
    m_functionMacros["EPHY_LNK_LED"] = "EPHY_LNK_LED";
    m_functionMacros["DBG_0"] = "DBG_0";

    // L2 引脚功能定义 Pin name: SD0_D0
    m_pinFunctions["SD0_D0"] = QStringList() << "SD0_D0" << "SDIO0_D_0"<<"CAM_MCLK1" 
                                      << "SPI0_SDI"<< "XGPIOA_9" << "UART3_TX" 
                                      << "PWM_13" << "WG0_D0" << "DBG_2" ;
    m_defaultFunctions["SD0_D0"] = "SDIO0_D_0";  // 默认功能 (function select 0)
    m_functionMacros["SDIO0_CLK"] = "SDIO0_CLK";
    m_functionMacros["SDIO0_D_0"] = "SDIO0_D_0";
    m_functionMacros["CAM_MCLK1"] = "CAM_MCLK1";
    m_functionMacros["SPI0_SDI"] = "SPI0_SDI";
    m_functionMacros["XGPIOA_9"] = "XGPIOA_9";
    m_functionMacros["UART3_TX"] = "UART3_TX";
    m_functionMacros["PWM_13"] = "PWM_13";
    m_functionMacros["WG0_D0"] = "WG0_D0";
    m_functionMacros["DBG_2"] = "DBG_2";

    // L3 引脚功能定义 Pin name: SD0_CMD
    m_pinFunctions["SD0_CMD"] = QStringList() << "SDIO0_CMD"<<"IIC1_SCL" << "SPI0_SDO" 
                                       << "XGPIOA_8" << "PWM_14"  << "EPHY_SPD_LED" 
                                       << "DBG_1";
    m_defaultFunctions["SD0_CMD"] = "SDIO0_CMD";  // 默认功能 (function select 3)
    m_functionMacros["SDIO0_CMD"] = "SDIO0_CMD";
    m_functionMacros["IIC1_SCL"] = "IIC1_SCL";
    m_functionMacros["SPI0_SDO"] = "SPI0_SDO";
    m_functionMacros["XGPIOA_8"] = "XGPIOA_8";
    m_functionMacros["PWM_14"] = "PWM_14";
    m_functionMacros["WG0_D0"] = "WG0_D0";
    m_functionMacros["EPHY_SPD_LED"] = "EPHY_SPD_LED" ;
    m_functionMacros["DBG_1"] = "DBG_1";

    // L14 引脚功能定义 Pin name: CLK32K
    m_pinFunctions["CLK32K"] = QStringList() << "CLK32K" << "AUX0" << "CV_4WTDI_CR_SCL0" 
                                        << "PWR_GPIO_10" << "PWM2" << "KEY_COL0" 
                                        << "CAM_MCLK0" << "DBG_0";
    m_defaultFunctions["CLK32K"] = "CLK32K";  // 默认功能 (function select 0)
    // 功能宏映射
    m_functionMacros["CLK32K"] = "CLK32K";
    m_functionMacros["AUX0"] = "AUX0";
    m_functionMacros["CV_4WTDI_CR_SCL0"] = "CV_4WTDI_CR_SCL0";
    m_functionMacros["PWR_GPIO_10"] = "PWR_GPIO_10";
    m_functionMacros["PWM_2"] = "PWM_2";
    m_functionMacros["KEY_COL0"] = "KEY_COL0";
    m_functionMacros["CAM_MCLK0"] = "CAM_MCLK0";
    m_functionMacros["DBG_0"] = "DBG_0";

    // L15 引脚功能定义 Pin name: UART2_RTS
    // 引脚名称使用实际的PAD名称
    m_pinFunctions["UART2_RTS"] = QStringList() << "UART2_RTS" << "PWM_8" << "PWR_GPIO_15" 
                                        << "KEY_ROW0" << "UART4_RTS" << "IIS2_DO" 
                                        << "WG1_D0";
    m_defaultFunctions["UART2_RTS"] = "PWR_GPIO_15";  // 默认功能 (function select 3)
    // 功能宏映射
    m_functionMacros["UART2_RTS"] = "UART2_RTS";
    m_functionMacros["PWM_8"] = "PWM_8";
    m_functionMacros["PWR_GPIO_15"] = "PWR_GPIO_15";
    m_functionMacros["KEY_ROW0"] = "KEY_ROW0";
    m_functionMacros["UART4_RTS"] = "UART4_RTS";
    m_functionMacros["IIS2_DO"] = "IIS2_DO";
    m_functionMacros["WG1_D0"] = "WG1_D0";

    // M1 引脚功能定义 Pin name:SD0_D3
    m_pinFunctions["SD0_D3"] = QStringList()  << "SDIO0_D_3" << "IIC1_SCLCAM_MCLK0" 
                                       << "SPI0_CS_X" << "XGPIOA_12" << "UART3_RX" << "PWM_10"
                                       << "WG1_D1" << "DBG_5";
    m_defaultFunctions["SD0_D3"] = "SDIO0_D_3";  // 默认功能 (function select 0)
    m_functionMacros["SDIO0_D_3"] = "SDIO0_D_3";
    m_functionMacros["IIC1_SCLCAM_MCLK0"] = "IIC1_SCLCAM_MCLK0";
    m_functionMacros["SPI0_SDO"] = "SPI0_CS_X";
    m_functionMacros["SXGPIOA_12"] = "XGPIOA_12";
    m_functionMacros["PWM_14"] = "PWM_14";
    m_functionMacros["UART3_RX"] = "UART3_RX";
    m_functionMacros["PWM_10"] = "PWM_10" ;
    m_functionMacros["WG1_D1"] = "WG1_D1";
    m_functionMacros["DBG_5"] = "DBG_5";

    // M2 引脚功能定义 Pin name:SD0_D2
    m_pinFunctions["SD0_D2"] = QStringList()  << "SDIO0_D_2 " << "IIC1_SCL" << "AUX1"
                                      << "XGPIOA_11"  << "UART1_RX" << "PWM_11"
                                      << "WG1_D0" << "DBG_4" ;
    m_defaultFunctions["SD0_D2"] = "SDIO0_D_2";  // 默认功能 (function select 0)
    m_functionMacros["SDIO0_D_2"] = "SDIO0_D_2";
    m_functionMacros["IIC1_SCL"] = "IIC1_SCL";
    m_functionMacros["AUX1"] = "AUX1";
    m_functionMacros["XGPIOA_11"] = "XGPIOA_11";
    m_functionMacros["UART1_RX"] = "UART1_RX";
    m_functionMacros["PWM_11"] = "PWM_11";
    m_functionMacros["WG1_D0"] = "WG1_D0";
    m_functionMacros["DBG_4"] = "DBG_4";

    // M4 引脚功能定义 Pin name:UART0_RX
    m_pinFunctions["UART0_RX"] = QStringList() << "UART0_RX" << "CAM_MCLK0 " << "PWM_5"
                                        << "XGPIOA_17"  << "UART1_RX" << "AUX0 " 
                                        << "DBG_7" ;
    m_defaultFunctions["UART0_RX"] = "UART0_RX";  // 默认功能 (function select 3)
    m_functionMacros["CAM_MCLK0"] = "CAM_MCLK0";
    m_functionMacros["UART0_RX"] = "UART0_RX";
    m_functionMacros["PWM_5"] = "PWM_5";
    m_functionMacros["XGPIOA_17"] = "XGPIOA_17";
    m_functionMacros["UART1_RX"] = "UART1_RX";
    m_functionMacros["AUX0"] = "AUX0";
    m_functionMacros["DBG_7"] = "DBG_7";

    // M5 引脚功能定义 Pin name: UART0_TX
    m_pinFunctions["UART0_TX"] = QStringList() << "UART0_TX" << "CAM_MCLK1" << "PWM_4" 
                                        << "XGPIOA_16" << "UART1_TX" << "AUX1" << "DBG_6";
    m_defaultFunctions["UART0_TX"] = "UART0_TX";
    m_functionMacros["UART0_TX"] = "UART0_TX";
    m_functionMacros["CAM_MCLK1"] = "CAM_MCLK1";
    m_functionMacros["PWM_4"] = "PWM_4";
    m_functionMacros["XGPIOA_16"] = "XGPIOA_16";
    m_functionMacros["UART1_TX"] = "UART1_TX";
    m_functionMacros["AUX1"] = "AUX1";
    m_functionMacros["DBG_6"] = "DBG_6";

    // M6 引脚功能定义 Pin name:JTAG_CPU_TRST
    m_pinFunctions["JTAG_CPU_TRST"] = QStringList() <<"JTAG_CPU_TRST" << "XGPIOA_20" << "VO_D_30";
    m_defaultFunctions["JTAG_CPU_TRST"] = "JTAG_CPU_TRST";  // 默认功能 (function select 3)
    m_functionMacros["JTAG_CPU_TRST"] = "JTAG_CPU_TRST";
    m_functionMacros["XGPIOA_20"] = "XGPIOA_20";
    m_functionMacros["VO_D_30"] = "VO_D_30";

    // M11 引脚功能定义 Pin name:PWR_ON
    m_pinFunctions["PWR_ON"] = QStringList() <<"PWR_ON" << "PWR_GPIO_9" << "VUART1_RX" 
                                      << "IIC4_SDA" << "EPHY_SPD_LED" << "WG0_D1";
    m_defaultFunctions["PWR_ON"] = "PWR_ON";  // 默认功能 (function select 0)
    m_functionMacros["PWR_ON"] = "PWR_ON";
    m_functionMacros["PWR_GPIO_9"] = "PWR_GPIO_9";
    m_functionMacros["VUART1_RX"] = "VUART1_RX";
    m_functionMacros["IIC4_SDA"] = "IIC4_SDA";
    m_functionMacros["EPHY_SPD_LED"] = "EPHY_SPD_LED";
    m_functionMacros["WG0_D1"] = "WG0_D1";

    // M12 引脚功能定义 Pin name:PWR_GPIO2
    m_pinFunctions["PWR_GPIO2"] = QStringList() <<"PWR_GPIO_2" << "PWR_SECTICK" << "EPHY_SPD_LED"
                                         << "PWM_10"  << "PWR_IIC_SDA " << "IIC2_SDA" <<  "CV_4WTCK_CR_2WTCK";
    m_defaultFunctions["PWR_GPIO2"] = "PWR_GPIO_2";  // 默认功能 (function select 3)
    m_functionMacros["PWR_GPIO_2"] = "PWR_GPIO_2";
    m_functionMacros["PWR_SECTICK"] = "PWR_SECTICK";
    m_functionMacros["EPHY_SPD_LED"] = "EPHY_SPD_LED";
    m_functionMacros["PWM_10"] = "PWM_10";
    m_functionMacros["PWR_IIC_SDA "] = "PWR_IIC_SDA ";
    m_functionMacros["IIC2_SDA"] = "IIC2_SDA";
    m_functionMacros["CV_4WTCK_CR_2WTCK"] = "CV_4WTCK_CR_2WTCK";

    // M13 引脚功能定义 Pin name:PWR_GPIO0
    m_pinFunctions["PWR_GPIO0"] = QStringList() <<"PWR_GPIO_0" << "UART2_TX" << "PWR_UART0_RX"
                                         << "PWM_8"  << "EPHY_SPD_LED" << "WG0_D1";
    m_defaultFunctions["PWR_GPIO0"] = "PWR_GPIO_0";  // 默认功能 (function select 3)
    m_functionMacros["PWR_GPIO_0"] = "PWR_GPIO_0";
    m_functionMacros["UART2_TX"] = "UART2_TX";
    m_functionMacros["PWR_UART0_RX"] = "PWR_UART0_RX";
    m_functionMacros["PWM_8"] = "PWM_8";

    
    // M14 引脚功能定义 Pin name:CLK25M
    m_pinFunctions["CLK25M"] = QStringList() <<"CLK25M" << "AUX1" << "CV_4WTDO_CR_2WTMS"
                                         << "PWR_GPIO_11"  << "PWM_3" << "KEY_COL1" << "CAM_MCLK1"<< "DBG_1";
    m_defaultFunctions["CLK25M"] = "CLK25M";  // 默认功能 (function select 3)
    m_functionMacros["CLK25M"] = "CLK25M";
    m_functionMacros["AUX1"] = "AUX1";
    m_functionMacros["CV_4WTDO_CR_2WTMS"] = "CV_4WTDO_CR_2WTMS";
    m_functionMacros["PWR_GPIO_11"] = "PWR_GPIO_11";
    m_functionMacros["PWM_3"] = "PWM_3";
    m_functionMacros["KEY_COL1"] = "KEY_COL1";
    m_functionMacros["CAM_MCLK1"] = "CAM_MCLK1";
    m_functionMacros["DBG_1"] = "DBG_1";

     // N1 引脚功能定义 Pin name:SD0_PWR_EN
    m_pinFunctions["SD0_PWR_EN"] = QStringList() << "SD0_PWR_EN" << "SDIO0_PWR_EN" << "XGPIOA_14";
    m_defaultFunctions["SD0_PWR_EN"] = "XGPIOA_14";  // 默认功能 (function select 3)
    m_functionMacros["SD0_PWR_EN"] = "SD0_PWR_EN";
    m_functionMacros["SDIO0_PWR_EN"] = "SDIO0_PWR_EN";
    m_functionMacros["XGPIOA_14"] = "XGPIOA_14";

    // N3 引脚功能定义 Pin name:SPK_EN
    m_pinFunctions["SPK_EN"] = QStringList() << "SPK_EN" << "XGPIOA_15 ";
    m_defaultFunctions["SPK_EN"] = "XGPIOA_15";  // 默认功能 (function select 3)
    m_functionMacros["XGPIOA_15"] = "XGPIOA_15";

    // N4 引脚功能定义 Pin name:JTAG_CPU_TCK
    m_pinFunctions["JTAG_CPU_TCK"] = QStringList() <<"CV_2WTCK_CR_4WTCK" << "CAM_MCLK1" << "PWM_6" 
                                            << "XGPIOA_18"<< "UART1_CTS"<< "AUX1"
                                            << "UART1_RX"<< "VO_D_29";
    m_defaultFunctions["JTAG_CPU_TCK"] = "CV_2WTCK_CR_4WTCK";  // 默认功能 (function select 0)
    m_functionMacros["CV_2WTCK_CR_4WTCK"] = "CV_2WTCK_CR_4WTCK";
    m_functionMacros["CAM_MCLK1"] = "CAM_MCLK1";
    m_functionMacros["PWM_6"] = "PWM_6";
    m_functionMacros["XGPIOA_18"] = "XGPIOA_18";
    m_functionMacros["UART1_CTS"] = "UART1_CTS";
    m_functionMacros["AUX1"] = "AUX1";
    m_functionMacros["UART1_RX"] = "UART1_RX";
    m_functionMacros["VO_D_29"] = "VO_D_29";
    
    // N6 引脚功能定义 Pin name:JTAG_CPU_TMS
    m_pinFunctions["JTAG_CPU_TMS"] = QStringList() << "CV_2WTMS_CR_4WTMS" << "CAM_MCLK0" << "PWM_7"
                                            << "XGPIOA_19"<< "UART1_RTS" << "AUX0"
                                            << "UART1_T"<< "VO_D_28";
    m_defaultFunctions["JTAG_CPU_TMS"] = "CV_2WTMS_CR_4WTMS";  // 默认功能 (function select 0)
    m_functionMacros["CV_2WTMS_CR_4WTMS"] = "CV_2WTMS_CR_4WTMS";
    m_functionMacros["CAM_MCLK0"] = "CAM_MCLK0";
    m_functionMacros["PWM_7"] = "PWM_7";
    m_functionMacros["XGPIOA_19"] = "XGPIOA_19";
    m_functionMacros["UART1_RTS"] = "UART1_RTS";
    m_functionMacros["AUX0"] = "AUX0";
    m_functionMacros["UART1_T"] = "UART1_T";
    m_functionMacros["VO_D_28"] = "VO_D_28";

    // N11 引脚功能定义 Pin name:PWR_WAKEUP1
    m_pinFunctions["PWR_WAKEUP1"] = QStringList() <<"PWR_WAKEUP1"<< "PWR_IR1" << "PWR_GPIO_7"
                                           << "UART1_TX"<< "IIC4_SCL" << "EPHY_LNK_LED"
                                            << "WG0_D0";
    m_defaultFunctions["PWR_WAKEUP1"] = "PWR_WAKEUP1";  // 默认功能 (function select 3)
    m_functionMacros["PWR_WAKEUP1"] = "PWR_WAKEUP1";
    m_functionMacros["PWR_IR1"] = "PWR_IR1";
    m_functionMacros["PWR_GPIO_7"] = "PWR_GPIO_7";
    m_functionMacros["UART1_TX"] = "UART1_TX";
    m_functionMacros["IIC4_SCL"] = "IIC4_SCL";
    m_functionMacros["EPHY_LNK_LED"] = "EPHY_LNK_LED";
    m_functionMacros["WG0_D0"] = "WG0_D0";

    // N12 引脚功能定义 Pin name:PWR_WAKEUP0
    m_pinFunctions["PWR_WAKEUP0"] = QStringList() << "PWR_WAKEUP0" << "PWR_IR0" << "PWR_UART0_TX"
                                           << "PWR_GPIO_6" << "UART1_TX"<< "IIC4_SCL"
                                           << "EPHY_LNK_LED"<< "WG2_D0";
    m_defaultFunctions["PWR_WAKEUP0"] = "PWR_WAKEUP0";  // 默认功能 (function select 3)
     m_functionMacros["PWR_WAKEUP0"] = "PWR_WAKEUP0";
    m_functionMacros["PWR_IR0"] = "PWR_IR0";
    m_functionMacros["PWR_UART0_TX"] = "PWR_UART0_TX";
    m_functionMacros["PWR_GPIO_6"] = "PWR_GPIO_6";
    m_functionMacros["UART1_TX"] = "UART1_TX";
    m_functionMacros["IIC4_SCL"] = "IIC4_SCL";
    m_functionMacros["EPHY_LNK_LED"] = "EPHY_LNK_LED";
    m_functionMacros["WG2_D0"] = "WG2_D0";

    // N13 引脚功能定义 Pin name:PWR_GPIO1
    m_pinFunctions["PWR_GPIO1"] = QStringList() << "PWR_GPIO_1" << "UART2_RX" << "EPHY_LNK_LED" 
                                         << "PWM_9" << "PWR_IIC_SCL"<< "IIC2_SCL"<< "CV_4WTMS_CR_SDA0";
    m_defaultFunctions["PWR_GPIO1"] = "PWR_GPIO_1";  // 默认功能 (function select 3)
    m_functionMacros["PWR_GPIO_1"] = "PWR_GPIO_1";
    m_functionMacros["UART2_RX"] = "UART2_RX";
    m_functionMacros["EPHY_LNK_LED"] = "EPHY_LNK_LED";
    m_functionMacros["PWR_IIC_SCL"] = "PWR_IIC_SCL";
    m_functionMacros["IIC2_SCL"] = "IIC2_SCL";
    m_functionMacros["CV_4WTMS_CR_SDA0"] = "CV_4WTMS_CR_SDA0";

    // P1 引脚功能定义 Pin name:EMMC_DAT3
    m_pinFunctions["EMMC_DAT3"] = QStringList() << "EMMC_DAT_3" << "SPINOR_WP_X" << "SPINAND_WP"
                                         << "XGPIOA_27";
    m_defaultFunctions["EMMC_DAT3"] = "SPINOR_WP_X";  // 默认功能 (function select 3)
    m_functionMacros["EMMC_DAT_3"] = "EMMC_DAT_3";
    m_functionMacros["SPINOR_WP_X"] = "SPINOR_WP_X";
    m_functionMacros["SPINAND_WP"] = "SPINAND_WP";
    m_functionMacros["XGPIOA_27"] = "XGPIOA_27";
    
    
    // P2 引脚功能定义 Pin name:EMMC_DAT0
    m_pinFunctions["EMMC_DAT0"] = QStringList() << "EMMC_DAT_0" << "SPINOR_MOSI " << "SPINAND_MOSI"
                                         << "XGPIOA_25";
    m_defaultFunctions["EMMC_DAT0"] = "SPINOR_MOSI";  // 默认功能 (function select 3)
    m_functionMacros["EMMC_DAT_0"] = "EMMC_DAT_0";
    m_functionMacros["SPINOR_MOSI"] = "SPINOR_MOSI";
    m_functionMacros["SPINAND_MOSI"] = "SPINAND_MOSI";
    m_functionMacros["XGPIOA_25"] = "XGPIOA_25";

    // P3 引脚功能定义 Pin name:EMMC_DAT2
    m_pinFunctions["EMMC_DAT2"] = QStringList() << "EMMC_DAT_2" << "SPINOR_HOLD_X " << "SPINAND_HOLD"
                                         << "XGPIOA_26";
    m_defaultFunctions["EMMC_DAT2"] = "SPINOR_HOLD_X";  // 默认功能 (function select 3)
    m_functionMacros["EMMC_DAT_2"] = "EMMC_DAT_2";
    m_functionMacros["SPINOR_HOLD_X"] = "SPINOR_HOLD_X";
    m_functionMacros["SPINAND_HOLD"] = "SPINAND_HOLD";
    m_functionMacros["XGPIOA_26"] = "XGPIOA_26";

    
    // P4 引脚功能定义 Pin name:EMMC_RSTN
    m_pinFunctions["EMMC_RSTN"] = QStringList() << "EMMC_RSTN" << "XGPIOA_21" << "AUX2";
    m_defaultFunctions["EMMC_RSTN"] = "EMMC_RSTN";  // 默认功能 (function select 0)
    m_functionMacros["EMMC_RSTN"] = "EMMC_RSTN";
    m_functionMacros["XGPIOA_21"] = "XGPIOA_21";
    m_functionMacros["AUX2"] = "AUX2";

    // P5 引脚功能定义 Pin name:AUX0
    m_pinFunctions["AUX0"] = QStringList() << "AUX0" <<"XGPIOA_30" << "IIS1_MCLK" 
                                    << "VO_D_31" << "WG1_D1"<< "DBG_12";
    m_defaultFunctions["AUX0"] = "XGPIOA_30";  // 默认功能 (function select 3)
    m_functionMacros["AUX0"] = "AUX0";
    m_functionMacros["XGPIOA_30"] = "XGPIOA_30";
    m_functionMacros["IIS1_MCLK"] = "IIS1_MCLK";
    m_functionMacros["VO_D_31"] = "VO_D_31";
    m_functionMacros["WG1_D1"] = "WG1_D1";
    m_functionMacros["DBG_12"] = "DBG_12";

    // P6 引脚功能定义 Pin name:IIC0_SDA
    m_pinFunctions["IIC0_SDA"] = QStringList() << "CV_SDA" <<"UART1_RX" << "UART2_RX"
                                        << " XGPIOA_29" << "WG0_D1"<< "WG1_D0"<< "DBG_11";
    m_defaultFunctions["IIC0_SDA"] = "CV_SDA";  // 默认功能 (function select 0)
    m_functionMacros["CV_SDA"] = "CV_SDA";
    m_functionMacros["UART1_RX"] = "UART1_RX";
    m_functionMacros["UART2_RX"] = "UART2_RX";
    m_functionMacros["XGPIOA_29"] = "XGPIOA_29";
    m_functionMacros["WG0_D1"] = "WG0_D1";
    m_functionMacros["WG1_D0"] = "WG1_D0";
    m_functionMacros["DBG_11"] = "DBG_11";

    // P10 引脚功能定义 Pin name:PWR_SEQ3
    m_pinFunctions["PWR_SEQ3"] = QStringList() << "PWR_SEQ3" <<"PWR_GPIO_5";
    m_defaultFunctions["PWR_SEQ3"] = "PWR_SEQ3";  // 默认功能 (function select 3)
    m_functionMacros["PWR_SEQ3"]="PWR_SEQ3";
    m_functionMacros["PWR_GPIO_5"] = "PWR_GPIO_5";

    // P11 引脚功能定义 Pin name:PWR_VBAT_DET
    m_pinFunctions["PWR_VBAT_DET"] = QStringList()<< "PWR_VBAT_DET" ;
    m_defaultFunctions["PWR_VBAT_DET"] = "PWR_VBAT_DET";  // 默认功能 (function select 3)
    m_functionMacros["PWR_VBAT_DET"] = "PWR_VBAT_DET";

    // P12 引脚功能定义 Pin name:PWR_SEQ1
    m_pinFunctions["PWR_SEQ1"] = QStringList() << "PWR_SEQ1"<< "PWR_GPIO_3";
    m_defaultFunctions["PWR_SEQ1"] = "PWR_SEQ1";  // 默认功能 (function select 3)
    m_functionMacros["WR_SEQ1"] = "WR_SEQ1";
    m_functionMacros["PWR_GPIO_3"] = "PWR_GPIO_3";

    // P13 引脚功能定义 Pin name:PWR_BUTTON1
    m_pinFunctions["PWR_BUTTON1"] = QStringList() << "PWR_BUTTON1"<< "PWR_GPIO_8" << "UART1_RX" 
                                           <<  "IIC4_SDA" <<  "EPHY_SPD_LED"<<  "WG2_D1";
    m_defaultFunctions["PWR_BUTTON1"] = "PWR_BUTTON1";  // 默认功能 (function select 3)
    m_functionMacros["PWR_GPIO_8"] = "PWR_GPIO_8";
    m_functionMacros["UART1_RX"] = "UART1_RX";
    m_functionMacros["IIC4_SDA"] = "IIC4_SDA";
    m_functionMacros["EPHY_SPD_LED"] = "EPHY_SPD_LED";
    m_functionMacros["WG2_D1"] = "WG2_D1";

    // R2 引脚功能定义 Pin name:EMMC_DAT1
    m_pinFunctions["EMMC_DAT1"] = QStringList() << "EMMC_DAT_1" << "SPINOR_CS_X" << "SPINAND_CS"
                                         << "XGPIOA_24";
    m_defaultFunctions["EMMC_DAT1"] = "SPINOR_CS_X";  // 默认功能 (function select 1)
    m_functionMacros["EMMC_DAT_1"] = "EMMC_DAT_1";
    m_functionMacros["SPINOR_CS_X"] = "SPINOR_CS_X";
    m_functionMacros["SPINAND_CS"] = "SPINAND_CS";
    m_functionMacros["XGPIOA_24"] = "XGPIOA_24";

    // R3 引脚功能定义 Pin name:EMMC_CMD
    m_pinFunctions["EMMC_CMD"] = QStringList() << "EMMC_CMD  " << "SPINOR_MISO" << "SPINAND_MISO" 
                                        << "XGPIOA_23";
    m_defaultFunctions["EMMC_CMD"] = "SPINOR_MISO";  // 默认功能 (function select 3)
    m_functionMacros["EMMC_CMD"] = "EMMC_CMD";
    m_functionMacros["SPINOR_MISO"] = "SPINOR_MISO";
    m_functionMacros["SPINAND_MISO"] = "SPINAND_MISO";
    m_functionMacros["XGPIOA_23"] = "XGPIOA_23";

    // R4 引脚功能定义 Pin name:EMMC_CLK
    m_pinFunctions["EMMC_CLK"] = QStringList() << "EMMC_CLK " << "SPINOR_SCK" << "SPINAND_CLK" 
                                        << "XGPIOA_22";
    m_defaultFunctions["EMMC_CLK"] = "SPINOR_SCK";  // 默认功能 (function select 3)
    m_functionMacros["EMMC_CLK"] = "EMMC_CLK";
    m_functionMacros["SPINOR_SCK"] = "SPINOR_SCK";
    m_functionMacros["SPINAND_CLK"] = "SPINAND_CLK";
    m_functionMacros["XGPIOA_22"] = "XGPIOA_22";

    // R6 引脚功能定义 Pin name:IIC0_SCL
    m_pinFunctions["IIC0_SCL"] = QStringList() << "CV_SCL" <<"UART1_TX" << "UART2_TX" 
                                        << "XGPIOA_28" << "WG0_D0" << "DBG_10";
    m_defaultFunctions["IIC0_SCL"] = "CV_SCL";  // 默认功能 (function select 0)
    m_functionMacros["CV_SCL"] = "CV_SCL";
    m_functionMacros["UART1_TX"] = "UART1_TX";
    m_functionMacros["UART2_TX"] = "UART2_TX";
    m_functionMacros["XGPIOA_28"] = "XGPIOA_28";
    m_functionMacros["WG0_D0"] = "WG0_D0";
    m_functionMacros["DBG_10"] = "DBG_10";
    
    // R10 引脚功能定义 Pin name:GPIO_ZQ___PAD_ZQ
    m_pinFunctions["GPIO_ZQ___PAD_ZQ"] = QStringList() << "PWR_GPIO_24" <<"PWM_2";
    m_defaultFunctions["GPIO_ZQ___PAD_ZQ"] = "PWR_GPIO_24";  // 默认功能 (function select 3)
    m_functionMacros["PWR_GPIO_24"] = "PWR_GPIO_24";
    m_functionMacros["PWM_2"] = "PWM_2";

    // R11 引脚功能定义 Pin name:PWR_RSTN
    m_pinFunctions["PWR_RSTN"] = QStringList() << "PWR_RSTN" ;
    m_defaultFunctions["PWR_RSTN"] = "PWR_RSTN";  // 默认功能 (function select 3)
    m_functionMacros["PWR_RSTN"] = "PWR_RSTN";

    // R12 引脚功能定义 Pin name:PWR_SEQ2
    m_pinFunctions["PWR_SEQ2"] = QStringList() <<"PWR_SEQ2" << "PWR_GPIO_4" ;
    m_defaultFunctions["PWR_SEQ2"] = "PWR_SEQ2";  // 默认功能 (function select 3)
    m_functionMacros["PWR_SEQ2"] = "PWR_SEQ2";
    m_functionMacros["PWR_GPIO_4"] = "PWR_GPIO_4";

    // R13 引脚功能定义 Pin name:XTAL_XIN
    m_pinFunctions["XTAL_XIN"] = QStringList() <<"PWR_XTAL_CLKIN" ;
    m_defaultFunctions["XTAL_XIN"] = "PWR_XTAL_CLKIN";  // 默认功能 (function select 3)
    m_functionMacros["PWR_XTAL_CLKIN"] = "PWR_XTAL_CLKIN";


    // 为其他引脚添加基本功能支持（可以根据需要扩展）
    QStringList basicFunctions = QStringList() << "GPIO" << "ADC" << "PWM" << "I2C" << "UART" << "SPI";
    
    // QFN引脚（数字编号1-88）
    for (int i = 1; i <= 88; ++i) {
        QString pinName = QString::number(i);
        if (!m_pinFunctions.contains(pinName)) {
            m_pinFunctions[pinName] = basicFunctions;
            m_defaultFunctions[pinName] = "GPIO";
        }
    }
    
    // BGA引脚（A1-R15格式，跳过I行）
    QString rows = "ABCDEFGHJKLMNOPQR";  // 跳过I
    for (int r = 0; r < rows.length(); ++r) {
        for (int c = 1; c <= 15; ++c) {
            QString pinName = QString("%1%2").arg(rows[r]).arg(c);
            
            // 跳过四个角的引脚
            bool isCorner = (r == 0 && c == 1) ||          // A1 - 左上角
                           (r == 0 && c == 15) ||          // A15 - 右上角
                           (r == rows.length()-1 && c == 1) ||   // R1 - 左下角
                           (r == rows.length()-1 && c == 15);    // R15 - 右下角
            
            if (!isCorner && !m_pinFunctions.contains(pinName)) {
                m_pinFunctions[pinName] = basicFunctions;
                m_defaultFunctions[pinName] = "GPIO";
            }
        }
    }
}
