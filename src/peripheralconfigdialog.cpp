#include "peripheralconfigdialog.h"
#include <QMessageBox>
#include <QDebug>
#include <QIntValidator>
#include <QDoubleSpinBox>
#include <QAbstractSpinBox>

PeripheralConfigDialog::PeripheralConfigDialog(const QString &peripheralType, DtsConfig *dtsConfig, QWidget *parent)
    : QDialog(parent)
    , m_peripheralType(peripheralType.toLower())
    , m_dtsConfig(dtsConfig)
    , m_mainLayout(nullptr)
    , m_peripheralGroup(nullptr)
    , m_peripheralLayout(nullptr)
    , m_peripheralLabel(nullptr)
    , m_peripheralComboBox(nullptr)
    , m_statusLabel(nullptr)
    , m_statusComboBox(nullptr)
    , m_clockLabel(nullptr)
    , m_clockLineEdit(nullptr)
    , m_freqLabel(nullptr)
    , m_freqSpinBox(nullptr)
    , m_freqDoubleSpinBox(nullptr)
    , m_freqUnitLabel(nullptr)
    , m_pwmCellsLabel(nullptr)
    , m_pwmCellsSpinBox(nullptr)
    , m_currentSpeedLabel(nullptr)
    , m_currentSpeedSpinBox(nullptr)
    , m_buttonLayout(nullptr)
    , m_applyButton(nullptr)
    , m_cancelButton(nullptr)
    , m_baudRateComboBox(nullptr)
    , m_customBaudLineEdit(nullptr)
{
    setupUI();
    updatePeripheralList();
}

void PeripheralConfigDialog::setupUI()
{
    setWindowTitle(QString("%1 外设配置").arg(m_peripheralType.toUpper()));
    setFixedSize(400, 300);
    setModal(true);

    // 创建主布局
    m_mainLayout = new QVBoxLayout(this);

    // 创建外设选择组
    m_peripheralGroup = new QGroupBox(QString("%1 外设配置").arg(m_peripheralType.toUpper()), this);
    m_peripheralLayout = new QGridLayout(m_peripheralGroup);

    int currentRow = 0;

    // 外设选择（所有外设都需要）
    m_peripheralLabel = new QLabel("选择外设:", this);
    m_peripheralComboBox = new QComboBox(this);
    m_peripheralLayout->addWidget(m_peripheralLabel, currentRow, 0);
    m_peripheralLayout->addWidget(m_peripheralComboBox, currentRow, 1);
    currentRow++;

    // 状态配置（所有外设都需要）
    m_statusLabel = new QLabel("状态:", this);
    m_statusComboBox = new QComboBox(this);
    m_statusComboBox->addItems({"okay", "disabled"});
    m_peripheralLayout->addWidget(m_statusLabel, currentRow, 0);
    m_peripheralLayout->addWidget(m_statusComboBox, currentRow, 1);
    currentRow++;

    // 根据外设类型添加特定的配置项
    setupPeripheralSpecificUI(currentRow);

    // 添加组到主布局
    m_mainLayout->addWidget(m_peripheralGroup);

    // 创建按钮布局
    m_buttonLayout = new QHBoxLayout();
    m_applyButton = new QPushButton("应用", this);
    m_cancelButton = new QPushButton("取消", this);

    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(m_applyButton);
    m_buttonLayout->addWidget(m_cancelButton);

    m_mainLayout->addLayout(m_buttonLayout);

    // 连接信号和槽
    setupConnections();

    // 设置样式
    setupStyles();
}

void PeripheralConfigDialog::updatePeripheralList()
{
    m_peripheralComboBox->clear();

    if (!m_dtsConfig) {
        return;
    }

    QMap<QString, PeripheralInfo> allPeripherals = m_dtsConfig->getPeripheralInfos();

    // 筛选当前外设类型
    for (auto it = allPeripherals.begin(); it != allPeripherals.end(); ++it) {
        const QString &peripheralName = it.key();

        if (peripheralName.startsWith(m_peripheralType)) {
            m_peripheralComboBox->addItem(peripheralName);
        }
    }

    // 如果有外设，选择第一个
    if (m_peripheralComboBox->count() > 0) {
        m_peripheralComboBox->setCurrentIndex(0);
        onPeripheralSelectionChanged();
    }
}

