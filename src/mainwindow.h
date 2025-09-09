#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QStackedWidget>
#include <QScrollArea>
#include <QMessageBox>
#include <QFileDialog>
#include <QLineEdit>
#include <QTimer>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QSplitter>
#include <QMenu>
#include <QAction>
#include <QTabWidget>
#include "chipconfig.h"
#include "pinwidget.h"
#include "codegenerator.h"
#include "dtsconfig.h"
#include "clockconfig.h"

QT_BEGIN_NAMESPACE
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onChipSelectionChanged();
    void onStartProject();
    void onGenerateCode();
    void onPinFunctionChanged(const QString& pinName, const QString& function);
    void onSearchTextChanged(const QString& text);
    void onBlinkTimeout();
    void onPeripheralItemClicked(QTreeWidgetItem* item, int column);
    void onPeripheralCheckBoxChanged(const QString& peripheral, bool enabled);
    void onClockConfigChanged();
    void onConfigTabChanged(int index);

private:
    void setupUI();
    void setupPinoutTab();
    void setupPinoutConfigPanel();
    void setupClockTab();
    void setupChipView();
    void createQFNLayout();
    void createBGALayout();
    void clearPinLayout();
    
    // 引脚名称映射：BGA位置 -> 实际PAD名称
    QString mapPinName(const QString& bgaPosition) const;
    void initializePinNameMappings();
    
    // 搜索功能
    void setupSearchBox();
    void highlightPin(const QString& pinName, bool highlight);
    
    // defconfig文件处理
    bool loadPeripheralStates();
    bool savePeripheralStates();
    QString getDefconfigPath() const;
    QMap<QString, QStringList> getPeripheralConfigs() const;
    
    // 设备树配置处理
    void initializeDtsConfig();
    void showPeripheralConfig(const QString& peripheralType);

    // UI Components
    QWidget *m_centralWidget;
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_headerLayout;
    QHBoxLayout *m_controlLayout;
    
    // 顶部配置标签页
    QTabWidget *m_configTabWidget;
    QWidget *m_pinoutTab;
    QWidget *m_clockTab;
    
    // Pinout配置页面的分隔器和布局
    QSplitter *m_pinoutSplitter;
    QWidget *m_pinoutConfigPanel;
    QVBoxLayout *m_pinoutConfigLayout;
    QTreeWidget *m_pinoutConfigTree;
    
    // 右侧内容区域
    QWidget *m_contentWidget;
    QVBoxLayout *m_contentLayout;
    
    QLabel *m_titleLabel;
    QComboBox *m_chipComboBox;
    QPushButton *m_startProjectButton;
    QPushButton *m_generateCodeButton;
    
    QStackedWidget *m_stackedWidget;
    QWidget *m_welcomePage;
    QScrollArea *m_chipViewPage;
    QWidget *m_chipContainer;
    QGridLayout *m_pinLayout;
    
    // 时钟配置页面
    ClockConfigWidget *m_clockConfigPage;
    
    // 搜索功能组件
    QHBoxLayout *m_searchLayout;
    QLabel *m_searchLabel;
    QLineEdit *m_searchLineEdit;
    QTimer *m_blinkTimer;
    
    // Data
    ChipConfig m_chipConfig;
    QString m_selectedChip;
    QMap<QString, PinWidget*> m_pinWidgets;
    CodeGenerator m_codeGenerator;
    
    // BGA位置到PAD名称的映射表
    QMap<QString, QString> m_pinNameMappings;
    
    // 搜索功能相关
    QString m_currentSearchText;
    PinWidget* m_highlightedPin;
    bool m_blinkState;
    
    // 外设配置状态
    QMap<QString, bool> m_peripheralStates;
    
    // 设备树配置管理器
    DtsConfig *m_dtsConfig;
};

#endif // MAINWINDOW_H
