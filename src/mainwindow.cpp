#include "mainwindow.h"
#include "peripheralconfigdialog.h"
#include "aichatdialog.h"
#include <QApplication>
#include <QScreen>
#include <QTimer>
#include <QLineEdit>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QSplitter>
#include <QMenu>
#include <QAction>
#include <QCheckBox>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_mainLayout(nullptr)
    , m_headerLayout(nullptr)
    , m_controlLayout(nullptr)
    , m_menuBar(nullptr)
    , m_toolsMenu(nullptr)
    , m_aiChatAction(nullptr)
    , m_configTabWidget(nullptr)
    , m_pinoutTab(nullptr)
    , m_clockTab(nullptr)
    , m_memoryTab(nullptr)
    , m_flashTab(nullptr)
    , m_pinoutSplitter(nullptr)
    , m_pinoutConfigPanel(nullptr)
    , m_pinoutConfigLayout(nullptr)
    , m_pinoutConfigTree(nullptr)
    , m_contentWidget(nullptr)
    , m_contentLayout(nullptr)
    , m_titleLabel(nullptr)
    , m_chipComboBox(nullptr)
    , m_startProjectButton(nullptr)
    , m_generateCodeButton(nullptr)
    , m_stackedWidget(nullptr)
    , m_welcomePage(nullptr)
    , m_chipViewPage(nullptr)
    , m_chipContainer(nullptr)
    , m_pinLayout(nullptr)
    , m_clockConfigPage(nullptr)
    , m_memoryConfigPage(nullptr)
    , m_flashConfigPage(nullptr)
    , m_searchLayout(nullptr)
    , m_searchLabel(nullptr)
    , m_searchLineEdit(nullptr)
    , m_blinkTimer(nullptr)
    , m_highlightedPin(nullptr)
    , m_blinkState(false)
    , m_dtsConfig(nullptr)
    , m_aiChatDialog(nullptr)
{
    // 首先显示路径选择对话框
    if (!selectSourcePath()) {
        // 如果用户取消选择路径，关闭应用程序
        QTimer::singleShot(0, this, &QWidget::close);
        return;
    }

    // 显示芯片选型对话框
    if (!selectChipType()) {
        // 如果用户取消选择芯片，关闭应用程序
        QTimer::singleShot(0, this, &QWidget::close);
        return;
    }

    setupUI();

    // 初始化引脚名称映射
    initializePinNameMappings();

    // 初始化设备树配置
    initializeDtsConfig();

    // 设置窗口属性
    setWindowTitle("CviCubeMX - 芯片引脚配置工具");
    setMinimumSize(1024, 768);

    // 居中显示
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
}

MainWindow::~MainWindow()
{
    if (m_dtsConfig) {
        delete m_dtsConfig;
    }
}

void MainWindow::setupUI()
{
    // 设置菜单栏
    setupMenuBar();

    // 创建中央部件
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);

    // 创建主布局
    m_mainLayout = new QVBoxLayout(m_centralWidget);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    // 创建标题
    m_titleLabel = new QLabel("CviCubeMX 芯片引脚配置工具", this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet("font-size: 14px; font-weight: bold; margin: 0px; padding: 6px; color: #2c3e50; background-color: #f8f9fa; border-bottom: 1px solid #dee2e6;");
    m_titleLabel->setMaximumHeight(30);

    // 创建顶部配置标签页
    m_configTabWidget = new QTabWidget(this);
    m_configTabWidget->setStyleSheet(
        "QTabWidget::pane { "
        "border: 1px solid #dee2e6; "
        "background-color: #ffffff; "
        "} "
        "QTabBar::tab { "
        "background-color: #f8f9fa; "
        "border: 1px solid #dee2e6; "
        "padding: 8px 16px; "
        "margin-right: 2px; "
        "} "
        "QTabBar::tab:selected { "
        "background-color: #ffffff; "
        "border-bottom-color: #ffffff; "
        "} "
        "QTabBar::tab:hover { "
        "background-color: #e9ecef; "
        "}"
    );

    // 创建Pinout配置标签页
    setupPinoutTab();

    // 创建时钟配置标签页
    setupClockTab();

    // 创建内存配置标签页
    setupMemoryTab();

    // 创建Flash配置标签页
    setupFlashTab();

    // 添加标签页
    m_configTabWidget->addTab(m_pinoutTab, "芯片引脚配置");
    m_configTabWidget->addTab(m_clockTab, "时钟配置");
    m_configTabWidget->addTab(m_memoryTab, "内存配置");
    m_configTabWidget->addTab(m_flashTab, "Flash分区配置");

    // 添加到主布局
    m_mainLayout->addWidget(m_titleLabel);
    m_mainLayout->addWidget(m_configTabWidget);

    // 连接信号和槽
    connect(m_chipComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onChipSelectionChanged);
    connect(m_startProjectButton, &QPushButton::clicked, this, &MainWindow::onStartProject);
    connect(m_generateCodeButton, &QPushButton::clicked, this, &MainWindow::onGenerateCode);
    connect(m_searchLineEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    connect(m_pinoutConfigTree, &QTreeWidget::itemClicked, this, &MainWindow::onPeripheralItemClicked);
    connect(m_configTabWidget, &QTabWidget::currentChanged, this, &MainWindow::onConfigTabChanged);

    // 连接时钟配置信号
    connect(m_clockConfigPage, &ClockConfigWidget::configChanged, this, &MainWindow::onClockConfigChanged);

    // 连接内存配置信号
    connect(m_memoryConfigPage, &MemoryConfigWidget::configChanged, this, &MainWindow::onMemoryConfigChanged);

    // 连接Flash配置信号
    connect(m_flashConfigPage, &FlashConfigWidget::configChanged, this, &MainWindow::onFlashConfigChanged);
}

void MainWindow::setupMenuBar()
{
    // 创建菜单栏
    m_menuBar = menuBar();

    // 创建工具菜单
    m_toolsMenu = m_menuBar->addMenu("工具(&T)");

    // 添加 AI 对话菜单项
    m_aiChatAction = new QAction("AI 智能助手(&A)", this);
    m_aiChatAction->setShortcut(QKeySequence("Ctrl+A"));
    m_aiChatAction->setStatusTip("打开 AI 智能助手进行问答对话");
    connect(m_aiChatAction, &QAction::triggered, this, &MainWindow::onShowAIChat);

    m_toolsMenu->addAction(m_aiChatAction);

    // 添加分隔符
    m_toolsMenu->addSeparator();

    // 可以在这里添加其他工具菜单项
}

void MainWindow::setupPinoutTab()
{
    // 创建Pinout配置标签页
    m_pinoutTab = new QWidget();

    // 创建分隔器（水平分隔：左侧配置面板，右侧芯片视图）
    m_pinoutSplitter = new QSplitter(Qt::Horizontal, m_pinoutTab);

    // 设置Pinout标签页布局
    QVBoxLayout* pinoutLayout = new QVBoxLayout(m_pinoutTab);
    pinoutLayout->setContentsMargins(0, 0, 0, 0);

    // 创建控制区域
    m_controlLayout = new QHBoxLayout();
    m_controlLayout->setContentsMargins(8, 8, 8, 8);
    m_controlLayout->setSpacing(8);

    // 芯片选型标签
    QLabel *chipLabel = new QLabel("芯片选型:", m_pinoutTab);
    chipLabel->setStyleSheet("font-size: 12px; font-weight: bold; color: #495057;");

    // 芯片选择下拉框
    m_chipComboBox = new QComboBox(m_pinoutTab);
    m_chipComboBox->addItems({
        "请选择芯片型号",
        "cv1840cp_wevb_0015a_spinor",
        "cv1841cp_wevb_0015a_emmc",
        "cv1841cp_wevb_0015a_spinand",
        "cv1841cp_wevb_0015a_spinor",
        "cv1842cp_wevb_0015a_spinand",
        "cv1842cp_wevb_0015a_spinor",
        "cv1842hp_wevb_0014a_emmc",
        "cv1842hp_wevb_0014a_spinand",
        "cv1842hp_wevb_0014a_spinor",
        "cv1843hp_wevb_0014a_emmc",
        "cv1843hp_wevb_0014a_spinand",
        "cv1843hp_wevb_0014a_spinor"
    });
    // 如果已经选择了芯片，设置为当前选择
    if (!m_selectedChip.isEmpty()) {
        int index = m_chipComboBox->findText(m_selectedChip);
        if (index >= 0) {
            m_chipComboBox->setCurrentIndex(index);
        }
    }
    m_chipComboBox->setMinimumWidth(200);
    m_chipComboBox->setMaximumHeight(28);
    m_chipComboBox->setStyleSheet("font-size: 11px; padding: 3px; border: 1px solid #ced4da; border-radius: 3px;");

    // 开始项目按钮
    m_startProjectButton = new QPushButton("开始项目", m_pinoutTab);
    // 如果已经选择了芯片，启用按钮
    m_startProjectButton->setEnabled(!m_selectedChip.isEmpty());
    m_startProjectButton->setMaximumHeight(28);
    m_startProjectButton->setStyleSheet(
        "QPushButton { "
        "background-color: #3498db; "
        "color: white; "
        "border: none; "
        "padding: 8px 16px; "
        "font-size: 12px; "
        "border-radius: 4px; "
        "} "
        "QPushButton:hover { "
        "background-color: #2980b9; "
        "} "
        "QPushButton:disabled { "
        "background-color: #bdc3c7; "
        "}"
    );

    // 生成代码按钮
    m_generateCodeButton = new QPushButton("生成代码", m_pinoutTab);
    m_generateCodeButton->setEnabled(false);
    m_generateCodeButton->setMaximumHeight(28);
    m_generateCodeButton->setStyleSheet(
        "QPushButton { "
        "background-color: #27ae60; "
        "color: white; "
        "border: none; "
        "padding: 8px 16px; "
        "font-size: 12px; "
        "border-radius: 4px; "
        "} "
        "QPushButton:hover { "
        "background-color: #229954; "
        "} "
        "QPushButton:disabled { "
        "background-color: #bdc3c7; "
        "}"
    );

    // 添加到控制布局
    m_controlLayout->addWidget(chipLabel);
    m_controlLayout->addWidget(m_chipComboBox);
    m_controlLayout->addWidget(m_startProjectButton);
    m_controlLayout->addWidget(m_generateCodeButton);

    // // 添加选择路径按钮
    // QPushButton *selectPathButton = new QPushButton("选择源码路径", m_pinoutTab);
    // selectPathButton->setMaximumHeight(28);
    // selectPathButton->setStyleSheet(
    //     "QPushButton { "
    //     "background-color: #e67e22; "
    //     "color: white; "
    //     "border: none; "
    //     "padding: 8px 16px; "
    //     "font-size: 12px; "
    //     "border-radius: 4px; "
    //     "} "
    //     "QPushButton:hover { "
    //     "background-color: #d35400; "
    //     "}"
    // );
    // connect(selectPathButton, &QPushButton::clicked, this, &MainWindow::onSelectSourcePath);
    // m_controlLayout->addWidget(selectPathButton);

    // // 显示当前路径的标签
    // QLabel *pathLabel = new QLabel(QString("当前路径: %1").arg(m_sourcePath), m_pinoutTab);
    // pathLabel->setStyleSheet("font-size: 10px; color: #666; max-width: 300px;");
    // pathLabel->setWordWrap(true);
    // m_controlLayout->addWidget(pathLabel);

    m_controlLayout->addStretch();

    // 创建左侧配置面板
    setupPinoutConfigPanel();

    // 创建右侧内容区域
    m_contentWidget = new QWidget();
    m_contentLayout = new QVBoxLayout(m_contentWidget);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);

    // 创建堆叠窗口部件
    m_stackedWidget = new QStackedWidget();

    // 创建欢迎页面
    m_welcomePage = new QWidget();
    QVBoxLayout *welcomeLayout = new QVBoxLayout(m_welcomePage);
    QLabel *welcomeLabel = new QLabel("请选择芯片型号并点击开始项目", m_welcomePage);
    welcomeLabel->setAlignment(Qt::AlignCenter);
    welcomeLabel->setStyleSheet("font-size: 18px; color: #7f8c8d; margin: 50px;");
    welcomeLayout->addWidget(welcomeLabel);

    // 创建芯片视图页面
    m_chipViewPage = new QScrollArea();
    m_chipViewPage->setWidgetResizable(true);
    m_chipViewPage->setAlignment(Qt::AlignCenter);

    // 添加页面到堆叠窗口
    m_stackedWidget->addWidget(m_welcomePage);
    m_stackedWidget->addWidget(m_chipViewPage);

    // 设置搜索框
    setupSearchBox();

    // 添加到右侧内容布局
    m_contentLayout->addWidget(m_stackedWidget);
    m_contentLayout->addLayout(m_searchLayout);

    // 添加到分隔器
    m_pinoutSplitter->addWidget(m_pinoutConfigPanel);
    m_pinoutSplitter->addWidget(m_contentWidget);

    // 设置分隔器比例（配置面板:主内容 = 1:3）
    m_pinoutSplitter->setStretchFactor(0, 1);
    m_pinoutSplitter->setStretchFactor(1, 3);
    m_pinoutSplitter->setSizes({250, 750});

    // 添加到Pinout标签页布局
    pinoutLayout->addLayout(m_controlLayout);
    pinoutLayout->addWidget(m_pinoutSplitter);
}

