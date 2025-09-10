#include "clockconfig.h"
#include <QApplication>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDebug>
#include <QCheckBox>
#include <QPainter>
#include <QPaintEvent>
#include <QPolygon>
#include <QVector2D>
#include <QScrollBar>
#include <QTimer>
#include <QResizeEvent>
#include <QEvent>

// ClockConfigWidget类实现
const double ClockConfigWidget::OSC_FREQUENCY = 25.0;  // 25MHz
const double ClockConfigWidget::RTC_FREQUENCY = 0.032768;  // 32.768kHz

const QStringList ClockConfigWidget::PLL_NAMES = {
    "FPLL", "MIPIMPLL", "MPLL", "TPLL", "APPLL", "RVPLL"
};

const QStringList ClockConfigWidget::SUB_PLL_NAMES = {
    "APLL", "CAM0PLL", "CAM1PLL", "DISPPLL"
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
    , m_subPllWidget(nullptr)
    , m_subPllLayout(nullptr)
    , m_outputWidget(nullptr)
    , m_outputLayout(nullptr)
    , m_clockTreeWidget(nullptr)
    , m_clockTreeLayout(nullptr)
    , m_clockTree(nullptr)
    , m_clockSourceGroup(nullptr)
    , m_clockSourceLayout(nullptr)
    , m_pllGroup(nullptr)
    , m_pllGroupLayout(nullptr)
    , m_outputGroup(nullptr)
    , m_outputGroupLayout(nullptr)
    , m_buttonLayout(nullptr)
    , m_resetButton(nullptr)
    , m_applyButton(nullptr)
    , m_connectionOverlay(nullptr)
{
    setupUI();
    setupClockSources();
    setupPLLs();
    setupSubPLLs();  // 新增：设置子PLL区域
    setupOutputs();
    setupClockTree();
    connectSignals();
    updateFrequencies();
    
    // 创建连接线覆盖层
    m_connectionOverlay = new QWidget(this);
    m_connectionOverlay->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_connectionOverlay->setAttribute(Qt::WA_NoSystemBackground);
    m_connectionOverlay->setStyleSheet("background: transparent;");
    
    // 重写覆盖层的paintEvent
    m_connectionOverlay->installEventFilter(this);
    
    // 延迟重绘以确保布局完成
    QTimer::singleShot(100, this, [this]() {
        updateConnectionOverlay();
    });
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
    m_flowLayout->setSpacing(20); // 减少间距为连接线留出空间
    
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
    
    // 添加顶部间距，用于调节OSC和RTC组件的位置
    m_inputLayout->addSpacing(30);  // 向下移动30像素

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

    // OSC和RTC之间添加间距
    m_inputLayout->addSpacing(200);
    
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
        
        // 初始化PLL配置，根据不同PLL设置不同的默认倍频
        PLLConfig config;
        config.name = pllName;
        config.enabled = true;  // 默认启用
        config.inputFreq = OSC_FREQUENCY;
        config.divider = 1;  // 分频器固定为1
        config.source = "OSC";
        
        // 根据PLL名称设置不同的默认倍频值
        if (pllName == "FPLL") {
            config.multiplier = 40;
        } else if (pllName == "MIPIMPLL") {
            config.multiplier = 36;
        } else if (pllName == "MPLL") {
            config.multiplier = 48;
        } else if (pllName == "TPLL") {
            config.multiplier = 60;
        } else if (pllName == "APLL") {
            config.multiplier = 20;  // 保持原有默认值
        } else if (pllName == "CAM0PLL") {
            config.multiplier = 20;  // 保持原有默认值
        } else if (pllName == "CAM1PLL") {
            config.multiplier = 20;  // 保持原有默认值
        } else if (pllName == "DISPPLL") {
            config.multiplier = 20;  // 保持原有默认值
        } else if (pllName == "APPLL") {
            config.multiplier = 40;
        } else if (pllName == "RVPLL") {
            config.multiplier = 48;
        } else {
            config.multiplier = 20;  // 其他PLL使用默认值
        }
        
        config.outputFreq = OSC_FREQUENCY * config.multiplier;
        
        m_pllConfigs[pllName] = config;
    }
    
    m_pllLayout->addStretch();
    
    // 添加到流程布局
    m_flowLayout->addWidget(m_pllWidget);
}

