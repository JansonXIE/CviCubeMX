#include "clockconfig.h"
#include <QApplication>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDebug>
#include <QCheckBox>

// 定义常量
const double ClockConfigWidget::OSC_FREQUENCY = 25.0;  // 25MHz
const double ClockConfigWidget::RTC_FREQUENCY = 0.032768;  // 32.768kHz

const QStringList ClockConfigWidget::PLL_NAMES = {
    "FPLL", "MIPIMPLL", "MPLL", "TPLL", "APLL", "CAM0PLL", "CAM1PLL", "DISPPLL", "APPLL", "RVPLL"
};

const QStringList ClockConfigWidget::CHANNEL_NAMES = {
    "通道1", "通道2", "通道3", "通道4"
};

const QStringList ClockConfigWidget::OUTPUT_NAMES = {
    "CPU_IP", "AXI_Subsys", "AHB_Subsys", "APB_Subsys", 
    "DDR_IP", "USB_IP", "ETH_IP", "CAM_IP"
};

ClockConfigWidget::ClockConfigWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_flowScrollArea(nullptr)
    , m_flowWidget(nullptr)
    , m_flowLayout(nullptr)
    , m_inputWidget(nullptr)
    , m_inputLayout(nullptr)
    , m_pllWidget(nullptr)
    , m_pllLayout(nullptr)
    , m_channelWidget(nullptr)
    , m_channelLayout(nullptr)
    , m_outputWidget(nullptr)
    , m_outputLayout(nullptr)
    , m_clockTreeWidget(nullptr)
    , m_clockTreeLayout(nullptr)
    , m_clockTree(nullptr)
    , m_clockSourceGroup(nullptr)
    , m_clockSourceLayout(nullptr)
    , m_pllGroup(nullptr)
    , m_pllGroupLayout(nullptr)
    , m_channelGroup(nullptr)
    , m_channelGroupLayout(nullptr)
    , m_outputGroup(nullptr)
    , m_outputGroupLayout(nullptr)
    , m_buttonLayout(nullptr)
    , m_resetButton(nullptr)
    , m_applyButton(nullptr)
{
    setupUI();
    setupClockSources();
    setupPLLs();
    setupClockChannels();
    setupOutputs();
    setupClockTree();
    connectSignals();
    updateFrequencies();
}

ClockConfigWidget::~ClockConfigWidget()
{
}

void ClockConfigWidget::setupUI()
{
    // 创建主布局（垂直）
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    m_mainLayout->setSpacing(10);
    
    // 添加标题
    QLabel* titleLabel = new QLabel("时钟配置 - 时钟流程图");
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50; padding: 10px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    m_mainLayout->addWidget(titleLabel);
    
    // 创建时钟流程显示区域（水平滚动）
    m_flowScrollArea = new QScrollArea();
    m_flowScrollArea->setWidgetResizable(true);
    m_flowScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_flowScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_flowScrollArea->setMinimumHeight(600);
    
    m_flowWidget = new QWidget();
    m_flowLayout = new QHBoxLayout(m_flowWidget);
    m_flowLayout->setContentsMargins(20, 20, 20, 20);
    m_flowLayout->setSpacing(30);
    
    // 设置流程区域样式
    m_flowWidget->setStyleSheet(
        "QWidget { "
        "background-color: #f8f9fa; "
        "border: 2px solid #dee2e6; "
        "border-radius: 8px; "
        "}"
    );
    
    m_flowScrollArea->setWidget(m_flowWidget);
    m_mainLayout->addWidget(m_flowScrollArea);
    
    // 创建控制按钮区域
    m_buttonLayout = new QHBoxLayout();
    
    m_resetButton = new QPushButton("重置为默认");
    m_resetButton->setStyleSheet(
        "QPushButton { "
        "background-color: #6c757d; "
        "color: white; "
        "border: none; "
        "padding: 10px 20px; "
        "border-radius: 6px; "
        "font-weight: bold; "
        "font-size: 14px; "
        "} "
        "QPushButton:hover { "
        "background-color: #545b62; "
        "} "
        "QPushButton:pressed { "
        "background-color: #494f54; "
        "}"
    );
    
    m_applyButton = new QPushButton("应用配置");
    m_applyButton->setStyleSheet(
        "QPushButton { "
        "background-color: #007bff; "
        "color: white; "
        "border: none; "
        "padding: 10px 20px; "
        "border-radius: 6px; "
        "font-weight: bold; "
        "font-size: 14px; "
        "} "
        "QPushButton:hover { "
        "background-color: #0056b3; "
        "} "
        "QPushButton:pressed { "
        "background-color: #004085; "
        "}"
    );
    
    m_buttonLayout->addWidget(m_resetButton);
    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(m_applyButton);
    
    m_mainLayout->addLayout(m_buttonLayout);
}

