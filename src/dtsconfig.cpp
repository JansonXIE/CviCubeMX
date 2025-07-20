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
        "saradc[0-9]*:"
    };
    
    for (const QString &pattern : peripheralPatterns) {
        QRegularExpression regex(pattern);
        QRegularExpressionMatchIterator iterator = regex.globalMatch(m_fileContent);
        
        while (iterator.hasNext()) {
            QRegularExpressionMatch match = iterator.next();
            QString nodeName = match.captured(0).chopped(1); // 移除末尾的冒号
            
            QPair<int, int> nodePos = findNodePosition(nodeName);
            if (nodePos.first != -1 && nodePos.second != -1) {
                PeripheralInfo info = parseNode(nodeName, nodePos.first, nodePos.second);
                info.name = nodeName;
                m_peripherals[nodeName] = info;
            }
        }
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
    
    const PeripheralInfo &info = m_peripherals[peripheral];
    
    QPair<int, int> nodePos = findNodePosition(peripheral);
    if (nodePos.first == -1 || nodePos.second == -1) {
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
    
    // 更新文件内容
    m_fileContent.replace(nodePos.first, nodePos.second - nodePos.first, newNodeContent);
}

QPair<int, int> DtsConfig::findNodePosition(const QString &nodeName)
{
    // 查找节点开始位置
    QRegularExpression nodeRegex(QString("\\b%1\\s*:\\s*[^{]*\\{").arg(QRegularExpression::escape(nodeName)));
    QRegularExpressionMatch match = nodeRegex.match(m_fileContent);
    
    if (!match.hasMatch()) {
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
