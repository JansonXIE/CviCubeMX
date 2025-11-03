#ifndef MEMORYCONFIG_H
#define MEMORYCONFIG_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QPushButton>
#include <QGroupBox>
#include <QLabel>
#include <QSpinBox>
#include <QLineEdit>
#include <QComboBox>
#include <QProgressBar>
#include <QSplitter>
#include <QTextEdit>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QMessageBox>
#include <QFileDialog>
#include <QMap>
#include <QString>
#include <QFrame>
#include <QToolButton>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QKeyEvent>

// 内存区域配置结构
struct MemoryRegion {
    QString name;           // 区域名称
    quint64 startAddress;   // 起始地址
    quint64 endAddress;     // 结束地址
    quint64 size;          // 大小（字节）
    QString sizeString;    // 大小字符串（M/K/B格式）
    bool isEditable;       // 是否可编辑
    QString description;   // 描述
};

class MemoryConfigWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MemoryConfigWidget(QWidget *parent = nullptr);
    ~MemoryConfigWidget();

    // 获取和设置内存区域配置
    MemoryRegion getMemoryRegion(const QString& name) const;
    void setMemoryRegion(const QString& name, const MemoryRegion& region);
    
    // 保存和加载配置
    bool saveConfig(const QString& filePath);
    bool loadConfig(const QString& filePath);
    
    // 导入导出功能
    bool exportToJson(const QString& filePath);
    bool importFromJson(const QString& filePath);
    
    // 导出到defconfig文件
    bool exportToDefconfig(const QString& sourcePath, const QString& chipType);
    
    // 设置源代码路径和芯片类型
    void setSourcePath(const QString& sourcePath);
    void setChipType(const QString& chipType);

signals:
    void configChanged();
    void memoryRegionChanged(const QString& regionName);

private slots:
    void onTableItemChanged(QTableWidgetItem* item);
    void onTableItemDoubleClicked(QTableWidgetItem* item);
    void onAddRegion();
    void onRemoveRegion();
    void onResetRegions();
    void onExportConfig();
    void onImportConfig();
    void onSearchTextChanged(const QString& searchText);
    void onSearchEnterPressed();
    void onTableSelectionChanged();
    void onConfigFieldChanged();
    void onToggleConfigPanel();

private:
    void setupUI();
    void setupMemoryTable();
    void setupControlPanel();
    void initializeMemoryRegions();
    void populateTable();
    void updateSizeString(int row);
    void updateMemoryVisualization();
    void validateMemoryLayout();
    QString formatSize(quint64 sizeInBytes);
    quint64 parseAddress(const QString& addressStr);
    QString formatAddress(quint64 address);
    void sortTableByAddress();
    void checkMemoryOverlap();
    void formatSizeForInput(quint64 sizeInBytes, QString& sizeText, QString& unit);
    void searchAndSelectRegion(const QString& searchText);
    void setConfigPanelExpanded(bool expanded);
    bool validateMemoryConstraints(QString& errorMessage);
    
    // UI组件
    QVBoxLayout* m_mainLayout;
    QSplitter* m_splitter;
    
    // 内存表格区域
    QWidget* m_tableWidget;
    QVBoxLayout* m_tableLayout;
    QTableWidget* m_memoryTable;
    QLabel* m_tableTitle;
    
    // 控制面板
    QWidget* m_controlPanel;
    QVBoxLayout* m_controlLayout;
    
    // 操作按钮组
    QGroupBox* m_operationGroup;
    QVBoxLayout* m_operationLayout;
    QPushButton* m_addButton;
    QPushButton* m_removeButton;
    QPushButton* m_resetButton;
    QPushButton* m_exportButton;
    QPushButton* m_importButton;
    
    // 地址区域搜索组
    QGroupBox* m_searchGroup;
    QVBoxLayout* m_searchLayout;
    QLineEdit* m_searchEdit;
    
    // 区域配置字段组
    QWidget* m_configContainer;
    QVBoxLayout* m_configContainerLayout;
    QWidget* m_configHeader;
    QHBoxLayout* m_configHeaderLayout;
    QLabel* m_configTitleLabel;
    QToolButton* m_configToggleButton;
    QFrame* m_configContent;
    QVBoxLayout* m_configLayout;
    QLineEdit* m_nameEdit;
    QLineEdit* m_startAddressEdit;
    QLineEdit* m_sizeEdit;
    QComboBox* m_sizeUnitCombo;
    bool m_configExpanded;
    
    // 内存可视化区域
    QGroupBox* m_visualizationGroup;
    QVBoxLayout* m_visualizationLayout;
    QTextEdit* m_memoryMapText;
    
    // 数据存储
    QMap<QString, MemoryRegion> m_memoryRegions;
    QStringList m_regionOrder;  // 用于保持原始顺序
    
    // 源代码路径和芯片类型
    QString m_sourcePath;
    QString m_chipType;
    
    // 常量
    static const quint64 TOTAL_MEMORY_SIZE;  // 总内存大小 (256MB)
    static const quint64 MEMORY_BASE_ADDRESS; // 内存基地址 (0x80000000)
    
    // 表格列索引
    enum TableColumns {
        COL_NAME = 0,
        COL_START_ADDRESS,
        COL_END_ADDRESS,
        COL_SIZE_HEX,
        COL_SIZE_FORMATTED,
        COL_COUNT
    };
};

#endif // MEMORYCONFIG_H