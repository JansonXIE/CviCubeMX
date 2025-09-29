#include "dtsconfig.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug>

DtsConfig::DtsConfig(QObject *parent) : QObject(parent)
{
}

bool DtsConfig::loadDtsFile(const QString &filePath)
{
    m_filePath = filePath;
    QFile file(filePath);
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "无法打开设备树文件：" << filePath;
        return false;
    }
    
    QTextStream in(&file);
    m_fileContent = in.readAll();
    file.close();
    
    parseDtsFile();
    return true;
}

bool DtsConfig::saveDtsFile()
{
    if (m_filePath.isEmpty()) {
        return false;
    }
    
    updateFileContent();
    
    QFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "无法写入设备树文件：" << m_filePath;
        return false;
    }
    
    QTextStream out(&file);
    out << m_fileContent;
    file.close();
    
    return true;
}

bool DtsConfig::savePeripheralConfig(const QString &peripheral)
{
    if (m_filePath.isEmpty()) {
        return false;
    }
    
    // 只更新指定外设的内容
    updateSinglePeripheralContent(peripheral);
    
    QFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "无法写入设备树文件：" << m_filePath;
        return false;
    }
    
    QTextStream out(&file);
    out << m_fileContent;
    file.close();
    
    return true;
}

QMap<QString, PeripheralInfo> DtsConfig::getPeripheralInfos() const
{
    return m_peripherals;
}

bool DtsConfig::setPeripheralStatus(const QString &peripheral, const QString &status)
{
    if (!m_peripherals.contains(peripheral)) {
        return false;
    }
    
    m_peripherals[peripheral].status = status;
    return true;
}

bool DtsConfig::setPeripheralClock(const QString &peripheral, const QString &clockName, const QString &clockFreq)
{
    if (!m_peripherals.contains(peripheral)) {
        return false;
    }
    
    m_peripherals[peripheral].clockName = clockName;
    if (!clockFreq.isEmpty()) {
        m_peripherals[peripheral].clockFreq = clockFreq;
    }
    return true;
}

PeripheralInfo DtsConfig::getPeripheralInfo(const QString &peripheral) const
{
    return m_peripherals.value(peripheral, PeripheralInfo());
}

void DtsConfig::parseDtsFile()
{
    m_peripherals.clear();
    
    // 定义要解析的外设类型及其模式
    QStringList peripheralPatterns = {
        "pwm[0-9]+:",
        "i2c[0-9]+:",
        "spi[0-9]+:",
        "uart[0-9]+:",
        "gpio[0-9]+:",
        "saradc[0-9]*:",
        "sysdma_remap\\s*\\{" // sysdma_remap节点格式不同，没有冒号
    };
    
    for (const QString &pattern : peripheralPatterns) {
        QRegularExpression regex(pattern);
        QRegularExpressionMatchIterator iterator = regex.globalMatch(m_fileContent);
        
        while (iterator.hasNext()) {
            QRegularExpressionMatch match = iterator.next();
            QString matched = match.captured(0);
            QString nodeName;
            
            // 处理不同的匹配格式
            if (matched.endsWith(":")) {
                nodeName = matched.chopped(1); // 移除末尾的冒号
            } else if (matched.contains("{")) {
                // 对于 "sysdma_remap {" 格式，提取节点名
                QRegularExpression nameRegex("^(\\w+)");
                QRegularExpressionMatch nameMatch = nameRegex.match(matched);
                if (nameMatch.hasMatch()) {
                    nodeName = nameMatch.captured(1);
                }
            } else {
                nodeName = matched;
            }
            
            if (!nodeName.isEmpty()) {
                QPair<int, int> nodePos = findNodePosition(nodeName);
                if (nodePos.first != -1 && nodePos.second != -1) {
                    PeripheralInfo info = parseNode(nodeName, nodePos.first, nodePos.second);
                    info.name = nodeName;
                    m_peripherals[nodeName] = info;
                }
            }
        }
    }
    
    // 确保sysdma_remap节点存在（即使DTS文件中没有）
    if (!m_peripherals.contains("sysdma_remap")) {
        PeripheralInfo sysdmaInfo;
        sysdmaInfo.name = "sysdma_remap";
        sysdmaInfo.status = "okay";
        sysdmaInfo.hasStatus = true;
        sysdmaInfo.hasSysdmaChannels = true;
        sysdmaInfo.sysdmaChannels = {"0", "5", "2", "3", "42", "42", "4", "7"}; // 默认通道映射
        m_peripherals["sysdma_remap"] = sysdmaInfo;
    }
}