void ClockConfigWidget::setupClockSources()
{
    // 创建输入源区域
    m_inputWidget = new QWidget();
    m_inputWidget->setFixedWidth(150);
    m_inputWidget->setStyleSheet(
        "QWidget { "
        "background-color: #e8f5e8; "
        "border: 2px solid #28a745; "
        "border-radius: 8px; "
        "}"
    );
    
    m_inputLayout = new QVBoxLayout(m_inputWidget);
    m_inputLayout->setContentsMargins(10, 10, 10, 10);
    m_inputLayout->setSpacing(10);
    
    // 添加标题
    QLabel* inputTitle = new QLabel("输入源");
    inputTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #155724; text-align: center;");
    inputTitle->setAlignment(Qt::AlignCenter);
    m_inputLayout->addWidget(inputTitle);
    
    // OSC 25MHz
    QWidget* oscWidget = new QWidget();
    oscWidget->setStyleSheet(
        "QWidget { "
        "background-color: #ffffff; "
        "border: 1px solid #28a745; "
        "border-radius: 6px; "
        "padding: 8px; "
        "}"
    );
    QVBoxLayout* oscLayout = new QVBoxLayout(oscWidget);
    oscLayout->setContentsMargins(5, 5, 5, 5);
    
    QLabel* oscLabel = new QLabel("25MHz");
    oscLabel->setStyleSheet("font-weight: bold; color: #28a745; font-size: 12px;");
    oscLabel->setAlignment(Qt::AlignCenter);
    
    QLabel* oscSubLabel = new QLabel("OSC");
    oscSubLabel->setStyleSheet("color: #28a745; font-size: 10px;");
    oscSubLabel->setAlignment(Qt::AlignCenter);
    
    oscLayout->addWidget(oscLabel);
    oscLayout->addWidget(oscSubLabel);
    m_inputLayout->addWidget(oscWidget);
    
    // RTC 32768Hz
    QWidget* rtcWidget = new QWidget();
    rtcWidget->setStyleSheet(
        "QWidget { "
        "background-color: #ffffff; "
        "border: 1px solid #17a2b8; "
        "border-radius: 6px; "
        "padding: 8px; "
        "}"
    );
    QVBoxLayout* rtcLayout = new QVBoxLayout(rtcWidget);
    rtcLayout->setContentsMargins(5, 5, 5, 5);
    
    QLabel* rtcLabel = new QLabel("32768Hz");
    rtcLabel->setStyleSheet("font-weight: bold; color: #17a2b8; font-size: 12px;");
    rtcLabel->setAlignment(Qt::AlignCenter);
    
    QLabel* rtcSubLabel = new QLabel("RTC");
    rtcSubLabel->setStyleSheet("color: #17a2b8; font-size: 10px;");
    rtcSubLabel->setAlignment(Qt::AlignCenter);
    
    rtcLayout->addWidget(rtcLabel);
    rtcLayout->addWidget(rtcSubLabel);
    m_inputLayout->addWidget(rtcWidget);
    
    m_inputLayout->addStretch();
    
    // 添加到流程布局
    m_flowLayout->addWidget(m_inputWidget);
}

