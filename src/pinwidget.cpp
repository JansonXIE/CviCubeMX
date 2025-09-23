#include "pinwidget.h"
#include <QApplication>

PinWidget::PinWidget(const QString& pinName, bool isSquare, QWidget *parent)
    : QPushButton(parent)
    , m_pinName(pinName)
    , m_function("GPIO")
    , m_isSquare(isSquare)
    , m_contextMenu(nullptr)
    , m_isHighlighted(false)
    , m_blinkState(false)
{
    // 设置基本属性
    setMinimumSize(25, 25);
    setMaximumSize(25, 25);

    // 初始化显示名称为引脚名称
    m_displayName = m_pinName;

    // 初始化引脚功能
    initializePinFunctions();

    // 设置上下文菜单
    setupContextMenu();

    // 更新按钮样式
    updateButtonStyle();

    // 更新悬浮提示
    updateTooltip();
}

void PinWidget::setFunction(const QString& function)
{
    if (m_functions.contains(function)) {
        m_function = function;
        updateButtonStyle();
        updateTooltip();
        emit functionChanged(m_pinName, m_function);
    }
}

QString PinWidget::getFunction() const
{
    return m_function;
}

QString PinWidget::getPinName() const
{
    return m_pinName;
}

void PinWidget::setDisplayName(const QString& displayName)
{
    m_displayName = displayName;
    updateTooltip();
}

QString PinWidget::getDisplayName() const
{
    return m_displayName;
}

void PinWidget::setSupportedFunctions(const QStringList& functions)
{
    m_functions = functions;

    // 如果当前功能不在支持列表中，设置为默认功能
    if (!m_functions.contains(m_function)) {
        m_function = m_pinFunction.getDefaultFunction(m_pinName);
    }

    // 重新设置上下文菜单
    setupContextMenu();
    updateButtonStyle();
    updateTooltip();
}

QStringList PinWidget::getSupportedFunctions() const
{
    return m_functions;
}

void PinWidget::setHighlight(bool highlight, bool blinkState)
{
    m_isHighlighted = highlight;
    m_blinkState = blinkState;
    update(); // 触发重绘
}

bool PinWidget::isHighlighted() const
{
    return m_isHighlighted;
}

void PinWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 获取颜色
    QColor color;
    if (m_function == "GPIO" || m_function.startsWith("XGPIO")) {
        color = QColor("#95a5a6");  // 灰色
    } else if (m_function == "ADC" || m_function.contains("ADC")) {
        color = QColor("#e74c3c");  // 红色
    } else if (m_function == "I2C" || m_function.contains("IIC")) {
        color = QColor("#3498db");  // 蓝色
    } else if (m_function == "UART" || m_function.contains("UART")) {
        color = QColor("#2ecc71");  // 绿色
    } else if (m_function == "SPI" || m_function.contains("SPI")) {
        color = QColor("#f39c12");  // 橙色
    } else if (m_function == "PWM" || m_function.contains("PWM")) {
        color = QColor("#9b59b6");  // 紫色
    } else if (m_function == "Timer" || m_function.contains("TIMER")) {
        color = QColor("#e67e22");  // 棕色
    } else if (m_function.contains("CAM")) {
        color = QColor("#1abc9c");  // 青绿色 - 摄像头接口
    } else if (m_function.contains("AUX")) {
        color = QColor("#f1c40f");  // 黄色 - 辅助接口
    } else if (m_function.contains("DBG")) {
        color = QColor("#8e44ad");  // 深紫色 - 调试接口
    } else if (m_function.contains("MIPI")) {
        color = QColor("#e91e63");  // 粉色 - MIPI接口
    } else if (m_function.contains("VI") || m_function.contains("VO")) {
        color = QColor("#ff5722");  // 深橙色 - 视频接口
    } else if (m_function.contains("SD")) {
        color = QColor("#607d8b");  // 蓝灰色 - SD卡接口
    } else if (m_function.contains("PAD")) {
        color = QColor("#795548");  // 棕色 - PAD接口
    } else {
        color = QColor("#95a5a6");  // 默认灰色
    }

    // 如果引脚被禁用，淡化颜色
    if (!isEnabled()) {
        // 将颜色的透明度降低到40%，营造淡化效果
        color.setAlpha(102);  // 102 = 255 * 0.4
        // 或者可以将颜色调亮一些
        color = color.lighter(150);  // 调亮70%
    }

    // 绘制形状
    painter.setBrush(color);
    painter.setPen(QPen(QColor("#ffffffff"), 2));

    // 如果是高亮状态，根据闪烁状态改变边框颜色
    if (m_isHighlighted && isEnabled()) {
        if (m_blinkState) {
            painter.setPen(QPen(QColor("#000000"), 4)); // 黑色粗边框
        } else {
            painter.setPen(QPen(QColor("#000000"), 2)); // 黑色细边框
        }
    }

    if (m_isSquare) {
        // 绘制方形（QFN封装）
        painter.drawRect(2, 2, width() - 4, height() - 4);
    } else {
        // 绘制圆形（BGA封装）
        painter.drawEllipse(2, 2, width() - 4, height() - 4);
    }

    // 如果引脚被禁用，绘制×标记
    if (!isEnabled()) {
        painter.setPen(QPen(QColor("#e74c3c"), 2));  // 红色×
        int margin = 4;
        // 绘制×的两条线
        painter.drawLine(margin, margin, width() - margin, height() - margin);
        painter.drawLine(width() - margin, margin, margin, height() - margin);
    }

    // 绘制悬停效果
    if (underMouse() && isEnabled()) {
        painter.setBrush(QBrush());
        painter.setPen(QPen(QColor("#34495e"), 3));
        if (m_isSquare) {
            painter.drawRect(1, 1, width() - 2, height() - 2);
        } else {
            painter.drawEllipse(1, 1, width() - 2, height() - 2);
        }
    }
}

void PinWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // 显示上下文菜单
        if (m_contextMenu) {
            m_contextMenu->exec(mapToGlobal(event->pos()));
        }
    }
    QPushButton::mousePressEvent(event);
}

void PinWidget::setupContextMenu()
{
    m_contextMenu = new QMenu(this);
    m_contextMenu->setStyleSheet(
        "QMenu { background:#ffffff; border:1px solid #dee2e6; }"
        "QMenu::item { padding:8px 14px; }"
        "QMenu::item:selected { background:#000000; color:#ffffff; }"
        /* 放大勾选指示器尺寸 */
        "QMenu::indicator { width:20px; height:20px; }"
        /* 未选中时（普通背景）使用空白图标以占位对齐 */
        "QMenu::indicator:unchecked { image:url(:/resources/icons/blank_20.svg); }"
        /* 选中时，根据不同状态使用不同的图标以增强对比度 */
        "QMenu::indicator:checked { image:url(:/resources/icons/check_black.svg); }"
        /* 在选中高亮条上，依然使用白色描边版本保证对比 */
        "QMenu::item:selected:!exclusive QMenu::indicator:checked { image:url(:/resources/icons/check_white.svg); }"
        "QMenu::item:selected:exclusive QMenu::indicator:checked { image:url(:/resources/icons/check_white.svg); }"
    );

    for (const QString& function : m_functions) {
        QAction *action = m_contextMenu->addAction(function);
        action->setData(function);
        action->setCheckable(true);
        action->setChecked(function == m_function);
        connect(action, &QAction::triggered, this, &PinWidget::onFunctionSelected);
    }
}

void PinWidget::onFunctionSelected()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (action) {
        QString selectedFunction = action->data().toString();

        // 更新菜单选中状态
        for (QAction* menuAction : m_contextMenu->actions()) {
            menuAction->setChecked(menuAction == action);
        }

        // 设置新功能
        setFunction(selectedFunction);
    }
}

void PinWidget::updateButtonStyle()
{
    // 强制重绘
    update();
}

void PinWidget::initializePinFunctions()
{
    // 获取该引脚支持的功能列表
    m_functions = m_pinFunction.getSupportedFunctions(m_pinName);

    // 设置默认功能
    m_function = m_pinFunction.getDefaultFunction(m_pinName);
}

void PinWidget::updateTooltip()
{
    // 格式："显示名称 - 实际引脚名称"
    // 例如："A2 - PAD_MIPI_TXM4"
    QString tooltip = QString("%1 - %2").arg(m_displayName, m_pinName);
    setToolTip(tooltip);
}
