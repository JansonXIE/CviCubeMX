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
#include <QMouseEvent>
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

// 缩放方向枚举
enum ResizeDirection {
    None = 0,
    TopLeft = 1,
    Top = 2,
    TopRight = 3,
    Right = 4,
    BottomRight = 5,
    Bottom = 6,
    BottomLeft = 7,
    Left = 8
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
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

signals:
    void configChanged();
    void pllConfigChanged(const QString& pllName);

private slots:
    void onPLLMultiplierChanged(const QString& pllName, int multiplier);
    void onSubPLLConfigChanged(const QString& pllName);  // 新增
    void onClockDividerChanged(const QString& outputName, int divider);
    void onClk1MSubNodeDividerChanged(const QString& nodeName, int divider);  // 新增：clk_1M子节点分频器变化
    void onClkCam1PLLSubNodeDividerChanged(const QString& nodeName, int divider);  // 新增：clk_cam1pll子节点分频器变化
    void onClkRawAxiSubNodeDividerChanged(const QString& nodeName, int divider);  // 新增：clk_raw_axi子节点分频器变化
    void onClkCam0PLLSubNodeDividerChanged(const QString& nodeName, int divider);  // 新增：clk_cam0pll子节点分频器变化
    void onClkDispPLLSubNodeDividerChanged(const QString& nodeName, int divider);  // 新增：clk_disppll子节点分频器变化
    void onClkSysDispSubNodeDividerChanged(const QString& nodeName, int divider);  // 新增：clk_sys_disp子节点分频器变化
    void onClkA0PLLSubNodeDividerChanged(const QString& nodeName, int divider);  // 新增：clk_a0pll子节点分频器变化
    void onClkRVPLLSubNodeDividerChanged(const QString& nodeName, int divider);  // 新增：clk_rvpll子节点分频器变化
    void onClkAPPLLSubNodeDividerChanged(const QString& nodeName, int divider);  // 新增：clk_appll子节点分频器变化
    void onClkFPLLSubNodeDividerChanged(const QString& nodeName, int divider);  // 新增：clk_fpll子节点分频器变化
    void onClkTPLLSubNodeDividerChanged(const QString& nodeName, int divider);  // 新增：clk_tpll子节点分频器变化
    void onClkMPLLSubNodeDividerChanged(const QString& nodeName, int divider);  // 新增：clk_mpll子节点分频器变化
    void onClkFAB100MSubNodeDividerChanged(const QString& nodeName, int divider);  // 新增：clk_fab_100M子节点分频器变化
    void onClkSPINANDSubNodeDividerChanged(const QString& nodeName, int divider);  // 新增：clk_spi_nand子节点分频器变化
    void onClkHSPeriSubNodeDividerChanged(const QString& nodeName, int divider);  // 新增：clk_hsperi子节点分频器变化
    void updateFrequencies();
    void resetToDefaults();

private:
    void setupUI();
    void setupClockSources();
    void setupPLLs();
    void setupSubPLLs();  // 新增：设置子PLL区域
    void setupOutputs();
    void setupClk1MSubNodes();  // 新增：设置clk_1M子节点区域
    void setupClkCam1PLLSubNodes();  // 新增：设置clk_cam1pll子节点区域
    void setupClkRawAxiSubNodes();  // 新增：设置clk_raw_axi子节点区域
    void setupClkCam0PLLSubNodes();  // 新增：设置clk_cam0pll子节点区域
    void setupClkDispPLLSubNodes();  // 新增：设置clk_disppll子节点区域
    void setupClkSysDispSubNodes();  // 新增：设置clk_sys_disp子节点区域
    void setupClkA0PLLSubNodes();  // 新增：设置clk_a0pll子节点区域
    void setupClkRVPLLSubNodes();  // 新增：设置clk_rvpll子节点区域
    void setupClkAPPLLSubNodes();  // 新增：设置clk_appll子节点区域
    void setupClkFPLLSubNodes();  // 新增：设置clk_fpll子节点区域
    void setupClkTPLLSubNodes();  // 新增：设置clk_tpll子节点区域
    void setupClkMPLLSubNodes();  // 新增：设置clk_mpll子节点区域
    void setupClkFAB100MSubNodes();  // 新增：设置clk_fab_100M子节点区域
    void setupClkSPINANDSubNodes();  // 新增：设置clk_spi_nand子节点区域
    void setupClkHSPeriSubNodes();  // 新增：设置clk_hsperi子节点区域
    void setupClockTree();
    void initializeModulePositions();  // 新增：初始化模块位置
    void createPLLWidget(const QString& pllName, QWidget* parent);
    void createSubPLLWidget(const QString& pllName, QWidget* parent);  // 新增：创建子PLL widget
    void createOutputWidget(const QString& outputName, QWidget* parent);
    void createClk1MSubNodeWidget(const QString& nodeName, QWidget* parent);  // 新增：创建clk_1M子节点widget
    void createClkCam1PLLSubNodeWidget(const QString& nodeName, QWidget* parent);  // 新增：创建clk_cam1pll子节点widget
    void createClkRawAxiSubNodeWidget(const QString& nodeName, QWidget* parent);  // 新增：创建clk_raw_axi子节点widget
    void createClkCam0PLLSubNodeWidget(const QString& nodeName, QWidget* parent);  // 新增：创建clk_cam0pll子节点widget
    void createClkDispPLLSubNodeWidget(const QString& nodeName, QWidget* parent);  // 新增：创建clk_disppll子节点widget
    void createClkSysDispSubNodeWidget(const QString& nodeName, QWidget* parent);  // 新增：创建clk_sys_disp子节点widget
    void createClkA0PLLSubNodeWidget(const QString& nodeName, QWidget* parent);  // 新增：创建clk_a0pll子节点widget
    void createClkRVPLLSubNodeWidget(const QString& nodeName, QWidget* parent);  // 新增：创建clk_rvpll子节点widget
    void createClkAPPLLSubNodeWidget(const QString& nodeName, QWidget* parent);  // 新增：创建clk_appll子节点widget
    void createClkFPLLSubNodeWidget(const QString& nodeName, QWidget* parent);  // 新增：创建clk_fpll子节点widget
    void createClkTPLLSubNodeWidget(const QString& nodeName, QWidget* parent);  // 新增：创建clk_tpll子节点widget
    void createClkMPLLSubNodeWidget(const QString& nodeName, QWidget* parent);  // 新增：创建clk_mpll子节点widget
    void createClkFAB100MSubNodeWidget(const QString& nodeName, QWidget* parent);  // 新增：创建clk_fab_100M子节点widget
    void createClkSPINANDSubNodeWidget(const QString& nodeName, QWidget* parent);  // 新增：创建clk_spi_nand子节点widget
    void createClkHSPeriSubNodeWidget(const QString& nodeName, QWidget* parent);  // 新增：创建clk_hsperi子节点widget
    void updatePLLFrequency(const QString& pllName);
    void updateSubPLLFrequency(const QString& pllName);  // 新增
    void updateAllSubPLLFrequencies();  // 新增
    void updateOutputFrequency(const QString& outputName);
    void updateClk1MSubNodeFrequency(const QString& nodeName);  // 新增：更新clk_1M子节点频率
    void updateAllClk1MSubNodeFrequencies();  // 新增：更新所有clk_1M子节点频率
    void updateClkCam1PLLSubNodeFrequency(const QString& nodeName);  // 新增：更新clk_cam1pll子节点频率
    void updateAllClkCam1PLLSubNodeFrequencies();  // 新增：更新所有clk_cam1pll子节点频率
    void updateClkRawAxiSubNodeFrequency(const QString& nodeName);  // 新增：更新clk_raw_axi子节点频率
    void updateAllClkRawAxiSubNodeFrequencies();  // 新增：更新所有clk_raw_axi子节点频率
    void updateClkCam0PLLSubNodeFrequency(const QString& nodeName);  // 新增：更新clk_cam0pll子节点频率
    void updateAllClkCam0PLLSubNodeFrequencies();  // 新增：更新所有clk_cam0pll子节点频率
    void updateClkDispPLLSubNodeFrequency(const QString& nodeName);  // 新增：更新clk_disppll子节点频率
    void updateAllClkDispPLLSubNodeFrequencies();  // 新增：更新所有clk_disppll子节点频率
    void updateClkSysDispSubNodeFrequency(const QString& nodeName);  // 新增：更新clk_sys_disp子节点频率
    void updateAllClkSysDispSubNodeFrequencies();  // 新增：更新所有clk_sys_disp子节点频率
    void updateClkA0PLLSubNodeFrequency(const QString& nodeName);  // 新增：更新clk_a0pll子节点频率
    void updateAllClkA0PLLSubNodeFrequencies();  // 新增：更新所有clk_a0pll子节点频率
    void updateClkRVPLLSubNodeFrequency(const QString& nodeName);  // 新增：更新clk_rvpll子节点频率
    void updateAllClkRVPLLSubNodeFrequencies();  // 新增：更新所有clk_rvpll子节点频率
    void updateClkAPPLLSubNodeFrequency(const QString& nodeName);  // 新增：更新clk_appll子节点频率
    void updateAllClkAPPLLSubNodeFrequencies();  // 新增：更新所有clk_appll子节点频率
    void updateClkFPLLSubNodeFrequency(const QString& nodeName);  // 新增：更新clk_fpll子节点频率
    void updateAllClkFPLLSubNodeFrequencies();  // 新增：更新所有clk_fpll子节点频率
    void updateClkTPLLSubNodeFrequency(const QString& nodeName);  // 新增：更新clk_tpll子节点频率
    void updateAllClkTPLLSubNodeFrequencies();  // 新增：更新所有clk_tpll子节点频率
    void updateClkMPLLSubNodeFrequency(const QString& nodeName);  // 新增：更新clk_mpll子节点频率
    void updateAllClkMPLLSubNodeFrequencies();  // 新增：更新所有clk_mpll子节点频率
    void updateClkFAB100MSubNodeFrequency(const QString& nodeName);  // 新增：更新clk_fab_100M子节点频率
    void updateAllClkFAB100MSubNodeFrequencies();  // 新增：更新所有clk_fab_100M子节点频率
    void updateClkSPINANDSubNodeFrequency(const QString& nodeName);  // 新增：更新clk_spi_nand子节点频率
    void updateAllClkSPINANDSubNodeFrequencies();  // 新增：更新所有clk_spi_nand子节点频率
    void updateClkHSPeriSubNodeFrequency(const QString& nodeName);  // 新增：更新clk_hsperi子节点频率
    void updateAllClkHSPeriSubNodeFrequencies();  // 新增：更新所有clk_hsperi子节点频率

