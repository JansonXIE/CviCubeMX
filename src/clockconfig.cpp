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
#include <QDialog>
#include <QFormLayout>
#include <QDialogButtonBox>

// ClockConfigWidget类实现
const double ClockConfigWidget::OSC_FREQUENCY = 25.0;  // 25MHz
const double ClockConfigWidget::RTC_FREQUENCY = 0.032768;  // 32.768kHz

const QStringList ClockConfigWidget::PLL_NAMES = {
    "FPLL", "clk_mipimpll", "MPLL", "TPLL", "APPLL", "clk_rvpll"
};

const QStringList ClockConfigWidget::SUB_PLL_NAMES = {
    "clk_a0pll", "clk_cam0pll", "clk_cam1pll", "clk_disppll", "clk_mipimpll_d3", "clk_cyc_dsi_syn"
};

const QStringList ClockConfigWidget::OUTPUT_NAMES = {
    "clk_saradc", "clk_tempsen", "clk_apb_spi3", "clk_apb_spi2", "clk_apb_spi1", "clk_apb_spi0",
    "clk_ahb_sf1", "clk_ahb_sf", "clk_axi4_eth1", "clk_axi4_eth0", "clk_axi4_sd1", "clk_axi4_sd0",
    "clk_axi4_emmc", "clk_ahb_rom", "clk_apb_audsrc", "clk_jpeg", "clk_ve", "clk_apb_jpeg",
    "clk_apb_ve", "clk_apb_vcsys", "clk_2de_vip", "clk_vo_mac_vip", "clk_csi2_rx_vip",
    "clk_csi1_rx_vip", "clk_csi0_rx_vip", "clk_dsi_mac_vip", "clk_lvds1_vip", "clk_lvds0_vip",
    "clk_pad_vi2_clk_vip", "clk_pad_vi1_clk_vip", "clk_pad_vi0_clk1_vip", "clk_pad_vi0_clk0_vip",
    "clk_ldc_vip", "clk_vpss3_vip", "clk_vpss2_vip", "clk_vpss1_vip", "clk_vpss0_vip",
    "clk_dbgsys", "clk_efuse_clk", "clk_keyscan_xclk", "clk_wgn_xclk", "clk_wdt_pclk",
    "clk_usb20_coreclkin", "clk_usb20_suspend", "clk_sc", "clk_dbg", "clk_1M"
};

const QStringList ClockConfigWidget::CLK_1M_SUB_NODES = {
    "clk_gpio_dbclk", "clk_emmc_100K", "clk_100k_sd1", "clk_100k_sd0"
};

const QStringList ClockConfigWidget::CLK_CAM1PLL_SUB_NODES = {
    "clk_emmc_card", "clk_sd1", "clk_sd0", "clk_vip_sys_2", "clk_raw_axi", "clk_vc_src1"
};

const QStringList ClockConfigWidget::CLK_RAW_AXI_SUB_NODES = {
    "clk_oenc", "clk_raw_vip", "clk_isp_top_vip", "clk_csi_be_vip", "clk_csi_mac2_vip", "clk_csi_mac1_vip", "clk_csi_mac0_vip"
};

const QStringList ClockConfigWidget::CLK_CAM0PLL_SUB_NODES = {
    "clk_cam0_vip", "clk_vc_src0", "clk_tpu_gdma"
};

const QStringList ClockConfigWidget::CLK_DISPPLL_SUB_NODES = {
    "clk_cam2_vip", "clk_cam1_vip", "clk_sys_disp"
};

const QStringList ClockConfigWidget::CLK_SYS_DISP_SUB_NODES = {
    "clk_disp_vip"
};

const QStringList ClockConfigWidget::CLK_A0PLL_SUB_NODES = {
    "clk_aud3", "clk_aud2", "clk_aud1", "clk_aud0", "clk_audsrc"
};

const QStringList ClockConfigWidget::CLK_RVPLL_SUB_NODES = {
    "clk_rv1"
};

ClockConfigWidget::ClockConfigWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_flowScrollArea(nullptr)
    , m_flowWidget(nullptr)
    // , m_flowLayout(nullptr)  // 不再使用
    , m_inputWidget(nullptr)
    , m_inputLayout(nullptr)
    , m_pllWidget(nullptr)
    , m_pllLayout(nullptr)
    , m_subPllWidget(nullptr)
    , m_subPllLayout(nullptr)
    , m_outputWidget(nullptr)
    , m_outputLayout(nullptr)
    , m_clk1MSubNodeWidget(nullptr)
    , m_clk1MSubNodeLayout(nullptr)
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
    , m_positionConfigButton(nullptr)
    , m_connectionOverlay(nullptr)
{
    setupUI();
    setupClockSources();
    setupPLLs();
    setupSubPLLs();  // 新增：设置子PLL区域
    setupOutputs();
    setupClkCam1PLLSubNodes();  // 新增：设置clk_cam1pll子节点区域
    setupClkRawAxiSubNodes();  // 新增：设置clk_raw_axi子节点区域
    setupClkCam0PLLSubNodes();  // 新增：设置clk_cam0pll子节点区域
    setupClkDispPLLSubNodes();  // 新增：设置clk_disppll子节点区域
    setupClkSysDispSubNodes();  // 新增：设置clk_sys_disp子节点区域
    setupClkA0PLLSubNodes();  // 新增：设置clk_a0pll子节点区域
    setupClkRVPLLSubNodes();  // 新增：设置clk_rvpll子节点区域
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
    
    // 初始化模块位置
    initializeModulePositions();
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
    m_flowScrollArea->setFocusPolicy(Qt::WheelFocus); // 启用滚轮焦点
    
    m_flowWidget = new QWidget();
    // 设置足够大的尺寸以包含所有模块，支持滚动
    m_flowWidget->setMinimumSize(1200, 4200); // 宽度1200，高度4200以容纳OSC输出的长列表
    m_flowWidget->setFocusPolicy(Qt::WheelFocus); // 启用滚轮焦点
    // 移除布局管理器，使用绝对定位
    // m_flowLayout = new QHBoxLayout(m_flowWidget);
    // m_flowLayout->setContentsMargins(20, 20, 20, 20);
    // m_flowLayout->setSpacing(20); // 减少间距为连接线留出空间
    
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
    
    m_positionConfigButton = new QPushButton("位置配置");
    m_positionConfigButton->setStyleSheet(
        "QPushButton { "
        "background-color: #28a745; "
        "color: white; "
        "border: none; "
        "padding: 10px 20px; "
        "border-radius: 6px; "
        "font-weight: bold; "
        "font-size: 14px; "
        "} "
        "QPushButton:hover { "
        "background-color: #218838; "
        "} "
        "QPushButton:pressed { "
        "background-color: #1e7e34; "
        "}"
    );
    
    m_buttonLayout->addWidget(m_resetButton);
    m_buttonLayout->addWidget(m_positionConfigButton);
    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(m_applyButton);
    
    m_mainLayout->addLayout(m_buttonLayout);
}

void ClockConfigWidget::setupClockSources()
{
    // 创建输入源区域
    m_inputWidget = new QWidget();
    m_inputWidget->setFixedSize(150, 400); // 设置固定尺寸而不是仅仅宽度
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
    
    // 设置父widget为m_flowWidget并使用绝对定位
    m_inputWidget->setParent(m_flowWidget);
    // 默认位置将在initializeModulePositions中设置
}

void ClockConfigWidget::setupPLLs()
{
    // 创建PLL区域
    m_pllWidget = new QWidget();
    m_pllWidget->setFixedSize(200, 700); // 设置固定尺寸
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
        } else if (pllName == "clk_mipimpll") {
            config.multiplier = 36;
        } else if (pllName == "MPLL") {
            config.multiplier = 48;
        } else if (pllName == "TPLL") {
            config.multiplier = 60;
        } else if (pllName == "APPLL") {
            config.multiplier = 40;
        } else if (pllName == "clk_rvpll") {
            config.multiplier = 64;
        } else {
            config.multiplier = 20;  // 其他PLL使用默认值
        }
        
        config.outputFreq = OSC_FREQUENCY * config.multiplier;
        
        m_pllConfigs[pllName] = config;
    }
    
    m_pllLayout->addStretch();
    
    // 设置父widget为m_flowWidget并使用绝对定位
    m_pllWidget->setParent(m_flowWidget);
    // 默认位置将在initializeModulePositions中设置
}

void ClockConfigWidget::setupSubPLLs()
{
    // 创建子PLL区域
    m_subPllWidget = new QWidget();
    m_subPllWidget->setFixedSize(180, 780); // 设置固定尺寸
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
        config.source = "clk_mipimpll";
        
        // 根据子PLL名称设置不同的默认倍频值
        if (pllName == "clk_a0pll") {
            config.multiplier = 4;  // 倍频为4
            config.divider = 7.32421875;  // 分频为7.32421875
        } else if (pllName == "clk_cam0pll") {
            config.multiplier = 52;  // 倍频为52
            config.divider = 36.0;  // 分频为36
        } else if (pllName == "clk_cam1pll") {
            config.multiplier = 64;  // 倍频为64
            config.divider = 36.0;  // 分频为36
        } else if (pllName == "clk_disppll") {
            config.multiplier = 12;  // 倍频为12
            config.divider = 9.09090909;  // 分频为9.09090909
        } else if (pllName == "clk_mipimpll_d3") {
            config.multiplier = 1;  // 倍频为1
            config.divider = 3;  // 分频为3
        } else if (pllName == "clk_cyc_dsi_syn") {
            config.multiplier = 1;  // 倍频为1
            config.divider = 1;  // 分频为1
        }
        
        config.outputFreq = config.inputFreq * config.multiplier / config.divider;
        
        m_pllConfigs[pllName] = config;
    }
    
    m_subPllLayout->addStretch();
    
    // 设置父widget为m_flowWidget并使用绝对定位
    m_subPllWidget->setParent(m_flowWidget);
    // 默认位置将在initializeModulePositions中设置
}

void ClockConfigWidget::setupOutputs()
{
    // 创建OSC输出区域
    m_outputWidget = new QWidget();
    m_outputWidget->setFixedSize(200, 4000); // 设置固定尺寸以容纳更多节点
    m_outputWidget->setStyleSheet(
        "QWidget { "
        "background-color: #f8d7da; "
        "border: 2px solid #dc3545; "
        "border-radius: 8px; "
        "}"
    );
    
    m_outputLayout = new QVBoxLayout(m_outputWidget);
    m_outputLayout->setContentsMargins(10, 10, 10, 10);
    m_outputLayout->setSpacing(5);  // 减少间距以容纳更多节点
    
    // 添加标题
    QLabel* outputTitle = new QLabel("OSC输出");
    outputTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #721c24; text-align: center;");
    outputTitle->setAlignment(Qt::AlignCenter);
    m_outputLayout->addWidget(outputTitle);
    
    // 为每个OSC输出创建显示界面
    for (const QString& outputName : OUTPUT_NAMES) {
        createOutputWidget(outputName, m_outputWidget);
        
        // 初始化OSC输出配置
        ClockOutput output;
        output.name = outputName;
        output.source = "OSC";
        output.frequency = OSC_FREQUENCY;
        output.enabled = true;  // OSC输出默认启用
        
        // 根据输出名称设置不同的默认分频值
        if (outputName == "clk_usb20_suspend") {
            output.divider = 125;
        } else if (outputName == "clk_1M") {
            output.divider = 250;  // 25MHz / 250 = 0.1MHz = 100kHz，接近1MHz
        } else {
            output.divider = 1;  // 其他节点默认分频为1
        }
        
        m_outputs[outputName] = output;
    }
    
    m_outputLayout->addStretch();
    
    // 设置父widget为m_flowWidget并使用绝对定位
    m_outputWidget->setParent(m_flowWidget);
    // 默认位置将在initializeModulePositions中设置
    
    // 创建clk_1M子节点区域
    setupClk1MSubNodes();
}

void ClockConfigWidget::setupClk1MSubNodes()
{
    // 创建clk_1M子节点区域
    m_clk1MSubNodeWidget = new QWidget();
    m_clk1MSubNodeWidget->setFixedSize(150, 350); // 设置固定尺寸
    m_clk1MSubNodeWidget->setStyleSheet(
        "QWidget { "
        "background-color: #e8f0ff; "
        "border: 2px solid #007bff; "
        "border-radius: 8px; "
        "}"
    );
    
    m_clk1MSubNodeLayout = new QVBoxLayout(m_clk1MSubNodeWidget);
    m_clk1MSubNodeLayout->setContentsMargins(10, 10, 10, 10);
    m_clk1MSubNodeLayout->setSpacing(5);
    
    // 添加标题
    QLabel* subNodeTitle = new QLabel("clk_1M子节点");
    subNodeTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #004085; text-align: center;");
    subNodeTitle->setAlignment(Qt::AlignCenter);
    m_clk1MSubNodeLayout->addWidget(subNodeTitle);
    
    // 为每个clk_1M子节点创建显示界面
    for (const QString& nodeName : CLK_1M_SUB_NODES) {
        createClk1MSubNodeWidget(nodeName, m_clk1MSubNodeWidget);
        
        // 初始化clk_1M子节点配置
        ClockOutput subNode;
        subNode.name = nodeName;
        subNode.source = "clk_1M";
        subNode.divider = 1;  // 默认分频为1
        subNode.enabled = true;
        
        // 计算频率：clk_1M的频率 / 分频器
        double clk1MFreq = OSC_FREQUENCY / 250;  // clk_1M = 25MHz / 250 = 0.1MHz
        subNode.frequency = clk1MFreq / subNode.divider;
        
        m_clk1MSubNodes[nodeName] = subNode;
    }
    
    m_clk1MSubNodeLayout->addStretch();
    
    // 设置父widget为m_flowWidget并使用绝对定位
    m_clk1MSubNodeWidget->setParent(m_flowWidget);
    // 默认位置将在initializeModulePositions中设置
}

void ClockConfigWidget::setupClkCam1PLLSubNodes()
{
    // 创建clk_cam1pll子节点区域
    m_clkCam1PLLSubNodeWidget = new QWidget();
    m_clkCam1PLLSubNodeWidget->setFixedSize(150, 450); // 设置固定尺寸，比clk_1M稍高些
    m_clkCam1PLLSubNodeWidget->setStyleSheet(
        "QWidget { "
        "background-color: #fff3cd; "
        "border: 2px solid #ffc107; "
        "border-radius: 8px; "
        "}"
    );
    
    m_clkCam1PLLSubNodeLayout = new QVBoxLayout(m_clkCam1PLLSubNodeWidget);
    m_clkCam1PLLSubNodeLayout->setContentsMargins(10, 10, 10, 10);
    m_clkCam1PLLSubNodeLayout->setSpacing(5);
    
    // 添加标题
    QLabel* subNodeTitle = new QLabel("clk_cam1pll子节点");
    subNodeTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #856404; text-align: center;");
    subNodeTitle->setAlignment(Qt::AlignCenter);
    m_clkCam1PLLSubNodeLayout->addWidget(subNodeTitle);
    
    // 为每个clk_cam1pll子节点创建显示界面
    for (const QString& nodeName : CLK_CAM1PLL_SUB_NODES) {
        createClkCam1PLLSubNodeWidget(nodeName, m_clkCam1PLLSubNodeWidget);
        
        // 初始化clk_cam1pll子节点配置
        ClockOutput subNode;
        subNode.name = nodeName;
        subNode.source = "clk_cam1pll";
        subNode.divider = 4;  // 默认分频为4
        subNode.enabled = true;
        
        // 计算频率：clk_cam1pll的频率 / 分频器
        // 假设clk_cam1pll的默认频率为800MHz (这个值可以根据实际情况调整)
        double clkCam1PLLFreq = 800.0; // MHz
        subNode.frequency = clkCam1PLLFreq / subNode.divider;
        
        m_clkCam1PLLSubNodes[nodeName] = subNode;
    }
    
    m_clkCam1PLLSubNodeLayout->addStretch();
    
    // 设置父widget为m_flowWidget并使用绝对定位
    m_clkCam1PLLSubNodeWidget->setParent(m_flowWidget);
    // 默认位置将在initializeModulePositions中设置
}

