#ifndef PERIPHERALCONFIGDIALOG_H
#define PERIPHERALCONFIGDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QGroupBox>
#include <QCheckBox>
#include <QSpinBox>
#include "dtsconfig.h"

class PeripheralConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PeripheralConfigDialog(const QString &peripheralType, DtsConfig *dtsConfig, QWidget *parent = nullptr);

private slots:
    void onApplyClicked();
    void onCancelClicked();
    void onPeripheralSelectionChanged();
    void onStatusChanged();
    void onClockChanged();
    void onFreqChanged();
    void onPwmCellsChanged();
    void onCurrentSpeedChanged();
    void onSysdmaChannelChanged();
    void onThermalTripChanged();
    void onTripSelectionChanged();


private:
    void setupUI();
    void setupPeripheralSpecificUI(int &currentRow);
    void setupConnections();
    void setupStyles();
    void updatePeripheralList();
    void loadPeripheralConfig();
    void savePeripheralConfig();

private:
    QString m_peripheralType;
    DtsConfig *m_dtsConfig;
    // UI组件
    QVBoxLayout *m_mainLayout;
    QGroupBox *m_peripheralGroup;
    QGridLayout *m_peripheralLayout;
    QLabel *m_peripheralLabel;
    QComboBox *m_peripheralComboBox;
    QLabel *m_statusLabel;
    QComboBox *m_statusComboBox;
    QLabel *m_clockLabel;
    QLineEdit *m_clockLineEdit;
    QLabel *m_freqLabel;
    QSpinBox *m_freqSpinBox;
    QDoubleSpinBox *m_freqDoubleSpinBox; // 新增：UART 使用
    QLabel *m_freqUnitLabel;
    QLabel *m_pwmCellsLabel;
    QSpinBox *m_pwmCellsSpinBox;
    QLabel *m_currentSpeedLabel;
    QSpinBox *m_currentSpeedSpinBox;
    QComboBox *m_baudRateComboBox;       // 新增：波特率下拉
    QLineEdit *m_customBaudLineEdit;     // 新增：自定义波特率输入
    // Thermal 控件
    QLabel *m_thermalZoneLabel;
    QComboBox *m_thermalZoneCombo;
    QLabel *m_pdpLabel; QSpinBox *m_pdpSpin;
    QLabel *m_pdLabel;  QSpinBox *m_pdSpin;
    QLabel *m_sensorLabel; QComboBox *m_sensorCombo;
    QLabel *m_tripSelectLabel; QComboBox *m_tripSelectCombo;
    QLabel *m_tripTempLabel; QSpinBox *m_tripTempSpin;
    QLabel *m_tripHystLabel; QSpinBox *m_tripHystSpin;
    QLabel *m_tripTypeLabel; QComboBox *m_tripTypeCombo;
    QLabel *m_mapInfoLabel; // 只读显示 maps/cooling 信息
    // SYSDMA 通道控件
    QLabel *m_sysdmaChannelLabels[8];    // 8个通道的标签
    QComboBox *m_sysdmaChannelComboBoxes[8]; // 8个通道的下拉框
    QHBoxLayout *m_buttonLayout;
    QPushButton *m_applyButton;
    QPushButton *m_cancelButton;
    // 当前选中的外设信息
    QString m_currentPeripheral;
    PeripheralInfo m_currentInfo;
    // Thermal: 记录上一次选中的 trip 名称，用于在切换时先保存旧值
    QString m_lastTripName;
};

#endif // PERIPHERALCONFIGDIALOG_H
