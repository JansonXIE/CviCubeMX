#include "memoryconfig.h"
#include <QApplication>
#include <QDebug>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QFont>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QRegularExpression>

// 常量定义
const quint64 MemoryConfigWidget::TOTAL_MEMORY_SIZE = 0x10000000;  // 256MB
const quint64 MemoryConfigWidget::MEMORY_BASE_ADDRESS = 0x80000000; // 内存基地址

MemoryConfigWidget::MemoryConfigWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_splitter(nullptr)
    , m_tableWidget(nullptr)
    , m_tableLayout(nullptr)
    , m_memoryTable(nullptr)
    , m_tableTitle(nullptr)
    , m_controlPanel(nullptr)
    , m_controlLayout(nullptr)
    , m_operationGroup(nullptr)
    , m_operationLayout(nullptr)
    , m_addButton(nullptr)
    , m_removeButton(nullptr)
    , m_resetButton(nullptr)
    , m_exportButton(nullptr)
    , m_importButton(nullptr)
    , m_searchGroup(nullptr)
    , m_searchLayout(nullptr)
    , m_searchEdit(nullptr)
    , m_configContainer(nullptr)
    , m_configContainerLayout(nullptr)
    , m_configHeader(nullptr)
    , m_configHeaderLayout(nullptr)
    , m_configTitleLabel(nullptr)
    , m_configToggleButton(nullptr)
    , m_configContent(nullptr)
    , m_configLayout(nullptr)
    , m_nameEdit(nullptr)
    , m_startAddressEdit(nullptr)
    , m_sizeEdit(nullptr)
    , m_sizeUnitCombo(nullptr)
    , m_configExpanded(false)
    , m_visualizationGroup(nullptr)
    , m_visualizationLayout(nullptr)
    , m_memoryMapText(nullptr)
    , m_sourcePath("")
    , m_chipType("")
{
    setupUI();
    initializeMemoryRegions();
    populateTable();
    updateMemoryVisualization();
}

MemoryConfigWidget::~MemoryConfigWidget()
{
}

void MemoryConfigWidget::setupUI()
{
    // 创建主布局
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    m_mainLayout->setSpacing(10);
    
    // 创建分隔器（水平分隔：左侧表格，右侧控制面板）
    m_splitter = new QSplitter(Qt::Horizontal, this);
    
    // 设置表格区域
    setupMemoryTable();
    
    // 设置控制面板
    setupControlPanel();
    
    // 添加到分隔器
    m_splitter->addWidget(m_tableWidget);
    m_splitter->addWidget(m_controlPanel);
    
    // 设置分隔器比例（表格:控制面板 = 7:3）
    m_splitter->setStretchFactor(0, 7);
    m_splitter->setStretchFactor(1, 3);
    m_splitter->setSizes({700, 300});
    
    // 添加到主布局
    m_mainLayout->addWidget(m_splitter);
}

void MemoryConfigWidget::setupMemoryTable()
{
    // 创建表格区域
    m_tableWidget = new QWidget();
    m_tableLayout = new QVBoxLayout(m_tableWidget);
    m_tableLayout->setContentsMargins(5, 5, 5, 5);
    m_tableLayout->setSpacing(5);
    
    // 创建标题
    m_tableTitle = new QLabel("内存布局配置", m_tableWidget);
    m_tableTitle->setAlignment(Qt::AlignCenter);
    m_tableTitle->setStyleSheet(
        "font-size: 16px; "
        "font-weight: bold; "
        "color: #2c3e50; "
        "background-color: #ecf0f1; "
        "padding: 8px; "
        "border: 1px solid #bdc3c7; "
        "border-radius: 4px;"
    );
    
    // 创建内存表格
    m_memoryTable = new QTableWidget(0, COL_COUNT, m_tableWidget);
    
    // 设置表头
    QStringList headers;
    headers << "Name" << "Start Address" << "End Address" << "Size" << "Size(M/K/B)";
    m_memoryTable->setHorizontalHeaderLabels(headers);
    
    // 设置表格属性
    m_memoryTable->setAlternatingRowColors(true);
    m_memoryTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_memoryTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_memoryTable->setSortingEnabled(false);  // 禁用排序功能，保持原始顺序，否则按字母排序
    
    // 设置表格样式
    m_memoryTable->setStyleSheet(
        "QTableWidget { "
        "gridline-color: #bdc3c7; "
        "background-color: #ffffff; "
        "border: 1px solid #bdc3c7; "
        "border-radius: 4px; "
        "} "
        "QTableWidget::item { "
        "padding: 8px; "
        "border-bottom: 1px solid #ecf0f1; "
        "} "
        "QTableWidget::item:selected { "
        "background-color: #3498db; "
        "color: white; "
        "} "
        "QHeaderView::section { "
        "background-color: #34495e; "
        "color: white; "
        "padding: 8px; "
        "border: none; "
        "font-weight: bold; "
        "}"
    );
    
    // 设置列宽
    QHeaderView* header = m_memoryTable->horizontalHeader();
    header->setStretchLastSection(true);
    header->resizeSection(COL_NAME, 150);
    header->resizeSection(COL_START_ADDRESS, 120);
    header->resizeSection(COL_END_ADDRESS, 120);
    header->resizeSection(COL_SIZE_HEX, 100);
    header->resizeSection(COL_SIZE_FORMATTED, 100);
    
    // 连接信号 - 移除itemChanged信号连接，因为表格现在不可编辑
    // connect(m_memoryTable, &QTableWidget::itemChanged, 
    //         this, &MemoryConfigWidget::onTableItemChanged);
    
    // 连接双击信号，用于展开配置面板
    connect(m_memoryTable, &QTableWidget::itemDoubleClicked, 
            this, &MemoryConfigWidget::onTableItemDoubleClicked);
    
    // 添加到布局
    m_tableLayout->addWidget(m_tableTitle);
    m_tableLayout->addWidget(m_memoryTable);
}