PeripheralInfo DtsConfig::parseNode(const QString &nodeName, int startPos, int endPos)
{
    PeripheralInfo info;
    
    QString nodeContent = m_fileContent.mid(startPos, endPos - startPos);
    
    // 解析status属性
    QRegularExpression statusRegex("status\\s*=\\s*\"([^\"]+)\";");
    QRegularExpressionMatch statusMatch = statusRegex.match(nodeContent);
    if (statusMatch.hasMatch()) {
        info.status = statusMatch.captured(1);
        info.hasStatus = true;
    } else {
        info.status = "okay"; // 默认状态
        info.hasStatus = false;
    }
    
    // 解析clock相关信息
    QRegularExpression clockRegex("clocks\\s*=\\s*<([^>]+)>;");
    QRegularExpressionMatch clockMatch = clockRegex.match(nodeContent);
    if (clockMatch.hasMatch()) {
        QString clockContent = clockMatch.captured(1);
        info.hasClock = true;
        
        // 提取时钟名称（通常是最后一个参数）
        QRegularExpression clockNameRegex("CV184X_CLK_(\\w+)");
        QRegularExpressionMatch clockNameMatch = clockNameRegex.match(clockContent);
        if (clockNameMatch.hasMatch()) {
            info.clockName = clockNameMatch.captured(1);
        }
    } else {
        info.hasClock = false;
    }
    
    // 解析clock-frequency属性
    QRegularExpression freqRegex("clock-frequency\\s*=\\s*<([^>]+)>;");
    QRegularExpressionMatch freqMatch = freqRegex.match(nodeContent);
    if (freqMatch.hasMatch()) {
        info.clockFreq = freqMatch.captured(1);
        info.hasClockFreq = true;
        bool ok;
        info.clockFrequency = freqMatch.captured(1).toInt(&ok);
        if (!ok) {
            info.clockFrequency = 0;
        }
    } else {
        info.hasClockFreq = false;
        info.clockFrequency = 0;
    }
    
    // 解析#pwm-cells属性
    QRegularExpression pwmCellsRegex("#pwm-cells\\s*=\\s*<([^>]+)>;");
    QRegularExpressionMatch pwmCellsMatch = pwmCellsRegex.match(nodeContent);
    if (pwmCellsMatch.hasMatch()) {
        info.hasPwmCells = true;
        bool ok;
        info.pwmCells = pwmCellsMatch.captured(1).toInt(&ok);
        if (!ok) {
            info.pwmCells = 1; // 默认值
        }
    } else {
        info.hasPwmCells = false;
        info.pwmCells = 1; // 默认值
    }
    
    // 解析current-speed属性（仅UART使用）
    QRegularExpression currentSpeedRegex("current-speed\\s*=\\s*<([^>]+)>;");
    QRegularExpressionMatch currentSpeedMatch = currentSpeedRegex.match(nodeContent);
    if (currentSpeedMatch.hasMatch()) {
        info.hasCurrentSpeed = true;
        bool ok;
        info.currentSpeed = currentSpeedMatch.captured(1).toInt(&ok);
        if (!ok) {
            info.currentSpeed = 115200; // 默认值
        }
    } else {
        info.hasCurrentSpeed = false;
        info.currentSpeed = 115200; // 默认值
    }
    
    // 解析ch-remap属性（仅SYSDMA使用）
    QRegularExpression chRemapRegex("ch-remap\\s*=\\s*<([^>]+)>;");
    QRegularExpressionMatch chRemapMatch = chRemapRegex.match(nodeContent);
    if (chRemapMatch.hasMatch()) {
        info.hasSysdmaChannels = true;
        QString chRemapContent = chRemapMatch.captured(1).trimmed();
        // 解析通道值，可能是数字或常量名
        QStringList channels = chRemapContent.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        
        // 如果是常量名形式，需要转换为数字
        QStringList channelNumbers;
        for (const QString &channel : channels) {
            if (channel.startsWith("CVI_")) {
                // 提取常量对应的数字值
                channelNumbers.append(getChannelNumber(channel));
            } else {
                channelNumbers.append(channel);
            }
        }
        info.sysdmaChannels = channelNumbers;
    } else {
        info.hasSysdmaChannels = false;
        // 设置默认的SYSDMA通道映射
        info.sysdmaChannels = {"0", "5", "2", "3", "42", "42", "4", "7"};
    }
    
    // 计算行号
    QString beforeNode = m_fileContent.left(startPos);
    info.lineNumber = beforeNode.count('\n') + 1;
    
    return info;
}