void ClockConfigWidget::setupClkRawAxiSubNodes()
{
    // 创建clk_raw_axi子节点区域
    m_clkRawAxiSubNodeWidget = new QWidget();
    m_clkRawAxiSubNodeWidget->setFixedSize(180, 550); // 设置固定尺寸，比较高以容纳7个子节点
    m_clkRawAxiSubNodeWidget->setStyleSheet(
        "QWidget { "
        "background-color: #f0e68c; "
        "border: 2px solid #daa520; "
        "border-radius: 8px; "
        "}"
    );
    
    m_clkRawAxiSubNodeLayout = new QVBoxLayout(m_clkRawAxiSubNodeWidget);
    m_clkRawAxiSubNodeLayout->setContentsMargins(10, 10, 10, 10);
    m_clkRawAxiSubNodeLayout->setSpacing(5);
    
    // 添加标题
    QLabel* subNodeTitle = new QLabel("clk_raw_axi子节点");
    subNodeTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #8b7355; text-align: center;");
    subNodeTitle->setAlignment(Qt::AlignCenter);
    m_clkRawAxiSubNodeLayout->addWidget(subNodeTitle);
    
    // 为每个clk_raw_axi子节点创建显示界面
    for (const QString& nodeName : CLK_RAW_AXI_SUB_NODES) {
        createClkRawAxiSubNodeWidget(nodeName, m_clkRawAxiSubNodeWidget);
        
        // 初始化clk_raw_axi子节点配置
        ClockOutput subNode;
        subNode.name = nodeName;
        subNode.source = "clk_raw_axi";
        subNode.divider = 1;  // 默认分频为1
        subNode.enabled = true;
        
        // 计算频率：clk_raw_axi的频率 / 分频器
        // 假设clk_raw_axi的默认频率为200MHz (这个值可以根据实际情况调整)
        double clkRawAxiFreq = 200.0; // MHz
        subNode.frequency = clkRawAxiFreq / subNode.divider;
        
        m_clkRawAxiSubNodes[nodeName] = subNode;
    }
    
    m_clkRawAxiSubNodeLayout->addStretch();
    
    // 设置父widget为m_flowWidget并使用绝对定位
    m_clkRawAxiSubNodeWidget->setParent(m_flowWidget);
    // 默认位置将在initializeModulePositions中设置
}

void ClockConfigWidget::setupClkCam0PLLSubNodes()
{
    // 创建clk_cam0pll子节点区域
    m_clkCam0PLLSubNodeWidget = new QWidget();
    m_clkCam0PLLSubNodeWidget->setFixedSize(150, 250); // 设置固定尺寸，3个子节点
    m_clkCam0PLLSubNodeWidget->setStyleSheet(
        "QWidget { "
        "background-color: #e6f3ff; "
        "border: 2px solid #4da6ff; "
        "border-radius: 8px; "
        "}"
    );
    
    m_clkCam0PLLSubNodeLayout = new QVBoxLayout(m_clkCam0PLLSubNodeWidget);
    m_clkCam0PLLSubNodeLayout->setContentsMargins(10, 10, 10, 10);
    m_clkCam0PLLSubNodeLayout->setSpacing(5);
    
    // 添加标题
    QLabel* subNodeTitle = new QLabel("clk_cam0pll子节点");
    subNodeTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #0066cc; text-align: center;");
    subNodeTitle->setAlignment(Qt::AlignCenter);
    m_clkCam0PLLSubNodeLayout->addWidget(subNodeTitle);
    
    // 为每个clk_cam0pll子节点创建显示界面
    for (const QString& nodeName : CLK_CAM0PLL_SUB_NODES) {
        createClkCam0PLLSubNodeWidget(nodeName, m_clkCam0PLLSubNodeWidget);
        
        // 初始化clk_cam0pll子节点配置
        ClockOutput subNode;
        subNode.name = nodeName;
        subNode.source = "clk_cam0pll";
        subNode.enabled = true;
        
        // 根据子节点设置不同的默认分频值
        if (nodeName == "clk_cam0_vip") {
            subNode.divider = 50;  // clk_cam0_vip默认分频系数是50
        } else {
            subNode.divider = 2;   // clk_vc_src0和clk_tpu_gdma默认分频系数是2
        }
        
        // 计算频率：clk_cam0pll的频率 / 分频器
        // 假设clk_cam0pll的默认频率为1000MHz (这个值可以根据实际情况调整)
        double clkCam0PLLFreq = 1000.0; // MHz
        subNode.frequency = clkCam0PLLFreq / subNode.divider;
        
        m_clkCam0PLLSubNodes[nodeName] = subNode;
    }
    
    m_clkCam0PLLSubNodeLayout->addStretch();
    
    // 设置父widget为m_flowWidget并使用绝对定位
    m_clkCam0PLLSubNodeWidget->setParent(m_flowWidget);
    // 默认位置将在initializeModulePositions中设置
}

void ClockConfigWidget::setupClkDispPLLSubNodes()
{
    // 创建clk_disppll子节点区域
    m_clkDispPLLSubNodeWidget = new QWidget();
    m_clkDispPLLSubNodeWidget->setFixedSize(150, 250); // 设置固定尺寸，3个子节点
    m_clkDispPLLSubNodeWidget->setStyleSheet(
        "QWidget { "
        "background-color: #ffe6f2; "
        "border: 2px solid #ff66b3; "
        "border-radius: 8px; "
        "}"
    );
    
    m_clkDispPLLSubNodeLayout = new QVBoxLayout(m_clkDispPLLSubNodeWidget);
    m_clkDispPLLSubNodeLayout->setContentsMargins(10, 10, 10, 10);
    m_clkDispPLLSubNodeLayout->setSpacing(5);
    
    // 添加标题
    QLabel* subNodeTitle = new QLabel("clk_disppll子节点");
    subNodeTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #cc0066; text-align: center;");
    subNodeTitle->setAlignment(Qt::AlignCenter);
    m_clkDispPLLSubNodeLayout->addWidget(subNodeTitle);
    
    // 为每个clk_disppll子节点创建显示界面
    for (const QString& nodeName : CLK_DISPPLL_SUB_NODES) {
        createClkDispPLLSubNodeWidget(nodeName, m_clkDispPLLSubNodeWidget);
        
        // 初始化clk_disppll子节点配置
        ClockOutput subNode;
        subNode.name = nodeName;
        subNode.source = "clk_disppll";
        subNode.enabled = true;
        
        // 根据子节点设置不同的默认分频值
        if (nodeName == "clk_cam2_vip") {
            subNode.divider = 32;  // clk_cam2_vip默认分频系数是32
        } else if (nodeName == "clk_cam1_vip") {
            subNode.divider = 44;  // clk_cam1_vip默认分频系数是44
        } else if (nodeName == "clk_sys_disp") {
            subNode.divider = 8;   // clk_sys_disp默认分频系数是8
        }
        
        // 计算频率：clk_disppll的频率 / 分频器
        // 假设clk_disppll的默认频率为1200MHz (这个值可以根据实际情况调整)
        double clkDispPLLFreq = 1200.0; // MHz
        subNode.frequency = clkDispPLLFreq / subNode.divider;
        
        m_clkDispPLLSubNodes[nodeName] = subNode;
    }
    
    m_clkDispPLLSubNodeLayout->addStretch();
    
    // 设置父widget为m_flowWidget并使用绝对定位
    m_clkDispPLLSubNodeWidget->setParent(m_flowWidget);
    // 默认位置将在initializeModulePositions中设置
}

void ClockConfigWidget::setupClkSysDispSubNodes()
{
    // 创建clk_sys_disp子节点区域
    m_clkSysDispSubNodeWidget = new QWidget();
    m_clkSysDispSubNodeWidget->setFixedSize(150, 120); // 设置固定尺寸，1个子节点
    m_clkSysDispSubNodeWidget->setStyleSheet(
        "QWidget { "
        "background-color: #e6f3ff; "
        "border: 2px solid #3399ff; "
        "border-radius: 8px; "
        "}"
    );
    
    m_clkSysDispSubNodeLayout = new QVBoxLayout(m_clkSysDispSubNodeWidget);
    m_clkSysDispSubNodeLayout->setContentsMargins(10, 10, 10, 10);
    m_clkSysDispSubNodeLayout->setSpacing(5);
    
    // 添加标题
    QLabel* subNodeTitle = new QLabel("clk_sys_disp子节点");
    subNodeTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #0066cc; text-align: center;");
    subNodeTitle->setAlignment(Qt::AlignCenter);
    m_clkSysDispSubNodeLayout->addWidget(subNodeTitle);
    
    // 为每个clk_sys_disp子节点创建显示界面
    for (const QString& nodeName : CLK_SYS_DISP_SUB_NODES) {
        createClkSysDispSubNodeWidget(nodeName, m_clkSysDispSubNodeWidget);
        
        // 初始化clk_sys_disp子节点配置
        ClockOutput subNode;
        subNode.name = nodeName;
        subNode.source = "clk_sys_disp";
        subNode.enabled = true;
        subNode.divider = 1;  // clk_disp_vip默认分频系数是1
        
        // 计算频率：clk_sys_disp的频率 / 分频器
        // clk_sys_disp的频率来自clk_disppll/8，假设clk_disppll为1200MHz，那么clk_sys_disp为150MHz
        double clkSysDispFreq = 150.0; // MHz (1200/8)
        subNode.frequency = clkSysDispFreq / subNode.divider;
        
        m_clkSysDispSubNodes[nodeName] = subNode;
    }
    
    m_clkSysDispSubNodeLayout->addStretch();
    
    // 设置父widget为m_flowWidget并使用绝对定位
    m_clkSysDispSubNodeWidget->setParent(m_flowWidget);
    // 默认位置将在initializeModulePositions中设置
}

void ClockConfigWidget::setupClkA0PLLSubNodes()
{
    // 创建clk_a0pll子节点区域
    m_clkA0PLLSubNodeWidget = new QWidget();
    m_clkA0PLLSubNodeWidget->setFixedSize(150, 450); // 设置固定尺寸，5个子节点
    m_clkA0PLLSubNodeWidget->setStyleSheet(
        "QWidget { "
        "background-color: #f0f8ff; "
        "border: 2px solid #4169e1; "
        "border-radius: 8px; "
        "}"
    );
    
    m_clkA0PLLSubNodeLayout = new QVBoxLayout(m_clkA0PLLSubNodeWidget);
    m_clkA0PLLSubNodeLayout->setContentsMargins(10, 10, 10, 10);
    m_clkA0PLLSubNodeLayout->setSpacing(5);
    
    // 添加标题
    QLabel* subNodeTitle = new QLabel("clk_a0pll子节点");
    subNodeTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #191970; text-align: center;");
    subNodeTitle->setAlignment(Qt::AlignCenter);
    m_clkA0PLLSubNodeLayout->addWidget(subNodeTitle);
    
    // 为每个clk_a0pll子节点创建显示界面
    for (const QString& nodeName : CLK_A0PLL_SUB_NODES) {
        createClkA0PLLSubNodeWidget(nodeName, m_clkA0PLLSubNodeWidget);
        
        // 初始化clk_a0pll子节点配置
        ClockOutput subNode;
        subNode.name = nodeName;
        subNode.source = "clk_a0pll";
        subNode.enabled = true;
        subNode.divider = 17;  // 默认分频系数是17
        
        // 计算频率：clk_a0pll的频率 / 分频器
        // 从clk_a0pll的配置中获取正确的频率
        double clkA0PLLFreq = 500.0; // 默认500MHz
        if (m_pllConfigs.contains("clk_a0pll")) {
            clkA0PLLFreq = m_pllConfigs["clk_a0pll"].outputFreq;
        }
        subNode.frequency = clkA0PLLFreq / subNode.divider;
        
        m_clkA0PLLSubNodes[nodeName] = subNode;
    }
    
    m_clkA0PLLSubNodeLayout->addStretch();
    
    // 设置父widget为m_flowWidget并使用绝对定位
    m_clkA0PLLSubNodeWidget->setParent(m_flowWidget);
    // 默认位置将在initializeModulePositions中设置
}

