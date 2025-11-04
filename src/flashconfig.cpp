#include "flashconfig.h"
#include <QApplication>
#include <QDebug>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QFont>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QRegularExpression>
#include <QCheckBox>

FlashConfigWidget::FlashConfigWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_splitter(nullptr)
    , m_tableWidget(nullptr)
    , m_tableLayout(nullptr)
    , m_partitionTable(nullptr)
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
    , m_labelEdit(nullptr)
    , m_sizeEdit(nullptr)
    , m_sizeUnitCombo(nullptr)
    , m_fileEdit(nullptr)
    , m_mountpointEdit(nullptr)
    , m_typeEdit(nullptr)
    , m_configExpanded(false)
    , m_visualizationGroup(nullptr)
    , m_visualizationLayout(nullptr)
    , m_partitionMapText(nullptr)
    , m_sourcePath("")
    , m_chipType("")
    , m_flashSize("32GB")
    , m_partitionCount(9)
{
    setupUI();
    initializePartitions();
    populateTable();
    updatePartitionVisualization();
}

FlashConfigWidget::~FlashConfigWidget()
{
}

void FlashConfigWidget::setupUI()
{
    // 创建主布局
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    m_mainLayout->setSpacing(10);
    
    // 创建分隔器（水平分隔：左侧表格，右侧控制面板）
    m_splitter = new QSplitter(Qt::Horizontal, this);
    
    // 设置表格区域
    setupPartitionTable();
    
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

void FlashConfigWidget::setupPartitionTable()
{
    // 创建表格区域
    m_tableWidget = new QWidget();
    m_tableLayout = new QVBoxLayout(m_tableWidget);
    m_tableLayout->setContentsMargins(5, 5, 5, 5);
    m_tableLayout->setSpacing(5);
    
    // 创建标题
    m_tableTitle = new QLabel("Flash 分区配置", m_tableWidget);
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
    
    // 创建分区表格
    m_partitionTable = new QTableWidget(0, COL_COUNT, m_tableWidget);
    
    // 设置表头
    QStringList headers;
    headers << "分区号" << "启用" << "标签" << "大小(KB)" << "文件" << "挂载点" << "类型";
    m_partitionTable->setHorizontalHeaderLabels(headers);
    
    // 设置表格属性
    m_partitionTable->setAlternatingRowColors(true);
    m_partitionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_partitionTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_partitionTable->setSortingEnabled(false);
    
    // 设置表格样式
    m_partitionTable->setStyleSheet(
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
    QHeaderView* header = m_partitionTable->horizontalHeader();
    header->setStretchLastSection(true);
    header->resizeSection(COL_PARTITION_NUM, 80);
    header->resizeSection(COL_ENABLED, 80);
    header->resizeSection(COL_LABEL, 150);
    header->resizeSection(COL_SIZE, 120);
    header->resizeSection(COL_FILE, 150);
    header->resizeSection(COL_MOUNTPOINT, 120);
    header->resizeSection(COL_TYPE, 100);
    
    // 连接双击信号
    connect(m_partitionTable, &QTableWidget::itemDoubleClicked, 
            this, &FlashConfigWidget::onTableItemDoubleClicked);
    
    // 添加到布局
    m_tableLayout->addWidget(m_tableTitle);
    m_tableLayout->addWidget(m_partitionTable);
}

void FlashConfigWidget::setupControlPanel()
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
    
    m_addButton = new QPushButton("添加分区", m_operationGroup);
    m_removeButton = new QPushButton("删除分区", m_operationGroup);
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
    
    // 分区搜索组
    m_searchGroup = new QGroupBox("分区搜索", m_controlPanel);
    m_searchLayout = new QVBoxLayout(m_searchGroup);
    m_searchLayout->setSpacing(5);
    
    m_searchEdit = new QLineEdit(m_searchGroup);
    m_searchEdit->setPlaceholderText("搜索分区标签...");
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
    
    // 分区配置可折叠面板
    m_configContainer = new QWidget(m_controlPanel);
    m_configContainerLayout = new QVBoxLayout(m_configContainer);
    m_configContainerLayout->setContentsMargins(0, 0, 0, 0);
    m_configContainerLayout->setSpacing(0);
    
    // 配置面板头部
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
    
    m_configTitleLabel = new QLabel("分区配置", m_configHeader);
    m_configTitleLabel->setStyleSheet(
        "QLabel { "
        "font-weight: bold; "
        "color: #2c3e50; "
        "background: transparent; "
        "border: none; "
        "}"
    );
    
    m_configToggleButton = new QToolButton(m_configHeader);
    m_configToggleButton->setArrowType(Qt::RightArrow);
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
    m_configToggleButton->setToolTip("展开/收起分区配置");
    
    m_configHeaderLayout->addWidget(m_configTitleLabel);
    m_configHeaderLayout->addStretch();
    m_configHeaderLayout->addWidget(m_configToggleButton);
    
    // 配置面板内容
    m_configContent = new QFrame(m_configContainer);
    m_configContent->setStyleSheet(
        "QFrame { "
        "border: 1px solid #bdc3c7; "
        "border-top: none; "
        "border-radius: 0 0 4px 4px; "
        "background-color: #ffffff; "
        "}"
    );
    m_configContent->setVisible(false);
    
    m_configLayout = new QVBoxLayout(m_configContent);
    m_configLayout->setContentsMargins(8, 8, 8, 8);
    m_configLayout->setSpacing(8);
    
    // Label字段
    QHBoxLayout* labelLayout = new QHBoxLayout();
    labelLayout->addWidget(new QLabel("标签:"));
    m_labelEdit = new QLineEdit(m_configContent);
    m_labelEdit->setPlaceholderText("输入分区标签");
    m_labelEdit->setEnabled(false);
    labelLayout->addWidget(m_labelEdit);
    
    // Size字段 - 只支持KB单位
    QHBoxLayout* sizeLayout = new QHBoxLayout();
    sizeLayout->addWidget(new QLabel("大小(KB):"));
    m_sizeEdit = new QLineEdit(m_configContent);
    m_sizeEdit->setPlaceholderText("1024 (必须是64的倍数)");
    m_sizeEdit->setEnabled(false);
    m_sizeUnitCombo = new QComboBox(m_configContent);
    m_sizeUnitCombo->addItems({"KB"});
    m_sizeUnitCombo->setCurrentText("KB");
    m_sizeUnitCombo->setEnabled(false);
    m_sizeUnitCombo->setVisible(false); // 隐藏单位选择，因为固定为KB
    sizeLayout->addWidget(m_sizeEdit);
    sizeLayout->addWidget(m_sizeUnitCombo);
    
    // File字段
    QHBoxLayout* fileLayout = new QHBoxLayout();
    fileLayout->addWidget(new QLabel("文件:"));
    m_fileEdit = new QLineEdit(m_configContent);
    m_fileEdit->setPlaceholderText("文件名");
    m_fileEdit->setEnabled(false);
    fileLayout->addWidget(m_fileEdit);
    
    // Mountpoint字段
    QHBoxLayout* mountpointLayout = new QHBoxLayout();
    mountpointLayout->addWidget(new QLabel("挂载点:"));
    m_mountpointEdit = new QLineEdit(m_configContent);
    m_mountpointEdit->setPlaceholderText("/mnt/xxx");
    m_mountpointEdit->setEnabled(false);
    mountpointLayout->addWidget(m_mountpointEdit);
    
    // Type字段
    QHBoxLayout* typeLayout = new QHBoxLayout();
    typeLayout->addWidget(new QLabel("类型:"));
    m_typeEdit = new QLineEdit(m_configContent);
    m_typeEdit->setPlaceholderText("ext4");
    m_typeEdit->setEnabled(false);
    typeLayout->addWidget(m_typeEdit);
    
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
    
    m_labelEdit->setStyleSheet(fieldStyle);
    m_sizeEdit->setStyleSheet(fieldStyle);
    m_sizeUnitCombo->setStyleSheet(fieldStyle);
    m_fileEdit->setStyleSheet(fieldStyle);
    m_mountpointEdit->setStyleSheet(fieldStyle);
    m_typeEdit->setStyleSheet(fieldStyle);
    
    m_configLayout->addLayout(labelLayout);
    m_configLayout->addLayout(sizeLayout);
    m_configLayout->addLayout(fileLayout);
    m_configLayout->addLayout(mountpointLayout);
    m_configLayout->addLayout(typeLayout);
    
    // 将头部和内容添加到容器
    m_configContainerLayout->addWidget(m_configHeader);
    m_configContainerLayout->addWidget(m_configContent);
    
    // 分区可视化组
    m_visualizationGroup = new QGroupBox("分区映射", m_controlPanel);
    m_visualizationLayout = new QVBoxLayout(m_visualizationGroup);
    
    m_partitionMapText = new QTextEdit(m_visualizationGroup);
    m_partitionMapText->setReadOnly(true);
    m_partitionMapText->setMaximumHeight(150);
    m_partitionMapText->setStyleSheet(
        "QTextEdit { "
        "background-color: #2c3e50; "
        "color: #ecf0f1; "
        "border: 1px solid #34495e; "
        "border-radius: 4px; "
        "font-family: 'Courier New', monospace; "
        "font-size: 10px; "
        "}"
    );
    
    m_visualizationLayout->addWidget(m_partitionMapText);
    
    // 添加到控制面板布局
    m_controlLayout->addWidget(m_operationGroup);
    m_controlLayout->addWidget(m_searchGroup);
    m_controlLayout->addWidget(m_configContainer);
    m_controlLayout->addWidget(m_visualizationGroup);
    m_controlLayout->addStretch();
    
    // 连接信号
    connect(m_addButton, &QPushButton::clicked, this, &FlashConfigWidget::onAddPartition);
    connect(m_removeButton, &QPushButton::clicked, this, &FlashConfigWidget::onRemovePartition);
    connect(m_resetButton, &QPushButton::clicked, this, &FlashConfigWidget::onResetPartitions);
    connect(m_exportButton, &QPushButton::clicked, this, &FlashConfigWidget::onExportConfig);
    connect(m_importButton, &QPushButton::clicked, this, &FlashConfigWidget::onImportConfig);
    
    connect(m_searchEdit, &QLineEdit::textChanged, this, &FlashConfigWidget::onSearchTextChanged);
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &FlashConfigWidget::onSearchEnterPressed);
    connect(m_configToggleButton, &QToolButton::clicked, this, &FlashConfigWidget::onToggleConfigPanel);
    connect(m_partitionTable, &QTableWidget::itemSelectionChanged, 
            this, &FlashConfigWidget::onTableSelectionChanged);
    
    connect(m_labelEdit, &QLineEdit::editingFinished, this, &FlashConfigWidget::onConfigFieldChanged);
    connect(m_sizeEdit, &QLineEdit::editingFinished, this, &FlashConfigWidget::onConfigFieldChanged);
    connect(m_sizeUnitCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &FlashConfigWidget::onConfigFieldChanged);
    connect(m_fileEdit, &QLineEdit::editingFinished, this, &FlashConfigWidget::onConfigFieldChanged);
    connect(m_mountpointEdit, &QLineEdit::editingFinished, this, &FlashConfigWidget::onConfigFieldChanged);
    connect(m_typeEdit, &QLineEdit::editingFinished, this, &FlashConfigWidget::onConfigFieldChanged);
}