void MainWindow::setupPinoutConfigPanel()
{
    // 创建左侧配置面板
    m_pinoutConfigPanel = new QWidget();
    m_pinoutConfigPanel->setMinimumWidth(200);
    m_pinoutConfigPanel->setMaximumWidth(300);
    m_pinoutConfigPanel->setStyleSheet(
        "QWidget { "
        "background-color: #f8f9fa; "
        "border-right: 1px solid #dee2e6; "
        "} "
        "QTreeWidget { "
        "background-color: #ffffff; "
        "border: 1px solid #dee2e6; "
        "border-radius: 4px; "
        "} "
        "QTreeWidget::item { "
        "padding: 5px; "
        "border-bottom: 1px solid #e9ecef; "
        "} "
        "QTreeWidget::item:selected { "
        "background-color: #007bff; "
        "color: white; "
        "} "
        "QTreeWidget::item:hover { "
        "background-color: #e9ecef; "
        "}"
    );

    // 创建配置布局
    m_pinoutConfigLayout = new QVBoxLayout(m_pinoutConfigPanel);
    m_pinoutConfigLayout->setContentsMargins(10, 10, 10, 10);

    // 创建配置面板标题
    QLabel *configTitle = new QLabel("外设配置", m_pinoutConfigPanel);
    configTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #495057; margin-bottom: 10px;");
    configTitle->setAlignment(Qt::AlignCenter);

    // 加载外设状态
    loadPeripheralStates();

    // 创建配置树
    m_pinoutConfigTree = new QTreeWidget(m_pinoutConfigPanel);
    m_pinoutConfigTree->setHeaderHidden(true);
    m_pinoutConfigTree->setRootIsDecorated(true);
    m_pinoutConfigTree->setIndentation(20);

    // 创建外设节点
    QTreeWidgetItem *peripheralItem = new QTreeWidgetItem(m_pinoutConfigTree);
    peripheralItem->setText(0, "外设");
    peripheralItem->setIcon(0, style()->standardIcon(QStyle::SP_DriveHDIcon));
    peripheralItem->setExpanded(true);

    // 创建外设子项
    QStringList peripherals = {"PWM", "I2C", "SPI", "UART", "GPIO", "ADC", "SYSDMA", "WIEGAND", "SPACC", "TRNG", "RTOS_CMDQU", "THERMAL"};
    for (const QString &peripheral : peripherals) {
        QTreeWidgetItem *subItem = new QTreeWidgetItem(peripheralItem);

        // 创建复选框
        QCheckBox *checkBox = new QCheckBox();
        checkBox->setText(peripheral);
        checkBox->setChecked(m_peripheralStates.value(peripheral, false));

        // 连接复选框信号
        connect(checkBox, &QCheckBox::toggled, this, [this, peripheral](bool checked) {
            onPeripheralCheckBoxChanged(peripheral, checked);
        });

        subItem->setIcon(0, style()->standardIcon(QStyle::SP_FileIcon));
        subItem->setData(0, Qt::UserRole, peripheral); // 存储外设类型

        // 将复选框设置为树节点的widget
        m_pinoutConfigTree->setItemWidget(subItem, 0, checkBox);
    }

    // 添加到布局
    m_pinoutConfigLayout->addWidget(configTitle);
    m_pinoutConfigLayout->addWidget(m_pinoutConfigTree);
    m_pinoutConfigLayout->addStretch();
}

void MainWindow::setupClockTab()
{
    // 创建时钟配置标签页
    m_clockTab = new QWidget();

    // 创建时钟配置页面
    m_clockConfigPage = new ClockConfigWidget();

    // 如果已经有源代码路径和芯片类型，设置到时钟配置页面
    if (!m_sourcePath.isEmpty()) {
        m_clockConfigPage->setSourcePath(m_sourcePath);
    }
    if (!m_selectedChip.isEmpty() && m_selectedChip != "请选择芯片型号") {
        m_clockConfigPage->setChipType(m_selectedChip);
    }

    // 设置时钟标签页布局
    QVBoxLayout* clockLayout = new QVBoxLayout(m_clockTab);
    clockLayout->setContentsMargins(0, 0, 0, 0);
    clockLayout->addWidget(m_clockConfigPage);
}

void MainWindow::setupMemoryTab()
{
    // 创建内存配置标签页
    m_memoryTab = new QWidget();

    // 创建内存配置页面
    m_memoryConfigPage = new MemoryConfigWidget();

    // 如果已经有源代码路径和芯片类型，设置到内存配置页面
    if (!m_sourcePath.isEmpty()) {
        m_memoryConfigPage->setSourcePath(m_sourcePath);
    }
    if (!m_selectedChip.isEmpty() && m_selectedChip != "请选择芯片型号") {
        m_memoryConfigPage->setChipType(m_selectedChip);
    }

    // 设置内存标签页布局
    QVBoxLayout* memoryLayout = new QVBoxLayout(m_memoryTab);
    memoryLayout->setContentsMargins(0, 0, 0, 0);
    memoryLayout->addWidget(m_memoryConfigPage);
}