void MemoryConfigWidget::setupControlPanel()
{
    // 创建控制面板
    m_controlPanel = new QWidget();
    m_controlLayout = new QVBoxLayout(m_controlPanel);
    m_controlLayout->setContentsMargins(5, 5, 5, 5);
    m_controlLayout->setSpacing(10);
    
    // 操作按钮组
    m_operationGroup = new QGroupBox("操作", m_controlPanel);
    m_operationLayout = new QVBoxLayout(m_operationGroup);
    m_operationLayout->setSpacing(5);
    
    m_addButton = new QPushButton("添加区域", m_operationGroup);
    m_removeButton = new QPushButton("删除区域", m_operationGroup);
    m_resetButton = new QPushButton("重置配置", m_operationGroup);
    m_exportButton = new QPushButton("导出配置", m_operationGroup);
    m_importButton = new QPushButton("导入配置", m_operationGroup);
    
    // 设置按钮样式
    QString buttonStyle = 
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
        "QPushButton:pressed { "
        "background-color: #21618c; "
        "}";
    
    m_addButton->setStyleSheet(buttonStyle);
    m_removeButton->setStyleSheet(buttonStyle.replace("#3498db", "#e74c3c").replace("#2980b9", "#c0392b").replace("#21618c", "#a93226"));
    m_resetButton->setStyleSheet(buttonStyle.replace("#e74c3c", "#f39c12").replace("#c0392b", "#e67e22").replace("#a93226", "#d35400"));
    m_exportButton->setStyleSheet(buttonStyle.replace("#f39c12", "#27ae60").replace("#e67e22", "#229954").replace("#d35400", "#1e8449"));
    m_importButton->setStyleSheet(buttonStyle.replace("#27ae60", "#8e44ad").replace("#229954", "#7d3c98").replace("#1e8449", "#6c3483"));
    
    m_operationLayout->addWidget(m_addButton);
    m_operationLayout->addWidget(m_removeButton);
    m_operationLayout->addWidget(m_resetButton);
    m_operationLayout->addWidget(m_exportButton);
    m_operationLayout->addWidget(m_importButton);
    
    // 地址区域搜索组
    m_searchGroup = new QGroupBox("地址区域搜索", m_controlPanel);
    m_searchLayout = new QVBoxLayout(m_searchGroup);
    m_searchLayout->setSpacing(5);
    
    m_searchEdit = new QLineEdit(m_searchGroup);
    m_searchEdit->setPlaceholderText("搜索内存区域名称...");
    m_searchEdit->setStyleSheet(
        "QLineEdit { "
        "padding: 8px; "
        "border: 1px solid #bdc3c7; "
        "border-radius: 4px; "
        "background-color: #ffffff; "
        "} "
        "QLineEdit:focus { "
        "border: 2px solid #3498db; "
        "}"
    );
    
    m_searchLayout->addWidget(m_searchEdit);
    
    // 区域配置可折叠面板
    m_configContainer = new QWidget(m_controlPanel);
    m_configContainerLayout = new QVBoxLayout(m_configContainer);
    m_configContainerLayout->setContentsMargins(0, 0, 0, 0);
    m_configContainerLayout->setSpacing(0);
    
    // 配置面板头部（带折叠按钮）
    m_configHeader = new QWidget(m_configContainer);
    m_configHeader->setStyleSheet(
        "QWidget { "
        "background-color: #ecf0f1; "
        "border: 1px solid #bdc3c7; "
        "border-radius: 4px; "
        "}"
    );
    m_configHeaderLayout = new QHBoxLayout(m_configHeader);
    m_configHeaderLayout->setContentsMargins(8, 8, 8, 8);
    
    m_configTitleLabel = new QLabel("区域配置", m_configHeader);
    m_configTitleLabel->setStyleSheet(
        "QLabel { "
        "font-weight: bold; "
        "color: #2c3e50; "
        "background: transparent; "
        "border: none; "
        "}"
    );
    
    m_configToggleButton = new QToolButton(m_configHeader);
    m_configToggleButton->setArrowType(Qt::RightArrow);  // 默认收起状态
    m_configToggleButton->setStyleSheet(
        "QToolButton { "
        "border: none; "
        "background: transparent; "
        "font-size: 12px; "
        "padding: 2px; "
        "} "
        "QToolButton:hover { "
        "background-color: #d5dbdb; "
        "border-radius: 2px; "
        "}"
    );
    m_configToggleButton->setToolTip("展开/收起区域配置");
    
    m_configHeaderLayout->addWidget(m_configTitleLabel);
    m_configHeaderLayout->addStretch();
    m_configHeaderLayout->addWidget(m_configToggleButton);
    
    // 配置面板内容区域
    m_configContent = new QFrame(m_configContainer);
    m_configContent->setStyleSheet(
        "QFrame { "
        "border: 1px solid #bdc3c7; "
        "border-top: none; "
        "border-radius: 0 0 4px 4px; "
        "background-color: #ffffff; "
        "}"
    );
    m_configContent->setVisible(false);  // 默认隐藏
    
    m_configLayout = new QVBoxLayout(m_configContent);
    m_configLayout->setContentsMargins(8, 8, 8, 8);
    m_configLayout->setSpacing(8);
    
    // Name 字段
    QHBoxLayout* nameLayout = new QHBoxLayout();
    nameLayout->addWidget(new QLabel("Name:"));
    m_nameEdit = new QLineEdit(m_configContent);
    m_nameEdit->setPlaceholderText("输入区域名称");
    m_nameEdit->setEnabled(false);  // 默认禁用
    nameLayout->addWidget(m_nameEdit);
    
    // Start address 字段
    QHBoxLayout* startLayout = new QHBoxLayout();
    startLayout->addWidget(new QLabel("Start address:"));
    m_startAddressEdit = new QLineEdit(m_configContent);
    m_startAddressEdit->setPlaceholderText("0x80000000");
    m_startAddressEdit->setEnabled(false);  // 默认禁用
    startLayout->addWidget(m_startAddressEdit);
    
    // Size 字段（复合控件）
    QHBoxLayout* sizeLayout = new QHBoxLayout();
    sizeLayout->addWidget(new QLabel("Size:"));
    m_sizeEdit = new QLineEdit(m_configContent);
    m_sizeEdit->setPlaceholderText("1");
    m_sizeEdit->setEnabled(false);  // 默认禁用
    m_sizeUnitCombo = new QComboBox(m_configContent);
    m_sizeUnitCombo->addItems({"B", "K", "M", "G"});
    m_sizeUnitCombo->setCurrentText("M");
    m_sizeUnitCombo->setEnabled(false);  // 默认禁用
    sizeLayout->addWidget(m_sizeEdit);
    sizeLayout->addWidget(m_sizeUnitCombo);
    
    // 设置字段样式
    QString fieldStyle = 
        "QLineEdit { "
        "padding: 6px; "
        "border: 1px solid #bdc3c7; "
        "border-radius: 4px; "
        "background-color: #ffffff; "
        "} "
        "QLineEdit:focus { "
        "border: 2px solid #3498db; "
        "} "
        "QComboBox { "
        "padding: 6px; "
        "border: 1px solid #bdc3c7; "
        "border-radius: 4px; "
        "background-color: #ffffff; "
        "min-width: 50px; "
        "}";
    
    m_nameEdit->setStyleSheet(fieldStyle);
    m_startAddressEdit->setStyleSheet(fieldStyle);
    m_sizeEdit->setStyleSheet(fieldStyle);
    m_sizeUnitCombo->setStyleSheet(fieldStyle);
    
    m_configLayout->addLayout(nameLayout);
    m_configLayout->addLayout(startLayout);
    m_configLayout->addLayout(sizeLayout);
    
    // 将头部和内容添加到容器
    m_configContainerLayout->addWidget(m_configHeader);
    m_configContainerLayout->addWidget(m_configContent);
    
    // 内存可视化组
    m_visualizationGroup = new QGroupBox("内存映射", m_controlPanel);
    m_visualizationLayout = new QVBoxLayout(m_visualizationGroup);
    
    m_memoryMapText = new QTextEdit(m_visualizationGroup);
    m_memoryMapText->setReadOnly(true);
    m_memoryMapText->setMaximumHeight(150);
    m_memoryMapText->setStyleSheet(
        "QTextEdit { "
        "background-color: #2c3e50; "
        "color: #ecf0f1; "
        "border: 1px solid #34495e; "
        "border-radius: 4px; "
        "font-family: 'Courier New', monospace; "
        "font-size: 10px; "
        "}"
    );
    
    m_visualizationLayout->addWidget(m_memoryMapText);
    
    // 添加到控制面板布局
    m_controlLayout->addWidget(m_operationGroup);
    m_controlLayout->addWidget(m_searchGroup);
    m_controlLayout->addWidget(m_configContainer);
    m_controlLayout->addWidget(m_visualizationGroup);
    m_controlLayout->addStretch();
    
    // 连接信号
    connect(m_addButton, &QPushButton::clicked, this, &MemoryConfigWidget::onAddRegion);
    connect(m_removeButton, &QPushButton::clicked, this, &MemoryConfigWidget::onRemoveRegion);
    connect(m_resetButton, &QPushButton::clicked, this, &MemoryConfigWidget::onResetRegions);
    connect(m_exportButton, &QPushButton::clicked, this, &MemoryConfigWidget::onExportConfig);
    connect(m_importButton, &QPushButton::clicked, this, &MemoryConfigWidget::onImportConfig);
    
    // 连接搜索和配置字段信号
    connect(m_searchEdit, &QLineEdit::textChanged, this, &MemoryConfigWidget::onSearchTextChanged);
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &MemoryConfigWidget::onSearchEnterPressed);
    connect(m_configToggleButton, &QToolButton::clicked, this, &MemoryConfigWidget::onToggleConfigPanel);
    connect(m_memoryTable, &QTableWidget::itemSelectionChanged, 
            this, &MemoryConfigWidget::onTableSelectionChanged);
    
    // 配置字段只在编辑完成时更新（使用editingFinished信号而不是textChanged）
    connect(m_nameEdit, &QLineEdit::editingFinished, this, &MemoryConfigWidget::onConfigFieldChanged);
    connect(m_startAddressEdit, &QLineEdit::editingFinished, this, &MemoryConfigWidget::onConfigFieldChanged);
    connect(m_sizeEdit, &QLineEdit::editingFinished, this, &MemoryConfigWidget::onConfigFieldChanged);
    connect(m_sizeUnitCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &MemoryConfigWidget::onConfigFieldChanged);
}