void ClockConfigWidget::setupClkRVPLLSubNodes()
{
    // 创建clk_rvpll子节点区域
    m_clkRVPLLSubNodeWidget = new QWidget();
    m_clkRVPLLSubNodeWidget->setFixedSize(150, 120); // 设置固定尺寸，1个子节点
    m_clkRVPLLSubNodeWidget->setStyleSheet(
        "QWidget { "
        "background-color: #f0f8ff; "
        "border: 2px solid #4169e1; "
        "border-radius: 8px; "
        "}"
    );

    m_clkRVPLLSubNodeLayout = new QVBoxLayout(m_clkRVPLLSubNodeWidget);
    m_clkRVPLLSubNodeLayout->setContentsMargins(10, 10, 10, 10);
    m_clkRVPLLSubNodeLayout->setSpacing(5);

    // 添加标题
    QLabel* subNodeTitle = new QLabel("clk_rvpll子节点");
    subNodeTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #0066cc; text-align: center;");
    subNodeTitle->setAlignment(Qt::AlignCenter);
    m_clkRVPLLSubNodeLayout->addWidget(subNodeTitle);

    // 为每个clk_rvpll子节点创建显示界面
    for (const QString& nodeName : CLK_RVPLL_SUB_NODES) {
        createClkRVPLLSubNodeWidget(nodeName, m_clkRVPLLSubNodeWidget);

        // 初始化clk_rvpll子节点配置
        ClockOutput subNode;
        subNode.name = nodeName;
        subNode.source = "clk_rvpll";
        subNode.enabled = true;
        subNode.divider = 2;  // clk_rvpll默认分频系数是2

        // 计算频率：clk_rvpll的频率 / 分频器
        // 从clk_rvpll的配置中获取正确的频率
        double clkRVPLLFreq = 1600.0; // 默认1600MHz
        if (m_pllConfigs.contains("clk_rvpll")) {
            clkRVPLLFreq = m_pllConfigs["clk_rvpll"].outputFreq;
        }
        subNode.frequency = clkRVPLLFreq / subNode.divider;

        m_clkRVPLLSubNodes[nodeName] = subNode;
    }

    m_clkRVPLLSubNodeLayout->addStretch();

    // 设置父widget为m_flowWidget并使用绝对定位
    m_clkRVPLLSubNodeWidget->setParent(m_flowWidget);
    // 默认位置将在initializeModulePositions中设置
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
    } else if (pllName == "clk_mipimpll") {
        defaultMultiplier = 36;
    } else if (pllName == "MPLL") {
        defaultMultiplier = 48;
    } else if (pllName == "TPLL") {
        defaultMultiplier = 60;
    } else if (pllName == "APPLL") {
        defaultMultiplier = 40;
    } else if (pllName == "clk_rvpll") {
        defaultMultiplier = 64;
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
    } else if (pllName == "clk_mipimpll") {
        freqText = "900.000 MHz";   // 25 * 36
    } else if (pllName == "MPLL") {
        freqText = "1200.000 MHz";  // 25 * 48
    } else if (pllName == "TPLL") {
        freqText = "1500.000 MHz";  // 25 * 60
    } else if (pllName == "APPLL") {
        freqText = "1000.000 MHz";  // 25 * 40
    } else if (pllName == "clk_rvpll") {
        freqText = "1600.000 MHz";  // 25 * 64
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
    if (pllName == "clk_a0pll") {
        defaultDivider = 7.32421875;
    } else if (pllName == "clk_cam0pll") {
        defaultDivider = 36.0;
    } else if (pllName == "clk_cam1pll") {
        defaultDivider = 36.0;
    } else if (pllName == "clk_disppll") {
        defaultDivider = 9.09090909;
    } else if (pllName == "clk_mipimpll_d3") {
        defaultDivider = 3.0;
    } else if (pllName == "clk_cyc_dsi_syn") {
        defaultDivider = 1.0;
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
    if (pllName == "clk_a0pll") {
        defaultMultiplier = 4;
    } else if (pllName == "clk_cam0pll") {
        defaultMultiplier = 52;
    } else if (pllName == "clk_cam1pll") {
        defaultMultiplier = 64;
    } else if (pllName == "clk_disppll") {
        defaultMultiplier = 12;
    } else if (pllName == "clk_mipimpll_d3") {
        defaultMultiplier = 1;
    } else if (pllName == "clk_cyc_dsi_syn") {
        defaultMultiplier = 1;
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
        "padding: 3px; "
        "margin: 1px; "
        "}"
    );
    
    QVBoxLayout* layout = new QVBoxLayout(outputWidget);
    layout->setSpacing(2);
    layout->setContentsMargins(3, 3, 3, 3);
    
    // 输出名称标签
    QLabel* nameLabel = new QLabel(outputName);
    nameLabel->setStyleSheet("font-weight: bold; color: #721c24; font-size: 11px;");
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setWordWrap(true);
    
    // 分频器配置
    QHBoxLayout* divConfigLayout = new QHBoxLayout();
    divConfigLayout->setSpacing(3);
    
    QLabel* divLabel = new QLabel("分频：");
    divLabel->setStyleSheet("color: #721c24; font-size: 10px; font-weight: bold;");
    
    QSpinBox* divBox = new QSpinBox();
    divBox->setRange(1, 1000);
    
    // 根据输出名称设置不同的默认分频值
    int defaultDivider = 1;
    if (outputName == "clk_usb20_suspend") {
        defaultDivider = 125;
    } else if (outputName == "clk_1M") {
        defaultDivider = 250;
    }
    
    divBox->setValue(defaultDivider);
    divBox->setFixedWidth(45);
    divBox->setStyleSheet("font-size: 10px;");
    
    divConfigLayout->addWidget(divLabel);
    divConfigLayout->addWidget(divBox);
    
    // 频率显示
    double calculatedFreq = OSC_FREQUENCY / defaultDivider;
    QString freqText;
    if (calculatedFreq >= 1.0) {
        freqText = QString("%1 MHz").arg(calculatedFreq, 0, 'f', 3);
    } else {
        freqText = QString("%1 kHz").arg(calculatedFreq * 1000, 0, 'f', 1);
    }
    
    QLabel* freqLabel = new QLabel(freqText);
    freqLabel->setStyleSheet("color: #dc3545; font-family: monospace; font-weight: bold; font-size: 9px;");
    freqLabel->setAlignment(Qt::AlignCenter);
    
    layout->addWidget(nameLabel);
    layout->addLayout(divConfigLayout);
    layout->addWidget(freqLabel);
    
    // 存储控件引用
    m_outputWidgets[outputName] = outputWidget;
    m_outputFreqLabels[outputName] = freqLabel;
    m_outputDividerBoxes[outputName] = divBox;  // 新增：存储分频器控件
    
    m_outputLayout->addWidget(outputWidget);
}

void ClockConfigWidget::createClk1MSubNodeWidget(const QString& nodeName, QWidget* parent)
{
    QWidget* subNodeWidget = new QWidget();
    subNodeWidget->setStyleSheet(
        "QWidget { "
        "background-color: #ffffff; "
        "border: 1px solid #007bff; "
        "border-radius: 4px; "
        "padding: 3px; "
        "margin: 1px; "
        "}"
    );
    
    QVBoxLayout* layout = new QVBoxLayout(subNodeWidget);
    layout->setSpacing(2);
    layout->setContentsMargins(3, 3, 3, 3);
    
    // 子节点名称标签
    QLabel* nameLabel = new QLabel(nodeName);
    nameLabel->setStyleSheet("font-weight: bold; color: #004085; font-size: 11px;");
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setWordWrap(true);
    
    // 分频器配置
    QHBoxLayout* divConfigLayout = new QHBoxLayout();
    divConfigLayout->setSpacing(3);
    
    QLabel* divLabel = new QLabel("分频：");
    divLabel->setStyleSheet("color: #004085; font-size: 10px; font-weight: bold;");
    
    QSpinBox* divBox = new QSpinBox();
    divBox->setRange(1, 1000);
    divBox->setValue(1);  // 默认分频为1
    divBox->setFixedWidth(45);
    divBox->setStyleSheet("font-size: 10px;");
    
    divConfigLayout->addWidget(divLabel);
    divConfigLayout->addWidget(divBox);
    
    // 频率显示
    double clk1MFreq = OSC_FREQUENCY / 250;  // clk_1M = 25MHz / 250 = 0.1MHz
    double calculatedFreq = clk1MFreq / 1;  // 默认分频为1
    QString freqText;
    if (calculatedFreq >= 1.0) {
        freqText = QString("%1 MHz").arg(calculatedFreq, 0, 'f', 3);
    } else {
        freqText = QString("%1 kHz").arg(calculatedFreq * 1000, 0, 'f', 1);
    }
    
    QLabel* freqLabel = new QLabel(freqText);
    freqLabel->setStyleSheet("color: #dc3545; font-family: monospace; font-weight: bold; font-size: 9px;");
    freqLabel->setAlignment(Qt::AlignCenter);
    
    layout->addWidget(nameLabel);
    layout->addLayout(divConfigLayout);
    layout->addWidget(freqLabel);
    
    // 存储控件引用
    m_clk1MSubNodeWidgets[nodeName] = subNodeWidget;
    m_clk1MSubNodeFreqLabels[nodeName] = freqLabel;
    m_clk1MSubNodeDividerBoxes[nodeName] = divBox;
    
    m_clk1MSubNodeLayout->addWidget(subNodeWidget);
}

void ClockConfigWidget::createClkCam1PLLSubNodeWidget(const QString& nodeName, QWidget* parent)
{
    QWidget* subNodeWidget = new QWidget();
    subNodeWidget->setStyleSheet(
        "QWidget { "
        "background-color: #ffffff; "
        "border: 1px solid #ffc107; "
        "border-radius: 4px; "
        "padding: 3px; "
        "margin: 1px; "
        "}"
    );
    
    QVBoxLayout* layout = new QVBoxLayout(subNodeWidget);
    layout->setSpacing(2);
    layout->setContentsMargins(3, 3, 3, 3);
    
    // 子节点名称标签
    QLabel* nameLabel = new QLabel(nodeName);
    nameLabel->setStyleSheet("font-weight: bold; color: #856404; font-size: 11px;");
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setWordWrap(true);
    
    // 分频器配置
    QHBoxLayout* divConfigLayout = new QHBoxLayout();
    divConfigLayout->setSpacing(3);
    
    QLabel* divLabel = new QLabel("分频：");
    divLabel->setStyleSheet("color: #856404; font-size: 10px; font-weight: bold;");
    
    QSpinBox* divBox = new QSpinBox();
    divBox->setRange(1, 1000);
    divBox->setValue(4);  // 默认分频为4
    divBox->setFixedWidth(45);
    divBox->setStyleSheet("font-size: 10px;");
    
    divConfigLayout->addWidget(divLabel);
    divConfigLayout->addWidget(divBox);
    
    // 频率显示
    double clkCam1PLLFreq = 800.0;  // 假设clk_cam1pll = 800MHz
    double calculatedFreq = clkCam1PLLFreq / 4;  // 默认分频为4
    QString freqText = QString("%1 MHz").arg(calculatedFreq, 0, 'f', 1);
    
    QLabel* freqLabel = new QLabel(freqText);
    freqLabel->setStyleSheet("color: #dc3545; font-family: monospace; font-weight: bold; font-size: 9px;");
    freqLabel->setAlignment(Qt::AlignCenter);
    
    layout->addWidget(nameLabel);
    layout->addLayout(divConfigLayout);
    layout->addWidget(freqLabel);
    
    // 存储控件引用
    m_clkCam1PLLSubNodeWidgets[nodeName] = subNodeWidget;
    m_clkCam1PLLSubNodeFreqLabels[nodeName] = freqLabel;
    m_clkCam1PLLSubNodeDividerBoxes[nodeName] = divBox;
    
    m_clkCam1PLLSubNodeLayout->addWidget(subNodeWidget);
    
    // 连接分频器变化信号
    connect(divBox, QOverload<int>::of(&QSpinBox::valueChanged),
            [this, nodeName](int value) {
                onClkCam1PLLSubNodeDividerChanged(nodeName, value);
            });
}

void ClockConfigWidget::createClkRawAxiSubNodeWidget(const QString& nodeName, QWidget* parent)
{
    QWidget* subNodeWidget = new QWidget();
    subNodeWidget->setStyleSheet(
        "QWidget { "
        "background-color: #ffffff; "
        "border: 1px solid #daa520; "
        "border-radius: 4px; "
        "padding: 3px; "
        "margin: 1px; "
        "}"
    );
    
    QVBoxLayout* layout = new QVBoxLayout(subNodeWidget);
    layout->setSpacing(2);
    layout->setContentsMargins(3, 3, 3, 3);
    
    // 子节点名称标签
    QLabel* nameLabel = new QLabel(nodeName);
    nameLabel->setStyleSheet("font-weight: bold; color: #8b7355; font-size: 11px;");
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setWordWrap(true);
    
    // 分频器配置
    QHBoxLayout* divConfigLayout = new QHBoxLayout();
    divConfigLayout->setSpacing(3);
    
    QLabel* divLabel = new QLabel("分频：");
    divLabel->setStyleSheet("color: #8b7355; font-size: 10px; font-weight: bold;");
    
    QSpinBox* divBox = new QSpinBox();
    divBox->setRange(1, 1000);
    divBox->setValue(1);  // 默认分频为1
    divBox->setFixedWidth(45);
    divBox->setStyleSheet("font-size: 10px;");
    
    divConfigLayout->addWidget(divLabel);
    divConfigLayout->addWidget(divBox);
    
    // 频率显示
    double clkRawAxiFreq = 200.0;  // 假设clk_raw_axi = 200MHz
    double calculatedFreq = clkRawAxiFreq / 1;  // 默认分频为1
    QString freqText = QString("%1 MHz").arg(calculatedFreq, 0, 'f', 1);
    
    QLabel* freqLabel = new QLabel(freqText);
    freqLabel->setStyleSheet("color: #dc3545; font-family: monospace; font-weight: bold; font-size: 9px;");
    freqLabel->setAlignment(Qt::AlignCenter);
    
    layout->addWidget(nameLabel);
    layout->addLayout(divConfigLayout);
    layout->addWidget(freqLabel);
    
    // 存储控件引用
    m_clkRawAxiSubNodeWidgets[nodeName] = subNodeWidget;
    m_clkRawAxiSubNodeFreqLabels[nodeName] = freqLabel;
    m_clkRawAxiSubNodeDividerBoxes[nodeName] = divBox;
    
    m_clkRawAxiSubNodeLayout->addWidget(subNodeWidget);
    
    // 连接分频器变化信号
    connect(divBox, QOverload<int>::of(&QSpinBox::valueChanged),
            [this, nodeName](int value) {
                onClkRawAxiSubNodeDividerChanged(nodeName, value);
            });
}

void ClockConfigWidget::createClkCam0PLLSubNodeWidget(const QString& nodeName, QWidget* parent)
{
    QWidget* subNodeWidget = new QWidget();
    subNodeWidget->setStyleSheet(
        "QWidget { "
        "background-color: #ffffff; "
        "border: 1px solid #4da6ff; "
        "border-radius: 4px; "
        "padding: 3px; "
        "margin: 1px; "
        "}"
    );
    
    QVBoxLayout* layout = new QVBoxLayout(subNodeWidget);
    layout->setSpacing(2);
    layout->setContentsMargins(3, 3, 3, 3);
    
    // 子节点名称标签
    QLabel* nameLabel = new QLabel(nodeName);
    nameLabel->setStyleSheet("font-weight: bold; color: #0066cc; font-size: 11px;");
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setWordWrap(true);
    
    // 分频器配置
    QHBoxLayout* divConfigLayout = new QHBoxLayout();
    divConfigLayout->setSpacing(3);
    
    QLabel* divLabel = new QLabel("分频：");
    divLabel->setStyleSheet("color: #0066cc; font-size: 10px; font-weight: bold;");
    
    QSpinBox* divBox = new QSpinBox();
    divBox->setRange(1, 1000);
    // 根据子节点设置不同的默认分频值
    if (nodeName == "clk_cam0_vip") {
        divBox->setValue(50);  // clk_cam0_vip默认分频系数是50
    } else {
        divBox->setValue(2);   // clk_vc_src0和clk_tpu_gdma默认分频系数是2
    }
    divBox->setFixedWidth(45);
    divBox->setStyleSheet("font-size: 10px;");
    
    divConfigLayout->addWidget(divLabel);
    divConfigLayout->addWidget(divBox);
    
    // 频率显示
    double clkCam0PLLFreq = 1000.0;  // 假设clk_cam0pll = 1000MHz
    double calculatedFreq;
    if (nodeName == "clk_cam0_vip") {
        calculatedFreq = clkCam0PLLFreq / 50;  // 默认分频为50
    } else {
        calculatedFreq = clkCam0PLLFreq / 2;   // 默认分频为2
    }
    QString freqText = QString("%1 MHz").arg(calculatedFreq, 0, 'f', 1);
    
    QLabel* freqLabel = new QLabel(freqText);
    freqLabel->setStyleSheet("color: #dc3545; font-family: monospace; font-weight: bold; font-size: 9px;");
    freqLabel->setAlignment(Qt::AlignCenter);
    
    layout->addWidget(nameLabel);
    layout->addLayout(divConfigLayout);
    layout->addWidget(freqLabel);
    
    // 存储控件引用
    m_clkCam0PLLSubNodeWidgets[nodeName] = subNodeWidget;
    m_clkCam0PLLSubNodeFreqLabels[nodeName] = freqLabel;
    m_clkCam0PLLSubNodeDividerBoxes[nodeName] = divBox;
    
    m_clkCam0PLLSubNodeLayout->addWidget(subNodeWidget);
    
    // 连接分频器变化信号
    connect(divBox, QOverload<int>::of(&QSpinBox::valueChanged),
            [this, nodeName](int value) {
                onClkCam0PLLSubNodeDividerChanged(nodeName, value);
            });
}

void ClockConfigWidget::createClkDispPLLSubNodeWidget(const QString& nodeName, QWidget* parent)
{
    QWidget* subNodeWidget = new QWidget();
    subNodeWidget->setStyleSheet(
        "QWidget { "
        "background-color: #ffffff; "
        "border: 1px solid #ff66b3; "
        "border-radius: 4px; "
        "padding: 3px; "
        "margin: 1px; "
        "}"
    );
    
    QVBoxLayout* layout = new QVBoxLayout(subNodeWidget);
    layout->setSpacing(2);
    layout->setContentsMargins(3, 3, 3, 3);
    
    // 子节点名称标签
    QLabel* nameLabel = new QLabel(nodeName);
    nameLabel->setStyleSheet("font-weight: bold; color: #cc0066; font-size: 11px;");
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setWordWrap(true);
    
    // 分频器配置
    QHBoxLayout* divConfigLayout = new QHBoxLayout();
    divConfigLayout->setSpacing(3);
    
    QLabel* divLabel = new QLabel("分频：");
    divLabel->setStyleSheet("color: #cc0066; font-size: 10px; font-weight: bold;");
    
    QSpinBox* divBox = new QSpinBox();
    divBox->setRange(1, 1000);
    // 根据子节点设置不同的默认分频值
    if (nodeName == "clk_cam2_vip") {
        divBox->setValue(32);  // clk_cam2_vip默认分频系数是32
    } else if (nodeName == "clk_cam1_vip") {
        divBox->setValue(44);  // clk_cam1_vip默认分频系数是44
    } else if (nodeName == "clk_sys_disp") {
        divBox->setValue(8);   // clk_sys_disp默认分频系数是8
    }
    divBox->setFixedWidth(45);
    divBox->setStyleSheet("font-size: 10px;");
    
    divConfigLayout->addWidget(divLabel);
    divConfigLayout->addWidget(divBox);
    
    // 频率显示
    double clkDispPLLFreq = 1200.0;  // 假设clk_disppll = 1200MHz
    double calculatedFreq;
    if (nodeName == "clk_cam2_vip") {
        calculatedFreq = clkDispPLLFreq / 32;  // 默认分频为32
    } else if (nodeName == "clk_cam1_vip") {
        calculatedFreq = clkDispPLLFreq / 44;  // 默认分频为44
    } else if (nodeName == "clk_sys_disp") {
        calculatedFreq = clkDispPLLFreq / 8;   // 默认分频为8
    }
    QString freqText = QString("%1 MHz").arg(calculatedFreq, 0, 'f', 1);
    
    QLabel* freqLabel = new QLabel(freqText);
    freqLabel->setStyleSheet("color: #dc3545; font-family: monospace; font-weight: bold; font-size: 9px;");
    freqLabel->setAlignment(Qt::AlignCenter);
    
    layout->addWidget(nameLabel);
    layout->addLayout(divConfigLayout);
    layout->addWidget(freqLabel);
    
    // 存储控件引用
    m_clkDispPLLSubNodeWidgets[nodeName] = subNodeWidget;
    m_clkDispPLLSubNodeFreqLabels[nodeName] = freqLabel;
    m_clkDispPLLSubNodeDividerBoxes[nodeName] = divBox;
    
    m_clkDispPLLSubNodeLayout->addWidget(subNodeWidget);
    
    // 连接分频器变化信号
    connect(divBox, QOverload<int>::of(&QSpinBox::valueChanged),
            [this, nodeName](int value) {
                onClkDispPLLSubNodeDividerChanged(nodeName, value);
            });
}

void ClockConfigWidget::createClkSysDispSubNodeWidget(const QString& nodeName, QWidget* parent)
{
    QWidget* subNodeWidget = new QWidget();
    subNodeWidget->setStyleSheet(
        "QWidget { "
        "background-color: #ffffff; "
        "border: 1px solid #3399ff; "
        "border-radius: 4px; "
        "padding: 3px; "
        "margin: 1px; "
        "}"
    );
    
    QVBoxLayout* layout = new QVBoxLayout(subNodeWidget);
    layout->setSpacing(2);
    layout->setContentsMargins(3, 3, 3, 3);
    
    // 子节点名称标签
    QLabel* nameLabel = new QLabel(nodeName);
    nameLabel->setStyleSheet("font-weight: bold; color: #0066cc; font-size: 11px;");
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setWordWrap(true);
    
    // 分频器配置
    QHBoxLayout* divConfigLayout = new QHBoxLayout();
    divConfigLayout->setSpacing(3);
    
    QLabel* divLabel = new QLabel("分频：");
    divLabel->setStyleSheet("color: #0066cc; font-size: 10px; font-weight: bold;");
    
    QSpinBox* divBox = new QSpinBox();
    divBox->setRange(1, 1000);
    divBox->setValue(1);  // clk_disp_vip默认分频系数是1
    divBox->setFixedWidth(45);
    divBox->setStyleSheet("font-size: 10px;");
    
    divConfigLayout->addWidget(divLabel);
    divConfigLayout->addWidget(divBox);
    
    // 频率显示
    double clkSysDispFreq = 150.0;  // clk_sys_disp = clk_disppll/8 = 1200/8 = 150MHz
    double calculatedFreq = clkSysDispFreq / 1;  // 默认分频为1
    QString freqText = QString("%1 MHz").arg(calculatedFreq, 0, 'f', 1);
    
    QLabel* freqLabel = new QLabel(freqText);
    freqLabel->setStyleSheet("color: #dc3545; font-family: monospace; font-weight: bold; font-size: 9px;");
    freqLabel->setAlignment(Qt::AlignCenter);
    
    layout->addWidget(nameLabel);
    layout->addLayout(divConfigLayout);
    layout->addWidget(freqLabel);
    
    // 存储控件引用
    m_clkSysDispSubNodeWidgets[nodeName] = subNodeWidget;
    m_clkSysDispSubNodeFreqLabels[nodeName] = freqLabel;
    m_clkSysDispSubNodeDividerBoxes[nodeName] = divBox;
    
    m_clkSysDispSubNodeLayout->addWidget(subNodeWidget);
    
    // 连接分频器变化信号
    connect(divBox, QOverload<int>::of(&QSpinBox::valueChanged),
            [this, nodeName](int value) {
                onClkSysDispSubNodeDividerChanged(nodeName, value);
            });
}

void ClockConfigWidget::createClkA0PLLSubNodeWidget(const QString& nodeName, QWidget* parent)
{
    QWidget* subNodeWidget = new QWidget();
    subNodeWidget->setStyleSheet(
        "QWidget { "
        "background-color: #ffffff; "
        "border: 1px solid #4169e1; "
        "border-radius: 4px; "
        "padding: 3px; "
        "margin: 1px; "
        "}"
    );
    
    QVBoxLayout* layout = new QVBoxLayout(subNodeWidget);
    layout->setSpacing(2);
    layout->setContentsMargins(3, 3, 3, 3);
    
    // 子节点名称标签
    QLabel* nameLabel = new QLabel(nodeName);
    nameLabel->setStyleSheet("font-weight: bold; color: #191970; font-size: 11px;");
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setWordWrap(true);
    
    // 分频器配置
    QHBoxLayout* divConfigLayout = new QHBoxLayout();
    divConfigLayout->setSpacing(3);
    
    QLabel* divLabel = new QLabel("分频：");
    divLabel->setStyleSheet("color: #191970; font-size: 10px; font-weight: bold;");
    
    QSpinBox* divBox = new QSpinBox();
    divBox->setRange(1, 1000);
    divBox->setValue(17);  // 默认分频系数是17
    divBox->setFixedWidth(45);
    divBox->setStyleSheet("font-size: 10px;");
    
    divConfigLayout->addWidget(divLabel);
    divConfigLayout->addWidget(divBox);
    
    // 频率显示
    double clkA0PLLFreq = 500.0;  // clk_a0pll假设为500MHz
    double calculatedFreq = clkA0PLLFreq / 17;  // 默认分频为17
    QString freqText = QString("%1 MHz").arg(calculatedFreq, 0, 'f', 1);
    
    QLabel* freqLabel = new QLabel(freqText);
    freqLabel->setStyleSheet("color: #dc3545; font-family: monospace; font-weight: bold; font-size: 9px;");
    freqLabel->setAlignment(Qt::AlignCenter);
    
    layout->addWidget(nameLabel);
    layout->addLayout(divConfigLayout);
    layout->addWidget(freqLabel);
    
    // 存储控件引用
    m_clkA0PLLSubNodeWidgets[nodeName] = subNodeWidget;
    m_clkA0PLLSubNodeFreqLabels[nodeName] = freqLabel;
    m_clkA0PLLSubNodeDividerBoxes[nodeName] = divBox;
    
    m_clkA0PLLSubNodeLayout->addWidget(subNodeWidget);
    
    // 连接分频器变化信号
    connect(divBox, QOverload<int>::of(&QSpinBox::valueChanged),
            [this, nodeName](int value) {
                onClkA0PLLSubNodeDividerChanged(nodeName, value);
            });
}

void ClockConfigWidget::createClkRVPLLSubNodeWidget(const QString& nodeName, QWidget* parent)
{
    QWidget* subNodeWidget = new QWidget();
    subNodeWidget->setStyleSheet(
        "QWidget { "
        "background-color: #ffffff; "
        "border: 1px solid #4169e1; "
        "border-radius: 4px; "
        "padding: 3px; "
        "margin: 1px; "
        "}"
    );

    QVBoxLayout* layout = new QVBoxLayout(subNodeWidget);
    layout->setSpacing(2);
    layout->setContentsMargins(3, 3, 3, 3);

    // 子节点名称标签
    QLabel* nameLabel = new QLabel(nodeName);
    nameLabel->setStyleSheet("font-weight: bold; color: #191970; font-size: 11px;");
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setWordWrap(true);

    // 分频器配置
    QHBoxLayout* divConfigLayout = new QHBoxLayout();
    divConfigLayout->setSpacing(3);

    QLabel* divLabel = new QLabel("分频：");
    divLabel->setStyleSheet("color: #191970; font-size: 10px; font-weight: bold;");

    QSpinBox* divBox = new QSpinBox();
    divBox->setRange(1, 1000);
    divBox->setValue(2);  // 默认分频系数是2
    divBox->setFixedWidth(45);
    divBox->setStyleSheet("font-size: 10px;");

    divConfigLayout->addWidget(divLabel);
    divConfigLayout->addWidget(divBox);

    // 频率显示
    double clkRVPLLFreq = 1600.0;  // 默认1600MHz
    if (m_pllConfigs.contains("clk_rvpll")) {
        clkRVPLLFreq = m_pllConfigs["clk_rvpll"].outputFreq;
    }
    double calculatedFreq = clkRVPLLFreq / 2;  // 默认分频为2
    QString freqText = QString("%1 MHz").arg(calculatedFreq, 0, 'f', 1);

    QLabel* freqLabel = new QLabel(freqText);
    freqLabel->setStyleSheet("color: #dc3545; font-family: monospace; font-weight: bold; font-size: 9px;");
    freqLabel->setAlignment(Qt::AlignCenter);

    layout->addWidget(nameLabel);
    layout->addLayout(divConfigLayout);
    layout->addWidget(freqLabel);

    // 存储控件引用
    m_clkRVPLLSubNodeWidgets[nodeName] = subNodeWidget;
    m_clkRVPLLSubNodeFreqLabels[nodeName] = freqLabel;
    m_clkRVPLLSubNodeDividerBoxes[nodeName] = divBox;

    m_clkRVPLLSubNodeLayout->addWidget(subNodeWidget);

    // 连接分频器变化信号
    connect(divBox, QOverload<int>::of(&QSpinBox::valueChanged),
            [this, nodeName](int value) {
                onClkRVPLLSubNodeDividerChanged(nodeName, value);
            });
}

void ClockConfigWidget::setupClockTree()
{
    // 由于现在使用流程图布局，时钟树功能可以简化或移除
    // 这里保留一个简单的实现作为备用
}

void ClockConfigWidget::initializeModulePositions()
{
    // 初始化各个模块的默认位置: x, y, 宽度, 高度
    ModulePosition inputPos = {"输入源", 20, 50, 150, 400};
    ModulePosition pllPos = {"锁相环", 190, 50, 200, 700};
    ModulePosition subPllPos = {"子锁相环", 410, 50, 180, 780};
    ModulePosition outputPos = {"OSC输出", 610, 50, 200, 1800}; // 调整高度以容纳所有输出节点
    ModulePosition clk1MSubPos = {"clk_1M子节点", 830, 50, 150, 350};
    ModulePosition clkCam1PLLSubPos = {"clk_cam1pll子节点", 1000, 50, 150, 450}; // 新增clk_cam1pll子节点位置
    ModulePosition clkRawAxiSubPos = {"clk_raw_axi子节点", 1170, 50, 180, 550}; // 新增clk_raw_axi子节点位置
    ModulePosition clkCam0PLLSubPos = {"clk_cam0pll子节点", 1370, 50, 150, 250}; // 新增clk_cam0pll子节点位置
    ModulePosition clkDispPLLSubPos = {"clk_disppll子节点", 1540, 50, 150, 250}; // 新增clk_disppll子节点位置
    ModulePosition clkSysDispSubPos = {"clk_sys_disp子节点", 1710, 50, 150, 120}; // 新增clk_sys_disp子节点位置
    ModulePosition clkA0PLLSubPos = {"clk_a0pll子节点", 1880, 50, 150, 450}; // 新增clk_a0pll子节点位置
    ModulePosition clkRVPLLSubPos = {"clk_rvpll子节点", 1880, 600, 150, 120}; // 新增clk_rvpll子节点位置

    m_modulePositions["输入源"] = inputPos;
    m_modulePositions["锁相环"] = pllPos;
    m_modulePositions["子锁相环"] = subPllPos;
    m_modulePositions["OSC输出"] = outputPos;
    m_modulePositions["clk_1M子节点"] = clk1MSubPos;
    m_modulePositions["clk_cam1pll子节点"] = clkCam1PLLSubPos;
    m_modulePositions["clk_raw_axi子节点"] = clkRawAxiSubPos;
    m_modulePositions["clk_cam0pll子节点"] = clkCam0PLLSubPos;
    m_modulePositions["clk_disppll子节点"] = clkDispPLLSubPos;
    m_modulePositions["clk_sys_disp子节点"] = clkSysDispSubPos;
    m_modulePositions["clk_a0pll子节点"] = clkA0PLLSubPos;
    m_modulePositions["clk_rvpll子节点"] = clkRVPLLSubPos;
    
    // 应用默认位置
    applyModulePositions();
}

void ClockConfigWidget::applyModulePositions()
{
    if (m_modulePositions.contains("输入源") && m_inputWidget) {
        ModulePosition pos = m_modulePositions["输入源"];
        m_inputWidget->move(pos.x, pos.y);
        m_inputWidget->resize(pos.width, pos.height);
    }
    
    if (m_modulePositions.contains("锁相环") && m_pllWidget) {
        ModulePosition pos = m_modulePositions["锁相环"];
        m_pllWidget->move(pos.x, pos.y);
        m_pllWidget->resize(pos.width, pos.height);
    }
    
    if (m_modulePositions.contains("子锁相环") && m_subPllWidget) {
        ModulePosition pos = m_modulePositions["子锁相环"];
        m_subPllWidget->move(pos.x, pos.y);
        m_subPllWidget->resize(pos.width, pos.height);
    }
    
    if (m_modulePositions.contains("OSC输出") && m_outputWidget) {
        ModulePosition pos = m_modulePositions["OSC输出"];
        m_outputWidget->move(pos.x, pos.y);
        m_outputWidget->resize(pos.width, pos.height);
    }
    
    if (m_modulePositions.contains("clk_1M子节点") && m_clk1MSubNodeWidget) {
        ModulePosition pos = m_modulePositions["clk_1M子节点"];
        m_clk1MSubNodeWidget->move(pos.x, pos.y);
        m_clk1MSubNodeWidget->resize(pos.width, pos.height);
    }
    
    if (m_modulePositions.contains("clk_cam1pll子节点") && m_clkCam1PLLSubNodeWidget) {
        ModulePosition pos = m_modulePositions["clk_cam1pll子节点"];
        m_clkCam1PLLSubNodeWidget->move(pos.x, pos.y);
        m_clkCam1PLLSubNodeWidget->resize(pos.width, pos.height);
    }
    
    if (m_modulePositions.contains("clk_raw_axi子节点") && m_clkRawAxiSubNodeWidget) {
        ModulePosition pos = m_modulePositions["clk_raw_axi子节点"];
        m_clkRawAxiSubNodeWidget->move(pos.x, pos.y);
        m_clkRawAxiSubNodeWidget->resize(pos.width, pos.height);
    }
    
    if (m_modulePositions.contains("clk_cam0pll子节点") && m_clkCam0PLLSubNodeWidget) {
        ModulePosition pos = m_modulePositions["clk_cam0pll子节点"];
        m_clkCam0PLLSubNodeWidget->move(pos.x, pos.y);
        m_clkCam0PLLSubNodeWidget->resize(pos.width, pos.height);
    }
    
    if (m_modulePositions.contains("clk_disppll子节点") && m_clkDispPLLSubNodeWidget) {
        ModulePosition pos = m_modulePositions["clk_disppll子节点"];
        m_clkDispPLLSubNodeWidget->move(pos.x, pos.y);
        m_clkDispPLLSubNodeWidget->resize(pos.width, pos.height);
    }
    
    if (m_modulePositions.contains("clk_sys_disp子节点") && m_clkSysDispSubNodeWidget) {
        ModulePosition pos = m_modulePositions["clk_sys_disp子节点"];
        m_clkSysDispSubNodeWidget->move(pos.x, pos.y);
        m_clkSysDispSubNodeWidget->resize(pos.width, pos.height);
    }
    
    if (m_modulePositions.contains("clk_a0pll子节点") && m_clkA0PLLSubNodeWidget) {
        ModulePosition pos = m_modulePositions["clk_a0pll子节点"];
        m_clkA0PLLSubNodeWidget->move(pos.x, pos.y);
        m_clkA0PLLSubNodeWidget->resize(pos.width, pos.height);
    }

    if (m_modulePositions.contains("clk_rvpll子节点") && m_clkRVPLLSubNodeWidget) {
        ModulePosition pos = m_modulePositions["clk_rvpll子节点"];
        m_clkRVPLLSubNodeWidget->move(pos.x, pos.y);
        m_clkRVPLLSubNodeWidget->resize(pos.width, pos.height);
    }
    
    // 重绘连接线
    updateConnectionOverlay();
}

void ClockConfigWidget::showPositionConfigDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle("模块位置配置");
    dialog.setModal(true);
    dialog.resize(400, 350);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(&dialog);
    
    QFormLayout* formLayout = new QFormLayout();
    
    // 为每个模块创建位置配置控件
    QMap<QString, QSpinBox*> xSpinBoxes;
    QMap<QString, QSpinBox*> ySpinBoxes;
    QMap<QString, QSpinBox*> widthSpinBoxes;
    QMap<QString, QSpinBox*> heightSpinBoxes;
    
    QStringList moduleNames = {"输入源", "锁相环", "子锁相环", "OSC输出", "clk_1M子节点", "clk_cam1pll子节点", "clk_raw_axi子节点", "clk_cam0pll子节点"};
    
    for (const QString& moduleName : moduleNames) {
        QGroupBox* moduleGroup = new QGroupBox(moduleName);
        QGridLayout* moduleLayout = new QGridLayout(moduleGroup);
        
        // X坐标
        QLabel* xLabel = new QLabel("X坐标:");
        QSpinBox* xSpinBox = new QSpinBox();
        xSpinBox->setRange(0, 2000);
        if (m_modulePositions.contains(moduleName)) {
            xSpinBox->setValue(m_modulePositions[moduleName].x);
        }
        xSpinBoxes[moduleName] = xSpinBox;
        
        // Y坐标
        QLabel* yLabel = new QLabel("Y坐标:");
        QSpinBox* ySpinBox = new QSpinBox();
        ySpinBox->setRange(0, 2000);
        if (m_modulePositions.contains(moduleName)) {
            ySpinBox->setValue(m_modulePositions[moduleName].y);
        }
        ySpinBoxes[moduleName] = ySpinBox;
        
        // 宽度
        QLabel* widthLabel = new QLabel("宽度:");
        QSpinBox* widthSpinBox = new QSpinBox();
        widthSpinBox->setRange(50, 500);
        if (m_modulePositions.contains(moduleName)) {
            widthSpinBox->setValue(m_modulePositions[moduleName].width);
        }
        widthSpinBoxes[moduleName] = widthSpinBox;
        
        // 高度
        QLabel* heightLabel = new QLabel("高度:");
        QSpinBox* heightSpinBox = new QSpinBox();
        heightSpinBox->setRange(50, 800);
        if (m_modulePositions.contains(moduleName)) {
            heightSpinBox->setValue(m_modulePositions[moduleName].height);
        }
        heightSpinBoxes[moduleName] = heightSpinBox;
        
        moduleLayout->addWidget(xLabel, 0, 0);
        moduleLayout->addWidget(xSpinBox, 0, 1);
        moduleLayout->addWidget(yLabel, 0, 2);
        moduleLayout->addWidget(ySpinBox, 0, 3);
        moduleLayout->addWidget(widthLabel, 1, 0);
        moduleLayout->addWidget(widthSpinBox, 1, 1);
        moduleLayout->addWidget(heightLabel, 1, 2);
        moduleLayout->addWidget(heightSpinBox, 1, 3);
        
        formLayout->addRow(moduleGroup);
    }
    
    mainLayout->addLayout(formLayout);
    
    // 添加按钮
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::RestoreDefaults);
    
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(buttonBox->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked, [&]() {
        // 恢复默认值
        initializeModulePositions();
        for (const QString& moduleName : moduleNames) {
            if (m_modulePositions.contains(moduleName)) {
                ModulePosition pos = m_modulePositions[moduleName];
                xSpinBoxes[moduleName]->setValue(pos.x);
                ySpinBoxes[moduleName]->setValue(pos.y);
                widthSpinBoxes[moduleName]->setValue(pos.width);
                heightSpinBoxes[moduleName]->setValue(pos.height);
            }
        }
    });
    
    mainLayout->addWidget(buttonBox);
    
    if (dialog.exec() == QDialog::Accepted) {
        // 应用新的位置配置
        for (const QString& moduleName : moduleNames) {
            ModulePosition pos;
            pos.moduleName = moduleName;
            pos.x = xSpinBoxes[moduleName]->value();
            pos.y = ySpinBoxes[moduleName]->value();
            pos.width = widthSpinBoxes[moduleName]->value();
            pos.height = heightSpinBoxes[moduleName]->value();
            
            m_modulePositions[moduleName] = pos;
        }
        
        applyModulePositions();
    }
}