void FlashConfigWidget::initializePartitions()
{
    // 根据用户提供的defconfig示例初始化分区
    m_partitions.clear();
    m_partitionOrder.clear();
    
    // 分区2: 2nd
    FlashPartition p2;
    p2.partitionNumber = 2;
    p2.label = "2nd";
    p2.size = 3072;
    p2.sizeString = formatSize(p2.size);
    p2.file = "yoc.bin";
    p2.mountpoint = "";
    p2.type = "";
    p2.enabled = true;
    m_partitions[2] = p2;
    m_partitionOrder.append(2);
    
    // 分区3: BOOT
    FlashPartition p3;
    p3.partitionNumber = 3;
    p3.label = "BOOT";
    p3.size = 8192;
    p3.sizeString = formatSize(p3.size);
    p3.file = "boot.emmc";
    p3.mountpoint = "";
    p3.type = "";
    p3.enabled = true;
    m_partitions[3] = p3;
    m_partitionOrder.append(3);
    
    // 分区4: MISC
    FlashPartition p4;
    p4.partitionNumber = 4;
    p4.label = "MISC";
    p4.size = 512;
    p4.sizeString = formatSize(p4.size);
    p4.file = "logo.jpg";
    p4.mountpoint = "";
    p4.type = "";
    p4.enabled = true;
    m_partitions[4] = p4;
    m_partitionOrder.append(4);
    
    // 分区5: ENV
    FlashPartition p5;
    p5.partitionNumber = 5;
    p5.label = "ENV";
    p5.size = 128;
    p5.sizeString = formatSize(p5.size);
    p5.file = "";
    p5.mountpoint = "";
    p5.type = "";
    p5.enabled = true;
    m_partitions[5] = p5;
    m_partitionOrder.append(5);
    
    // 分区6: ROOTFS
    FlashPartition p6;
    p6.partitionNumber = 6;
    p6.label = "ROOTFS";
    p6.size = 70656;
    p6.sizeString = formatSize(p6.size);
    p6.file = "rootfs.emmc";
    p6.mountpoint = "";
    p6.type = "";
    p6.enabled = true;
    m_partitions[6] = p6;
    m_partitionOrder.append(6);
    
    // 分区7: SYSTEM
    FlashPartition p7;
    p7.partitionNumber = 7;
    p7.label = "SYSTEM";
    p7.size = 40960;
    p7.sizeString = formatSize(p7.size);
    p7.file = "system.emmc";
    p7.mountpoint = "/mnt/system";
    p7.type = "ext4";
    p7.enabled = true;
    m_partitions[7] = p7;
    m_partitionOrder.append(7);
    
    // 分区8: CFG
    FlashPartition p8;
    p8.partitionNumber = 8;
    p8.label = "CFG";
    p8.size = 15240;
    p8.sizeString = formatSize(p8.size);
    p8.file = "cfg.emmc";
    p8.mountpoint = "mnt/cfg";
    p8.type = "ext4";
    p8.enabled = true;
    m_partitions[8] = p8;
    m_partitionOrder.append(8);
    
    // 分区9: DATA
    FlashPartition p9;
    p9.partitionNumber = 9;
    p9.label = "DATA";
    p9.size = 3145728;
    p9.sizeString = formatSize(p9.size);
    p9.file = "data.emmc";
    p9.mountpoint = "mnt/data";
    p9.type = "ext4";
    p9.enabled = true;
    m_partitions[9] = p9;
    m_partitionOrder.append(9);
}