void MemoryConfigWidget::initializeMemoryRegions()
{
    // 初始化内存区域配置（基于起始地址和大小）
    struct MemoryRegionData {
        QString name;
        quint64 startAddress;
        quint64 size;           // 改为使用size而不是endAddress
        bool isEditable;
        QString description;
        int order;  // 添加顺序字段
    };
    
    QList<MemoryRegionData> regionData = {
        {"MONITOR", 0x80000000, 0x0, true, "监控区域", 0},                     // 0KB
        {"KERNEL_MEMORY", 0x80000000, 0x10000000, true, "内核内存区域", 1},           // 256M
        {"FSBL_C906L_START", 0x800a0000, 0x0, true, "FSBL C906L启动区域", 2},  // 0KB
        {"OPENSBI_FDT", 0x800a0000, 0x0, true, "OpenSBI设备树", 3},          // 0KB
        {"RTOS_LOG", 0x804a0000, 0x20000, true, "RTOS日志区域", 4},               // 128KB
        {"SHARE_MEM", 0x804c0000, 0x20000, true, "共享内存", 5},                  // 128KB
        {"SHARE_PARAM", 0x804e0000, 0x10000, true, "共享参数", 6},                // 64KB
        {"PQBIN", 0x80500000, 0x80000, true, "PQBIN区域", 7},                     // 512KB
        {"RTOS_LOGO", 0x80580000, 0x0, true, "RTOS Logo", 8},                 // 0KB
        {"CVI_UPDATE_HEADER", 0x813ffc00, 0x400, true, "CVI更新头", 9},           // 1KB
        {"FSBL_UNZIP", 0x81400000, 0x400000, true, "FSBL解压区域", 10},           // 4MB
        {"UIMAG", 0x81400000, 0x400000, true, "UI镜像", 11},                     // 4MB
        {"RTOS_COMPRESS_BIN", 0x81ea0000, 0x0, true, "RTOS压缩二进制", 12},    // 0KB
        {"H26X_BITSTREAM", 0x85500000, 0x0, true, "H26X比特流", 13},         // 0MB
        {"H26X_ENC_BUFF", 0x85500000, 0x0, true, "H26X编码缓冲", 14},        // 0MB
        {"ION", 0x85500000, 0x4b00000, true, "ION内存池", 15},                   // 75MB
        {"ISP_MEM_BASE", 0x85500000, 0x0, true, "ISP内存基址", 16},          // 0MB
        {"BOOTLOGO", 0x89e3e000, 0x1c2000, true, "启动Logo", 17},                // ~1.8MB
        {"RTOS_ION", 0x8a000000, 0x6000000, true, "RTOS ION", 18}                // 96MB
    };
    
    m_memoryRegions.clear();
    m_regionOrder.clear();  // 清空顺序列表
    
    for (const auto& data : regionData) {
        MemoryRegion region;
        region.name = data.name;
        region.startAddress = data.startAddress;
        region.size = data.size;
        region.endAddress = data.startAddress + data.size;  // 计算结束地址
        region.sizeString = formatSize(region.size);
        region.isEditable = data.isEditable;
        region.description = data.description;
        
        m_memoryRegions[region.name] = region;
        m_regionOrder.append(region.name);  // 保存顺序
    }
}

void MemoryConfigWidget::populateTable()
{
    // 暂时断开信号，避免在填充过程中触发itemChanged
    m_memoryTable->blockSignals(true);
    
    // 清空表格
    m_memoryTable->setRowCount(0);
    
    // 按照原始顺序填充表格
    for (int i = 0; i < m_regionOrder.size(); ++i) {
        const QString& regionName = m_regionOrder[i];
        if (!m_memoryRegions.contains(regionName)) continue;
        
        const MemoryRegion& region = m_memoryRegions[regionName];
        
        m_memoryTable->insertRow(i);
        
        // 名称列 - 设置为只读，只能通过配置面板修改
        QTableWidgetItem* nameItem = new QTableWidgetItem(region.name);
        nameItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);  // 移除 ItemIsEditable 标志
        m_memoryTable->setItem(i, COL_NAME, nameItem);
        
        // 起始地址列 - 设置为只读，只能通过配置面板修改
        QTableWidgetItem* startItem = new QTableWidgetItem(formatAddress(region.startAddress));
        startItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);  // 移除 ItemIsEditable 标志
        m_memoryTable->setItem(i, COL_START_ADDRESS, startItem);
        
        // 结束地址列 - 设置为只读，只能通过配置面板修改
        QTableWidgetItem* endItem = new QTableWidgetItem(formatAddress(region.endAddress));
        endItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);  // 移除 ItemIsEditable 标志
        m_memoryTable->setItem(i, COL_END_ADDRESS, endItem);
        
        // 大小（十六进制）列
        QTableWidgetItem* sizeItem = new QTableWidgetItem(QString("0x%1").arg(region.size, 0, 16));
        sizeItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        sizeItem->setBackground(QBrush(QColor("#f8f9fa")));
        m_memoryTable->setItem(i, COL_SIZE_HEX, sizeItem);
        
        // 大小（格式化）列
        QTableWidgetItem* sizeFormattedItem = new QTableWidgetItem(region.sizeString);
        sizeFormattedItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        sizeFormattedItem->setBackground(QBrush(QColor("#f8f9fa")));
        m_memoryTable->setItem(i, COL_SIZE_FORMATTED, sizeFormattedItem);
    }
    
    // 重新启用信号
    m_memoryTable->blockSignals(false);
}

void MemoryConfigWidget::onTableItemChanged(QTableWidgetItem* item)
{
    if (!item) return;
    
    int row = item->row();
    int col = item->column();
    
    // 确保行号在有效范围内
    if (row < 0 || row >= m_regionOrder.size()) return;
    
    // 从顺序列表获取正确的区域名称
    QString regionName = m_regionOrder[row];
    if (!m_memoryRegions.contains(regionName)) return;
    
    MemoryRegion& region = m_memoryRegions[regionName];
    
    // 根据修改的列更新数据
    switch (col) {
        case COL_NAME:
            // 名称更改，需要更新映射
            if (item->text() != regionName) {
                // 更新顺序列表中的名称
                m_regionOrder[row] = item->text();
                
                m_memoryRegions.remove(regionName);
                region.name = item->text();
                m_memoryRegions[region.name] = region;
            }
            break;
            
        case COL_START_ADDRESS:
            region.startAddress = parseAddress(item->text());
            // 基于size重新计算endAddress
            region.endAddress = region.startAddress + region.size;
            break;
            
        case COL_END_ADDRESS:
            // 当endAddress改变时，重新计算size
            {
                quint64 newEndAddress = parseAddress(item->text());
                if (newEndAddress > region.startAddress) {
                    region.size = newEndAddress - region.startAddress;
                    region.endAddress = newEndAddress;
                    region.sizeString = formatSize(region.size);
                } else {
                    // 如果输入的结束地址无效，恢复原值
                    item->setText(formatAddress(region.endAddress));
                }
            }
            break;
    }
    
    // 更新表格中的计算列
    updateSizeString(row);
    
    // 更新内存可视化
    updateMemoryVisualization();
    validateMemoryLayout();
    
    // 发送信号
    emit configChanged();
    emit memoryRegionChanged(region.name);
}