void ClockConfigWidget::setupSubPLLs()
{
    // 创建子PLL区域
    m_subPllWidget = new QWidget();
    m_subPllWidget->setFixedWidth(180);
    m_subPllWidget->setStyleSheet(
        "QWidget { "
        "background-color: #e8f4f8; "
        "border: 2px solid #17a2b8; "
        "border-radius: 8px; "
        "}"
    );
    
    m_subPllLayout = new QVBoxLayout(m_subPllWidget);
    m_subPllLayout->setContentsMargins(10, 10, 10, 10);
    m_subPllLayout->setSpacing(8);
    
    // 添加标题
    QLabel* subPllTitle = new QLabel("子锁相环(PLLs)");
    subPllTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #0c5460; text-align: center;");
    subPllTitle->setAlignment(Qt::AlignCenter);
    m_subPllLayout->addWidget(subPllTitle);
    
    // 为每个子PLL创建配置界面
    for (const QString& pllName : SUB_PLL_NAMES) {
        createSubPLLWidget(pllName, m_subPllWidget);
        
        // 初始化子PLL配置
        PLLConfig config;
        config.name = pllName;
        config.enabled = true;  // 默认启用
        config.inputFreq = 900.0;  // 来自MIPIMPLL (25 * 36)
        config.divider = 1.0;  // 默认分频器（double类型）
        config.source = "MIPIMPLL";
        
        // 根据子PLL名称设置不同的默认倍频值
        if (pllName == "APLL") {
            config.multiplier = 4;  // 倍频为4
            config.divider = 7.32421875;  // 分频为7.32421875
        } else if (pllName == "CAM0PLL") {
            config.multiplier = 52;  // 倍频为52
            config.divider = 36.0;  // 分频为36
        } else if (pllName == "CAM1PLL") {
            config.multiplier = 64;  // 倍频为64
            config.divider = 36.0;  // 分频为36
        } else if (pllName == "DISPPLL") {
            config.multiplier = 12;  // 倍频为12
            config.divider = 9.09090909;  // 分频为9.09090909
        }
        
        config.outputFreq = config.inputFreq * config.multiplier / config.divider;
        
        m_pllConfigs[pllName] = config;
    }
    
    m_subPllLayout->addStretch();
    
    // 添加到流程布局
    m_flowLayout->addWidget(m_subPllWidget);
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
    
    // 根据PLL名称设置不同的默认倍频值
    int defaultMultiplier = 20;  // 默认值
    if (pllName == "FPLL") {
        defaultMultiplier = 40;
    } else if (pllName == "MIPIMPLL") {
        defaultMultiplier = 36;
    } else if (pllName == "MPLL") {
        defaultMultiplier = 48;
    } else if (pllName == "TPLL") {
        defaultMultiplier = 60;
    } else if (pllName == "APPLL") {
        defaultMultiplier = 40;
    } else if (pllName == "RVPLL") {
        defaultMultiplier = 48;
    }
    
    multBox->setValue(defaultMultiplier);
    multBox->setFixedWidth(60);
    multBox->setStyleSheet("font-size: 10px;");
    
    configLayout->addWidget(multLabel);
    configLayout->addWidget(multBox);
    
    // 频率显示
    QString freqText;
    if (pllName == "FPLL") {
        freqText = "1000.000 MHz";  // 25 * 40
    } else if (pllName == "MIPIMPLL") {
        freqText = "900.000 MHz";   // 25 * 36
    } else if (pllName == "MPLL") {
        freqText = "1200.000 MHz";  // 25 * 48
    } else if (pllName == "TPLL") {
        freqText = "1500.000 MHz";  // 25 * 60
    } else if (pllName == "APPLL") {
        freqText = "1000.000 MHz";  // 25 * 40
    } else if (pllName == "RVPLL") {
        freqText = "1200.000 MHz";  // 25 * 48
    } else {
        freqText = "500.000 MHz";   // 25 * 20 (其他PLL的默认值)
    }
    
    QLabel* freqLabel = new QLabel(freqText);
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

void ClockConfigWidget::createSubPLLWidget(const QString& pllName, QWidget* parent)
{
    QWidget* pllWidget = new QWidget();
    pllWidget->setStyleSheet(
        "QWidget { "
        "background-color: #ffffff; "
        "border: 1px solid #17a2b8; "
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
    nameLabel->setStyleSheet("font-weight: bold; color: #0c5460; font-size: 11px;");
    nameLabel->setAlignment(Qt::AlignCenter);
    
    // 分频器配置
    QHBoxLayout* divConfigLayout = new QHBoxLayout();
    divConfigLayout->setSpacing(5);
    
    QLabel* divLabel = new QLabel("分频：");
    divLabel->setStyleSheet("color: #0c5460; font-size: 10px;");
    
    QDoubleSpinBox* divBox = new QDoubleSpinBox();
    divBox->setRange(0.00000001, 256.0);  // 支持小数范围
    divBox->setDecimals(8);  // 保留8位小数
    
    // 根据PLL名称设置不同的默认分频值
    double defaultDivider = 1.0;
    if (pllName == "APLL") {
        defaultDivider = 7.32421875;
    } else if (pllName == "CAM0PLL") {
        defaultDivider = 36.0;
    } else if (pllName == "CAM1PLL") {
        defaultDivider = 36.0;
    } else if (pllName == "DISPPLL") {
        defaultDivider = 9.09090909;
    }
    
    divBox->setValue(defaultDivider);
    divBox->setFixedWidth(80);  // 增加宽度以显示更多数字
    divBox->setStyleSheet("font-size: 10px;");
    
    divConfigLayout->addWidget(divLabel);
    divConfigLayout->addWidget(divBox);
    
    // 倍频器配置
    QHBoxLayout* multConfigLayout = new QHBoxLayout();
    multConfigLayout->setSpacing(5);
    
    QLabel* multLabel = new QLabel("倍频：");
    multLabel->setStyleSheet("color: #0c5460; font-size: 10px;");
    
    QSpinBox* multBox = new QSpinBox();
    multBox->setRange(1, 100);
    
    // 根据PLL名称设置不同的默认倍频值
    int defaultMultiplier = 1;
    if (pllName == "APLL") {
        defaultMultiplier = 4;
    } else if (pllName == "CAM0PLL") {
        defaultMultiplier = 52;
    } else if (pllName == "CAM1PLL") {
        defaultMultiplier = 64;
    } else if (pllName == "DISPPLL") {
        defaultMultiplier = 12;
    }
    
    multBox->setValue(defaultMultiplier);
    multBox->setFixedWidth(60);
    multBox->setStyleSheet("font-size: 10px;");
    
    multConfigLayout->addWidget(multLabel);
    multConfigLayout->addWidget(multBox);
    
    // 频率显示 - 根据默认值计算初始频率
    double mipimpllFreq = 900.0;  // MIPIMPLL的频率 (25 * 36)
    double calculatedFreq = (mipimpllFreq * defaultMultiplier) / defaultDivider;
    QString freqText = QString("%1 MHz").arg(calculatedFreq, 0, 'f', 8);
    
    QLabel* freqLabel = new QLabel(freqText);
    freqLabel->setStyleSheet("color: #dc3545; font-family: monospace; font-weight: bold; font-size: 9px;");
    freqLabel->setAlignment(Qt::AlignCenter);
    
    layout->addWidget(nameLabel);
    layout->addLayout(divConfigLayout);
    layout->addLayout(multConfigLayout);
    layout->addWidget(freqLabel);
    
    // 存储控件引用
    m_subPllWidgets[pllName] = pllWidget;
    m_subPllDividerBoxes[pllName] = divBox;
    m_subPllMultiplierBoxes[pllName] = multBox;
    m_subPllFreqLabels[pllName] = freqLabel;
    
    m_subPllLayout->addWidget(pllWidget);
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
    
    // 连接子PLL控制信号
    for (const QString& pllName : SUB_PLL_NAMES) {
        QSpinBox* multBox = m_subPllMultiplierBoxes[pllName];
        QDoubleSpinBox* divBox = m_subPllDividerBoxes[pllName];  // 改为QDoubleSpinBox
        
        connect(multBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this, pllName](int value) {
            onSubPLLConfigChanged(pllName);
        });
        
        connect(divBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this, pllName](double value) {
            onSubPLLConfigChanged(pllName);
        });
    }
    
    // 连接按钮信号
    connect(m_resetButton, &QPushButton::clicked, this, &ClockConfigWidget::resetToDefaults);
    connect(m_applyButton, &QPushButton::clicked, this, [this]() {
        emit configChanged();
        QMessageBox::information(this, "配置", "时钟配置已应用！");
    });
    
    // 连接滚动条信号以重绘连接线
    connect(m_flowScrollArea->horizontalScrollBar(), &QScrollBar::valueChanged, 
            this, [this]() { updateConnectionOverlay(); });
    connect(m_flowScrollArea->verticalScrollBar(), &QScrollBar::valueChanged, 
            this, [this]() { updateConnectionOverlay(); });
}