void DtsConfig::updateFileContent()
{
    for (auto it = m_peripherals.begin(); it != m_peripherals.end(); ++it) {
        const QString &peripheral = it.key();
        const PeripheralInfo &info = it.value();
        
        QPair<int, int> nodePos = findNodePosition(peripheral);
        if (nodePos.first == -1 || nodePos.second == -1) {
            continue;
        }
        
        QString nodeContent = m_fileContent.mid(nodePos.first, nodePos.second - nodePos.first);
        QString newNodeContent = nodeContent;
        
        // 更新status属性
        QRegularExpression statusRegex("\\s*status\\s*=\\s*\"[^\"]+\"\\s*;");
        QString statusLine = QString("\n\t\tstatus = \"%1\";").arg(info.status);
        
        if (statusRegex.match(newNodeContent).hasMatch()) {
            // 替换现有status
            newNodeContent.replace(statusRegex, statusLine);
        } else if (info.hasStatus || info.status != "okay") {
            // 添加新的status行（在节点最后一个属性后）
            int lastSemicolon = newNodeContent.lastIndexOf(';');
            if (lastSemicolon != -1) {
                newNodeContent.insert(lastSemicolon + 1, statusLine);
            }
        }
        
        // 更新时钟信息（如果需要的话）
        if (info.hasClock && !info.clockName.isEmpty()) {
            QRegularExpression clockRegex("\\s*clocks\\s*=\\s*<[^>]+>\\s*;");
            QString clockLine = QString("\n\t\tclocks = <&clk CV184X_CLK_%1>;").arg(info.clockName);
            
            if (clockRegex.match(newNodeContent).hasMatch()) {
                newNodeContent.replace(clockRegex, clockLine);
            }
        }
        
        // 更新时钟频率（如果需要的话）
        if (info.hasClockFreq && info.clockFrequency > 0) {
            QRegularExpression freqRegex("\\s*clock-frequency\\s*=\\s*<[^>]+>\\s*;");
            QString freqLine = QString("\n\t\tclock-frequency = <%1>;").arg(info.clockFrequency);
            
            if (freqRegex.match(newNodeContent).hasMatch()) {
                // 替换现有clock-frequency
                newNodeContent.replace(freqRegex, freqLine);
            }
        }
        
        // 更新PWM cells（如果需要的话）
        if (info.hasPwmCells && info.pwmCells > 0) {
            QRegularExpression pwmCellsRegex("\\s*#pwm-cells\\s*=\\s*<[^>]+>\\s*;");
            QString pwmCellsLine = QString("\n\t\t#pwm-cells = <%1>;").arg(info.pwmCells);
            
            if (pwmCellsRegex.match(newNodeContent).hasMatch()) {
                // 替换现有#pwm-cells
                newNodeContent.replace(pwmCellsRegex, pwmCellsLine);
            }
        }
        
        // 更新文件内容
        m_fileContent.replace(nodePos.first, nodePos.second - nodePos.first, newNodeContent);
    }
}

