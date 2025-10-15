#ifndef DTSCONFIG_H
#define DTSCONFIG_H

#include <QString>
#include <QMap>
#include <QObject>

struct ThermalTripInfo {
    QString name;
    int temperature = 0;        // millicelsius
    int hysteresis = 0;         // millicelsius
    QString type;               // "passive" / "critical" / etc.
};

struct ThermalZoneInfo {
    QString name;                             // soc_thermal_0
    int pollingDelayPassive = 0;              // milliseconds
    int pollingDelay = 0;                     // milliseconds
    QString thermalSensorsRef;                // e.g. "&thermal 0"
    QMap<QString, ThermalTripInfo> trips;     // trip name -> info
    // maps: map name -> pair(tripRef, coolingDeviceRaw)
    QMap<QString, QPair<QString, QString>> maps;
};

struct PeripheralInfo {
    QString name;
    QString status;  // "okay", "disabled", or empty (defaults to okay)
    QString clockName;
    QString clockFreq;
    int clockFrequency = 0;  // 时钟频率 (从 clock-frequency 属性解析)
    int pwmCells = 1;        // PWM cells 数量 (从 #pwm-cells 属性解析)
    int currentSpeed = 115200;    // 波特率 (从 current-speed 属性解析，仅UART使用)
    QStringList sysdmaChannels; // SYSDMA通道映射 (仅SYSDMA使用)
    bool hasStatus = false;
    bool hasClock = false;
    bool hasClockFreq = false;  // 是否有 clock-frequency 属性
    bool hasPwmCells = false;   // 是否有 #pwm-cells 属性
    bool hasCurrentSpeed = false; // 是否有 current-speed 属性 (仅UART使用)
    bool hasSysdmaChannels = false; // 是否有 SYSDMA 通道映射
    int lineNumber = 0;  // 在文件中的行号，用于定位修改

    // 默认构造函数
    PeripheralInfo() {
        status = "okay";
        sysdmaChannels = {"0", "5", "12", "13", "42", "42", "4", "7"}; // 默认SYSDMA通道映射
    }
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

    // 设置SYSDMA通道映射 (ch-remap)
    bool setPeripheralSysdmaChannels(const QString &peripheral, const QStringList &channels);

    // 获取特定外设信息
    PeripheralInfo getPeripheralInfo(const QString &peripheral) const;

    // Thermal 配置接口
    bool hasThermalZones() const;
    QStringList getThermalZoneNames() const;
    ThermalZoneInfo getThermalZone(const QString &zoneName) const;
    bool setThermalPolling(const QString &zoneName, int pollingDelayPassiveMs, int pollingDelayMs);
    bool setThermalSensor(const QString &zoneName, const QString &sensorRef);
    bool setThermalTrip(const QString &zoneName, const QString &tripName, int temperatureMilliC, int hysteresisMilliC, const QString &type);
    bool saveThermalZone(const QString &zoneName);
    // 根据 zone 与 trip 计算映射的冷却频率（返回 <cpuHz, tpuHz>；任一为0表示未解析到）
    QPair<int,int> getCoolingFrequenciesForTrip(const QString &zoneName, const QString &tripName) const;

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

    // 将SYSDMA常量名转换为数字
    QString getChannelNumber(const QString &channelName);

    // 将数字转换为SYSDMA常量名
    QString getChannelName(const QString &channelNumber);

    // 创建sysdma_remap节点
    void createSysdmaRemapNode(const PeripheralInfo &info);

    // 更新外设DMA配置
    void updatePeripheralDmaConfigWithPrevious(const QStringList &previousChannels, const QStringList &sysdmaChannels);

    // 根据通道号获取外设节点名
    QString getPeripheralNodeFromChannel(const QString &channelNumber);

    // 判断通道是否为RX通道
    bool isChannelRx(const QString &channelNumber);

    // 为外设添加DMA配置
    void addDmaConfigToPeripheral(const QString &peripheralNode, const QString &channelIndex1, const QString &channelIndex2, const QString &capability);

    // 清除DMA通道映射发生变化的外设的DMA配置
    void clearChangedPeripheralDmaConfigs(const QStringList &previousChannels, const QStringList &newChannels);

    // Thermal 解析与保存
    void parseThermalZones();
    QPair<int, int> findZoneInThermal(const QString &zoneName) const;
    QPair<int, int> findBlockRange(int startPos) const;
    bool replaceOrInsertLine(QString &block, const QRegularExpression &regex, const QString &lineToInsert, bool insertAtBlockStart = false) const;

    // 存储 Thermal 数据
    QMap<QString, ThermalZoneInfo> m_thermalZones;
};

#endif // DTSCONFIG_H
