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
    int divider;          // 分频器值
    int multiplier;       // 倍频器值
    QString source;       // 时钟源
};

// 时钟输出通道结构
struct ClockChannel {
    QString name;         // 通道名称
    QString pllSource1;   // PLL源1
    QString pllSource2;   // PLL源2
    int divider1;        // 分频器1
    int divider2;        // 分频器2
    int muxSelection;    // MUX选择(0=源1, 1=源2)
    double frequency;    // 输出频率(MHz)
    bool enabled;        // 是否启用
};

// 时钟输出结构
struct ClockOutput {
    QString name;        // 输出名称
    QString source;      // 时钟源
    int divider;        // 分频器值
    double frequency;   // 频率(MHz)
    bool enabled;       // 是否启用
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
    
    // 获取和设置时钟通道配置
    ClockChannel getClockChannel(const QString& channelName) const;
    void setClockChannel(const QString& channelName, const ClockChannel& channel);
    
    // 保存和加载配置
    bool saveConfig(const QString& filePath);
    bool loadConfig(const QString& filePath);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

signals:
    void configChanged();
    void pllConfigChanged(const QString& pllName);
    void channelConfigChanged(const QString& channelName);

private slots:
    void onPLLMultiplierChanged(const QString& pllName, int multiplier);
    void onClockDividerChanged(const QString& outputName, int divider);
    void onClockTreeItemClicked(QTreeWidgetItem* item, int column);
    void onChannelConfigChanged(const QString& channelName);
    void onChannelMuxChanged(const QString& channelName, int selection);
    void updateFrequencies();
    void resetToDefaults();

private:
    void setupUI();
    void setupClockSources();
    void setupPLLs();
    void setupClockChannels();
    void setupOutputs();
    void setupClockTree();
    void createPLLWidget(const QString& pllName, QWidget* parent);
    void createClockChannelWidget(const QString& channelName, QWidget* parent);
    void createOutputWidget(const QString& outputName, QWidget* parent);
    void updatePLLFrequency(const QString& pllName);
    void updateChannelFrequency(const QString& channelName);
    void updateOutputFrequency(const QString& outputName);
    void connectSignals();
    
    // 连接线绘制相关方法
    void drawConnectionLines(QPainter& painter);
    void drawArrowLine(QPainter& painter, const QPoint& start, const QPoint& end, const QColor& color = Qt::blue);
    QPoint getOSCConnectionPoint() const;
    QPoint getPLLConnectionPoint(const QString& pllName) const;
    void updateConnectionOverlay();
    
    // UI组件
    QVBoxLayout* m_mainLayout;
    
    // 时钟流程显示区域
    QScrollArea* m_flowScrollArea;
    QWidget* m_flowWidget;
    QHBoxLayout* m_flowLayout;
    
    // 输入源区域
    QWidget* m_inputWidget;
    QVBoxLayout* m_inputLayout;
    
    // PLL区域
    QWidget* m_pllWidget;
    QVBoxLayout* m_pllLayout;
    
    // 时钟通道区域
    QWidget* m_channelWidget;
    QVBoxLayout* m_channelLayout;
    
    // 输出区域
    QWidget* m_outputWidget;
    QVBoxLayout* m_outputLayout;
    
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
    
    // 时钟通道配置组
    QGroupBox* m_channelGroup;
    QVBoxLayout* m_channelGroupLayout;
    QMap<QString, QWidget*> m_channelWidgets;
    QMap<QString, QComboBox*> m_channelSource1Boxes;
    QMap<QString, QComboBox*> m_channelSource2Boxes;
    QMap<QString, QSpinBox*> m_channelDiv1Boxes;
    QMap<QString, QSpinBox*> m_channelDiv2Boxes;
    QMap<QString, QComboBox*> m_channelMuxBoxes;
    QMap<QString, QLabel*> m_channelFreqLabels;
    
    // 输出配置组
    QGroupBox* m_outputGroup;
    QVBoxLayout* m_outputGroupLayout;
    QMap<QString, QWidget*> m_outputWidgets;
    QMap<QString, QLabel*> m_outputFreqLabels;
    
    // 控制按钮
    QHBoxLayout* m_buttonLayout;
    QPushButton* m_resetButton;
    QPushButton* m_applyButton;
    
    // 数据存储
    QMap<QString, PLLConfig> m_pllConfigs;
    QMap<QString, ClockChannel> m_clockChannels;
    QMap<QString, ClockOutput> m_outputs;
    
    // 常量
    static const double OSC_FREQUENCY;  // 25MHz
    static const double RTC_FREQUENCY;  // 32.768kHz
    static const QStringList PLL_NAMES;
    static const QStringList CHANNEL_NAMES;
    static const QStringList OUTPUT_NAMES;
    
    // 连接线覆盖层
    QWidget* m_connectionOverlay;
};

#endif // CLOCKCONFIG_H