void DtsConfig::updateSinglePeripheralContent(const QString &peripheral)
{
    if (!m_peripherals.contains(peripheral)) {
        return;
    }
    
    // 如果是SYSDMA节点，先保存原始的通道配置（从DTS文件中解析）
    QStringList previousSysdmaChannels = {"0", "5", "2", "3", "42", "42", "4", "7"}; // 默认值
    if (peripheral == "sysdma_remap") {
        // 从当前DTS文件内容中解析原始配置
        QPair<int, int> currentNodePos = findNodePosition(peripheral);
        if (currentNodePos.first != -1 && currentNodePos.second != -1) {
            QString currentNodeContent = m_fileContent.mid(currentNodePos.first, currentNodePos.second - currentNodePos.first);
            QRegularExpression chRemapRegex("ch-remap\\s*=\\s*<([^>]+)>;");
            QRegularExpressionMatch chRemapMatch = chRemapRegex.match(currentNodeContent);
            if (chRemapMatch.hasMatch()) {
                QString chRemapContent = chRemapMatch.captured(1).trimmed();
                QStringList channels = chRemapContent.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
                
                QStringList channelNumbers;
                for (const QString &channel : channels) {
                    if (channel.startsWith("CVI_")) {
                        channelNumbers.append(getChannelNumber(channel));
                    } else {
                        channelNumbers.append(channel);
                    }
                }
                if (!channelNumbers.isEmpty()) {
                    previousSysdmaChannels = channelNumbers;
                }
            }
        }
    }
    
    const PeripheralInfo &info = m_peripherals[peripheral];
    
    QPair<int, int> nodePos = findNodePosition(peripheral);
    if (nodePos.first == -1 || nodePos.second == -1) {
        // 如果节点不存在，且是sysdma_remap，则创建新节点
        if (peripheral == "sysdma_remap") {
            createSysdmaRemapNode(info);
        }
        return;
    }
    
    QString nodeContent = m_fileContent.mid(nodePos.first, nodePos.second - nodePos.first);
    QString newNodeContent = nodeContent;
    
    // 更新status属性
    QRegularExpression statusRegex("\\s*status\\s*=\\s*\"[^\"]+\"\\s*;");
    QString statusLine = QString("\n\t\tstatus = \"%1\";").arg(info.status);
    
    if (statusRegex.match(newNodeContent).hasMatch()) {
        // 替换现有status，确保换行
        newNodeContent.replace(statusRegex, statusLine);
    } else if (info.hasStatus || info.status != "okay") {
        // 添加新的status行（在节点最后一个分号后）
        int lastSemicolon = newNodeContent.lastIndexOf(';');
        if (lastSemicolon != -1) {
            newNodeContent.insert(lastSemicolon + 1, statusLine);
        }
    }
    
    // 更新时钟信息（如果需要的话）
    if (info.hasClock && !info.clockName.isEmpty()) {
        QRegularExpression clockRegex("\\s*clocks\\s*=\\s*<[^>]+>\\s*;");
        QString clockLine = QString("\n\t\tclocks = <&clk CV184X_CLK_%1>;").arg(info.clockName);
        
        if (clockRegex.match(newNodeContent).hasMatch()) {
            // 替换现有clocks，确保换行
            newNodeContent.replace(clockRegex, clockLine);
        } else {
            // 添加新的clocks行
            int lastSemicolon = newNodeContent.lastIndexOf(';');
            if (lastSemicolon != -1) {
                newNodeContent.insert(lastSemicolon + 1, clockLine);
            }
        }
    }
    
    // 更新时钟频率（如果需要的话）
    if (info.hasClockFreq && info.clockFrequency > 0) {
        QRegularExpression freqRegex("\\s*clock-frequency\\s*=\\s*<[^>]+>\\s*;");
        QString freqLine = QString("\n\t\tclock-frequency = <%1>;").arg(info.clockFrequency);
        
        if (freqRegex.match(newNodeContent).hasMatch()) {
            // 替换现有clock-frequency
            newNodeContent.replace(freqRegex, freqLine);
        } else {
            // 添加新的clock-frequency行
            int lastSemicolon = newNodeContent.lastIndexOf(';');
            if (lastSemicolon != -1) {
                newNodeContent.insert(lastSemicolon + 1, freqLine);
            }
        }
    }
    
    // 更新PWM cells（如果需要的话）
    if (info.hasPwmCells && info.pwmCells > 0) {
        QRegularExpression pwmCellsRegex("\\s*#pwm-cells\\s*=\\s*<[^>]+>\\s*;");
        QString pwmCellsLine = QString("\n\t\t#pwm-cells = <%1>;").arg(info.pwmCells);
        
        if (pwmCellsRegex.match(newNodeContent).hasMatch()) {
            // 替换现有#pwm-cells
            newNodeContent.replace(pwmCellsRegex, pwmCellsLine);
        } else {
            // 添加新的#pwm-cells行
            int lastSemicolon = newNodeContent.lastIndexOf(';');
            if (lastSemicolon != -1) {
                newNodeContent.insert(lastSemicolon + 1, pwmCellsLine);
            }
        }
    }
    
    // 更新current-speed（如果需要的话，仅UART使用）
    if (info.hasCurrentSpeed && info.currentSpeed > 0) {
        QRegularExpression currentSpeedRegex("\\s*current-speed\\s*=\\s*<[^>]+>\\s*;");
        QString currentSpeedLine = QString("\n\t\tcurrent-speed = <%1>;").arg(info.currentSpeed);
        
        if (currentSpeedRegex.match(newNodeContent).hasMatch()) {
            // 替换现有current-speed
            newNodeContent.replace(currentSpeedRegex, currentSpeedLine);
        } else {
            // 添加新的current-speed行
            int lastSemicolon = newNodeContent.lastIndexOf(';');
            if (lastSemicolon != -1) {
                newNodeContent.insert(lastSemicolon + 1, currentSpeedLine);
            }
        }
    }
    
    // 更新ch-remap（如果需要的话，仅SYSDMA使用）
    if (info.hasSysdmaChannels && !info.sysdmaChannels.isEmpty()) {
        QRegularExpression chRemapRegex("\\s*ch-remap\\s*=\\s*<[^>]+>\\s*;");
        
        // 将数字通道转换为常量名
        QStringList channelNames;
        for (const QString &channel : info.sysdmaChannels) {
            channelNames.append(getChannelName(channel));
        }
        
        // 格式化为两行：前4个常量名在第一行，后4个在第二行并缩进
        QString chRemapLine;
        if (channelNames.size() >= 8) {
            QStringList firstLine = channelNames.mid(0, 4);
            QStringList secondLine = channelNames.mid(4, 4);
            chRemapLine = QString("\n\t\tch-remap = <%1\n\t\t\t\t\t%2>;").arg(firstLine.join(" "), secondLine.join(" "));
        } else {
            chRemapLine = QString("\n\t\tch-remap = <%1>;").arg(channelNames.join(" "));
        }
        
        if (chRemapRegex.match(newNodeContent).hasMatch()) {
            // 替换现有ch-remap
            newNodeContent.replace(chRemapRegex, chRemapLine);
        } else {
            // 添加新的ch-remap行
            int lastSemicolon = newNodeContent.lastIndexOf(';');
            if (lastSemicolon != -1) {
                newNodeContent.insert(lastSemicolon + 1, chRemapLine);
            }
        }
    }
    
    // 更新文件内容
    m_fileContent.replace(nodePos.first, nodePos.second - nodePos.first, newNodeContent);
    
    // 如果是SYSDMA节点，还需要更新相关外设的DMA配置
    if (peripheral == "sysdma_remap") {
        updatePeripheralDmaConfigWithPrevious(previousSysdmaChannels, info.sysdmaChannels);
    }
}

