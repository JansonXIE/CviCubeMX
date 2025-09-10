#ifndef CLOCKCONFIG_H
#define CLOCKCONFIG_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QScrollArea>
#include <QPushButton>
#include <QFrame>
#include <QSplitter>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QCheckBox>
#include <QMap>
#include <QString>
#include <QPainter>
#include <QPaintEvent>
#include <QPolygon>

// 前向声明
class ConnectionOverlay;

// PLL配置结构
struct PLLConfig {
    QString name;           // PLL名称
    bool enabled;          // 是否启用
    double inputFreq;      // 输入频率(MHz)
    double outputFreq;     // 输出频率(MHz)
    double divider;        // 分频器值（支持小数）
    int multiplier;       // 倍频器值
    QString source;       // 时钟源
};

// 时钟输出结构
struct ClockOutput {
    QString name;        // 输出名称
    QString source;      // 时钟源
    int divider;        // 分频器值
    double frequency;   // 频率(MHz)
    bool enabled;       // 是否启用
};

// 模块位置配置结构
struct ModulePosition {
    QString moduleName;  // 模块名称
    int x;              // X坐标
    int y;              // Y坐标
    int width;          // 宽度
    int height;         // 高度
};

class ClockConfigWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ClockConfigWidget(QWidget *parent = nullptr);
    ~ClockConfigWidget();

    // 获取和设置PLL配置
    PLLConfig getPLLConfig(const QString& pllName) const;
    void setPLLConfig(const QString& pllName, const PLLConfig& config);
    
    // 保存和加载配置
    bool saveConfig(const QString& filePath);
    bool loadConfig(const QString& filePath);
    
    // 模块位置配置相关函数
    void showPositionConfigDialog();
    void setModulePosition(const QString& moduleName, int x, int y);
    ModulePosition getModulePosition(const QString& moduleName) const;
    void resetModulePositions();
    void applyModulePositions();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

signals:
    void configChanged();
    void pllConfigChanged(const QString& pllName);

private slots:
    void onPLLMultiplierChanged(const QString& pllName, int multiplier);
    void onSubPLLConfigChanged(const QString& pllName);  // 新增
    void onClockDividerChanged(const QString& outputName, int divider);
    void onClk1MSubNodeDividerChanged(const QString& nodeName, int divider);  // 新增：clk_1M子节点分频器变化
    void updateFrequencies();
    void resetToDefaults();

