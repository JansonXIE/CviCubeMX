#ifndef PINWIDGET_H
#define PINWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QMenu>
#include <QAction>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>

class PinWidget : public QPushButton
{
    Q_OBJECT

public:
    explicit PinWidget(const QString& pinName, bool isSquare = true, QWidget *parent = nullptr);
    
    void setFunction(const QString& function);
    QString getFunction() const;
    QString getPinName() const;

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
    
    QString m_pinName;
    QString m_function;
    bool m_isSquare;
    QMenu *m_contextMenu;
    QStringList m_functions;
};

#endif // PINWIDGET_H
