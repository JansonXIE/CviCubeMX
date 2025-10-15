#include "peripheralconfigdialog.h"
#include <QMessageBox>
#include <QDebug>
#include <QIntValidator>
#include <QDoubleSpinBox>
#include <QAbstractSpinBox>
#include <QRegularExpression>

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
    // Thermal 指针全部置空，避免 setupConnections 误连接
    , m_thermalZoneLabel(nullptr)
    , m_thermalZoneCombo(nullptr)
    , m_pdpLabel(nullptr)
    , m_pdpSpin(nullptr)
    , m_pdLabel(nullptr)
    , m_pdSpin(nullptr)
    , m_sensorLabel(nullptr)
    , m_sensorCombo(nullptr)
    , m_tripSelectLabel(nullptr)
    , m_tripSelectCombo(nullptr)
    , m_tripTempLabel(nullptr)
    , m_tripTempSpin(nullptr)
    , m_tripHystLabel(nullptr)
    , m_tripHystSpin(nullptr)
    , m_tripTypeLabel(nullptr)
    , m_tripTypeCombo(nullptr)
    , m_mapInfoLabel(nullptr)
{
    // 初始化SYSDMA通道控件指针为nullptr
    for (int i = 0; i < 8; i++) {
        m_sysdmaChannelLabels[i] = nullptr;
        m_sysdmaChannelComboBoxes[i] = nullptr;
    }
    setupUI();
    updatePeripheralList();
}


