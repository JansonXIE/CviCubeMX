#ifndef FLASHCONFIG_H
#define FLASHCONFIG_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QPushButton>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
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

// Flash分区配置结构
struct FlashPartition {
    int partitionNumber;      // 分区号
    QString label;            // 分区标签
    quint64 size;             // 大小（KB）
    QString sizeString;       // 大小字符串（方便显示）
    QString file;             // 关联文件
    QString mountpoint;       // 挂载点
    QString type;             // 类型（ext4等）
    bool enabled;             // 是否启用
};

class FlashConfigWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FlashConfigWidget(QWidget *parent = nullptr);
    ~FlashConfigWidget();

    // 获取和设置分区配置
    FlashPartition getPartition(int partitionNumber) const;
    void setPartition(int partitionNumber, const FlashPartition& partition);
    
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
    void partitionChanged(int partitionNumber);

private slots:
    void onTableItemDoubleClicked(QTableWidgetItem* item);
    void onAddPartition();
    void onRemovePartition();
    void onResetPartitions();
    void onExportConfig();
    void onImportConfig();
    void onSearchTextChanged(const QString& searchText);
    void onSearchEnterPressed();
    void onTableSelectionChanged();
    void onConfigFieldChanged();
    void onToggleConfigPanel();

private:
    void setupUI();
    void setupPartitionTable();
    void setupControlPanel();
    void initializePartitions();
    void populateTable();
    void updateSizeString(int row);
    void updatePartitionVisualization();
    void validatePartitionLayout();
    QString formatSize(quint64 sizeInKB);
    quint64 parseSize(const QString& sizeStr);
    void searchAndSelectPartition(const QString& searchText);
    void setConfigPanelExpanded(bool expanded);
    void formatSizeForInput(quint64 sizeInKB, QString& sizeText, QString& unit);
    QString getDefconfigPath() const;
    
    // UI组件
    QVBoxLayout* m_mainLayout;
    QSplitter* m_splitter;
    
    // 分区表格区域
    QWidget* m_tableWidget;
    QVBoxLayout* m_tableLayout;
    QTableWidget* m_partitionTable;
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
    
    // 分区搜索组
    QGroupBox* m_searchGroup;
    QVBoxLayout* m_searchLayout;
    QLineEdit* m_searchEdit;
    
    // 分区配置字段组
    QWidget* m_configContainer;
    QVBoxLayout* m_configContainerLayout;
    QWidget* m_configHeader;
    QHBoxLayout* m_configHeaderLayout;
    QLabel* m_configTitleLabel;
    QToolButton* m_configToggleButton;
    QFrame* m_configContent;
    QVBoxLayout* m_configLayout;
    QLineEdit* m_labelEdit;
    QLineEdit* m_sizeEdit;
    QComboBox* m_sizeUnitCombo;
    QLineEdit* m_fileEdit;
    QLineEdit* m_mountpointEdit;
    QLineEdit* m_typeEdit;
    bool m_configExpanded;
    
    // 分区可视化区域
    QGroupBox* m_visualizationGroup;
    QVBoxLayout* m_visualizationLayout;
    QTextEdit* m_partitionMapText;
    
    // 数据存储
    QMap<int, FlashPartition> m_partitions;
    QList<int> m_partitionOrder;  // 用于保持原始顺序
    
    // 源代码路径和芯片类型
    QString m_sourcePath;
    QString m_chipType;
    
    // Flash配置
    QString m_flashSize;  // Flash大小（如"32GB"）
    int m_partitionCount;  // 分区数量
    
    // 表格列索引
    enum TableColumns {
        COL_PARTITION_NUM = 0,
        COL_ENABLED,
        COL_LABEL,
        COL_SIZE,
        COL_FILE,
        COL_MOUNTPOINT,
        COL_TYPE,
        COL_COUNT
    };
};

#endif // FLASHCONFIG_H