void ClockConfigWidget::setModulePosition(const QString& moduleName, int x, int y)
{
    if (m_modulePositions.contains(moduleName)) {
        m_modulePositions[moduleName].x = x;
        m_modulePositions[moduleName].y = y;
        applyModulePositions();
    }
}

ModulePosition ClockConfigWidget::getModulePosition(const QString& moduleName) const
{
    return m_modulePositions.value(moduleName, ModulePosition());
}

void ClockConfigWidget::resetModulePositions()
{
    initializeModulePositions();
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
    
    // 连接输出分频器控制信号
    for (const QString& outputName : OUTPUT_NAMES) {
        if (m_outputDividerBoxes.contains(outputName)) {
            QSpinBox* divBox = m_outputDividerBoxes[outputName];
            connect(divBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this, outputName](int value) {
                onClockDividerChanged(outputName, value);
            });
        }
    }
    
    // 连接clk_1M子节点分频器控制信号
    for (const QString& nodeName : CLK_1M_SUB_NODES) {
        if (m_clk1MSubNodeDividerBoxes.contains(nodeName)) {
            QSpinBox* divBox = m_clk1MSubNodeDividerBoxes[nodeName];
            connect(divBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this, nodeName](int value) {
                onClk1MSubNodeDividerChanged(nodeName, value);
            });
        }
    }
    
    // 连接按钮信号
    connect(m_resetButton, &QPushButton::clicked, this, &ClockConfigWidget::resetToDefaults);
    connect(m_positionConfigButton, &QPushButton::clicked, this, &ClockConfigWidget::showPositionConfigDialog);
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
    if (pllName == "clk_mipimpll") {
        updateAllSubPLLFrequencies();
    }

    // 如果是clk_rvpll，还需要更新其子节点的频率
    if (pllName == "clk_rvpll") {
        updateAllClkRVPLLSubNodeFrequencies();
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
    
    // 如果是clk_a0pll，还需要更新其子节点的频率
    if (pllName == "clk_a0pll") {
        updateAllClkA0PLLSubNodeFrequencies();
    }
    
    emit configChanged();
}