void PeripheralConfigDialog::setupUI()
{
    setWindowTitle(QString("%1 外设配置").arg(m_peripheralType.toUpper()));
    setFixedSize(400, 400);
    setModal(true);

    // 创建主布局
    m_mainLayout = new QVBoxLayout(this);

    // 创建外设选择组
    m_peripheralGroup = new QGroupBox(QString("%1 外设配置").arg(m_peripheralType.toUpper()), this);
    m_peripheralLayout = new QGridLayout(m_peripheralGroup);

    int currentRow = 0;

    // 外设选择与状态配置
    m_peripheralLabel = new QLabel("选择外设:", this);
    m_peripheralComboBox = new QComboBox(this);
    m_statusLabel = new QLabel("状态:", this);
    m_statusComboBox = new QComboBox(this);
    m_statusComboBox->addItems({"okay", "disabled"});
    if (m_peripheralType == "sysdma") {
        // sysdma: 不显示外设选择与状态栏
        m_peripheralLabel->setVisible(false);
        m_peripheralComboBox->setVisible(false);
        m_statusLabel->setVisible(false);
        m_statusComboBox->setVisible(false);
    } else if (m_peripheralType == "thermal") {
        // thermal: 不显示“选择外设”，仅显示“状态”
        m_peripheralLabel->setVisible(false);
        m_peripheralComboBox->setVisible(false);
        m_peripheralLayout->addWidget(m_statusLabel, currentRow, 0);
        m_peripheralLayout->addWidget(m_statusComboBox, currentRow, 1);
        currentRow++;
    } else {
        m_peripheralLayout->addWidget(m_peripheralLabel, currentRow, 0);
        m_peripheralLayout->addWidget(m_peripheralComboBox, currentRow, 1);
        currentRow++;
        m_peripheralLayout->addWidget(m_statusLabel, currentRow, 0);
        m_peripheralLayout->addWidget(m_statusComboBox, currentRow, 1);
        currentRow++;
    }

    // 根据外设类型添加特定的配置项
    setupPeripheralSpecificUI(currentRow);

    // 添加组到主布局（顶部对齐，不再垂直居中）
    m_mainLayout->addWidget(m_peripheralGroup, 0, Qt::AlignTop);

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

    if (m_peripheralType == "sysdma") {
        // 直接定位到 sysdma_remap
        m_currentPeripheral = "sysdma_remap";
        onPeripheralSelectionChanged();
        return;
    }
    if (m_peripheralType == "thermal") {
        // thermal 节点用于写 status，直接加载，不触发下拉槽
        m_currentPeripheral = "thermal";
        loadPeripheralConfig();
        return;
    }

    QMap<QString, PeripheralInfo> allPeripherals = m_dtsConfig->getPeripheralInfos();

    // 筛选当前外设类型
    for (auto it = allPeripherals.begin(); it != allPeripherals.end(); ++it) {
        const QString &peripheralName = it.key();

        // ADC 使用 "saradc" 节点前缀
        if (m_peripheralType == "adc") {
            if (peripheralName.startsWith("saradc", Qt::CaseInsensitive)) {
                m_peripheralComboBox->addItem(peripheralName);
            }
            continue;
        }

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
        m_clockLineEdit->setReadOnly(true); // 只显示不可改
        m_clockLineEdit->setStyleSheet("QLineEdit{background:transparent;border:0;padding:0;}");
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
        m_clockLineEdit->setReadOnly(true); // 只显示不可改
        m_clockLineEdit->setStyleSheet("QLineEdit{background:transparent;border:0;padding:0;}");
        m_peripheralLayout->addWidget(m_clockLabel, currentRow, 0);
        m_peripheralLayout->addWidget(m_clockLineEdit, currentRow, 1);
        currentRow++;

        m_pwmCellsLabel = new QLabel("PWM Cells:", this);
        m_pwmCellsSpinBox = new QSpinBox(this);
        m_pwmCellsSpinBox->setRange(1, 10);
        m_pwmCellsSpinBox->setValue(1);  // 默认值
        m_pwmCellsSpinBox->setReadOnly(false); // 还原为可修改
        m_peripheralLayout->addWidget(m_pwmCellsLabel, currentRow, 0);
        m_peripheralLayout->addWidget(m_pwmCellsSpinBox, currentRow, 1);
        currentRow++;

    } else if (m_peripheralType == "spi") {
        // SPI: 只显示时钟名称
        m_clockLabel = new QLabel("时钟名称:", this);
        m_clockLineEdit = new QLineEdit(this);
        m_clockLineEdit->setPlaceholderText("例如: SPI");
        m_clockLineEdit->setReadOnly(true);
        m_clockLineEdit->setStyleSheet("QLineEdit{background:transparent;border:0;padding:0;}");
        m_peripheralLayout->addWidget(m_clockLabel, currentRow, 0);
        m_peripheralLayout->addWidget(m_clockLineEdit, currentRow, 1);
        currentRow++;

    } else if (m_peripheralType == "sysdma") {
        // SYSDMA: 显示8个通道的映射配置
        QStringList channelOptions = {
            "CVI_I2S0_RX (0)", "CVI_I2S0_TX (1)", "CVI_I2S1_RX (2)", "CVI_I2S1_TX (3)",
            "CVI_I2S2_RX (4)", "CVI_I2S2_TX (5)", "CVI_I2S3_RX (6)", "CVI_I2S3_TX (7)",
            "CVI_UART0_RX (8)", "CVI_UART0_TX (9)", "CVI_UART1_RX (10)", "CVI_UART1_TX (11)",
            "CVI_UART2_RX (12)", "CVI_UART2_TX (13)", "CVI_UART3_RX (14)", "CVI_UART3_TX (15)",
            "CVI_SPI0_RX (16)", "CVI_SPI0_TX (17)", "CVI_SPI1_RX (18)", "CVI_SPI1_TX (19)",
            "CVI_SPI2_RX (20)", "CVI_SPI2_TX (21)", "CVI_SPI3_RX (22)", "CVI_SPI3_TX (23)",
            "CVI_I2C0_RX (24)", "CVI_I2C0_TX (25)", "CVI_I2C1_RX (26)", "CVI_I2C1_TX (27)",
            "CVI_I2C2_RX (28)", "CVI_I2C2_TX (29)", "CVI_I2C3_RX (30)", "CVI_I2C3_TX (31)",
            "CVI_I2C4_RX (32)", "CVI_I2C4_TX (33)", "CVI_TDM0_RX (34)", "CVI_TDM0_TX (35)",
            "CVI_TDM1_RX (36)", "CVI_AUDSRC (37)", "CVI_SPI_NOR_RX (38)", "CVI_SPI_NOR_TX (39)",
            "CVI_UART4_RX (40)", "CVI_UART4_TX (41)", "CVI_SPI_NAND (42)"
        };

        // 默认通道映射
        QStringList defaultChannels = {
            "CVI_I2S0_RX (0)", "CVI_I2S2_TX (5)", "CVI_I2S1_RX (2)", "CVI_I2S1_TX (3)",
            "CVI_SPI_NAND (42)", "CVI_SPI_NAND (42)", "CVI_I2S2_RX (4)", "CVI_I2S3_TX (7)"
        };

        for (int i = 0; i < 8; i++) {
            m_sysdmaChannelLabels[i] = new QLabel(QString("通道 %1:").arg(i), this);
            m_sysdmaChannelComboBoxes[i] = new QComboBox(this);
            m_sysdmaChannelComboBoxes[i]->addItems(channelOptions);

            // 设置默认值
            if (i < defaultChannels.size()) {
                int index = m_sysdmaChannelComboBoxes[i]->findText(defaultChannels[i]);
                if (index >= 0) {
                    m_sysdmaChannelComboBoxes[i]->setCurrentIndex(index);
                }
            }

            m_peripheralLayout->addWidget(m_sysdmaChannelLabels[i], currentRow, 0);
            m_peripheralLayout->addWidget(m_sysdmaChannelComboBoxes[i], currentRow, 1);
            currentRow++;
        }

    } else if (m_peripheralType == "gpio" || m_peripheralType == "saradc") {
        // GPIO和SARADC: 只有状态配置，不需要额外控件
        // 这里不添加任何额外控件
    } else if (m_peripheralType == "thermal") {
        // Thermal: 固定使用 soc_thermal_0；显示中文标签
        m_pdpLabel = new QLabel("被动轮询间隔(ms):", this);
        m_pdpSpin = new QSpinBox(this); m_pdpSpin->setRange(0, 100000);
        m_peripheralLayout->addWidget(m_pdpLabel, currentRow, 0);
        m_peripheralLayout->addWidget(m_pdpSpin, currentRow, 1);
        currentRow++;

        m_pdLabel = new QLabel("主动轮询间隔(ms):", this);
        m_pdSpin = new QSpinBox(this); m_pdSpin->setRange(0, 100000);
        m_peripheralLayout->addWidget(m_pdLabel, currentRow, 0);
        m_peripheralLayout->addWidget(m_pdSpin, currentRow, 1);
        currentRow++;

        m_sensorLabel = new QLabel("温度传感器:", this);
        m_sensorCombo = new QComboBox(this);
        // 当前仅 thermal0
        m_sensorCombo->addItems({"thermal0"});
        m_peripheralLayout->addWidget(m_sensorLabel, currentRow, 0);
        m_peripheralLayout->addWidget(m_sensorCombo, currentRow, 1);
        currentRow++;

        m_tripSelectLabel = new QLabel("阈值项:", this);
        m_tripSelectCombo = new QComboBox(this);
        m_tripSelectCombo->addItems({"soc_thermal_trip_0", "soc_thermal_trip_1", "soc_thermal_crtical_0"});
        m_peripheralLayout->addWidget(m_tripSelectLabel, currentRow, 0);
        m_peripheralLayout->addWidget(m_tripSelectCombo, currentRow, 1);
        currentRow++;

        m_tripTempLabel = new QLabel("温度(毫摄氏度):", this);
        m_tripTempSpin = new QSpinBox(this); m_tripTempSpin->setRange(0, 200000);
        m_peripheralLayout->addWidget(m_tripTempLabel, currentRow, 0);
        m_peripheralLayout->addWidget(m_tripTempSpin, currentRow, 1);
        currentRow++;

        m_tripHystLabel = new QLabel("回滞(毫摄氏度):", this);
        m_tripHystSpin = new QSpinBox(this); m_tripHystSpin->setRange(0, 50000);
        m_peripheralLayout->addWidget(m_tripHystLabel, currentRow, 0);
        m_peripheralLayout->addWidget(m_tripHystSpin, currentRow, 1);
        currentRow++;

        m_tripTypeLabel = new QLabel("类型:", this);
        m_tripTypeCombo = new QComboBox(this);
        // 类型使用英文
        m_tripTypeCombo->addItems({"passive", "critical"});
        m_peripheralLayout->addWidget(m_tripTypeLabel, currentRow, 0);
        m_peripheralLayout->addWidget(m_tripTypeCombo, currentRow, 1);
        currentRow++;

        // 仅显示所选阈值项对应策略的频率（MHz）- 先下线
        m_mapInfoLabel = new QLabel("", this);
        m_mapInfoLabel->setVisible(false);
        m_peripheralLayout->addWidget(m_mapInfoLabel, currentRow, 0);
        m_peripheralLayout->addWidget(new QLabel("", this), currentRow, 1);
        currentRow++;
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
    if (m_peripheralType == "thermal") {
        if (m_thermalZoneCombo) {
            connect(m_thermalZoneCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int){ loadPeripheralConfig(); });
        }
        if (m_tripSelectCombo) {
            connect(m_tripSelectCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PeripheralConfigDialog::onTripSelectionChanged);
        }
        if (m_pdpSpin) connect(m_pdpSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &PeripheralConfigDialog::onFreqChanged);
        if (m_pdSpin)  connect(m_pdSpin,  QOverload<int>::of(&QSpinBox::valueChanged), this, &PeripheralConfigDialog::onFreqChanged);
        if (m_sensorCombo) connect(m_sensorCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PeripheralConfigDialog::onThermalTripChanged);
        if (m_tripTempSpin) connect(m_tripTempSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &PeripheralConfigDialog::onThermalTripChanged);
        if (m_tripHystSpin) connect(m_tripHystSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &PeripheralConfigDialog::onThermalTripChanged);
        if (m_tripTypeCombo) connect(m_tripTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PeripheralConfigDialog::onThermalTripChanged);
    }
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

    // SYSDMA通道变化连接
    for (int i = 0; i < 8; i++) {
        if (m_sysdmaChannelComboBoxes[i]) {
            connect(m_sysdmaChannelComboBoxes[i], QOverload<int>::of(&QComboBox::currentIndexChanged),
                    this, &PeripheralConfigDialog::onSysdmaChannelChanged);
        }
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

    // 添加SYSDMA通道控件到屏蔽列表
    for (int i = 0; i < 8; i++) {
        if (m_sysdmaChannelComboBoxes[i]) {
            toBlock.append(m_sysdmaChannelComboBoxes[i]);
        }
    }

    // Thermal 控件屏蔽，避免加载时触发保存
    if (m_peripheralType == "thermal") {
        if (m_pdpSpin) toBlock.append(m_pdpSpin);
        if (m_pdSpin) toBlock.append(m_pdSpin);
        if (m_sensorCombo) toBlock.append(m_sensorCombo);
        if (m_tripSelectCombo) toBlock.append(m_tripSelectCombo);
        if (m_tripTempSpin) toBlock.append(m_tripTempSpin);
        if (m_tripHystSpin) toBlock.append(m_tripHystSpin);
        if (m_tripTypeCombo) toBlock.append(m_tripTypeCombo);
    }

    for (QObject* o : toBlock) {
        if (o) o->blockSignals(true);
    }

    // 更新外设选择（如果有）
    if (m_peripheralComboBox) {
        int idx = m_peripheralComboBox->findText(m_currentPeripheral);
        if (idx >= 0) m_peripheralComboBox->setCurrentIndex(idx);
    }

    // Thermal: 加载所选 zone 的参数与 trips/maps
    if (m_peripheralType == "thermal" && m_dtsConfig) {
        if (!m_dtsConfig->hasThermalZones()) {
            // 无 thermal-zones，避免后续空访问
            for (QObject* o : toBlock) { if (o) o->blockSignals(false); }
            return;
        }
        // 固定使用 soc_thermal_0
        QString zoneName = "soc_thermal_0";
        ThermalZoneInfo z = m_dtsConfig->getThermalZone(zoneName);
        if (z.trips.isEmpty() && z.pollingDelay == 0 && z.pollingDelayPassive == 0 && z.thermalSensorsRef.isEmpty()) {
            // 解析失败或未找到该 zone
            for (QObject* o : toBlock) { if (o) o->blockSignals(false); }
            return;
        }
        if (m_pdpSpin) m_pdpSpin->setValue(z.pollingDelayPassive);
        if (m_pdSpin)  m_pdSpin->setValue(z.pollingDelay);
        if (m_sensorCombo) m_sensorCombo->setCurrentText("thermal0");
        // 默认展示已选 trip 的配置；若无选择，则选第一个
        QString tname = m_tripSelectCombo ? m_tripSelectCombo->currentText() : QString();
        if (tname.isEmpty() && m_tripSelectCombo && m_tripSelectCombo->count() > 0) {
            m_tripSelectCombo->setCurrentIndex(0);
            tname = m_tripSelectCombo->currentText();
        }
        if (!tname.isEmpty() && z.trips.contains(tname)) {
            const ThermalTripInfo &ti = z.trips[tname];
            if (m_tripTempSpin) m_tripTempSpin->setValue(ti.temperature);
            if (m_tripHystSpin) m_tripHystSpin->setValue(ti.hysteresis);
            if (m_tripTypeCombo) {
                int tii = m_tripTypeCombo->findText(ti.type);
                if (tii >= 0) m_tripTypeCombo->setCurrentIndex(tii);
            }
        }
        // 记录当前 trip 名
        m_lastTripName = tname;
        // 仅显示当前选中阈值项对应的频率（MHz）
        if (false && m_mapInfoLabel && m_tripSelectCombo) {
            QString curTrip = m_tripSelectCombo->currentText();
            QPair<int,int> freqs = m_dtsConfig->getCoolingFrequenciesForTrip(zoneName, curTrip);
            if (freqs.first > 0 && freqs.second > 0) {
                m_mapInfoLabel->setText(QString("策略频率: cpu=%1MHz tpu=%2MHz")
                    .arg(freqs.first / 1e6, 0, 'f', 0).arg(freqs.second / 1e6, 0, 'f', 0));
            } else {
                m_mapInfoLabel->setText("策略频率: -");
            }
        }
    }

    // 状态
    if (m_statusComboBox) {
        QString targetStatus = info.status; // 仅由DTS决定（hasStatus?status:"okay"）
        if (!targetStatus.isEmpty() && m_statusComboBox->findText(targetStatus) >= 0) {
            m_statusComboBox->setCurrentText(targetStatus);
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

    // SYSDMA通道配置
    if (m_peripheralType == "sysdma" && info.hasSysdmaChannels) {
        for (int i = 0; i < 8 && i < info.sysdmaChannels.size(); i++) {
            if (m_sysdmaChannelComboBoxes[i]) {
                QString channelNumber = info.sysdmaChannels[i];
                // 查找包含该数字的选项（格式为 "CVI_XXX_XXX (数字)"）
                QString searchPattern = QString("(%1)").arg(channelNumber);
                int index = m_sysdmaChannelComboBoxes[i]->findText(searchPattern, Qt::MatchContains);
                if (index >= 0) {
                    m_sysdmaChannelComboBoxes[i]->setCurrentIndex(index);
                } else {
                    // 如果没有找到，使用默认值（第一个选项）
                    m_sysdmaChannelComboBoxes[i]->setCurrentIndex(0);
                }
            }
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

    // 保存SYSDMA通道映射（如果支持）
    if (m_peripheralType == "sysdma" && m_sysdmaChannelComboBoxes[0]) {
        QStringList channels;
        for (int i = 0; i < 8; i++) {
            if (m_sysdmaChannelComboBoxes[i]) {
                QString selectedText = m_sysdmaChannelComboBoxes[i]->currentText();
                // 从 "CVI_I2S0_RX (0)" 格式中提取数字值
                QRegularExpression re(R"(\((\d+)\))");
                QRegularExpressionMatch match = re.match(selectedText);
                if (match.hasMatch()) {
                    channels.append(match.captured(1));
                } else {
                    channels.append("0"); // 默认值
                }
            }
        }
        m_dtsConfig->setPeripheralSysdmaChannels(m_currentPeripheral, channels);
    }
}

void PeripheralConfigDialog::onPeripheralSelectionChanged()
{
    if (m_peripheralType == "thermal") {
        // thermal 不依赖下拉选择，直接返回
        return;
    }
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

void PeripheralConfigDialog::onSysdmaChannelChanged()
{
    // 实时更新SYSDMA通道配置
    if (!m_currentPeripheral.isEmpty()) {
        savePeripheralConfig();
    }
}

void PeripheralConfigDialog::onThermalTripChanged()
{
    if (!m_dtsConfig || m_peripheralType != "thermal") return;
    // 固定使用 soc_thermal_0
    QString zoneName = "soc_thermal_0";

    // 保存 polling 与 sensor
    if (m_pdpSpin && m_pdSpin) {
        m_dtsConfig->setThermalPolling(zoneName, m_pdpSpin->value(), m_pdSpin->value());
    }
    if (m_sensorCombo) {
        // UI 显示 thermal0，写回 <&thermal 0>
        const QString sensorUi = m_sensorCombo->currentText();
        const QString sensorRef = (sensorUi == "thermal0") ? "&thermal 0" : sensorUi;
        m_dtsConfig->setThermalSensor(zoneName, sensorRef);
    }
    // 保存当前 trip 参数
    if (m_tripSelectCombo && m_tripTempSpin && m_tripHystSpin && m_tripTypeCombo) {
        QString typeEn = m_tripTypeCombo->currentText();
        m_dtsConfig->setThermalTrip(zoneName,
            m_tripSelectCombo->currentText(),
            m_tripTempSpin->value(),
            m_tripHystSpin->value(),
            typeEn);
    }

    // 同步刷新当前策略频率显示
    if (false && m_mapInfoLabel && m_tripSelectCombo) {
        QPair<int,int> freqs = m_dtsConfig->getCoolingFrequenciesForTrip(zoneName, m_tripSelectCombo->currentText());
        if (freqs.first > 0 && freqs.second > 0) {
            m_mapInfoLabel->setText(QString("策略频率: cpu=%1MHz tpu=%2MHz")
                .arg(freqs.first / 1e6, 0, 'f', 0).arg(freqs.second / 1e6, 0, 'f', 0));
        } else {
            m_mapInfoLabel->setText("策略频率: -");
        }
    }
}

void PeripheralConfigDialog::onTripSelectionChanged()
{
    if (!m_dtsConfig || m_peripheralType != "thermal") return;
    QString zoneName = "soc_thermal_0";
    // 切换阈值项时，先把上一个阈值项的编辑值写回（使用上一次记录的 trip 名称）
    if (!m_lastTripName.isEmpty() && m_tripTempSpin && m_tripHystSpin && m_tripTypeCombo) {
        QString typeEn = m_tripTypeCombo->currentText();
        m_dtsConfig->setThermalTrip(zoneName, m_lastTripName, m_tripTempSpin->value(), m_tripHystSpin->value(), typeEn);
    }

    // 刷新所选 trip 的数值显示
    ThermalZoneInfo z = m_dtsConfig->getThermalZone(zoneName);
    QString curTrip = m_tripSelectCombo ? m_tripSelectCombo->currentText() : QString();
    if (!curTrip.isEmpty() && z.trips.contains(curTrip)) {
        const ThermalTripInfo &ti = z.trips[curTrip];
        if (m_tripTempSpin) m_tripTempSpin->setValue(ti.temperature);
        if (m_tripHystSpin) m_tripHystSpin->setValue(ti.hysteresis);
        if (m_tripTypeCombo) {
            int idx = m_tripTypeCombo->findText(ti.type);
            if (idx >= 0) m_tripTypeCombo->setCurrentIndex(idx);
        }
    }
    // 更新 last trip 标记
    m_lastTripName = curTrip;
    // 刷新频率显示
    if (m_mapInfoLabel && !curTrip.isEmpty()) {
        QPair<int,int> freqs = m_dtsConfig->getCoolingFrequenciesForTrip(zoneName, curTrip);
        if (freqs.first > 0 && freqs.second > 0) {
            m_mapInfoLabel->setText(QString("策略频率: cpu=%1MHz tpu=%2MHz")
                .arg(freqs.first / 1e6, 0, 'f', 0).arg(freqs.second / 1e6, 0, 'f', 0));
        } else {
            m_mapInfoLabel->setText("策略频率: -");
        }
    }
}

void PeripheralConfigDialog::onApplyClicked()
{
    savePeripheralConfig();

    // 只保存当前外设的配置，而不是整个文件
    bool ok = true;
    if (m_peripheralType == "thermal") {
        // 保存 thermal zone 到 DTS
        QString zoneName = "soc_thermal_0";
        ok = m_dtsConfig->saveThermalZone(zoneName);
    } else {
        ok = m_dtsConfig->savePeripheralConfig(m_currentPeripheral);
    }
    if (ok) {
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
