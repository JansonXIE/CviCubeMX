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
    "clk_fpll", "clk_mipimpll", "clk_mpll", "clk_tpll", "clk_appll", "clk_rvpll"
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

const QStringList ClockConfigWidget::CLK_APPLL_SUB_NODES = {
    "clk_cpu"
};

const QStringList ClockConfigWidget::CLK_FPLL_SUB_NODES = {
    "clk_xtal_misc", "clk_pwm", "clk_i2c", "clk_eth_pll", "clk_cyc_dsi_esc",
    "clk_scan_100M", "clk_video_axi", "clk_fab_500M", "clk_fab_100M"
};

const QStringList ClockConfigWidget::CLK_TPLL_SUB_NODES = {
    "clk_tpu", "clk_tpu_gdma"
};

const QStringList ClockConfigWidget::CLK_MPLL_SUB_NODES = {
    "clk_uart0", "clk_uart4", "clk_uart3", "clk_uart2", "clk_uart1", "clk_spi",
    "clk_spi_nand", "clk_spi_nor", "clk_usb20_ref", "clk_usb20_bus_early", "clk_rtc_spi_nor",
    "clk_cyc_scan_300M", "clk_vip_sys_4", "clk_vip_sys_3", "clk_vip_sys_1", "clk_vip_sys_0",
    "clk_vc_src0", "clk_tpu_sys", "clk_gic", "clk_bus", "clk_rtc_sys", "clk_hsperi"
};

const QStringList ClockConfigWidget::CLK_FAB_100M_SUB_NODES = {
    "clk_fab6_100M_free", "clk_efuse_pclk", "clk_x2p"
};

const QStringList ClockConfigWidget::CLK_SPI_NAND_SUB_NODES = {
    "clk_spi_nand_gate"
};

const QStringList ClockConfigWidget::CLK_HSPERI_SUB_NODES = {
    "clk_sdma1_axi", "clk_sdma0_axi"
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
    , m_isDragging(false)
    , m_isResizing(false)
    , m_selectedWidget(nullptr)
    , m_lastMousePos()
    , m_dragStartPos()
    , m_originalGeometry()
    , m_resizeDirection(None)
    , m_handleSize(8)
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
    setupClkAPPLLSubNodes();  // 新增：设置clk_appll子节点区域
    setupClkFPLLSubNodes();  // 新增：设置clk_fpll子节点区域
    setupClkTPLLSubNodes();  // 新增：设置clk_tpll子节点区域
    setupClkMPLLSubNodes();  // 新增：设置clk_mpll子节点区域
    setupClkFAB100MSubNodes();  // 新增：设置clk_fab_100M子节点区域
    setupClkSPINANDSubNodes();  // 新增：设置clk_spi_nand子节点区域
    setupClkHSPeriSubNodes();  // 新增：设置clk_hsperi子节点区域
    setupClockTree();
    connectSignals();
    updateFrequencies();
    
    // 初始化模块位置
    initializeModulePositions();
}

ClockConfigWidget::~ClockConfigWidget()
{
}

