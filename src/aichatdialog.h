#ifndef AICHATDIALOG_H
#define AICHATDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QSplitter>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QScrollBar>
#include <QTimer>

class AIChatDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AIChatDialog(QWidget *parent = nullptr);
    ~AIChatDialog();

private slots:
    void onSendMessage();
    void onTextChanged();
    void onNetworkReplyFinished();
    void onNetworkReplyReadyRead();
    void onRetryConnection();

private:
    void setupUI();
    void setupStyles();
    void sendMessageToAI(const QString& message);
    void appendUserMessage(const QString& message);
    void appendAIMessage(const QString& message);
    void appendAIMessageWithMarkdown(const QString& markdown);
    bool isMarkdownContent(const QString& content);
    void updateAIResponse();
    void updateAIResponseWithContent(const QString& content);
    void showErrorMessage(const QString& error);
    void setConnectionStatus(bool connected);
    void scrollToBottom();
    
    // UI组件
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_statusLayout;
    QHBoxLayout *m_inputLayout;
    
    QLabel *m_titleLabel;
    QLabel *m_statusLabel;
    QPushButton *m_retryButton;
    
    QTextEdit *m_chatDisplay;
    QLineEdit *m_inputLineEdit;
    QPushButton *m_sendButton;
    QPushButton *m_clearButton;
    
    // 网络相关
    QNetworkAccessManager *m_networkManager;
    QNetworkReply *m_currentReply;
    
    // AI API配置
    QString m_apiKey;
    QString m_baseUrl;
    QString m_model;
    
    // 状态管理
    bool m_isConnected;
    QString m_currentAIResponse;
    
    // UI状态
    bool m_isWaitingForResponse;
};

#endif // AICHATDIALOG_H