void PeripheralConfigDialog::setupPeripheralSpecificUI(int &currentRow)
{
    // 根据外设类型显示不同的配置项
    if (m_peripheralType == "i2c") {
        // I2C: 显示时钟名称和时钟频率
        m_clockLabel = new QLabel("时钟名称:", this);
        m_clockLineEdit = new QLineEdit(this);
        m_clockLineEdit->setPlaceholderText("例如: I2C");
        m_peripheralLayout->addWidget(m_clockLabel, currentRow, 0);
        m_peripheralLayout->addWidget(m_clockLineEdit, currentRow, 1);
        currentRow++;

        m_freqLabel = new QLabel("时钟频率:", this);
        m_freqSpinBox = new QSpinBox(this);
        // 改为 KHz 单位，仅保留自由输入框
        m_freqSpinBox->setRange(1, 500000); // KHz
        m_freqSpinBox->setSuffix(" KHz");
        m_freqSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
        m_freqSpinBox->setValue(400);  // I2C 默认 400 KHz
        m_peripheralLayout->addWidget(m_freqLabel, currentRow, 0);
        m_peripheralLayout->addWidget(m_freqSpinBox, currentRow, 1);
        currentRow++;

    } else if (m_peripheralType == "uart") {
        // UART: 时钟频率只填写数字，单位为 MHz（无需选择单位）
        m_freqLabel = new QLabel("时钟频率:", this);
        m_freqDoubleSpinBox = new QDoubleSpinBox(this);
        m_freqDoubleSpinBox->setRange(0.01, 1000.0); // MHz
        m_freqDoubleSpinBox->setDecimals(3);
        m_freqDoubleSpinBox->setSingleStep(0.1);
        m_freqDoubleSpinBox->setSuffix(" MHz");
        m_freqDoubleSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
        m_freqDoubleSpinBox->setValue(25.0); // 默认 25 MHz
        m_peripheralLayout->addWidget(m_freqLabel, currentRow, 0);
        m_peripheralLayout->addWidget(m_freqDoubleSpinBox, currentRow, 1);
        currentRow++;

        m_currentSpeedLabel = new QLabel("波特率:", this);

        // 下拉框：固定选项 + 自定义
        m_baudRateComboBox = new QComboBox(this);
        QStringList baudItems = {
            "1200","2400","4800","9600","19200","38400","57600",
            "115200","230400","460800","921600","自定义"
        };
        m_baudRateComboBox->addItems(baudItems);
        m_baudRateComboBox->setCurrentText("115200");

        // 自定义输入，仅在选择“自定义”时可见
        m_customBaudLineEdit = new QLineEdit(this);
        m_customBaudLineEdit->setPlaceholderText("输入自定义波特率（bps）");
        m_customBaudLineEdit->setValidator(new QIntValidator(1, 3000000, this));
        m_customBaudLineEdit->setVisible(false);

        // 布局：把下拉和自定义输入放在同一列（下拉优先）
        m_peripheralLayout->addWidget(m_currentSpeedLabel, currentRow, 0);
        m_peripheralLayout->addWidget(m_baudRateComboBox, currentRow, 1);
        currentRow++;
        m_peripheralLayout->addWidget(new QLabel("", this), currentRow, 0); // 占位
        m_peripheralLayout->addWidget(m_customBaudLineEdit, currentRow, 1);
        currentRow++;

    } else if (m_peripheralType == "pwm") {
        // PWM: 显示时钟名称和PWM cells
        m_clockLabel = new QLabel("时钟名称:", this);
        m_clockLineEdit = new QLineEdit(this);
        m_clockLineEdit->setPlaceholderText("例如: PWM");
        m_peripheralLayout->addWidget(m_clockLabel, currentRow, 0);
        m_peripheralLayout->addWidget(m_clockLineEdit, currentRow, 1);
        currentRow++;

        m_pwmCellsLabel = new QLabel("PWM Cells:", this);
        m_pwmCellsSpinBox = new QSpinBox(this);
        m_pwmCellsSpinBox->setRange(1, 10);
        m_pwmCellsSpinBox->setValue(1);  // 默认值
        m_peripheralLayout->addWidget(m_pwmCellsLabel, currentRow, 0);
        m_peripheralLayout->addWidget(m_pwmCellsSpinBox, currentRow, 1);
        currentRow++;

    } else if (m_peripheralType == "spi") {
        // SPI: 只显示时钟名称
        m_clockLabel = new QLabel("时钟名称:", this);
        m_clockLineEdit = new QLineEdit(this);
        m_clockLineEdit->setPlaceholderText("例如: SPI");
        m_peripheralLayout->addWidget(m_clockLabel, currentRow, 0);
        m_peripheralLayout->addWidget(m_clockLineEdit, currentRow, 1);
        currentRow++;

    } else if (m_peripheralType == "gpio" || m_peripheralType == "saradc") {
        // GPIO和SARADC: 只有状态配置，不需要额外控件
        // 这里不添加任何额外控件
    }
}