    void connectSignals();
    
    // 连接线绘制相关方法
    void drawConnectionLines(QPainter& painter);
    void drawArrowLine(QPainter& painter, const QPoint& start, const QPoint& end, const QColor& color = Qt::blue);
    void drawElbowArrowLine(QPainter& painter, const QPoint& start, const QPoint& elbow1, const QPoint& elbow2, const QPoint& end, const QColor& color = Qt::blue);
    void calculateElbowPoints(const QPoint& start, const QPoint& end, QPoint& elbow1, QPoint& elbow2) const;  // 新增：计算肘形拐点
    QPoint getOSCConnectionPoint() const;
    QPoint getPLLConnectionPoint(const QString& pllName) const;
    QPoint getSubPLLConnectionPoint() const;  // 新增：获取子PLL连接点
    QPoint getMIPIMPLLConnectionPoint() const;  // 新增：获取MIPIMPLL输出连接点
    QPoint getOutputAreaConnectionPoint() const;  // 新增：获取输出区域连接点
    QPoint getClk1MConnectionPoint() const;  // 新增：获取clk_1M连接点
    QPoint getClk1MSubNodeConnectionPoint() const;  // 新增：获取clk_1M子节点连接点
    QPoint getClkCam1PLLConnectionPoint() const;  // 新增：获取clk_cam1pll连接点
    QPoint getClkCam1PLLSubNodeConnectionPoint() const;  // 新增：获取clk_cam1pll子节点连接点
    QPoint getClkRawAxiConnectionPoint() const;  // 新增：获取clk_raw_axi连接点
    QPoint getClkRawAxiSubNodeConnectionPoint() const;  // 新增：获取clk_raw_axi子节点连接点
    QPoint getClkCam0PLLConnectionPoint() const;  // 新增：获取clk_cam0pll连接点
    QPoint getClkCam0PLLSubNodeConnectionPoint() const;  // 新增：获取clk_cam0pll子节点连接点
    QPoint getClkDispPLLConnectionPoint() const;  // 新增：获取clk_disppll连接点
    QPoint getClkDispPLLSubNodeConnectionPoint() const;  // 新增：获取clk_disppll子节点连接点
    QPoint getClkSysDispConnectionPoint() const;  // 新增：获取clk_sys_disp连接点
    QPoint getClkSysDispSubNodeConnectionPoint() const;  // 新增：获取clk_sys_disp子节点连接点
    QPoint getClkA0PLLConnectionPoint() const;  // 新增：获取clk_a0pll连接点
    QPoint getClkA0PLLSubNodeConnectionPoint() const;  // 新增：获取clk_a0pll子节点连接点
    QPoint getClkRVPLLConnectionPoint() const;  // 新增：获取clk_rvpll连接点
    QPoint getClkRVPLLSubNodeConnectionPoint() const;  // 新增：获取clk_rvpll子节点连接点
    QPoint getClkAPPLLConnectionPoint() const;  // 新增：获取clk_appll连接点
    QPoint getClkAPPLLSubNodeConnectionPoint() const;  // 新增：获取clk_appll子节点连接点
    QPoint getClkFPLLConnectionPoint() const;  // 新增：获取clk_fpll连接点
    QPoint getClkFPLLSubNodeConnectionPoint() const;  // 新增：获取clk_fpll子节点连接点
    QPoint getClkTPLLConnectionPoint() const;  // 新增：获取clk_tpll连接点
    QPoint getClkTPLLSubNodeConnectionPoint() const;  // 新增：获取clk_tpll子节点连接点
    QPoint getClkMPLLConnectionPoint() const;  // 新增：获取clk_mpll连接点
    QPoint getClkMPLLSubNodeConnectionPoint() const;  // 新增：获取clk_mpll子节点连接点
    QPoint getClkFAB100MConnectionPoint() const;  // 新增：获取clk_fab_100M连接点
    QPoint getClkFAB100MSubNodeConnectionPoint() const;  // 新增：获取clk_fab_100M子节点连接点
    QPoint getClkSPINANDConnectionPoint() const;  // 新增：获取clk_spi_nand连接点
    QPoint getClkSPINANDSubNodeConnectionPoint() const;  // 新增：获取clk_spi_nand子节点连接点
    QPoint getClkHSPeriConnectionPoint() const;  // 新增：获取clk_hsperi连接点
    QPoint getClkHSPeriSubNodeConnectionPoint() const;  // 新增：获取clk_hsperi子节点连接点
    void updateConnectionOverlay();
    