void FlashConfigWidget::populateTable()
{
    // 暂时断开信号
    m_partitionTable->blockSignals(true);
    
    // 清空表格
    m_partitionTable->setRowCount(0);
    
    // 按顺序填充表格
    for (int i = 0; i < m_partitionOrder.size(); ++i) {
        int partNum = m_partitionOrder[i];
        if (!m_partitions.contains(partNum)) continue;
        
        const FlashPartition& partition = m_partitions[partNum];
        
        m_partitionTable->insertRow(i);
        
        // 分区号列
        QTableWidgetItem* numItem = new QTableWidgetItem(QString::number(partition.partitionNumber));
        numItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        m_partitionTable->setItem(i, COL_PARTITION_NUM, numItem);
        
        // 启用列 - 使用复选框
        QTableWidgetItem* enabledItem = new QTableWidgetItem();
        enabledItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
        enabledItem->setCheckState(partition.enabled ? Qt::Checked : Qt::Unchecked);
        m_partitionTable->setItem(i, COL_ENABLED, enabledItem);
        
        // 标签列
        QTableWidgetItem* labelItem = new QTableWidgetItem(partition.label);
        labelItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        m_partitionTable->setItem(i, COL_LABEL, labelItem);
        
        // 大小列
        QTableWidgetItem* sizeItem = new QTableWidgetItem(QString::number(partition.size));
        sizeItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        m_partitionTable->setItem(i, COL_SIZE, sizeItem);
        
        // 文件列
        QTableWidgetItem* fileItem = new QTableWidgetItem(partition.file);
        fileItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        m_partitionTable->setItem(i, COL_FILE, fileItem);
        
        // 挂载点列
        QTableWidgetItem* mountpointItem = new QTableWidgetItem(partition.mountpoint);
        mountpointItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        m_partitionTable->setItem(i, COL_MOUNTPOINT, mountpointItem);
        
        // 类型列
        QTableWidgetItem* typeItem = new QTableWidgetItem(partition.type);
        typeItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        m_partitionTable->setItem(i, COL_TYPE, typeItem);
    }
    
    // 重新启用信号
    m_partitionTable->blockSignals(false);
}