void MainWindow::setupFlashTab()
{
    // 创建Flash配置标签页
    m_flashTab = new QWidget();

    // 创建Flash配置页面
    m_flashConfigPage = new FlashConfigWidget();

    // 如果已经有源代码路径和芯片类型，设置到Flash配置页面
    if (!m_sourcePath.isEmpty()) {
        m_flashConfigPage->setSourcePath(m_sourcePath);
    }
    if (!m_selectedChip.isEmpty() && m_selectedChip != "请选择芯片型号") {
        m_flashConfigPage->setChipType(m_selectedChip);
    }

    // 设置Flash标签页布局
    QVBoxLayout* flashLayout = new QVBoxLayout(m_flashTab);
    flashLayout->setContentsMargins(0, 0, 0, 0);
    flashLayout->addWidget(m_flashConfigPage);
}

void MainWindow::setupSearchBox()
{
    // 创建搜索布局
    m_searchLayout = new QHBoxLayout();

    // 创建搜索标签
    m_searchLabel = new QLabel("搜索管脚:", this);
    m_searchLabel->setStyleSheet("font-size: 14px; font-weight: bold;");

    // 创建搜索输入框
    m_searchLineEdit = new QLineEdit(this);
    m_searchLineEdit->setPlaceholderText("搜索管脚或功能 (例如: A2, UART1_TX, I2C0)");
    m_searchLineEdit->setMaximumWidth(300);
    m_searchLineEdit->setStyleSheet(
        "QLineEdit { "
        "border: 2px solid #bdc3c7; "
        "border-radius: 4px; "
        "padding: 5px; "
        "font-size: 12px; "
        "} "
        "QLineEdit:focus { "
        "border-color: #3498db; "
        "}"
    );

    // 添加到搜索布局
    m_searchLayout->addWidget(m_searchLabel);
    m_searchLayout->addWidget(m_searchLineEdit);
    m_searchLayout->addStretch();

    // 初始化闪烁定时器
    m_blinkTimer = new QTimer(this);
    m_blinkTimer->setInterval(500); // 500ms 闪烁间隔
    connect(m_blinkTimer, &QTimer::timeout, this, &MainWindow::onBlinkTimeout);
}

void MainWindow::onChipSelectionChanged()
{
    QString selectedChip = m_chipComboBox->currentText();
    m_startProjectButton->setEnabled(selectedChip != "请选择芯片型号");
    m_selectedChip = selectedChip;

    // 更新内存配置页面的芯片类型
    if (m_memoryConfigPage && selectedChip != "请选择芯片型号") {
        m_memoryConfigPage->setChipType(selectedChip);
    }

    // 更新时钟配置页面的芯片类型
    if (m_clockConfigPage && selectedChip != "请选择芯片型号") {
        m_clockConfigPage->setChipType(selectedChip);
    }

    // 更新Flash配置页面的芯片类型
    if (m_flashConfigPage && selectedChip != "请选择芯片型号") {
        m_flashConfigPage->setChipType(selectedChip);
    }

    // 重新加载外设状态（基于新选择的芯片类型）
    if (selectedChip != "请选择芯片型号" && !m_sourcePath.isEmpty()) {
        loadPeripheralStates();
        updatePeripheralCheckBoxes();
    }
}

void MainWindow::onStartProject()
{
    if (m_selectedChip.isEmpty() || m_selectedChip == "请选择芯片型号") {
        QMessageBox::warning(this, "警告", "请先选择芯片型号!");
        return;
    }

    // 清空之前的引脚布局
    clearPinLayout();

    // 设置芯片配置
    m_chipConfig.setChipType(m_selectedChip);

    // 创建芯片视图
    setupChipView();

    // 切换到芯片视图页面
    m_stackedWidget->setCurrentWidget(m_chipViewPage);

    // 启用生成代码按钮
    m_generateCodeButton->setEnabled(true);
}

void MainWindow::setupChipView()
{
    // 创建芯片容器
    m_chipContainer = new QWidget();
    m_chipContainer->setMinimumSize(600, 600);
    m_chipContainer->setStyleSheet("background-color: #ecf0f1; border: 2px solid #bdc3c7; border-radius: 8px;");

    // 创建引脚布局
    m_pinLayout = new QGridLayout(m_chipContainer);
    m_pinLayout->setSpacing(2);

    // 根据芯片类型创建不同的布局
    // QFN封装：芯片名称包含 "cp_" (如 cv1840cp_, cv1841cp_, cv1842cp_)
    // BGA封装：芯片名称包含 "hp_" (如 cv1842hp_, cv1843hp_)
    bool isQFN = m_selectedChip.contains("cp_");
    bool isBGA = m_selectedChip.contains("hp_");

    if (isQFN) {
        // QFN封装 - 引脚在外围
        createQFNLayout();
    } else if (isBGA) {
        // BGA封装 - 引脚在内部
        createBGALayout();
    }

    // 设置芯片视图页面的内容
    m_chipViewPage->setWidget(m_chipContainer);
}

void MainWindow::createQFNLayout()
{
    // QFN封装：引脚在正方形外围四周
    int totalPins = m_chipConfig.getPinCount();
    int pinsPerSide = totalPins / 4;

    // 根据每边引脚数量动态调整网格大小
    // 为了保证引脚排列整齐，网格大小应该比每边引脚数多2（留出边缘空间）
    int gridSize = pinsPerSide + 2;

    // 清空引脚部件映射
    m_pinWidgets.clear();

    int pinNumber = 1; // QFN从1号引脚开始

    // QFN引脚编号：从左侧第一个引脚开始，逆时针方向
    // 1. 左侧引脚（从上到下）
    for (int i = 1; i < gridSize - 1; i++) {
        if (pinNumber <= totalPins) {
            QString pinName = QString("%1").arg(pinNumber);

            // 检查是否有映射关系
            bool hasMappingRelation = m_pinNameMappings.contains(pinName);

            PinWidget *pinWidget = new PinWidget(pinName, true, this); // true表示方形

            // QFN的显示名称就是引脚编号，所以设置为相同
            pinWidget->setDisplayName(pinName);

            // 如果没有映射关系，禁用该引脚
            if (!hasMappingRelation) {
                pinWidget->setEnabled(false);
                pinWidget->setToolTip(QString("%1 - 未映射引脚（禁用）").arg(pinName));
            }

            connect(pinWidget, &PinWidget::functionChanged, this, &MainWindow::onPinFunctionChanged);
            m_pinLayout->addWidget(pinWidget, i, 0);
            m_pinWidgets[pinName] = pinWidget;
            pinNumber++;
        }
    }

    // 2. 底部引脚（从左到右）
    for (int i = 1; i < gridSize - 1; i++) {
        if (pinNumber <= totalPins) {
            QString pinName = QString("%1").arg(pinNumber);

            // 检查是否有映射关系
            bool hasMappingRelation = m_pinNameMappings.contains(pinName);

            PinWidget *pinWidget = new PinWidget(pinName, true, this);

            // QFN的显示名称就是引脚编号，所以设置为相同
            pinWidget->setDisplayName(pinName);

            // 如果没有映射关系，禁用该引脚
            if (!hasMappingRelation) {
                pinWidget->setEnabled(false);
                pinWidget->setToolTip(QString("%1 - 未映射引脚（禁用）").arg(pinName));
            }

            connect(pinWidget, &PinWidget::functionChanged, this, &MainWindow::onPinFunctionChanged);
            m_pinLayout->addWidget(pinWidget, gridSize - 1, i);
            m_pinWidgets[pinName] = pinWidget;
            pinNumber++;
        }
    }

    // 3. 右侧引脚（从下到上）
    for (int i = gridSize - 2; i > 0; i--) {
        if (pinNumber <= totalPins) {
            QString pinName = QString("%1").arg(pinNumber);

            // 检查是否有映射关系
            bool hasMappingRelation = m_pinNameMappings.contains(pinName);

            PinWidget *pinWidget = new PinWidget(pinName, true, this);

            // QFN的显示名称就是引脚编号，所以设置为相同
            pinWidget->setDisplayName(pinName);

            // 如果没有映射关系，禁用该引脚
            if (!hasMappingRelation) {
                pinWidget->setEnabled(false);
                pinWidget->setToolTip(QString("%1 - 未映射引脚（禁用）").arg(pinName));
            }

            connect(pinWidget, &PinWidget::functionChanged, this, &MainWindow::onPinFunctionChanged);
            m_pinLayout->addWidget(pinWidget, i, gridSize - 1);
            m_pinWidgets[pinName] = pinWidget;
            pinNumber++;
        }
    }

    // 4. 顶部引脚（从右到左）
    for (int i = gridSize - 2; i > 0; i--) {
        if (pinNumber <= totalPins) {
            QString pinName = QString("%1").arg(pinNumber);

            // 检查是否有映射关系
            bool hasMappingRelation = m_pinNameMappings.contains(pinName);

            PinWidget *pinWidget = new PinWidget(pinName, true, this);

            // QFN的显示名称就是引脚编号，所以设置为相同
            pinWidget->setDisplayName(pinName);

            // 如果没有映射关系，禁用该引脚
            if (!hasMappingRelation) {
                pinWidget->setEnabled(false);
                pinWidget->setToolTip(QString("%1 - 未映射引脚（禁用）").arg(pinName));
            }

            connect(pinWidget, &PinWidget::functionChanged, this, &MainWindow::onPinFunctionChanged);
            m_pinLayout->addWidget(pinWidget, 0, i);
            m_pinWidgets[pinName] = pinWidget;
            pinNumber++;
        }
    }
}