void PeripheralConfigDialog::setupConnections()
{
    // 基本连接
    connect(m_peripheralComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PeripheralConfigDialog::onPeripheralSelectionChanged);
    connect(m_statusComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PeripheralConfigDialog::onStatusChanged);
    connect(m_applyButton, &QPushButton::clicked, this, &PeripheralConfigDialog::onApplyClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &PeripheralConfigDialog::onCancelClicked);

    // 根据外设类型连接特定信号
    if (m_clockLineEdit) {
        connect(m_clockLineEdit, &QLineEdit::textChanged, this, &PeripheralConfigDialog::onClockChanged);
    }

    if (m_freqSpinBox) {
        connect(m_freqSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                this, &PeripheralConfigDialog::onFreqChanged);
    }
    if (m_freqDoubleSpinBox) {
        connect(m_freqDoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, &PeripheralConfigDialog::onFreqChanged);
    }

    if (m_pwmCellsSpinBox) {
        connect(m_pwmCellsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                this, &PeripheralConfigDialog::onPwmCellsChanged);
    }

    if (m_baudRateComboBox) {
        connect(m_baudRateComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                [this](int idx){
                    if (!m_baudRateComboBox) return;
                    bool isCustom = (m_baudRateComboBox->currentText() == "自定义");
                    if (m_customBaudLineEdit) m_customBaudLineEdit->setVisible(isCustom);
                    onCurrentSpeedChanged();
                });
    }

    if (m_customBaudLineEdit) {
        connect(m_customBaudLineEdit, &QLineEdit::textChanged, this, &PeripheralConfigDialog::onCurrentSpeedChanged);
    }
}