QPair<int, int> DtsConfig::findNodePosition(const QString &nodeName)
{
    // 查找节点开始位置，支持两种格式：
    // 1. nodeName: { } (设备树覆盖格式)
    // 2. nodeName { } (普通节点格式)
    QRegularExpression nodeRegex1(QString("\\b%1\\s*:\\s*[^{]*\\{").arg(QRegularExpression::escape(nodeName)));
    QRegularExpressionMatch match1 = nodeRegex1.match(m_fileContent);
    
    QRegularExpression nodeRegex2(QString("\\b%1\\s+\\{").arg(QRegularExpression::escape(nodeName)));
    QRegularExpressionMatch match2 = nodeRegex2.match(m_fileContent);
    
    QRegularExpressionMatch match;
    if (match1.hasMatch()) {
        match = match1;
    } else if (match2.hasMatch()) {
        match = match2;
    } else {
        return QPair<int, int>(-1, -1);
    }
    
    int startPos = match.capturedStart();
    int braceStart = match.capturedEnd() - 1; // '{' 的位置
    
    // 查找匹配的结束大括号
    int braceCount = 1;
    int pos = braceStart + 1;
    
    while (pos < m_fileContent.length() && braceCount > 0) {
        if (m_fileContent.at(pos) == '{') {
            braceCount++;
        } else if (m_fileContent.at(pos) == '}') {
            braceCount--;
        }
        pos++;
    }
    
    if (braceCount == 0) {
        return QPair<int, int>(startPos, pos);
    }
    
    return QPair<int, int>(-1, -1);
}

bool DtsConfig::setPeripheralClockFrequency(const QString &peripheral, int frequency)
{
    if (!m_peripherals.contains(peripheral)) {
        return false;
    }
    
    m_peripherals[peripheral].clockFrequency = frequency;
    m_peripherals[peripheral].hasClockFreq = true;
    
    return true;
}

bool DtsConfig::setPeripheralPwmCells(const QString &peripheral, int cells)
{
    if (!m_peripherals.contains(peripheral)) {
        return false;
    }
    
    m_peripherals[peripheral].pwmCells = cells;
    m_peripherals[peripheral].hasPwmCells = true;
    
    return true;
}

bool DtsConfig::setPeripheralCurrentSpeed(const QString &peripheral, int speed)
{
    if (!m_peripherals.contains(peripheral)) {
        return false;
    }
    
    m_peripherals[peripheral].currentSpeed = speed;
    m_peripherals[peripheral].hasCurrentSpeed = true;
    
    return true;
}

bool DtsConfig::setPeripheralSysdmaChannels(const QString &peripheral, const QStringList &channels)
{
    if (!m_peripherals.contains(peripheral)) {
        return false;
    }
    
    m_peripherals[peripheral].sysdmaChannels = channels;
    m_peripherals[peripheral].hasSysdmaChannels = true;
    
    return true;
}

QString DtsConfig::getChannelNumber(const QString &channelName)
{
    // SYSDMA通道常量名到数字的映射
    static QMap<QString, QString> channelMap = {
        {"CVI_I2S0_RX", "0"}, {"CVI_I2S0_TX", "1"}, {"CVI_I2S1_RX", "2"}, {"CVI_I2S1_TX", "3"},
        {"CVI_I2S2_RX", "4"}, {"CVI_I2S2_TX", "5"}, {"CVI_I2S3_RX", "6"}, {"CVI_I2S3_TX", "7"},
        {"CVI_UART0_RX", "8"}, {"CVI_UART0_TX", "9"}, {"CVI_UART1_RX", "10"}, {"CVI_UART1_TX", "11"},
        {"CVI_UART2_RX", "12"}, {"CVI_UART2_TX", "13"}, {"CVI_UART3_RX", "14"}, {"CVI_UART3_TX", "15"},
        {"CVI_SPI0_RX", "16"}, {"CVI_SPI0_TX", "17"}, {"CVI_SPI1_RX", "18"}, {"CVI_SPI1_TX", "19"},
        {"CVI_SPI2_RX", "20"}, {"CVI_SPI2_TX", "21"}, {"CVI_SPI3_RX", "22"}, {"CVI_SPI3_TX", "23"},
        {"CVI_I2C0_RX", "24"}, {"CVI_I2C0_TX", "25"}, {"CVI_I2C1_RX", "26"}, {"CVI_I2C1_TX", "27"},
        {"CVI_I2C2_RX", "28"}, {"CVI_I2C2_TX", "29"}, {"CVI_I2C3_RX", "30"}, {"CVI_I2C3_TX", "31"},
        {"CVI_I2C4_RX", "32"}, {"CVI_I2C4_TX", "33"}, {"CVI_TDM0_RX", "34"}, {"CVI_TDM0_TX", "35"},
        {"CVI_TDM1_RX", "36"}, {"CVI_AUDSRC", "37"}, {"CVI_SPI_NOR_RX", "38"}, {"CVI_SPI_NOR_TX", "39"},
        {"CVI_UART4_RX", "40"}, {"CVI_UART4_TX", "41"}, {"CVI_SPI_NAND", "42"}
    };
    
    return channelMap.value(channelName, "0");
}