void ClockConfigWidget::setupPLLs()
{
    // 创建PLL区域
    m_pllWidget = new QWidget();
    m_pllWidget->setFixedWidth(200);
    m_pllWidget->setStyleSheet(
        "QWidget { "
        "background-color: #fff3cd; "
        "border: 2px solid #ffc107; "
        "border-radius: 8px; "
        "}"
    );
    
    m_pllLayout = new QVBoxLayout(m_pllWidget);
    m_pllLayout->setContentsMargins(10, 10, 10, 10);
    m_pllLayout->setSpacing(8);
    
    // 添加标题
    QLabel* pllTitle = new QLabel("锁相环(PLLs)");
    pllTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #856404; text-align: center;");
    pllTitle->setAlignment(Qt::AlignCenter);
    m_pllLayout->addWidget(pllTitle);
    
    // 为每个PLL创建配置界面
    for (const QString& pllName : PLL_NAMES) {
        createPLLWidget(pllName, m_pllWidget);
        
        // 初始化PLL配置
        PLLConfig config;
        config.name = pllName;
        config.enabled = true;  // 默认启用
        config.inputFreq = OSC_FREQUENCY;
        config.outputFreq = OSC_FREQUENCY * 20;  // 默认20倍频
        config.divider = 1;  // 分频器固定为1
        config.multiplier = 20;  // 默认20倍频
        config.source = "OSC";
        
        m_pllConfigs[pllName] = config;
    }
    
    m_pllLayout->addStretch();
    
    // 添加到流程布局
    m_flowLayout->addWidget(m_pllWidget);
}

void ClockConfigWidget::setupClockChannels()
{
    // 创建时钟通道区域
    m_channelWidget = new QWidget();
    m_channelWidget->setFixedWidth(300);
    m_channelWidget->setStyleSheet(
        "QWidget { "
        "background-color: #d1ecf1; "
        "border: 2px solid #17a2b8; "
        "border-radius: 8px; "
        "}"
    );
    
    m_channelLayout = new QVBoxLayout(m_channelWidget);
    m_channelLayout->setContentsMargins(10, 10, 10, 10);
    m_channelLayout->setSpacing(10);
    
    // 添加标题
    QLabel* channelTitle = new QLabel("时钟分频与选择");
    channelTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #0c5460; text-align: center;");
    channelTitle->setAlignment(Qt::AlignCenter);
    m_channelLayout->addWidget(channelTitle);
    
    // 为每个通道创建配置界面
    for (const QString& channelName : CHANNEL_NAMES) {
        createClockChannelWidget(channelName, m_channelWidget);
        
        // 初始化通道配置
        ClockChannel channel;
        channel.name = channelName;
        channel.pllSource1 = "FPLL";
        channel.pllSource2 = "MPLL";
        channel.divider1 = 1;
        channel.divider2 = 1;
        channel.muxSelection = 0;
        channel.frequency = OSC_FREQUENCY;
        channel.enabled = false;
        
        m_clockChannels[channelName] = channel;
    }
    
    m_channelLayout->addStretch();
    
    // 添加到流程布局
    m_flowLayout->addWidget(m_channelWidget);
}

void ClockConfigWidget::setupOutputs()
{
    // 创建输出区域
    m_outputWidget = new QWidget();
    m_outputWidget->setFixedWidth(150);
    m_outputWidget->setStyleSheet(
        "QWidget { "
        "background-color: #f8d7da; "
        "border: 2px solid #dc3545; "
        "border-radius: 8px; "
        "}"
    );
    
    m_outputLayout = new QVBoxLayout(m_outputWidget);
    m_outputLayout->setContentsMargins(10, 10, 10, 10);
    m_outputLayout->setSpacing(8);
    
    // 添加标题
    QLabel* outputTitle = new QLabel("输出");
    outputTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #721c24; text-align: center;");
    outputTitle->setAlignment(Qt::AlignCenter);
    m_outputLayout->addWidget(outputTitle);
    
    // 为每个输出创建显示界面
    for (const QString& outputName : OUTPUT_NAMES) {
        createOutputWidget(outputName, m_outputWidget);
        
        // 初始化输出配置
        ClockOutput output;
        output.name = outputName;
        output.source = "通道1";
        output.frequency = OSC_FREQUENCY;
        output.divider = 1;
        output.enabled = false;
        
        m_outputs[outputName] = output;
    }
    
    m_outputLayout->addStretch();
    
    // 添加到流程布局
    m_flowLayout->addWidget(m_outputWidget);
}

