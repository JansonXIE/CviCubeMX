#include "mainwindow.h"
#include <QApplication>
#include <QScreen>
#include <QTimer>
#include <QLineEdit>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QSplitter>
#include <QMenu>
#include <QAction>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_mainLayout(nullptr)
    , m_headerLayout(nullptr)
    , m_controlLayout(nullptr)
    , m_mainSplitter(nullptr)
    , m_configPanel(nullptr)
    , m_configLayout(nullptr)
    , m_configTree(nullptr)
    , m_titleLabel(nullptr)
    , m_chipComboBox(nullptr)
    , m_startProjectButton(nullptr)
    , m_generateCodeButton(nullptr)
    , m_stackedWidget(nullptr)
    , m_welcomePage(nullptr)
    , m_chipViewPage(nullptr)
    , m_chipContainer(nullptr)
    , m_pinLayout(nullptr)
    , m_searchLayout(nullptr)
    , m_searchLabel(nullptr)
    , m_searchLineEdit(nullptr)
    , m_blinkTimer(nullptr)
    , m_highlightedPin(nullptr)
    , m_blinkState(false)
{
    setupUI();
    
    // 初始化引脚名称映射
    initializePinNameMappings();
    
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
}

void MainWindow::setupUI()
{
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
    
    // 创建控制区域
    m_controlLayout = new QHBoxLayout();
    m_controlLayout->setContentsMargins(8, 4, 8, 4);
    m_controlLayout->setSpacing(8);
    
    // 芯片选型标签
    QLabel *chipLabel = new QLabel("芯片选型:", this);
    chipLabel->setStyleSheet("font-size: 12px; font-weight: bold; color: #495057;");
    
    // 芯片选择下拉框
    m_chipComboBox = new QComboBox(this);
    m_chipComboBox->addItems({"请选择芯片型号", "cv1801c", "cv1801h", "cv1811c", "cv1811h", "cv1842cp", "cv1842hp"});
    m_chipComboBox->setMinimumWidth(160);
    m_chipComboBox->setMaximumHeight(28);
    m_chipComboBox->setStyleSheet("font-size: 11px; padding: 3px; border: 1px solid #ced4da; border-radius: 3px;");
    
    // 开始项目按钮
    m_startProjectButton = new QPushButton("开始项目", this);
    m_startProjectButton->setEnabled(false);
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
    m_generateCodeButton = new QPushButton("生成代码", this);
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
    m_controlLayout->addStretch();
    
    // 创建堆叠窗口部件
    m_stackedWidget = new QStackedWidget(this);
    
    // 创建主分隔器
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);
    
    // 设置配置面板
    setupConfigPanel();
    
    // 创建右侧内容部件
    QWidget *rightContent = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightContent);
    rightLayout->addWidget(m_stackedWidget);
    
    // 设置搜索框
    setupSearchBox();
    rightLayout->addLayout(m_searchLayout);
    
    // 添加到分隔器
    m_mainSplitter->addWidget(m_configPanel);
    m_mainSplitter->addWidget(rightContent);
    
    // 设置分隔器比例（配置面板:主内容 = 1:3）
    m_mainSplitter->setStretchFactor(0, 1);
    m_mainSplitter->setStretchFactor(1, 3);
    m_mainSplitter->setSizes({250, 750});
    
    // 创建欢迎页面
    m_welcomePage = new QWidget();
    QVBoxLayout *welcomeLayout = new QVBoxLayout(m_welcomePage);
    QLabel *welcomeLabel = new QLabel("请选择芯片型号并点击", m_welcomePage);
    QLabel *startProjectLabel = new QLabel("开始项目", m_welcomePage);
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
    
    // 添加到主布局
    m_mainLayout->addWidget(m_titleLabel);
    m_mainLayout->addLayout(m_controlLayout);
    m_mainLayout->addWidget(m_mainSplitter);
    
    // 连接信号和槽
    connect(m_chipComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onChipSelectionChanged);
    connect(m_startProjectButton, &QPushButton::clicked, this, &MainWindow::onStartProject);
    connect(m_generateCodeButton, &QPushButton::clicked, this, &MainWindow::onGenerateCode);
    connect(m_searchLineEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    connect(m_configTree, &QTreeWidget::itemClicked, this, &MainWindow::onPeripheralItemClicked);
}