void MainWindow::createBGALayout()
{
    // BGA封装：引脚在正方形内部
    // 使用固定的15x15网格（跳过I行）
    int bgaSize = 15;

    // 外部网格大小，留出边缘空间
    int gridSize = bgaSize + 4;

    // 清空引脚部件映射
    m_pinWidgets.clear();

    // BGA引脚命名系统：行号A-R（跳过I），列号1-15
    const QStringList rowLabels = {"A", "B", "C", "D", "E", "F", "G", "H", "J",
                                   "K", "L", "M", "N", "P", "R"};

    int startRow = (gridSize - bgaSize) / 2;
    int startCol = (gridSize - bgaSize) / 2;

    // 在内部区域创建BGA引脚
    for (int row = 0; row < bgaSize; row++) {
        for (int col = 0; col < bgaSize; col++) {
            // 跳过四个角的引脚位置
            bool isTopLeft = (row == 0 && col == 0);
            bool isTopRight = (row == 0 && col == bgaSize - 1);
            bool isBottomLeft = (row == bgaSize - 1 && col == 0);
            bool isBottomRight = (row == bgaSize - 1 && col == bgaSize - 1);

            if (isTopLeft || isTopRight || isBottomLeft || isBottomRight) {
                continue; // 跳过四个角的位置
            }

            // 生成BGA引脚名称，如A1, A2, B1, B2等
            QString rowLabel = (row < rowLabels.size()) ? rowLabels[row] : QString("R%1").arg(row);
            QString bgaPosition = QString("%1%2").arg(rowLabel).arg(col + 1);

            // 检查是否有映射关系
            bool hasMappingRelation = m_pinNameMappings.contains(bgaPosition);

            // 映射到实际的PAD名称
            QString actualPinName = mapPinName(bgaPosition);

            PinWidget *pinWidget = new PinWidget(actualPinName, false, this); // false表示圆形

            // 设置显示名称为BGA位置
            pinWidget->setDisplayName(bgaPosition);

            // 如果没有映射关系，禁用该引脚
            if (!hasMappingRelation) {
                pinWidget->setEnabled(false);
                // 设置特殊的工具提示
                pinWidget->setToolTip(QString("%1 - 未映射引脚（禁用）").arg(bgaPosition));
            }

            connect(pinWidget, &PinWidget::functionChanged, this, &MainWindow::onPinFunctionChanged);
            m_pinLayout->addWidget(pinWidget, startRow + row, startCol + col);

            // 使用BGA位置作为key，但引脚显示实际名称
            m_pinWidgets[bgaPosition] = pinWidget;
        }
    }
}

void MainWindow::clearPinLayout()
{
    if (m_pinLayout) {
        // 清空所有引脚部件
        for (auto it = m_pinWidgets.begin(); it != m_pinWidgets.end(); ++it) {
            it.value()->deleteLater();
        }
        m_pinWidgets.clear();

        // 清空布局
        QLayoutItem *item;
        while ((item = m_pinLayout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }

        if (m_chipContainer) {
            m_chipContainer->deleteLater();
            m_chipContainer = nullptr;
        }
    }
}

void MainWindow::onPinFunctionChanged(const QString& pinName, const QString& function)
{
    // 更新引脚功能映射
    m_chipConfig.setPinFunction(pinName, function);

    // 检查是否有高亮的引脚被配置了
    bool shouldClearHighlight = false;
    
    // 如果当前正在高亮这个引脚，且功能不是默认GPIO，则取消高亮
    if (m_highlightedPin &&
        (m_highlightedPin->getPinName() == pinName || m_highlightedPin->getDisplayName() == pinName) &&
        function != "GPIO") {
        shouldClearHighlight = true;
    }
    
    // 检查多个高亮引脚中是否有被配置的
    for (PinWidget* pin : m_highlightedPins) {
        if (pin && (pin->getPinName() == pinName || pin->getDisplayName() == pinName) &&
            function != "GPIO") {
            shouldClearHighlight = true;
            break;
        }
    }

    if (shouldClearHighlight) {
        // 停止闪烁并清除所有高亮
        m_blinkTimer->stop();
        
        if (m_highlightedPin) {
            m_highlightedPin->setHighlight(false);
            m_highlightedPin = nullptr;
        }
        
        for (PinWidget* pin : m_highlightedPins) {
            if (pin) {
                pin->setHighlight(false);
            }
        }
        m_highlightedPins.clear();

        // 清空搜索框
        m_searchLineEdit->clear();
    }
}

void MainWindow::onGenerateCode()
{
    // 首先保存DTS配置
    if (m_dtsConfig && m_dtsConfig->saveDtsFile()) {
        qDebug() << "DTS配置已保存";
    } else {
        qDebug() << "DTS配置保存失败或未加载DTS文件";
    }

    // 直接更新或生成代码到默认位置
    QString result = m_codeGenerator.generateCode(m_chipConfig);

    // 检查结果是否为错误信息
    if (result.startsWith("Error:")) {
        QMessageBox::critical(this, "错误", result);
    } else if (result == "File updated successfully") {
        QMessageBox::information(this, "成功", "已成功更新 cvi_board_init.c 文件的 PINMUX 配置！");
    } else if (result == "Existing generated configurations removed") {
        QMessageBox::information(this, "信息", "已移除原有的生成配置（当前无新配置需要添加）。");
    } else if (result == "No pin configurations to add") {
        QMessageBox::information(this, "信息", "当前没有引脚配置需要生成。");
    } else {
        // 如果返回的是生成的代码内容（文件不存在的情况）
        QString fileName = QFileDialog::getSaveFileName(this,
            "保存代码文件",
            "cvi_board_init.c",
            "C Files (*.c);;All Files (*)");

        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream stream(&file);
                stream << result;
                file.close();
                QMessageBox::information(this, "成功", "代码文件已生成: " + fileName);
            } else {
                QMessageBox::critical(this, "错误", "无法保存文件!");
            }
        }
    }
}