void ClockConfigWidget::createPLLWidget(const QString& pllName, QWidget* parent)
{
    QWidget* pllWidget = new QWidget();
    pllWidget->setStyleSheet(
        "QWidget { "
        "background-color: #ffffff; "
        "border: 1px solid #ffc107; "
        "border-radius: 4px; "
        "padding: 5px; "
        "margin: 2px; "
        "}"
    );
    
    QVBoxLayout* layout = new QVBoxLayout(pllWidget);
    layout->setSpacing(3);
    layout->setContentsMargins(5, 5, 5, 5);
    
    // PLL名称标签
    QLabel* nameLabel = new QLabel(pllName);
    nameLabel->setStyleSheet("font-weight: bold; color: #856404; font-size: 11px;");
    nameLabel->setAlignment(Qt::AlignCenter);
    
    // 倍频器配置（居中布局）
    QHBoxLayout* configLayout = new QHBoxLayout();
    configLayout->setSpacing(5);
    
    QLabel* multLabel = new QLabel("倍频：");
    multLabel->setStyleSheet("color: #856404; font-size: 10px;");
    
    QSpinBox* multBox = new QSpinBox();
    multBox->setRange(1, 100);
    multBox->setValue(20);  // 默认值改为20倍频
    multBox->setFixedWidth(60);
    multBox->setStyleSheet("font-size: 10px;");
    
    configLayout->addWidget(multLabel);
    configLayout->addWidget(multBox);
    
    // 频率显示
    QLabel* freqLabel = new QLabel("500.000 MHz");
    freqLabel->setStyleSheet("color: #dc3545; font-family: monospace; font-weight: bold; font-size: 9px;");
    freqLabel->setAlignment(Qt::AlignCenter);
    
    layout->addWidget(nameLabel);
    layout->addLayout(configLayout);
    layout->addWidget(freqLabel);
    
    // 存储控件引用
    m_pllWidgets[pllName] = pllWidget;
    m_pllMultiplierBoxes[pllName] = multBox;
    m_pllFreqLabels[pllName] = freqLabel;
    
    m_pllLayout->addWidget(pllWidget);
}