void PeripheralConfigDialog::loadPeripheralConfig()
{
    if (m_currentPeripheral.isEmpty() || !m_dtsConfig) {
        return;
    }

    // 获取保存的信息（可能为空结构）
    PeripheralInfo info = m_dtsConfig->getPeripheralInfo(m_currentPeripheral);

    // 屏蔽信号，防止 setXXX 触发回调并导致递归写入或闪退
    QList<QObject*> toBlock = {
        m_peripheralComboBox, m_statusComboBox,
        m_clockLineEdit, m_freqSpinBox, m_freqDoubleSpinBox,
        m_pwmCellsSpinBox, m_baudRateComboBox, m_customBaudLineEdit
    };
    for (QObject* o : toBlock) {
        if (o) o->blockSignals(true);
    }

    // 更新外设选择（如果有）
    if (m_peripheralComboBox) {
        int idx = m_peripheralComboBox->findText(m_currentPeripheral);
        if (idx >= 0) m_peripheralComboBox->setCurrentIndex(idx);
    }

    // 状态
    if (m_statusComboBox) {
        if (!info.status.isEmpty() && m_statusComboBox->findText(info.status) >= 0) {
            m_statusComboBox->setCurrentText(info.status);
        } else {
            // 保持现有或默认
        }
    }

    // 时钟名称
    if (m_clockLineEdit) {
        m_clockLineEdit->setText(info.clockName);
    }

    // I2C 使用 m_freqSpinBox（单位 KHz）
    if (m_freqSpinBox) {
        if (info.hasClockFreq && info.clockFrequency > 0) {
            // 限制到控件范围
            int v = info.clockFrequency / 1000; // Hz -> KHz
            if (v < m_freqSpinBox->minimum()) v = m_freqSpinBox->minimum();
            if (v > m_freqSpinBox->maximum()) v = m_freqSpinBox->maximum();
            m_freqSpinBox->setValue(v);
        } else {
            m_freqSpinBox->setValue(m_freqSpinBox->minimum());
        }
    }

    // UART 使用 m_freqDoubleSpinBox（单位 MHz）
    if (m_freqDoubleSpinBox) {
        if (info.hasClockFreq && info.clockFrequency > 0) {
            double mhz = static_cast<double>(info.clockFrequency) / 1e6;
            if (mhz < m_freqDoubleSpinBox->minimum()) mhz = m_freqDoubleSpinBox->minimum();
            if (mhz > m_freqDoubleSpinBox->maximum()) mhz = m_freqDoubleSpinBox->maximum();
            m_freqDoubleSpinBox->setValue(mhz);
        } else {
            m_freqDoubleSpinBox->setValue(m_freqDoubleSpinBox->minimum());
        }
    }

    // PWM cells
    if (m_pwmCellsSpinBox) {
        if (info.hasPwmCells && info.pwmCells > 0) {
            int v = info.pwmCells;
            if (v < m_pwmCellsSpinBox->minimum()) v = m_pwmCellsSpinBox->minimum();
            if (v > m_pwmCellsSpinBox->maximum()) v = m_pwmCellsSpinBox->maximum();
            m_pwmCellsSpinBox->setValue(v);
        } else {
            m_pwmCellsSpinBox->setValue(m_pwmCellsSpinBox->minimum());
        }
    }

    // 波特率：预设或自定义
    if (m_baudRateComboBox) {
        int saved = (info.hasCurrentSpeed && info.currentSpeed > 0) ? info.currentSpeed : 115200;
        QString savedStr = QString::number(saved);
        int found = m_baudRateComboBox->findText(savedStr);
        if (found >= 0) {
            m_baudRateComboBox->setCurrentIndex(found);
            if (m_customBaudLineEdit) m_customBaudLineEdit->setVisible(false);
        } else {
            int customIdx = m_baudRateComboBox->findText(QStringLiteral("自定义"));
            if (customIdx >= 0) m_baudRateComboBox->setCurrentIndex(customIdx);
            if (m_customBaudLineEdit) {
                m_customBaudLineEdit->setText(savedStr);
                m_customBaudLineEdit->setVisible(true);
            }
        }
    } else if (m_currentSpeedSpinBox) {
        if (info.hasCurrentSpeed && info.currentSpeed > 0) {
            m_currentSpeedSpinBox->setValue(info.currentSpeed);
        } else {
            m_currentSpeedSpinBox->setValue(m_currentSpeedSpinBox->minimum());
        }
    }

    // 恢复信号
    for (QObject* o : toBlock) {
        if (o) o->blockSignals(false);
    }
}