void FlashConfigWidget::onTableItemDoubleClicked(QTableWidgetItem* item)
{
    if (!item) return;
    
    int row = item->row();
    if (row < 0 || row >= m_partitionOrder.size()) return;
    
    // 选择整行
    m_partitionTable->selectRow(row);
    m_partitionTable->setCurrentItem(item);
    
    // 展开配置面板
    setConfigPanelExpanded(true);
    
    // 启用配置字段
    m_labelEdit->setEnabled(true);
    m_sizeEdit->setEnabled(true);
    m_sizeUnitCombo->setEnabled(true);
    m_fileEdit->setEnabled(true);
    m_mountpointEdit->setEnabled(true);
    m_typeEdit->setEnabled(true);
    
    // 更新配置字段内容
    onTableSelectionChanged();
}

void FlashConfigWidget::onAddPartition()
{
    // 查找下一个可用的分区号
    int newPartNum = 2;  // 从分区2开始（分区1是eMMC的fip.bin）
    while (m_partitions.contains(newPartNum)) {
        newPartNum++;
    }
    
    FlashPartition partition;
    partition.partitionNumber = newPartNum;
    partition.label = QString("NEW_PART_%1").arg(newPartNum);
    partition.size = 1024;
    partition.sizeString = formatSize(partition.size);
    partition.file = "";
    partition.mountpoint = "";
    partition.type = "";
    partition.enabled = false;
    
    m_partitions[newPartNum] = partition;
    m_partitionOrder.append(newPartNum);
    
    populateTable();
    updatePartitionVisualization();
    
    emit configChanged();
}

void FlashConfigWidget::onRemovePartition()
{
    QTableWidgetItem* currentItem = m_partitionTable->currentItem();
    if (!currentItem) {
        QMessageBox::warning(this, "警告", "请选择要删除的分区！");
        return;
    }
    
    int currentRow = currentItem->row();
    if (currentRow < 0 || currentRow >= m_partitionOrder.size()) return;
    
    int partNum = m_partitionOrder[currentRow];
    
    int ret = QMessageBox::question(this, "确认删除", 
                                   QString("确定要删除分区 %1 吗？").arg(partNum),
                                   QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        m_partitions.remove(partNum);
        m_partitionOrder.removeAll(partNum);
        populateTable();
        updatePartitionVisualization();
        
        emit configChanged();
    }
}

void FlashConfigWidget::onResetPartitions()
{
    int ret = QMessageBox::question(this, "确认重置", 
                                   "确定要重置所有分区配置吗？这将丢失所有修改！",
                                   QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        initializePartitions();
        populateTable();
        updatePartitionVisualization();
        
        emit configChanged();
    }
}

void FlashConfigWidget::onExportConfig()
{
    if (m_sourcePath.isEmpty() || m_chipType.isEmpty()) {
        QMessageBox::warning(this, "警告", 
                           "请先选择芯片类型和源代码路径！");
        return;
    }
    
    if (exportToDefconfig(m_sourcePath, m_chipType)) {
        QString defconfigPath = getDefconfigPath();
        QMessageBox::information(this, "成功", 
                               QString("Flash分区配置已导出到:\n%1").arg(defconfigPath));
    } else {
        QMessageBox::critical(this, "错误", "导出Flash分区配置失败！");
    }
}

void FlashConfigWidget::onImportConfig()
{
    QString fileName = QFileDialog::getOpenFileName(this, 
        "导入分区配置", 
        "", 
        "JSON Files (*.json);;All Files (*)");
    
    if (!fileName.isEmpty()) {
        if (importFromJson(fileName)) {
            populateTable();
            updatePartitionVisualization();
            QMessageBox::information(this, "成功", "分区配置已导入: " + fileName);
            emit configChanged();
        } else {
            QMessageBox::critical(this, "错误", "导入分区配置失败！");
        }
    }
}

void FlashConfigWidget::onSearchTextChanged(const QString& searchText)
{
    for (int row = 0; row < m_partitionTable->rowCount(); ++row) {
        QTableWidgetItem* labelItem = m_partitionTable->item(row, COL_LABEL);
        if (labelItem) {
            bool visible = searchText.isEmpty() || 
                          labelItem->text().contains(searchText, Qt::CaseInsensitive);
            m_partitionTable->setRowHidden(row, !visible);
        }
    }
}

void FlashConfigWidget::onSearchEnterPressed()
{
    QString searchText = m_searchEdit->text().trimmed();
    if (!searchText.isEmpty()) {
        searchAndSelectPartition(searchText);
    }
}