void ClockConfigWidget::createClockChannelWidget(const QString& channelName, QWidget* parent)
{
    QWidget* channelWidget = new QWidget();
    channelWidget->setStyleSheet(
        "QWidget { "
        "background-color: #ffffff; "
        "border: 1px solid #17a2b8; "
        "border-radius: 4px; "
        "padding: 8px; "
        "margin: 3px; "
        "}"
    );
    
    QVBoxLayout* layout = new QVBoxLayout(channelWidget);
    layout->setSpacing(5);
    layout->setContentsMargins(5, 5, 5, 5);
    
    // 通道名称标签
    QLabel* nameLabel = new QLabel(channelName);
    nameLabel->setStyleSheet("font-weight: bold; color: #0c5460; font-size: 12px;");
    nameLabel->setAlignment(Qt::AlignCenter);
    
    // 第一路: PLL源 + 分频器
    QHBoxLayout* path1Layout = new QHBoxLayout();
    QComboBox* source1Box = new QComboBox();
    for (const QString& pll : PLL_NAMES) {
        source1Box->addItem(pll);
    }
    source1Box->setFixedWidth(80);
    source1Box->setStyleSheet("font-size: 9px;");
    
    QSpinBox* div1Box = new QSpinBox();
    div1Box->setRange(1, 256);
    div1Box->setValue(1);
    div1Box->setPrefix("÷");
    div1Box->setFixedWidth(50);
    div1Box->setStyleSheet("font-size: 9px;");
    
    path1Layout->addWidget(source1Box);
    path1Layout->addWidget(div1Box);
    
    // 第二路: PLL源 + 分频器
    QHBoxLayout* path2Layout = new QHBoxLayout();
    QComboBox* source2Box = new QComboBox();
    for (const QString& pll : PLL_NAMES) {
        source2Box->addItem(pll);
    }
    source2Box->setCurrentIndex(2); // 默认选择MPLL
    source2Box->setFixedWidth(80);
    source2Box->setStyleSheet("font-size: 9px;");
    
    QSpinBox* div2Box = new QSpinBox();
    div2Box->setRange(1, 256);
    div2Box->setValue(1);
    div2Box->setPrefix("÷");
    div2Box->setFixedWidth(50);
    div2Box->setStyleSheet("font-size: 9px;");
    
    path2Layout->addWidget(source2Box);
    path2Layout->addWidget(div2Box);
    
    // MUX选择器
    QComboBox* muxBox = new QComboBox();
    muxBox->addItem("路径1");
    muxBox->addItem("路径2");
    muxBox->setStyleSheet("font-size: 10px;");
    
    // 频率显示
    QLabel* freqLabel = new QLabel("25.000 MHz");
    freqLabel->setStyleSheet("color: #28a745; font-family: monospace; font-weight: bold; font-size: 9px;");
    freqLabel->setAlignment(Qt::AlignCenter);
    
    layout->addWidget(nameLabel);
    layout->addLayout(path1Layout);
    layout->addLayout(path2Layout);
    layout->addWidget(muxBox);
    layout->addWidget(freqLabel);
    
    // 存储控件引用
    m_channelWidgets[channelName] = channelWidget;
    m_channelSource1Boxes[channelName] = source1Box;
    m_channelSource2Boxes[channelName] = source2Box;
    m_channelDiv1Boxes[channelName] = div1Box;
    m_channelDiv2Boxes[channelName] = div2Box;
    m_channelMuxBoxes[channelName] = muxBox;
    m_channelFreqLabels[channelName] = freqLabel;
    
    m_channelLayout->addWidget(channelWidget);
}

void ClockConfigWidget::createOutputWidget(const QString& outputName, QWidget* parent)
{
    QWidget* outputWidget = new QWidget();
    outputWidget->setStyleSheet(
        "QWidget { "
        "background-color: #ffffff; "
        "border: 1px solid #dc3545; "
        "border-radius: 4px; "
        "padding: 5px; "
        "margin: 2px; "
        "}"
    );
    
    QVBoxLayout* layout = new QVBoxLayout(outputWidget);
    layout->setSpacing(3);
    layout->setContentsMargins(5, 5, 5, 5);
    
    // 输出名称标签
    QLabel* nameLabel = new QLabel(outputName);
    nameLabel->setStyleSheet("font-weight: bold; color: #721c24; font-size: 10px;");
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setWordWrap(true);
    
    // 频率显示
    QLabel* freqLabel = new QLabel("25.000 MHz");
    freqLabel->setStyleSheet("color: #dc3545; font-family: monospace; font-weight: bold; font-size: 8px;");
    freqLabel->setAlignment(Qt::AlignCenter);
    
    layout->addWidget(nameLabel);
    layout->addWidget(freqLabel);
    
    // 存储控件引用
    m_outputWidgets[outputName] = outputWidget;
    m_outputFreqLabels[outputName] = freqLabel;
    
    m_outputLayout->addWidget(outputWidget);
}

void ClockConfigWidget::setupClockTree()
{
    // 由于现在使用流程图布局，时钟树功能可以简化或移除
    // 这里保留一个简单的实现作为备用
}

