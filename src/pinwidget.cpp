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
    , m_userConfigured(false)
{
    // 设置基本属性：高度固定，宽度可以扩展以容纳文本
    setMinimumSize(25, 25);
    setMaximumHeight(25);
    setMaximumWidth(200);  // 设置最大宽度，避免文本太长时过度扩展

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
    if (m_functions.contains(function) || function.compare("reset_state", Qt::CaseInsensitive) == 0) {
        m_function = function;
        // 用户通过菜单选择，标记为已配置
        m_userConfigured = true;
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
    if (!m_functions.contains("reset_state")) {
        m_functions.append("reset_state");
    }

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

    // 两态着色：仅当被用户配置过且功能不是reset_state 时显示为绿色
    const bool isConfigured = (m_userConfigured && !(m_function.compare("reset_state", Qt::CaseInsensitive) == 0));

    QColor color = isConfigured ? QColor("#2ecc71") : QColor("#95a5a6");

    // 如果引脚被禁用，淡化颜色
    if (!isEnabled()) {
        // 将颜色的透明度降低到40%，营造淡化效果
        color.setAlpha(102);  // 102 = 255 * 0.4
        // 或者可以将颜色调亮一些
        color = color.lighter(150);  // 调亮70%
    }

    // 绘制形状（固定在左侧25x25区域）
    painter.setBrush(color);
    painter.setPen(QPen(QColor("#ffffffff"), 2));

    if (m_isHighlighted && isEnabled()) {
        if (m_blinkState) {
            painter.setPen(QPen(QColor("#000000"), 4)); // 黑色粗边框
        } else {
            painter.setPen(QPen(QColor("#000000"), 2)); // 黑色细边框
        }
    }

    if (m_isSquare) {
        // 绘制方形（QFN封装）：固定为正方形，不随控件加宽变大
        painter.drawRect(2, 2, 21, 21);
    } else {
        // 绘制圆形（BGA封装）：固定为正圆，不随控件加宽变椭圆
        painter.drawEllipse(2, 2, 21, 21);
    }

    // 如果引脚被禁用，绘制×标记（与圆圈尺寸一致，落在圆圈内框）
    if (!isEnabled()) {
        painter.setPen(QPen(QColor("#e74c3c"), 2));  // 红色×
        int margin = 4;
        // 绘制×的两条线
        painter.drawLine(margin, margin, 21 + 2, 21 + 2);
        painter.drawLine(21 + 2, margin, margin, 21 + 2);
    }

    // 绘制悬停效果
    if (underMouse() && isEnabled()) {
        painter.setBrush(QBrush());
        painter.setPen(QPen(QColor("#34495e"), 3));
        if (m_isSquare) {
            painter.drawRect(1, 1, 23, 23);
        } else {
            painter.drawEllipse(1, 1, 23, 23);
        }
    }

    // 如果引脚已配置，在右侧绘制功能名称
    if (isConfigured && isEnabled()) {
        painter.setPen(QColor("#2c3e50"));  // 深色文字
        QFont font = painter.font();
        font.setPointSize(8);
        font.setBold(true);
        painter.setFont(font);

        // 计算文本绘制区域（在圆圈/方框右侧）
        QRect textRect(28, 0, width() - 30, height());
        painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, m_function);
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

    // 如果列表中没有 reset_state，追加一个“复位状态(reset_state)”选项，便于显式取消 PINMUX 生成
    if (!m_functions.contains("reset_state")) {
        m_contextMenu->addSeparator();
        QAction *resetAct = m_contextMenu->addAction("reset_state");
        resetAct->setData("reset_state");
        resetAct->setCheckable(true);
        resetAct->setChecked(m_function.compare("reset_state", Qt::CaseInsensitive) == 0);
        connect(resetAct, &QAction::triggered, this, &PinWidget::onFunctionSelected);
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
    // 根据是否配置了功能来调整控件宽度
    const bool isConfigured = (m_userConfigured && !(m_function.compare("reset_state", Qt::CaseInsensitive) == 0));
    
    if (isConfigured && isEnabled()) {
        // 计算文本所需宽度
        QFont font;
        font.setPointSize(8);
        font.setBold(true);
        QFontMetrics fm(font);
        int textWidth = fm.horizontalAdvance(m_function);
        
        // 控件宽度 = 圆圈/方框宽度(25) + 间距(5) + 文本宽度 + 右边距(5)
        int totalWidth = 25 + 5 + textWidth + 5;
        setMinimumWidth(totalWidth);
    } else {
        // 未配置时，恢复为正方形
        setMinimumWidth(25);
    }
    
    // 强制重绘
    update();
}

void PinWidget::initializePinFunctions()
{
    // 获取该引脚支持的功能列表
    m_functions = m_pinFunction.getSupportedFunctions(m_pinName);
    if (!m_functions.contains("reset_state")) {
        m_functions.append("reset_state");
    }

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