void PeripheralConfigDialog::savePeripheralConfig()
{
    if (m_currentPeripheral.isEmpty() || !m_dtsConfig) {
        return;
    }

    // 保存状态
    m_dtsConfig->setPeripheralStatus(m_currentPeripheral, m_statusComboBox->currentText());

    // 保存时钟配置（如果支持）
    if (m_clockLineEdit && !m_clockLineEdit->text().isEmpty()) {
        QString clockFreq;
        if (m_freqDoubleSpinBox) {
            // UART: 存储为 Hz（从 MHz 转换）
            double mhz = m_freqDoubleSpinBox->value();
            qint64 hz = static_cast<qint64>(mhz * 1e6);
            clockFreq = QString::number(hz);
        } else if (m_freqSpinBox) {
            // I2C: 存储为 Hz（从 KHz 转换）
            qint64 hz = static_cast<qint64>(m_freqSpinBox->value()) * 1000;
            clockFreq = QString::number(hz);
        }
        m_dtsConfig->setPeripheralClock(m_currentPeripheral, m_clockLineEdit->text(), clockFreq);
    }

    // 保存时钟频率（如果支持）
    if (m_freqDoubleSpinBox) {
        // 存储为 Hz（从 MHz 转换）
        double mhz = m_freqDoubleSpinBox->value();
        qint64 hz = static_cast<qint64>(mhz * 1e6);
        m_dtsConfig->setPeripheralClockFrequency(m_currentPeripheral, static_cast<int>(hz));
    } else if (m_freqSpinBox) {
        // I2C: 存储为 Hz（从 KHz 转换）
        qint64 hz = static_cast<qint64>(m_freqSpinBox->value()) * 1000;
        m_dtsConfig->setPeripheralClockFrequency(m_currentPeripheral, static_cast<int>(hz));
    }

    // 保存PWM cells（如果支持）
    if (m_pwmCellsSpinBox) {
        m_dtsConfig->setPeripheralPwmCells(m_currentPeripheral, m_pwmCellsSpinBox->value());
    }

    // 保存波特率（如果支持）：优先从下拉/自定义读取
    if (m_baudRateComboBox) {
        QString sel = m_baudRateComboBox->currentText();
        if (sel == "自定义") {
            if (m_customBaudLineEdit && !m_customBaudLineEdit->text().isEmpty()) {
                int v = m_customBaudLineEdit->text().toInt();
                m_dtsConfig->setPeripheralCurrentSpeed(m_currentPeripheral, v);
            }
        } else {
            int v = sel.toInt();
            m_dtsConfig->setPeripheralCurrentSpeed(m_currentPeripheral, v);
        }
    } else if (m_currentSpeedSpinBox) {
        m_dtsConfig->setPeripheralCurrentSpeed(m_currentPeripheral, m_currentSpeedSpinBox->value());
    }
}

void PeripheralConfigDialog::onPeripheralSelectionChanged()
{
    m_currentPeripheral = m_peripheralComboBox->currentText();
    loadPeripheralConfig();
}

void PeripheralConfigDialog::onStatusChanged()
{
    // 实时更新配置
    if (!m_currentPeripheral.isEmpty()) {
        savePeripheralConfig();
    }
}

void PeripheralConfigDialog::onClockChanged()
{
    // 实时更新时钟配置
    if (!m_currentPeripheral.isEmpty()) {
        savePeripheralConfig();
    }
}

void PeripheralConfigDialog::onFreqChanged()
{
    // 实时更新频率配置
    if (!m_currentPeripheral.isEmpty()) {
        savePeripheralConfig();
    }
}

void PeripheralConfigDialog::onPwmCellsChanged()
{
    // 实时更新PWM cells配置
    if (!m_currentPeripheral.isEmpty()) {
        savePeripheralConfig();
    }
}

void PeripheralConfigDialog::onCurrentSpeedChanged()
{
    // 实时更新波特率配置
    if (!m_currentPeripheral.isEmpty()) {
        savePeripheralConfig();
    }
}

void PeripheralConfigDialog::onApplyClicked()
{
    savePeripheralConfig();

    // 只保存当前外设的配置，而不是整个文件
    if (m_dtsConfig->savePeripheralConfig(m_currentPeripheral)) {
        QMessageBox::information(this, "成功", QString("外设 %1 配置已保存到设备树文件！").arg(m_currentPeripheral));
        accept();
    } else {
        QMessageBox::critical(this, "错误", "无法保存设备树文件！");
    }
}

void PeripheralConfigDialog::onCancelClicked()
{
    reject();
}

void PeripheralConfigDialog::setupStyles()
{
    setStyleSheet(
        "QGroupBox { "
        "font-size: 14px; "
        "font-weight: bold; "
        "border: 2px solid #cccccc; "
        "border-radius: 8px; "
        "margin-top: 10px; "
        "padding-top: 15px; "
        "} "
        "QGroupBox::title { "
        "subcontrol-origin: margin; "
        "subcontrol-position: top center; "
        "padding: 0 5px; "
        "} "
        "QPushButton { "
        "background-color: #3498db; "
        "color: white; "
        "border: none; "
        "padding: 8px 16px; "
        "font-size: 12px; "
        "border-radius: 4px; "
        "min-width: 80px; "
        "} "
        "QPushButton:hover { "
        "background-color: #2980b9; "
        "} "
        "QPushButton:pressed { "
        "background-color: #1f5582; "
        "}"
    );
}