void ClockConfigWidget::onPLLMultiplierChanged(const QString& pllName, int multiplier)
{
    m_pllConfigs[pllName].multiplier = multiplier;
    updatePLLFrequency(pllName);
    
    // 如果是MIPIMPLL，需要更新所有子PLL
    if (pllName == "MIPIMPLL") {
        updateAllSubPLLFrequencies();
    }
    
    emit pllConfigChanged(pllName);
    emit configChanged();
}

void ClockConfigWidget::onSubPLLConfigChanged(const QString& pllName)
{
    // 获取子PLL的分频和倍频值
    double divider = m_subPllDividerBoxes[pllName]->value();  // 改为double
    int multiplier = m_subPllMultiplierBoxes[pllName]->value();
    
    // 更新配置
    m_pllConfigs[pllName].divider = divider;
    m_pllConfigs[pllName].multiplier = multiplier;
    
    // 更新频率
    updateSubPLLFrequency(pllName);
    
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
}

void ClockConfigWidget::updateSubPLLFrequency(const QString& pllName)
{
    if (!m_pllConfigs.contains(pllName)) return;
    
    PLLConfig& config = m_pllConfigs[pllName];
    
    // 子PLL的输入频率来自MIPIMPLL
    if (m_pllConfigs.contains("MIPIMPLL")) {
        config.inputFreq = m_pllConfigs["MIPIMPLL"].outputFreq;
    }
    
    // 计算子PLL输出频率: (输入频率 * 倍频器) / 分频器
    config.outputFreq = (config.inputFreq * config.multiplier) / config.divider;
    
    // 更新显示
    if (m_subPllFreqLabels.contains(pllName)) {
        QString freqText = QString("%1 MHz").arg(config.outputFreq, 0, 'f', 8);  // 保留8位小数
        m_subPllFreqLabels[pllName]->setText(freqText);
    }
}

