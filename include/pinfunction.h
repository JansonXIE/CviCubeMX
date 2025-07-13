#ifndef PINFUNCTION_H
#define PINFUNCTION_H

#include <QString>
#include <QStringList>
#include <QMap>

/**
 * @brief 引脚功能管理类
 * 管理每个引脚支持的多种功能选择
 */
class PinFunction
{
public:
    PinFunction();
    
    /**
     * @brief 获取指定引脚支持的所有功能列表
     * @param pinName 引脚名称（如 "C4", "M5", "A1" 等）
     * @return 支持的功能列表
     */
    QStringList getSupportedFunctions(const QString& pinName) const;
    
    /**
     * @brief 获取指定引脚的默认功能
     * @param pinName 引脚名称
     * @return 默认功能名称
     */
    QString getDefaultFunction(const QString& pinName) const;
    
    /**
     * @brief 获取引脚功能的完整名称（用于代码生成）
     * @param pinName 引脚名称
     * @param function 功能名称
     * @return 完整的功能宏定义名称
     */
    QString getFunctionMacroName(const QString& pinName, const QString& function) const;
    
    /**
     * @brief 检查引脚是否支持指定功能
     * @param pinName 引脚名称
     * @param function 功能名称
     * @return 是否支持
     */
    bool isPinFunctionSupported(const QString& pinName, const QString& function) const;

private:
    void initializePinFunctions();
    
    // 引脚功能映射表: 引脚名称 -> 支持的功能列表
    QMap<QString, QStringList> m_pinFunctions;
    
    // 引脚默认功能映射表: 引脚名称 -> 默认功能
    QMap<QString, QString> m_defaultFunctions;
    
    // 引脚功能宏定义映射表: 引脚名称_功能名称 -> 宏定义名称
    QMap<QString, QString> m_functionMacros;
};

#endif // PINFUNCTION_H