    // 拖拽和缩放辅助方法
    QWidget* getWidgetAt(const QPoint& pos);
    ResizeDirection getResizeDirection(const QPoint& pos, const QRect& widgetRect);
    void updateCursor(const QPoint& pos);
    void updateWidgetGeometry(QWidget* widget, const QRect& newGeometry);
    QString getWidgetModuleName(QWidget* widget);
    QWidget* getWidgetByModuleName(const QString& moduleName);  // 新增：根据模块名称获取widget
    void drawResizeHandles(QPainter& painter, const QRect& rect);
    QRect getResizeHandleRect(const QRect& widgetRect, ResizeDirection direction);
    QPoint convertToFlowWidgetCoordinate(const QPoint& pos) const;  // 新增：坐标转换方法
    
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
    QWidget* m_clkCam1PLLSubNodeWidget;
    QVBoxLayout* m_clkCam1PLLSubNodeLayout;
    QWidget* m_clkRawAxiSubNodeWidget;
    QVBoxLayout* m_clkRawAxiSubNodeLayout;
    QWidget* m_clkCam0PLLSubNodeWidget;
    QVBoxLayout* m_clkCam0PLLSubNodeLayout;
    QWidget* m_clkDispPLLSubNodeWidget;
    QWidget* m_clkSysDispSubNodeWidget;
    QVBoxLayout* m_clkDispPLLSubNodeLayout;
    QVBoxLayout* m_clkSysDispSubNodeLayout;
    