void ClockConfigWidget::updateAllSubPLLFrequencies()
{
    for (const QString& subPllName : SUB_PLL_NAMES) {
        updateSubPLLFrequency(subPllName);
    }
}

void ClockConfigWidget::updateOutputFrequency(const QString& outputName)
{
    if (!m_outputs.contains(outputName)) return;
    
    ClockOutput& output = m_outputs[outputName];
    
    // 由于移除了通道模块，直接使用25MHz作为输出频率
    output.frequency = OSC_FREQUENCY;
    
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
    
    // 更新所有子PLL频率
    for (const QString& subPllName : SUB_PLL_NAMES) {
        updateSubPLLFrequency(subPllName);
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
        // 根据PLL名称设置不同的默认倍频值
        int defaultMultiplier = 20;  // 默认值
        if (pllName == "FPLL") {
            defaultMultiplier = 40;
        } else if (pllName == "MIPIMPLL") {
            defaultMultiplier = 36;
        } else if (pllName == "MPLL") {
            defaultMultiplier = 48;
        } else if (pllName == "TPLL") {
            defaultMultiplier = 60;
        } else if (pllName == "APPLL") {
            defaultMultiplier = 40;
        } else if (pllName == "RVPLL") {
            defaultMultiplier = 48;
        }
        
        if (m_pllMultiplierBoxes.contains(pllName)) {
            m_pllMultiplierBoxes[pllName]->setValue(defaultMultiplier);
        }
        
        m_pllConfigs[pllName].enabled = true;  // 始终启用
        m_pllConfigs[pllName].multiplier = defaultMultiplier;
        m_pllConfigs[pllName].divider = 1.0;      // 分频器固定为1.0（double类型）
    }
    
    // 重置子PLL配置
    for (const QString& pllName : SUB_PLL_NAMES) {
        // 根据PLL名称设置不同的默认值
        int defaultMultiplier = 1;
        double defaultDivider = 1.0;
        
        if (pllName == "APLL") {
            defaultMultiplier = 4;
            defaultDivider = 7.32421875;
        } else if (pllName == "CAM0PLL") {
            defaultMultiplier = 52;
            defaultDivider = 36.0;
        } else if (pllName == "CAM1PLL") {
            defaultMultiplier = 64;
            defaultDivider = 36.0;
        } else if (pllName == "DISPPLL") {
            defaultMultiplier = 12;
            defaultDivider = 9.09090909;
        }
        
        if (m_subPllMultiplierBoxes.contains(pllName)) {
            m_subPllMultiplierBoxes[pllName]->setValue(defaultMultiplier);
        }
        if (m_subPllDividerBoxes.contains(pllName)) {
            m_subPllDividerBoxes[pllName]->setValue(defaultDivider);
        }
        
        m_pllConfigs[pllName].enabled = true;
        m_pllConfigs[pllName].multiplier = defaultMultiplier;
        m_pllConfigs[pllName].divider = defaultDivider;
    }
    
    // 重置输出配置
    for (const QString& outputName : OUTPUT_NAMES) {
        m_outputs[outputName].source = "OSC";
        m_outputs[outputName].divider = 1;
        m_outputs[outputName].enabled = false;
    }
    
    updateFrequencies();
    emit configChanged();
    updateConnectionOverlay();  // 重绘连接线
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

void ClockConfigWidget::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
    // 连接线现在由覆盖层绘制，这里不再需要绘制
}

void ClockConfigWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateConnectionOverlay();
}

bool ClockConfigWidget::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == m_connectionOverlay && event->type() == QEvent::Paint) {
        QPaintEvent* paintEvent = static_cast<QPaintEvent*>(event);
        
        QPainter painter(m_connectionOverlay);
        painter.setRenderHint(QPainter::Antialiasing, true);
        
        // 绘制连接线
        drawConnectionLines(painter);
        
        return true;  // 事件已处理
    }
    
    return QWidget::eventFilter(obj, event);
}

void ClockConfigWidget::drawConnectionLines(QPainter& painter)
{
    // 确保flow widget已经布局完成
    if (!m_flowWidget || !m_inputWidget || !m_pllWidget) {
        return;
    }
    
    // 获取OSC连接点
    QPoint oscPoint = getOSCConnectionPoint();
    
    // 需要连接的PLL列表（根据需求：FPLL、MIPIMPLL、MPLL、TPLL、APPLL和RVPLL）
    QStringList targetPLLs = {"FPLL", "MIPIMPLL", "MPLL", "TPLL", "APPLL", "RVPLL"};
    
    // 为每个目标PLL绘制从OSC的连接线
    for (const QString& pllName : targetPLLs) {
        QPoint pllPoint = getPLLConnectionPoint(pllName);
        if (!pllPoint.isNull() && !oscPoint.isNull()) {
            // 使用不同颜色来区分不同的连接线
            QColor lineColor = Qt::blue;
            if (pllName == "FPLL") lineColor = QColor(220, 53, 69);      // 红色
            else if (pllName == "MIPIMPLL") lineColor = QColor(255, 193, 7);  // 黄色
            else if (pllName == "MPLL") lineColor = QColor(40, 167, 69);      // 绿色
            else if (pllName == "TPLL") lineColor = QColor(23, 162, 184);     // 青色
            else if (pllName == "APPLL") lineColor = QColor(102, 16, 242);    // 紫色
            else if (pllName == "RVPLL") lineColor = QColor(255, 102, 0);     // 橙色
            
            drawArrowLine(painter, oscPoint, pllPoint, lineColor);
        }
    }
    
    // 绘制MIPIMPLL到子PLL的连接线
    if (m_subPllWidget) {
        QPoint mipimpllPoint = getMIPIMPLLConnectionPoint();
        
        for (const QString& subPllName : SUB_PLL_NAMES) {
            QPoint subPllPoint = getSubPLLConnectionPoint(subPllName);
            if (!subPllPoint.isNull() && !mipimpllPoint.isNull()) {
                // 使用橙色来表示MIPIMPLL到子PLL的连接
                QColor lineColor = QColor(255, 165, 0);  // 橙色
                drawArrowLine(painter, mipimpllPoint, subPllPoint, lineColor);
            }
        }
    }
}