QString DtsConfig::getChannelName(const QString &channelNumber)
{
    // 数字到SYSDMA通道常量名的映射（getChannelNumber的逆映射）
    static QMap<QString, QString> numberToNameMap = {
        {"0", "CVI_I2S0_RX"}, {"1", "CVI_I2S0_TX"}, {"2", "CVI_I2S1_RX"}, {"3", "CVI_I2S1_TX"},
        {"4", "CVI_I2S2_RX"}, {"5", "CVI_I2S2_TX"}, {"6", "CVI_I2S3_RX"}, {"7", "CVI_I2S3_TX"},
        {"8", "CVI_UART0_RX"}, {"9", "CVI_UART0_TX"}, {"10", "CVI_UART1_RX"}, {"11", "CVI_UART1_TX"},
        {"12", "CVI_UART2_RX"}, {"13", "CVI_UART2_TX"}, {"14", "CVI_UART3_RX"}, {"15", "CVI_UART3_TX"},
        {"16", "CVI_SPI0_RX"}, {"17", "CVI_SPI0_TX"}, {"18", "CVI_SPI1_RX"}, {"19", "CVI_SPI1_TX"},
        {"20", "CVI_SPI2_RX"}, {"21", "CVI_SPI2_TX"}, {"22", "CVI_SPI3_RX"}, {"23", "CVI_SPI3_TX"},
        {"24", "CVI_I2C0_RX"}, {"25", "CVI_I2C0_TX"}, {"26", "CVI_I2C1_RX"}, {"27", "CVI_I2C1_TX"},
        {"28", "CVI_I2C2_RX"}, {"29", "CVI_I2C2_TX"}, {"30", "CVI_I2C3_RX"}, {"31", "CVI_I2C3_TX"},
        {"32", "CVI_I2C4_RX"}, {"33", "CVI_I2C4_TX"}, {"34", "CVI_TDM0_RX"}, {"35", "CVI_TDM0_TX"},
        {"36", "CVI_TDM1_RX"}, {"37", "CVI_AUDSRC"}, {"38", "CVI_SPI_NOR_RX"}, {"39", "CVI_SPI_NOR_TX"},
        {"40", "CVI_UART4_RX"}, {"41", "CVI_UART4_TX"}, {"42", "CVI_SPI_NAND"}
    };
    
    return numberToNameMap.value(channelNumber, "CVI_I2S0_RX");
}

void DtsConfig::createSysdmaRemapNode(const PeripheralInfo &info)
{
    // 将数字通道转换为常量名
    QStringList channelNames;
    for (const QString &channel : info.sysdmaChannels) {
        channelNames.append(getChannelName(channel));
    }
    
    // 格式化ch-remap为两行：前4个常量名在第一行，后4个在第二行并缩进
    QString chRemapValue;
    if (channelNames.size() >= 8) {
        QStringList firstLine = channelNames.mid(0, 4);
        QStringList secondLine = channelNames.mid(4, 4);
        chRemapValue = QString("%1\n\t\t\t\t\t%2").arg(firstLine.join(" "), secondLine.join(" "));
    } else {
        chRemapValue = channelNames.join(" ");
    }
    
    // 创建新的sysdma_remap节点，使用标准DTS格式（不是覆盖格式）
    QString newNode = QString(
        "\n\n"
        "sysdma_remap {\n"
        "\tcompatible = \"cvitek,sysdma_remap\";\n"
        "\treg = <0x0 0x03000154 0x0 0x10>;\n"
        "\tstatus = \"%1\";\n"
        "\tch-remap = <%2>;\n"
        "\tint_mux_base = <0x03000298>;\n"
        "\tint_mux = <0x1FF>; /* enable bit [0..8] for CPU0(CA53) */\n"
        "};\n"
    ).arg(info.status, chRemapValue);
    
    // 在文件末尾添加新节点
    m_fileContent.append(newNode);
    
    qDebug() << "创建sysdma_remap节点:" << newNode;
}