void MainWindow::initializePinNameMappings()
{
    // 初始化BGA/QFN位置到PAD名称的映射表
    // 只有在这里定义的引脚才会被启用，其他引脚会被禁用并显示×标记

    // BGA引脚映射（字母数字格式）
    m_pinNameMappings["A2"] = "PAD_MIPI_TXM4";
    m_pinNameMappings["A4"] = "PAD_MIPIRX0N";
    m_pinNameMappings["A6"] = "PAD_MIPIRX3P";
    m_pinNameMappings["A7"] = "PAD_MIPIRX4P";
    m_pinNameMappings["A9"] = "VIVO_D2";
    m_pinNameMappings["A10"] = "VIVO_D3";
    m_pinNameMappings["A12"] = "VIVO_D10";
    m_pinNameMappings["A13"] = "USB_VBUS_DET";
    m_pinNameMappings["B1"] = "PAD_MIPI_TXP3";
    m_pinNameMappings["B2"] = "PAD_MIPI_TXM3";
    m_pinNameMappings["B3"] = "PAD_MIPI_TXP4";
    m_pinNameMappings["B4"] = "PAD_MIPIRX0P";
    m_pinNameMappings["B5"] = "PAD_MIPIRX1N";
    m_pinNameMappings["B6"] = "PAD_MIPIRX2N";
    m_pinNameMappings["B7"] = "PAD_MIPIRX4N";
    m_pinNameMappings["B8"] = "PAD_MIPIRX5N";
    m_pinNameMappings["B9"] = "VIVO_D1";
    m_pinNameMappings["B10"] = "VIVO_D5";
    m_pinNameMappings["B11"] = "VIVO_D7";
    m_pinNameMappings["B12"] = "VIVO_D9";
    m_pinNameMappings["B13"] = "USB_ID";
    m_pinNameMappings["B15"] = "PAD_ETH_RXM";
    m_pinNameMappings["C1"] = "PAD_MIPI_TXP2";
    m_pinNameMappings["C2"] = "PAD_MIPI_TXM2";
    m_pinNameMappings["C3"] = "CAM_PD0";
    m_pinNameMappings["C4"] = "CAM_MCLK0";
    m_pinNameMappings["C5"] = "PAD_MIPIRX1P";
    m_pinNameMappings["C6"] = "PAD_MIPIRX2P";
    m_pinNameMappings["C7"] = "PAD_MIPIRX3N";
    m_pinNameMappings["C8"] = "PAD_MIPIRX5P";
    m_pinNameMappings["C9"] = "VIVO_CLK";
    m_pinNameMappings["C10"] = "VIVO_D6";
    m_pinNameMappings["C11"] = "VIVO_D8";
    m_pinNameMappings["C12"] = "USB_VBUS_EN";
    m_pinNameMappings["C14"] = "PAD_ETH_RXP";
    m_pinNameMappings["C15"] = "GPIO_RTX";
    m_pinNameMappings["D1"] = "PAD_MIPI_TXP1";
    m_pinNameMappings["D2"] = "PAD_MIPI_TXM1";
    m_pinNameMappings["D3"] = "CAM_MCLK1";
	m_pinNameMappings["D4"] = "IIC3_SCL";
    m_pinNameMappings["D10"] = "VIVO_D4";
    m_pinNameMappings["D14"] = "PAD_ETH_TXM";
    m_pinNameMappings["D15"] = "PAD_ETH_TXP";
    m_pinNameMappings["E1"] = "PAD_MIPI_TXP0";
    m_pinNameMappings["E2"] = "PAD_MIPI_TXM0";
    m_pinNameMappings["E4"] = "CAM_PD1";
    m_pinNameMappings["E5"] = "CAM_RST0";
    m_pinNameMappings["E10"] = "VIVO_D0";
    m_pinNameMappings["E13"] = "ADC1";
    m_pinNameMappings["E14"] = "ADC2";
    m_pinNameMappings["E15"] = "ADC3";
    m_pinNameMappings["F2"] = "PAD_AUD_AOUTL";
    m_pinNameMappings["F4"] = "IIC3_SDA";
    m_pinNameMappings["F14"] = "SD1_D2";
    m_pinNameMappings["G2"] = "PAD_AUD_AOUTR";
    m_pinNameMappings["G13"] = "SD1_D3";
    m_pinNameMappings["G14"] = "SD1_CLK";
    m_pinNameMappings["G15"] = "SD1_CMD";
    m_pinNameMappings["H1"] = "PAD_AUD_AINL_MIC";
    m_pinNameMappings["H12"] = "RSTN";
    m_pinNameMappings["H13"] = "PWM0_BUCK";
    m_pinNameMappings["H14"] = "SD1_D1";
    m_pinNameMappings["H15"] = "SD1_D0";
    m_pinNameMappings["J1"] = "PAD_AUD_AINR_MIC";
    m_pinNameMappings["J13"] = "IIC2_SCL";
    m_pinNameMappings["J14"] = "IIC2_SDA";
    m_pinNameMappings["K2"] = "SD0_CD";
    m_pinNameMappings["K3"] = "SD0_D1";
    m_pinNameMappings["K13"] = "UART2_RX";
    m_pinNameMappings["K14"] = "UART2_CTS";
    m_pinNameMappings["K15"] = "UART2_TX";
    m_pinNameMappings["L1"] = "SD0_CLK";
    m_pinNameMappings["L2"] = "SD0_D0";
    m_pinNameMappings["L3"] = "SD0_CMD";
    m_pinNameMappings["L14"] = "CLK32K";
    m_pinNameMappings["L15"] = "UART2_RTS";
    m_pinNameMappings["M1"] = "SD0_D3";
    m_pinNameMappings["M2"] = "SD0_D2";
    m_pinNameMappings["M4"] = "UART0_RX";
    m_pinNameMappings["M5"] = "UART0_TX";
    m_pinNameMappings["M6"] = "JTAG_CPU_TRST";
    m_pinNameMappings["M11"] = "PWR_ON";
    m_pinNameMappings["M12"] = "PWR_GPIO2";
    m_pinNameMappings["M13"] = "PWR_GPIO0";
    m_pinNameMappings["M14"] = "CLK25M";
    m_pinNameMappings["N1"] = "SD0_PWR_EN";
    m_pinNameMappings["N3"] = "SPK_EN";
    m_pinNameMappings["N4"] = "JTAG_CPU_TCK";
    m_pinNameMappings["N6"] = "JTAG_CPU_TMS";
    m_pinNameMappings["N11"] = "PWR_WAKEUP1";
    m_pinNameMappings["N12"] = "PWR_WAKEUP0";
    m_pinNameMappings["N13"] = "PWR_GPIO1";
    m_pinNameMappings["P1"] = "EMMC_DAT3";
    m_pinNameMappings["P2"] = "EMMC_DAT0";
    m_pinNameMappings["P3"] = "EMMC_DAT2";
    m_pinNameMappings["P4"] = "EMMC_RSTN";
    m_pinNameMappings["P5"] = "AUX0";
    m_pinNameMappings["P6"] = "IIC0_SDA";
    m_pinNameMappings["P10"] = "PWR_SEQ3";
    m_pinNameMappings["P11"] = "PWR_VBAT_DET";
    m_pinNameMappings["P12"] = "PWR_SEQ1";
    m_pinNameMappings["P13"] = "PWR_BUTTON1";
    m_pinNameMappings["R2"] = "EMMC_DAT1";
    m_pinNameMappings["R3"] = "EMMC_CMD";
    m_pinNameMappings["R4"] = "EMMC_CLK";
    m_pinNameMappings["R6"] = "IIC0_SCL";
    m_pinNameMappings["R10"] = "GPIO_ZQ";
    m_pinNameMappings["R11"] = "PWR_RSTN";
    m_pinNameMappings["R12"] = "PWR_SEQ2";
    m_pinNameMappings["R13"] = "XTAL_XIN";

    // QFN引脚映射（数字格式）
    m_pinNameMappings["1"] = "PAD_UART0_TX";
    m_pinNameMappings["2"] = "PAD_UART0_RX";
    m_pinNameMappings["5"] = "PAD_I2C0_SCL";
    m_pinNameMappings["6"] = "PAD_I2C0_SDA";
    m_pinNameMappings["10"] = "PAD_SPI0_CLK";
    m_pinNameMappings["11"] = "PAD_SPI0_MOSI";
    m_pinNameMappings["12"] = "PAD_SPI0_MISO";
    m_pinNameMappings["15"] = "PAD_PWM0";
    m_pinNameMappings["16"] = "PAD_PWM1";
    // 可以在这里添加更多引脚映射
    // BGA格式：
    // m_pinNameMappings["B3"] = "PAD_MIPI_TXP4";
    // m_pinNameMappings["D5"] = "PAD_I2C1_SCL";
    // m_pinNameMappings["E6"] = "PAD_I2C1_SDA";
    // QFN格式：
    // m_pinNameMappings["20"] = "PAD_GPIO0";
    // m_pinNameMappings["21"] = "PAD_GPIO1";
    // 等等...
}

QString MainWindow::mapPinName(const QString& bgaPosition) const
{
    // 如果存在映射关系，返回映射的PAD名称
    // 否则返回原始的BGA位置名称
    return m_pinNameMappings.value(bgaPosition, bgaPosition);
}

void MainWindow::onSearchTextChanged(const QString& text)
{
    // 清除之前的高亮
    if (m_highlightedPin) {
        m_highlightedPin->setHighlight(false);
        m_highlightedPin = nullptr;
    }
    
    // 清除所有之前高亮的引脚
    for (PinWidget* pin : m_highlightedPins) {
        if (pin) {
            pin->setHighlight(false);
        }
    }
    m_highlightedPins.clear();

    // 停止闪烁定时器
    m_blinkTimer->stop();

    // 更新当前搜索文本
    m_currentSearchText = text.trimmed();

    // 如果搜索文本为空，直接返回
    if (m_currentSearchText.isEmpty()) {
        return;
    }

    // 查找匹配的引脚
    QList<PinWidget*> matchedPins;

    // 在引脚部件映射中查找
    for (auto it = m_pinWidgets.begin(); it != m_pinWidgets.end(); ++it) {
        const QString& pinKey = it.key();
        PinWidget* pinWidget = it.value();

        // 如果引脚被禁用，跳过
        if (!pinWidget->isEnabled()) {
            continue;
        }

        bool isMatched = false;

        // 1. 检查是否匹配引脚号或显示名称
        if (pinKey.contains(m_currentSearchText, Qt::CaseInsensitive) ||
            pinWidget->getDisplayName().contains(m_currentSearchText, Qt::CaseInsensitive)) {
            isMatched = true;
        }

        // 2. 检查是否匹配引脚支持的功能
        if (!isMatched) {
            QStringList supportedFunctions = pinWidget->getSupportedFunctions();
            for (const QString& func : supportedFunctions) {
                if (func.contains(m_currentSearchText, Qt::CaseInsensitive)) {
                    isMatched = true;
                    break;
                }
            }
        }

        if (isMatched) {
            matchedPins.append(pinWidget);
        }
    }

    // 如果找到匹配的引脚
    if (!matchedPins.isEmpty()) {
        m_highlightedPins = matchedPins;
        
        // 开始闪烁效果
        m_blinkState = true;
        for (PinWidget* pin : m_highlightedPins) {
            pin->setHighlight(true, m_blinkState);
        }
        
        // 为了兼容旧代码，设置第一个匹配的引脚为主高亮引脚
        m_highlightedPin = matchedPins.first();
        
        m_blinkTimer->start();
        
        // 在状态栏或标题显示找到的引脚数量
        if (matchedPins.size() > 1) {
            qDebug() << "找到" << matchedPins.size() << "个支持" << m_currentSearchText << "的引脚";
        }
    }
}