    // clk_a0pll子节点区域
    QWidget* m_clkA0PLLSubNodeWidget;
    QVBoxLayout* m_clkA0PLLSubNodeLayout;

    // clk_rvpll子节点区域
    QWidget* m_clkRVPLLSubNodeWidget;
    QVBoxLayout* m_clkRVPLLSubNodeLayout;

    // clk_appll子节点区域
    QWidget* m_clkAPPLLSubNodeWidget;
    QVBoxLayout* m_clkAPPLLSubNodeLayout;

    // clk_fpll子节点区域
    QWidget* m_clkFPLLSubNodeWidget;
    QVBoxLayout* m_clkFPLLSubNodeLayout;

    // clk_tpll子节点区域
    QWidget* m_clkTPLLSubNodeWidget;
    QVBoxLayout* m_clkTPLLSubNodeLayout;

    // clk_mpll子节点区域
    QWidget* m_clkMPLLSubNodeWidget;
    QVBoxLayout* m_clkMPLLSubNodeLayout;

    // clk_fab_100M子节点区域
    QWidget* m_clkFAB100MSubNodeWidget;
    QVBoxLayout* m_clkFAB100MSubNodeLayout;

    // clk_spi_nand子节点区域
    QWidget* m_clkSPINANDSubNodeWidget;
    QVBoxLayout* m_clkSPINANDSubNodeLayout;

