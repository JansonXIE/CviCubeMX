#ifndef DTSCONFIG_H
#define DTSCONFIG_H

#include <QString>
#include <QMap>
#include <QObject>

struct PeripheralInfo {
    QString name;
    QString status;  // "okay", "disabled", or empty (defaults to okay)
    QString clockName;
    QString clockFreq;
    int clockFrequency;  // 时钟频率 (从 clock-frequency 属性解析)
    int pwmCells;        // PWM cells 数量 (从 #pwm-cells 属性解析)
    int currentSpeed;    // 波特率 (从 current-speed 属性解析，仅UART使用)
    bool hasStatus;
    bool hasClock;
    bool hasClockFreq;  // 是否有 clock-frequency 属性
    bool hasPwmCells;   // 是否有 #pwm-cells 属性
    bool hasCurrentSpeed; // 是否有 current-speed 属性 (仅UART使用)
    int lineNumber;  // 在文件中的行号，用于定位修改
};

class DtsConfig : public QObject
{
    Q_OBJECT

public:
    explicit DtsConfig(QObject *parent = nullptr);

    // 加载设备树文件
    bool loadDtsFile(const QString &filePath);
    
    // 保存设备树文件
    bool saveDtsFile();
    
    // 保存单个外设配置到文件
    bool savePeripheralConfig(const QString &peripheral);
    
    // 获取外设信息
    QMap<QString, PeripheralInfo> getPeripheralInfos() const;
    
    // 设置外设状态
    bool setPeripheralStatus(const QString &peripheral, const QString &status);
    
    // 设置外设时钟
    bool setPeripheralClock(const QString &peripheral, const QString &clockName, const QString &clockFreq = "");
    
    // 设置外设时钟频率 (clock-frequency)
    bool setPeripheralClockFrequency(const QString &peripheral, int frequency);
    
    // 设置PWM cells数量 (#pwm-cells)
    bool setPeripheralPwmCells(const QString &peripheral, int cells);
    
    // 设置UART波特率 (current-speed)
    bool setPeripheralCurrentSpeed(const QString &peripheral, int speed);
    
    // 获取特定外设信息
    PeripheralInfo getPeripheralInfo(const QString &peripheral) const;

private:
    QString m_filePath;
    QString m_fileContent;
    QMap<QString, PeripheralInfo> m_peripherals;
    
    // 解析设备树文件
    void parseDtsFile();
    
    // 查找节点信息
    PeripheralInfo parseNode(const QString &nodeName, int startPos, int endPos);
    
    // 更新文件内容
    void updateFileContent();
    
    // 更新单个外设的文件内容
    void updateSinglePeripheralContent(const QString &peripheral);
    
    // 查找节点在文件中的位置
    QPair<int, int> findNodePosition(const QString &nodeName);
};

#endif // DTSCONFIG_H