void MainWindow::onBlinkTimeout()
{
    // 切换闪烁状态
    m_blinkState = !m_blinkState;
    
    // 更新所有高亮引脚的闪烁状态
    for (PinWidget* pin : m_highlightedPins) {
        if (pin) {
            pin->setHighlight(true, m_blinkState);
        }
    }
    
    // 为了兼容旧代码，也更新主高亮引脚
    if (m_highlightedPin) {
        m_highlightedPin->setHighlight(true, m_blinkState);
    }
}

void MainWindow::highlightPin(const QString& pinName, bool highlight)
{
    if (m_pinWidgets.contains(pinName)) {
        PinWidget* pin = m_pinWidgets[pinName];
        pin->setHighlight(highlight);
    }
}

/**
 * 处理外设项点击事件
 * 如果点击的是父节点（外设），则展开或折叠子项
 * 如果点击的是子项，则显示外设配置菜单
 */
// void MainWindow::onPeripheralItemClicked(QTreeWidgetItem* item, int column)
// {
//     Q_UNUSED(column)

//     if (!item) return;

//     // 检查是否点击的是父节点（外设）
//     if (item->text(0) == "外设") {
//         // 如果是父节点，展开或折叠
//         item->setExpanded(!item->isExpanded());
//         return;
//     }

//     // 对于子项，现在包含复选框，获取外设类型需要从复选框获取
//     QCheckBox *checkBox = qobject_cast<QCheckBox*>(m_configTree->itemWidget(item, 0));
//     if (!checkBox) return;

//     QString peripheralType = checkBox->text();

//     if (peripheralType.isEmpty()) {
//         return;
//     }

//     // 显示外设配置菜单
//     QMenu contextMenu(this);
//     contextMenu.setStyleSheet(
//         "QMenu { "
//         "background-color: #ffffff; "
//         "border: 1px solid #dee2e6; "
//         "border-radius: 4px; "
//         "padding: 5px; "
//         "} "
//         "QMenu::item { "
//         "padding: 8px 16px; "
//         "} "
//         "QMenu::item:selected { "
//         "background-color: #007bff; "
//         "color: white; "
//         "}"
//     );

//     // 根据外设类型添加不同的选项
//     if (peripheralType == "PWM") {
//         QAction *pwm0Action = contextMenu.addAction("PWM0");
//         QAction *pwm1Action = contextMenu.addAction("PWM1");
//         QAction *pwm2Action = contextMenu.addAction("PWM2");
//         QAction *pwm3Action = contextMenu.addAction("PWM3");

//         pwm0Action->setData("PWM0");
//         pwm1Action->setData("PWM1");
//         pwm2Action->setData("PWM2");
//         pwm3Action->setData("PWM3");

//         connect(pwm0Action, &QAction::triggered, this, &MainWindow::onPeripheralActionTriggered);
//         connect(pwm1Action, &QAction::triggered, this, &MainWindow::onPeripheralActionTriggered);
//         connect(pwm2Action, &QAction::triggered, this, &MainWindow::onPeripheralActionTriggered);
//         connect(pwm3Action, &QAction::triggered, this, &MainWindow::onPeripheralActionTriggered);
//     }
//     else if (peripheralType == "I2C") {
//         QAction *i2c0Action = contextMenu.addAction("I2C0");
//         QAction *i2c1Action = contextMenu.addAction("I2C1");
//         QAction *i2c2Action = contextMenu.addAction("I2C2");

//         i2c0Action->setData("I2C0");
//         i2c1Action->setData("I2C1");
//         i2c2Action->setData("I2C2");

//         connect(i2c0Action, &QAction::triggered, this, &MainWindow::onPeripheralActionTriggered);
//         connect(i2c1Action, &QAction::triggered, this, &MainWindow::onPeripheralActionTriggered);
//         connect(i2c2Action, &QAction::triggered, this, &MainWindow::onPeripheralActionTriggered);
//     }
//     else if (peripheralType == "SPI") {
//         QAction *spi0Action = contextMenu.addAction("SPI0");
//         QAction *spi1Action = contextMenu.addAction("SPI1");
//         QAction *spi2Action = contextMenu.addAction("SPI2");

//         spi0Action->setData("SPI0");
//         spi1Action->setData("SPI1");
//         spi2Action->setData("SPI2");

//         connect(spi0Action, &QAction::triggered, this, &MainWindow::onPeripheralActionTriggered);
//         connect(spi1Action, &QAction::triggered, this, &MainWindow::onPeripheralActionTriggered);
//         connect(spi2Action, &QAction::triggered, this, &MainWindow::onPeripheralActionTriggered);
//     }
//     else if (peripheralType == "UART") {
//         QAction *uart0Action = contextMenu.addAction("UART0");
//         QAction *uart1Action = contextMenu.addAction("UART1");
//         QAction *uart2Action = contextMenu.addAction("UART2");
//         QAction *uart3Action = contextMenu.addAction("UART3");

//         uart0Action->setData("UART0");
//         uart1Action->setData("UART1");
//         uart2Action->setData("UART2");
//         uart3Action->setData("UART3");

//         connect(uart0Action, &QAction::triggered, this, &MainWindow::onPeripheralActionTriggered);
//         connect(uart1Action, &QAction::triggered, this, &MainWindow::onPeripheralActionTriggered);
//         connect(uart2Action, &QAction::triggered, this, &MainWindow::onPeripheralActionTriggered);
//         connect(uart3Action, &QAction::triggered, this, &MainWindow::onPeripheralActionTriggered);
//     }
//     else if (peripheralType == "GPIO") {
//         QAction *gpioAction = contextMenu.addAction("配置GPIO");
//         gpioAction->setData("GPIO");
//         connect(gpioAction, &QAction::triggered, this, &MainWindow::onPeripheralActionTriggered);
//     }
//     else if (peripheralType == "ADC") {
//         QAction *adc0Action = contextMenu.addAction("ADC0");
//         QAction *adc1Action = contextMenu.addAction("ADC1");

//         adc0Action->setData("ADC0");
//         adc1Action->setData("ADC1");

//         connect(adc0Action, &QAction::triggered, this, &MainWindow::onPeripheralActionTriggered);
//         connect(adc1Action, &QAction::triggered, this, &MainWindow::onPeripheralActionTriggered);
//     }
//     else if (peripheralType == "Timer") {
//         QAction *timer0Action = contextMenu.addAction("Timer0");
//         QAction *timer1Action = contextMenu.addAction("Timer1");
//         QAction *timer2Action = contextMenu.addAction("Timer2");

//         timer0Action->setData("Timer0");
//         timer1Action->setData("Timer1");
//         timer2Action->setData("Timer2");

//         connect(timer0Action, &QAction::triggered, this, &MainWindow::onPeripheralActionTriggered);
//         connect(timer1Action, &QAction::triggered, this, &MainWindow::onPeripheralActionTriggered);
//         connect(timer2Action, &QAction::triggered, this, &MainWindow::onPeripheralActionTriggered);
//     }

//     // 在鼠标位置显示菜单
//     QPoint globalPos = m_configTree->mapToGlobal(m_configTree->visualItemRect(item).center());
//     contextMenu.exec(globalPos);
// }

// void MainWindow::onPeripheralActionTriggered()
// {
//     QAction *action = qobject_cast<QAction*>(sender());
//     if (!action) return;

//     QString peripheralFunction = action->data().toString();

//     // 提取外设类型（例如从 "PWM0" 提取 "PWM"）
//     QString peripheralType;
//     if (peripheralFunction.startsWith("PWM")) {
//         peripheralType = "PWM";
//     } else if (peripheralFunction.startsWith("I2C")) {
//         peripheralType = "I2C";
//     } else if (peripheralFunction.startsWith("SPI")) {
//         peripheralType = "SPI";
//     } else if (peripheralFunction.startsWith("UART")) {
//         peripheralType = "UART";
//     } else if (peripheralFunction.startsWith("GPIO")) {
//         peripheralType = "GPIO";
//     } else if (peripheralFunction.startsWith("ADC")) {
//         peripheralType = "ADC";
//     } else {
//         QMessageBox::information(this, "外设配置",
//             QString("您选择了：%1\n这里可以添加具体的外设配置功能").arg(peripheralFunction));
//         return;
//     }

//     // 显示外设配置对话框
//     showPeripheralConfig(peripheralType);
// }

void MainWindow::onPeripheralItemClicked(QTreeWidgetItem* item, int column)
{
    Q_UNUSED(column)

    if (!item) return;

    // 检查是否点击的是父节点（外设）
    if (item->text(0) == "外设") {
        // 如果是父节点，展开或折叠
        item->setExpanded(!item->isExpanded());
        return;
    }

    // 对于子项，现在包含复选框，获取外设类型需要从复选框获取
    QCheckBox *checkBox = qobject_cast<QCheckBox*>(m_pinoutConfigTree->itemWidget(item, 0));
    if (!checkBox) return;

    QString peripheralType = checkBox->text();

    if (peripheralType.isEmpty()) {
        return;
    }

    // 直接显示外设配置对话框，不需要二级菜单选择
    showPeripheralConfig(peripheralType);
}

