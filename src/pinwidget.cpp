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
    // 设置基本属性：为底部标签预留额外高度（不改变圆圈尺寸）
    setMinimumSize(60, 25);
    setMaximumSize(60, 25);

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

    // 绘制形状
    painter.setBrush(color);
    painter.setPen(QPen(QColor("#ffffffff"), 2));

    // 记录是否需要高亮，避免修改圆圈边框笔画（保持白边），高亮改为外部矩形框
    const bool needHighlightBorder = (m_isHighlighted && isEnabled());

    // 圆圈以高度为基准（与未配置相同），文字绘制在圆圈右侧
    const int shapeSize = qMax(4, height() - 4);

    if (m_isSquare) {
        // 绘制方形（QFN封装）：固定为正方形，不随控件加宽变大
        painter.drawRect(2, 2, shapeSize, shapeSize);
    } else {
        // 绘制圆形（BGA封装）：固定为正圆，不随控件加宽变椭圆
        painter.drawEllipse(2, 2, shapeSize, shapeSize);
    }

    // 如果引脚被禁用，绘制×标记（与圆圈尺寸一致，落在圆圈内框）
    if (!isEnabled()) {
        painter.setPen(QPen(QColor("#e74c3c"), 2));  // 红色×
        const int inset = 4; // 留出与白色边框一致的内缩
        QRect shapeRect(2, 2, shapeSize, shapeSize);
        QPoint a(shapeRect.left() + inset, shapeRect.top() + inset);
        QPoint b(shapeRect.right() - inset, shapeRect.bottom() - inset);
        QPoint c(shapeRect.right() - inset, shapeRect.top() + inset);
        QPoint d(shapeRect.left() + inset, shapeRect.bottom() - inset);
        painter.drawLine(a, b);
        painter.drawLine(c, d);
    }

    // 绘制悬停效果：仅绘制矩形边框，避免出现椭圆圈
    if (underMouse() && isEnabled()) {
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(QColor("#34495e"), 2));
        painter.drawRect(1, 1, width() - 2, height() - 2);
    }

    // 已配置时在右侧显示功能名（与选择框高度齐平），避免省略
    if (isConfigured) {
        QFont font = painter.font();
        int ps = font.pointSize();
        if (ps <= 0) ps = 9; // 兜底
        int newPs = ps - 5; // 小号字体
        if (newPs < 4) newPs = 4;
        if (newPs > 9) newPs = 9;
        font.setPointSize(newPs);
        painter.setFont(font);
        painter.setPen(QPen(QColor("#000000")));
        QString text = m_function; // 显示所选功能名（如 VO_D_8）
        // 在圆圈右侧绘制，垂直居中
        const int margin = 4;
        QRect textRect(2 + shapeSize + margin, 2, width() - (2 + shapeSize + margin) - 2, shapeSize);
        painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft | Qt::TextWrapAnywhere, text);
    }

    // 绘制高亮边框（矩形），避免椭圆外圈
    if (needHighlightBorder) {
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(QColor("#000000"), m_blinkState ? 4 : 2));
        painter.drawRect(1, 1, width() - 2, height() - 2);
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
