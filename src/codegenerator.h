#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <QDateTime>
#include "chipconfig.h"
#include "pinfunction.h"

class CodeGenerator
{
public:
    CodeGenerator();
    
    QString generateCode(const ChipConfig& config);
    QString updateExistingFile(const QString& filePath, const ChipConfig& config);
    
    // 设置源代码路径
    void setSourcePath(const QString& sourcePath);
    QString getSourcePath() const;

private:
    QString generateHeader();
    QString generatePinmuxFunction(const ChipConfig& config);
    QString generatePinmuxConfig(const ChipConfig& config);
    QString generateFooter();
    
    QString functionToMacro(const QString& function, const QString& pinName);
    QString getPinMuxName(const QString& pinName, const QString& function);
    
    // 获取默认的 cvi_board_init.c 文件路径
    QString getDefaultBoardInitFilePath(const ChipConfig& config) const;
    
    QMap<QString, QString> m_functionMacros;
    PinFunction m_pinFunction;
    QString m_sourcePath; // 源代码根路径
    
    void initializeFunctionMacros();
};

#endif // CODEGENERATOR_H