    // clk_hsperi子节点区域
    QWidget* m_clkHSPeriSubNodeWidget;
    QVBoxLayout* m_clkHSPeriSubNodeLayout;

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
    
    // clk子节点配置组
    QMap<QString, QWidget*> m_clk1MSubNodeWidgets;
    QMap<QString, QLabel*> m_clk1MSubNodeFreqLabels;
    QMap<QString, QSpinBox*> m_clk1MSubNodeDividerBoxes;
    QMap<QString, QWidget*> m_clkCam1PLLSubNodeWidgets;
    QMap<QString, QLabel*> m_clkCam1PLLSubNodeFreqLabels;
    QMap<QString, QSpinBox*> m_clkCam1PLLSubNodeDividerBoxes;
    QMap<QString, QWidget*> m_clkRawAxiSubNodeWidgets;
    QMap<QString, QLabel*> m_clkRawAxiSubNodeFreqLabels;
    QMap<QString, QSpinBox*> m_clkRawAxiSubNodeDividerBoxes;
    QMap<QString, QWidget*> m_clkCam0PLLSubNodeWidgets;
    QMap<QString, QLabel*> m_clkCam0PLLSubNodeFreqLabels;
    QMap<QString, QSpinBox*> m_clkCam0PLLSubNodeDividerBoxes;
    QMap<QString, QWidget*> m_clkDispPLLSubNodeWidgets;
    QMap<QString, QWidget*> m_clkSysDispSubNodeWidgets;
    QMap<QString, QLabel*> m_clkDispPLLSubNodeFreqLabels;
    QMap<QString, QSpinBox*> m_clkDispPLLSubNodeDividerBoxes;
    QMap<QString, QLabel*> m_clkSysDispSubNodeFreqLabels;
    QMap<QString, QSpinBox*> m_clkSysDispSubNodeDividerBoxes;
    QMap<QString, QWidget*> m_clkA0PLLSubNodeWidgets;
    QMap<QString, QLabel*> m_clkA0PLLSubNodeFreqLabels;
    QMap<QString, QSpinBox*> m_clkA0PLLSubNodeDividerBoxes;
    QMap<QString, QWidget*> m_clkRVPLLSubNodeWidgets;
    QMap<QString, QLabel*> m_clkRVPLLSubNodeFreqLabels;
    QMap<QString, QSpinBox*> m_clkRVPLLSubNodeDividerBoxes;
    QMap<QString, QWidget*> m_clkAPPLLSubNodeWidgets;
    QMap<QString, QLabel*> m_clkAPPLLSubNodeFreqLabels;
    QMap<QString, QSpinBox*> m_clkAPPLLSubNodeDividerBoxes;
    QMap<QString, QWidget*> m_clkFPLLSubNodeWidgets;
    QMap<QString, QLabel*> m_clkFPLLSubNodeFreqLabels;
    QMap<QString, QSpinBox*> m_clkFPLLSubNodeDividerBoxes;
    QMap<QString, QWidget*> m_clkTPLLSubNodeWidgets;
    QMap<QString, QLabel*> m_clkTPLLSubNodeFreqLabels;
    QMap<QString, QSpinBox*> m_clkTPLLSubNodeDividerBoxes;
    QMap<QString, QWidget*> m_clkMPLLSubNodeWidgets;
    QMap<QString, QLabel*> m_clkMPLLSubNodeFreqLabels;
    QMap<QString, QSpinBox*> m_clkMPLLSubNodeDividerBoxes;
    QMap<QString, QWidget*> m_clkFAB100MSubNodeWidgets;
    QMap<QString, QLabel*> m_clkFAB100MSubNodeFreqLabels;
    QMap<QString, QSpinBox*> m_clkFAB100MSubNodeDividerBoxes;
    QMap<QString, QWidget*> m_clkSPINANDSubNodeWidgets;
    QMap<QString, QLabel*> m_clkSPINANDSubNodeFreqLabels;
    QMap<QString, QSpinBox*> m_clkSPINANDSubNodeDividerBoxes;
    QMap<QString, QWidget*> m_clkHSPeriSubNodeWidgets;
    QMap<QString, QLabel*> m_clkHSPeriSubNodeFreqLabels;
    QMap<QString, QSpinBox*> m_clkHSPeriSubNodeDividerBoxes;
    
