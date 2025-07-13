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
#include "chipconfig.h"
#include "pinwidget.h"
#include "codegenerator.h"

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

private:
    void setupUI();
    void setupChipView();
    void createQFNLayout();
    void createBGALayout();
    void clearPinLayout();

    // UI Components
    QWidget *m_centralWidget;
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_headerLayout;
    QHBoxLayout *m_controlLayout;
    
    QLabel *m_titleLabel;
    QComboBox *m_chipComboBox;
    QPushButton *m_startProjectButton;
    QPushButton *m_generateCodeButton;
    
    QStackedWidget *m_stackedWidget;
    QWidget *m_welcomePage;
    QScrollArea *m_chipViewPage;
    QWidget *m_chipContainer;
    QGridLayout *m_pinLayout;
    
    // Data
    ChipConfig m_chipConfig;
    QString m_selectedChip;
    QMap<QString, PinWidget*> m_pinWidgets;
    CodeGenerator m_codeGenerator;
};

#endif // MAINWINDOW_H
