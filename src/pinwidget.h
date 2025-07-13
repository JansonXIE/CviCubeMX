#ifndef PINWIDGET_H
#define PINWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QMenu>
#include <QAction>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include "pinfunction.h"

class PinWidget : public QPushButton
{
    Q_OBJECT

public:
    explicit PinWidget(const QString& pinName, bool isSquare = true, QWidget *parent = nullptr);
    
    void setFunction(const QString& function);
    QString getFunction() const;
    QString getPinName() const;
    
    /**
     * @brief 设置显示名称（用于悬浮提示）
     * @param displayName 显示名称，如"A2"
     */
    void setDisplayName(const QString& displayName);
    
    /**
     * @brief 获取显示名称
     * @return 显示名称
     */
    QString getDisplayName() const;
    
    /**
     * @brief 设置引脚支持的功能列表
     * @param functions 支持的功能列表
     */
    void setSupportedFunctions(const QStringList& functions);
    
    /**
     * @brief 获取引脚支持的功能列表
     * @return 支持的功能列表
     */
    QStringList getSupportedFunctions() const;

signals:
    void functionChanged(const QString& pinName, const QString& function);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void onFunctionSelected();

private:
    void setupContextMenu();
    void updateButtonStyle();
    void initializePinFunctions();
    void updateTooltip();  // 更新悬浮提示
    
    QString m_pinName;        // 实际引脚名称（如PAD_MIPI_TXM4）
    QString m_displayName;    // 显示名称（如A2）
    QString m_function;
    bool m_isSquare;
    QMenu *m_contextMenu;
    QStringList m_functions;
    PinFunction m_pinFunction;
};

#endif // PINWIDGET_H