    // 控制按钮
    QHBoxLayout* m_buttonLayout;
    QPushButton* m_resetButton;
    QPushButton* m_applyButton;
    QPushButton* m_positionConfigButton;  // 新增：位置配置按钮
    
    // 数据存储
    QMap<QString, PLLConfig> m_pllConfigs;
    QMap<QString, ClockOutput> m_outputs;
    QMap<QString, ClockOutput> m_clk1MSubNodes;  // 新增：clk_1M子节点数据
    QMap<QString, ClockOutput> m_clkCam1PLLSubNodes;  // 新增：clk_cam1pll子节点数据
    QMap<QString, ClockOutput> m_clkRawAxiSubNodes;  // 新增：clk_raw_axi子节点数据
    QMap<QString, ClockOutput> m_clkCam0PLLSubNodes;  // 新增：clk_cam0pll子节点数据
    QMap<QString, ClockOutput> m_clkDispPLLSubNodes;  // 新增：clk_disppll子节点数据
    QMap<QString, ClockOutput> m_clkSysDispSubNodes;  // 新增：clk_sys_disp子节点数据
    QMap<QString, ClockOutput> m_clkA0PLLSubNodes;  // 新增：clk_a0pll子节点数据
    QMap<QString, ClockOutput> m_clkRVPLLSubNodes;  // 新增：clk_rvpll子节点数据
    QMap<QString, ClockOutput> m_clkAPPLLSubNodes;  // 新增：clk_appll子节点数据
    QMap<QString, ClockOutput> m_clkFPLLSubNodes;  // 新增：clk_fpll子节点数据
    QMap<QString, ClockOutput> m_clkTPLLSubNodes;  // 新增：clk_tpll子节点数据
    QMap<QString, ClockOutput> m_clkMPLLSubNodes;  // 新增：clk_mpll子节点数据
    QMap<QString, ClockOutput> m_clkFAB100MSubNodes;  // 新增：clk_fab_100M子节点数据
    QMap<QString, ClockOutput> m_clkSPINANDSubNodes;  // 新增：clk_spi_nand子节点数据
    QMap<QString, ClockOutput> m_clkHSPeriSubNodes;  // 新增：clk_hsperi子节点数据
    QMap<QString, ModulePosition> m_modulePositions;  // 新增：模块位置配置
    
