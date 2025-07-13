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
    // 引脚名称使用实际的PAD名称，而不是BGA位置名称
    m_pinFunctions["PAD_MIPI_TXM4"] = QStringList() << "XGPIOC_18" << "VI0_D_15" << "SD1_CLK" 
                                        << "VO_D_24" << "CAM_MCLK1" << "PWM_12" 
                                        << "IIC1_SDA" << "DBG_18";
    m_defaultFunctions["PAD_MIPI_TXM4"] = "XGPIOC_18";  // 默认功能 (function select 3)
    
    // 功能宏映射
    m_functionMacros["PAD_MIPI_TXM4_XGPIOC_18"] = "XGPIOC_18";
    m_functionMacros["PAD_MIPI_TXM4_VI0_D_15"] = "VI0_D_15";
    m_functionMacros["PAD_MIPI_TXM4_SD1_CLK"] = "SD1_CLK";
    m_functionMacros["PAD_MIPI_TXM4_VO_D_24"] = "VO_D_24";
    m_functionMacros["PAD_MIPI_TXM4_CAM_MCLK1"] = "CAM_MCLK1";
    m_functionMacros["PAD_MIPI_TXM4_PWM_12"] = "PWM_12";
    m_functionMacros["PAD_MIPI_TXM4_IIC1_SDA"] = "IIC1_SDA";
    m_functionMacros["PAD_MIPI_TXM4_DBG_18"] = "DBG_18";

    // C4 引脚功能定义
    m_pinFunctions["C4"] = QStringList() << "CAM_MCLK0" << "AUX1" << "XGPIOA_0";
    m_defaultFunctions["C4"] = "CAM_MCLK0";
    m_functionMacros["C4_CAM_MCLK0"] = "CAM_MCLK0";
    m_functionMacros["C4_AUX1"] = "AUX1";
    m_functionMacros["C4_XGPIOA_0"] = "XGPIOA_0";
    
    // M5 引脚功能定义
    m_pinFunctions["M5"] = QStringList() << "UART0_TX" << "CAM_MCLK1" << "PWM_4" 
                                        << "XGPIOA_16" << "UART1_TX" << "AUX1" << "DBG_6";
    m_defaultFunctions["M5"] = "UART0_TX";
    m_functionMacros["M5_UART0_TX"] = "UART0_TX";
    m_functionMacros["M5_CAM_MCLK1"] = "CAM_MCLK1";
    m_functionMacros["M5_PWM_4"] = "PWM_4";
    m_functionMacros["M5_XGPIOA_16"] = "XGPIOA_16";
    m_functionMacros["M5_UART1_TX"] = "UART1_TX";
    m_functionMacros["M5_AUX1"] = "AUX1";
    m_functionMacros["M5_DBG_6"] = "DBG_6";
    
    // B1 引脚功能定义
    m_pinFunctions["B1"] = QStringList() << "XGPIOA_3" << "IIC1_SDA" << "UART1_RX" << "SPI0_MISO";
    m_defaultFunctions["B1"] = "XGPIOA_3";
    m_functionMacros["B1_XGPIOA_3"] = "B1_XGPIOA_3";
    m_functionMacros["B1_IIC1_SDA"] = "B1_IIC1_SDA";
    m_functionMacros["B1_UART1_RX"] = "B1_UART1_RX";
    m_functionMacros["B1_SPI0_MISO"] = "B1_SPI0_MISO";
    
    // B2 引脚功能定义
    m_pinFunctions["B2"] = QStringList() << "XGPIOA_4" << "IIC1_SCL" << "UART1_TX" << "SPI0_MOSI";
    m_defaultFunctions["B2"] = "XGPIOA_4";
    m_functionMacros["B2_XGPIOA_4"] = "B2_XGPIOA_4";
    m_functionMacros["B2_IIC1_SCL"] = "B2_IIC1_SCL";
    m_functionMacros["B2_UART1_TX"] = "B2_UART1_TX";
    m_functionMacros["B2_SPI0_MOSI"] = "B2_SPI0_MOSI";
    
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