void MemoryConfigWidget::onTableItemDoubleClicked(QTableWidgetItem* item)
{
    if (!item) return;
    
    int row = item->row();
    
    // 确保行号在有效范围内
    if (row < 0 || row >= m_regionOrder.size()) return;
    
    // 选择整行
    m_memoryTable->selectRow(row);
    m_memoryTable->setCurrentItem(item);
    
    // 展开配置面板
    setConfigPanelExpanded(true);
    
    // 启用配置字段
    m_nameEdit->setEnabled(true);
    m_startAddressEdit->setEnabled(true);
    m_sizeEdit->setEnabled(true);
    m_sizeUnitCombo->setEnabled(true);
    
    // 更新配置字段内容
    onTableSelectionChanged();
}

void MemoryConfigWidget::updateSizeString(int row)
{
    // 确保行号在有效范围内
    if (row < 0 || row >= m_regionOrder.size()) return;
    
    // 从顺序列表获取正确的区域名称
    QString regionName = m_regionOrder[row];
    if (!m_memoryRegions.contains(regionName)) return;
    
    const MemoryRegion& region = m_memoryRegions[regionName];
    
    // 更新大小列
    QTableWidgetItem* sizeItem = m_memoryTable->item(row, COL_SIZE_HEX);
    if (sizeItem) {
        sizeItem->setText(QString("0x%1").arg(region.size, 0, 16));
    }
    
    QTableWidgetItem* sizeFormattedItem = m_memoryTable->item(row, COL_SIZE_FORMATTED);
    if (sizeFormattedItem) {
        sizeFormattedItem->setText(region.sizeString);
    }
    
    // 更新结束地址列
    QTableWidgetItem* endItem = m_memoryTable->item(row, COL_END_ADDRESS);
    if (endItem) {
        endItem->setText(formatAddress(region.endAddress));
    }
}

void MemoryConfigWidget::onAddRegion()
{
    // 添加新的内存区域
    QString newName = QString("NEW_REGION_%1").arg(m_memoryRegions.size() + 1);
    
    MemoryRegion region;
    region.name = newName;
    region.startAddress = MEMORY_BASE_ADDRESS;
    region.size = 0x100000; // 默认1MB
    region.endAddress = region.startAddress + region.size; // 基于size计算endAddress
    region.sizeString = formatSize(region.size);
    region.isEditable = true;
    region.description = "用户自定义区域";
    
    m_memoryRegions[region.name] = region;
    m_regionOrder.append(region.name);  // 添加到顺序列表
    
    // 重新填充表格
    populateTable();
    updateMemoryVisualization();
    
    emit configChanged();
}

void MemoryConfigWidget::onRemoveRegion()
{
    // 删除选中的内存区域
    QTableWidgetItem* currentItem = m_memoryTable->currentItem();
    if (!currentItem) {
        QMessageBox::warning(this, "警告", "请选择要删除的内存区域！");
        return;
    }
    
    int currentRow = currentItem->row();
    QTableWidgetItem* nameItem = m_memoryTable->item(currentRow, COL_NAME);
    if (!nameItem) return;
    
    QString regionName = nameItem->text();
    
    // 确认删除
    int ret = QMessageBox::question(this, "确认删除", 
                                   QString("确定要删除内存区域 \"%1\" 吗？").arg(regionName),
                                   QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        m_memoryRegions.remove(regionName);
        m_regionOrder.removeAll(regionName);  // 从顺序列表中移除
        populateTable();
        updateMemoryVisualization();
        
        emit configChanged();
    }
}

void MemoryConfigWidget::onResetRegions()
{
    // 重置到默认配置
    int ret = QMessageBox::question(this, "确认重置", 
                                   "确定要重置所有内存区域配置吗？这将丢失所有修改！",
                                   QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        initializeMemoryRegions();
        populateTable();
        updateMemoryVisualization();
        
        emit configChanged();
    }
}

void MemoryConfigWidget::onExportConfig()
{
    // 检查是否设置了源代码路径和芯片类型
    if (m_sourcePath.isEmpty() || m_chipType.isEmpty()) {
        QMessageBox::warning(this, "警告", 
                           "请先选择芯片类型和源代码路径！");
        return;
    }
    
    // 导出到defconfig文件
    if (exportToDefconfig(m_sourcePath, m_chipType)) {
        QString defconfigPath = QString("build/boards/cv184x/%1/%1_defconfig")
                               .arg(m_chipType);
        QMessageBox::information(this, "成功", 
                               QString("内存配置已导出到: %1").arg(defconfigPath));
    } else {
        QMessageBox::critical(this, "错误", "导出内存配置失败！");
    }
}

void MemoryConfigWidget::onImportConfig()
{
    QString fileName = QFileDialog::getOpenFileName(this, 
        "导入内存配置", 
        "", 
        "JSON Files (*.json);;All Files (*)");
    
    if (!fileName.isEmpty()) {
        if (importFromJson(fileName)) {
            populateTable();
            updateMemoryVisualization();
            QMessageBox::information(this, "成功", "内存配置已导入: " + fileName);
            emit configChanged();
        } else {
            QMessageBox::critical(this, "错误", "导入内存配置失败！");
        }
    }
}

void MemoryConfigWidget::onSearchEnterPressed()
{
    QString searchText = m_searchEdit->text().trimmed();
    if (!searchText.isEmpty()) {
        searchAndSelectRegion(searchText);
    }
}

void MemoryConfigWidget::onToggleConfigPanel()
{
    setConfigPanelExpanded(!m_configExpanded);
}

void MemoryConfigWidget::searchAndSelectRegion(const QString& searchText)
{
    // 精确匹配搜索文本与内存区域名称
    for (int row = 0; row < m_memoryTable->rowCount(); ++row) {
        QTableWidgetItem* nameItem = m_memoryTable->item(row, COL_NAME);
        if (nameItem && nameItem->text().compare(searchText, Qt::CaseInsensitive) == 0) {
            // 找到匹配的区域 - 选择整行并设置当前项
            m_memoryTable->selectRow(row);
            m_memoryTable->setCurrentItem(nameItem);
            
            // 展开配置面板
            setConfigPanelExpanded(true);
            
            // 启用配置字段
            m_nameEdit->setEnabled(true);
            m_startAddressEdit->setEnabled(true);
            m_sizeEdit->setEnabled(true);
            m_sizeUnitCombo->setEnabled(true);
            
            // 更新配置字段内容
            onTableSelectionChanged();
            
            return;
        }
    }
    
    // 如果没有找到精确匹配，显示提示
    QMessageBox::information(this, "搜索结果", 
                            QString("未找到名称为 \"%1\" 的内存区域").arg(searchText));
}