void MainWindow::setupConfigPanel()
{
    // 创建配置面板
    m_configPanel = new QWidget();
    m_configPanel->setMinimumWidth(200);
    m_configPanel->setMaximumWidth(300);
    m_configPanel->setStyleSheet(
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
    m_configLayout = new QVBoxLayout(m_configPanel);
    m_configLayout->setContentsMargins(10, 10, 10, 10);
    
    // 创建配置面板标题
    QLabel *configTitle = new QLabel("配置面板", m_configPanel);
    configTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #495057; margin-bottom: 10px;");
    configTitle->setAlignment(Qt::AlignCenter);
    
    // 创建配置树
    m_configTree = new QTreeWidget(m_configPanel);
    m_configTree->setHeaderHidden(true);
    m_configTree->setRootIsDecorated(true);
    m_configTree->setIndentation(20);
    
    // 创建外设节点
    QTreeWidgetItem *peripheralItem = new QTreeWidgetItem(m_configTree);
    peripheralItem->setText(0, "外设");
    peripheralItem->setIcon(0, style()->standardIcon(QStyle::SP_ComputerIcon));
    peripheralItem->setExpanded(true);
    
    // 创建外设子项
    QStringList peripherals = {"PWM", "I2C", "SPI", "UART", "GPIO", "ADC"};
    for (const QString &peripheral : peripherals) {
        QTreeWidgetItem *subItem = new QTreeWidgetItem(peripheralItem);
        subItem->setText(0, peripheral);
        subItem->setIcon(0, style()->standardIcon(QStyle::SP_FileIcon));
        subItem->setData(0, Qt::UserRole, peripheral); // 存储外设类型
    }
    
    // 添加到布局
    m_configLayout->addWidget(configTitle);
    m_configLayout->addWidget(m_configTree);
    m_configLayout->addStretch();
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
    m_searchLineEdit->setPlaceholderText("输入管脚号 (例如: A2, 15)");
    m_searchLineEdit->setMaximumWidth(200);
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
    bool isQFN = (m_selectedChip.endsWith('c') || m_selectedChip == "cv1842cp");
    bool isBGA = (m_selectedChip.endsWith('h') || m_selectedChip == "cv1842hp");
    
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
    
    // 如果当前正在高亮这个引脚，且功能不是默认GPIO，则取消高亮
    if (m_highlightedPin && 
        (m_highlightedPin->getPinName() == pinName || m_highlightedPin->getDisplayName() == pinName) &&
        function != "GPIO") {
        
        // 停止闪烁并清除高亮
        m_blinkTimer->stop();
        m_highlightedPin->setHighlight(false);
        m_highlightedPin = nullptr;
        
        // 清空搜索框
        m_searchLineEdit->clear();
    }
}

void MainWindow::onGenerateCode()
{
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
    m_pinNameMappings["B1"] = "PAD_MIPI_TXP3";
    m_pinNameMappings["C4"] = "PAD_CAM_MCLK0";
    m_pinNameMappings["L15"] = "UART2_RTS";
    m_pinNameMappings["M5"] = "PAD_PWM0";
    
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
    
    // 停止闪烁定时器
    m_blinkTimer->stop();
    
    // 更新当前搜索文本
    m_currentSearchText = text.trimmed();
    
    // 如果搜索文本为空，直接返回
    if (m_currentSearchText.isEmpty()) {
        return;
    }
    
    // 查找匹配的引脚
    PinWidget* foundPin = nullptr;
    
    // 在引脚部件映射中查找
    for (auto it = m_pinWidgets.begin(); it != m_pinWidgets.end(); ++it) {
        const QString& pinKey = it.key();
        PinWidget* pinWidget = it.value();
        
        // 检查是否匹配（支持部分匹配）
        if (pinKey.contains(m_currentSearchText, Qt::CaseInsensitive) ||
            pinWidget->getDisplayName().contains(m_currentSearchText, Qt::CaseInsensitive)) {
            foundPin = pinWidget;
            break;
        }
    }
    
    // 如果找到匹配的引脚
    if (foundPin && foundPin->isEnabled()) {
        m_highlightedPin = foundPin;
        // 开始闪烁效果
        m_blinkState = true;
        m_highlightedPin->setHighlight(true, m_blinkState);
        m_blinkTimer->start();
    }
}

void MainWindow::onBlinkTimeout()
{
    if (m_highlightedPin) {
        // 切换闪烁状态
        m_blinkState = !m_blinkState;
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

void MainWindow::onPeripheralItemClicked(QTreeWidgetItem* item, int column)
{
    Q_UNUSED(column)
    
    if (!item) return;
    
    // 获取外设类型
    QString peripheralType = item->data(0, Qt::UserRole).toString();
    
    if (peripheralType.isEmpty()) {
        // 如果是父节点（外设），展开或折叠
        if (item->text(0) == "外设") {
            item->setExpanded(!item->isExpanded());
        }
        return;
    }
    
    // 显示外设配置菜单
    QMenu contextMenu(this);
    contextMenu.setStyleSheet(
        "QMenu { "
        "background-color: #ffffff; "
        "border: 1px solid #dee2e6; "
        "border-radius: 4px; "
        "padding: 5px; "
        "} "
        "QMenu::item { "
        "padding: 8px 16px; "
        "} "
        "QMenu::item:selected { "
        "background-color: #007bff; "
        "color: white; "
        "}"
    );
    
    // 根据外设类型添加不同的选项
    if (peripheralType == "PWM") {
        QAction *pwm0Action = contextMenu.addAction("PWM0");
        QAction *pwm1Action = contextMenu.addAction("PWM1");
        QAction *pwm2Action = contextMenu.addAction("PWM2");
        QAction *pwm3Action = contextMenu.addAction("PWM3");
        
        pwm0Action->setData("PWM0");
        pwm1Action->setData("PWM1");
        pwm2Action->setData("PWM2");
        pwm3Action->setData("PWM3");
        
        connect(pwm0Action, &QAction::triggered, this, &MainWindow::onPeripheralActionTriggered);
        connect(pwm1Action, &QAction::triggered, this, &MainWindow::onPeripheralActionTriggered);
        connect(pwm2Action, &QAction::triggered, this, &MainWindow::onPeripheralActionTriggered);
        connect(pwm3Action, &QAction::triggered, this, &MainWindow::onPeripheralActionTriggered);
    }
    else if (peripheralType == "I2C") {
        QAction *i2c0Action = contextMenu.addAction("I2C0");
        QAction *i2c1Action = contextMenu.addAction("I2C1");
        QAction *i2c2Action = contextMenu.addAction("I2C2");
        
        i2c0Action->setData("I2C0");
        i2c1Action->setData("I2C1");
        i2c2Action->setData("I2C2");
        
        connect(i2c0Action, &QAction::triggered, this, &MainWindow::onPeripheralActionTriggered);
        connect(i2c1Action, &QAction::triggered, this, &MainWindow::onPeripheralActionTriggered);
        connect(i2c2Action, &QAction::triggered, this, &MainWindow::onPeripheralActionTriggered);
    }
    else if (peripheralType == "SPI") {
        QAction *spi0Action = contextMenu.addAction("SPI0");
        QAction *spi1Action = contextMenu.addAction("SPI1");
        QAction *spi2Action = contextMenu.addAction("SPI2");
        
        spi0Action->setData("SPI0");
        spi1Action->setData("SPI1");
        spi2Action->setData("SPI2");
        
        connect(spi0Action, &QAction::triggered, this, &MainWindow::onPeripheralActionTriggered);
        connect(spi1Action, &QAction::triggered, this, &MainWindow::onPeripheralActionTriggered);
        connect(spi2Action, &QAction::triggered, this, &MainWindow::onPeripheralActionTriggered);
    }
    else if (peripheralType == "UART") {
        QAction *uart0Action = contextMenu.addAction("UART0");
        QAction *uart1Action = contextMenu.addAction("UART1");
        QAction *uart2Action = contextMenu.addAction("UART2");
        QAction *uart3Action = contextMenu.addAction("UART3");
        
        uart0Action->setData("UART0");
        uart1Action->setData("UART1");
        uart2Action->setData("UART2");
        uart3Action->setData("UART3");
        
        connect(uart0Action, &QAction::triggered, this, &MainWindow::onPeripheralActionTriggered);
        connect(uart1Action, &QAction::triggered, this, &MainWindow::onPeripheralActionTriggered);
        connect(uart2Action, &QAction::triggered, this, &MainWindow::onPeripheralActionTriggered);
        connect(uart3Action, &QAction::triggered, this, &MainWindow::onPeripheralActionTriggered);
    }
    else if (peripheralType == "GPIO") {
        QAction *gpioAction = contextMenu.addAction("配置GPIO");
        gpioAction->setData("GPIO");
        connect(gpioAction, &QAction::triggered, this, &MainWindow::onPeripheralActionTriggered);
    }
    else if (peripheralType == "ADC") {
        QAction *adc0Action = contextMenu.addAction("ADC0");
        QAction *adc1Action = contextMenu.addAction("ADC1");
        
        adc0Action->setData("ADC0");
        adc1Action->setData("ADC1");
        
        connect(adc0Action, &QAction::triggered, this, &MainWindow::onPeripheralActionTriggered);
        connect(adc1Action, &QAction::triggered, this, &MainWindow::onPeripheralActionTriggered);
    }
    else if (peripheralType == "Timer") {
        QAction *timer0Action = contextMenu.addAction("Timer0");
        QAction *timer1Action = contextMenu.addAction("Timer1");
        QAction *timer2Action = contextMenu.addAction("Timer2");
        
        timer0Action->setData("Timer0");
        timer1Action->setData("Timer1");
        timer2Action->setData("Timer2");
        
        connect(timer0Action, &QAction::triggered, this, &MainWindow::onPeripheralActionTriggered);
        connect(timer1Action, &QAction::triggered, this, &MainWindow::onPeripheralActionTriggered);
        connect(timer2Action, &QAction::triggered, this, &MainWindow::onPeripheralActionTriggered);
    }
    
    // 在鼠标位置显示菜单
    QPoint globalPos = m_configTree->mapToGlobal(m_configTree->visualItemRect(item).center());
    contextMenu.exec(globalPos);
}

void MainWindow::onPeripheralActionTriggered()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (!action) return;
    
    QString peripheralFunction = action->data().toString();
    
    // 显示提示信息
    QMessageBox::information(this, "外设配置", 
        QString("您选择了：%1\n这里可以添加具体的外设配置功能").arg(peripheralFunction));
    
    // 这里可以添加具体的外设配置逻辑
    // 例如：打开外设配置对话框，设置引脚功能等
}