void ClockConfigWidget::connectSignals()
{
    // 连接PLL控制信号
    for (const QString& pllName : PLL_NAMES) {
        QSpinBox* multBox = m_pllMultiplierBoxes[pllName];
        
        connect(multBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this, pllName](int value) {
            onPLLMultiplierChanged(pllName, value);
        });
    }
    
    // 连接时钟通道控制信号
    for (const QString& channelName : CHANNEL_NAMES) {
        QComboBox* source1Box = m_channelSource1Boxes[channelName];
        QComboBox* source2Box = m_channelSource2Boxes[channelName];
        QSpinBox* div1Box = m_channelDiv1Boxes[channelName];
        QSpinBox* div2Box = m_channelDiv2Boxes[channelName];
        QComboBox* muxBox = m_channelMuxBoxes[channelName];
        
        connect(source1Box, QOverload<const QString&>::of(&QComboBox::currentTextChanged), 
                this, [this, channelName](const QString& source) {
            onChannelConfigChanged(channelName);
        });
        
        connect(source2Box, QOverload<const QString&>::of(&QComboBox::currentTextChanged), 
                this, [this, channelName](const QString& source) {
            onChannelConfigChanged(channelName);
        });
        
        connect(div1Box, QOverload<int>::of(&QSpinBox::valueChanged), this, [this, channelName](int value) {
            onChannelConfigChanged(channelName);
        });
        
        connect(div2Box, QOverload<int>::of(&QSpinBox::valueChanged), this, [this, channelName](int value) {
            onChannelConfigChanged(channelName);
        });
        
        connect(muxBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, channelName](int index) {
            onChannelMuxChanged(channelName, index);
        });
    }
    
    // 连接按钮信号
    connect(m_resetButton, &QPushButton::clicked, this, &ClockConfigWidget::resetToDefaults);
    connect(m_applyButton, &QPushButton::clicked, this, [this]() {
        emit configChanged();
        QMessageBox::information(this, "配置", "时钟配置已应用！");
    });
}

void ClockConfigWidget::onPLLMultiplierChanged(const QString& pllName, int multiplier)
{
    m_pllConfigs[pllName].multiplier = multiplier;
    updatePLLFrequency(pllName);
    emit pllConfigChanged(pllName);
    emit configChanged();
}

void ClockConfigWidget::onClockDividerChanged(const QString& outputName, int divider)
{
    if (m_outputs.contains(outputName)) {
        m_outputs[outputName].divider = divider;
        updateOutputFrequency(outputName);
        emit configChanged();
    }
}

void ClockConfigWidget::onChannelConfigChanged(const QString& channelName)
{
    updateChannelFrequency(channelName);
    
    // 更新所有输出频率，因为它们可能依赖这个通道
    for (const QString& outputName : OUTPUT_NAMES) {
        updateOutputFrequency(outputName);
    }
    
    emit configChanged();
}

void ClockConfigWidget::onChannelMuxChanged(const QString& channelName, int selection)
{
    if (m_clockChannels.contains(channelName)) {
        m_clockChannels[channelName].muxSelection = selection;
        updateChannelFrequency(channelName);
        
        // 更新所有输出频率
        for (const QString& outputName : OUTPUT_NAMES) {
            updateOutputFrequency(outputName);
        }
        
        emit configChanged();
    }
}

void ClockConfigWidget::onClockTreeItemClicked(QTreeWidgetItem* item, int column)
{
    Q_UNUSED(column)
    
    if (!item) return;
    
    QString itemData = item->data(0, Qt::UserRole).toString();
    if (itemData.startsWith("PLL:")) {
        QString pllName = itemData.mid(4);
        // 滚动到对应的PLL配置
        if (m_pllWidgets.contains(pllName)) {
            // 由于现在使用流程布局，这里可以简化或删除滚动逻辑
        }
    } else if (itemData.startsWith("CHANNEL:")) {
        QString channelName = itemData.mid(8);
        // 滚动到对应的时钟通道配置
        if (m_channelWidgets.contains(channelName)) {
            // 由于现在使用流程布局，这里可以简化或删除滚动逻辑
        }
    }
}