void FlashConfigWidget::onTableSelectionChanged()
{
    if (!m_configExpanded) {
        return;
    }
    
    QTableWidgetItem* currentItem = m_partitionTable->currentItem();
    if (!currentItem) {
        m_labelEdit->clear();
        m_sizeEdit->clear();
        m_sizeUnitCombo->setCurrentText("KB");
        m_fileEdit->clear();
        m_mountpointEdit->clear();
        m_typeEdit->clear();
        return;
    }
    
    int currentRow = currentItem->row();
    if (currentRow < 0 || currentRow >= m_partitionOrder.size()) {
        m_labelEdit->clear();
        m_sizeEdit->clear();
        m_sizeUnitCombo->setCurrentText("KB");
        m_fileEdit->clear();
        m_mountpointEdit->clear();
        m_typeEdit->clear();
        return;
    }
    
    int partNum = m_partitionOrder[currentRow];
    if (m_partitions.contains(partNum)) {
        const FlashPartition& partition = m_partitions[partNum];
        
        m_labelEdit->blockSignals(true);
        m_sizeEdit->blockSignals(true);
        m_sizeUnitCombo->blockSignals(true);
        m_fileEdit->blockSignals(true);
        m_mountpointEdit->blockSignals(true);
        m_typeEdit->blockSignals(true);
        
        m_labelEdit->setText(partition.label);
        
        // 大小直接显示KB值，不进行单位转换
        m_sizeEdit->setText(QString::number(partition.size));
        m_sizeUnitCombo->setCurrentText("KB");
        
        m_fileEdit->setText(partition.file);
        m_mountpointEdit->setText(partition.mountpoint);
        m_typeEdit->setText(partition.type);
        
        m_labelEdit->blockSignals(false);
        m_sizeEdit->blockSignals(false);
        m_sizeUnitCombo->blockSignals(false);
        m_fileEdit->blockSignals(false);
        m_mountpointEdit->blockSignals(false);
        m_typeEdit->blockSignals(false);
    }
}

void FlashConfigWidget::onConfigFieldChanged()
{
    if (!m_labelEdit->isEnabled()) {
        return;
    }
    
    QTableWidgetItem* currentItem = m_partitionTable->currentItem();
    if (!currentItem) {
        return;
    }
    
    int currentRow = currentItem->row();
    if (currentRow < 0 || currentRow >= m_partitionOrder.size()) {
        return;
    }
    
    int partNum = m_partitionOrder[currentRow];
    if (!m_partitions.contains(partNum)) {
        return;
    }
    
    FlashPartition& partition = m_partitions[partNum];
    
    // 保存旧值，以便验证失败时恢复
    quint64 oldSize = partition.size;
    QString oldLabel = partition.label;
    QString oldFile = partition.file;
    QString oldMountpoint = partition.mountpoint;
    QString oldType = partition.type;
    
    // 更新标签
    partition.label = m_labelEdit->text().trimmed();
    
    // 更新大小 - 只支持KB单位，且必须是64的倍数
    bool ok;
    quint64 sizeValue = m_sizeEdit->text().toULongLong(&ok);
    if (ok && sizeValue >= 0) {
        // 检查是否是64的倍数
        if (sizeValue % 64 != 0) {
            // 显示错误消息
            QMessageBox::warning(this, "大小验证错误", 
                               QString("分区大小必须是64的倍数！\n"
                                      "当前输入: %1 KB\n"
                                      "建议值: %2 KB 或 %3 KB")
                               .arg(sizeValue)
                               .arg((sizeValue / 64) * 64)
                               .arg(((sizeValue / 64) + 1) * 64));
            
            // 恢复旧值
            m_sizeEdit->blockSignals(true);
            m_sizeEdit->setText(QString::number(oldSize));
            m_sizeEdit->blockSignals(false);
            
            return; // 不继续更新
        }
        
        // 大小验证通过
        partition.size = sizeValue;
        partition.sizeString = formatSize(sizeValue);
    } else if (ok && sizeValue < 0) {
        // 负数错误
        QMessageBox::warning(this, "大小验证错误", "分区大小不能为负数！");
        
        // 恢复旧值
        m_sizeEdit->blockSignals(true);
        m_sizeEdit->setText(QString::number(oldSize));
        m_sizeEdit->blockSignals(false);
        
        return;
    } else {
        // 解析错误
        QMessageBox::warning(this, "大小验证错误", "请输入有效的数字！");
        
        // 恢复旧值
        m_sizeEdit->blockSignals(true);
        m_sizeEdit->setText(QString::number(oldSize));
        m_sizeEdit->blockSignals(false);
        
        return;
    }
    
    // 更新文件
    partition.file = m_fileEdit->text().trimmed();
    
    // 更新挂载点
    partition.mountpoint = m_mountpointEdit->text().trimmed();
    
    // 更新类型
    partition.type = m_typeEdit->text().trimmed();
    
    // 更新表格显示
    updateSizeString(currentRow);
    
    // 更新表格中的其他字段
    if (QTableWidgetItem* labelItem = m_partitionTable->item(currentRow, COL_LABEL)) {
        labelItem->setText(partition.label);
    }
    if (QTableWidgetItem* fileItem = m_partitionTable->item(currentRow, COL_FILE)) {
        fileItem->setText(partition.file);
    }
    if (QTableWidgetItem* mountpointItem = m_partitionTable->item(currentRow, COL_MOUNTPOINT)) {
        mountpointItem->setText(partition.mountpoint);
    }
    if (QTableWidgetItem* typeItem = m_partitionTable->item(currentRow, COL_TYPE)) {
        typeItem->setText(partition.type);
    }
    
    updatePartitionVisualization();
    
    // 显示配置成功提示
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setWindowTitle("配置成功");
    msgBox.setText(QString("分区 %1 配置已更新").arg(partNum));
    msgBox.setInformativeText(QString("标签: %1\n大小: %2 KB\n文件: %3\n挂载点: %4\n类型: %5")
                             .arg(partition.label)
                             .arg(partition.size)
                             .arg(partition.file.isEmpty() ? "(空)" : partition.file)
                             .arg(partition.mountpoint.isEmpty() ? "(空)" : partition.mountpoint)
                             .arg(partition.type.isEmpty() ? "(空)" : partition.type));
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
    
    emit configChanged();
    emit partitionChanged(partNum);
}