void ClockConfigWidget::drawArrowLine(QPainter& painter, const QPoint& start, const QPoint& end, const QColor& color)
{
    painter.setPen(QPen(color, 2));
    painter.setBrush(QBrush(color));
    
    // 绘制连接线
    painter.drawLine(start, end);
    
    // 计算箭头
    QVector2D line(end - start);
    line = line.normalized();
    
    // 箭头大小
    const int arrowSize = 8;
    
    // 计算箭头的两个点
    QVector2D arrowP1 = QVector2D(end) - line * arrowSize;
    QVector2D perpendicular(-line.y(), line.x());
    
    QPoint arrow1 = (arrowP1 + perpendicular * (arrowSize / 2)).toPoint();
    QPoint arrow2 = (arrowP1 - perpendicular * (arrowSize / 2)).toPoint();
    
    // 绘制箭头
    QPolygon arrowHead;
    arrowHead << end << arrow1 << arrow2;
    painter.drawPolygon(arrowHead);
}

QPoint ClockConfigWidget::getOSCConnectionPoint() const
{
    if (!m_inputWidget || !m_flowWidget) {
        return QPoint();
    }
    
    // 获取OSC widget在flow widget中的相对位置
    QPoint inputPos = m_inputWidget->pos();
    QRect inputRect = m_inputWidget->rect();
    
    // OSC连接点位于输入源widget的右侧中央
    QPoint oscPoint = QPoint(
        inputPos.x() + inputRect.width(),
        inputPos.y() + inputRect.height() / 3 + 30  // 考虑添加的间距，调整OSC位置
    );
    
    // 转换为相对于ClockConfigWidget的坐标
    QPoint flowPos = m_flowWidget->pos();
    QPoint scrollOffset = QPoint(
        m_flowScrollArea->horizontalScrollBar()->value(),
        m_flowScrollArea->verticalScrollBar()->value()
    );
    
    return QPoint(
        flowPos.x() + oscPoint.x() - scrollOffset.x(),
        flowPos.y() + oscPoint.y() - scrollOffset.y() + 60  // 加上标题高度
    );
}

QPoint ClockConfigWidget::getPLLConnectionPoint(const QString& pllName) const
{
    if (!m_pllWidget || !m_flowWidget || !m_pllWidgets.contains(pllName)) {
        return QPoint();
    }
    
    QWidget* pllWidget = m_pllWidgets[pllName];
    if (!pllWidget) {
        return QPoint();
    }
    
    // 获取PLL widget在其父widget中的位置
    QPoint pllPos = pllWidget->pos();
    QRect pllRect = pllWidget->rect();
    
    // 获取PLL区域在flow widget中的位置
    QPoint pllAreaPos = m_pllWidget->pos();
    
    // PLL连接点位于PLL widget的左侧中央
    QPoint pllPoint = QPoint(
        pllAreaPos.x() + pllPos.x(),
        pllAreaPos.y() + pllPos.y() + pllRect.height() / 2
    );
    
    // 转换为相对于ClockConfigWidget的坐标
    QPoint flowPos = m_flowWidget->pos();
    QPoint scrollOffset = QPoint(
        m_flowScrollArea->horizontalScrollBar()->value(),
        m_flowScrollArea->verticalScrollBar()->value()
    );
    
    return QPoint(
        flowPos.x() + pllPoint.x() - scrollOffset.x(),
        flowPos.y() + pllPoint.y() - scrollOffset.y() + 60  // 加上标题高度
    );
}