void ClockConfigWidget::onClockDividerChanged(const QString& outputName, int divider)
{
    if (m_outputs.contains(outputName)) {
        m_outputs[outputName].divider = divider;
        updateOutputFrequency(outputName);
        
        // 如果是clk_1M的分频器变化，需要更新所有clk_1M子节点
        if (outputName == "clk_1M") {
            updateAllClk1MSubNodeFrequencies();
        }
        
        emit configChanged();
    }
}

void ClockConfigWidget::onClk1MSubNodeDividerChanged(const QString& nodeName, int divider)
{
    if (m_clk1MSubNodes.contains(nodeName)) {
        m_clk1MSubNodes[nodeName].divider = divider;
        updateClk1MSubNodeFrequency(nodeName);
        emit configChanged();
    }
}

void ClockConfigWidget::onClkCam1PLLSubNodeDividerChanged(const QString& nodeName, int divider)
{
    if (m_clkCam1PLLSubNodes.contains(nodeName)) {
        m_clkCam1PLLSubNodes[nodeName].divider = divider;
        updateClkCam1PLLSubNodeFrequency(nodeName);
        emit configChanged();
    }
}

void ClockConfigWidget::onClkRawAxiSubNodeDividerChanged(const QString& nodeName, int divider)
{
    if (m_clkRawAxiSubNodes.contains(nodeName)) {
        m_clkRawAxiSubNodes[nodeName].divider = divider;
        updateClkRawAxiSubNodeFrequency(nodeName);
        emit configChanged();
    }
}

void ClockConfigWidget::onClkCam0PLLSubNodeDividerChanged(const QString& nodeName, int divider)
{
    if (m_clkCam0PLLSubNodes.contains(nodeName)) {
        m_clkCam0PLLSubNodes[nodeName].divider = divider;
        updateClkCam0PLLSubNodeFrequency(nodeName);
        emit configChanged();
    }
}

void ClockConfigWidget::onClkDispPLLSubNodeDividerChanged(const QString& nodeName, int divider)
{
    if (m_clkDispPLLSubNodes.contains(nodeName)) {
        m_clkDispPLLSubNodes[nodeName].divider = divider;
        updateClkDispPLLSubNodeFrequency(nodeName);
        emit configChanged();
    }
}

void ClockConfigWidget::onClkSysDispSubNodeDividerChanged(const QString& nodeName, int divider)
{
    if (m_clkSysDispSubNodes.contains(nodeName)) {
        m_clkSysDispSubNodes[nodeName].divider = divider;
        updateClkSysDispSubNodeFrequency(nodeName);
        emit configChanged();
    }
}

void ClockConfigWidget::onClkA0PLLSubNodeDividerChanged(const QString& nodeName, int divider)
{
    if (m_clkA0PLLSubNodes.contains(nodeName)) {
        m_clkA0PLLSubNodes[nodeName].divider = divider;
        updateClkA0PLLSubNodeFrequency(nodeName);
        emit configChanged();
    }
}