void FlashConfigWidget::onToggleConfigPanel()
{
    setConfigPanelExpanded(!m_configExpanded);
}

void FlashConfigWidget::updateSizeString(int row)
{
    if (row < 0 || row >= m_partitionOrder.size()) return;
    
    int partNum = m_partitionOrder[row];
    if (!m_partitions.contains(partNum)) return;
    
    const FlashPartition& partition = m_partitions[partNum];
    
    QTableWidgetItem* sizeItem = m_partitionTable->item(row, COL_SIZE);
    if (sizeItem) {
        sizeItem->setText(QString::number(partition.size));
    }
}

void FlashConfigWidget::updatePartitionVisualization()
{
    QString partitionMap;
    
    partitionMap += "Flash分区映射:\n";
    partitionMap += "========================================\n";
    partitionMap += QString("Flash大小: %1\n").arg(m_flashSize);
    partitionMap += QString("分区数量: %1\n").arg(m_partitionCount);
    partitionMap += "========================================\n";
    
    for (int i = 0; i < m_partitionOrder.size(); ++i) {
        int partNum = m_partitionOrder[i];
        if (!m_partitions.contains(partNum)) continue;
        
        const FlashPartition& partition = m_partitions[partNum];
        
        partitionMap += QString("分区%1 [%2]: %3 (%4 KB)\n")
                       .arg(partition.partitionNumber)
                       .arg(partition.enabled ? "启用" : "禁用")
                       .arg(partition.label, -12)
                       .arg(partition.size);
        
        if (!partition.file.isEmpty()) {
            partitionMap += QString("  文件: %1\n").arg(partition.file);
        }
        if (!partition.mountpoint.isEmpty()) {
            partitionMap += QString("  挂载点: %1\n").arg(partition.mountpoint);
        }
        if (!partition.type.isEmpty()) {
            partitionMap += QString("  类型: %1\n").arg(partition.type);
        }
    }
    
    m_partitionMapText->setPlainText(partitionMap);
}

void FlashConfigWidget::validatePartitionLayout()
{
    // 可以在这里添加分区布局验证逻辑
    // 例如：检查分区是否重叠、总大小是否超过Flash容量等
}

QString FlashConfigWidget::formatSize(quint64 sizeInKB)
{
    if (sizeInKB == 0) {
        return "0KB";
    }
    
    const quint64 MB = 1024;
    const quint64 GB = MB * 1024;
    
    if (sizeInKB >= GB) {
        return QString("%1GB").arg(sizeInKB / GB);
    } else if (sizeInKB >= MB) {
        return QString("%1MB").arg(sizeInKB / MB);
    } else {
        return QString("%1KB").arg(sizeInKB);
    }
}

quint64 FlashConfigWidget::parseSize(const QString& sizeStr)
{
    QString cleanStr = sizeStr.trimmed().toUpper();
    bool ok;
    quint64 size = 0;
    
    if (cleanStr.endsWith("GB")) {
        size = cleanStr.left(cleanStr.length() - 2).toULongLong(&ok) * 1024 * 1024;
    } else if (cleanStr.endsWith("MB")) {
        size = cleanStr.left(cleanStr.length() - 2).toULongLong(&ok) * 1024;
    } else if (cleanStr.endsWith("KB")) {
        size = cleanStr.left(cleanStr.length() - 2).toULongLong(&ok);
    } else {
        size = cleanStr.toULongLong(&ok);
    }
    
    return ok ? size : 0;
}

void FlashConfigWidget::searchAndSelectPartition(const QString& searchText)
{
    for (int row = 0; row < m_partitionTable->rowCount(); ++row) {
        QTableWidgetItem* labelItem = m_partitionTable->item(row, COL_LABEL);
        if (labelItem && labelItem->text().compare(searchText, Qt::CaseInsensitive) == 0) {
            m_partitionTable->selectRow(row);
            m_partitionTable->setCurrentItem(labelItem);
            
            setConfigPanelExpanded(true);
            
            m_labelEdit->setEnabled(true);
            m_sizeEdit->setEnabled(true);
            m_sizeUnitCombo->setEnabled(true);
            m_fileEdit->setEnabled(true);
            m_mountpointEdit->setEnabled(true);
            m_typeEdit->setEnabled(true);
            
            onTableSelectionChanged();
            
            return;
        }
    }
    
    QMessageBox::information(this, "搜索结果", 
                            QString("未找到标签为 \"%1\" 的分区").arg(searchText));
}