private:
    void setupUI();
    void setupClockSources();
    void setupPLLs();
    void setupSubPLLs();  // 新增：设置子PLL区域
    void setupOutputs();
    void setupClk1MSubNodes();  // 新增：设置clk_1M子节点区域
    void setupClockTree();
    void initializeModulePositions();  // 新增：初始化模块位置
    void createPLLWidget(const QString& pllName, QWidget* parent);
    void createSubPLLWidget(const QString& pllName, QWidget* parent);  // 新增：创建子PLL widget
    void createOutputWidget(const QString& outputName, QWidget* parent);
    void createClk1MSubNodeWidget(const QString& nodeName, QWidget* parent);  // 新增：创建clk_1M子节点widget
    void updatePLLFrequency(const QString& pllName);
    void updateSubPLLFrequency(const QString& pllName);  // 新增
    void updateAllSubPLLFrequencies();  // 新增
    void updateOutputFrequency(const QString& outputName);
    void updateClk1MSubNodeFrequency(const QString& nodeName);  // 新增：更新clk_1M子节点频率
    void updateAllClk1MSubNodeFrequencies();  // 新增：更新所有clk_1M子节点频率
    void connectSignals();
    
    // 连接线绘制相关方法
    void drawConnectionLines(QPainter& painter);
    void drawArrowLine(QPainter& painter, const QPoint& start, const QPoint& end, const QColor& color = Qt::blue);
    QPoint getOSCConnectionPoint() const;
    QPoint getPLLConnectionPoint(const QString& pllName) const;
    QPoint getSubPLLConnectionPoint(const QString& pllName) const;  // 新增：获取子PLL连接点
    QPoint getMIPIMPLLConnectionPoint() const;  // 新增：获取MIPIMPLL输出连接点
    QPoint getOutputAreaConnectionPoint() const;  // 新增：获取输出区域连接点
    QPoint getClk1MConnectionPoint() const;  // 新增：获取clk_1M连接点
    QPoint getClk1MSubNodeConnectionPoint(const QString& nodeName) const;  // 新增：获取clk_1M子节点连接点
    void updateConnectionOverlay();
    
    // UI组件
    QVBoxLayout* m_mainLayout;
    
    // 时钟流程显示区域
    QScrollArea* m_flowScrollArea;
    QWidget* m_flowWidget;
    // QHBoxLayout* m_flowLayout;  // 不再使用布局管理器，改为绝对定位
    
    // 输入源区域
    QWidget* m_inputWidget;
    QVBoxLayout* m_inputLayout;
    
    // PLL区域
    QWidget* m_pllWidget;
    QVBoxLayout* m_pllLayout;
    
    // 子PLL区域
    QWidget* m_subPllWidget;
    QVBoxLayout* m_subPllLayout;
    
    // 输出区域
    QWidget* m_outputWidget;
    QVBoxLayout* m_outputLayout;
    
    // clk_1M子节点区域
    QWidget* m_clk1MSubNodeWidget;
    QVBoxLayout* m_clk1MSubNodeLayout;
    
    // 左侧时钟树面板  
    QWidget* m_clockTreeWidget;
    QVBoxLayout* m_clockTreeLayout;
    QTreeWidget* m_clockTree;
    
    // 右侧配置面板已删除，现在使用流程布局
    
    // 时钟源配置组件
    QGroupBox* m_clockSourceGroup;
    QVBoxLayout* m_clockSourceLayout;
    QLabel* m_oscLabel;
    QLabel* m_rtcLabel;
    QLabel* m_oscFreqLabel;
    QLabel* m_rtcFreqLabel;
    
    // PLL配置组
    QGroupBox* m_pllGroup;
    QVBoxLayout* m_pllGroupLayout;
    QMap<QString, QWidget*> m_pllWidgets;
    QMap<QString, QSpinBox*> m_pllMultiplierBoxes;
    QMap<QString, QLabel*> m_pllFreqLabels;
    
    // 子PLL配置组
    QMap<QString, QWidget*> m_subPllWidgets;
    QMap<QString, QSpinBox*> m_subPllMultiplierBoxes;
    QMap<QString, QDoubleSpinBox*> m_subPllDividerBoxes;  // 改为QDoubleSpinBox支持小数
    QMap<QString, QLabel*> m_subPllFreqLabels;
    
    // 输出配置组
    QGroupBox* m_outputGroup;
    QVBoxLayout* m_outputGroupLayout;
    QMap<QString, QWidget*> m_outputWidgets;
    QMap<QString, QLabel*> m_outputFreqLabels;
    QMap<QString, QSpinBox*> m_outputDividerBoxes;  // 新增：输出分频器控件
    
    // clk_1M子节点配置组
    QMap<QString, QWidget*> m_clk1MSubNodeWidgets;
    QMap<QString, QLabel*> m_clk1MSubNodeFreqLabels;
    QMap<QString, QSpinBox*> m_clk1MSubNodeDividerBoxes;
    
    // 控制按钮
    QHBoxLayout* m_buttonLayout;
    QPushButton* m_resetButton;
    QPushButton* m_applyButton;
    QPushButton* m_positionConfigButton;  // 新增：位置配置按钮
    
    // 数据存储
    QMap<QString, PLLConfig> m_pllConfigs;
    QMap<QString, ClockOutput> m_outputs;
    QMap<QString, ClockOutput> m_clk1MSubNodes;  // 新增：clk_1M子节点数据
    QMap<QString, ModulePosition> m_modulePositions;  // 新增：模块位置配置
    
    // 常量
    static const double OSC_FREQUENCY;  // 25MHz
    static const double RTC_FREQUENCY;  // 32.768kHz
    static const QStringList PLL_NAMES;
    static const QStringList SUB_PLL_NAMES;  // 新增：子PLL名称列表
    static const QStringList OUTPUT_NAMES;
    static const QStringList CLK_1M_SUB_NODES;  // 新增：clk_1M子节点列表
    
    // 连接线覆盖层
    QWidget* m_connectionOverlay;
};

#endif // CLOCKCONFIG_H
