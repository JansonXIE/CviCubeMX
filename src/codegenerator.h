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

private:
    QString generateHeader();
    QString generatePinmuxFunction(const ChipConfig& config);
    QString generatePinmuxConfig(const ChipConfig& config);
    QString generateFooter();
    
    QString functionToMacro(const QString& function, const QString& pinName);
    QString getPinMuxName(const QString& pinName, const QString& function);
    
    QMap<QString, QString> m_functionMacros;
    PinFunction m_pinFunction;
    void initializeFunctionMacros();
};

#endif // CODEGENERATOR_H