void FlashConfigWidget::setConfigPanelExpanded(bool expanded)
{
    m_configExpanded = expanded;
    m_configContent->setVisible(expanded);
    
    if (expanded) {
        m_configToggleButton->setArrowType(Qt::DownArrow);
        m_configToggleButton->setToolTip("收起分区配置");
    } else {
        m_configToggleButton->setArrowType(Qt::RightArrow);
        m_configToggleButton->setToolTip("展开分区配置");
        
        m_labelEdit->setEnabled(false);
        m_sizeEdit->setEnabled(false);
        m_sizeUnitCombo->setEnabled(false);
        m_fileEdit->setEnabled(false);
        m_mountpointEdit->setEnabled(false);
        m_typeEdit->setEnabled(false);
    }
}

void FlashConfigWidget::formatSizeForInput(quint64 sizeInKB, QString& sizeText, QString& unit)
{
    const quint64 MB = 1024;
    const quint64 GB = MB * 1024;
    
    if (sizeInKB == 0) {
        sizeText = "0";
        unit = "KB";
    } else if (sizeInKB >= GB && sizeInKB % GB == 0) {
        sizeText = QString::number(sizeInKB / GB);
        unit = "GB";
    } else if (sizeInKB >= MB && sizeInKB % MB == 0) {
        sizeText = QString::number(sizeInKB / MB);
        unit = "MB";
    } else {
        sizeText = QString::number(sizeInKB);
        unit = "KB";
    }
}

FlashPartition FlashConfigWidget::getPartition(int partitionNumber) const
{
    return m_partitions.value(partitionNumber, FlashPartition());
}

void FlashConfigWidget::setPartition(int partitionNumber, const FlashPartition& partition)
{
    m_partitions[partitionNumber] = partition;
    if (!m_partitionOrder.contains(partitionNumber)) {
        m_partitionOrder.append(partitionNumber);
    }
    populateTable();
    updatePartitionVisualization();
    emit configChanged();
}

bool FlashConfigWidget::exportToJson(const QString& filePath)
{
    QJsonObject root;
    QJsonArray partitionsArray;
    
    root["flashSize"] = m_flashSize;
    root["partitionCount"] = m_partitionCount;
    
    for (const auto& partition : m_partitions) {
        QJsonObject partObj;
        partObj["partitionNumber"] = partition.partitionNumber;
        partObj["label"] = partition.label;
        partObj["size"] = QString::number(partition.size);
        partObj["file"] = partition.file;
        partObj["mountpoint"] = partition.mountpoint;
        partObj["type"] = partition.type;
        partObj["enabled"] = partition.enabled;
        
        partitionsArray.append(partObj);
    }
    
    root["partitions"] = partitionsArray;
    
    QJsonDocument doc(root);
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    file.write(doc.toJson());
    file.close();
    
    return true;
}

bool FlashConfigWidget::importFromJson(const QString& filePath)
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
    
    m_flashSize = root["flashSize"].toString();
    m_partitionCount = root["partitionCount"].toInt();
    
    QJsonArray partitionsArray = root["partitions"].toArray();
    
    m_partitions.clear();
    m_partitionOrder.clear();
    
    for (const auto& value : partitionsArray) {
        QJsonObject partObj = value.toObject();
        
        FlashPartition partition;
        partition.partitionNumber = partObj["partitionNumber"].toInt();
        partition.label = partObj["label"].toString();
        partition.size = partObj["size"].toString().toULongLong();
        partition.sizeString = formatSize(partition.size);
        partition.file = partObj["file"].toString();
        partition.mountpoint = partObj["mountpoint"].toString();
        partition.type = partObj["type"].toString();
        partition.enabled = partObj["enabled"].toBool();
        
        m_partitions[partition.partitionNumber] = partition;
        m_partitionOrder.append(partition.partitionNumber);
    }
    
    return true;
}

void FlashConfigWidget::setSourcePath(const QString& sourcePath)
{
    m_sourcePath = sourcePath;
}

void FlashConfigWidget::setChipType(const QString& chipType)
{
    m_chipType = chipType;
}

QString FlashConfigWidget::getDefconfigPath() const
{
    QString chipType = m_chipType;
    if (chipType.isEmpty() || chipType == "请选择芯片型号") {
        chipType = "cv1842hp_wevb_0014a_emmc";
    }
    
    QString defconfigPath = QString("build/boards/cv184x/%1/%1_defconfig")
                           .arg(chipType);
    
    QDir workspaceDir(m_sourcePath);
    return workspaceDir.absoluteFilePath(defconfigPath);
}