void ClockConfigWidget::onClkRVPLLSubNodeDividerChanged(const QString& nodeName, int divider)
{
    if (m_clkRVPLLSubNodes.contains(nodeName)) {
        m_clkRVPLLSubNodes[nodeName].divider = divider;
        updateClkRVPLLSubNodeFrequency(nodeName);
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
    if (m_pllConfigs.contains("clk_mipimpll")) {
        config.inputFreq = m_pllConfigs["clk_mipimpll"].outputFreq;
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
    
    // OSC输出频率 = OSC频率 / 分频器
    output.frequency = OSC_FREQUENCY / output.divider;
    
    // 更新显示
    if (m_outputFreqLabels.contains(outputName)) {
        QString freqText;
        if (output.frequency >= 1.0) {
            freqText = QString("%1 MHz").arg(output.frequency, 0, 'f', 3);
        } else {
            freqText = QString("%1 kHz").arg(output.frequency * 1000, 0, 'f', 1);
        }
        m_outputFreqLabels[outputName]->setText(freqText);
    }
}

void ClockConfigWidget::updateClk1MSubNodeFrequency(const QString& nodeName)
{
    if (!m_clk1MSubNodes.contains(nodeName)) return;
    
    ClockOutput& subNode = m_clk1MSubNodes[nodeName];
    
    // 获取clk_1M的频率
    double clk1MFreq = OSC_FREQUENCY / 250;  // clk_1M = 25MHz / 250 = 0.1MHz
    if (m_outputs.contains("clk_1M")) {
        clk1MFreq = m_outputs["clk_1M"].frequency;
    }
    
    // clk_1M子节点频率 = clk_1M频率 / 分频器
    subNode.frequency = clk1MFreq / subNode.divider;
    
    // 更新显示
    if (m_clk1MSubNodeFreqLabels.contains(nodeName)) {
        QString freqText;
        if (subNode.frequency >= 1.0) {
            freqText = QString("%1 MHz").arg(subNode.frequency, 0, 'f', 3);
        } else {
            freqText = QString("%1 kHz").arg(subNode.frequency * 1000, 0, 'f', 1);
        }
        m_clk1MSubNodeFreqLabels[nodeName]->setText(freqText);
    }
}

void ClockConfigWidget::updateAllClk1MSubNodeFrequencies()
{
    for (const QString& nodeName : CLK_1M_SUB_NODES) {
        updateClk1MSubNodeFrequency(nodeName);
    }
}

void ClockConfigWidget::updateClkCam1PLLSubNodeFrequency(const QString& nodeName)
{
    if (!m_clkCam1PLLSubNodes.contains(nodeName)) return;
    
    ClockOutput& subNode = m_clkCam1PLLSubNodes[nodeName];
    
    // 获取clk_cam1pll的频率
    double clkCam1PLLFreq = 800.0; // 默认800MHz，这个值可以根据实际的clk_cam1pll配置来调整
    if (m_pllConfigs.contains("clk_cam1pll")) {
        clkCam1PLLFreq = m_pllConfigs["clk_cam1pll"].outputFreq;
    }
    
    // clk_cam1pll子节点频率 = clk_cam1pll频率 / 分频器
    subNode.frequency = clkCam1PLLFreq / subNode.divider;
    
    // 更新显示
    if (m_clkCam1PLLSubNodeFreqLabels.contains(nodeName)) {
        QString freqText = QString("%1 MHz").arg(subNode.frequency, 0, 'f', 1);
        m_clkCam1PLLSubNodeFreqLabels[nodeName]->setText(freqText);
    }
}

void ClockConfigWidget::updateAllClkCam1PLLSubNodeFrequencies()
{
    for (const QString& nodeName : CLK_CAM1PLL_SUB_NODES) {
        updateClkCam1PLLSubNodeFrequency(nodeName);
    }
}

void ClockConfigWidget::updateClkRawAxiSubNodeFrequency(const QString& nodeName)
{
    if (!m_clkRawAxiSubNodes.contains(nodeName)) return;
    
    ClockOutput& subNode = m_clkRawAxiSubNodes[nodeName];
    
    // 获取clk_raw_axi的频率
    // clk_raw_axi是clk_cam1pll的子节点，我们需要从clk_cam1pll子节点中获取
    double clkRawAxiFreq = 200.0; // 默认200MHz
    if (m_clkCam1PLLSubNodes.contains("clk_raw_axi")) {
        clkRawAxiFreq = m_clkCam1PLLSubNodes["clk_raw_axi"].frequency;
    }
    
    // clk_raw_axi子节点频率 = clk_raw_axi频率 / 分频器
    subNode.frequency = clkRawAxiFreq / subNode.divider;
    
    // 更新显示
    if (m_clkRawAxiSubNodeFreqLabels.contains(nodeName)) {
        QString freqText = QString("%1 MHz").arg(subNode.frequency, 0, 'f', 1);
        m_clkRawAxiSubNodeFreqLabels[nodeName]->setText(freqText);
    }
}

void ClockConfigWidget::updateAllClkRawAxiSubNodeFrequencies()
{
    for (const QString& nodeName : CLK_RAW_AXI_SUB_NODES) {
        updateClkRawAxiSubNodeFrequency(nodeName);
    }
}

void ClockConfigWidget::updateClkCam0PLLSubNodeFrequency(const QString& nodeName)
{
    if (!m_clkCam0PLLSubNodes.contains(nodeName)) return;
    
    ClockOutput& subNode = m_clkCam0PLLSubNodes[nodeName];
    
    // 获取clk_cam0pll的频率
    double clkCam0PLLFreq = 1000.0; // 默认1000MHz，这个值可以根据实际的clk_cam0pll配置来调整
    if (m_pllConfigs.contains("clk_cam0pll")) {
        clkCam0PLLFreq = m_pllConfigs["clk_cam0pll"].outputFreq;
    }
    
    // clk_cam0pll子节点频率 = clk_cam0pll频率 / 分频器
    subNode.frequency = clkCam0PLLFreq / subNode.divider;
    
    // 更新显示
    if (m_clkCam0PLLSubNodeFreqLabels.contains(nodeName)) {
        QString freqText = QString("%1 MHz").arg(subNode.frequency, 0, 'f', 1);
        m_clkCam0PLLSubNodeFreqLabels[nodeName]->setText(freqText);
    }
}

void ClockConfigWidget::updateAllClkCam0PLLSubNodeFrequencies()
{
    for (const QString& nodeName : CLK_CAM0PLL_SUB_NODES) {
        updateClkCam0PLLSubNodeFrequency(nodeName);
    }
}

void ClockConfigWidget::updateClkDispPLLSubNodeFrequency(const QString& nodeName)
{
    if (!m_clkDispPLLSubNodes.contains(nodeName)) return;
    
    ClockOutput& subNode = m_clkDispPLLSubNodes[nodeName];
    
    // 获取clk_disppll的频率
    double clkDispPLLFreq = 1200.0; // 默认1200MHz，这个值可以根据实际的clk_disppll配置来调整
    if (m_pllConfigs.contains("clk_disppll")) {
        clkDispPLLFreq = m_pllConfigs["clk_disppll"].outputFreq;
    }
    
    // clk_disppll子节点频率 = clk_disppll频率 / 分频器
    subNode.frequency = clkDispPLLFreq / subNode.divider;
    
    // 更新显示
    if (m_clkDispPLLSubNodeFreqLabels.contains(nodeName)) {
        QString freqText = QString("%1 MHz").arg(subNode.frequency, 0, 'f', 1);
        m_clkDispPLLSubNodeFreqLabels[nodeName]->setText(freqText);
    }
}

void ClockConfigWidget::updateAllClkDispPLLSubNodeFrequencies()
{
    for (const QString& nodeName : CLK_DISPPLL_SUB_NODES) {
        updateClkDispPLLSubNodeFrequency(nodeName);
    }
}

void ClockConfigWidget::updateClkSysDispSubNodeFrequency(const QString& nodeName)
{
    if (!m_clkSysDispSubNodes.contains(nodeName)) return;
    
    ClockOutput& subNode = m_clkSysDispSubNodes[nodeName];
    
    // 获取clk_sys_disp的频率（来自clk_disppll的子节点）
    double clkSysDispFreq = 150.0; // 默认150MHz (1200/8)
    if (m_clkDispPLLSubNodes.contains("clk_sys_disp")) {
        clkSysDispFreq = m_clkDispPLLSubNodes["clk_sys_disp"].frequency;
    }
    
    // clk_sys_disp子节点频率 = clk_sys_disp频率 / 分频器
    subNode.frequency = clkSysDispFreq / subNode.divider;
    
    // 更新显示
    if (m_clkSysDispSubNodeFreqLabels.contains(nodeName)) {
        QString freqText = QString("%1 MHz").arg(subNode.frequency, 0, 'f', 1);
        m_clkSysDispSubNodeFreqLabels[nodeName]->setText(freqText);
    }
}

void ClockConfigWidget::updateAllClkSysDispSubNodeFrequencies()
{
    for (const QString& nodeName : CLK_SYS_DISP_SUB_NODES) {
        updateClkSysDispSubNodeFrequency(nodeName);
    }
}

void ClockConfigWidget::updateClkA0PLLSubNodeFrequency(const QString& nodeName)
{
    if (!m_clkA0PLLSubNodes.contains(nodeName)) return;
    
    ClockOutput& subNode = m_clkA0PLLSubNodes[nodeName];
    
    // 获取clk_a0pll的频率（来自子PLL配置）
    double clkA0PLLFreq = 500.0; // 默认500MHz
    if (m_pllConfigs.contains("clk_a0pll")) {
        clkA0PLLFreq = m_pllConfigs["clk_a0pll"].outputFreq;
    }
    
    // clk_a0pll子节点频率 = clk_a0pll频率 / 分频器
    subNode.frequency = clkA0PLLFreq / subNode.divider;
    
    // 更新显示
    if (m_clkA0PLLSubNodeFreqLabels.contains(nodeName)) {
        QString freqText = QString("%1 MHz").arg(subNode.frequency, 0, 'f', 1);
        m_clkA0PLLSubNodeFreqLabels[nodeName]->setText(freqText);
    }
}

void ClockConfigWidget::updateAllClkA0PLLSubNodeFrequencies()
{
    for (const QString& nodeName : CLK_A0PLL_SUB_NODES) {
        updateClkA0PLLSubNodeFrequency(nodeName);
    }
}

void ClockConfigWidget::updateClkRVPLLSubNodeFrequency(const QString& nodeName)
{
    if (!m_clkRVPLLSubNodes.contains(nodeName)) return;

    ClockOutput& subNode = m_clkRVPLLSubNodes[nodeName];

    // 获取clk_rvpll的频率
    double clkRVPLLFreq = 1600.0; // 默认1600MHz
    if (m_pllConfigs.contains("clk_rvpll")) {
        clkRVPLLFreq = m_pllConfigs["clk_rvpll"].outputFreq;
    }

    // clk_rvpll子节点频率 = clk_rvpll频率 / 分频器
    subNode.frequency = clkRVPLLFreq / subNode.divider;

    // 更新显示
    if (m_clkRVPLLSubNodeFreqLabels.contains(nodeName)) {
        QString freqText = QString("%1 MHz").arg(subNode.frequency, 0, 'f', 1);
        m_clkRVPLLSubNodeFreqLabels[nodeName]->setText(freqText);
    }
}

void ClockConfigWidget::updateAllClkRVPLLSubNodeFrequencies()
{
    for (const QString& nodeName : CLK_RVPLL_SUB_NODES) {
        updateClkRVPLLSubNodeFrequency(nodeName);
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
    
    // 更新所有clk_1M子节点频率
    for (const QString& nodeName : CLK_1M_SUB_NODES) {
        updateClk1MSubNodeFrequency(nodeName);
    }
    
    // 更新所有clk_cam1pll子节点频率
    for (const QString& nodeName : CLK_CAM1PLL_SUB_NODES) {
        updateClkCam1PLLSubNodeFrequency(nodeName);
    }
    
    // 更新所有clk_raw_axi子节点频率
    for (const QString& nodeName : CLK_RAW_AXI_SUB_NODES) {
        updateClkRawAxiSubNodeFrequency(nodeName);
    }
    
    // 更新所有clk_cam0pll子节点频率
    for (const QString& nodeName : CLK_CAM0PLL_SUB_NODES) {
        updateClkCam0PLLSubNodeFrequency(nodeName);
    }
    
    // 更新所有clk_disppll子节点频率
    for (const QString& nodeName : CLK_DISPPLL_SUB_NODES) {
        updateClkDispPLLSubNodeFrequency(nodeName);
    }
    
    // 更新所有clk_sys_disp子节点频率
    for (const QString& nodeName : CLK_SYS_DISP_SUB_NODES) {
        updateClkSysDispSubNodeFrequency(nodeName);
    }
    
    // 更新所有clk_a0pll子节点频率
    for (const QString& nodeName : CLK_A0PLL_SUB_NODES) {
        updateClkA0PLLSubNodeFrequency(nodeName);
    }

    // 更新所有clk_rvpll子节点频率
    for (const QString& nodeName : CLK_RVPLL_SUB_NODES) {
        updateClkRVPLLSubNodeFrequency(nodeName);
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
        } else if (pllName == "clk_mipimpll") {
            defaultMultiplier = 36;
        } else if (pllName == "MPLL") {
            defaultMultiplier = 48;
        } else if (pllName == "TPLL") {
            defaultMultiplier = 60;
        } else if (pllName == "APPLL") {
            defaultMultiplier = 40;
        } else if (pllName == "clk_rvpll") {
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
        
        if (pllName == "clk_a0pll") {
            defaultMultiplier = 4;
            defaultDivider = 7.32421875;
        } else if (pllName == "clk_cam0pll") {
            defaultMultiplier = 52;
            defaultDivider = 36.0;
        } else if (pllName == "clk_cam1pll") {
            defaultMultiplier = 64;
            defaultDivider = 36.0;
        } else if (pllName == "clk_disppll") {
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
    
    // 重置OSC输出配置
    for (const QString& outputName : OUTPUT_NAMES) {
        m_outputs[outputName].source = "OSC";
        m_outputs[outputName].enabled = true;  // OSC输出默认启用
        
        // 根据输出名称设置不同的默认分频值
        int defaultDivider = 1;
        if (outputName == "clk_usb20_suspend") {
            defaultDivider = 125;
        } else if (outputName == "clk_1M") {
            defaultDivider = 250;
        }
        
        m_outputs[outputName].divider = defaultDivider;
        
        // 更新UI分频器控件
        if (m_outputDividerBoxes.contains(outputName)) {
            m_outputDividerBoxes[outputName]->setValue(defaultDivider);
        }
    }
    
    // 重置clk_1M子节点配置
    for (const QString& nodeName : CLK_1M_SUB_NODES) {
        m_clk1MSubNodes[nodeName].source = "clk_1M";
        m_clk1MSubNodes[nodeName].divider = 1;  // 默认分频为1
        m_clk1MSubNodes[nodeName].enabled = true;
        
        // 更新UI分频器控件
        if (m_clk1MSubNodeDividerBoxes.contains(nodeName)) {
            m_clk1MSubNodeDividerBoxes[nodeName]->setValue(1);
        }
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
    QStringList targetPLLs = {"FPLL", "clk_mipimpll", "MPLL", "TPLL", "APPLL", "clk_rvpll"};
    
    // 为每个目标PLL绘制从OSC的连接线
    for (const QString& pllName : targetPLLs) {
        QPoint pllPoint = getPLLConnectionPoint(pllName);
        if (!pllPoint.isNull() && !oscPoint.isNull()) {
            // 使用不同颜色来区分不同的连接线
            QColor lineColor = Qt::blue;
            if (pllName == "FPLL") lineColor = QColor(220, 53, 69);      // 红色
            else if (pllName == "clk_mipimpll") lineColor = QColor(255, 193, 7);  // 黄色
            else if (pllName == "MPLL") lineColor = QColor(40, 167, 69);      // 绿色
            else if (pllName == "TPLL") lineColor = QColor(23, 162, 184);     // 青色
            else if (pllName == "APPLL") lineColor = QColor(102, 16, 242);    // 紫色
            else if (pllName == "clk_rvpll") lineColor = QColor(255, 102, 0);     // 橙色
            
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
    
    // 绘制OSC到OSC输出的连接线
    if (m_outputWidget) {
        QPoint oscOutputPoint = getOSCConnectionPoint();  // 重用OSC连接点
        QPoint outputAreaPoint = getOutputAreaConnectionPoint();
        
        if (!oscOutputPoint.isNull() && !outputAreaPoint.isNull()) {
            // 使用绿色来表示OSC到输出的连接
            QColor lineColor = QColor(40, 167, 69);  // 绿色
            drawArrowLine(painter, oscOutputPoint, outputAreaPoint, lineColor);
        }
    }
    
    // 绘制clk_1M到其子节点的连接线
    if (m_clk1MSubNodeWidget) {
        QPoint clk1MPoint = getClk1MConnectionPoint();
        
        for (const QString& nodeName : CLK_1M_SUB_NODES) {
            QPoint subNodePoint = getClk1MSubNodeConnectionPoint(nodeName);
            if (!subNodePoint.isNull() && !clk1MPoint.isNull()) {
                // 使用紫色来表示clk_1M到子节点的连接
                QColor lineColor = QColor(102, 16, 242);  // 紫色
                drawArrowLine(painter, clk1MPoint, subNodePoint, lineColor);
            }
        }
    }
    
    // 绘制clk_cam1pll到其子节点的连接线
    if (m_clkCam1PLLSubNodeWidget) {
        QPoint cam1PLLPoint = getClkCam1PLLConnectionPoint();
        
        for (const QString& nodeName : CLK_CAM1PLL_SUB_NODES) {
            QPoint subNodePoint = getClkCam1PLLSubNodeConnectionPoint(nodeName);
            if (!subNodePoint.isNull() && !cam1PLLPoint.isNull()) {
                // 使用橙色来表示clk_cam1pll到子节点的连接
                QColor lineColor = QColor(255, 165, 0);  // 橙色
                drawArrowLine(painter, cam1PLLPoint, subNodePoint, lineColor);
            }
        }
    }
    
    // 绘制clk_raw_axi到其子节点的连接线
    if (m_clkRawAxiSubNodeWidget) {
        QPoint rawAxiPoint = getClkRawAxiConnectionPoint();
        
        for (const QString& nodeName : CLK_RAW_AXI_SUB_NODES) {
            QPoint subNodePoint = getClkRawAxiSubNodeConnectionPoint(nodeName);
            if (!subNodePoint.isNull() && !rawAxiPoint.isNull()) {
                // 使用绿色来表示clk_raw_axi到子节点的连接
                QColor lineColor = QColor(34, 139, 34);  // 森林绿
                drawArrowLine(painter, rawAxiPoint, subNodePoint, lineColor);
            }
        }
    }
    
    // 绘制clk_cam0pll到其子节点的连接线
    if (m_clkCam0PLLSubNodeWidget) {
        QPoint cam0PLLPoint = getClkCam0PLLConnectionPoint();
        
        for (const QString& nodeName : CLK_CAM0PLL_SUB_NODES) {
            QPoint subNodePoint = getClkCam0PLLSubNodeConnectionPoint(nodeName);
            if (!subNodePoint.isNull() && !cam0PLLPoint.isNull()) {
                // 使用深蓝色来表示clk_cam0pll到子节点的连接
                QColor lineColor = QColor(25, 25, 112);  // 午夜蓝
                drawArrowLine(painter, cam0PLLPoint, subNodePoint, lineColor);
            }
        }
    }
    
    // 绘制clk_disppll到其子节点的连接线
    if (m_clkDispPLLSubNodeWidget) {
        QPoint dispPLLPoint = getClkDispPLLConnectionPoint();
        
        for (const QString& nodeName : CLK_DISPPLL_SUB_NODES) {
            QPoint subNodePoint = getClkDispPLLSubNodeConnectionPoint(nodeName);
            if (!subNodePoint.isNull() && !dispPLLPoint.isNull()) {
                // 使用深紫色来表示clk_disppll到子节点的连接
                QColor lineColor = QColor(75, 0, 130);  // 靛蓝色
                drawArrowLine(painter, dispPLLPoint, subNodePoint, lineColor);
            }
        }
    }
    
    // 绘制clk_sys_disp到其子节点的连接线
    if (m_clkSysDispSubNodeWidget) {
        QPoint sysDispPoint = getClkSysDispConnectionPoint();
        
        for (const QString& nodeName : CLK_SYS_DISP_SUB_NODES) {
            QPoint subNodePoint = getClkSysDispSubNodeConnectionPoint(nodeName);
            if (!subNodePoint.isNull() && !sysDispPoint.isNull()) {
                // 使用青绿色来表示clk_sys_disp到子节点的连接
                QColor lineColor = QColor(0, 150, 136);  // 青绿色
                drawArrowLine(painter, sysDispPoint, subNodePoint, lineColor);
            }
        }
    }
    
    // 绘制clk_a0pll到其子节点的连接线
    if (m_clkA0PLLSubNodeWidget) {
        QPoint a0PLLPoint = getClkA0PLLConnectionPoint();
        
        for (const QString& nodeName : CLK_A0PLL_SUB_NODES) {
            QPoint subNodePoint = getClkA0PLLSubNodeConnectionPoint(nodeName);
            if (!subNodePoint.isNull() && !a0PLLPoint.isNull()) {
                // 使用深红色来表示clk_a0pll到子节点的连接
                QColor lineColor = QColor(139, 0, 0);  // 深红色
                drawArrowLine(painter, a0PLLPoint, subNodePoint, lineColor);
            }
        }
    }

    // 绘制clk_rvpll到其子节点的连接线
    if (m_clkRVPLLSubNodeWidget) {
        QPoint rvPLLPoint = getClkRVPLLConnectionPoint();
        
        for (const QString& nodeName : CLK_RVPLL_SUB_NODES) {
            QPoint subNodePoint = getClkRVPLLSubNodeConnectionPoint(nodeName);
            if (!subNodePoint.isNull() && !rvPLLPoint.isNull()) {
                // 使用深橙色来表示clk_rvpll到子节点的连接
                QColor lineColor = QColor(255, 140, 0);  // 深橙色
                drawArrowLine(painter, rvPLLPoint, subNodePoint, lineColor);
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
    
    // 直接使用widget的位置，因为现在使用绝对定位
    QPoint inputPos = m_inputWidget->pos();
    QRect inputRect = m_inputWidget->rect();
    
    // OSC连接点位于输入源widget的右侧中央，考虑RTC和OSC的位置
    QPoint oscPoint = QPoint(
        inputPos.x() + inputRect.width(),
        inputPos.y() + inputRect.height() / 3 * 2 + 30  // OSC位置调整
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
    
    // 获取PLL区域的绝对位置
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
    if (!m_pllWidget || !m_flowWidget || !m_pllWidgets.contains("clk_mipimpll")) {
        return QPoint();
    }
    
    QWidget* mipimpllWidget = m_pllWidgets["clk_mipimpll"];
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

QPoint ClockConfigWidget::getOutputAreaConnectionPoint() const
{
    if (!m_outputWidget || !m_flowWidget) {
        return QPoint();
    }
    
    // 获取输出区域在flow widget中的位置
    QPoint outputAreaPos = m_outputWidget->pos();
    QRect outputRect = m_outputWidget->rect();
    
    // 输出区域连接点位于左侧中央
    QPoint outputPoint = QPoint(
        outputAreaPos.x(),
        outputAreaPos.y() + outputRect.height() / 2
    );
    
    // 转换为相对于ClockConfigWidget的坐标
    QPoint flowPos = m_flowWidget->pos();
    QPoint scrollOffset = QPoint(
        m_flowScrollArea->horizontalScrollBar()->value(),
        m_flowScrollArea->verticalScrollBar()->value()
    );
    
    return QPoint(
        flowPos.x() + outputPoint.x() - scrollOffset.x(),
        flowPos.y() + outputPoint.y() - scrollOffset.y() + 60  // 加上标题高度
    );
}

QPoint ClockConfigWidget::getClk1MConnectionPoint() const
{
    if (!m_outputWidget || !m_flowWidget || !m_outputWidgets.contains("clk_1M")) {
        return QPoint();
    }
    
    QWidget* clk1MWidget = m_outputWidgets["clk_1M"];
    if (!clk1MWidget) {
        return QPoint();
    }
    
    // 获取clk_1M widget在其父widget中的位置
    QPoint clk1MPos = clk1MWidget->pos();
    QRect clk1MRect = clk1MWidget->rect();
    
    // 获取输出区域在flow widget中的位置
    QPoint outputAreaPos = m_outputWidget->pos();
    
    // clk_1M连接点位于widget的右侧中央
    QPoint clk1MPoint = QPoint(
        outputAreaPos.x() + clk1MPos.x() + clk1MRect.width(),
        outputAreaPos.y() + clk1MPos.y() + clk1MRect.height() / 2
    );
    
    // 转换为相对于ClockConfigWidget的坐标
    QPoint flowPos = m_flowWidget->pos();
    QPoint scrollOffset = QPoint(
        m_flowScrollArea->horizontalScrollBar()->value(),
        m_flowScrollArea->verticalScrollBar()->value()
    );
    
    return QPoint(
        flowPos.x() + clk1MPoint.x() - scrollOffset.x(),
        flowPos.y() + clk1MPoint.y() - scrollOffset.y() + 60  // 加上标题高度
    );
}

QPoint ClockConfigWidget::getClk1MSubNodeConnectionPoint(const QString& nodeName) const
{
    if (!m_clk1MSubNodeWidget || !m_flowWidget || !m_clk1MSubNodeWidgets.contains(nodeName)) {
        return QPoint();
    }
    
    QWidget* subNodeWidget = m_clk1MSubNodeWidgets[nodeName];
    if (!subNodeWidget) {
        return QPoint();
    }
    
    // 获取子节点widget在其父widget中的位置
    QPoint subNodePos = subNodeWidget->pos();
    QRect subNodeRect = subNodeWidget->rect();
    
    // 获取clk_1M子节点区域在flow widget中的位置
    QPoint subNodeAreaPos = m_clk1MSubNodeWidget->pos();
    
    // 子节点连接点位于widget的左侧中央
    QPoint subNodePoint = QPoint(
        subNodeAreaPos.x() + subNodePos.x(),
        subNodeAreaPos.y() + subNodePos.y() + subNodeRect.height() / 2
    );
    
    // 转换为相对于ClockConfigWidget的坐标
    QPoint flowPos = m_flowWidget->pos();
    QPoint scrollOffset = QPoint(
        m_flowScrollArea->horizontalScrollBar()->value(),
        m_flowScrollArea->verticalScrollBar()->value()
    );
    
    return QPoint(
        flowPos.x() + subNodePoint.x() - scrollOffset.x(),
        flowPos.y() + subNodePoint.y() - scrollOffset.y() + 60  // 加上标题高度
    );
}

QPoint ClockConfigWidget::getClkCam1PLLConnectionPoint() const
{
    if (!m_subPllWidget || !m_flowWidget || !m_subPllWidgets.contains("clk_cam1pll")) {
        return QPoint();
    }
    
    QWidget* cam1PLLWidget = m_subPllWidgets["clk_cam1pll"];
    if (!cam1PLLWidget) {
        return QPoint();
    }
    
    // 获取clk_cam1pll widget在其父widget中的位置
    QPoint cam1PLLPos = cam1PLLWidget->pos();
    QRect cam1PLLRect = cam1PLLWidget->rect();
    
    // 获取子PLL区域在flow widget中的位置
    QPoint subPllAreaPos = m_subPllWidget->pos();
    
    // clk_cam1pll连接点位于widget的右侧中央
    QPoint cam1PLLPoint = QPoint(
        subPllAreaPos.x() + cam1PLLPos.x() + cam1PLLRect.width(),
        subPllAreaPos.y() + cam1PLLPos.y() + cam1PLLRect.height() / 2
    );
    
    // 转换为相对于ClockConfigWidget的坐标
    QPoint flowPos = m_flowWidget->pos();
    QPoint scrollOffset = QPoint(
        m_flowScrollArea->horizontalScrollBar()->value(),
        m_flowScrollArea->verticalScrollBar()->value()
    );
    
    return QPoint(
        flowPos.x() + cam1PLLPoint.x() - scrollOffset.x(),
        flowPos.y() + cam1PLLPoint.y() - scrollOffset.y() + 60  // 加上标题高度
    );
}

QPoint ClockConfigWidget::getClkCam1PLLSubNodeConnectionPoint(const QString& nodeName) const
{
    if (!m_clkCam1PLLSubNodeWidget || !m_flowWidget || !m_clkCam1PLLSubNodeWidgets.contains(nodeName)) {
        return QPoint();
    }
    
    QWidget* subNodeWidget = m_clkCam1PLLSubNodeWidgets[nodeName];
    if (!subNodeWidget) {
        return QPoint();
    }
    
    // 获取子节点widget在其父widget中的位置
    QPoint subNodePos = subNodeWidget->pos();
    QRect subNodeRect = subNodeWidget->rect();
    
    // 获取clk_cam1pll子节点区域在flow widget中的位置
    QPoint subNodeAreaPos = m_clkCam1PLLSubNodeWidget->pos();
    
    // 子节点连接点位于widget的左侧中央
    QPoint subNodePoint = QPoint(
        subNodeAreaPos.x() + subNodePos.x(),
        subNodeAreaPos.y() + subNodePos.y() + subNodeRect.height() / 2
    );
    
    // 转换为相对于ClockConfigWidget的坐标
    QPoint flowPos = m_flowWidget->pos();
    QPoint scrollOffset = QPoint(
        m_flowScrollArea->horizontalScrollBar()->value(),
        m_flowScrollArea->verticalScrollBar()->value()
    );
    
    return QPoint(
        flowPos.x() + subNodePoint.x() - scrollOffset.x(),
        flowPos.y() + subNodePoint.y() - scrollOffset.y() + 60  // 加上标题高度
    );
}

QPoint ClockConfigWidget::getClkRawAxiConnectionPoint() const
{
    if (!m_clkCam1PLLSubNodeWidget || !m_flowWidget || !m_clkCam1PLLSubNodeWidgets.contains("clk_raw_axi")) {
        return QPoint();
    }
    
    QWidget* rawAxiWidget = m_clkCam1PLLSubNodeWidgets["clk_raw_axi"];
    if (!rawAxiWidget) {
        return QPoint();
    }
    
    // 获取clk_raw_axi widget在其父widget中的位置
    QPoint rawAxiPos = rawAxiWidget->pos();
    QRect rawAxiRect = rawAxiWidget->rect();
    
    // 获取clk_cam1pll子节点区域在flow widget中的位置
    QPoint cam1PLLSubAreaPos = m_clkCam1PLLSubNodeWidget->pos();
    
    // clk_raw_axi连接点位于widget的右侧中央
    QPoint rawAxiPoint = QPoint(
        cam1PLLSubAreaPos.x() + rawAxiPos.x() + rawAxiRect.width(),
        cam1PLLSubAreaPos.y() + rawAxiPos.y() + rawAxiRect.height() / 2
    );
    
    // 转换为相对于ClockConfigWidget的坐标
    QPoint flowPos = m_flowWidget->pos();
    QPoint scrollOffset = QPoint(
        m_flowScrollArea->horizontalScrollBar()->value(),
        m_flowScrollArea->verticalScrollBar()->value()
    );
    
    return QPoint(
        flowPos.x() + rawAxiPoint.x() - scrollOffset.x(),
        flowPos.y() + rawAxiPoint.y() - scrollOffset.y() + 60  // 加上标题高度
    );
}

QPoint ClockConfigWidget::getClkRawAxiSubNodeConnectionPoint(const QString& nodeName) const
{
    if (!m_clkRawAxiSubNodeWidget || !m_flowWidget || !m_clkRawAxiSubNodeWidgets.contains(nodeName)) {
        return QPoint();
    }
    
    QWidget* subNodeWidget = m_clkRawAxiSubNodeWidgets[nodeName];
    if (!subNodeWidget) {
        return QPoint();
    }
    
    // 获取子节点widget在其父widget中的位置
    QPoint subNodePos = subNodeWidget->pos();
    QRect subNodeRect = subNodeWidget->rect();
    
    // 获取clk_raw_axi子节点区域在flow widget中的位置
    QPoint subNodeAreaPos = m_clkRawAxiSubNodeWidget->pos();
    
    // 子节点连接点位于widget的左侧中央
    QPoint subNodePoint = QPoint(
        subNodeAreaPos.x() + subNodePos.x(),
        subNodeAreaPos.y() + subNodePos.y() + subNodeRect.height() / 2
    );
    
    // 转换为相对于ClockConfigWidget的坐标
    QPoint flowPos = m_flowWidget->pos();
    QPoint scrollOffset = QPoint(
        m_flowScrollArea->horizontalScrollBar()->value(),
        m_flowScrollArea->verticalScrollBar()->value()
    );
    
    return QPoint(
        flowPos.x() + subNodePoint.x() - scrollOffset.x(),
        flowPos.y() + subNodePoint.y() - scrollOffset.y() + 60  // 加上标题高度
    );
}

QPoint ClockConfigWidget::getClkCam0PLLConnectionPoint() const
{
    if (!m_subPllWidget || !m_flowWidget || !m_subPllWidgets.contains("clk_cam0pll")) {
        return QPoint();
    }
    
    QWidget* cam0PLLWidget = m_subPllWidgets["clk_cam0pll"];
    if (!cam0PLLWidget) {
        return QPoint();
    }
    
    // 获取clk_cam0pll widget在其父widget中的位置
    QPoint cam0PLLPos = cam0PLLWidget->pos();
    QRect cam0PLLRect = cam0PLLWidget->rect();
    
    // 获取子PLL区域在flow widget中的位置
    QPoint subPllAreaPos = m_subPllWidget->pos();
    
    // clk_cam0pll连接点位于widget的右侧中央
    QPoint cam0PLLPoint = QPoint(
        subPllAreaPos.x() + cam0PLLPos.x() + cam0PLLRect.width(),
        subPllAreaPos.y() + cam0PLLPos.y() + cam0PLLRect.height() / 2
    );
    
    // 转换为相对于ClockConfigWidget的坐标
    QPoint flowPos = m_flowWidget->pos();
    QPoint scrollOffset = QPoint(
        m_flowScrollArea->horizontalScrollBar()->value(),
        m_flowScrollArea->verticalScrollBar()->value()
    );
    
    return QPoint(
        flowPos.x() + cam0PLLPoint.x() - scrollOffset.x(),
        flowPos.y() + cam0PLLPoint.y() - scrollOffset.y() + 60  // 加上标题高度
    );
}

QPoint ClockConfigWidget::getClkCam0PLLSubNodeConnectionPoint(const QString& nodeName) const
{
    if (!m_clkCam0PLLSubNodeWidget || !m_flowWidget || !m_clkCam0PLLSubNodeWidgets.contains(nodeName)) {
        return QPoint();
    }
    
    QWidget* subNodeWidget = m_clkCam0PLLSubNodeWidgets[nodeName];
    if (!subNodeWidget) {
        return QPoint();
    }
    
    // 获取子节点widget在其父widget中的位置
    QPoint subNodePos = subNodeWidget->pos();
    QRect subNodeRect = subNodeWidget->rect();
    
    // 获取clk_cam0pll子节点区域在flow widget中的位置
    QPoint subNodeAreaPos = m_clkCam0PLLSubNodeWidget->pos();
    
    // 子节点连接点位于widget的左侧中央
    QPoint subNodePoint = QPoint(
        subNodeAreaPos.x() + subNodePos.x(),
        subNodeAreaPos.y() + subNodePos.y() + subNodeRect.height() / 2
    );
    
    // 转换为相对于ClockConfigWidget的坐标
    QPoint flowPos = m_flowWidget->pos();
    QPoint scrollOffset = QPoint(
        m_flowScrollArea->horizontalScrollBar()->value(),
        m_flowScrollArea->verticalScrollBar()->value()
    );
    
    return QPoint(
        flowPos.x() + subNodePoint.x() - scrollOffset.x(),
        flowPos.y() + subNodePoint.y() - scrollOffset.y() + 60  // 加上标题高度
    );
}

QPoint ClockConfigWidget::getClkDispPLLConnectionPoint() const
{
    if (!m_subPllWidget || !m_flowWidget || !m_subPllWidgets.contains("clk_disppll")) {
        return QPoint();
    }
    
    QWidget* dispPLLWidget = m_subPllWidgets["clk_disppll"];
    if (!dispPLLWidget) {
        return QPoint();
    }
    
    // 获取clk_disppll widget在其父widget中的位置
    QPoint dispPLLPos = dispPLLWidget->pos();
    QRect dispPLLRect = dispPLLWidget->rect();
    
    // 获取子PLL区域在flow widget中的位置
    QPoint subPllAreaPos = m_subPllWidget->pos();
    
    // clk_disppll连接点位于widget的右侧中央
    QPoint dispPLLPoint = QPoint(
        subPllAreaPos.x() + dispPLLPos.x() + dispPLLRect.width(),
        subPllAreaPos.y() + dispPLLPos.y() + dispPLLRect.height() / 2
    );
    
    // 转换为相对于ClockConfigWidget的坐标
    QPoint flowPos = m_flowWidget->pos();
    QPoint scrollOffset = QPoint(
        m_flowScrollArea->horizontalScrollBar()->value(),
        m_flowScrollArea->verticalScrollBar()->value()
    );
    
    return QPoint(
        flowPos.x() + dispPLLPoint.x() - scrollOffset.x(),
        flowPos.y() + dispPLLPoint.y() - scrollOffset.y() + 60  // 加上标题高度
    );
}

QPoint ClockConfigWidget::getClkDispPLLSubNodeConnectionPoint(const QString& nodeName) const
{
    if (!m_clkDispPLLSubNodeWidget || !m_flowWidget || !m_clkDispPLLSubNodeWidgets.contains(nodeName)) {
        return QPoint();
    }
    
    QWidget* subNodeWidget = m_clkDispPLLSubNodeWidgets[nodeName];
    if (!subNodeWidget) {
        return QPoint();
    }
    
    // 获取子节点widget在其父widget中的位置
    QPoint subNodePos = subNodeWidget->pos();
    QRect subNodeRect = subNodeWidget->rect();
    
    // 获取clk_disppll子节点区域在flow widget中的位置
    QPoint subNodeAreaPos = m_clkDispPLLSubNodeWidget->pos();
    
    // 子节点连接点位于widget的左侧中央
    QPoint subNodePoint = QPoint(
        subNodeAreaPos.x() + subNodePos.x(),
        subNodeAreaPos.y() + subNodePos.y() + subNodeRect.height() / 2
    );
    
    // 转换为相对于ClockConfigWidget的坐标
    QPoint flowPos = m_flowWidget->pos();
    QPoint scrollOffset = QPoint(
        m_flowScrollArea->horizontalScrollBar()->value(),
        m_flowScrollArea->verticalScrollBar()->value()
    );
    
    return QPoint(
        flowPos.x() + subNodePoint.x() - scrollOffset.x(),
        flowPos.y() + subNodePoint.y() - scrollOffset.y() + 60  // 加上标题高度
    );
}

QPoint ClockConfigWidget::getClkSysDispConnectionPoint() const
{
    if (!m_clkDispPLLSubNodeWidget || !m_flowWidget || !m_clkDispPLLSubNodeWidgets.contains("clk_sys_disp")) {
        return QPoint();
    }
    
    QWidget* sysDispWidget = m_clkDispPLLSubNodeWidgets["clk_sys_disp"];
    if (!sysDispWidget) {
        return QPoint();
    }
    
    // 获取clk_sys_disp widget在其父widget中的位置
    QPoint sysDispPos = sysDispWidget->pos();
    QRect sysDispRect = sysDispWidget->rect();
    
    // 获取clk_disppll子节点区域在flow widget中的位置
    QPoint subNodeAreaPos = m_clkDispPLLSubNodeWidget->pos();
    
    // clk_sys_disp连接点位于widget的右侧中央
    QPoint sysDispPoint = QPoint(
        subNodeAreaPos.x() + sysDispPos.x() + sysDispRect.width(),
        subNodeAreaPos.y() + sysDispPos.y() + sysDispRect.height() / 2
    );
    
    // 转换为相对于ClockConfigWidget的坐标
    QPoint flowPos = m_flowWidget->pos();
    QPoint scrollOffset = QPoint(
        m_flowScrollArea->horizontalScrollBar()->value(),
        m_flowScrollArea->verticalScrollBar()->value()
    );
    
    return QPoint(
        flowPos.x() + sysDispPoint.x() - scrollOffset.x(),
        flowPos.y() + sysDispPoint.y() - scrollOffset.y() + 60  // 加上标题高度
    );
}

QPoint ClockConfigWidget::getClkSysDispSubNodeConnectionPoint(const QString& nodeName) const
{
    if (!m_clkSysDispSubNodeWidget || !m_flowWidget || !m_clkSysDispSubNodeWidgets.contains(nodeName)) {
        return QPoint();
    }
    
    QWidget* subNodeWidget = m_clkSysDispSubNodeWidgets[nodeName];
    if (!subNodeWidget) {
        return QPoint();
    }
    
    // 获取子节点widget在其父widget中的位置
    QPoint subNodePos = subNodeWidget->pos();
    QRect subNodeRect = subNodeWidget->rect();
    
    // 获取clk_sys_disp子节点区域在flow widget中的位置
    QPoint subNodeAreaPos = m_clkSysDispSubNodeWidget->pos();
    
    // 子节点连接点位于widget的左侧中央
    QPoint subNodePoint = QPoint(
        subNodeAreaPos.x() + subNodePos.x(),
        subNodeAreaPos.y() + subNodePos.y() + subNodeRect.height() / 2
    );
    
    // 转换为相对于ClockConfigWidget的坐标
    QPoint flowPos = m_flowWidget->pos();
    QPoint scrollOffset = QPoint(
        m_flowScrollArea->horizontalScrollBar()->value(),
        m_flowScrollArea->verticalScrollBar()->value()
    );
    
    return QPoint(
        flowPos.x() + subNodePoint.x() - scrollOffset.x(),
        flowPos.y() + subNodePoint.y() - scrollOffset.y() + 60  // 加上标题高度
    );
}

QPoint ClockConfigWidget::getClkA0PLLConnectionPoint() const
{
    if (!m_subPllWidget || !m_flowWidget || !m_subPllWidgets.contains("clk_a0pll")) {
        return QPoint();
    }
    
    QWidget* a0pllWidget = m_subPllWidgets["clk_a0pll"];
    if (!a0pllWidget) {
        return QPoint();
    }
    
    // 获取clk_a0pll widget在其父widget中的位置
    QPoint a0pllPos = a0pllWidget->pos();
    QRect a0pllRect = a0pllWidget->rect();
    
    // 获取子PLL区域在flow widget中的位置
    QPoint subPllAreaPos = m_subPllWidget->pos();
    
    // clk_a0pll连接点位于widget的右侧中央
    QPoint a0pllPoint = QPoint(
        subPllAreaPos.x() + a0pllPos.x() + a0pllRect.width(),
        subPllAreaPos.y() + a0pllPos.y() + a0pllRect.height() / 2
    );
    
    // 转换为相对于ClockConfigWidget的坐标
    QPoint flowPos = m_flowWidget->pos();
    QPoint scrollOffset = QPoint(
        m_flowScrollArea->horizontalScrollBar()->value(),
        m_flowScrollArea->verticalScrollBar()->value()
    );
    
    return QPoint(
        flowPos.x() + a0pllPoint.x() - scrollOffset.x(),
        flowPos.y() + a0pllPoint.y() - scrollOffset.y() + 60  // 加上标题高度
    );
}

QPoint ClockConfigWidget::getClkA0PLLSubNodeConnectionPoint(const QString& nodeName) const
{
    if (!m_clkA0PLLSubNodeWidget || !m_flowWidget || !m_clkA0PLLSubNodeWidgets.contains(nodeName)) {
        return QPoint();
    }
    
    QWidget* subNodeWidget = m_clkA0PLLSubNodeWidgets[nodeName];
    if (!subNodeWidget) {
        return QPoint();
    }
    
    // 获取子节点widget在其父widget中的位置
    QPoint subNodePos = subNodeWidget->pos();
    QRect subNodeRect = subNodeWidget->rect();
    
    // 获取clk_a0pll子节点区域在flow widget中的位置
    QPoint subNodeAreaPos = m_clkA0PLLSubNodeWidget->pos();
    
    // 子节点连接点位于widget的左侧中央
    QPoint subNodePoint = QPoint(
        subNodeAreaPos.x() + subNodePos.x(),
        subNodeAreaPos.y() + subNodePos.y() + subNodeRect.height() / 2
    );
    
    // 转换为相对于ClockConfigWidget的坐标
    QPoint flowPos = m_flowWidget->pos();
    QPoint scrollOffset = QPoint(
        m_flowScrollArea->horizontalScrollBar()->value(),
        m_flowScrollArea->verticalScrollBar()->value()
    );
    
    return QPoint(
        flowPos.x() + subNodePoint.x() - scrollOffset.x(),
        flowPos.y() + subNodePoint.y() - scrollOffset.y() + 60  // 加上标题高度
    );
}

QPoint ClockConfigWidget::getClkRVPLLConnectionPoint() const
{
    if (!m_pllWidget || !m_flowWidget || !m_pllWidgets.contains("clk_rvpll")) {
        return QPoint();
    }

    QWidget* rvpllWidget = m_pllWidgets["clk_rvpll"];
    if (!rvpllWidget) {
        return QPoint();
    }
    
    // 获取clk_rvpll widget在其父widget中的位置
    QPoint rvpllPos = rvpllWidget->pos();
    QRect rvpllRect = rvpllWidget->rect();
    
    // 获取子PLL区域在flow widget中的位置
    QPoint subPllAreaPos = m_pllWidget->pos();
    
    // clk_rvpll连接点位于widget的右侧中央
    QPoint rvpllPoint = QPoint(
        subPllAreaPos.x() + rvpllPos.x() + rvpllRect.width(),
        subPllAreaPos.y() + rvpllPos.y() + rvpllRect.height() / 2
    );
    
    // 转换为相对于ClockConfigWidget的坐标
    QPoint flowPos = m_flowWidget->pos();
    QPoint scrollOffset = QPoint(
        m_flowScrollArea->horizontalScrollBar()->value(),
        m_flowScrollArea->verticalScrollBar()->value()
    );
    
    return QPoint(
        flowPos.x() + rvpllPoint.x() - scrollOffset.x(),
        flowPos.y() + rvpllPoint.y() - scrollOffset.y() + 60  // 加上标题高度
    );
}

QPoint ClockConfigWidget::getClkRVPLLSubNodeConnectionPoint(const QString& nodeName) const
{
    if (!m_clkRVPLLSubNodeWidget || !m_flowWidget || !m_clkRVPLLSubNodeWidgets.contains(nodeName)) {
        return QPoint();
    }
    
    QWidget* subNodeWidget = m_clkRVPLLSubNodeWidgets[nodeName];
    if (!subNodeWidget) {
        return QPoint();
    }
    
    // 获取子节点widget在其父widget中的位置
    QPoint subNodePos = subNodeWidget->pos();
    QRect subNodeRect = subNodeWidget->rect();
    
    // 获取clk_rvpll子节点区域在flow widget中的位置
    QPoint subNodeAreaPos = m_clkRVPLLSubNodeWidget->pos();
    
    // 子节点连接点位于widget的左侧中央
    QPoint subNodePoint = QPoint(
        subNodeAreaPos.x() + subNodePos.x(),
        subNodeAreaPos.y() + subNodePos.y() + subNodeRect.height() / 2
    );
    
    // 转换为相对于ClockConfigWidget的坐标
    QPoint flowPos = m_flowWidget->pos();
    QPoint scrollOffset = QPoint(
        m_flowScrollArea->horizontalScrollBar()->value(),
        m_flowScrollArea->verticalScrollBar()->value()
    );
    
    return QPoint(
        flowPos.x() + subNodePoint.x() - scrollOffset.x(),
        flowPos.y() + subNodePoint.y() - scrollOffset.y() + 60  // 加上标题高度
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