void MainWindow::onPeripheralCheckBoxChanged(const QString& peripheral, bool enabled)
{
    // 更新状态
    m_peripheralStates[peripheral] = enabled;

    // 保存到defconfig文件
    if (savePeripheralStates()) {
        qDebug() << QString("外设 %1 已%2").arg(peripheral).arg(enabled ? "启用" : "禁用");
    } else {
        // 如果保存失败，恢复复选框状态
        QMessageBox::warning(this, "警告", "无法更新defconfig文件配置");

        // 恢复复选框状态
        m_peripheralStates[peripheral] = !enabled;

        // 找到对应的复选框并恢复状态
        QTreeWidgetItem *peripheralItem = m_pinoutConfigTree->topLevelItem(0);
        if (peripheralItem) {
            for (int i = 0; i < peripheralItem->childCount(); ++i) {
                QTreeWidgetItem *subItem = peripheralItem->child(i);
                QCheckBox *checkBox = qobject_cast<QCheckBox*>(m_pinoutConfigTree->itemWidget(subItem, 0));
                if (checkBox && checkBox->text() == peripheral) {
                    checkBox->setChecked(!enabled);
                    break;
                }
            }
        }
    }
}

QString MainWindow::getDefconfigPath() const
{
    // 根据选择的芯片类型返回对应的defconfig文件路径
    QString chipType = m_selectedChip;
    if (chipType.isEmpty() || chipType == "请选择芯片型号") {
        chipType = "cv1842hp_wevb_0014a_emmc"; // 默认使用cv1842hp_wevb_0014a_emmc
    }

    QString defconfigPath = QString("build/boards/cv184x/%1/linux/cvitek_%1_defconfig")
                           .arg(chipType);

    // 使用用户选择的源代码路径
    QDir workspaceDir(m_sourcePath);
    return workspaceDir.absoluteFilePath(defconfigPath);
}

QMap<QString, QStringList> MainWindow::getPeripheralConfigs() const
{
    QMap<QString, QStringList> configs;

    // 定义每个外设对应的CONFIG项
    configs["PWM"] = {"CONFIG_PWM", "CONFIG_PWM_SYSFS", "CONFIG_CVI_PWM"};
    configs["I2C"] = {"CONFIG_I2C", "CONFIG_I2C_SMBUS", "CONFIG_I2C_CHARDEV", "CONFIG_I2C_DESIGNWARE_PLATFORM"};
    configs["SPI"] = {"CONFIG_SPI"};
    configs["UART"] = {"CONFIG_SERIAL_8250", "CONFIG_SERIAL_8250_CONSOLE", "CONFIG_SERIAL_8250_DW"};
    configs["GPIO"] = {"CONFIG_GPIOLIB", "CONFIG_GPIO_SYSFS", "CONFIG_GPIO_DWAPB"};
    configs["ADC"] = {"CONFIG_IIO", "CONFIG_IIO_BUFFER", "CONFIG_IIO_TRIGGER"};
    configs["SYSDMA"] = {"CONFIG_CVI_SYSDMA_REMAP","CONFIG_DW_DMAC_CVITEK","CONFIG_DMADEVICES","CONFIG_DMA_CMA"};
    configs["WIEGAND"] = {"CONFIG_CVI_WIEGAND"};
    configs["SPACC"] = {"CONFIG_CVI_SPACC"};
    configs["TRNG"] = {"CONFIG_HW_RANDOM"};
    configs["RTOS_CMDQU"] = {"CONFIG_RTOS_CMDQU"};
    configs["THERMAL"] = {"CONFIG_THERMAL","CONFIG_THERMAL_NETLINK","CONFIG_THERMAL_WRITABLE_TRIPS","CONFIG_THERMAL_EMULATION","CONFIG_CV184X_THERMAL","CONFIG_CVI_CLK_COOLING"};
    return configs;
}

bool MainWindow::loadPeripheralStates()
{
    QString defconfigPath = getDefconfigPath();
    QFile file(defconfigPath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "无法打开defconfig文件：" << defconfigPath;

        // 如果文件不存在，设置默认状态（所有外设都禁用）
        QStringList peripherals = {"PWM", "I2C", "SPI", "UART", "GPIO", "ADC", "SYSDMA", "WIEGAND", "SPACC", "TRNG", "RTOS_CMDQU"};
        for (const QString &peripheral : peripherals) {
            m_peripheralStates[peripheral] = false;
        }
        return false;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    QMap<QString, QStringList> peripheralConfigs = getPeripheralConfigs();

    // 检查每个外设的CONFIG项
    for (auto it = peripheralConfigs.begin(); it != peripheralConfigs.end(); ++it) {
        const QString &peripheral = it.key();
        const QStringList &configItems = it.value();

        bool isEnabled = true;

        // 检查所有相关的CONFIG项是否都启用
        for (const QString &configItem : configItems) {
            QString enabledPattern = configItem + "=y";
            QString disabledPattern = "# " + configItem + " is not set";

            if (content.contains(disabledPattern) || !content.contains(enabledPattern)) {
                isEnabled = false;
                break;
            }
        }

        m_peripheralStates[peripheral] = isEnabled;
    }

    return true;
}

bool MainWindow::savePeripheralStates()
{
    QString defconfigPath = getDefconfigPath();
    QFile file(defconfigPath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "无法打开defconfig文件：" << defconfigPath;
        return false;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    QMap<QString, QStringList> peripheralConfigs = getPeripheralConfigs();

    // 更新每个外设的CONFIG项
    for (auto it = peripheralConfigs.begin(); it != peripheralConfigs.end(); ++it) {
        const QString &peripheral = it.key();
        const QStringList &configItems = it.value();
        bool isEnabled = m_peripheralStates.value(peripheral, false);

        for (const QString &configItem : configItems) {
            QString enabledPattern = configItem + "=y";
            QString disabledPattern = "# " + configItem + " is not set";

            if (isEnabled) {
                // 启用该配置项
                if (content.contains(disabledPattern)) {
                    content.replace(disabledPattern, enabledPattern);
                } else if (!content.contains(enabledPattern)) {
                    // 如果既没有启用也没有禁用的配置，在适当位置添加
                    content.append("\n" + enabledPattern);
                }
            } else {
                // 禁用该配置项
                if (content.contains(enabledPattern)) {
                    content.replace(enabledPattern, disabledPattern);
                } else if (!content.contains(disabledPattern)) {
                    // 如果既没有启用也没有禁用的配置，在适当位置添加
                    content.append("\n" + disabledPattern);
                }
            }
        }
    }

    // 写回文件
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "无法写入defconfig文件：" << defconfigPath;
        return false;
    }

    QTextStream out(&file);
    out << content;
    file.close();

    return true;
}

void MainWindow::updatePeripheralCheckBoxes()
{
    // 更新UI中外设复选框的状态
    QTreeWidgetItem *peripheralItem = m_pinoutConfigTree->topLevelItem(0);
    if (!peripheralItem) {
        return;
    }

    // 遍历所有外设子项，更新复选框状态
    for (int i = 0; i < peripheralItem->childCount(); ++i) {
        QTreeWidgetItem *subItem = peripheralItem->child(i);
        QCheckBox *checkBox = qobject_cast<QCheckBox*>(m_pinoutConfigTree->itemWidget(subItem, 0));
        if (checkBox) {
            QString peripheralType = checkBox->text();
            bool isEnabled = m_peripheralStates.value(peripheralType, false);

            // 临时断开信号连接，避免触发onPeripheralCheckBoxChanged
            checkBox->blockSignals(true);
            checkBox->setChecked(isEnabled);
            checkBox->blockSignals(false);

            qDebug() << QString("更新外设 %1 的复选框状态为: %2").arg(peripheralType).arg(isEnabled ? "启用" : "禁用");
        }
    }
}

void MainWindow::initializeDtsConfig()
{
    m_dtsConfig = new DtsConfig(this);

    // 使用用户选择的源代码路径加载设备树文件
    QString dtsFilePath = QDir(m_sourcePath).absoluteFilePath("build/boards/default/dts/cv184x/cv184x_base.dtsi");
    if (!m_dtsConfig->loadDtsFile(dtsFilePath)) {
        qDebug() << "警告：无法加载设备树文件，外设配置功能将不可用";
        qDebug() << "尝试加载的路径：" << dtsFilePath;
    }
}

void MainWindow::showPeripheralConfig(const QString& peripheralType)
{
    if (!m_dtsConfig) {
        QMessageBox::warning(this, "警告", "设备树配置未初始化！");
        return;
    }

    PeripheralConfigDialog dialog(peripheralType, m_dtsConfig, this);
    dialog.exec();
}

void MainWindow::onClockConfigChanged()
{
    // 时钟配置更改时的处理
    qDebug() << "时钟配置已更改";

    // 可以在这里添加保存配置或其他处理逻辑
    // 比如自动保存时钟配置到文件
    // m_clockConfigPage->saveConfig("clock_config.json");
}

void MainWindow::onMemoryConfigChanged()
{
    // 内存配置更改时的处理
    qDebug() << "内存配置已更改";

    // 可以在这里添加保存配置或其他处理逻辑
    // 比如自动保存内存配置到文件
    // m_memoryConfigPage->exportToJson("memory_config.json");
}

void MainWindow::onFlashConfigChanged()
{
    // Flash配置更改时的处理
    qDebug() << "Flash分区配置已更改";

    // 可以在这里添加保存配置或其他处理逻辑
    // 比如自动保存Flash配置到文件
    // m_flashConfigPage->exportToJson("flash_config.json");
}

void MainWindow::onConfigTabChanged(int index)
{
    // 标签页切换时的处理
    if (index == 0) {
        // 切换到Pinout配置标签页
        qDebug() << "切换到芯片引脚配置页面";
    } else if (index == 1) {
        // 切换到时钟配置标签页
        qDebug() << "切换到时钟配置页面";
    } else if (index == 2) {
        // 切换到内存配置标签页
        qDebug() << "切换到内存配置页面";
    } else if (index == 3) {
        // 切换到Flash配置标签页
        qDebug() << "切换到Flash分区配置页面";
    }
}

bool MainWindow::selectSourcePath()
{
    showPathSelectionDialog();
    return !m_sourcePath.isEmpty();
}

void MainWindow::showPathSelectionDialog()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("CviCubeMX - 源代码路径选择");
    msgBox.setText("欢迎使用 CviCubeMX!");
    msgBox.setInformativeText("请选择包含设备树dts文件、defconfig 和 cvi_board_init.c 的源代码根目录。\n\n"
                             "该目录应包含以下结构：\n"
                             "- build/");
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Ok);
    int ret = msgBox.exec();
    if (ret == QMessageBox::Cancel) {
        return;
    }
    QString selectedPath = QFileDialog::getExistingDirectory(
        this,
        "选择源代码根目录",
        QDir::homePath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );
    if (selectedPath.isEmpty()) {
        return;
    }
    if (validateSourcePath(selectedPath)) {
        m_sourcePath = selectedPath;
        // 更新CodeGenerator的路径
        m_codeGenerator.setSourcePath(m_sourcePath);

        // 更新内存配置页面的源代码路径
        if (m_memoryConfigPage) {
            m_memoryConfigPage->setSourcePath(m_sourcePath);
        }

        // 更新时钟配置页面的源代码路径
        if (m_clockConfigPage) {
            m_clockConfigPage->setSourcePath(m_sourcePath);
        }

        // 更新Flash配置页面的源代码路径
        if (m_flashConfigPage) {
            m_flashConfigPage->setSourcePath(m_sourcePath);
        }

        // QMessageBox::information(this, "成功",
        //     QString("源代码路径设置成功：\n%1").arg(m_sourcePath));
    } else {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "路径验证失败",
            QString("所选目录缺少必要的文件结构：\n%1\n\n"
                   "是否要重新选择路径？").arg(selectedPath),
            QMessageBox::Yes | QMessageBox::No
        );

        if (reply == QMessageBox::Yes) {
            showPathSelectionDialog(); // 递归调用重新选择
        }
    }
}