void MemoryConfigWidget::setConfigPanelExpanded(bool expanded)
{
    m_configExpanded = expanded;
    m_configContent->setVisible(expanded);
    
    // 更新箭头方向
    if (expanded) {
        m_configToggleButton->setArrowType(Qt::DownArrow);
        m_configToggleButton->setToolTip("收起区域配置");
    } else {
        m_configToggleButton->setArrowType(Qt::RightArrow);
        m_configToggleButton->setToolTip("展开区域配置");
        
        // 收起时禁用配置字段
        m_nameEdit->setEnabled(false);
        m_startAddressEdit->setEnabled(false);
        m_sizeEdit->setEnabled(false);
        m_sizeUnitCombo->setEnabled(false);
    }
}

void MemoryConfigWidget::onSearchTextChanged(const QString& searchText)
{
    // 实现搜索过滤功能（实时过滤，但不自动选择）
    for (int row = 0; row < m_memoryTable->rowCount(); ++row) {
        QTableWidgetItem* nameItem = m_memoryTable->item(row, COL_NAME);
        if (nameItem) {
            bool visible = searchText.isEmpty() || 
                          nameItem->text().contains(searchText, Qt::CaseInsensitive);
            m_memoryTable->setRowHidden(row, !visible);
        }
    }
}

void MemoryConfigWidget::onTableSelectionChanged()
{
    // 只有在配置面板展开时才更新配置字段
    if (!m_configExpanded) {
        return;
    }
    
    // 当表格选择改变时，更新配置字段
    QTableWidgetItem* currentItem = m_memoryTable->currentItem();
    if (!currentItem) {
        // 清空配置字段
        m_nameEdit->clear();
        m_startAddressEdit->clear();
        m_sizeEdit->clear();
        m_sizeUnitCombo->setCurrentText("M");
        return;
    }
    
    int currentRow = currentItem->row();
    if (currentRow < 0 || currentRow >= m_regionOrder.size()) {
        // 清空配置字段
        m_nameEdit->clear();
        m_startAddressEdit->clear();
        m_sizeEdit->clear();
        m_sizeUnitCombo->setCurrentText("M");
        return;
    }
    
    // 获取选中区域的数据
    QString regionName = m_regionOrder[currentRow];
    if (m_memoryRegions.contains(regionName)) {
        const MemoryRegion& region = m_memoryRegions[regionName];
        
        // 暂时断开信号，避免在填充过程中触发onConfigFieldChanged
        m_nameEdit->blockSignals(true);
        m_startAddressEdit->blockSignals(true);
        m_sizeEdit->blockSignals(true);
        m_sizeUnitCombo->blockSignals(true);
        
        // 填充配置字段
        m_nameEdit->setText(region.name);
        m_startAddressEdit->setText(formatAddress(region.startAddress));
        
        // 对于ION区域，Start Address是基于End Address和Size计算的，设置为只读
        if (regionName == "ION") {
            m_startAddressEdit->setReadOnly(true);
            m_startAddressEdit->setToolTip("ION的起始地址由结束地址和大小自动计算");
            m_startAddressEdit->setStyleSheet(
                "QLineEdit { "
                "padding: 6px; "
                "border: 1px solid #bdc3c7; "
                "border-radius: 4px; "
                "background-color: #f0f0f0; "
                "color: #7f8c8d; "
                "}"
            );
        } else {
            m_startAddressEdit->setReadOnly(false);
            m_startAddressEdit->setToolTip("");
            m_startAddressEdit->setStyleSheet(
                "QLineEdit { "
                "padding: 6px; "
                "border: 1px solid #bdc3c7; "
                "border-radius: 4px; "
                "background-color: #ffffff; "
                "} "
                "QLineEdit:focus { "
                "border: 2px solid #3498db; "
                "}"
            );
        }
        
        // 自动选择合适的单位显示大小
        QString sizeText;
        QString unit;
        formatSizeForInput(region.size, sizeText, unit);
        
        m_sizeEdit->setText(sizeText);
        m_sizeUnitCombo->setCurrentText(unit);
        
        // 重新连接信号
        m_nameEdit->blockSignals(false);
        m_startAddressEdit->blockSignals(false);
        m_sizeEdit->blockSignals(false);
        m_sizeUnitCombo->blockSignals(false);
    }
}

