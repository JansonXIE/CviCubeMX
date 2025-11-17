#include "chipconfig.h"

ChipConfig::ChipConfig()
{
    initializeAvailableFunctions();
}

void ChipConfig::setChipType(const QString& chipType)
{
    m_chipType = chipType;
    clearPinFunctions();
}

QString ChipConfig::getChipType() const
{
    return m_chipType;
}

int ChipConfig::getPinCount() const
{
    return getPinCountForChip(m_chipType);
}

void ChipConfig::setPinFunction(const QString& pinName, const QString& function)
{
    m_pinFunctions[pinName] = function;
}

QString ChipConfig::getPinFunction(const QString& pinName) const
{
    return m_pinFunctions.value(pinName, "GPIO");
}

QStringList ChipConfig::getAvailableFunctions() const
{
    return m_availableFunctions;
}

QMap<QString, QString> ChipConfig::getAllPinFunctions() const
{
    return m_pinFunctions;
}

void ChipConfig::clearPinFunctions()
{
    m_pinFunctions.clear();
}

void ChipConfig::initializeAvailableFunctions()
{
    m_availableFunctions << "GPIO" << "ADC" << "I2C" << "UART" << "SPI" << "PWM" << "Timer" << "reset_state";
}

int ChipConfig::getPinCountForChip(const QString& chipType) const
{
    // 根据芯片型号返回引脚数量
    if (chipType == "cv1801c" || chipType == "cv1801h") {
        if (chipType == "cv1801h") {
            return 60; // BGA封装，8x8=64，去掉四个角 = 60
        }
        return 64; // QFN封装
    } else if (chipType == "cv1811c" || chipType == "cv1811h") {
        if (chipType == "cv1811h") {
            return 84; // BGA封装，自适应网格，去掉四个角
        }
        return 88; // QFN封装
    } else if (chipType.contains("cp_")) {
        return 88;  // QFN封装，每边22个引脚，4*22=88
    } else if (chipType.contains("hp_")) {
        return 221; // BGA封装，15x15=225，去掉四个角 = 221
    }
    return 64; // 默认值
}
