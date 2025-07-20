#include "peripheralconfigdialog.h"
#include <QMessageBox>
#include <QDebug>

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
    , m_freqUnitLabel(nullptr)
    , m_pwmCellsLabel(nullptr)
    , m_pwmCellsSpinBox(nullptr)
    , m_buttonLayout(nullptr)
    , m_applyButton(nullptr)
    , m_cancelButton(nullptr)
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

void PeripheralConfigDialog::loadPeripheralConfig()
{
    if (m_currentPeripheral.isEmpty() || !m_dtsConfig) {
        return;
    }
    
    m_currentInfo = m_dtsConfig->getPeripheralInfo(m_currentPeripheral);
    
    // 更新UI
    m_statusComboBox->setCurrentText(m_currentInfo.status);
    
    // 根据外设类型更新相关控件
    if (m_clockLineEdit) {
        m_clockLineEdit->setText(m_currentInfo.clockName);
    }
    
    if (m_freqSpinBox) {
        // 设置时钟频率
        if (m_currentInfo.hasClockFreq && m_currentInfo.clockFrequency > 0) {
            m_freqSpinBox->setValue(m_currentInfo.clockFrequency);
        }
    }
    
    if (m_pwmCellsSpinBox) {
        // 设置PWM cells
        if (m_currentInfo.hasPwmCells && m_currentInfo.pwmCells > 0) {
            m_pwmCellsSpinBox->setValue(m_currentInfo.pwmCells);
        }
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
        QString clockFreq = m_freqSpinBox ? QString::number(m_freqSpinBox->value()) : "";
        m_dtsConfig->setPeripheralClock(m_currentPeripheral, m_clockLineEdit->text(), clockFreq);
    }
    
    // 保存时钟频率（如果支持）
    if (m_freqSpinBox) {
        m_dtsConfig->setPeripheralClockFrequency(m_currentPeripheral, m_freqSpinBox->value());
    }
    
    // 保存PWM cells（如果支持）
    if (m_pwmCellsSpinBox) {
        m_dtsConfig->setPeripheralPwmCells(m_currentPeripheral, m_pwmCellsSpinBox->value());
    }
    
    // 保存PWM cells（如果支持）
    bool hasPwmCellsSupport = (m_peripheralType == "pwm");
    if (hasPwmCellsSupport) {
        m_dtsConfig->setPeripheralPwmCells(m_currentPeripheral, m_pwmCellsSpinBox->value());
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
        m_freqSpinBox->setRange(1000, 500000000);
        m_freqSpinBox->setSuffix(" Hz");
        m_freqSpinBox->setValue(400000);  // I2C默认400kHz
        m_peripheralLayout->addWidget(m_freqLabel, currentRow, 0);
        m_peripheralLayout->addWidget(m_freqSpinBox, currentRow, 1);
        currentRow++;
        
    } else if (m_peripheralType == "uart") {
        // UART: 显示时钟频率
        m_freqLabel = new QLabel("时钟频率:", this);
        m_freqSpinBox = new QSpinBox(this);
        m_freqSpinBox->setRange(1000, 500000000);
        m_freqSpinBox->setSuffix(" Hz");
        m_freqSpinBox->setValue(25000000);  // UART默认25MHz
        m_peripheralLayout->addWidget(m_freqLabel, currentRow, 0);
        m_peripheralLayout->addWidget(m_freqSpinBox, currentRow, 1);
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
    
    if (m_pwmCellsSpinBox) {
        connect(m_pwmCellsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), 
                this, &PeripheralConfigDialog::onPwmCellsChanged);
    }
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