    // 常量
    static const double OSC_FREQUENCY;  // 25MHz
    static const double RTC_FREQUENCY;  // 32.768kHz
    static const QStringList PLL_NAMES;
    static const QStringList SUB_PLL_NAMES;  // 新增：子PLL名称列表
    static const QStringList OUTPUT_NAMES;
    static const QStringList CLK_1M_SUB_NODES;  // 新增：clk_1M子节点列表
    static const QStringList CLK_CAM1PLL_SUB_NODES;  // 新增：clk_cam1pll子节点列表
    static const QStringList CLK_RAW_AXI_SUB_NODES;  // 新增：clk_raw_axi子节点列表
    static const QStringList CLK_CAM0PLL_SUB_NODES;  // 新增：clk_cam0pll子节点列表
    static const QStringList CLK_DISPPLL_SUB_NODES;  // 新增：clk_disppll子节点列表
    static const QStringList CLK_SYS_DISP_SUB_NODES;  // 新增：clk_sys_disp子节点列表
    static const QStringList CLK_A0PLL_SUB_NODES;  // 新增：clk_a0pll子节点列表
    static const QStringList CLK_RVPLL_SUB_NODES;  // 新增：clk_rvpll子节点列表
    static const QStringList CLK_APPLL_SUB_NODES;  // 新增：clk_appll子节点列表
    static const QStringList CLK_FPLL_SUB_NODES;  // 新增：clk_fpll子节点列表
    static const QStringList CLK_TPLL_SUB_NODES;  // 新增：clk_tpll子节点列表
    static const QStringList CLK_MPLL_SUB_NODES;  // 新增：clk_mpll子节点列表
    static const QStringList CLK_FAB_100M_SUB_NODES;  // 新增：clk_fab_100M子节点列表
    static const QStringList CLK_SPI_NAND_SUB_NODES;  // 新增：clk_spi_nand子节点列表
    static const QStringList CLK_HSPERI_SUB_NODES;  // 新增：clk_hsperi子节点列表
    
    // 拖拽和缩放相关的成员变量
    bool m_isDragging;              // 是否正在拖拽
    bool m_isResizing;              // 是否正在缩放
    QWidget* m_selectedWidget;      // 当前选中的widget
    QPoint m_lastMousePos;          // 上次鼠标位置
    QPoint m_dragStartPos;          // 拖拽开始位置
    QRect m_originalGeometry;       // 原始几何形状
    ResizeDirection m_resizeDirection;  // 缩放方向
    int m_handleSize;               // 缩放手柄大小
    
    // 连接线覆盖层
    QWidget* m_connectionOverlay;
};

#endif // CLOCKCONFIG_H