bool FlashConfigWidget::exportToDefconfig(const QString& sourcePath, const QString& chipType)
{
    QString defconfigPath = getDefconfigPath();
    
    QFile defconfigFile(defconfigPath);
    if (!defconfigFile.exists()) {
        QMessageBox::critical(nullptr, "错误", 
                            QString("Defconfig文件不存在:\n%1").arg(defconfigPath));
        return false;
    }
    
    if (!defconfigFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(nullptr, "错误", 
                            QString("无法读取defconfig文件:\n%1").arg(defconfigPath));
        return false;
    }
    
    QTextStream in(&defconfigFile);
    QStringList lines;
    while (!in.atEnd()) {
        lines.append(in.readLine());
    }
    defconfigFile.close();
    
    // 只更新修改的配置项，不重新生成整个段
    bool fileModified = false;
    
    for (int partNum : m_partitionOrder) {
        if (!m_partitions.contains(partNum)) continue;
        
        const FlashPartition& partition = m_partitions[partNum];
        
        // 获取启用状态
        QTableWidgetItem* enabledItem = nullptr;
        for (int row = 0; row < m_partitionTable->rowCount(); ++row) {
            QTableWidgetItem* numItem = m_partitionTable->item(row, COL_PARTITION_NUM);
            if (numItem && numItem->text().toInt() == partNum) {
                enabledItem = m_partitionTable->item(row, COL_ENABLED);
                break;
            }
        }
        
        bool enabled = enabledItem ? (enabledItem->checkState() == Qt::Checked) : partition.enabled;
        
        // 更新各个配置项
        QString enabledConfig = QString("CONFIG_PARTITION_%1=y").arg(partNum);
        QString labelConfig = QString("CONFIG_PARTITION_%1_LABEL=").arg(partNum);
        QString sizeConfig = QString("CONFIG_PARTITION_%1_SIZE=").arg(partNum);
        QString fileConfig = QString("CONFIG_PARTITION_%1_FILE=").arg(partNum);
        QString mountpointConfig = QString("CONFIG_PARTITION_%1_MOUNTPOINT=").arg(partNum);
        QString typeConfig = QString("CONFIG_PARTITION_%1_TYPE=").arg(partNum);
        
        // 遍历文件查找并更新对应的配置行
        for (int i = 0; i < lines.size(); ++i) {
            QString& line = lines[i];
            
            // 更新启用状态
            if (line.startsWith(enabledConfig)) {
                if (!enabled) {
                    // 如果不启用，删除这行
                    lines.removeAt(i);
                    i--;
                    fileModified = true;
                }
            }
            // 更新标签
            else if (line.startsWith(labelConfig)) {
                QString newLine = QString("CONFIG_PARTITION_%1_LABEL=\"%2\"").arg(partNum).arg(partition.label);
                if (line != newLine) {
                    line = newLine;
                    fileModified = true;
                }
            }
            // 更新大小
            else if (line.startsWith(sizeConfig)) {
                QString newLine = QString("CONFIG_PARTITION_%1_SIZE=\"%2\"").arg(partNum).arg(partition.size);
                if (line != newLine) {
                    line = newLine;
                    fileModified = true;
                }
            }
            // 更新文件
            else if (line.startsWith(fileConfig)) {
                QString newLine = QString("CONFIG_PARTITION_%1_FILE=\"%2\"").arg(partNum).arg(partition.file);
                if (line != newLine) {
                    line = newLine;
                    fileModified = true;
                }
            }
            // 更新挂载点
            else if (line.startsWith(mountpointConfig)) {
                QString newLine = QString("CONFIG_PARTITION_%1_MOUNTPOINT=\"%2\"").arg(partNum).arg(partition.mountpoint);
                if (line != newLine) {
                    line = newLine;
                    fileModified = true;
                }
            }
            // 更新类型
            else if (line.startsWith(typeConfig)) {
                QString newLine = QString("CONFIG_PARTITION_%1_TYPE=\"%2\"").arg(partNum).arg(partition.type);
                if (line != newLine) {
                    line = newLine;
                    fileModified = true;
                }
            }
        }
        
        // 如果启用但配置不存在，需要添加（这种情况主要用于新增分区）
        if (enabled) {
            bool hasEnabledConfig = false;
            for (const QString& line : lines) {
                if (line.startsWith(enabledConfig)) {
                    hasEnabledConfig = true;
                    break;
                }
            }
            
            if (!hasEnabledConfig) {
                // 查找合适的插入位置（在分区配置段内）
                int insertPos = -1;
                for (int i = 0; i < lines.size(); ++i) {
                    if (lines[i].contains("# Partition Configuration")) {
                        insertPos = i + 3; // 跳过标题行
                        break;
                    }
                }
                
                if (insertPos == -1) {
                    // 如果没有分区配置段，在文件末尾添加
                    insertPos = lines.size();
                    lines.append("#");
                    lines.append("# Partition Configuration");
                    lines.append("#");
                    insertPos = lines.size();
                }
                
                // 添加新分区的所有配置
                lines.insert(insertPos++, enabledConfig);
                lines.insert(insertPos++, QString("CONFIG_PARTITION_%1_LABEL=\"%2\"").arg(partNum).arg(partition.label));
                lines.insert(insertPos++, QString("CONFIG_PARTITION_%1_SIZE=\"%2\"").arg(partNum).arg(partition.size));
                lines.insert(insertPos++, QString("CONFIG_PARTITION_%1_FILE=\"%2\"").arg(partNum).arg(partition.file));
                lines.insert(insertPos++, QString("CONFIG_PARTITION_%1_MOUNTPOINT=\"%2\"").arg(partNum).arg(partition.mountpoint));
                lines.insert(insertPos++, QString("CONFIG_PARTITION_%1_TYPE=\"%2\"").arg(partNum).arg(partition.type));
                lines.insert(insertPos++, "");
                fileModified = true;
            }
        }
    }
    
    // 只有在文件被修改时才写回
    if (!fileModified) {
        return true; // 没有修改，直接返回成功
    }
    
    // 写回文件
    if (!defconfigFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(nullptr, "错误", 
                            QString("无法写入defconfig文件:\n%1").arg(defconfigPath));
        return false;
    }
    
    QTextStream out(&defconfigFile);
    for (const QString& line : lines) {
        out << line << "\n";
    }
    defconfigFile.close();
    
    return true;
}

bool FlashConfigWidget::saveConfig(const QString& filePath)
{
    return exportToJson(filePath);
}

bool FlashConfigWidget::loadConfig(const QString& filePath)
{
    return importFromJson(filePath);
}
