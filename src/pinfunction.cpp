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

    // B1 引脚功能定义 Pin name: PAD_MIPI_TXP3
    // 引脚名称使用实际的PAD名称
    m_pinFunctions["PAD_MIPI_TXP3"] = QStringList() << "VI0_D_18" << "SD1_D1" << "VO_D_27" 
                                        << "XGPIOC_21" << "IIC2_SCL" << "PWM_15" 
                                        << "IIC1_SCL" << "CAM_HS0";
    m_defaultFunctions["PAD_MIPI_TXP3"] = "XGPIOC_21";  // 默认功能 (function select 3)
    // 功能宏映射
    m_functionMacros["VI0_D_18"] = "VI0_D_18";
    m_functionMacros["VO_D_27"] = "VO_D_27";
    m_functionMacros["XGPIOC_21"] = "XGPIOC_21";
    m_functionMacros["IIC2_SCL"] = "IIC2_SCL";
    m_functionMacros["CAM_HS0"] = "CAM_HS0";
    m_functionMacros["PWM_15"] = "PWM_15";
    m_functionMacros["IIC1_SCL"] = "IIC1_SCL";

    // C1 引脚功能定义 Pin name: PAD_CAM_MCLK0
    m_pinFunctions["PAD_CAM_MCLK0"] = QStringList() << "CAM_MCLK0" << "AUX1" << "XGPIOA_0";
    m_defaultFunctions["PAD_CAM_MCLK0"] = "CAM_MCLK0";
    m_functionMacros["CAM_MCLK0"] = "CAM_MCLK0";
    m_functionMacros["AUX1"] = "AUX1";
    m_functionMacros["XGPIOA_0"] = "XGPIOA_0";

    // D4 引脚功能定义 Pin name: IIC3_SCL
    m_pinFunctions["IIC3_SCL"] = QStringList() << "IIC3_SCL" << "XGPIOA_5";
    m_defaultFunctions["IIC3_SCL"] = "XGPIOA_5";  // 默认功能 (function select 3)
    m_functionMacros["IIC3_SCL"] = "IIC3_SCL";
    m_functionMacros["XGPIOA_5"] = "XGPIOA_5";

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
    
    // M5 引脚功能定义 Pin name: PAD_PWM0
    m_pinFunctions["PAD_PWM0"] = QStringList() << "UART0_TX" << "CAM_MCLK1" << "PWM_4" 
                                        << "XGPIOA_16" << "UART1_TX" << "AUX1" << "DBG_6";
    m_defaultFunctions["PAD_PWM0"] = "UART0_TX";
    m_functionMacros["UART0_TX"] = "UART0_TX";
    m_functionMacros["CAM_MCLK1"] = "CAM_MCLK1";
    m_functionMacros["PWM_4"] = "PWM_4";
    m_functionMacros["XGPIOA_16"] = "XGPIOA_16";
    m_functionMacros["UART1_TX"] = "UART1_TX";
    m_functionMacros["AUX1"] = "AUX1";
    m_functionMacros["DBG_6"] = "DBG_6";
    
    
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
