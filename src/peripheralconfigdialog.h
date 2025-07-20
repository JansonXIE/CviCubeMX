#ifndef PERIPHERALCONFIGDIALOG_H
#define PERIPHERALCONFIGDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
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
    QLabel *m_freqUnitLabel;
    
    QLabel *m_pwmCellsLabel;
    QSpinBox *m_pwmCellsSpinBox;
    
    QLabel *m_currentSpeedLabel;
    QSpinBox *m_currentSpeedSpinBox;
    
    QHBoxLayout *m_buttonLayout;
    QPushButton *m_applyButton;
    QPushButton *m_cancelButton;
    
    // 当前选中的外设信息
    QString m_currentPeripheral;
    PeripheralInfo m_currentInfo;
};

#endif // PERIPHERALCONFIGDIALOG_H