void MemoryConfigWidget::onConfigFieldChanged()
{
    // 只有在配置字段启用时才处理更改
    if (!m_nameEdit->isEnabled()) {
        return;
    }
    
    // 当配置字段改变时，更新对应的内存区域
    QTableWidgetItem* currentItem = m_memoryTable->currentItem();
    if (!currentItem) {
        return;
    }
    
    int currentRow = currentItem->row();
    if (currentRow < 0 || currentRow >= m_regionOrder.size()) {
        return;
    }
    
    QString regionName = m_regionOrder[currentRow];
    if (!m_memoryRegions.contains(regionName)) {
        return;
    }
    
    MemoryRegion& region = m_memoryRegions[regionName];
    
    // 保存所有旧值，用于验证失败时回滚
    MemoryRegion oldRegion = region;
    QString oldRegionName = regionName;
    QMap<QString, MemoryRegion> oldMemoryRegions = m_memoryRegions;
    
    // 保存旧的size值，用于判断是否有变化
    quint64 oldSize = region.size;
    
    // 更新区域数据
    QString newName = m_nameEdit->text().trimmed();
    if (!newName.isEmpty() && newName != region.name) {
        // 更新名称映射
        m_memoryRegions.remove(regionName);
        m_regionOrder[currentRow] = newName;
        region.name = newName;
        m_memoryRegions[newName] = region;
        regionName = newName;
    }
    
    // 更新起始地址（ION区域除外，因为ION的Start Address需要基于End Address和Size计算）
    if (regionName != "ION") {
        region.startAddress = parseAddress(m_startAddressEdit->text());
    }
    
    // 更新大小
    bool ok;
    double sizeValue = m_sizeEdit->text().toDouble(&ok);
    if (ok && sizeValue >= 0) {
        QString unit = m_sizeUnitCombo->currentText();
        quint64 actualSize = 0;
        if (unit == "K") {
            actualSize = static_cast<quint64>(sizeValue * 1024.0);
        } else if (unit == "M") {
            actualSize = static_cast<quint64>(sizeValue * 1024.0 * 1024.0);
        } else if (unit == "G") {
            actualSize = static_cast<quint64>(sizeValue * 1024.0 * 1024.0 * 1024.0);
        } else {
            // B
            actualSize = static_cast<quint64>(sizeValue);
        }
        
        region.size = actualSize;
        
        // 特殊处理：ION和RTOS_ION区域在size改变时需要调整Start Address
        if (regionName == "ION") {
            // ION区域：End Address固定不变，通过调整Start Address来适应新的size
            // 保持当前的End Address，向前调整Start Address
            // 如果是初始化状态，使用默认的End Address
            if (region.endAddress == 0 || region.endAddress == region.startAddress + oldSize) {
                // 使用当前的End Address（如果已经设置）或计算默认值
                if (region.endAddress == 0) {
                    // 初始化：使用BOOTLOGO的起始地址作为ION的End Address
                    region.endAddress = 0x89e3e000;
                }
            }
            // 基于固定的End Address和新的size计算Start Address
            region.startAddress = region.endAddress - actualSize;
            
            // 同步更新H26X_BITSTREAM、H26X_ENC_BUFF、ISP_MEM_BASE的Start Address
            // 这些区域与ION共享起始地址
            QStringList relatedRegions = {"H26X_BITSTREAM", "H26X_ENC_BUFF", "ISP_MEM_BASE"};
            for (const QString& relatedName : relatedRegions) {
                if (m_memoryRegions.contains(relatedName)) {
                    m_memoryRegions[relatedName].startAddress = region.startAddress;
                    m_memoryRegions[relatedName].endAddress = m_memoryRegions[relatedName].startAddress + m_memoryRegions[relatedName].size;
                    // 更新相关区域的sizeString
                    m_memoryRegions[relatedName].sizeString = formatSize(m_memoryRegions[relatedName].size);
                }
            }
        } else if (regionName == "RTOS_ION" && actualSize != oldSize) {
            // RTOS_ION区域：调整Start Address，End Address固定在总内存末尾
            // 总内存末尾：0x80000000 + 0x10000000 = 0x90000000
            quint64 memoryEnd = MEMORY_BASE_ADDRESS + TOTAL_MEMORY_SIZE;
            region.endAddress = memoryEnd;
            region.startAddress = region.endAddress - actualSize;
            
            // 当RTOS_ION改变时，需要调整ION及相关区域的Start Address
            // 计算新的ION起始地址：RTOS_ION的Start Address之前
            quint64 newIonStartAddress = region.startAddress;
            
            if (m_memoryRegions.contains("ION")) {
                quint64 ionSize = m_memoryRegions["ION"].size;
                m_memoryRegions["ION"].startAddress = newIonStartAddress - ionSize;
                m_memoryRegions["ION"].endAddress = m_memoryRegions["ION"].startAddress + ionSize;
                // 更新ION的sizeString
                m_memoryRegions["ION"].sizeString = formatSize(ionSize);
                
                // H26X_BITSTREAM、H26X_ENC_BUFF、ISP_MEM_BASE与ION共享Start Address
                QStringList relatedRegions = {"H26X_BITSTREAM", "H26X_ENC_BUFF", "ISP_MEM_BASE"};
                for (const QString& relatedName : relatedRegions) {
                    if (m_memoryRegions.contains(relatedName)) {
                        m_memoryRegions[relatedName].startAddress = m_memoryRegions["ION"].startAddress;
                        m_memoryRegions[relatedName].endAddress = m_memoryRegions[relatedName].startAddress + m_memoryRegions[relatedName].size;
                        // 更新相关区域的sizeString
                        m_memoryRegions[relatedName].sizeString = formatSize(m_memoryRegions[relatedName].size);
                    }
                }
            }
        } else {
            // 其他区域：基于start address + size计算end address
            region.endAddress = region.startAddress + region.size;
        }
        
        // 确保sizeString使用实际的size值更新
        region.sizeString = formatSize(actualSize);
    }
    
    // 先更新内存区域映射中的数据
    m_memoryRegions[regionName] = region;
    
    // 更新表格显示
    updateSizeString(currentRow);
    
    // 如果是ION或RTOS_ION，需要更新相关区域的表格显示
    if (regionName == "ION" || regionName == "RTOS_ION") {
        QStringList relatedRegions;
        if (regionName == "ION") {
            relatedRegions = {"H26X_BITSTREAM", "H26X_ENC_BUFF", "ISP_MEM_BASE"};
        } else {
            relatedRegions = {"ION", "H26X_BITSTREAM", "H26X_ENC_BUFF", "ISP_MEM_BASE"};
        }
        
        for (const QString& relatedName : relatedRegions) {
            int relatedRow = m_regionOrder.indexOf(relatedName);
            if (relatedRow >= 0) {
                updateSizeString(relatedRow);
                
                // 更新表格中的地址
                if (QTableWidgetItem* startItem = m_memoryTable->item(relatedRow, COL_START_ADDRESS)) {
                    startItem->setText(formatAddress(m_memoryRegions[relatedName].startAddress));
                }
                if (QTableWidgetItem* endItem = m_memoryTable->item(relatedRow, COL_END_ADDRESS)) {
                    endItem->setText(formatAddress(m_memoryRegions[relatedName].endAddress));
                }
            }
        }
    }
    
    // 更新表格中的名称和地址
    if (QTableWidgetItem* nameItem = m_memoryTable->item(currentRow, COL_NAME)) {
        nameItem->setText(region.name);
    }
    if (QTableWidgetItem* startItem = m_memoryTable->item(currentRow, COL_START_ADDRESS)) {
        startItem->setText(formatAddress(region.startAddress));
    }
    if (QTableWidgetItem* endItem = m_memoryTable->item(currentRow, COL_END_ADDRESS)) {
        endItem->setText(formatAddress(region.endAddress));
    }
    
    // 验证内存约束
    QString errorMessage;
    if (!validateMemoryConstraints(errorMessage)) {
        // 验证失败，回滚所有修改
        m_memoryRegions = oldMemoryRegions;
        
        // 如果名称改变了，需要恢复顺序列表
        if (oldRegionName != region.name) {
            m_regionOrder[currentRow] = oldRegionName;
        }
        
        // 显示错误信息
        QMessageBox::warning(this, "内存配置约束错误", 
                           QString("当前配置违反了内存约束条件:\n\n%1\n\n"
                                  "修改已被撤销。").arg(errorMessage));
        
        // 恢复配置面板显示（需要阻塞信号避免再次触发）
        m_nameEdit->blockSignals(true);
        m_startAddressEdit->blockSignals(true);
        m_sizeEdit->blockSignals(true);
        m_sizeUnitCombo->blockSignals(true);
        
        m_nameEdit->setText(oldRegion.name);
        m_startAddressEdit->setText(formatAddress(oldRegion.startAddress));
        
        QString sizeText;
        QString unit;
        formatSizeForInput(oldRegion.size, sizeText, unit);
        m_sizeEdit->setText(sizeText);
        m_sizeUnitCombo->setCurrentText(unit);
        
        m_nameEdit->blockSignals(false);
        m_startAddressEdit->blockSignals(false);
        m_sizeEdit->blockSignals(false);
        m_sizeUnitCombo->blockSignals(false);
        
        // 确保恢复后的数据sizeString是正确的
        for (auto it = m_memoryRegions.begin(); it != m_memoryRegions.end(); ++it) {
            it.value().sizeString = formatSize(it.value().size);
        }
        
        // 恢复表格显示
        updateSizeString(currentRow);
        
        // 如果是ION或RTOS_ION，需要恢复相关区域的表格显示
        if (oldRegionName == "ION" || oldRegionName == "RTOS_ION") {
            QStringList relatedRegions;
            if (oldRegionName == "ION") {
                relatedRegions = {"H26X_BITSTREAM", "H26X_ENC_BUFF", "ISP_MEM_BASE"};
            } else {
                relatedRegions = {"ION", "H26X_BITSTREAM", "H26X_ENC_BUFF", "ISP_MEM_BASE"};
            }
            
            for (const QString& relatedName : relatedRegions) {
                int relatedRow = m_regionOrder.indexOf(relatedName);
                if (relatedRow >= 0) {
                    updateSizeString(relatedRow);
                    
                    // 更新表格中的地址
                    if (QTableWidgetItem* startItem = m_memoryTable->item(relatedRow, COL_START_ADDRESS)) {
                        startItem->setText(formatAddress(m_memoryRegions[relatedName].startAddress));
                    }
                    if (QTableWidgetItem* endItem = m_memoryTable->item(relatedRow, COL_END_ADDRESS)) {
                        endItem->setText(formatAddress(m_memoryRegions[relatedName].endAddress));
                    }
                }
            }
        }
        
        // 更新表格中的名称和地址
        if (QTableWidgetItem* nameItem = m_memoryTable->item(currentRow, COL_NAME)) {
            nameItem->setText(oldRegionName);
        }
        if (QTableWidgetItem* startItem = m_memoryTable->item(currentRow, COL_START_ADDRESS)) {
            startItem->setText(formatAddress(oldRegion.startAddress));
        }
        if (QTableWidgetItem* endItem = m_memoryTable->item(currentRow, COL_END_ADDRESS)) {
            endItem->setText(formatAddress(oldRegion.endAddress));
        }
        
        // 更新内存可视化（使用回滚后的数据）
        updateMemoryVisualization();
        
        return;  // 不发送信号，因为修改被撤销了
    }
    
    // 验证通过，更新内存可视化
    updateMemoryVisualization();
    validateMemoryLayout();
    
    // 发送信号
    emit configChanged();
    emit memoryRegionChanged(region.name);
}

