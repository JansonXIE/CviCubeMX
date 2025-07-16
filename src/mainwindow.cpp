#include "mainwindow.h"
#include <QApplication>
#include <QScreen>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_mainLayout(nullptr)
    , m_headerLayout(nullptr)
    , m_controlLayout(nullptr)
    , m_titleLabel(nullptr)
    , m_chipComboBox(nullptr)
    , m_startProjectButton(nullptr)
    , m_generateCodeButton(nullptr)
    , m_stackedWidget(nullptr)
    , m_welcomePage(nullptr)
    , m_chipViewPage(nullptr)
    , m_chipContainer(nullptr)
    , m_pinLayout(nullptr)
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
    
    // 创建标题
    m_titleLabel = new QLabel("CviCubeMX 芯片引脚配置工具", this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; margin: 20px; color: #2c3e50;");
    
    // 创建控制区域
    m_controlLayout = new QHBoxLayout();
    
    // 芯片选型标签
    QLabel *chipLabel = new QLabel("芯片选型:", this);
    chipLabel->setStyleSheet("font-size: 14px; font-weight: bold;");
    
    // 芯片选择下拉框
    m_chipComboBox = new QComboBox(this);
    m_chipComboBox->addItems({"请选择芯片型号", "cv1801c", "cv1801h", "cv1811c", "cv1811h", "cv1842cp", "cv1842hp"});
    m_chipComboBox->setMinimumWidth(200);
    m_chipComboBox->setStyleSheet("font-size: 12px; padding: 5px;");
    
    // 开始项目按钮
    m_startProjectButton = new QPushButton("开始项目", this);
    m_startProjectButton->setEnabled(false);
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
    m_mainLayout->addWidget(m_stackedWidget);
    
    // 连接信号和槽
    connect(m_chipComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onChipSelectionChanged);
    connect(m_startProjectButton, &QPushButton::clicked, this, &MainWindow::onStartProject);
    connect(m_generateCodeButton, &QPushButton::clicked, this, &MainWindow::onGenerateCode);
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
}

void MainWindow::onGenerateCode()
{
    // 生成代码
    QString code = m_codeGenerator.generateCode(m_chipConfig);
    
    // 保存文件
    QString fileName = QFileDialog::getSaveFileName(this, 
        "保存代码文件", 
        "cvi_board_init.c", 
        "C Files (*.c);;All Files (*)");
    
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << code;
            file.close();
            QMessageBox::information(this, "成功", "代码文件已生成: " + fileName);
        } else {
            QMessageBox::critical(this, "错误", "无法保存文件!");
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