void DtsConfig::updatePeripheralDmaConfigWithPrevious(const QStringList &previousChannels, const QStringList &sysdmaChannels)
{
    // 默认SYSDMA通道映射
    QStringList defaultChannels = {"0", "5", "2", "3", "42", "42", "4", "7"};
    
    // 智能清除：只清除那些通道映射发生变化的外设
    clearChangedPeripheralDmaConfigs(previousChannels, sysdmaChannels);
    
    // 记录已处理的外设，避免重复
    QSet<QString> processedPeripherals;
    
    // 首先收集所有修改过的通道及其对应的外设
    QMap<QString, QList<QPair<int, QString>>> peripheralChannels; // 外设名 -> [(通道索引, 通道号), ...]
    
    for (int i = 0; i < sysdmaChannels.size() && i < 8; ++i) {
        QString channelNumber = sysdmaChannels[i];
        QString defaultChannel = (i < defaultChannels.size()) ? defaultChannels[i] : "";
        
        // 只有当通道不是默认值时，才处理
        if (channelNumber != defaultChannel) {
            QString peripheralNode = getPeripheralNodeFromChannel(channelNumber);
            if (!peripheralNode.isEmpty()) {
                peripheralChannels[peripheralNode].append(QPair<int, QString>(i, channelNumber));
            } else {
                qDebug() << "通道" << i << "已修改，但未找到对应的外设节点";
            }
        }
    }
    
    // 为每个外设生成DMA配置
    for (auto it = peripheralChannels.begin(); it != peripheralChannels.end(); ++it) {
        const QString &peripheralNode = it.key();
        const QList<QPair<int, QString>> &channels = it.value();
        
        if (channels.size() == 1) {
            // 单通道配置
            int channelIndex = channels[0].first;
            bool isRx = (channelIndex % 2 == 0);
            QString channelType = isRx ? "rx" : "tx";
            addDmaConfigToPeripheral(peripheralNode, QString::number(channelIndex), "", channelType);
            qDebug() << "通道" << channelIndex << "已修改，为外设" << peripheralNode << "添加" << channelType << "DMA配置";
        } else if (channels.size() == 2) {
            // 双通道配置，需要确定哪个是RX，哪个是TX
            int channel1Index = channels[0].first;
            int channel2Index = channels[1].first;
            QString channel1Number = channels[0].second;
            QString channel2Number = channels[1].second;
            
            // 判断RX和TX通道
            bool isChannel1Rx = isChannelRx(channel1Number);
            bool isChannel2Rx = isChannelRx(channel2Number);
            
            QString rxIndex, txIndex;
            if (isChannel1Rx && !isChannel2Rx) {
                rxIndex = QString::number(channel1Index);
                txIndex = QString::number(channel2Index);
            } else if (!isChannel1Rx && isChannel2Rx) {
                rxIndex = QString::number(channel2Index);
                txIndex = QString::number(channel1Index);
            } else {
                // 如果两个都是RX或都是TX，按索引判断（偶数为RX，奇数为TX）
                if (channel1Index % 2 == 0) {
                    rxIndex = QString::number(channel1Index);
                    txIndex = QString::number(channel2Index);
                } else {
                    rxIndex = QString::number(channel2Index);
                    txIndex = QString::number(channel1Index);
                }
            }
            
            addDmaConfigToPeripheral(peripheralNode, rxIndex, txIndex, "txrx");
            qDebug() << "通道" << rxIndex << "(RX)和" << txIndex << "(TX)都已修改，为外设" << peripheralNode << "添加双通道DMA配置";
        } else if (channels.size() > 2) {
            qDebug() << "警告：外设" << peripheralNode << "配置了" << channels.size() << "个通道，只支持最多2个通道";
        }
    }
}

QString DtsConfig::getPeripheralNodeFromChannel(const QString &channelNumber)
{
    // 根据通道号确定对应的外设节点名
    int channel = channelNumber.toInt();
    
    // UART通道映射 (RX通道为偶数，TX通道为奇数)
    if (channel >= 8 && channel <= 15) {
        int uartIndex = (channel - 8) / 2;
        return QString("uart%1").arg(uartIndex);
    }
    // UART4特殊情况
    else if (channel >= 40 && channel <= 41) {
        return "uart4";
    }
    // I2S通道映射
    else if (channel >= 0 && channel <= 7) {
        int i2sIndex = channel / 2;
        return QString("i2s%1").arg(i2sIndex);
    }
    // SPI通道映射
    else if (channel >= 16 && channel <= 23) {
        int spiIndex = (channel - 16) / 2;
        return QString("spi%1").arg(spiIndex);
    }
    // I2C通道映射
    else if (channel >= 24 && channel <= 33) {
        int i2cIndex = (channel - 24) / 2;
        return QString("i2c%1").arg(i2cIndex);
    }
    
    return QString(); // 不支持的通道
}

bool DtsConfig::isChannelRx(const QString &channelNumber)
{
    // 根据通道号判断是RX还是TX通道
    int channel = channelNumber.toInt();
    
    // 大部分通道遵循偶数为RX，奇数为TX的规则
    if ((channel >= 0 && channel <= 7) ||      // I2S
        (channel >= 8 && channel <= 15) ||     // UART0-3
        (channel >= 16 && channel <= 23) ||    // SPI
        (channel >= 24 && channel <= 33) ||    // I2C
        (channel >= 34 && channel <= 36) ||    // TDM
        (channel >= 38 && channel <= 41)) {    // SPI_NOR, UART4
        return (channel % 2 == 0);
    }
    
    // 特殊通道 CVI_AUDSRC (37) 和 CVI_SPI_NAND (42) 没有配对
    return true; // 默认为RX
}