void MemoryConfigWidget::formatSizeForInput(quint64 sizeInBytes, QString& sizeText, QString& unit)
{
    const quint64 KB = 1024;
    const quint64 MB = KB * 1024;
    const quint64 GB = MB * 1024;
    
    if (sizeInBytes == 0) {
        sizeText = "0";
        unit = "B";
    } else if (sizeInBytes >= GB) {
        if (sizeInBytes % GB == 0) {
            sizeText = QString::number(sizeInBytes / GB);
            unit = "G";
        } else {
            // 显示小数点后最多2位
            double gbValue = static_cast<double>(sizeInBytes) / GB;
            sizeText = QString::number(gbValue, 'f', 2).remove(QRegularExpression("\\.?0+$"));
            unit = "G";
        }
    } else if (sizeInBytes >= MB) {
        if (sizeInBytes % MB == 0) {
            sizeText = QString::number(sizeInBytes / MB);
            unit = "M";
        } else {
            // 显示小数点后最多2位
            double mbValue = static_cast<double>(sizeInBytes) / MB;
            sizeText = QString::number(mbValue, 'f', 2).remove(QRegularExpression("\\.?0+$"));
            unit = "M";
        }
    } else if (sizeInBytes >= KB) {
        if (sizeInBytes % KB == 0) {
            sizeText = QString::number(sizeInBytes / KB);
            unit = "K";
        } else {
            // 显示小数点后最多2位
            double kbValue = static_cast<double>(sizeInBytes) / KB;
            sizeText = QString::number(kbValue, 'f', 2).remove(QRegularExpression("\\.?0+$"));
            unit = "K";
        }
    } else {
        sizeText = QString::number(sizeInBytes);
        unit = "B";
    }
}

void MemoryConfigWidget::updateMemoryVisualization()
{
    QString memoryMap;
    
    // 创建内存映射文本
    memoryMap += "内存布局可视化:\n";
    memoryMap += "========================================\n";
    
    // 按地址排序的区域列表
    QList<MemoryRegion> sortedRegions;
    for (const auto& region : m_memoryRegions) {
        if (region.size > 0) {
            sortedRegions.append(region);
        }
    }
    
    std::sort(sortedRegions.begin(), sortedRegions.end(), 
              [](const MemoryRegion& a, const MemoryRegion& b) {
                  return a.startAddress < b.startAddress;
              });
    
    // 显示每个区域
    for (const auto& region : sortedRegions) {
        memoryMap += QString("%1: %2 - %3 (%4)\n")
                    .arg(region.name, -20)
                    .arg(formatAddress(region.startAddress))
                    .arg(formatAddress(region.endAddress))
                    .arg(region.sizeString);
    }
    
    m_memoryMapText->setPlainText(memoryMap);
}

void MemoryConfigWidget::validateMemoryLayout()
{
    // 检查内存重叠
    checkMemoryOverlap();
}

void MemoryConfigWidget::checkMemoryOverlap()
{
    // 按地址排序的区域列表
    QList<MemoryRegion> sortedRegions;
    for (const auto& region : m_memoryRegions) {
        if (region.size > 0) {
            sortedRegions.append(region);
        }
    }
    
    std::sort(sortedRegions.begin(), sortedRegions.end(), 
              [](const MemoryRegion& a, const MemoryRegion& b) {
                  return a.startAddress < b.startAddress;
              });
    
    // 检查重叠
    for (int i = 0; i < sortedRegions.size() - 1; ++i) {
        const MemoryRegion& current = sortedRegions[i];
        const MemoryRegion& next = sortedRegions[i + 1];
        
        if (current.endAddress > next.startAddress) {
            qDebug() << "内存重叠检测：" << current.name << "与" << next.name << "存在重叠";
            // 可以在这里添加UI提示
        }
    }
}

bool MemoryConfigWidget::validateMemoryConstraints(QString& errorMessage)
{
    // 辅助函数：获取区域的起始地址
    auto getAddr = [this](const QString& name) -> quint64 {
        if (m_memoryRegions.contains(name)) {
            return m_memoryRegions[name].startAddress;
        }
        return 0;
    };
    
    // 辅助函数：获取区域的大小
    auto getSize = [this](const QString& name) -> quint64 {
        if (m_memoryRegions.contains(name)) {
            return m_memoryRegions[name].size;
        }
        return 0;
    };
    
    // 计算RTOS_SYS_SIZE（这是一个计算值，需要根据实际情况定义）
    // 从FSBL_C906L_START到RTOS_COMPRESS_BIN之间的区域总和
    quint64 rtosLogSize = getSize("RTOS_LOG");
    quint64 shareMemSize = getSize("SHARE_MEM");
    quint64 shareParamSize = getSize("SHARE_PARAM");
    quint64 pqbinSize = getSize("PQBIN");
    quint64 rtosLogoSize = getSize("RTOS_LOGO");
    quint64 cviUpdateHeaderSize = getSize("CVI_UPDATE_HEADER");
    quint64 fsblUnzipSize = getSize("FSBL_UNZIP");
    quint64 uimagSize = getSize("UIMAG");
    
    // RTOS_SYS_SIZE = 从FSBL_C906L_START到所有RTOS相关区域的总大小
    // 简化计算：使用RTOS_COMPRESS_BIN的地址 - FSBL_C906L_START的地址作为RTOS_SYS_SIZE
    quint64 fsblC906lStartAddr = getAddr("FSBL_C906L_START");
    quint64 rtosCompressBinAddr = getAddr("RTOS_COMPRESS_BIN");
    quint64 rtosSysSize = rtosCompressBinAddr > fsblC906lStartAddr ? 
                          rtosCompressBinAddr - fsblC906lStartAddr : 0;
    
    // 约束1: RTOS_ION_ADDR >= FSBL_C906L_START_ADDR + RTOS_SYS_SIZE
    quint64 rtosIonAddr = getAddr("RTOS_ION");
    if (rtosIonAddr < fsblC906lStartAddr + rtosSysSize) {
        errorMessage = QString("约束违反: RTOS_ION地址(0x%1) 必须 >= FSBL_C906L_START地址(0x%2) + RTOS_SYS_SIZE(0x%3)")
                      .arg(rtosIonAddr, 0, 16)
                      .arg(fsblC906lStartAddr, 0, 16)
                      .arg(rtosSysSize, 0, 16);
        return false;
    }
    
    // 约束2: RTOS_COMPRESS_BIN_ADDR >= UIMAG_ADDR + UIMAG_SIZE
    quint64 uimagAddr = getAddr("UIMAG");
    if (rtosCompressBinAddr < uimagAddr + uimagSize) {
        errorMessage = QString("约束违反: RTOS_COMPRESS_BIN地址(0x%1) 必须 >= UIMAG地址(0x%2) + UIMAG大小(0x%3)")
                      .arg(rtosCompressBinAddr, 0, 16)
                      .arg(uimagAddr, 0, 16)
                      .arg(uimagSize, 0, 16);
        return false;
    }
    
    // 约束3: ION_ADDR >= FSBL_C906L_START_ADDR + RTOS_SYS_SIZE
    quint64 ionAddr = getAddr("ION");
    if (ionAddr < fsblC906lStartAddr + rtosSysSize) {
        errorMessage = QString("约束违反: ION地址(0x%1) 必须 >= FSBL_C906L_START地址(0x%2) + RTOS_SYS_SIZE(0x%3)")
                      .arg(ionAddr, 0, 16)
                      .arg(fsblC906lStartAddr, 0, 16)
                      .arg(rtosSysSize, 0, 16);
        return false;
    }
    
    // 注意：CONFIG_SYS_TEXT_BASE和CONFIG_SYS_INIT_SP_ADDR不在当前的内存区域列表中
    // 如果需要检查这些约束，需要添加这些区域到配置中
    // 或者假设它们与UIMAG有关联
    
    // 约束4和5暂时跳过，因为CONFIG_SYS_TEXT_BASE和CONFIG_SYS_INIT_SP_ADDR未定义
    // 如果这些是固定值或需要添加到配置中，请告知
    
    return true;
}