bool MainWindow::validateSourcePath(const QString& path)
{
    QDir dir(path);

    // 检查必要的目录结构
    QStringList requiredDirs = {"build"};

    for (const QString& reqDir : requiredDirs) {
        if (!dir.exists(reqDir)) {
            qDebug() << "缺少目录：" << reqDir;
            return false;
        }
    }

    // 检查关键文件是否存在
    QStringList criticalPaths = {
        "build/boards/cv184x",
        "build/boards/default/dts/cv184x"
    };

    for (const QString& criticalPath : criticalPaths) {
        if (!dir.exists(criticalPath)) {
            qDebug() << "缺少关键路径：" << criticalPath;
            return false;
        }
    }

    return true;
}

void MainWindow::onSelectSourcePath()
{
    showPathSelectionDialog();

    // 更新UI中的路径显示
    if (!m_sourcePath.isEmpty()) {
        // 查找并更新路径标签
        QLabel *pathLabel = m_pinoutTab->findChild<QLabel*>();
        if (pathLabel) {
            QList<QLabel*> labels = m_pinoutTab->findChildren<QLabel*>();
            for (QLabel* label : labels) {
                if (label->text().startsWith("当前路径:")) {
                    label->setText(QString("当前路径: %1").arg(m_sourcePath));
                    break;
                }
            }
        }
    }
}

void MainWindow::onShowAIChat()
{
    // 如果对话窗口还没有创建，创建一个新的
    if (!m_aiChatDialog) {
        m_aiChatDialog = new AIChatDialog(this);
    }

    // 显示对话窗口
    m_aiChatDialog->show();
    m_aiChatDialog->raise();
    m_aiChatDialog->activateWindow();
}

bool MainWindow::selectChipType()
{
    QDialog chipDialog(this);
    chipDialog.setWindowTitle("CviCubeMX - 芯片选型");
    chipDialog.setModal(true);
    chipDialog.setMinimumWidth(400);

    QVBoxLayout *mainLayout = new QVBoxLayout(&chipDialog);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // 标题
    QLabel *titleLabel = new QLabel("请选择芯片型号", &chipDialog);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #2c3e50;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // 说明文字
    QLabel *infoLabel = new QLabel("请从下方列表中选择您要配置的芯片型号", &chipDialog);
    infoLabel->setStyleSheet("font-size: 12px; color: #7f8c8d;");
    infoLabel->setAlignment(Qt::AlignCenter);
    infoLabel->setWordWrap(true);
    mainLayout->addWidget(infoLabel);

    // 芯片选择下拉框
    QComboBox *chipCombo = new QComboBox(&chipDialog);
    chipCombo->addItems({
        "cv1840cp_wevb_0015a_spinor",
        "cv1841cp_wevb_0015a_emmc",
        "cv1841cp_wevb_0015a_spinand",
        "cv1841cp_wevb_0015a_spinor",
        "cv1842cp_wevb_0015a_spinand",
        "cv1842cp_wevb_0015a_spinor",
        "cv1842hp_wevb_0014a_emmc",
        "cv1842hp_wevb_0014a_spinand",
        "cv1842hp_wevb_0014a_spinor",
        "cv1843hp_wevb_0014a_emmc",
        "cv1843hp_wevb_0014a_spinand",
        "cv1843hp_wevb_0014a_spinor"
    });
    // 设置默认选择为 cv1842hp_wevb_0014a_emmc
    chipCombo->setCurrentText("cv1842hp_wevb_0014a_emmc");
    chipCombo->setStyleSheet(
        "QComboBox { "
        "font-size: 12px; "
        "padding: 8px; "
        "border: 2px solid #bdc3c7; "
        "border-radius: 4px; "
        "background-color: white; "
        "} "
        "QComboBox:hover { "
        "border-color: #3498db; "
        "} "
        "QComboBox::drop-down { "
        "border: none; "
        "} "
        "QComboBox::down-arrow { "
        "image: url(:/icons/down-arrow.png); "
        "}"
    );
    mainLayout->addWidget(chipCombo);

    // 按钮区域
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);

    QPushButton *okButton = new QPushButton("确定", &chipDialog);
    okButton->setStyleSheet(
        "QPushButton { "
        "background-color: #3498db; "
        "color: white; "
        "border: none; "
        "padding: 10px 30px; "
        "font-size: 12px; "
        "border-radius: 4px; "
        "min-width: 80px; "
        "} "
        "QPushButton:hover { "
        "background-color: #2980b9; "
        "}"
    );

    QPushButton *cancelButton = new QPushButton("取消", &chipDialog);
    cancelButton->setStyleSheet(
        "QPushButton { "
        "background-color: #95a5a6; "
        "color: white; "
        "border: none; "
        "padding: 10px 30px; "
        "font-size: 12px; "
        "border-radius: 4px; "
        "min-width: 80px; "
        "} "
        "QPushButton:hover { "
        "background-color: #7f8c8d; "
        "}"
    );

    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout);

    // 连接信号
    connect(okButton, &QPushButton::clicked, &chipDialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, &chipDialog, &QDialog::reject);

    // 显示对话框
    if (chipDialog.exec() == QDialog::Accepted) {
        m_selectedChip = chipCombo->currentText();
        
        // 设置到芯片下拉框
        if (m_chipComboBox) {
            int index = m_chipComboBox->findText(m_selectedChip);
            if (index >= 0) {
                m_chipComboBox->setCurrentIndex(index);
            }
        }
        
        // 更新各个配置页面的芯片类型
        if (m_memoryConfigPage) {
            m_memoryConfigPage->setChipType(m_selectedChip);
        }
        if (m_clockConfigPage) {
            m_clockConfigPage->setChipType(m_selectedChip);
        }
        if (m_flashConfigPage) {
            m_flashConfigPage->setChipType(m_selectedChip);
        }
        
        // 加载外设状态
        if (!m_sourcePath.isEmpty()) {
            loadPeripheralStates();
            if (m_pinoutConfigTree) {
                updatePeripheralCheckBoxes();
            }
        }
        
        return true;
    }
    
    return false;
}
