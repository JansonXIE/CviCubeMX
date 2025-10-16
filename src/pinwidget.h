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

    /**
     * @brief 设置引脚是否闪烁高亮
     * @param highlight 是否高亮
     * @param blinkState 闪烁状态（用于控制闪烁效果）
     */
    void setHighlight(bool highlight, bool blinkState = false);

    /**
     * @brief 获取引脚是否处于高亮状态
     * @return 是否高亮
     */
    bool isHighlighted() const;

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

    // 高亮闪烁相关
    bool m_isHighlighted;
    bool m_blinkState;
    // 是否被用户主动配置过（用于区分“默认初始值”与“已配置”）
    bool m_userConfigured;
};

#endif // PINWIDGET_H