QPoint ClockConfigWidget::getMIPIMPLLConnectionPoint() const
{
    // 获取MIPIMPLL的连接点，作为输出点位于右侧
    if (!m_pllWidget || !m_flowWidget || !m_pllWidgets.contains("MIPIMPLL")) {
        return QPoint();
    }
    
    QWidget* mipimpllWidget = m_pllWidgets["MIPIMPLL"];
    if (!mipimpllWidget) {
        return QPoint();
    }
    
    // 获取MIPIMPLL widget在其父widget中的位置
    QPoint pllPos = mipimpllWidget->pos();
    QRect pllRect = mipimpllWidget->rect();
    
    // 获取PLL区域在flow widget中的位置
    QPoint pllAreaPos = m_pllWidget->pos();
    
    // MIPIMPLL输出连接点位于widget的右侧中央
    QPoint mipimpllPoint = QPoint(
        pllAreaPos.x() + pllPos.x() + pllRect.width(),
        pllAreaPos.y() + pllPos.y() + pllRect.height() / 2
    );
    
    // 转换为相对于ClockConfigWidget的坐标
    QPoint flowPos = m_flowWidget->pos();
    QPoint scrollOffset = QPoint(
        m_flowScrollArea->horizontalScrollBar()->value(),
        m_flowScrollArea->verticalScrollBar()->value()
    );
    
    return QPoint(
        flowPos.x() + mipimpllPoint.x() - scrollOffset.x(),
        flowPos.y() + mipimpllPoint.y() - scrollOffset.y() + 60  // 加上标题高度
    );
}

QPoint ClockConfigWidget::getSubPLLConnectionPoint(const QString& pllName) const
{
    if (!m_subPllWidget || !m_flowWidget || !m_subPllWidgets.contains(pllName)) {
        return QPoint();
    }
    
    QWidget* subPllWidget = m_subPllWidgets[pllName];
    if (!subPllWidget) {
        return QPoint();
    }
    
    // 获取子PLL widget在其父widget中的位置
    QPoint pllPos = subPllWidget->pos();
    QRect pllRect = subPllWidget->rect();
    
    // 获取子PLL区域在flow widget中的位置
    QPoint subPllAreaPos = m_subPllWidget->pos();
    
    // 子PLL连接点位于widget的左侧中央
    QPoint subPllPoint = QPoint(
        subPllAreaPos.x() + pllPos.x(),
        subPllAreaPos.y() + pllPos.y() + pllRect.height() / 2
    );
    
    // 转换为相对于ClockConfigWidget的坐标
    QPoint flowPos = m_flowWidget->pos();
    QPoint scrollOffset = QPoint(
        m_flowScrollArea->horizontalScrollBar()->value(),
        m_flowScrollArea->verticalScrollBar()->value()
    );
    
    return QPoint(
        flowPos.x() + subPllPoint.x() - scrollOffset.x(),
        flowPos.y() + subPllPoint.y() - scrollOffset.y() + 60  // 加上标题高度
    );
}

void ClockConfigWidget::updateConnectionOverlay()
{
    if (!m_connectionOverlay) {
        return;
    }
    
    // 设置覆盖层的大小和位置覆盖整个widget
    m_connectionOverlay->setGeometry(rect());
    m_connectionOverlay->raise();  // 确保覆盖层在最上层
    m_connectionOverlay->update(); // 重绘覆盖层
}
