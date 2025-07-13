#include "pinwidget.h"
#include <QApplication>

PinWidget::PinWidget(const QString& pinName, bool isSquare, QWidget *parent)
    : QPushButton(parent)
    , m_pinName(pinName)
    , m_function("GPIO")
    , m_isSquare(isSquare)
    , m_contextMenu(nullptr)
{
    // 设置基本属性
    setMinimumSize(25, 25);
    setMaximumSize(25, 25);
    setToolTip(QString("%1 - %2").arg(m_pinName, m_function));
    
    // 初始化可用功能
    m_functions << "GPIO" << "ADC" << "I2C" << "UART" << "SPI" << "PWM" << "Timer";
    
    // 设置上下文菜单
    setupContextMenu();
    
    // 更新按钮样式
    updateButtonStyle();
}

void PinWidget::setFunction(const QString& function)
{
    if (m_functions.contains(function)) {
        m_function = function;
        updateButtonStyle();
        setToolTip(QString("%1 - %2").arg(m_pinName, m_function));
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

void PinWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 获取颜色
    QColor color;
    if (m_function == "GPIO") {
        color = QColor("#95a5a6");
    } else if (m_function == "ADC") {
        color = QColor("#e74c3c");
    } else if (m_function == "I2C") {
        color = QColor("#3498db");
    } else if (m_function == "UART") {
        color = QColor("#2ecc71");
    } else if (m_function == "SPI") {
        color = QColor("#f39c12");
    } else if (m_function == "PWM") {
        color = QColor("#9b59b6");
    } else if (m_function == "Timer") {
        color = QColor("#e67e22");
    } else {
        color = QColor("#95a5a6");
    }
    
    // 绘制形状
    painter.setBrush(color);
    painter.setPen(QPen(QColor("#2c3e50"), 2));
    
    if (m_isSquare) {
        // 绘制方形（QFN封装）
        painter.drawRect(2, 2, width() - 4, height() - 4);
    } else {
        // 绘制圆形（BGA封装）
        painter.drawEllipse(2, 2, width() - 4, height() - 4);
    }
    
    // 绘制悬停效果
    if (underMouse()) {
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