void ClockConfigWidget::setupUI()
{
    // 启用鼠标追踪以接收鼠标移动事件
    setMouseTracking(true);
    
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
    
    // 创建连接线覆盖层（作为flowWidget的子部件）
    m_connectionOverlay = new QWidget(m_flowWidget);
    m_connectionOverlay->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_connectionOverlay->setAttribute(Qt::WA_NoSystemBackground);
    m_connectionOverlay->setStyleSheet("background: transparent;");
    m_connectionOverlay->installEventFilter(this);
    
    m_mainLayout->addWidget(m_flowScrollArea);
    
    // 延迟重绘以确保布局完成
    QTimer::singleShot(100, this, [this]() {
        updateConnectionOverlay();
    });
    
    // 创建控制按钮区域
    m_buttonLayout = new QHBoxLayout();
    
    m_resetButton = new QPushButton("重置为默认ND");
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
    
    m_applyButton = new QPushButton("OD超频配置");
    m_applyButton->setObjectName("overclockButton");
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
        if (pllName == "clk_fpll") {
            config.multiplier = 40;
        } else if (pllName == "clk_mipimpll") {
            config.multiplier = 36;
        } else if (pllName == "clk_mpll") {
            config.multiplier = 48;
        } else if (pllName == "clk_tpll") {
            config.multiplier = 60;
        } else if (pllName == "clk_appll") {
            config.multiplier = 40;
        } else if (pllName == "clk_rvpll") {
            config.multiplier = 48;
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
        // 从clk_cam1pll的配置中获取正确的频率
        double clkCam1PLLFreq = 800.0; // MHz
        if (m_pllConfigs.contains("clk_cam1pll")) {
            clkCam1PLLFreq = m_pllConfigs["clk_cam1pll"].outputFreq;
        }
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
        // 从配置中获取正确的频率
        double clkRawAxiFreq = 200.0; // MHz
        if (m_pllConfigs.contains("clk_raw_axi")) {
            clkRawAxiFreq = m_pllConfigs["clk_raw_axi"].outputFreq;
        }
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
        // 从配置中获取正确的频率
        double clkCam0PLLFreq = 1000.0; // MHz
        if (m_pllConfigs.contains("clk_cam0pll")) {
            clkCam0PLLFreq = m_pllConfigs["clk_cam0pll"].outputFreq;
        }
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
        // 从配置中获取正确的频率
        double clkDispPLLFreq = 1200.0; // MHz
        if (m_pllConfigs.contains("clk_disppll")) {
            clkDispPLLFreq = m_pllConfigs["clk_disppll"].outputFreq;
        }
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
        // 从配置中获取正确的频率
        double clkSysDispFreq = 150.0; // MHz (1200/8)
        if (m_pllConfigs.contains("clk_disppll")) {
            clkSysDispFreq = m_pllConfigs["clk_disppll"].outputFreq / 8;
        }
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

void ClockConfigWidget::setupClkAPPLLSubNodes()
{
    // 创建clk_appll子节点区域
    m_clkAPPLLSubNodeWidget = new QWidget();
    m_clkAPPLLSubNodeWidget->setFixedSize(150, 120); // 设置固定尺寸，1个子节点
    m_clkAPPLLSubNodeWidget->setStyleSheet(
        "QWidget { "
        "background-color: #f0f8ff; "
        "border: 2px solid #20c997; "
        "border-radius: 8px; "
        "}"
    );

    m_clkAPPLLSubNodeLayout = new QVBoxLayout(m_clkAPPLLSubNodeWidget);
    m_clkAPPLLSubNodeLayout->setContentsMargins(10, 10, 10, 10);
    m_clkAPPLLSubNodeLayout->setSpacing(5);

    // 添加标题
    QLabel* subNodeTitle = new QLabel("clk_appll子节点");
    subNodeTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #117a65; text-align: center;");
    subNodeTitle->setAlignment(Qt::AlignCenter);
    m_clkAPPLLSubNodeLayout->addWidget(subNodeTitle);

    // 为每个clk_appll子节点创建显示界面
    for (const QString& nodeName : CLK_APPLL_SUB_NODES) {
        createClkAPPLLSubNodeWidget(nodeName, m_clkAPPLLSubNodeWidget);

        // 初始化clk_appll子节点配置
        ClockOutput subNode;
        subNode.name = nodeName;
        subNode.source = "clk_appll";
        subNode.enabled = true;
        subNode.divider = 1;  // clk_appll默认分频系数是1

        // 计算频率：clk_appll的频率 / 分频器
        // 从clk_appll的配置中获取正确的频率
        double clkAPPLLFreq = 1000.0; // 默认1000MHz
        if (m_pllConfigs.contains("clk_appll")) {
            clkAPPLLFreq = m_pllConfigs["clk_appll"].outputFreq;
        }
        subNode.frequency = clkAPPLLFreq / subNode.divider;

        m_clkAPPLLSubNodes[nodeName] = subNode;
    }

    m_clkAPPLLSubNodeLayout->addStretch();

    // 设置父widget为m_flowWidget并使用绝对定位
    m_clkAPPLLSubNodeWidget->setParent(m_flowWidget);
    // 默认位置将在initializeModulePositions中设置
}

void ClockConfigWidget::setupClkFPLLSubNodes()
{
    // 创建clk_fpll子节点区域
    m_clkFPLLSubNodeWidget = new QWidget();
    m_clkFPLLSubNodeWidget->setFixedSize(150, 800); // 设置固定宽高尺寸
    m_clkFPLLSubNodeWidget->setStyleSheet(
        "QWidget { "
        "background-color: #f0f8ff; "
        "border: 2px solid #20c997; "
        "border-radius: 8px; "
        "}"
    );

    m_clkFPLLSubNodeLayout = new QVBoxLayout(m_clkFPLLSubNodeWidget);
    m_clkFPLLSubNodeLayout->setContentsMargins(10, 10, 10, 10);
    m_clkFPLLSubNodeLayout->setSpacing(5);

    // 添加标题
    QLabel* subNodeTitle = new QLabel("clk_fpll子节点");
    subNodeTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #117a65; text-align: center;");
    subNodeTitle->setAlignment(Qt::AlignCenter);
    m_clkFPLLSubNodeLayout->addWidget(subNodeTitle);

    // 为每个clk_fpll子节点创建显示界面
    for (const QString& nodeName : CLK_FPLL_SUB_NODES) {
        createClkFPLLSubNodeWidget(nodeName, m_clkFPLLSubNodeWidget);

        // 初始化clk_fpll子节点配置
        ClockOutput subNode;
        subNode.name = nodeName;
        subNode.source = "clk_fpll";
        subNode.enabled = true;

        // 根据子节点设置不同的默认分频值
        if (nodeName == "clk_xtal_misc") {
            subNode.divider = 40;  // clk_xtal_misc默认分频系数是40
        } else if (nodeName == "clk_pwm") {
            subNode.divider = 4;  // clk_pwm默认分频系数是4
        } else if (nodeName == "clk_i2c") {
            subNode.divider = 10;   // clk_i2c默认分频系数是10
        } else if (nodeName == "clk_eth_pll") {
            subNode.divider = 2;  // clk_eth_pll默认分频系数是2
        } else if (nodeName == "clk_cyc_dsi_esc") {
            subNode.divider = 51;   // clk_cyc_dsi_esc默认分频系数是51
        } else if (nodeName == "clk_scan_100M") {
            subNode.divider = 51;  // clk_scan_100M默认分频系数是51
        } else if (nodeName == "clk_video_axi") {
            subNode.divider = 2;   // clk_video_axi默认分频系数是2
        } else if (nodeName == "clk_fab_500M") {
            subNode.divider = 2;  // clk_fab_500M默认分频系数是2
        } else if (nodeName == "clk_fab_100M") {
            subNode.divider = 10;   // clk_fab_100M默认分频系数是10
        }

        // 计算频率：clk_fpll的频率 / 分频器
        // 从clk_fpll的配置中获取正确的频率
        double clkFPLLFreq = 1000.0; // 默认1000MHz
        if (m_pllConfigs.contains("clk_fpll")) {
            clkFPLLFreq = m_pllConfigs["clk_fpll"].outputFreq;
        }
        subNode.frequency = clkFPLLFreq / subNode.divider;

        m_clkFPLLSubNodes[nodeName] = subNode;
    }

    m_clkFPLLSubNodeLayout->addStretch();

    // 将clk_fab_100M的配置同步到m_pllConfigs中，以便其子节点能正确访问
    if (m_clkFPLLSubNodes.contains("clk_fab_100M")) {
        PLLConfig fab100MConfig;
        fab100MConfig.name = "clk_fab_100M";
        fab100MConfig.enabled = true;
        fab100MConfig.inputFreq = 1000.0; // clk_fpll频率
        fab100MConfig.multiplier = 1;
        fab100MConfig.outputFreq = m_clkFPLLSubNodes["clk_fab_100M"].frequency;
        m_pllConfigs["clk_fab_100M"] = fab100MConfig;
    }

    // 设置父widget为m_flowWidget并使用绝对定位
    m_clkFPLLSubNodeWidget->setParent(m_flowWidget);
    // 默认位置将在initializeModulePositions中设置
}

void ClockConfigWidget::setupClkTPLLSubNodes()
{
    // 创建clk_tpll子节点区域
    m_clkTPLLSubNodeWidget = new QWidget();
    m_clkTPLLSubNodeWidget->setFixedSize(150, 200); // 设置固定尺寸，2个子节点
    m_clkTPLLSubNodeWidget->setStyleSheet(
        "QWidget { "
        "background-color: #ffffff; "
        "border: 2px solid #ff6600; "
        "border-radius: 8px; "
        "}"
    );

    m_clkTPLLSubNodeLayout = new QVBoxLayout(m_clkTPLLSubNodeWidget);
    m_clkTPLLSubNodeLayout->setContentsMargins(10, 10, 10, 10);
    m_clkTPLLSubNodeLayout->setSpacing(5);

    // 添加标题
    QLabel* subNodeTitle = new QLabel("clk_tpll子节点");
    subNodeTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #cc5200; text-align: center;");
    subNodeTitle->setAlignment(Qt::AlignCenter);
    m_clkTPLLSubNodeLayout->addWidget(subNodeTitle);

    // 为每个clk_tpll子节点创建显示界面
    for (const QString& nodeName : CLK_TPLL_SUB_NODES) {
        createClkTPLLSubNodeWidget(nodeName, m_clkTPLLSubNodeWidget);

        // 初始化clk_tpll子节点配置
        ClockOutput subNode;
        subNode.name = nodeName;
        subNode.source = "clk_tpll";
        subNode.enabled = true;
        // 根据子节点设置不同的默认分频值
        if (nodeName == "clk_tpu") {
            subNode.divider = 3;  // clk_tpu默认分频系数是3
        } else if (nodeName == "clk_tpu_gdma") {
            subNode.divider = 3;  // clk_tpu_gdma默认分频系数是3
        }

        // 计算频率：clk_tpll的频率 / 分频器
        // 从clk_tpll的配置中获取正确的频率
        double clkTPLLFreq = 1500.0; // 默认1500MHz
        if (m_pllConfigs.contains("clk_tpll")) {
            clkTPLLFreq = m_pllConfigs["clk_tpll"].outputFreq;
        }
        subNode.frequency = clkTPLLFreq / subNode.divider;

        m_clkTPLLSubNodes[nodeName] = subNode;
    }

    m_clkTPLLSubNodeLayout->addStretch();

    // 设置父widget为m_flowWidget并使用绝对定位
    m_clkTPLLSubNodeWidget->setParent(m_flowWidget);
    // 默认位置将在initializeModulePositions中设置
}

void ClockConfigWidget::setupClkMPLLSubNodes()
{
    // 创建clk_mipimpll子节点区域
    m_clkMPLLSubNodeWidget = new QWidget();
    m_clkMPLLSubNodeWidget->setFixedSize(150, 1600); // 设置固定尺寸，25个子节点
    m_clkMPLLSubNodeWidget->setStyleSheet(
        "QWidget { "
        "background-color: #ffffff; "
        "border: 2px solid #ffcc00; "
        "border-radius: 8px; "
        "}"
    );

    m_clkMPLLSubNodeLayout = new QVBoxLayout(m_clkMPLLSubNodeWidget);
    m_clkMPLLSubNodeLayout->setContentsMargins(10, 10, 10, 10);
    m_clkMPLLSubNodeLayout->setSpacing(5);

    // 添加标题
    QLabel* subNodeTitle = new QLabel("clk_mpll子节点");
    subNodeTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #996600; text-align: center;");
    subNodeTitle->setAlignment(Qt::AlignCenter);
    m_clkMPLLSubNodeLayout->addWidget(subNodeTitle);

    // 为每个clk_mpll子节点创建显示界面
    for (const QString& nodeName : CLK_MPLL_SUB_NODES) {
        createClkMPLLSubNodeWidget(nodeName, m_clkMPLLSubNodeWidget);

        // 初始化clk_mpll子节点配置
        ClockOutput subNode;
        subNode.name = nodeName;
        subNode.source = "clk_mpll";
        subNode.enabled = true;
        // 根据子节点设置不同的默认分频值
        if (nodeName == "clk_uart0") {
            subNode.divider = 651;  // clk_uart0默认分频系数是651
        } else if (nodeName == "clk_uart4") {
            subNode.divider = 6;  // clk_uart4默认分频系数是6
        } else if (nodeName == "clk_uart3") {
            subNode.divider = 6;  // clk_uart3默认分频系数是6
        } else if (nodeName == "clk_uart2") {
            subNode.divider = 6;  // clk_uart2默认分频系数是6
        } else if (nodeName == "clk_uart1") {
            subNode.divider = 6;  // clk_uart1默认分频系数是6
        } else if (nodeName == "clk_spi") {
            subNode.divider = 6;  // clk_spi默认分频系数是6
        } else if (nodeName == "clk_spi_nand") {
            subNode.divider = 4;  // clk_spi_nand默认分频系数是4
        } else if (nodeName == "clk_spi_nor") {
            subNode.divider = 4;  // clk_spi_nor默认分频系数是4
        } else if (nodeName == "clk_usb20_ref") {
            subNode.divider = 50;  // clk_usb20_ref默认分频系数是50
        } else if (nodeName == "clk_usb20_bus_early") {
            subNode.divider = 4;  // clk_usb20_bus_early默认分频系数是4
        } else if (nodeName == "clk_rtc_spi_nor") {
            subNode.divider = 4;  // clk_rtc_spi_nor默认分频系数是4
        } else if (nodeName == "clk_cyc_scan_300M") {
            subNode.divider = 4;  // clk_cyc_scan_300M默认分频系数是4
        } else if (nodeName == "clk_vip_sys_4") {
            subNode.divider = 6;  // clk_vip_sys_4默认分频系数是6
        } else if (nodeName == "clk_vip_sys_3") {
            subNode.divider = 2;  // clk_vip_sys_3默认分频系数是2
        } else if (nodeName == "clk_vip_sys_1") {
            subNode.divider = 4;  // clk_vip_sys_1默认分频系数是4
        } else if (nodeName == "clk_vip_sys_0") {
            subNode.divider = 8;  // clk_vip_sys_0默认分频系数是8
        } else if (nodeName == "clk_vc_src0") {
            subNode.divider = 2;  // clk_vc_src0默认分频系数是2
        } else if (nodeName == "clk_tpu_sys") {
            subNode.divider = 4;  // clk_tpu_sys默认分频系数是4
        } else if (nodeName == "clk_gic") {
            subNode.divider = 4;  // clk_gic默认分频系数是4
        } else if (nodeName == "clk_bus") {
            subNode.divider = 2;  // clk_bus默认分频系数是2
        } else if (nodeName == "clk_rtc_sys") {
            subNode.divider = 4;  // clk_rtc_sys默认分频系数是4
        } else if (nodeName == "clk_hsperi") {
            subNode.divider = 4;  // clk_hsperi默认分频系数是4
        }

        // 计算频率：clk_mpll的频率 / 分频器
        // 从clk_mpll的配置中获取正确的频率
        double clkMPLLFreq = 1200.0; // 默认1200MHz
        if (m_pllConfigs.contains("clk_mpll")) {
            clkMPLLFreq = m_pllConfigs["clk_mpll"].outputFreq;
        }
        subNode.frequency = clkMPLLFreq / subNode.divider;

        m_clkMPLLSubNodes[nodeName] = subNode;
    }

    m_clkMPLLSubNodeLayout->addStretch();

    // 将clk_spi_nand的配置同步到m_pllConfigs中，以便其子节点能正确访问
    if (m_clkMPLLSubNodes.contains("clk_spi_nand")) {
        PLLConfig spiNandConfig;
        spiNandConfig.name = "clk_spi_nand";
        spiNandConfig.enabled = true;
        spiNandConfig.inputFreq = 1200.0; // clk_mpll频率
        spiNandConfig.multiplier = 1;
        spiNandConfig.outputFreq = m_clkMPLLSubNodes["clk_spi_nand"].frequency;
        m_pllConfigs["clk_spi_nand"] = spiNandConfig;
    }

    // 将clk_hsperi的配置同步到m_pllConfigs中，以便其子节点能正确访问
    if (m_clkMPLLSubNodes.contains("clk_hsperi")) {
        PLLConfig hsperiConfig;
        hsperiConfig.name = "clk_hsperi";
        hsperiConfig.enabled = true;
        hsperiConfig.inputFreq = 1200.0; // clk_mpll频率
        hsperiConfig.multiplier = 1;
        hsperiConfig.outputFreq = m_clkMPLLSubNodes["clk_hsperi"].frequency;
        m_pllConfigs["clk_hsperi"] = hsperiConfig;
    }

    // 设置父widget为m_flowWidget并使用绝对定位
    m_clkMPLLSubNodeWidget->setParent(m_flowWidget);
    // 默认位置将在initializeModulePositions中设置
}

void ClockConfigWidget::setupClkFAB100MSubNodes()
{
    // 创建clk_fab_100M子节点区域
    m_clkFAB100MSubNodeWidget = new QWidget();
    m_clkFAB100MSubNodeWidget->setFixedSize(150, 300); // 设置固定尺寸，3个子节点
    m_clkFAB100MSubNodeWidget->setStyleSheet(
        "QWidget { "
        "background-color: #ffffff; "
        "border: 2px solid #6f42c1; "
        "border-radius: 8px; "
        "}"
    );

    m_clkFAB100MSubNodeLayout = new QVBoxLayout(m_clkFAB100MSubNodeWidget);
    m_clkFAB100MSubNodeLayout->setContentsMargins(10, 10, 10, 10);
    m_clkFAB100MSubNodeLayout->setSpacing(5);

    // 添加标题
    QLabel* subNodeTitle = new QLabel("clk_fab_100M子节点");
    subNodeTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #4b0082; text-align: center;");
    subNodeTitle->setAlignment(Qt::AlignCenter);
    m_clkFAB100MSubNodeLayout->addWidget(subNodeTitle);

    // 为每个clk_fab_100M子节点创建显示界面
    for (const QString& nodeName : CLK_FAB_100M_SUB_NODES) {
        createClkFAB100MSubNodeWidget(nodeName, m_clkFAB100MSubNodeWidget);

        // 初始化clk_fab_100M子节点配置
        ClockOutput subNode;
        subNode.name = nodeName;
        subNode.source = "clk_fab_100M";
        subNode.enabled = true;
        // 根据子节点设置不同的默认分频值
        if (nodeName == "clk_fab6_100M_free") {
            subNode.divider = 1;  // clk_fab6_100M_free默认分频系数是1
        } else if (nodeName == "clk_efuse_pclk") {
            subNode.divider = 1;  // clk_efuse_pclk默认分频系数是1
        } else if (nodeName == "clk_x2p") {
            subNode.divider = 1;  // clk_x2p默认分频系数是1
        }

        // 计算频率：clk_fab_100M的频率 / 分频器
        // 从clk_fab_100M的配置中获取正确的频率
        double clkFAB100MFreq = 100.0; // 默认100MHz
        if (m_pllConfigs.contains("clk_fab_100M")) {
            clkFAB100MFreq = m_pllConfigs["clk_fab_100M"].outputFreq;
        }
        subNode.frequency = clkFAB100MFreq / subNode.divider;

        m_clkFAB100MSubNodes[nodeName] = subNode;
    }

    m_clkFAB100MSubNodeLayout->addStretch();

    // 设置父widget为m_flowWidget并使用绝对定位
    m_clkFAB100MSubNodeWidget->setParent(m_flowWidget);
    // 默认位置将在initializeModulePositions中设置
}

void ClockConfigWidget::setupClkSPINANDSubNodes()
{
    // 创建clk_spi_nand子节点区域
    m_clkSPINANDSubNodeWidget = new QWidget();
    m_clkSPINANDSubNodeWidget->setFixedSize(150, 120); // 设置固定尺寸，1个子节点
    m_clkSPINANDSubNodeWidget->setStyleSheet(
        "QWidget { "
        "background-color: #ffffff; "
        "border: 2px solid #343a40; "
        "border-radius: 8px; "
        "}"
    );

    m_clkSPINANDSubNodeLayout = new QVBoxLayout(m_clkSPINANDSubNodeWidget);
    m_clkSPINANDSubNodeLayout->setContentsMargins(10, 10, 10, 10);
    m_clkSPINANDSubNodeLayout->setSpacing(5);

    // 添加标题
    QLabel* subNodeTitle = new QLabel("clk_spi_nand子节点");
    subNodeTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #212529; text-align: center;");
    subNodeTitle->setAlignment(Qt::AlignCenter);
    m_clkSPINANDSubNodeLayout->addWidget(subNodeTitle);

    // 为每个clk_spi_nand子节点创建显示界面
    for (const QString& nodeName : CLK_SPI_NAND_SUB_NODES) {
        createClkSPINANDSubNodeWidget(nodeName, m_clkSPINANDSubNodeWidget);

        // 初始化clk_spi_nand子节点配置
        ClockOutput subNode;
        subNode.name = nodeName;
        subNode.source = "clk_spi_nand";
        subNode.enabled = true;
        subNode.divider = 1;  // clk_spi_nand默认分频系数是1

        // 计算频率：clk_spi_nand的频率 / 分频器
        // 从clk_spi_nand的配置中获取正确的频率
        double clkSPINANDFreq = 300.0; // 默认300MHz
        if (m_pllConfigs.contains("clk_spi_nand")) {
            clkSPINANDFreq = m_pllConfigs["clk_spi_nand"].outputFreq;
        }
        subNode.frequency = clkSPINANDFreq / subNode.divider;

        m_clkSPINANDSubNodes[nodeName] = subNode;
    }

    m_clkSPINANDSubNodeLayout->addStretch();

    // 设置父widget为m_flowWidget并使用绝对定位
    m_clkSPINANDSubNodeWidget->setParent(m_flowWidget);
    // 默认位置将在initializeModulePositions中设置
}

void ClockConfigWidget::setupClkHSPeriSubNodes()
{
    // 创建clk_hsperi子节点区域
    m_clkHSPeriSubNodeWidget = new QWidget();
    m_clkHSPeriSubNodeWidget->setFixedSize(150, 200); // 设置固定尺寸，2个子节点
    m_clkHSPeriSubNodeWidget->setStyleSheet(
        "QWidget { "
        "background-color: #ffffff; "
        "border: 2px solid #17a2b8; "
        "border-radius: 8px; "
        "}"
    );

    m_clkHSPeriSubNodeLayout = new QVBoxLayout(m_clkHSPeriSubNodeWidget);
    m_clkHSPeriSubNodeLayout->setContentsMargins(10, 10, 10, 10);
    m_clkHSPeriSubNodeLayout->setSpacing(5);

    // 添加标题
    QLabel* subNodeTitle = new QLabel("clk_hsperi子节点");
    subNodeTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #117a8b; text-align: center;");
    subNodeTitle->setAlignment(Qt::AlignCenter);
    m_clkHSPeriSubNodeLayout->addWidget(subNodeTitle);

    // 为每个clk_hsperi子节点创建显示界面
    for (const QString& nodeName : CLK_HSPERI_SUB_NODES) {
        createClkHSPeriSubNodeWidget(nodeName, m_clkHSPeriSubNodeWidget);

        // 初始化clk_hsperi子节点配置
        ClockOutput subNode;
        subNode.name = nodeName;
        subNode.source = "clk_hsperi";
        subNode.enabled = true;
        // 根据子节点设置不同的默认分频值
        if (nodeName == "clk_sdma1_axi") {
            subNode.divider = 1;  // clk_sdma1_axi默认分频系数是1
        } else if (nodeName == "clk_sdma0_axi") {
            subNode.divider = 1;  // clk_sdma0_axi默认分频系数是1
        }

        // 计算频率：clk_hsperi的频率 / 分频器
        // 从clk_hsperi的配置中获取正确的频率
        double clkHSPeriFreq = 300.0; // 默认300MHz
        if (m_pllConfigs.contains("clk_hsperi")) {
            clkHSPeriFreq = m_pllConfigs["clk_hsperi"].outputFreq;
        }
        subNode.frequency = clkHSPeriFreq / subNode.divider;

        m_clkHSPeriSubNodes[nodeName] = subNode;
    }

    m_clkHSPeriSubNodeLayout->addStretch();

    // 设置父widget为m_flowWidget并使用绝对定位
    m_clkHSPeriSubNodeWidget->setParent(m_flowWidget);
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
    if (pllName == "clk_fpll") {
        defaultMultiplier = 40;
    } else if (pllName == "clk_mipimpll") {
        defaultMultiplier = 36;
    } else if (pllName == "clk_mpll") {
        defaultMultiplier = 48;
    } else if (pllName == "clk_tpll") {
        defaultMultiplier = 60;
    } else if (pllName == "clk_appll") {
        defaultMultiplier = 40;
    } else if (pllName == "clk_rvpll") {
        defaultMultiplier = 48;
    }
    
    multBox->setValue(defaultMultiplier);
    multBox->setFixedWidth(60);
    multBox->setStyleSheet("font-size: 10px;");
    
    configLayout->addWidget(multLabel);
    configLayout->addWidget(multBox);
    
    // 频率显示
    QString freqText;
    if (pllName == "clk_fpll") {
        freqText = "1000.000 MHz";  // 25 * 40
    } else if (pllName == "clk_mipimpll") {
        freqText = "900.000 MHz";   // 25 * 36
    } else if (pllName == "clk_mpll") {
        freqText = "1200.000 MHz";  // 25 * 48
    } else if (pllName == "clk_tpll") {
        freqText = "1500.000 MHz";  // 25 * 60
    } else if (pllName == "clk_appll") {
        freqText = "1000.000 MHz";  // 25 * 40
    } else if (pllName == "clk_rvpll") {
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

void ClockConfigWidget::createClkAPPLLSubNodeWidget(const QString& nodeName, QWidget* parent)
{
    QWidget* subNodeWidget = new QWidget();
    subNodeWidget->setStyleSheet(
        "QWidget { "
        "background-color: #ffffff; "
        "border: 1px solid #20c997; "
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
    nameLabel->setStyleSheet("font-weight: bold; color: #117a65; font-size: 11px;");
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setWordWrap(true);
    
    // 分频器配置
    QHBoxLayout* divConfigLayout = new QHBoxLayout();
    divConfigLayout->setSpacing(3);
    
    QLabel* divLabel = new QLabel("分频：");
    divLabel->setStyleSheet("color: #117a65; font-size: 10px; font-weight: bold;");
    
    QSpinBox* divBox = new QSpinBox();
    divBox->setRange(1, 1000);
    divBox->setValue(1);  // 默认分频系数是1
    divBox->setFixedWidth(45);
    divBox->setStyleSheet("font-size: 10px;");
    
    divConfigLayout->addWidget(divLabel);
    divConfigLayout->addWidget(divBox);
    
    // 频率显示
    double clkAPPLLFreq = 1000.0;  // 假设clk_appll = 1000MHz
    double calculatedFreq = clkAPPLLFreq / 1;  // 默认分频为1
    QString freqText = QString("%1 MHz").arg(calculatedFreq, 0, 'f', 1);
    
    QLabel* freqLabel = new QLabel(freqText);
    freqLabel->setStyleSheet("color: #dc3545; font-family: monospace; font-weight: bold; font-size: 9px;");
    freqLabel->setAlignment(Qt::AlignCenter);
    
    layout->addWidget(nameLabel);
    layout->addLayout(divConfigLayout);
    layout->addWidget(freqLabel);

    // 存储控件引用
    m_clkAPPLLSubNodeWidgets[nodeName] = subNodeWidget;
    m_clkAPPLLSubNodeFreqLabels[nodeName] = freqLabel;
    m_clkAPPLLSubNodeDividerBoxes[nodeName] = divBox;

    m_clkAPPLLSubNodeLayout->addWidget(subNodeWidget);

    // 连接分频器变化信号
    connect(divBox, QOverload<int>::of(&QSpinBox::valueChanged),
            [this, nodeName](int value) {
                onClkAPPLLSubNodeDividerChanged(nodeName, value);
            });
}

void ClockConfigWidget::createClkFPLLSubNodeWidget(const QString& nodeName, QWidget* parent)
{
    QWidget* subNodeWidget = new QWidget();
    subNodeWidget->setStyleSheet(
        "QWidget { "
        "background-color: #ffffff; "
        "border: 1px solid #28a745; "
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
    nameLabel->setStyleSheet("font-weight: bold; color: #155724; font-size: 11px;");
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setWordWrap(true);
    
    // 分频器配置
    QHBoxLayout* divConfigLayout = new QHBoxLayout();
    divConfigLayout->setSpacing(3);
    
    QLabel* divLabel = new QLabel("分频：");
    divLabel->setStyleSheet("color: #155724; font-size: 10px; font-weight: bold;");
    
    QSpinBox* divBox = new QSpinBox();
    divBox->setRange(1, 1000);
    // 根据子节点设置不同的默认分频值
    if (nodeName == "clk_xtal_misc") {
        divBox->setValue(40);  // clk_xtal_misc默认分频系数是40
    } else if (nodeName == "clk_pwm") {
        divBox->setValue(4);  // clk_pwm默认分频系数是4
    } else if (nodeName == "clk_i2c") {
        divBox->setValue(10);   // clk_i2c默认分频系数是10
    } else if (nodeName == "clk_eth_pll") {
        divBox->setValue(2);  // clk_eth_pll默认分频系数是2
    } else if (nodeName == "clk_cyc_dsi_esc") {
        divBox->setValue(51);   // clk_cyc_dsi_esc默认分频系数是51
    } else if (nodeName == "clk_scan_100M") {
        divBox->setValue(51);  // clk_scan_100M默认分频系数是51
    } else if (nodeName == "clk_video_axi") {
        divBox->setValue(2);   // clk_video_axi默认分频系数是2
    } else if (nodeName == "clk_fab_500M") {
        divBox->setValue(2);  // clk_fab_500M默认分频系数是2
    } else if (nodeName == "clk_fab_100M") {
        divBox->setValue(10);   // clk_fab_100M默认分频系数是10
    }
    divBox->setFixedWidth(45);
    divBox->setStyleSheet("font-size: 10px;");
    
    divConfigLayout->addWidget(divLabel);
    divConfigLayout->addWidget(divBox);
    
    // 频率显示
    double clkFPLLFreq = 1000.0;  // 假设clk_fpll = 1000MHz
    double calculatedFreq;
    if (nodeName == "clk_xtal_misc") {
        calculatedFreq = clkFPLLFreq / 40;  // clk_xtal_misc默认分频系数是40
    } else if (nodeName == "clk_pwm") {
       calculatedFreq = clkFPLLFreq / 4;  // clk_pwm默认分频系数是4
    } else if (nodeName == "clk_i2c") {
        calculatedFreq = clkFPLLFreq / 10;   // clk_i2c默认分频系数是10
    } else if (nodeName == "clk_eth_pll") {
        calculatedFreq = clkFPLLFreq / 2;  // clk_eth_pll默认分频系数是2
    } else if (nodeName == "clk_cyc_dsi_esc") {
        calculatedFreq = clkFPLLFreq / 51;   // clk_cyc_dsi_esc默认分频系数是51
    } else if (nodeName == "clk_scan_100M") {
        calculatedFreq = clkFPLLFreq / 51;  // clk_scan_100M默认分频系数是51
    } else if (nodeName == "clk_video_axi") {
        calculatedFreq = clkFPLLFreq / 2;   // clk_video_axi默认分频系数是2
    } else if (nodeName == "clk_fab_500M") {
        calculatedFreq = clkFPLLFreq / 2;  // clk_fab_500M默认分频系数是2
    } else if (nodeName == "clk_fab_100M") {
        calculatedFreq = clkFPLLFreq / 10;   // clk_fab_100M默认分频系数是10
    }
    QString freqText = QString("%1 MHz").arg(calculatedFreq, 0, 'f', 1);
    
    QLabel* freqLabel = new QLabel(freqText);
    freqLabel->setStyleSheet("color: #dc3545; font-family: monospace; font-weight: bold; font-size: 9px;");
    freqLabel->setAlignment(Qt::AlignCenter);
    
    layout->addWidget(nameLabel);
    layout->addLayout(divConfigLayout);
    layout->addWidget(freqLabel);

    // 存储控件引用
    m_clkFPLLSubNodeWidgets[nodeName] = subNodeWidget;
    m_clkFPLLSubNodeFreqLabels[nodeName] = freqLabel;
    m_clkFPLLSubNodeDividerBoxes[nodeName] = divBox;

    m_clkFPLLSubNodeLayout->addWidget(subNodeWidget);

    // 连接分频器变化信号
    connect(divBox, QOverload<int>::of(&QSpinBox::valueChanged),
            [this, nodeName](int value) {
                onClkFPLLSubNodeDividerChanged(nodeName, value);
            });
}

void ClockConfigWidget::createClkTPLLSubNodeWidget(const QString& nodeName, QWidget* parent)
{
    QWidget* subNodeWidget = new QWidget();
    subNodeWidget->setStyleSheet(
        "QWidget { "
        "background-color: #ffffff; "
        "border: 1px solid #ff6600; "
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
    nameLabel->setStyleSheet("font-weight: bold; color: #cc5200; font-size: 11px;");
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setWordWrap(true);
    
    // 分频器配置
    QHBoxLayout* divConfigLayout = new QHBoxLayout();
    divConfigLayout->setSpacing(3);
    
    QLabel* divLabel = new QLabel("分频：");
    divLabel->setStyleSheet("color: #cc5200; font-size: 10px; font-weight: bold;");
    
    QSpinBox* divBox = new QSpinBox();
    divBox->setRange(1, 1000);
    // 根据子节点设置不同的默认分频值
    if (nodeName == "clk_tpu") {
        divBox->setValue(3);  // clk_tpu默认分频系数是3
    } else if (nodeName == "clk_tpu_gdma") {
        divBox->setValue(3);  // clk_tpu_gdma默认分频系数是3
    }
    divBox->setFixedWidth(45);
    divBox->setStyleSheet("font-size: 10px;");
    
    divConfigLayout->addWidget(divLabel);
    divConfigLayout->addWidget(divBox);
    
    // 频率显示
    double clkTPLLFreq = 1500.0;  // 假设clk_tpll = 1500MHz
    double calculatedFreq;
    if (nodeName == "clk_tpu") {
        calculatedFreq = clkTPLLFreq / 3;  // clk_tpu默认分频系数是3
    } else if (nodeName == "clk_tpu_gdma") {
       calculatedFreq = clkTPLLFreq / 3;  // clk_tpu_gdma默认分频系数是3
    }
    QString freqText = QString("%1 MHz").arg(calculatedFreq, 0, 'f', 1);
    
    QLabel* freqLabel = new QLabel(freqText);
    freqLabel->setStyleSheet("color: #dc3545; font-family: monospace; font-weight: bold; font-size: 9px;");
    freqLabel->setAlignment(Qt::AlignCenter);
    
    layout->addWidget(nameLabel);
    layout->addLayout(divConfigLayout);
    layout->addWidget(freqLabel);

    // 存储控件引用
    m_clkTPLLSubNodeWidgets[nodeName] = subNodeWidget;
    m_clkTPLLSubNodeFreqLabels[nodeName] = freqLabel;
    m_clkTPLLSubNodeDividerBoxes[nodeName] = divBox;

    m_clkTPLLSubNodeLayout->addWidget(subNodeWidget);

    // 连接分频器变化信号
    connect(divBox, QOverload<int>::of(&QSpinBox::valueChanged),
            [this, nodeName](int value) {
                onClkTPLLSubNodeDividerChanged(nodeName, value);
            });
}

void ClockConfigWidget::createClkMPLLSubNodeWidget(const QString& nodeName, QWidget* parent)
{
    QWidget* subNodeWidget = new QWidget();
    subNodeWidget->setStyleSheet(
        "QWidget { "
        "background-color: #ffffff; "
        "border: 1px solid #ffcc00; "
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
    nameLabel->setStyleSheet("font-weight: bold; color: #996600; font-size: 11px;");
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setWordWrap(true);
    
    // 分频器配置
    QHBoxLayout* divConfigLayout = new QHBoxLayout();
    divConfigLayout->setSpacing(3);
    
    QLabel* divLabel = new QLabel("分频：");
    divLabel->setStyleSheet("color: #996600; font-size: 10px; font-weight: bold;");
    
    QSpinBox* divBox = new QSpinBox();
    divBox->setRange(1, 1000);
    // 根据子节点设置不同的默认分频值
    if (nodeName == "clk_uart0") {
        divBox->setValue(651);  // clk_uart0默认分频系数是651
    } else if (nodeName == "clk_uart4") {
        divBox->setValue(6);  // clk_uart4默认分频系数是6
    } else if (nodeName == "clk_uart3") {
        divBox->setValue(6);  // clk_uart3默认分频系数是6
    } else if (nodeName == "clk_uart2") {
        divBox->setValue(6);  // clk_uart2默认分频系数是6
    } else if (nodeName == "clk_uart1") {
        divBox->setValue(6);  // clk_uart1默认分频系数是6
    } else if (nodeName == "clk_spi") {
        divBox->setValue(6);  // clk_spi默认分频系数是6
    } else if (nodeName == "clk_spi_nand") {
        divBox->setValue(4);  // clk_spi_nand默认分频系数是4
    } else if (nodeName == "clk_spi_nor") {
        divBox->setValue(4);  // clk_spi_nor默认分频系数是4
    } else if (nodeName == "clk_usb20_ref") {
        divBox->setValue(50);  // clk_usb20_ref默认分频系数是50
    } else if (nodeName == "clk_usb20_bus_early") {
        divBox->setValue(4);  // clk_usb20_bus_early默认分频系数是4
    } else if (nodeName == "clk_rtc_spi_nor") {
        divBox->setValue(4);  // clk_rtc_spi_nor默认分频系数是4
    } else if (nodeName == "clk_cyc_scan_300M") {
        divBox->setValue(4);  // clk_cyc_scan_300M默认分频系数是4
    } else if (nodeName == "clk_vip_sys_4") {
        divBox->setValue(6);  // clk_vip_sys_4默认分频系数是6
    } else if (nodeName == "clk_vip_sys_3") {
        divBox->setValue(2);  // clk_vip_sys_3默认分频系数是2
    } else if (nodeName == "clk_vip_sys_1") {
        divBox->setValue(4);  // clk_vip_sys_1默认分频系数是4
    } else if (nodeName == "clk_vip_sys_0") {
        divBox->setValue(8);  // clk_vip_sys_0默认分频系数是8
    } else if (nodeName == "clk_vc_src0") {
        divBox->setValue(2);  // clk_vc_src0默认分频系数是2
    } else if (nodeName == "clk_tpu_sys") {
        divBox->setValue(4);  // clk_tpu_sys默认分频系数是4
    } else if (nodeName == "clk_gic") {
        divBox->setValue(4);  // clk_gic默认分频系数是4
    } else if (nodeName == "clk_bus") {
        divBox->setValue(2);  // clk_bus默认分频系数是2
    } else if (nodeName == "clk_rtc_sys") {
        divBox->setValue(4);  // clk_rtc_sys默认分频系数是4
    } else if (nodeName == "clk_hsperi") {
        divBox->setValue(4);  // clk_hsperi默认分频系数是4
    }
    divBox->setFixedWidth(45);
    divBox->setStyleSheet("font-size: 10px;");
    
    divConfigLayout->addWidget(divLabel);
    divConfigLayout->addWidget(divBox);
    
    // 频率显示
    double clkMPLLFreq = 1200.0;  // 假设clk_mpll = 1200MHz
    double calculatedFreq;
    if (nodeName == "clk_uart0") {
        calculatedFreq = clkMPLLFreq / 651;  // clk_uart0默认分频系数是651
    } else if (nodeName == "clk_uart4") {
        calculatedFreq = clkMPLLFreq / 6;  // clk_uart4默认分频系数是6
    } else if (nodeName == "clk_uart3") {
        calculatedFreq = clkMPLLFreq / 6;  // clk_uart3默认分频系数是6
    } else if (nodeName == "clk_uart2") {
        calculatedFreq = clkMPLLFreq / 6;  // clk_uart2默认分频系数是6
    } else if (nodeName == "clk_uart1") {
        calculatedFreq = clkMPLLFreq / 6;  // clk_uart1默认分频系数是6
    } else if (nodeName == "clk_spi") {
        calculatedFreq = clkMPLLFreq / 6;  // clk_spi默认分频系数是6
    } else if (nodeName == "clk_spi_nand") {
        calculatedFreq = clkMPLLFreq / 4;  // clk_spi_nand默认分频系数是4
    } else if (nodeName == "clk_spi_nor") {
        calculatedFreq = clkMPLLFreq / 4;  // clk_spi_nor默认分频系数是4
    } else if (nodeName == "clk_usb20_ref") {
        calculatedFreq = clkMPLLFreq / 50;  // clk_usb20_ref默认分频系数是50
    } else if (nodeName == "clk_usb20_bus_early") {
        calculatedFreq = clkMPLLFreq / 4;  // clk_usb20_bus_early默认分频系数是4
    } else if (nodeName == "clk_rtc_spi_nor") {
        calculatedFreq = clkMPLLFreq / 4;  // clk_rtc_spi_nor默认分频系数是4
    } else if (nodeName == "clk_cyc_scan_300M") {
        calculatedFreq = clkMPLLFreq / 4;  // clk_cyc_scan_300M默认分频系数是4
    } else if (nodeName == "clk_vip_sys_4") {
        calculatedFreq = clkMPLLFreq / 6;  // clk_vip_sys_4默认分频系数是6
    } else if (nodeName == "clk_vip_sys_3") {
        calculatedFreq = clkMPLLFreq / 2;  // clk_vip_sys_3默认分频系数是2
    } else if (nodeName == "clk_vip_sys_1") {
        calculatedFreq = clkMPLLFreq / 4;  // clk_vip_sys_1默认分频系数是4
    } else if (nodeName == "clk_vip_sys_0") {
        calculatedFreq = clkMPLLFreq / 8;  // clk_vip_sys_0默认分频系数是8
    } else if (nodeName == "clk_vc_src0") {
        calculatedFreq = clkMPLLFreq / 2;  // clk_vc_src0默认分频系数是2
    } else if (nodeName == "clk_tpu_sys") {
        calculatedFreq = clkMPLLFreq / 4;  // clk_tpu_sys默认分频系数是4
    } else if (nodeName == "clk_gic") {
        calculatedFreq = clkMPLLFreq / 4;  // clk_gic默认分频系数是4
    } else if (nodeName == "clk_bus") {
        calculatedFreq = clkMPLLFreq / 2;  // clk_bus默认分频系数是2
    } else if (nodeName == "clk_rtc_sys") {
        calculatedFreq = clkMPLLFreq / 4;  // clk_rtc_sys默认分频系数是4
    } else if (nodeName == "clk_hsperi") {
        calculatedFreq = clkMPLLFreq / 4;  // clk_hsperi默认分频系数是4
    }
    QString freqText = QString("%1 MHz").arg(calculatedFreq, 0, 'f', 1);
    
    QLabel* freqLabel = new QLabel(freqText);
    freqLabel->setStyleSheet("color: #dc3545; font-family: monospace; font-weight: bold; font-size: 9px;");
    freqLabel->setAlignment(Qt::AlignCenter);
    
    layout->addWidget(nameLabel);
    layout->addLayout(divConfigLayout);
    layout->addWidget(freqLabel);

    // 存储控件引用
    m_clkMPLLSubNodeWidgets[nodeName] = subNodeWidget;
    m_clkMPLLSubNodeFreqLabels[nodeName] = freqLabel;
    m_clkMPLLSubNodeDividerBoxes[nodeName] = divBox;

    m_clkMPLLSubNodeLayout->addWidget(subNodeWidget);

    // 连接分频器变化信号
    connect(divBox, QOverload<int>::of(&QSpinBox::valueChanged),
            [this, nodeName](int value) {
                onClkMPLLSubNodeDividerChanged(nodeName, value);
            });
}

void ClockConfigWidget::createClkFAB100MSubNodeWidget(const QString& nodeName, QWidget* parent)
{
    QWidget* subNodeWidget = new QWidget();
    subNodeWidget->setStyleSheet(
        "QWidget { "
        "background-color: #ffffff; "
        "border: 1px solid #6f42c1; "
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
    nameLabel->setStyleSheet("font-weight: bold; color: #4b0082; font-size: 11px;");
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setWordWrap(true);
    
    // 分频器配置
    QHBoxLayout* divConfigLayout = new QHBoxLayout();
    divConfigLayout->setSpacing(3);
    
    QLabel* divLabel = new QLabel("分频：");
    divLabel->setStyleSheet("color: #4b0082; font-size: 10px; font-weight: bold;");
    
    QSpinBox* divBox = new QSpinBox();
    divBox->setRange(1, 1000);
    // 根据子节点设置不同的默认分频值
    if (nodeName == "clk_fab6_100M_free") {
        divBox->setValue(1);  // clk_fab6_100M_free默认分频系数是1
    } else if (nodeName == "clk_efuse_pclk") {
        divBox->setValue(1);  // clk_efuse_pclk默认分频系数是1
    } else if (nodeName == "clk_x2p") {
        divBox->setValue(1);  // clk_x2p默认分频系数是1
    }
    divBox->setFixedWidth(45);
    divBox->setStyleSheet("font-size: 10px;");
    
    divConfigLayout->addWidget(divLabel);
    divConfigLayout->addWidget(divBox);
    
    // 频率显示
    double clkFAB100MFreq = 100.0;  // 假设clk_fab_100M = 100MHz
    double calculatedFreq;
    if (nodeName == "clk_fab6_100M_free") {
        calculatedFreq = clkFAB100MFreq / 1;  // clk_fab6_100M_free默认分频系数是1
    } else if (nodeName == "clk_efuse_pclk") {
        calculatedFreq = clkFAB100MFreq / 1;  // clk_efuse_pclk默认分频系数是1
    } else if (nodeName == "clk_x2p") {
        calculatedFreq = clkFAB100MFreq / 1;  // clk_x2p默认分频系数是1
    }
    QString freqText = QString("%1 MHz").arg(calculatedFreq, 0, 'f', 1);
    
    QLabel* freqLabel = new QLabel(freqText);
    freqLabel->setStyleSheet("color: #dc3545; font-family: monospace; font-weight: bold; font-size: 9px;");
    freqLabel->setAlignment(Qt::AlignCenter);
    
    layout->addWidget(nameLabel);
    layout->addLayout(divConfigLayout);
    layout->addWidget(freqLabel);

    // 存储控件引用
    m_clkFAB100MSubNodeWidgets[nodeName] = subNodeWidget;
    m_clkFAB100MSubNodeFreqLabels[nodeName] = freqLabel;
    m_clkFAB100MSubNodeDividerBoxes[nodeName] = divBox;

    m_clkFAB100MSubNodeLayout->addWidget(subNodeWidget);

    // 连接分频器变化信号
    connect(divBox, QOverload<int>::of(&QSpinBox::valueChanged),
            [this, nodeName](int value) {
                onClkFAB100MSubNodeDividerChanged(nodeName, value);
            });
}

void ClockConfigWidget::createClkSPINANDSubNodeWidget(const QString& nodeName, QWidget* parent)
{
    QWidget* subNodeWidget = new QWidget();
    subNodeWidget->setStyleSheet(
        "QWidget { "
        "background-color: #ffffff; "
        "border: 1px solid #343a40; "
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
    nameLabel->setStyleSheet("font-weight: bold; color: #212529; font-size: 11px;");
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setWordWrap(true);
    
    // 分频器配置
    QHBoxLayout* divConfigLayout = new QHBoxLayout();
    divConfigLayout->setSpacing(3);
    
    QLabel* divLabel = new QLabel("分频：");
    divLabel->setStyleSheet("color: #212529; font-size: 10px; font-weight: bold;");
    
    QSpinBox* divBox = new QSpinBox();
    divBox->setRange(1, 1000);
    divBox->setValue(1);  // 默认分频系数是1
    divBox->setFixedWidth(45);
    divBox->setStyleSheet("font-size: 10px;");
    
    divConfigLayout->addWidget(divLabel);
    divConfigLayout->addWidget(divBox);
    
    // 频率显示
    double clkSPINANDFreq = 300.0;  // 假设clk_spi_nand = 300MHz
    double calculatedFreq = clkSPINANDFreq / 1;  // 默认分频为1
    QString freqText = QString("%1 MHz").arg(calculatedFreq, 0, 'f', 1);
    
    QLabel* freqLabel = new QLabel(freqText);
    freqLabel->setStyleSheet("color: #dc3545; font-family: monospace; font-weight: bold; font-size: 9px;");
    freqLabel->setAlignment(Qt::AlignCenter);
    
    layout->addWidget(nameLabel);
    layout->addLayout(divConfigLayout);
    layout->addWidget(freqLabel);

    // 存储控件引用
    m_clkSPINANDSubNodeWidgets[nodeName] = subNodeWidget;
    m_clkSPINANDSubNodeFreqLabels[nodeName] = freqLabel;
    m_clkSPINANDSubNodeDividerBoxes[nodeName] = divBox;

    m_clkSPINANDSubNodeLayout->addWidget(subNodeWidget);

    // 连接分频器变化信号
    connect(divBox, QOverload<int>::of(&QSpinBox::valueChanged),
            [this, nodeName](int value) {
                onClkSPINANDSubNodeDividerChanged(nodeName, value);
            });
}

void ClockConfigWidget::createClkHSPeriSubNodeWidget(const QString& nodeName, QWidget* parent)
{
    QWidget* subNodeWidget = new QWidget();
    subNodeWidget->setStyleSheet(
        "QWidget { "
        "background-color: #ffffff; "
        "border: 1px solid #17a2b8; "
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
    nameLabel->setStyleSheet("font-weight: bold; color: #117a8b; font-size: 11px;");
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setWordWrap(true);
    
    // 分频器配置
    QHBoxLayout* divConfigLayout = new QHBoxLayout();
    divConfigLayout->setSpacing(3);
    
    QLabel* divLabel = new QLabel("分频：");
    divLabel->setStyleSheet("color: #117a8b; font-size: 10px; font-weight: bold;");
    
    QSpinBox* divBox = new QSpinBox();
    divBox->setRange(1, 1000);
    // 根据子节点设置不同的默认分频值
    if (nodeName == "clk_sdma1_axi") {
        divBox->setValue(1);  // clk_sdma1_axi默认分频系数是1
    } else if (nodeName == "clk_sdma0_axi") {
        divBox->setValue(1);  // clk_sdma0_axi默认分频系数是1
    }
    divBox->setFixedWidth(45);
    divBox->setStyleSheet("font-size: 10px;");
    
    divConfigLayout->addWidget(divLabel);
    divConfigLayout->addWidget(divBox);
    
    // 频率显示
    double clkHSPeriFreq = 300.0;  // 假设clk_hsperi = 300MHz
    double calculatedFreq;
    if (nodeName == "clk_sdma1_axi") {
        calculatedFreq = clkHSPeriFreq / 1;  // clk_sdma1_axi默认分频系数是1
    } else if (nodeName == "clk_sdma0_axi") {
        calculatedFreq = clkHSPeriFreq / 1;  // clk_sdma0_axi默认分频系数是1
    }
    QString freqText = QString("%1 MHz").arg(calculatedFreq, 0, 'f', 1);
    
    QLabel* freqLabel = new QLabel(freqText);
    freqLabel->setStyleSheet("color: #dc3545; font-family: monospace; font-weight: bold; font-size: 9px;");
    freqLabel->setAlignment(Qt::AlignCenter);
    
    layout->addWidget(nameLabel);
    layout->addLayout(divConfigLayout);
    layout->addWidget(freqLabel);

    // 存储控件引用
    m_clkHSPeriSubNodeWidgets[nodeName] = subNodeWidget;
    m_clkHSPeriSubNodeFreqLabels[nodeName] = freqLabel;
    m_clkHSPeriSubNodeDividerBoxes[nodeName] = divBox;

    m_clkHSPeriSubNodeLayout->addWidget(subNodeWidget);

    // 连接分频器变化信号
    connect(divBox, QOverload<int>::of(&QSpinBox::valueChanged),
            [this, nodeName](int value) {
                onClkHSPeriSubNodeDividerChanged(nodeName, value);
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
    ModulePosition inputPos = {"输入源", 25, 730, 100, 350};
    ModulePosition pllPos = {"锁相环", 240, 600, 200, 700};
    ModulePosition subPllPos = {"子锁相环", 980, 475, 180, 780};
    ModulePosition outputPos = {"OSC输出", 1780, 25, 200, 3500}; // 调整高度以容纳所有输出节点
    ModulePosition clk1MSubPos = {"clk_1M子节点", 2050, 3000, 150, 350};
    ModulePosition clkCam1PLLSubPos = {"clk_cam1pll子节点", 1280, 650, 150, 450}; // 新增clk_cam1pll子节点位置
    ModulePosition clkRawAxiSubPos = {"clk_raw_axi子节点", 1500, 750, 180, 550}; // 新增clk_raw_axi子节点位置
    ModulePosition clkCam0PLLSubPos = {"clk_cam0pll子节点", 1500, 480, 150, 250}; // 新增clk_cam0pll子节点位置
    ModulePosition clkDispPLLSubPos = {"clk_disppll子节点", 1280, 1160, 150, 250}; // 新增clk_disppll子节点位置
    ModulePosition clkSysDispSubPos = {"clk_sys_disp子节点", 1500, 1325, 150, 120}; // 新增clk_sys_disp子节点位置
    ModulePosition clkA0PLLSubPos = {"clk_a0pll子节点", 1280, 130, 150, 450}; // 新增clk_a0pll子节点位置
    ModulePosition clkRVPLLSubPos = {"clk_rvpll子节点", 510, 1300, 150, 120}; // 新增clk_rvpll子节点位置
    ModulePosition clkAPPLLSubPos = {"clk_appll子节点", 510, 1130, 150, 120}; // 新增clk_appll子节点位置
    ModulePosition clkFPLLSubPos = {"clk_fpll子节点", 510, 55, 150, 800}; // 新增clk_fpll子节点位置
    ModulePosition clkTPLLSubPos = {"clk_tpu子节点", 510, 920, 150, 200}; // 新增clk_tpu子节点位置
    ModulePosition clkMPLLSubPos = {"clk_mpll子节点", 750, 880, 150, 1600}; // 新增clk_mpll子节点位置
    ModulePosition clkFAB100MSubPos = {"clk_fab100m子节点", 750, 450, 150, 300}; // 新增clk_fab100m子节点位置
    ModulePosition clkSPINANDSubPos = {"clk_spi_nand子节点", 984, 1335, 150, 120}; // 新增clk_spi_nand子节点位置
    ModulePosition clkHSPISubPos = {"clk_hspi子节点", 955, 2352, 150, 210}; // 新增clk_hspi子节点位置

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
    m_modulePositions["clk_appll子节点"] = clkAPPLLSubPos;
    m_modulePositions["clk_fpll子节点"] = clkFPLLSubPos;
    m_modulePositions["clk_tpu子节点"] = clkTPLLSubPos;
    m_modulePositions["clk_mpll子节点"] = clkMPLLSubPos;
    m_modulePositions["clk_fab100m子节点"] = clkFAB100MSubPos;
    m_modulePositions["clk_spi_nand子节点"] = clkSPINANDSubPos;
    m_modulePositions["clk_hspi子节点"] = clkHSPISubPos;
    
    // 应用默认位置
    applyModulePositions();
}

void ClockConfigWidget::applyModulePositions()
{
    if (m_modulePositions.contains("输入源") && m_inputWidget) {
        ModulePosition pos = m_modulePositions["输入源"];
        m_inputWidget->move(pos.x, pos.y);
        m_inputWidget->setFixedSize(pos.width, pos.height);
    }
    
    if (m_modulePositions.contains("锁相环") && m_pllWidget) {
        ModulePosition pos = m_modulePositions["锁相环"];
        m_pllWidget->move(pos.x, pos.y);
        m_pllWidget->setFixedSize(pos.width, pos.height);
    }
    
    if (m_modulePositions.contains("子锁相环") && m_subPllWidget) {
        ModulePosition pos = m_modulePositions["子锁相环"];
        m_subPllWidget->move(pos.x, pos.y);
        m_subPllWidget->setFixedSize(pos.width, pos.height);
    }
    
    if (m_modulePositions.contains("OSC输出") && m_outputWidget) {
        ModulePosition pos = m_modulePositions["OSC输出"];
        m_outputWidget->move(pos.x, pos.y);
        m_outputWidget->setFixedSize(pos.width, pos.height);
    }
    
    if (m_modulePositions.contains("clk_1M子节点") && m_clk1MSubNodeWidget) {
        ModulePosition pos = m_modulePositions["clk_1M子节点"];
        m_clk1MSubNodeWidget->move(pos.x, pos.y);
        m_clk1MSubNodeWidget->setFixedSize(pos.width, pos.height);
    }
    
    if (m_modulePositions.contains("clk_cam1pll子节点") && m_clkCam1PLLSubNodeWidget) {
        ModulePosition pos = m_modulePositions["clk_cam1pll子节点"];
        m_clkCam1PLLSubNodeWidget->move(pos.x, pos.y);
        m_clkCam1PLLSubNodeWidget->setFixedSize(pos.width, pos.height);
    }
    
    if (m_modulePositions.contains("clk_raw_axi子节点") && m_clkRawAxiSubNodeWidget) {
        ModulePosition pos = m_modulePositions["clk_raw_axi子节点"];
        m_clkRawAxiSubNodeWidget->move(pos.x, pos.y);
        m_clkRawAxiSubNodeWidget->setFixedSize(pos.width, pos.height);
    }
    
    if (m_modulePositions.contains("clk_cam0pll子节点") && m_clkCam0PLLSubNodeWidget) {
        ModulePosition pos = m_modulePositions["clk_cam0pll子节点"];
        m_clkCam0PLLSubNodeWidget->move(pos.x, pos.y);
        m_clkCam0PLLSubNodeWidget->setFixedSize(pos.width, pos.height);
    }
    
    if (m_modulePositions.contains("clk_disppll子节点") && m_clkDispPLLSubNodeWidget) {
        ModulePosition pos = m_modulePositions["clk_disppll子节点"];
        m_clkDispPLLSubNodeWidget->move(pos.x, pos.y);
        m_clkDispPLLSubNodeWidget->setFixedSize(pos.width, pos.height);
    }
    
    if (m_modulePositions.contains("clk_sys_disp子节点") && m_clkSysDispSubNodeWidget) {
        ModulePosition pos = m_modulePositions["clk_sys_disp子节点"];
        m_clkSysDispSubNodeWidget->move(pos.x, pos.y);
        m_clkSysDispSubNodeWidget->setFixedSize(pos.width, pos.height);
    }
    
    if (m_modulePositions.contains("clk_a0pll子节点") && m_clkA0PLLSubNodeWidget) {
        ModulePosition pos = m_modulePositions["clk_a0pll子节点"];
        m_clkA0PLLSubNodeWidget->move(pos.x, pos.y);
        m_clkA0PLLSubNodeWidget->setFixedSize(pos.width, pos.height);
    }

    if (m_modulePositions.contains("clk_rvpll子节点") && m_clkRVPLLSubNodeWidget) {
        ModulePosition pos = m_modulePositions["clk_rvpll子节点"];
        m_clkRVPLLSubNodeWidget->move(pos.x, pos.y);
        m_clkRVPLLSubNodeWidget->setFixedSize(pos.width, pos.height);
    }

    if (m_modulePositions.contains("clk_appll子节点") && m_clkAPPLLSubNodeWidget) {
        ModulePosition pos = m_modulePositions["clk_appll子节点"];
        m_clkAPPLLSubNodeWidget->move(pos.x, pos.y);
        m_clkAPPLLSubNodeWidget->setFixedSize(pos.width, pos.height);
    }

    if (m_modulePositions.contains("clk_fpll子节点") && m_clkFPLLSubNodeWidget) {
        ModulePosition pos = m_modulePositions["clk_fpll子节点"];
        m_clkFPLLSubNodeWidget->move(pos.x, pos.y);
        m_clkFPLLSubNodeWidget->setFixedSize(pos.width, pos.height);
    }

    if (m_modulePositions.contains("clk_tpu子节点") && m_clkTPLLSubNodeWidget) {
        ModulePosition pos = m_modulePositions["clk_tpu子节点"];
        m_clkTPLLSubNodeWidget->move(pos.x, pos.y);
        m_clkTPLLSubNodeWidget->setFixedSize(pos.width, pos.height);
    }

    if (m_modulePositions.contains("clk_mpll子节点") && m_clkMPLLSubNodeWidget) {
        ModulePosition pos = m_modulePositions["clk_mpll子节点"];
        m_clkMPLLSubNodeWidget->move(pos.x, pos.y);
        m_clkMPLLSubNodeWidget->setFixedSize(pos.width, pos.height);
    }

    if (m_modulePositions.contains("clk_fab100m子节点") && m_clkFAB100MSubNodeWidget) {
        ModulePosition pos = m_modulePositions["clk_fab100m子节点"];
        m_clkFAB100MSubNodeWidget->move(pos.x, pos.y);
        m_clkFAB100MSubNodeWidget->setFixedSize(pos.width, pos.height);
    }

    if (m_modulePositions.contains("clk_spi_nand子节点") && m_clkSPINANDSubNodeWidget) {
        ModulePosition pos = m_modulePositions["clk_spi_nand子节点"];
        m_clkSPINANDSubNodeWidget->move(pos.x, pos.y);
        m_clkSPINANDSubNodeWidget->setFixedSize(pos.width, pos.height);
    }

    if (m_modulePositions.contains("clk_hspi子节点") && m_clkHSPeriSubNodeWidget) {
        ModulePosition pos = m_modulePositions["clk_hspi子节点"];
        m_clkHSPeriSubNodeWidget->move(pos.x, pos.y);
        m_clkHSPeriSubNodeWidget->setFixedSize(pos.width, pos.height);
    }
    
    // 重绘连接线
    updateConnectionOverlay();
}

void ClockConfigWidget::showPositionConfigDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle("模块位置配置");
    dialog.setModal(true);
    dialog.resize(600, 800); // 增加宽度和高度以容纳更多模块
    
    QVBoxLayout* mainLayout = new QVBoxLayout(&dialog);
    
    // 创建滚动区域
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    QWidget* scrollWidget = new QWidget();
    QFormLayout* formLayout = new QFormLayout(scrollWidget);
    
    // 为每个模块创建位置配置控件
    QMap<QString, QSpinBox*> xSpinBoxes;
    QMap<QString, QSpinBox*> ySpinBoxes;
    QMap<QString, QSpinBox*> widthSpinBoxes;
    QMap<QString, QSpinBox*> heightSpinBoxes;
    
    // 建立显示名称和内部模块key的映射关系
    QMap<QString, QString> displayNameToModuleKey = {
        {"输入源", "input"},
        {"锁相环", "pll"},
        {"子锁相环", "subpll"},
        {"OSC输出", "output"},
        {"clk_1M子节点", "clk_1M"},
        {"clk_cam1pll子节点", "clk_cam1pll"},
        {"clk_raw_axi子节点", "clk_raw_axi"},
        {"clk_cam0pll子节点", "clk_cam0pll"},
        {"clk_disppll子节点", "clk_disppll"},
        {"clk_sys_disp子节点", "clk_sys_disp"},
        {"clk_a0pll子节点", "clk_a0pll"},
        {"clk_rvpll子节点", "clk_rvpll"},
        {"clk_appll子节点", "clk_appll"},
        {"clk_fpll子节点", "clk_fpll"},
        {"clk_tpu子节点", "clk_tpll"},
        {"clk_mpll子节点", "clk_mpll"},
        {"clk_fab100m子节点", "clk_fab_100M"},
        {"clk_spi_nand子节点", "clk_spi_nand"},
        {"clk_hspi子节点", "clk_hsperi"}
    };
    
    QStringList moduleNames = displayNameToModuleKey.keys();
    
    for (const QString& moduleName : moduleNames) {
        QGroupBox* moduleGroup = new QGroupBox(moduleName);
        QGridLayout* moduleLayout = new QGridLayout(moduleGroup);
        
        // 获取对应的widget来读取当前实际位置
        QString moduleKey = displayNameToModuleKey.value(moduleName);
        QWidget* widget = getWidgetByModuleName(moduleKey);
        
        // X坐标
        QLabel* xLabel = new QLabel("X坐标:");
        QSpinBox* xSpinBox = new QSpinBox();
        xSpinBox->setRange(0, 3000);
        if (widget) {
            xSpinBox->setValue(widget->x());
        } else if (m_modulePositions.contains(moduleName)) {
            xSpinBox->setValue(m_modulePositions[moduleName].x);
        }
        xSpinBoxes[moduleName] = xSpinBox;
        
        // Y坐标
        QLabel* yLabel = new QLabel("Y坐标:");
        QSpinBox* ySpinBox = new QSpinBox();
        ySpinBox->setRange(0, 3000);
        if (widget) {
            ySpinBox->setValue(widget->y());
        } else if (m_modulePositions.contains(moduleName)) {
            ySpinBox->setValue(m_modulePositions[moduleName].y);
        }
        ySpinBoxes[moduleName] = ySpinBox;
        
        // 宽度
        QLabel* widthLabel = new QLabel("宽度:");
        QSpinBox* widthSpinBox = new QSpinBox();
        widthSpinBox->setRange(50, 2000); // 增加最大值以支持更大的模块
        if (widget) {
            widthSpinBox->setValue(widget->width());
        } else if (m_modulePositions.contains(moduleName)) {
            widthSpinBox->setValue(m_modulePositions[moduleName].width);
        }
        widthSpinBoxes[moduleName] = widthSpinBox;
        
        // 高度
        QLabel* heightLabel = new QLabel("高度:");
        QSpinBox* heightSpinBox = new QSpinBox();
        heightSpinBox->setRange(50, 5000); // 增加最大值以支持更高的模块
        if (widget) {
            heightSpinBox->setValue(widget->height());
        } else if (m_modulePositions.contains(moduleName)) {
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
    
    scrollArea->setWidget(scrollWidget);
    mainLayout->addWidget(scrollArea);
    
    // 添加按钮
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::RestoreDefaults);
    
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(buttonBox->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked, [&]() {
        // 恢复默认值
        initializeModulePositions();
        for (const QString& moduleName : moduleNames) {
            QString moduleKey = displayNameToModuleKey.value(moduleName);
            QWidget* widget = getWidgetByModuleName(moduleKey);
            
            if (widget) {
                // 使用当前widget的实际位置
                xSpinBoxes[moduleName]->setValue(widget->x());
                ySpinBoxes[moduleName]->setValue(widget->y());
                widthSpinBoxes[moduleName]->setValue(widget->width());
                heightSpinBoxes[moduleName]->setValue(widget->height());
            } else if (m_modulePositions.contains(moduleName)) {
                // 回退到存储的位置
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
    connect(m_applyButton, &QPushButton::clicked, this, &ClockConfigWidget::applyOverclockConfig);
    
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

    // 如果是clk_appll，还需要更新其子节点的频率
    if (pllName == "clk_appll") {
        updateAllClkAPPLLSubNodeFrequencies();
    }

    // 如果是clk_fpll，还需要更新其子节点的频率
    if (pllName == "clk_fpll") {
        updateAllClkFPLLSubNodeFrequencies();
    }

    // 如果是clk_tpll，还需要更新其子节点的频率
    if (pllName == "clk_tpll") {
        updateAllClkTPLLSubNodeFrequencies();
    }

    // 如果是clk_mpll，还需要更新其子节点的频率
    if (pllName == "clk_mpll") {
        updateAllClkMPLLSubNodeFrequencies();
    }

    // 如果是clk_fab_100M，还需要更新其子节点的频率
    if (pllName == "clk_fab_100M") {
        updateAllClkFAB100MSubNodeFrequencies();
    }

    // 如果是clk_spi_nand，还需要更新其子节点的频率
    if (pllName == "clk_spi_nand") {
        updateAllClkSPINANDSubNodeFrequencies();
    }

    // 如果是clk_hspi，还需要更新其子节点的频率
    if (pllName == "clk_hspi") {
        updateAllClkHSPeriSubNodeFrequencies();
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

    // 如果是clk_cam0pll，还需要更新其子节点的频率
    if (pllName == "clk_cam0pll") {
        updateAllClkCam0PLLSubNodeFrequencies();
    }

    // 如果是clk_cam1pll，还需要更新其子节点的频率
    if (pllName == "clk_cam1pll") {
        updateAllClkCam1PLLSubNodeFrequencies();
    }

    // 如果是clk_disppll，还需要更新其子节点的频率
    if (pllName == "clk_disppll") {
        updateAllClkDispPLLSubNodeFrequencies();
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

        // 如果修改的是clk_raw_axi，更新其子节点
        if (nodeName == "clk_raw_axi") {
            // 同步更新m_pllConfigs中的频率
            if (m_pllConfigs.contains("clk_raw_axi")) {
                m_pllConfigs["clk_raw_axi"].outputFreq = m_clkMPLLSubNodes[nodeName].frequency;
            }
            // 更新clk_raw_axi的所有子节点频率
            updateAllClkRawAxiSubNodeFrequencies();
        }

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

        // 如果修改的是clk_sys_disp，并更新其子节点
        if (nodeName == "clk_sys_disp") {
            // 同步更新m_pllConfigs中的频率
            if (m_pllConfigs.contains("clk_sys_disp")) {
                m_pllConfigs["clk_sys_disp"].outputFreq = m_clkMPLLSubNodes[nodeName].frequency;
            }
            // 更新clk_sys_disp的所有子节点频率
            updateAllClkSysDispSubNodeFrequencies();
        }

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

void ClockConfigWidget::onClkAPPLLSubNodeDividerChanged(const QString& nodeName, int divider)
{
    if (m_clkAPPLLSubNodes.contains(nodeName)) {
        m_clkAPPLLSubNodes[nodeName].divider = divider;
        updateClkAPPLLSubNodeFrequency(nodeName);
        emit configChanged();
    }
}

void ClockConfigWidget::onClkFPLLSubNodeDividerChanged(const QString& nodeName, int divider)
{
    if (m_clkFPLLSubNodes.contains(nodeName)) {
        m_clkFPLLSubNodes[nodeName].divider = divider;
        updateClkFPLLSubNodeFrequency(nodeName);

        // 如果修改的是clk_fab_100M，需要同步更新其在m_pllConfigs中的频率，并更新其子节点
        if (nodeName == "clk_fab_100M") {
            // 同步更新m_pllConfigs中的频率
            if (m_pllConfigs.contains("clk_fab_100M")) {
                m_pllConfigs["clk_fab_100M"].outputFreq = m_clkFPLLSubNodes[nodeName].frequency;
            }
            // 更新clk_fab_100M的所有子节点频率
            updateAllClkFAB100MSubNodeFrequencies();
        }

        emit configChanged();
    }
}

void ClockConfigWidget::onClkTPLLSubNodeDividerChanged(const QString& nodeName, int divider)
{
    if (m_clkTPLLSubNodes.contains(nodeName)) {
        m_clkTPLLSubNodes[nodeName].divider = divider;
        updateClkTPLLSubNodeFrequency(nodeName);
        emit configChanged();
    }
}

void ClockConfigWidget::onClkMPLLSubNodeDividerChanged(const QString& nodeName, int divider)
{
    if (m_clkMPLLSubNodes.contains(nodeName)) {
        m_clkMPLLSubNodes[nodeName].divider = divider;
        updateClkMPLLSubNodeFrequency(nodeName);
        
        // 如果修改的是clk_spi_nand，需要同步更新其在m_pllConfigs中的频率，并更新其子节点
        if (nodeName == "clk_spi_nand") {
            // 同步更新m_pllConfigs中的频率
            if (m_pllConfigs.contains("clk_spi_nand")) {
                m_pllConfigs["clk_spi_nand"].outputFreq = m_clkMPLLSubNodes[nodeName].frequency;
            }
            // 更新clk_spi_nand的所有子节点频率
            updateAllClkSPINANDSubNodeFrequencies();
        }

        // 如果修改的是clk_hsperi，需要同步更新其在m_pllConfigs中的频率，并更新其子节点
        if (nodeName == "clk_hsperi") {
            // 同步更新m_pllConfigs中的频率
            if (m_pllConfigs.contains("clk_hsperi")) {
                m_pllConfigs["clk_hsperi"].outputFreq = m_clkMPLLSubNodes[nodeName].frequency;
            }
            // 更新clk_hsperi的所有子节点频率
            updateAllClkHSPeriSubNodeFrequencies();
        }
        
        emit configChanged();
    }
}

void ClockConfigWidget::onClkFAB100MSubNodeDividerChanged(const QString& nodeName, int divider)
{
    if (m_clkFAB100MSubNodes.contains(nodeName)) {
        m_clkFAB100MSubNodes[nodeName].divider = divider;
        updateClkFAB100MSubNodeFrequency(nodeName);
        emit configChanged();
    }
}

void ClockConfigWidget::onClkSPINANDSubNodeDividerChanged(const QString& nodeName, int divider)
{
    if (m_clkSPINANDSubNodes.contains(nodeName)) {
        m_clkSPINANDSubNodes[nodeName].divider = divider;
        updateClkSPINANDSubNodeFrequency(nodeName);
        emit configChanged();
    }
}

void ClockConfigWidget::onClkHSPeriSubNodeDividerChanged(const QString& nodeName, int divider)
{
    if (m_clkHSPeriSubNodes.contains(nodeName)) {
        m_clkHSPeriSubNodes[nodeName].divider = divider;
        updateClkHSPeriSubNodeFrequency(nodeName);
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
    // 同步更新m_pllConfigs[子锁相环子节点]中的频率
    m_pllConfigs[pllName].outputFreq = config.outputFreq;
    
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

    // 更新clk_a0pll的所有子节点频率
    updateAllClkA0PLLSubNodeFrequencies();

    // 更新clk_cam0pll的所有子节点频率
    updateAllClkCam0PLLSubNodeFrequencies();

    // 更新clk_cam1pll的所有子节点频率
    updateAllClkCam1PLLSubNodeFrequencies();

    // 更新clk_disppll的所有子节点频率
    updateAllClkDispPLLSubNodeFrequencies();
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
    // 同步更新m_pllConfigs[clk_cam1pll子节点]中的频率
    m_pllConfigs[nodeName].outputFreq = subNode.frequency;
    
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

    // 更新clk_raw_axi的所有子节点频率
    updateAllClkRawAxiSubNodeFrequencies();
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
    // 同步更新m_pllConfigs[clk_disppll子节点]中的频率
    m_pllConfigs[nodeName].outputFreq = subNode.frequency;
    
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

    // 更新clk_sys_disp的所有子节点频率
    updateAllClkSysDispSubNodeFrequencies();
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

void ClockConfigWidget::updateClkAPPLLSubNodeFrequency(const QString& nodeName)
{
    if (!m_clkAPPLLSubNodes.contains(nodeName)) return;

    ClockOutput& subNode = m_clkAPPLLSubNodes[nodeName];

    // 获取clk_appll的频率
    double clkAPPLLFreq = 1000.0; // 默认1000MHz
    if (m_pllConfigs.contains("clk_appll")) {
        clkAPPLLFreq = m_pllConfigs["clk_appll"].outputFreq;
    }

    // clk_appll子节点频率 = clk_appll频率 / 分频器
    subNode.frequency = clkAPPLLFreq / subNode.divider;

    // 更新显示
    if (m_clkAPPLLSubNodeFreqLabels.contains(nodeName)) {
        QString freqText = QString("%1 MHz").arg(subNode.frequency, 0, 'f', 1);
        m_clkAPPLLSubNodeFreqLabels[nodeName]->setText(freqText);
    }
}
void ClockConfigWidget::updateAllClkAPPLLSubNodeFrequencies()
{
    for (const QString& nodeName : CLK_APPLL_SUB_NODES) {
        updateClkAPPLLSubNodeFrequency(nodeName);
    }
}

void ClockConfigWidget::updateClkFPLLSubNodeFrequency(const QString& nodeName)
{
    if (!m_clkFPLLSubNodes.contains(nodeName)) return;

    ClockOutput& subNode = m_clkFPLLSubNodes[nodeName];

    // 获取clk_fpll的频率
    double clkFPLLFreq = 1200.0; // 默认1200MHz
    if (m_pllConfigs.contains("clk_fpll")) {
        clkFPLLFreq = m_pllConfigs["clk_fpll"].outputFreq;
    }

    // clk_fpll子节点频率 = clk_fpll频率 / 分频器
    subNode.frequency = clkFPLLFreq / subNode.divider;
    // 同步更新m_pllConfigs[clk_fpll子节点]中的频率
    m_pllConfigs[nodeName].outputFreq = subNode.frequency;

    // 更新显示
    if (m_clkFPLLSubNodeFreqLabels.contains(nodeName)) {
        QString freqText = QString("%1 MHz").arg(subNode.frequency, 0, 'f', 1);
        m_clkFPLLSubNodeFreqLabels[nodeName]->setText(freqText);
    }
}

void ClockConfigWidget::updateAllClkFPLLSubNodeFrequencies()
{
    for (const QString& nodeName : CLK_FPLL_SUB_NODES) {
        updateClkFPLLSubNodeFrequency(nodeName);
    }

    // 更新clk_fab_100M的所有子节点频率
    updateAllClkFAB100MSubNodeFrequencies();
}

void ClockConfigWidget::updateClkTPLLSubNodeFrequency(const QString& nodeName)
{
    if (!m_clkTPLLSubNodes.contains(nodeName)) return;

    ClockOutput& subNode = m_clkTPLLSubNodes[nodeName];

    // 获取clk_tpll的频率
    double clkTPLLFreq = 1500.0; // 默认1500MHz
    if (m_pllConfigs.contains("clk_tpll")) {
        clkTPLLFreq = m_pllConfigs["clk_tpll"].outputFreq;
    }

    // clk_tpll子节点频率 = clk_tpll频率 / 分频器
    subNode.frequency = clkTPLLFreq / subNode.divider;

    // 更新显示
    if (m_clkTPLLSubNodeFreqLabels.contains(nodeName)) {
        QString freqText = QString("%1 MHz").arg(subNode.frequency, 0, 'f', 1);
        m_clkTPLLSubNodeFreqLabels[nodeName]->setText(freqText);
    }
}

void ClockConfigWidget::updateAllClkTPLLSubNodeFrequencies()
{
    for (const QString& nodeName : CLK_TPLL_SUB_NODES) {
        updateClkTPLLSubNodeFrequency(nodeName);
    }
}

void ClockConfigWidget::updateClkMPLLSubNodeFrequency(const QString& nodeName)
{
    if (!m_clkMPLLSubNodes.contains(nodeName)) return;

    ClockOutput& subNode = m_clkMPLLSubNodes[nodeName];

    // 获取clk_mpll的频率
    double clkMPLLFreq = 1200.0; // 默认1200MHz
    if (m_pllConfigs.contains("clk_mpll")) {
        clkMPLLFreq = m_pllConfigs["clk_mpll"].outputFreq;
    }

    // clk_mpll子节点频率 = clk_mpll频率 / 分频器
    subNode.frequency = clkMPLLFreq / subNode.divider;
    // 同步更新m_pllConfigs[clk_mpll子节点]中的频率
    m_pllConfigs[nodeName].outputFreq = subNode.frequency;

    // 更新显示
    if (m_clkMPLLSubNodeFreqLabels.contains(nodeName)) {
        QString freqText = QString("%1 MHz").arg(subNode.frequency, 0, 'f', 1);
        m_clkMPLLSubNodeFreqLabels[nodeName]->setText(freqText);
    }
}

void ClockConfigWidget::updateAllClkMPLLSubNodeFrequencies()
{
    for (const QString& nodeName : CLK_MPLL_SUB_NODES) {
        updateClkMPLLSubNodeFrequency(nodeName);
    }

    // 更新clk_spi_nand的所有子节点频率
    updateAllClkSPINANDSubNodeFrequencies();

    // 更新clk_hsperi的所有子节点频率
    updateAllClkHSPeriSubNodeFrequencies();
}

void ClockConfigWidget::updateClkFAB100MSubNodeFrequency(const QString& nodeName)
{
    if (!m_clkFAB100MSubNodes.contains(nodeName)) return;

    ClockOutput& subNode = m_clkFAB100MSubNodes[nodeName];

    // 获取clk_fab_100m的频率
    double clkFAB100MFreq = 100.0; // 默认100MHz
    if (m_pllConfigs.contains("clk_fab_100M")) {
        clkFAB100MFreq = m_pllConfigs["clk_fab_100M"].outputFreq;
    }

    // clk_fab_100m子节点频率 = clk_fab_100m频率 / 分频器
    subNode.frequency = clkFAB100MFreq / subNode.divider;

    // 更新显示
    if (m_clkFAB100MSubNodeFreqLabels.contains(nodeName)) {
        QString freqText = QString("%1 MHz").arg(subNode.frequency, 0, 'f', 1);
        m_clkFAB100MSubNodeFreqLabels[nodeName]->setText(freqText);
    }
}

void ClockConfigWidget::updateAllClkFAB100MSubNodeFrequencies()
{
    for (const QString& nodeName : CLK_FAB_100M_SUB_NODES) {
        updateClkFAB100MSubNodeFrequency(nodeName);
    }
}

void ClockConfigWidget::updateClkSPINANDSubNodeFrequency(const QString& nodeName)
{
    if (!m_clkSPINANDSubNodes.contains(nodeName)) return;

    ClockOutput& subNode = m_clkSPINANDSubNodes[nodeName];

    // 获取clk_spi_nand的频率
    double clkSPINANDFreq = 300.0; // 默认300MHz
    if (m_pllConfigs.contains("clk_spi_nand")) {
        clkSPINANDFreq = m_pllConfigs["clk_spi_nand"].outputFreq;
    }

    // clk_spi_nand子节点频率 = clk_spi_nand频率 / 分频器
    subNode.frequency = clkSPINANDFreq / subNode.divider;

    // 更新显示
    if (m_clkSPINANDSubNodeFreqLabels.contains(nodeName)) {
        QString freqText = QString("%1 MHz").arg(subNode.frequency, 0, 'f', 1);
        m_clkSPINANDSubNodeFreqLabels[nodeName]->setText(freqText);
    }
}

void ClockConfigWidget::updateAllClkSPINANDSubNodeFrequencies()
{
    for (const QString& nodeName : CLK_SPI_NAND_SUB_NODES) {
        updateClkSPINANDSubNodeFrequency(nodeName);
    }
}

void ClockConfigWidget::updateClkHSPeriSubNodeFrequency(const QString& nodeName)
{
    if (!m_clkHSPeriSubNodes.contains(nodeName)) return;

    ClockOutput& subNode = m_clkHSPeriSubNodes[nodeName];

    // 获取clk_hsperi的频率
    double clkHSPeriFreq = 300.0; // 默认300MHz
    if (m_pllConfigs.contains("clk_hsperi")) {
        clkHSPeriFreq = m_pllConfigs["clk_hsperi"].outputFreq;
    }

    // clk_hsperi子节点频率 = clk_hsperi频率 / 分频器
    subNode.frequency = clkHSPeriFreq / subNode.divider;

    // 更新显示
    if (m_clkHSPeriSubNodeFreqLabels.contains(nodeName)) {
        QString freqText = QString("%1 MHz").arg(subNode.frequency, 0, 'f', 1);
        m_clkHSPeriSubNodeFreqLabels[nodeName]->setText(freqText);
    }
}

void ClockConfigWidget::updateAllClkHSPeriSubNodeFrequencies()
{
    for (const QString& nodeName : CLK_HSPERI_SUB_NODES) {
        updateClkHSPeriSubNodeFrequency(nodeName);
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

    // 更新所有clk_appll子节点频率
    for (const QString& nodeName : CLK_APPLL_SUB_NODES) {
        updateClkAPPLLSubNodeFrequency(nodeName);
    }

    // 更新所有clk_fpll子节点频率
    for (const QString& nodeName : CLK_FPLL_SUB_NODES) {
        updateClkFPLLSubNodeFrequency(nodeName);
    }

    // 更新所有clk_tpll子节点频率
    for (const QString& nodeName : CLK_TPLL_SUB_NODES) {
        updateClkTPLLSubNodeFrequency(nodeName);
    }

    // 更新所有clk_mpll子节点频率
    for (const QString& nodeName : CLK_MPLL_SUB_NODES) {
        updateClkMPLLSubNodeFrequency(nodeName);
    }

    // 更新所有clk_fab100m子节点频率
    for (const QString& nodeName : CLK_FAB_100M_SUB_NODES) {
        updateClkFAB100MSubNodeFrequency(nodeName);
    }

    // 更新所有clk_spinand子节点频率
    for (const QString& nodeName : CLK_SPI_NAND_SUB_NODES) {
        updateClkSPINANDSubNodeFrequency(nodeName);
    }

    // 更新所有clk_hsperi子节点频率
    for (const QString& nodeName : CLK_HSPERI_SUB_NODES) {
        updateClkHSPeriSubNodeFrequency(nodeName);
    }
}

void ClockConfigWidget::resetToDefaults()
{
    // 重置PLL配置
    for (const QString& pllName : PLL_NAMES) {
        // 根据PLL名称设置不同的默认倍频值
        int defaultMultiplier = 20;  // 默认值
        if (pllName == "clk_fpll") {
            defaultMultiplier = 40;
        } else if (pllName == "clk_mipimpll") {
            defaultMultiplier = 36;
        } else if (pllName == "clk_mpll") {
            defaultMultiplier = 48;
        } else if (pllName == "clk_tpll") {
            defaultMultiplier = 60;
        } else if (pllName == "clk_appll") {
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

    // clk_tpu和clk_tpu_gdma的分频值为3
    if (m_clkTPLLSubNodeDividerBoxes.contains("clk_tpu")) {
        m_clkTPLLSubNodeDividerBoxes["clk_tpu"]->setValue(3);
    }
    if (m_clkTPLLSubNodeDividerBoxes.contains("clk_tpu_gdma")) {
        m_clkTPLLSubNodeDividerBoxes["clk_tpu_gdma"]->setValue(3);
    }
    
    updateFrequencies();
    emit configChanged();
    updateConnectionOverlay();  // 重绘连接线

    // 弹出提示信息
    QMessageBox::information(this, "默认ND配置", "默认ND配置已应用!");
}

void ClockConfigWidget::applyOverclockConfig()
{
    // 重置PLL配置为超频的倍频值
    // clk_appll的倍频值为44
    if (m_pllMultiplierBoxes.contains("clk_appll")) {
        m_pllMultiplierBoxes["clk_appll"]->setValue(44);
    }

    // clk_rvpll的倍频值为64
    if (m_pllMultiplierBoxes.contains("clk_rvpll")) {
        m_pllMultiplierBoxes["clk_rvpll"]->setValue(64);
    }

    // clk_tpll的倍频值不变，但子节点分频值改变
    // clk_tpu和clk_tpu_gdma的分频值为2
    if (m_clkTPLLSubNodeDividerBoxes.contains("clk_tpu")) {
        m_clkTPLLSubNodeDividerBoxes["clk_tpu"]->setValue(2);
    }
    if (m_clkTPLLSubNodeDividerBoxes.contains("clk_tpu_gdma")) {
        m_clkTPLLSubNodeDividerBoxes["clk_tpu_gdma"]->setValue(2);
    }

    // 触发全局更新和信号
    updateFrequencies();
    emit configChanged();

    // 弹出提示信息
    QMessageBox::information(this, "超频配置", "OD超频配置已应用!");
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
    
    // 如果有选中的widget，绘制选中边框和缩放手柄
    if (m_selectedWidget) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        
        // 绘制选中边框
        QRect widgetRect = m_selectedWidget->geometry();
        painter.setPen(QPen(Qt::blue, 3, Qt::DashLine));
        painter.drawRect(widgetRect);
        
        // 绘制缩放手柄
        drawResizeHandles(painter, widgetRect);
    }
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
        
        // 获取滚动偏移量，用于调整绘制坐标
        QPoint scrollOffset = QPoint(
            m_flowScrollArea->horizontalScrollBar()->value(),
            m_flowScrollArea->verticalScrollBar()->value()
        );
        
        // 将坐标系平移，使得flowWidget坐标对应到覆盖层坐标
        painter.translate(-scrollOffset);
        
        // 绘制连接线
        drawConnectionLines(painter);
        
        return true;  // 事件已处理
    }
    
    return QWidget::eventFilter(obj, event);
}

void ClockConfigWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        QPoint pos = event->pos();
        
        // 转换鼠标位置到flowWidget坐标系
        QPoint flowPos = convertToFlowWidgetCoordinate(pos);
        
        // 检查是否点击了某个模块widget
        QWidget* clickedWidget = getWidgetAt(flowPos);
        if (clickedWidget) {
            m_selectedWidget = clickedWidget;
            m_lastMousePos = pos;  // 保持原始坐标用于计算delta
            m_dragStartPos = pos;  // 保持原始坐标用于计算delta
            m_originalGeometry = clickedWidget->geometry();
            
            // 检查是否点击了缩放手柄
            QRect widgetRect = clickedWidget->geometry();
            ResizeDirection resizeDir = getResizeDirection(flowPos, widgetRect);
            
            if (resizeDir != None) {
                m_isResizing = true;
                m_resizeDirection = resizeDir;
            } else {
                m_isDragging = true;
            }
            
            // 重绘以显示选中状态
            update();
        } else {
            // 点击空白区域，取消选中
            m_selectedWidget = nullptr;
            update();
        }
    }
    
    QWidget::mousePressEvent(event);
}

void ClockConfigWidget::mouseMoveEvent(QMouseEvent* event)
{
    QPoint pos = event->pos();
    
    if (m_isDragging && m_selectedWidget) {
        // 拖拽模式
        QPoint delta = pos - m_lastMousePos;
        QRect newGeometry = m_selectedWidget->geometry();
        newGeometry.translate(delta);
        
        updateWidgetGeometry(m_selectedWidget, newGeometry);
        m_lastMousePos = pos;
        
        // 更新连接线
        m_connectionOverlay->update();
    } else if (m_isResizing && m_selectedWidget) {
        // 缩放模式
        QRect newGeometry = m_originalGeometry;
        QPoint delta = pos - m_dragStartPos;
        
        switch (m_resizeDirection) {
            case TopLeft:
                newGeometry.setTopLeft(newGeometry.topLeft() + delta);
                break;
            case Top:
                newGeometry.setTop(newGeometry.top() + delta.y());
                break;
            case TopRight:
                newGeometry.setTopRight(newGeometry.topRight() + QPoint(delta.x(), delta.y()));
                break;
            case Right:
                newGeometry.setRight(newGeometry.right() + delta.x());
                break;
            case BottomRight:
                newGeometry.setBottomRight(newGeometry.bottomRight() + delta);
                break;
            case Bottom:
                newGeometry.setBottom(newGeometry.bottom() + delta.y());
                break;
            case BottomLeft:
                newGeometry.setBottomLeft(newGeometry.bottomLeft() + QPoint(delta.x(), delta.y()));
                break;
            case Left:
                newGeometry.setLeft(newGeometry.left() + delta.x());
                break;
            default:
                break;
        }
        
        // 确保最小尺寸
        if (newGeometry.width() < 100) newGeometry.setWidth(100);
        if (newGeometry.height() < 50) newGeometry.setHeight(50);
        
        updateWidgetGeometry(m_selectedWidget, newGeometry);
        
        // 更新连接线
        m_connectionOverlay->update();
    } else {
        // 更新鼠标光标 - 转换坐标到flowWidget坐标系
        QPoint flowPos = convertToFlowWidgetCoordinate(pos);
        updateCursor(flowPos);
    }
    
    QWidget::mouseMoveEvent(event);
}

void ClockConfigWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        if (m_isDragging || m_isResizing) {
            // 完成拖拽或缩放，更新模块位置配置
            if (m_selectedWidget) {
                QString moduleName = getWidgetModuleName(m_selectedWidget);
                if (!moduleName.isEmpty()) {
                    QRect geometry = m_selectedWidget->geometry();
                    ModulePosition pos;
                    pos.moduleName = moduleName;
                    pos.x = geometry.x();
                    pos.y = geometry.y();
                    pos.width = geometry.width();
                    pos.height = geometry.height();
                    m_modulePositions[moduleName] = pos;
                }
            }
        }
        
        m_isDragging = false;
        m_isResizing = false;
        m_resizeDirection = None;
        
        // 恢复默认光标
        setCursor(Qt::ArrowCursor);
    }
    
    QWidget::mouseReleaseEvent(event);
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
    QStringList targetPLLs = {"clk_fpll", "clk_mipimpll", "clk_mpll", "clk_tpll", "clk_appll", "clk_rvpll"};
    
    // 为每个目标PLL绘制从OSC的连接线（使用肘形箭头）
    for (const QString& pllName : targetPLLs) {
        QPoint pllPoint = getPLLConnectionPoint(pllName);
        if (!pllPoint.isNull() && !oscPoint.isNull()) {
            // 使用不同颜色来区分不同的连接线
            QColor lineColor = Qt::blue;
            if (pllName == "clk_fpll") lineColor = QColor(220, 53, 69);      // 红色
            else if (pllName == "clk_mipimpll") lineColor = QColor(255, 193, 7);  // 黄色
            else if (pllName == "clk_mpll") lineColor = QColor(40, 167, 69);      // 绿色
            else if (pllName == "clk_tpll") lineColor = QColor(23, 162, 184);     // 青色
            else if (pllName == "clk_appll") lineColor = QColor(102, 16, 242);    // 紫色
            else if (pllName == "clk_rvpll") lineColor = QColor(255, 102, 0);     // 橙色
            
            // 计算肘形箭头的拐点
            // 第一个拐点：从起点向右延伸一定距离
            int horizontalOffset = 40;  // 水平延伸距离
            QPoint elbow1(oscPoint.x() + horizontalOffset, oscPoint.y());
            
            // 第二个拐点：垂直对齐到目标点
            QPoint elbow2(elbow1.x(), pllPoint.y());
            
            drawElbowArrowLine(painter, oscPoint, elbow1, elbow2, pllPoint, lineColor);
        }
    }
    
    // 绘制MIPIMPLL到子PLL的连接线
    if (m_subPllWidget) {
        QPoint mipimpllPoint = getMIPIMPLLConnectionPoint();
        QPoint subPllPoint = getSubPLLConnectionPoint();
        if (!subPllPoint.isNull() && !mipimpllPoint.isNull()) {
            // 使用橙色来表示MIPIMPLL到子PLL的连接
            QColor lineColor = QColor(255, 165, 0);  // 橙色
            
            // 计算肘形拐点
            QPoint elbow1, elbow2;
            calculateElbowPoints(mipimpllPoint, subPllPoint, elbow1, elbow2);
            drawElbowArrowLine(painter, mipimpllPoint, elbow1, elbow2, subPllPoint, lineColor);
        }
    }
    
    // 绘制OSC到OSC输出的连接线
    if (m_outputWidget) {
        QPoint oscOutputPoint = getOSCConnectionPoint();  // 重用OSC连接点
        QPoint outputAreaPoint = getOutputAreaConnectionPoint();
        
        if (!oscOutputPoint.isNull() && !outputAreaPoint.isNull()) {
            // 使用绿色来表示OSC到输出的连接
            QColor lineColor = QColor(40, 167, 69);  // 绿色
            
            // 计算肘形拐点
            QPoint elbow1, elbow2;
            calculateElbowPoints(oscOutputPoint, outputAreaPoint, elbow1, elbow2);
            drawElbowArrowLine(painter, oscOutputPoint, elbow1, elbow2, outputAreaPoint, lineColor);
        }
    }
    
    // 绘制clk_1M到其子节点的连接线
    if (m_clk1MSubNodeWidget) {
        QPoint clk1MPoint = getClk1MConnectionPoint();
        QPoint subNodePoint = getClk1MSubNodeConnectionPoint();
        if (!subNodePoint.isNull() && !clk1MPoint.isNull()) {
            // 使用紫色来表示clk_1M到子节点的连接
            QColor lineColor = QColor(102, 16, 242);  // 紫色
            
            // 计算肘形拐点
            QPoint elbow1, elbow2;
            calculateElbowPoints(clk1MPoint, subNodePoint, elbow1, elbow2);
            drawElbowArrowLine(painter, clk1MPoint, elbow1, elbow2, subNodePoint, lineColor);
        }
    }
    
    // 绘制clk_cam1pll到其子节点的连接线
    if (m_clkCam1PLLSubNodeWidget) {
        QPoint cam1PLLPoint = getClkCam1PLLConnectionPoint();
        QPoint subNodePoint = getClkCam1PLLSubNodeConnectionPoint();
        if (!subNodePoint.isNull() && !cam1PLLPoint.isNull()) {
            // 使用橙色来表示clk_cam1pll到子节点的连接
            QColor lineColor = QColor(255, 165, 0);  // 橙色
            
            // 计算肘形拐点
            QPoint elbow1, elbow2;
            calculateElbowPoints(cam1PLLPoint, subNodePoint, elbow1, elbow2);
            drawElbowArrowLine(painter, cam1PLLPoint, elbow1, elbow2, subNodePoint, lineColor);
        }
    }
    
    // 绘制clk_raw_axi到其子节点的连接线
    if (m_clkRawAxiSubNodeWidget) {
        QPoint rawAxiPoint = getClkRawAxiConnectionPoint();
        QPoint subNodePoint = getClkRawAxiSubNodeConnectionPoint();
        if (!subNodePoint.isNull() && !rawAxiPoint.isNull()) {
            // 使用绿色来表示clk_raw_axi到子节点的连接
            QColor lineColor = QColor(34, 139, 34);  // 森林绿
            
            // 计算肘形拐点
            QPoint elbow1, elbow2;
            calculateElbowPoints(rawAxiPoint, subNodePoint, elbow1, elbow2);
            drawElbowArrowLine(painter, rawAxiPoint, elbow1, elbow2, subNodePoint, lineColor);
        }
    }
    
    // 绘制clk_cam0pll到其子节点的连接线
    if (m_clkCam0PLLSubNodeWidget) {
        QPoint cam0PLLPoint = getClkCam0PLLConnectionPoint();
        QPoint subNodePoint = getClkCam0PLLSubNodeConnectionPoint();
        if (!subNodePoint.isNull() && !cam0PLLPoint.isNull()) {
            // 使用深蓝色来表示clk_cam0pll到子节点的连接
            QColor lineColor = QColor(25, 25, 112);  // 午夜蓝
            
            // 计算肘形拐点
            QPoint elbow1, elbow2;
            calculateElbowPoints(cam0PLLPoint, subNodePoint, elbow1, elbow2);
            drawElbowArrowLine(painter, cam0PLLPoint, elbow1, elbow2, subNodePoint, lineColor);
        }
    }
    
    // 绘制clk_disppll到其子节点的连接线
    if (m_clkDispPLLSubNodeWidget) {
        QPoint dispPLLPoint = getClkDispPLLConnectionPoint();
        QPoint subNodePoint = getClkDispPLLSubNodeConnectionPoint();
        if (!subNodePoint.isNull() && !dispPLLPoint.isNull()) {
            // 使用深紫色来表示clk_disppll到子节点的连接
            QColor lineColor = QColor(75, 0, 130);  // 靛蓝色
            
            // 计算肘形拐点
            QPoint elbow1, elbow2;
            calculateElbowPoints(dispPLLPoint, subNodePoint, elbow1, elbow2);
            drawElbowArrowLine(painter, dispPLLPoint, elbow1, elbow2, subNodePoint, lineColor);
        }
    }
    
    // 绘制clk_sys_disp到其子节点的连接线
    if (m_clkSysDispSubNodeWidget) {
        QPoint sysDispPoint = getClkSysDispConnectionPoint();
        QPoint subNodePoint = getClkSysDispSubNodeConnectionPoint();
        if (!subNodePoint.isNull() && !sysDispPoint.isNull()) {
            // 使用青绿色来表示clk_sys_disp到子节点的连接
            QColor lineColor = QColor(0, 150, 136);  // 青绿色
            
            // 计算肘形拐点
            QPoint elbow1, elbow2;
            calculateElbowPoints(sysDispPoint, subNodePoint, elbow1, elbow2);
            drawElbowArrowLine(painter, sysDispPoint, elbow1, elbow2, subNodePoint, lineColor);
        }
    }
    
    // 绘制clk_a0pll到其子节点的连接线
    if (m_clkA0PLLSubNodeWidget) {
        QPoint a0PLLPoint = getClkA0PLLConnectionPoint();
        QPoint subNodePoint = getClkA0PLLSubNodeConnectionPoint();
        if (!subNodePoint.isNull() && !a0PLLPoint.isNull()) {
            // 使用深红色来表示clk_a0pll到子节点的连接
            QColor lineColor = QColor(139, 0, 0);  // 深红色
            
            // 计算肘形拐点
            QPoint elbow1, elbow2;
            calculateElbowPoints(a0PLLPoint, subNodePoint, elbow1, elbow2);
            drawElbowArrowLine(painter, a0PLLPoint, elbow1, elbow2, subNodePoint, lineColor);
        }
    }

    // 绘制clk_rvpll到其子节点的连接线
    if (m_clkRVPLLSubNodeWidget) {
        QPoint rvPLLPoint = getClkRVPLLConnectionPoint();
        QPoint subNodePoint = getClkRVPLLSubNodeConnectionPoint();
        if (!subNodePoint.isNull() && !rvPLLPoint.isNull()) {
            // 使用深橙色来表示clk_rvpll到子节点的连接
            QColor lineColor = QColor(255, 140, 0);  // 深橙色
            
            // 计算肘形拐点
            QPoint elbow1, elbow2;
            calculateElbowPoints(rvPLLPoint, subNodePoint, elbow1, elbow2);
            drawElbowArrowLine(painter, rvPLLPoint, elbow1, elbow2, subNodePoint, lineColor);
        }
    }

    // 绘制clk_appll到其子节点的连接线
    if (m_clkAPPLLSubNodeWidget) {
        QPoint apPLLPoint = getClkAPPLLConnectionPoint();
        QPoint subNodePoint = getClkAPPLLSubNodeConnectionPoint();
        if (!subNodePoint.isNull() && !apPLLPoint.isNull()) {
            // 使用深紫色来表示clk_appll到子节点的连接
            QColor lineColor = QColor(128, 0, 128);  // 深紫色
            
            // 计算肘形拐点
            QPoint elbow1, elbow2;
            calculateElbowPoints(apPLLPoint, subNodePoint, elbow1, elbow2);
            drawElbowArrowLine(painter, apPLLPoint, elbow1, elbow2, subNodePoint, lineColor);
        }
    }

    // 绘制clk_fpll到其子节点的连接线
    if (m_clkFPLLSubNodeWidget) {
        QPoint fPLLPoint = getClkFPLLConnectionPoint();
        QPoint subNodePoint = getClkFPLLSubNodeConnectionPoint();
        if (!subNodePoint.isNull() && !fPLLPoint.isNull()) {
            // 使用深蓝色来表示clk_fpll到子节点的连接
            QColor lineColor = QColor(0, 0, 139);  // 深蓝色
            
            // 计算肘形拐点
            QPoint elbow1, elbow2;
            calculateElbowPoints(fPLLPoint, subNodePoint, elbow1, elbow2);
            drawElbowArrowLine(painter, fPLLPoint, elbow1, elbow2, subNodePoint, lineColor);
        }
    }

    // 绘制clk_tpll到其子节点的连接线
    if (m_clkTPLLSubNodeWidget) {
        QPoint tPLLPoint = getClkTPLLConnectionPoint();
        QPoint subNodePoint = getClkTPLLSubNodeConnectionPoint();
        if (!subNodePoint.isNull() && !tPLLPoint.isNull()) {
            // 使用深青色来表示clk_tpll到子节点的连接
            QColor lineColor = QColor(0, 139, 139);  // 深青色
            
            // 计算肘形拐点
            QPoint elbow1, elbow2;
            calculateElbowPoints(tPLLPoint, subNodePoint, elbow1, elbow2);
            drawElbowArrowLine(painter, tPLLPoint, elbow1, elbow2, subNodePoint, lineColor);
        }
    }

    // 绘制clk_mpll到其子节点的连接线
    if (m_clkMPLLSubNodeWidget) {
        QPoint mPLLPoint = getClkMPLLConnectionPoint();
        QPoint subNodePoint = getClkMPLLSubNodeConnectionPoint();
        if (!subNodePoint.isNull() && !mPLLPoint.isNull()) {
            // 使用深绿色来表示clk_mpll到子节点的连接
            QColor lineColor = QColor(0, 128, 0);  // 深绿色
            
            // 计算肘形拐点
            QPoint elbow1, elbow2;
            calculateElbowPoints(mPLLPoint, subNodePoint, elbow1, elbow2);
            drawElbowArrowLine(painter, mPLLPoint, elbow1, elbow2, subNodePoint, lineColor);
        }
    }

    // 绘制clk_fab_100m到其子节点的连接线
    if (m_clkFAB100MSubNodeWidget) {
        QPoint fab100MPoint = getClkFAB100MConnectionPoint();
        QPoint subNodePoint = getClkFAB100MSubNodeConnectionPoint();
        if (!subNodePoint.isNull() && !fab100MPoint.isNull()) {
            // 使用深粉色来表示clk_fab_100m到子节点的连接
            QColor lineColor = QColor(255, 20, 147);  // 深粉色
            
            // 计算肘形拐点
            QPoint elbow1, elbow2;
            calculateElbowPoints(fab100MPoint, subNodePoint, elbow1, elbow2);
            drawElbowArrowLine(painter, fab100MPoint, elbow1, elbow2, subNodePoint, lineColor);
        }
    }

    // 绘制clk_spinand到其子节点的连接线
    if (m_clkSPINANDSubNodeWidget) {
        QPoint spinandPoint = getClkSPINANDConnectionPoint();
        QPoint subNodePoint = getClkSPINANDSubNodeConnectionPoint();
        if (!subNodePoint.isNull() && !spinandPoint.isNull()) {
            // 使用深灰色来表示clk_spinand到子节点的连接
            QColor lineColor = QColor(105, 105, 105);  // 深灰色
            
            // 计算肘形拐点
            QPoint elbow1, elbow2;
            calculateElbowPoints(spinandPoint, subNodePoint, elbow1, elbow2);
            drawElbowArrowLine(painter, spinandPoint, elbow1, elbow2, subNodePoint, lineColor);
        }
    }

    // 绘制clk_hsperi到其子节点的连接线
    if (m_clkHSPeriSubNodeWidget) {
        QPoint hsperiPoint = getClkHSPeriConnectionPoint();
        QPoint subNodePoint = getClkHSPeriSubNodeConnectionPoint();
        if (!subNodePoint.isNull() && !hsperiPoint.isNull()) {
            // 使用深青绿色来表示clk_hsperi到子节点的连接
            QColor lineColor = QColor(0, 100, 0);  // 深青绿色
            
            // 计算肘形拐点
            QPoint elbow1, elbow2;
            calculateElbowPoints(hsperiPoint, subNodePoint, elbow1, elbow2);
            drawElbowArrowLine(painter, hsperiPoint, elbow1, elbow2, subNodePoint, lineColor);
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

void ClockConfigWidget::drawElbowArrowLine(QPainter& painter, const QPoint& start, const QPoint& elbow1, const QPoint& elbow2, const QPoint& end, const QColor& color)
{
    painter.setPen(QPen(color, 2));
    
    // 绘制肘形连接线
    // 第一段：从起点到第一个拐点
    painter.drawLine(start, elbow1);
    // 第二段：从第一个拐点到第二个拐点
    painter.drawLine(elbow1, elbow2);
    // 第三段：从第二个拐点到终点
    painter.drawLine(elbow2, end);
    
    // 绘制箭头（在最后一段线上）
    painter.setBrush(QBrush(color));
    
    // 计算箭头方向和大小
    int arrowSize = 10;
    QVector2D direction(end - elbow2);
    direction.normalize();
    
    // 计算箭头的两个边点
    QVector2D perpendicular(-direction.y(), direction.x());
    QVector2D arrowBase = QVector2D(end) - direction * arrowSize;
    
    QPoint arrowP1 = (arrowBase + perpendicular * (arrowSize / 2)).toPoint();
    QPoint arrow1 = (arrowBase + perpendicular * (arrowSize / 2)).toPoint();
    QPoint arrow2 = (arrowBase - perpendicular * (arrowSize / 2)).toPoint();
    
    // 绘制箭头
    QPolygon arrowHead;
    arrowHead << end << arrow1 << arrow2;
    painter.drawPolygon(arrowHead);
}

void ClockConfigWidget::calculateElbowPoints(const QPoint& start, const QPoint& end, QPoint& elbow1, QPoint& elbow2) const
{
    // 计算肘形连接的两个拐点
    int horizontalOffset = 40;  // 水平延伸距离
    
    // 如果起点在终点左侧，向右延伸；否则向左延伸
    if (start.x() < end.x()) {
        // 第一个拐点：从起点向右延伸
        elbow1 = QPoint(start.x() + horizontalOffset, start.y());
        // 第二个拐点：垂直对齐到目标点
        elbow2 = QPoint(elbow1.x(), end.y());
    } else {
        // 第一个拐点：从起点向左延伸
        elbow1 = QPoint(start.x() - horizontalOffset, start.y());
        // 第二个拐点：垂直对齐到目标点
        elbow2 = QPoint(elbow1.x(), end.y());
    }
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
    
    return oscPoint;
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
    
    return pllPoint;
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
    return QPoint(
        pllAreaPos.x() + pllPos.x() + pllRect.width(),
        pllAreaPos.y() + pllPos.y() + pllRect.height() / 2
    );
}

QPoint ClockConfigWidget::getSubPLLConnectionPoint() const
{
    if (!m_subPllWidget || !m_flowWidget) {
        return QPoint();
    }
    
    // 获取子PLL widget在其父widget中的位置
    QPoint pllPos = m_subPllWidget->pos();
    QRect pllRect = m_subPllWidget->rect();
    
    // 子PLL连接点位于widget的左侧中央
    return QPoint(
        pllPos.x(),
        pllPos.y() + pllRect.height() / 2
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
    return QPoint(
        outputAreaPos.x(),
        outputAreaPos.y() + 20 // 考虑标题栏高度20px
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
    return QPoint(
        outputAreaPos.x() + clk1MPos.x() + clk1MRect.width(),
        outputAreaPos.y() + clk1MPos.y() + clk1MRect.height() / 2
    );
}

QPoint ClockConfigWidget::getClk1MSubNodeConnectionPoint() const
{
    if (!m_clk1MSubNodeWidget || !m_flowWidget) {
        return QPoint();
    }

    // 获取子节点widget在其父widget中的位置
    QPoint subNodePos = m_clk1MSubNodeWidget->pos();
    QRect subNodeRect = m_clk1MSubNodeWidget->rect();

    // 子节点连接点位于widget的左侧中央
    return QPoint(
        subNodePos.x(),
        subNodePos.y() + subNodeRect.height() / 2
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
    return QPoint(
        subPllAreaPos.x() + cam1PLLPos.x() + cam1PLLRect.width(),
        subPllAreaPos.y() + cam1PLLPos.y() + cam1PLLRect.height() / 2
    );
}

QPoint ClockConfigWidget::getClkCam1PLLSubNodeConnectionPoint() const
{
    if (!m_clkCam1PLLSubNodeWidget || !m_flowWidget) {
        return QPoint();
    }

    // 获取子节点widget在其父widget中的位置
    QPoint subNodePos = m_clkCam1PLLSubNodeWidget->pos();
    QRect subNodeRect = m_clkCam1PLLSubNodeWidget->rect();

    // 子节点连接点位于widget的左侧中央
    return QPoint(
        subNodePos.x(),
        subNodePos.y() + subNodeRect.height() / 2
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
    return QPoint(
        cam1PLLSubAreaPos.x() + rawAxiPos.x() + rawAxiRect.width(),
        cam1PLLSubAreaPos.y() + rawAxiPos.y() + rawAxiRect.height() / 2
    );
}

QPoint ClockConfigWidget::getClkRawAxiSubNodeConnectionPoint() const
{
    if (!m_clkRawAxiSubNodeWidget || !m_flowWidget) {
        return QPoint();
    }

    // 获取子节点widget在其父widget中的位置
    QPoint subNodePos = m_clkRawAxiSubNodeWidget->pos();
    QRect subNodeRect = m_clkRawAxiSubNodeWidget->rect();

    // 子节点连接点位于widget的左侧中央
    return QPoint(
        subNodePos.x(),
        subNodePos.y() + subNodeRect.height() / 2
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
    
    return cam0PLLPoint;
}

QPoint ClockConfigWidget::getClkCam0PLLSubNodeConnectionPoint() const
{
    if (!m_clkCam0PLLSubNodeWidget || !m_flowWidget) {
        return QPoint();
    }

    // 获取子节点widget在其父widget中的位置
    QPoint subNodePos = m_clkCam0PLLSubNodeWidget->pos();
    QRect subNodeRect = m_clkCam0PLLSubNodeWidget->rect();

    // 子节点连接点位于widget的左侧中央
    QPoint subNodePoint = QPoint(
        subNodePos.x(),
        subNodePos.y() + subNodeRect.height() / 2
    );

    return subNodePoint;
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
    
    return dispPLLPoint;
}

QPoint ClockConfigWidget::getClkDispPLLSubNodeConnectionPoint() const
{
    if (!m_clkDispPLLSubNodeWidget || !m_flowWidget) {
        return QPoint();
    }

    // 获取子节点widget在其父widget中的位置
    QPoint subNodePos = m_clkDispPLLSubNodeWidget->pos();
    QRect subNodeRect = m_clkDispPLLSubNodeWidget->rect();

    // 子节点连接点位于widget的左侧中央
    QPoint subNodePoint = QPoint(
        subNodePos.x(),
        subNodePos.y() + subNodeRect.height() / 2
    );
    
    return subNodePoint;
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
    
    return sysDispPoint;
}

QPoint ClockConfigWidget::getClkSysDispSubNodeConnectionPoint() const
{
    if (!m_clkSysDispSubNodeWidget || !m_flowWidget) {
        return QPoint();
    }
    
    // 获取子节点widget在其父widget中的位置
    QPoint subNodePos = m_clkSysDispSubNodeWidget->pos();
    QRect subNodeRect = m_clkSysDispSubNodeWidget->rect();

    // 子节点连接点位于widget的左侧中央
    QPoint subNodePoint = QPoint(
        subNodePos.x(),
        subNodePos.y() + subNodeRect.height() / 2
    );
    
    return subNodePoint;
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
    
    return a0pllPoint;
}

QPoint ClockConfigWidget::getClkA0PLLSubNodeConnectionPoint() const
{
    if (!m_clkA0PLLSubNodeWidget || !m_flowWidget) {
        return QPoint();
    }
    
    // 获取子节点widget在其父widget中的位置
    QPoint subNodePos = m_clkA0PLLSubNodeWidget->pos();
    QRect subNodeRect = m_clkA0PLLSubNodeWidget->rect();

    // 子节点连接点位于widget的左侧中央
    QPoint subNodePoint = QPoint(
        subNodePos.x(),
        subNodePos.y() + subNodeRect.height() / 2
    );
    
    return subNodePoint;
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
    
    return rvpllPoint;
}

QPoint ClockConfigWidget::getClkRVPLLSubNodeConnectionPoint() const
{
    if (!m_clkRVPLLSubNodeWidget || !m_flowWidget) {
        return QPoint();
    }
    
    // 获取子节点widget在其父widget中的位置
    QPoint subNodePos = m_clkRVPLLSubNodeWidget->pos();
    QRect subNodeRect = m_clkRVPLLSubNodeWidget->rect();
    
    // 子节点连接点位于widget的左侧中央
    QPoint subNodePoint = QPoint(
        subNodePos.x(),
        subNodePos.y() + subNodeRect.height() / 2
    );
    
    return subNodePoint;
}

QPoint ClockConfigWidget::getClkAPPLLConnectionPoint() const
{
    if (!m_pllWidget || !m_flowWidget || !m_pllWidgets.contains("clk_appll")) {
        return QPoint();
    }

    QWidget* apllWidget = m_pllWidgets["clk_appll"];
    if (!apllWidget) {
        return QPoint();
    }
    
    // 获取clk_appll widget在其父widget中的位置
    QPoint apllPos = apllWidget->pos();
    QRect apllRect = apllWidget->rect();
    
    // 获取子PLL区域在flow widget中的位置
    QPoint subPllAreaPos = m_pllWidget->pos();
    
    // clk_appll连接点位于widget的右侧中央
    QPoint apllPoint = QPoint(
        subPllAreaPos.x() + apllPos.x() + apllRect.width(),
        subPllAreaPos.y() + apllPos.y() + apllRect.height() / 2
    );
    
    return apllPoint;
}

QPoint ClockConfigWidget::getClkAPPLLSubNodeConnectionPoint() const
{
    if (!m_clkAPPLLSubNodeWidget || !m_flowWidget) {
        return QPoint();
    }

    // 获取子节点widget在其父widget中的位置
    QPoint subNodePos = m_clkAPPLLSubNodeWidget->pos();
    QRect subNodeRect = m_clkAPPLLSubNodeWidget->rect();

    // 子节点连接点位于widget的左侧中央
    QPoint subNodePoint = QPoint(
        subNodePos.x(),
        subNodePos.y() + subNodeRect.height() / 2
    );
    
    return subNodePoint;
}

QPoint ClockConfigWidget::getClkFPLLConnectionPoint() const
{
    if (!m_pllWidget || !m_flowWidget || !m_pllWidgets.contains("clk_fpll")) {
        return QPoint();
    }

    QWidget* fpllWidget = m_pllWidgets["clk_fpll"];
    if (!fpllWidget) {
        return QPoint();
    }
    
    // 获取clk_fpll widget在其父widget中的位置
    QPoint fpllPos = fpllWidget->pos();
    QRect fpllRect = fpllWidget->rect();
    
    // 获取子PLL区域在flow widget中的位置
    QPoint subPllAreaPos = m_pllWidget->pos();
    
    // clk_fpll连接点位于widget的右侧中央
    QPoint fpllPoint = QPoint(
        subPllAreaPos.x() + fpllPos.x() + fpllRect.width(),
        subPllAreaPos.y() + fpllPos.y() + fpllRect.height() / 2
    );
    
    return fpllPoint;
}

QPoint ClockConfigWidget::getClkFPLLSubNodeConnectionPoint() const
{
    if (!m_clkFPLLSubNodeWidget || !m_flowWidget) {
        return QPoint();
    }

    // 获取子节点widget在其父widget中的位置
    QPoint subNodePos = m_clkFPLLSubNodeWidget->pos();
    QRect subNodeRect = m_clkFPLLSubNodeWidget->rect();

    // 子节点连接点位于widget的左侧中央
    QPoint subNodePoint = QPoint(
        subNodePos.x(),
        subNodePos.y() + subNodeRect.height() / 2
    );
    
    return subNodePoint;
}

QPoint ClockConfigWidget::getClkTPLLConnectionPoint() const
{
    if (!m_pllWidget || !m_flowWidget || !m_pllWidgets.contains("clk_tpll")) {
        return QPoint();
    }

    QWidget* tpllWidget = m_pllWidgets["clk_tpll"];
    if (!tpllWidget) {
        return QPoint();
    }
    
    // 获取clk_tpll widget在其父widget中的位置
    QPoint tpllPos = tpllWidget->pos();
    QRect tpllRect = tpllWidget->rect();
    
    // 获取子PLL区域在flow widget中的位置
    QPoint subPllAreaPos = m_pllWidget->pos();
    
    // clk_tpll连接点位于widget的右侧中央
    QPoint tpllPoint = QPoint(
        subPllAreaPos.x() + tpllPos.x() + tpllRect.width(),
        subPllAreaPos.y() + tpllPos.y() + tpllRect.height() / 2
    );
    
    return tpllPoint;
}

QPoint ClockConfigWidget::getClkTPLLSubNodeConnectionPoint() const
{
    if (!m_clkTPLLSubNodeWidget || !m_flowWidget) {
        return QPoint();
    }
    
    // 获取子节点widget在其父widget中的位置
    QPoint subNodePos = m_clkTPLLSubNodeWidget->pos();
    QRect subNodeRect = m_clkTPLLSubNodeWidget->rect();

    // 子节点连接点位于widget的左侧中央
    QPoint subNodePoint = QPoint(
        subNodePos.x(),
        subNodePos.y() + subNodeRect.height() / 2
    );
    
    return subNodePoint;
}

QPoint ClockConfigWidget::getClkMPLLConnectionPoint() const
{
    if (!m_pllWidget || !m_flowWidget || !m_pllWidgets.contains("clk_mpll")) {
        return QPoint();
    }

    QWidget* mpllWidget = m_pllWidgets["clk_mpll"];
    if (!mpllWidget) {
        return QPoint();
    }
    
    // 获取clk_mpll widget在其父widget中的位置
    QPoint mpllPos = mpllWidget->pos();
    QRect mpllRect = mpllWidget->rect();
    
    // 获取子PLL区域在flow widget中的位置
    QPoint subPllAreaPos = m_pllWidget->pos();
    
    // clk_mpll连接点位于widget的右侧中央
    QPoint mpllPoint = QPoint(
        subPllAreaPos.x() + mpllPos.x() + mpllRect.width(),
        subPllAreaPos.y() + mpllPos.y() + mpllRect.height() / 2
    );
    
    return mpllPoint;
}

QPoint ClockConfigWidget::getClkMPLLSubNodeConnectionPoint() const
{
    if (!m_clkMPLLSubNodeWidget || !m_flowWidget) {
        return QPoint();
    }

    // 获取子节点widget在其父widget中的位置
    QPoint subNodePos = m_clkMPLLSubNodeWidget->pos();
    QRect subNodeRect = m_clkMPLLSubNodeWidget->rect();

    // 子节点连接点位于widget的左侧中央
    return QPoint(
        subNodePos.x(),
        subNodePos.y() + 20
    );
}

QPoint ClockConfigWidget::getClkFAB100MConnectionPoint() const
{
    if (!m_clkFPLLSubNodeWidget || !m_flowWidget || !m_clkFPLLSubNodeWidgets.contains("clk_fab_100M")) {
        return QPoint();
    }

    QWidget* fab100MWidget = m_clkFPLLSubNodeWidgets["clk_fab_100M"];
    if (!fab100MWidget) {
        return QPoint();
    }
    
    // 获取clk_fab100m widget在其父widget中的位置
    QPoint fab100MPos = fab100MWidget->pos();
    QRect fab100MRect = fab100MWidget->rect();
    
    // 获取子PLL区域在flow widget中的位置
    QPoint subPllAreaPos = m_clkFPLLSubNodeWidget->pos();
    
    // clk_fab100m连接点位于widget的右侧中央
    QPoint fab100MPoint = QPoint(
        subPllAreaPos.x() + fab100MPos.x() + fab100MRect.width(),
        subPllAreaPos.y() + fab100MPos.y() + fab100MRect.height() / 2
    );
    
    return fab100MPoint;
}

QPoint ClockConfigWidget::getClkFAB100MSubNodeConnectionPoint() const
{
    if (!m_clkFAB100MSubNodeWidget || !m_flowWidget) {
        return QPoint();
    }

    // 获取子节点widget在其父widget中的位置
    QPoint subNodePos = m_clkFAB100MSubNodeWidget->pos();
    QRect subNodeRect = m_clkFAB100MSubNodeWidget->rect();

    // 子节点连接点位于widget的左侧中央
    return QPoint(
        subNodePos.x(),
        subNodePos.y() + subNodeRect.height() / 2
    );
}

QPoint ClockConfigWidget::getClkSPINANDConnectionPoint() const
{
    if (!m_clkMPLLSubNodeWidget || !m_flowWidget || !m_clkMPLLSubNodeWidgets.contains("clk_spi_nand")) {
        return QPoint();
    }

    QWidget* spinandWidget = m_clkMPLLSubNodeWidgets["clk_spi_nand"];
    if (!spinandWidget) {
        return QPoint();
    }
    
    // 获取clk_spinand widget在其父widget中的位置
    QPoint spinandPos = spinandWidget->pos();
    QRect spinandRect = spinandWidget->rect();
    
    // 获取子PLL区域在flow widget中的位置
    QPoint subPllAreaPos = m_clkMPLLSubNodeWidget->pos();
    
    // clk_spinand连接点位于widget的右侧中央
    QPoint spinandPoint = QPoint(
        subPllAreaPos.x() + spinandPos.x() + spinandRect.width(),
        subPllAreaPos.y() + spinandPos.y() + spinandRect.height() / 2
    );
    
    return spinandPoint;
}

QPoint ClockConfigWidget::getClkSPINANDSubNodeConnectionPoint() const
{
    if (!m_clkSPINANDSubNodeWidget || !m_flowWidget) {
        return QPoint();
    }

    // 获取子节点widget在其父widget中的位置
    QPoint subNodePos = m_clkSPINANDSubNodeWidget->pos();
    QRect subNodeRect = m_clkSPINANDSubNodeWidget->rect();

    // 子节点连接点位于widget的左侧中央
    QPoint subNodePoint = QPoint(
        subNodePos.x(),
        subNodePos.y() + subNodeRect.height() / 2
    );

    return subNodePoint;
}

QPoint ClockConfigWidget::getClkHSPeriConnectionPoint() const
{
    if (!m_clkMPLLSubNodeWidget || !m_flowWidget || !m_clkMPLLSubNodeWidgets.contains("clk_hsperi")) {
        return QPoint();
    }

    QWidget* hsperiWidget = m_clkMPLLSubNodeWidgets["clk_hsperi"];
    if (!hsperiWidget) {
        return QPoint();
    }
    
    // 获取clk_hsperi widget在其父widget中的位置
    QPoint hsperiPos = hsperiWidget->pos();
    QRect hsperiRect = hsperiWidget->rect();
    
    // 获取子PLL区域在flow widget中的位置
    QPoint subPllAreaPos = m_clkMPLLSubNodeWidget->pos();
    
    // clk_hsperi连接点位于widget的右侧中央
    QPoint hsperiPoint = QPoint(
        subPllAreaPos.x() + hsperiPos.x() + hsperiRect.width(),
        subPllAreaPos.y() + hsperiPos.y() + hsperiRect.height() / 2
    );
    
    return hsperiPoint;
}

QPoint ClockConfigWidget::getClkHSPeriSubNodeConnectionPoint() const
{
    if (!m_clkHSPeriSubNodeWidget || !m_flowWidget) {
        return QPoint();
    }

    // 获取子节点widget在其父widget中的位置
    QPoint subNodePos = m_clkHSPeriSubNodeWidget->pos();
    QRect subNodeRect = m_clkHSPeriSubNodeWidget->rect();

    // 子节点连接点位于widget的左侧中央
    QPoint subNodePoint = QPoint(
        subNodePos.x(),
        subNodePos.y() + subNodeRect.height() / 2
    );

    return subNodePoint;
}

void ClockConfigWidget::updateConnectionOverlay()
{
    if (!m_connectionOverlay || !m_flowWidget || !m_flowScrollArea) {
        return;
    }
    
    // 获取滚动区域的视口大小
    QSize viewportSize = m_flowScrollArea->viewport()->size();
    
    // 获取滚动偏移量
    QPoint scrollOffset = QPoint(
        m_flowScrollArea->horizontalScrollBar()->value(),
        m_flowScrollArea->verticalScrollBar()->value()
    );
    
    // 设置覆盖层的位置和大小，覆盖从滚动偏移开始的视口区域
    m_connectionOverlay->setGeometry(scrollOffset.x(), scrollOffset.y(), 
                                   viewportSize.width(), viewportSize.height());
    m_connectionOverlay->raise();  // 确保覆盖层在最上层
    m_connectionOverlay->update(); // 重绘覆盖层
}

QWidget* ClockConfigWidget::getWidgetAt(const QPoint& pos)
{
    // 检查所有模块widget是否包含该点
    QList<QWidget*> allWidgets;
    allWidgets << m_inputWidget << m_pllWidget << m_subPllWidget << m_outputWidget 
               << m_clk1MSubNodeWidget << m_clkCam1PLLSubNodeWidget << m_clkRawAxiSubNodeWidget
               << m_clkCam0PLLSubNodeWidget << m_clkDispPLLSubNodeWidget << m_clkSysDispSubNodeWidget
               << m_clkA0PLLSubNodeWidget << m_clkRVPLLSubNodeWidget << m_clkAPPLLSubNodeWidget
               << m_clkFPLLSubNodeWidget << m_clkTPLLSubNodeWidget << m_clkMPLLSubNodeWidget
               << m_clkFAB100MSubNodeWidget << m_clkSPINANDSubNodeWidget << m_clkHSPeriSubNodeWidget;
    
    for (QWidget* widget : allWidgets) {
        if (widget && widget->geometry().contains(pos)) {
            return widget;
        }
    }
    
    return nullptr;
}

ResizeDirection ClockConfigWidget::getResizeDirection(const QPoint& pos, const QRect& widgetRect)
{
    int margin = m_handleSize;
    
    // 检查各个缩放手柄区域
    if (getResizeHandleRect(widgetRect, TopLeft).contains(pos)) return TopLeft;
    if (getResizeHandleRect(widgetRect, Top).contains(pos)) return Top;
    if (getResizeHandleRect(widgetRect, TopRight).contains(pos)) return TopRight;
    if (getResizeHandleRect(widgetRect, Right).contains(pos)) return Right;
    if (getResizeHandleRect(widgetRect, BottomRight).contains(pos)) return BottomRight;
    if (getResizeHandleRect(widgetRect, Bottom).contains(pos)) return Bottom;
    if (getResizeHandleRect(widgetRect, BottomLeft).contains(pos)) return BottomLeft;
    if (getResizeHandleRect(widgetRect, Left).contains(pos)) return Left;
    
    return None;
}

void ClockConfigWidget::updateCursor(const QPoint& pos)
{
    QWidget* widget = getWidgetAt(pos);
    if (widget) {
        ResizeDirection dir = getResizeDirection(pos, widget->geometry());
        
        switch (dir) {
            case TopLeft:
            case BottomRight:
                setCursor(Qt::SizeFDiagCursor);
                break;
            case Top:
            case Bottom:
                setCursor(Qt::SizeVerCursor);
                break;
            case TopRight:
            case BottomLeft:
                setCursor(Qt::SizeBDiagCursor);
                break;
            case Left:
            case Right:
                setCursor(Qt::SizeHorCursor);
                break;
            default:
                setCursor(Qt::SizeAllCursor);  // 拖拽光标
                break;
        }
    } else {
        setCursor(Qt::ArrowCursor);
    }
}

void ClockConfigWidget::updateWidgetGeometry(QWidget* widget, const QRect& newGeometry)
{
    if (!widget) return;
    
    widget->setGeometry(newGeometry);
    
    // 更新模块位置配置
    QString moduleName = getWidgetModuleName(widget);
    if (!moduleName.isEmpty()) {
        ModulePosition pos;
        pos.moduleName = moduleName;
        pos.x = newGeometry.x();
        pos.y = newGeometry.y();
        pos.width = newGeometry.width();
        pos.height = newGeometry.height();
        m_modulePositions[moduleName] = pos;
    }
}

QPoint ClockConfigWidget::convertToFlowWidgetCoordinate(const QPoint& pos) const
{
    // 获取flowScrollArea在ClockConfigWidget中的位置
    QPoint scrollAreaPos = m_flowScrollArea->pos();
    
    // 获取滚动偏移量
    QPoint scrollOffset = QPoint(
        m_flowScrollArea->horizontalScrollBar()->value(),
        m_flowScrollArea->verticalScrollBar()->value()
    );
    
    // 转换坐标：从ClockConfigWidget坐标系转换到flowWidget坐标系
    QPoint flowPos = pos - scrollAreaPos + scrollOffset;
    
    return flowPos;
}

QString ClockConfigWidget::getWidgetModuleName(QWidget* widget)
{
    if (widget == m_inputWidget) return "input";
    if (widget == m_pllWidget) return "pll";
    if (widget == m_subPllWidget) return "subpll";
    if (widget == m_outputWidget) return "output";
    if (widget == m_clk1MSubNodeWidget) return "clk_1M";
    if (widget == m_clkCam1PLLSubNodeWidget) return "clk_cam1pll";
    if (widget == m_clkRawAxiSubNodeWidget) return "clk_raw_axi";
    if (widget == m_clkCam0PLLSubNodeWidget) return "clk_cam0pll";
    if (widget == m_clkDispPLLSubNodeWidget) return "clk_disppll";
    if (widget == m_clkSysDispSubNodeWidget) return "clk_sys_disp";
    if (widget == m_clkA0PLLSubNodeWidget) return "clk_a0pll";
    if (widget == m_clkRVPLLSubNodeWidget) return "clk_rvpll";
    if (widget == m_clkAPPLLSubNodeWidget) return "clk_appll";
    if (widget == m_clkFPLLSubNodeWidget) return "clk_fpll";
    if (widget == m_clkTPLLSubNodeWidget) return "clk_tpll";
    if (widget == m_clkMPLLSubNodeWidget) return "clk_mpll";
    if (widget == m_clkFAB100MSubNodeWidget) return "clk_fab_100M";
    if (widget == m_clkSPINANDSubNodeWidget) return "clk_spi_nand";
    if (widget == m_clkHSPeriSubNodeWidget) return "clk_hsperi";
    
    return QString();
}

QWidget* ClockConfigWidget::getWidgetByModuleName(const QString& moduleName)
{
    if (moduleName == "input") return m_inputWidget;
    if (moduleName == "pll") return m_pllWidget;
    if (moduleName == "subpll") return m_subPllWidget;
    if (moduleName == "output") return m_outputWidget;
    if (moduleName == "clk_1M") return m_clk1MSubNodeWidget;
    if (moduleName == "clk_cam1pll") return m_clkCam1PLLSubNodeWidget;
    if (moduleName == "clk_raw_axi") return m_clkRawAxiSubNodeWidget;
    if (moduleName == "clk_cam0pll") return m_clkCam0PLLSubNodeWidget;
    if (moduleName == "clk_disppll") return m_clkDispPLLSubNodeWidget;
    if (moduleName == "clk_sys_disp") return m_clkSysDispSubNodeWidget;
    if (moduleName == "clk_a0pll") return m_clkA0PLLSubNodeWidget;
    if (moduleName == "clk_rvpll") return m_clkRVPLLSubNodeWidget;
    if (moduleName == "clk_appll") return m_clkAPPLLSubNodeWidget;
    if (moduleName == "clk_fpll") return m_clkFPLLSubNodeWidget;
    if (moduleName == "clk_tpll") return m_clkTPLLSubNodeWidget;
    if (moduleName == "clk_mpll") return m_clkMPLLSubNodeWidget;
    if (moduleName == "clk_fab_100M") return m_clkFAB100MSubNodeWidget;
    if (moduleName == "clk_spi_nand") return m_clkSPINANDSubNodeWidget;
    if (moduleName == "clk_hsperi") return m_clkHSPeriSubNodeWidget;
    
    return nullptr;
}

void ClockConfigWidget::drawResizeHandles(QPainter& painter, const QRect& rect)
{
    painter.setPen(QPen(Qt::blue, 2));
    painter.setBrush(QBrush(Qt::blue));
    
    // 绘制8个缩放手柄
    QList<ResizeDirection> directions = {TopLeft, Top, TopRight, Right, BottomRight, Bottom, BottomLeft, Left};
    
    for (ResizeDirection dir : directions) {
        QRect handleRect = getResizeHandleRect(rect, dir);
        painter.drawRect(handleRect);
    }
}

QRect ClockConfigWidget::getResizeHandleRect(const QRect& widgetRect, ResizeDirection direction)
{
    int size = m_handleSize;
    
    switch (direction) {
        case TopLeft:
            return QRect(widgetRect.left() - size/2, widgetRect.top() - size/2, size, size);
        case Top:
            return QRect(widgetRect.left() + widgetRect.width()/2 - size/2, widgetRect.top() - size/2, size, size);
        case TopRight:
            return QRect(widgetRect.right() - size/2, widgetRect.top() - size/2, size, size);
        case Right:
            return QRect(widgetRect.right() - size/2, widgetRect.top() + widgetRect.height()/2 - size/2, size, size);
        case BottomRight:
            return QRect(widgetRect.right() - size/2, widgetRect.bottom() - size/2, size, size);
        case Bottom:
            return QRect(widgetRect.left() + widgetRect.width()/2 - size/2, widgetRect.bottom() - size/2, size, size);
        case BottomLeft:
            return QRect(widgetRect.left() - size/2, widgetRect.bottom() - size/2, size, size);
        case Left:
            return QRect(widgetRect.left() - size/2, widgetRect.top() + widgetRect.height()/2 - size/2, size, size);
        default:
            return QRect();
    }
}