void DtsConfig::addDmaConfigToPeripheral(const QString &peripheralNode, const QString &channelIndex1, const QString &channelIndex2, const QString &capability)
{
    // 查找外设节点
    QPair<int, int> nodePos = findNodePosition(peripheralNode);
    if (nodePos.first == -1 || nodePos.second == -1) {
        qDebug() << "未找到外设节点:" << peripheralNode;
        return;
    }
    
    QString nodeContent = m_fileContent.mid(nodePos.first, nodePos.second - nodePos.first);
    QString newNodeContent = nodeContent;
    
    // 根据capability生成相应的DMA配置
    QString dmasLine, dmaNamesLine, capabilityLine;
    
    if (capability == "txrx") {
        // 双通道配置
        dmasLine = QString("\n\t\tdmas = <&dmac %1 1 1\n\t\t\t&dmac %2 1 1>;").arg(channelIndex1, channelIndex2);
        dmaNamesLine = "\n\t\tdma-names = \"rx\", \"tx\";";
        capabilityLine = "\n\t\tcapability = \"txrx\";";
    } else if (capability == "rx") {
        // 单RX通道配置
        dmasLine = QString("\n\t\tdmas = <&dmac %1 1 1>;").arg(channelIndex1);
        dmaNamesLine = "\n\t\tdma-names = \"rx\";";
        capabilityLine = "\n\t\tcapability = \"rx\";";
    } else if (capability == "tx") {
        // 单TX通道配置
        dmasLine = QString("\n\t\tdmas = <&dmac %1 1 1>;").arg(channelIndex1);
        dmaNamesLine = "\n\t\tdma-names = \"tx\";";
        capabilityLine = "\n\t\tcapability = \"tx\";";
    } else {
        qDebug() << "未知的capability类型:" << capability;
        return;
    }
    
    // 检查是否已经存在DMA配置
    QRegularExpression dmasRegex("\\s*dmas\\s*=\\s*<[^>]+>\\s*;");
    QRegularExpression dmaNamesRegex("\\s*dma-names\\s*=\\s*[^;]+;");
    QRegularExpression capabilityRegex("\\s*capability\\s*=\\s*[^;]+;");
    
    // 更新或添加dmas属性
    if (dmasRegex.match(newNodeContent).hasMatch()) {
        newNodeContent.replace(dmasRegex, dmasLine);
    } else {
        int lastSemicolon = newNodeContent.lastIndexOf(';');
        if (lastSemicolon != -1) {
            newNodeContent.insert(lastSemicolon + 1, dmasLine);
        }
    }
    
    // 更新或添加dma-names属性
    if (dmaNamesRegex.match(newNodeContent).hasMatch()) {
        newNodeContent.replace(dmaNamesRegex, dmaNamesLine);
    } else {
        int lastSemicolon = newNodeContent.lastIndexOf(';');
        if (lastSemicolon != -1) {
            newNodeContent.insert(lastSemicolon + 1, dmaNamesLine);
        }
    }
    
    // 更新或添加capability属性
    if (capabilityRegex.match(newNodeContent).hasMatch()) {
        newNodeContent.replace(capabilityRegex, capabilityLine);
    } else {
        int lastSemicolon = newNodeContent.lastIndexOf(';');
        if (lastSemicolon != -1) {
            newNodeContent.insert(lastSemicolon + 1, capabilityLine);
        }
    }
    
    // 更新文件内容
    m_fileContent.replace(nodePos.first, nodePos.second - nodePos.first, newNodeContent);
    
    qDebug() << "为外设节点" << peripheralNode << "添加" << capability << "DMA配置，通道:" << channelIndex1 << (channelIndex2.isEmpty() ? "" : ("," + channelIndex2));
}

void DtsConfig::clearChangedPeripheralDmaConfigs(const QStringList &previousChannels, const QStringList &newChannels)
{
    // 收集需要清除DMA配置的外设
    QSet<QString> peripheralsToClean;
    
    // 检查每个通道位置，找出哪些外设的通道映射发生了变化
    for (int i = 0; i < 8; ++i) {
        QString previousChannel = (i < previousChannels.size()) ? previousChannels[i] : "";
        QString newChannel = (i < newChannels.size()) ? newChannels[i] : "";
        
        // 如果通道发生了变化
        if (previousChannel != newChannel) {
            // 添加之前使用该通道的外设（需要清除其DMA配置）
            if (!previousChannel.isEmpty()) {
                QString previousPeripheral = getPeripheralNodeFromChannel(previousChannel);
                if (!previousPeripheral.isEmpty()) {
                    peripheralsToClean.insert(previousPeripheral);
                }
            }
        }
    }
    
    // 清除收集到的外设的DMA配置
    for (const QString &peripheral : peripheralsToClean) {
        QPair<int, int> nodePos = findNodePosition(peripheral);
        if (nodePos.first != -1 && nodePos.second != -1) {
            QString nodeContent = m_fileContent.mid(nodePos.first, nodePos.second - nodePos.first);
            QString newNodeContent = nodeContent;
            
            // 移除DMA相关属性
            QRegularExpression dmasRegex("\\s*dmas\\s*=\\s*<[^>]*>\\s*;");
            QRegularExpression dmaNamesRegex("\\s*dma-names\\s*=\\s*[^;]*;");
            QRegularExpression capabilityRegex("\\s*capability\\s*=\\s*[^;]*;");
            
            bool hasChanges = false;
            
            if (dmasRegex.match(newNodeContent).hasMatch()) {
                newNodeContent.remove(dmasRegex);
                hasChanges = true;
            }
            
            if (dmaNamesRegex.match(newNodeContent).hasMatch()) {
                newNodeContent.remove(dmaNamesRegex);
                hasChanges = true;
            }
            
            if (capabilityRegex.match(newNodeContent).hasMatch()) {
                newNodeContent.remove(capabilityRegex);
                hasChanges = true;
            }
            
            // 只有在确实有变更时才更新文件内容
            if (hasChanges) {
                m_fileContent.replace(nodePos.first, nodePos.second - nodePos.first, newNodeContent);
                qDebug() << "清除外设" << peripheral << "的DMA配置（通道映射已变化）";
            }
        }
    }
}