void ClockConfigWidget::updatePLLFrequency(const QString& pllName)
{
    if (!m_pllConfigs.contains(pllName)) return;
    
    PLLConfig& config = m_pllConfigs[pllName];
    
    // 计算PLL输出频率: 输入频率 * 倍频器 (分频器固定为1)
    config.outputFreq = config.inputFreq * config.multiplier;
    
    // 更新显示
    if (m_pllFreqLabels.contains(pllName)) {
        QString freqText = QString("%1 MHz").arg(config.outputFreq, 0, 'f', 1);
        m_pllFreqLabels[pllName]->setText(freqText);
        
        // 颜色始终为启用状态
        m_pllFreqLabels[pllName]->setStyleSheet(
            "color: #dc3545; font-family: monospace; font-weight: bold; font-size: 9px;"
        );
    }
    
    // 更新依赖此PLL的通道频率
    for (const QString& channelName : CHANNEL_NAMES) {
        updateChannelFrequency(channelName);
    }
}

void ClockConfigWidget::updateChannelFrequency(const QString& channelName)
{
    if (!m_clockChannels.contains(channelName)) return;
    
    ClockChannel& channel = m_clockChannels[channelName];
    
    // 根据MUX选择计算输出频率
    QString selectedPLL;
    int selectedDivider;
    
    if (channel.muxSelection == 0) {
        // 选择路径1
        selectedPLL = m_channelSource1Boxes[channelName]->currentText();
        selectedDivider = m_channelDiv1Boxes[channelName]->value();
    } else {
        // 选择路径2
        selectedPLL = m_channelSource2Boxes[channelName]->currentText();
        selectedDivider = m_channelDiv2Boxes[channelName]->value();
    }
    
    // 获取PLL输出频率
    double pllFreq = 0.0;
    if (m_pllConfigs.contains(selectedPLL)) {
        pllFreq = m_pllConfigs[selectedPLL].outputFreq;
    }
    
    // 计算通道输出频率
    channel.frequency = pllFreq / selectedDivider;
    
    // 更新显示
    if (m_channelFreqLabels.contains(channelName)) {
        QString freqText = QString("%1 MHz").arg(channel.frequency, 0, 'f', 1);
        m_channelFreqLabels[channelName]->setText(freqText);
    }
}

void ClockConfigWidget::updateOutputFrequency(const QString& outputName)
{
    if (!m_outputs.contains(outputName)) return;
    
    ClockOutput& output = m_outputs[outputName];
    
    // 简单地假设每个输出对应一个通道
    // 实际实现中可能需要更复杂的映射关系
    int outputIndex = OUTPUT_NAMES.indexOf(outputName);
    int channelIndex = outputIndex % CHANNEL_NAMES.size();
    QString channelName = CHANNEL_NAMES[channelIndex];
    
    if (m_clockChannels.contains(channelName)) {
        output.frequency = m_clockChannels[channelName].frequency;
    }
    
    // 更新显示
    if (m_outputFreqLabels.contains(outputName)) {
        QString freqText = QString("%1 MHz").arg(output.frequency, 0, 'f', 1);
        m_outputFreqLabels[outputName]->setText(freqText);
    }
}

void ClockConfigWidget::updateFrequencies()
{
    // 更新所有PLL频率
    for (const QString& pllName : PLL_NAMES) {
        updatePLLFrequency(pllName);
    }
    
    // 更新所有通道频率
    for (const QString& channelName : CHANNEL_NAMES) {
        updateChannelFrequency(channelName);
    }
    
    // 更新所有输出频率
    for (const QString& outputName : OUTPUT_NAMES) {
        updateOutputFrequency(outputName);
    }
}

