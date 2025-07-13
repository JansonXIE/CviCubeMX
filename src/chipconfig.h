#ifndef CHIPCONFIG_H
#define CHIPCONFIG_H

#include <QString>
#include <QMap>
#include <QStringList>

class ChipConfig
{
public:
    ChipConfig();
    
    void setChipType(const QString& chipType);
    QString getChipType() const;
    
    int getPinCount() const;
    
    void setPinFunction(const QString& pinName, const QString& function);
    QString getPinFunction(const QString& pinName) const;
    
    QStringList getAvailableFunctions() const;
    QMap<QString, QString> getAllPinFunctions() const;
    
    void clearPinFunctions();

private:
    QString m_chipType;
    QMap<QString, QString> m_pinFunctions;
    QStringList m_availableFunctions;
    
    void initializeAvailableFunctions();
    int getPinCountForChip(const QString& chipType) const;
};

#endif // CHIPCONFIG_H