QString MemoryConfigWidget::formatSize(quint64 sizeInBytes)
{
    if (sizeInBytes == 0) {
        return "0M";  // 默认显示为M单位
    }
    
    const quint64 KB = 1024;
    const quint64 MB = KB * 1024;
    const quint64 GB = MB * 1024;
    
    if (sizeInBytes >= GB) {
        return QString("%1G").arg(sizeInBytes / GB);
    } else if (sizeInBytes >= MB) {
        return QString("%1M").arg(sizeInBytes / MB);
    } else if (sizeInBytes >= KB) {
        return QString("%1K").arg(sizeInBytes / KB);
    } else {
        return QString("%1B").arg(sizeInBytes);
    }
}

quint64 MemoryConfigWidget::parseAddress(const QString& addressStr)
{
    QString cleanStr = addressStr.trimmed();
    bool ok;
    quint64 address;
    
    if (cleanStr.startsWith("0x") || cleanStr.startsWith("0X")) {
        address = cleanStr.mid(2).toULongLong(&ok, 16);
    } else {
        address = cleanStr.toULongLong(&ok, 16);
    }
    
    return ok ? address : 0;
}

QString MemoryConfigWidget::formatAddress(quint64 address)
{
    return QString("0x%1").arg(address, 8, 16, QChar('0')).toUpper();
}

MemoryRegion MemoryConfigWidget::getMemoryRegion(const QString& name) const
{
    return m_memoryRegions.value(name, MemoryRegion());
}

void MemoryConfigWidget::setMemoryRegion(const QString& name, const MemoryRegion& region)
{
    m_memoryRegions[name] = region;
    populateTable();
    updateMemoryVisualization();
    emit configChanged();
}

bool MemoryConfigWidget::exportToJson(const QString& filePath)
{
    QJsonObject root;
    QJsonArray regionsArray;
    
    for (const auto& region : m_memoryRegions) {
        QJsonObject regionObj;
        regionObj["name"] = region.name;
        regionObj["startAddress"] = QString("0x%1").arg(region.startAddress, 0, 16);
        regionObj["endAddress"] = QString("0x%1").arg(region.endAddress, 0, 16);
        regionObj["size"] = QString("0x%1").arg(region.size, 0, 16);
        regionObj["sizeString"] = region.sizeString;
        regionObj["isEditable"] = region.isEditable;
        regionObj["description"] = region.description;
        
        regionsArray.append(regionObj);
    }
    
    root["memoryRegions"] = regionsArray;
    root["totalMemorySize"] = QString("0x%1").arg(TOTAL_MEMORY_SIZE, 0, 16);
    root["memoryBaseAddress"] = QString("0x%1").arg(MEMORY_BASE_ADDRESS, 0, 16);
    
    QJsonDocument doc(root);
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    file.write(doc.toJson());
    file.close();
    
    return true;
}

bool MemoryConfigWidget::importFromJson(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        return false;
    }
    
    QJsonObject root = doc.object();
    QJsonArray regionsArray = root["memoryRegions"].toArray();
    
    m_memoryRegions.clear();
    
    for (const auto& value : regionsArray) {
        QJsonObject regionObj = value.toObject();
        
        MemoryRegion region;
        region.name = regionObj["name"].toString();
        region.startAddress = parseAddress(regionObj["startAddress"].toString());
        region.endAddress = parseAddress(regionObj["endAddress"].toString());
        region.size = parseAddress(regionObj["size"].toString());
        region.sizeString = regionObj["sizeString"].toString();
        region.isEditable = regionObj["isEditable"].toBool();
        region.description = regionObj["description"].toString();
        
        m_memoryRegions[region.name] = region;
    }
    
    return true;
}

void MemoryConfigWidget::setSourcePath(const QString& sourcePath)
{
    m_sourcePath = sourcePath;
}

void MemoryConfigWidget::setChipType(const QString& chipType)
{
    m_chipType = chipType;
}

bool MemoryConfigWidget::exportToDefconfig(const QString& sourcePath, const QString& chipType)
{
    // 构建defconfig文件路径
    QString defconfigPath = QString("%1/build/boards/cv184x/%2/%2_defconfig")
                           .arg(sourcePath)
                           .arg(chipType);
    
    // 检查文件是否存在
    QFile defconfigFile(defconfigPath);
    if (!defconfigFile.exists()) {
        QMessageBox::critical(nullptr, "错误", 
                            QString("Defconfig文件不存在: %1").arg(defconfigPath));
        return false;
    }
    
    // 读取现有文件内容
    if (!defconfigFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(nullptr, "错误", 
                            QString("无法读取defconfig文件: %1").arg(defconfigPath));
        return false;
    }
    
    QTextStream in(&defconfigFile);
    QStringList lines;
    while (!in.atEnd()) {
        lines.append(in.readLine());
    }
    defconfigFile.close();
    
    // 获取相关内存区域的大小
    quint64 ionSize = 0;
    quint64 rtosIonSize = 0;
    
    // 从内存区域映射中获取大小
    if (m_memoryRegions.contains("ION")) {
        ionSize = m_memoryRegions["ION"].size;
    }
    if (m_memoryRegions.contains("RTOS_ION")) {
        rtosIonSize = m_memoryRegions["RTOS_ION"].size;
    }
    
    // 转换为十六进制字符串（不包含0x前缀）
    QString ionSizeHex = QString("%1").arg(ionSize, 0, 16);
    QString rtosIonSizeHex = QString("%1").arg(rtosIonSize, 0, 16);
    
    // 更新配置行
    bool foundIonSize = false;
    bool foundRtosIonSize = false;
    
    for (int i = 0; i < lines.size(); ++i) {
        QString& line = lines[i];
        
        if (line.startsWith("CONFIG_ION_SIZE=")) {
            line = QString("CONFIG_ION_SIZE=0x%1").arg(ionSizeHex);
            foundIonSize = true;
        } else if (line.startsWith("CONFIG_RTOS_ION_SIZE=")) {
            line = QString("CONFIG_RTOS_ION_SIZE=0x%1").arg(rtosIonSizeHex);
            foundRtosIonSize = true;
        }
    }
    
    // 如果没有找到配置项，则添加到文件末尾
    if (!foundIonSize) {
        lines.append(QString("CONFIG_ION_SIZE=0x%1").arg(ionSizeHex));
    }
    if (!foundRtosIonSize) {
        lines.append(QString("CONFIG_RTOS_ION_SIZE=0x%1").arg(rtosIonSizeHex));
    }
    
    // 写回文件
    if (!defconfigFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(nullptr, "错误", 
                            QString("无法写入defconfig文件: %1").arg(defconfigPath));
        return false;
    }
    
    QTextStream out(&defconfigFile);
    for (const QString& line : lines) {
        out << line << "\n";
    }
    defconfigFile.close();
    
    return true;
}