void ClockConfigWidget::resetToDefaults()
{
    // 重置PLL配置
    for (const QString& pllName : PLL_NAMES) {
        if (m_pllMultiplierBoxes.contains(pllName)) {
            m_pllMultiplierBoxes[pllName]->setValue(20);  // 默认20倍频
        }
        
        m_pllConfigs[pllName].enabled = true;  // 始终启用
        m_pllConfigs[pllName].multiplier = 20;  // 默认20倍频
        m_pllConfigs[pllName].divider = 1;      // 分频器固定为1
    }
    
    // 重置时钟通道配置
    for (const QString& channelName : CHANNEL_NAMES) {
        if (m_channelSource1Boxes.contains(channelName)) {
            m_channelSource1Boxes[channelName]->setCurrentIndex(0); // FPLL
        }
        if (m_channelSource2Boxes.contains(channelName)) {
            m_channelSource2Boxes[channelName]->setCurrentIndex(2); // MPLL
        }
        if (m_channelDiv1Boxes.contains(channelName)) {
            m_channelDiv1Boxes[channelName]->setValue(1);
        }
        if (m_channelDiv2Boxes.contains(channelName)) {
            m_channelDiv2Boxes[channelName]->setValue(1);
        }
        if (m_channelMuxBoxes.contains(channelName)) {
            m_channelMuxBoxes[channelName]->setCurrentIndex(0); // 路径1
        }
        
        m_clockChannels[channelName].pllSource1 = "FPLL";
        m_clockChannels[channelName].pllSource2 = "MPLL";
        m_clockChannels[channelName].divider1 = 1;
        m_clockChannels[channelName].divider2 = 1;
        m_clockChannels[channelName].muxSelection = 0;
        m_clockChannels[channelName].enabled = false;
    }
    
    // 重置输出配置
    for (const QString& outputName : OUTPUT_NAMES) {
        m_outputs[outputName].source = "通道1";
        m_outputs[outputName].divider = 1;
        m_outputs[outputName].enabled = false;
    }
    
    updateFrequencies();
    emit configChanged();
}

PLLConfig ClockConfigWidget::getPLLConfig(const QString& pllName) const
{
    return m_pllConfigs.value(pllName, PLLConfig());
}

void ClockConfigWidget::setPLLConfig(const QString& pllName, const PLLConfig& config)
{
    m_pllConfigs[pllName] = config;
    
    // 更新UI
    if (m_pllMultiplierBoxes.contains(pllName)) {
        m_pllMultiplierBoxes[pllName]->setValue(config.multiplier);
    }
    
    updatePLLFrequency(pllName);
}

ClockChannel ClockConfigWidget::getClockChannel(const QString& channelName) const
{
    return m_clockChannels.value(channelName, ClockChannel());
}

void ClockConfigWidget::setClockChannel(const QString& channelName, const ClockChannel& channel)
{
    m_clockChannels[channelName] = channel;
    
    // 更新UI
    if (m_channelSource1Boxes.contains(channelName)) {
        int index = m_channelSource1Boxes[channelName]->findText(channel.pllSource1);
        if (index >= 0) {
            m_channelSource1Boxes[channelName]->setCurrentIndex(index);
        }
    }
    if (m_channelSource2Boxes.contains(channelName)) {
        int index = m_channelSource2Boxes[channelName]->findText(channel.pllSource2);
        if (index >= 0) {
            m_channelSource2Boxes[channelName]->setCurrentIndex(index);
        }
    }
    if (m_channelDiv1Boxes.contains(channelName)) {
        m_channelDiv1Boxes[channelName]->setValue(channel.divider1);
    }
    if (m_channelDiv2Boxes.contains(channelName)) {
        m_channelDiv2Boxes[channelName]->setValue(channel.divider2);
    }
    if (m_channelMuxBoxes.contains(channelName)) {
        m_channelMuxBoxes[channelName]->setCurrentIndex(channel.muxSelection);
    }
    
    updateChannelFrequency(channelName);
}

bool ClockConfigWidget::saveConfig(const QString& filePath)
{
    // TODO: 实现配置保存功能
    Q_UNUSED(filePath)
    return true;
}

bool ClockConfigWidget::loadConfig(const QString& filePath)
{
    // TODO: 实现配置加载功能
    Q_UNUSED(filePath)
    return true;
}
