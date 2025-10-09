#include "aichatdialog.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QMessageBox>
#include <QApplication>
#include <QClipboard>
#include <QTextCursor>
#include <QScrollBar>
#include <QDebug>
#include <QStyle>
#include <QTimer>
#include <QTextDocument>
#include <QRegularExpression>

AIChatDialog::AIChatDialog(QWidget *parent)
    : QDialog(parent)
    , m_mainLayout(nullptr)
    , m_statusLayout(nullptr)
    , m_inputLayout(nullptr)
    , m_titleLabel(nullptr)
    , m_statusLabel(nullptr)
    , m_retryButton(nullptr)
    , m_chatDisplay(nullptr)
    , m_inputLineEdit(nullptr)
    , m_sendButton(nullptr)
    , m_clearButton(nullptr)
    , m_networkManager(nullptr)
    , m_currentReply(nullptr)
    , m_isConnected(false)
    , m_isWaitingForResponse(false)
{
    // 初始化AI API配置
    m_apiKey = "QWGNmtHwAu1WRJkjmHD4FSpIMgDCS1puYeMq5gglfhpIXBGitoD1YPdygcKwEAQ-PKQZETbIVRUcuiiY1D8JaQ";
    m_baseUrl = "https://www.sophnet.com/api/open-apis";
    m_model = "QwQ-32B";
    
    setupUI();
    setupStyles();
    
    // 初始化网络管理器
    m_networkManager = new QNetworkAccessManager(this);
    
    // 设置初始连接状态
    setConnectionStatus(true);
}

AIChatDialog::~AIChatDialog()
{
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
    }
}

void AIChatDialog::setupUI()
{
    setWindowTitle("AI 智能问答助手");
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    resize(800, 600);
    
    // 主布局
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(15, 15, 15, 15);
    
    // 标题和状态栏
    m_statusLayout = new QHBoxLayout();
    m_titleLabel = new QLabel("SophNet 智能助手");
    m_titleLabel->setObjectName("titleLabel");
    
    m_statusLabel = new QLabel("已连接");
    m_statusLabel->setObjectName("statusLabel");
    
    m_retryButton = new QPushButton("重新连接");
    m_retryButton->setObjectName("retryButton");
    m_retryButton->setVisible(false);
    connect(m_retryButton, &QPushButton::clicked, this, &AIChatDialog::onRetryConnection);
    
    m_statusLayout->addWidget(m_titleLabel);
    m_statusLayout->addStretch();
    m_statusLayout->addWidget(m_statusLabel);
    m_statusLayout->addWidget(m_retryButton);
    
    // 聊天显示区域
    m_chatDisplay = new QTextEdit();
    m_chatDisplay->setObjectName("chatDisplay");
    m_chatDisplay->setReadOnly(true);
    m_chatDisplay->setMinimumHeight(400);
    
    // 输入区域
    m_inputLayout = new QHBoxLayout();
    m_inputLineEdit = new QTextEdit();
    m_inputLineEdit->setObjectName("inputLineEdit");
    m_inputLineEdit->setPlaceholderText("请输入您的问题... (Enter发送 | Shift+Enter换行)");
    m_inputLineEdit->setMinimumHeight(40);
    m_inputLineEdit->setMaximumHeight(120); // 限制最大高度
    m_inputLineEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_inputLineEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_inputLineEdit->setLineWrapMode(QTextEdit::WidgetWidth);
    m_inputLineEdit->setAcceptRichText(false); // 只接受纯文本
    m_inputLineEdit->installEventFilter(this); // 安装事件过滤器
    
    m_sendButton = new QPushButton("发送");
    m_sendButton->setObjectName("sendButton");
    m_sendButton->setMinimumHeight(40);
    m_sendButton->setMinimumWidth(80);
    
    m_clearButton = new QPushButton("清空");
    m_clearButton->setObjectName("clearButton");
    m_clearButton->setMinimumHeight(40);
    m_clearButton->setMinimumWidth(80);
    
    m_inputLayout->addWidget(m_inputLineEdit);
    m_inputLayout->addWidget(m_sendButton);
    m_inputLayout->addWidget(m_clearButton);
    
    // 添加到主布局
    m_mainLayout->addLayout(m_statusLayout);
    m_mainLayout->addWidget(m_chatDisplay);
    m_mainLayout->addLayout(m_inputLayout);
    
    // 连接信号槽
    connect(m_sendButton, &QPushButton::clicked, this, &AIChatDialog::onSendMessage);
    connect(m_inputLineEdit, &QTextEdit::textChanged, this, &AIChatDialog::onTextChanged);
    connect(m_clearButton, &QPushButton::clicked, m_chatDisplay, &QTextEdit::clear);
    
    // 初始状态
    m_sendButton->setEnabled(false);
    
    // 添加欢迎消息
    // appendAIMessage("您好！我是 SophNet 智能助手，很高兴为您服务。请随时向我提问，我会尽力帮助您解答问题。");
}

void AIChatDialog::setupStyles()
{
    QString styleSheet = R"(
        QDialog {
            background-color: #f5f5f5;
        }
        
        #titleLabel {
            font-size: 18px;
            font-weight: bold;
            color: #2c3e50;
            padding: 5px;
        }
        
        #statusLabel {
            font-size: 12px;
            padding: 5px 10px;
            border-radius: 10px;
            background-color: #27ae60;
            color: white;
        }
        
        #statusLabel[status="disconnected"] {
            background-color: #e74c3c;
        }
        
        #chatDisplay {
            background-color: white;
            border: 1px solid #bdc3c7;
            border-radius: 8px;
            padding: 10px;
            font-family: "Microsoft YaHei", Arial, sans-serif;
            font-size: 14px;
            line-height: 1.4;
        }
        
        #inputLineEdit {
            background-color: white;
            border: 2px solid #bdc3c7;
            border-radius: 8px;
            padding: 8px 12px;
            font-size: 14px;
            font-family: "Microsoft YaHei", Arial, sans-serif;
        }
        
        #inputLineEdit:focus {
            border-color: #3498db;
        }
        
        #sendButton {
            background-color: #3498db;
            color: white;
            border: none;
            border-radius: 8px;
            font-size: 14px;
            font-weight: bold;
        }
        
        #sendButton:hover {
            background-color: #2980b9;
        }
        
        #sendButton:disabled {
            background-color: #bdc3c7;
        }
        
        #clearButton {
            background-color: #95a5a6;
            color: white;
            border: none;
            border-radius: 8px;
            font-size: 14px;
        }
        
        #clearButton:hover {
            background-color: #7f8c8d;
        }
        
        #retryButton {
            background-color: #e67e22;
            color: white;
            border: none;
            border-radius: 8px;
            font-size: 12px;
            padding: 5px 10px;
        }
        
        #retryButton:hover {
            background-color: #d35400;
        }
        
        #testButton {
            background-color: #9b59b6;
            color: white;
            border: none;
            border-radius: 8px;
            font-size: 14px;
        }
        
        #testButton:hover {
            background-color: #8e44ad;
        }
    )";
    
    setStyleSheet(styleSheet);
}

void AIChatDialog::onSendMessage()
{
    QString message = m_inputLineEdit->toPlainText().trimmed();
    if (message.isEmpty() || m_isWaitingForResponse) {
        return;
    }
    
    // 显示用户消息
    appendUserMessage(message);
    
    // 清空输入框
    m_inputLineEdit->clear();
    
    // 重置输入框高度
    m_inputLineEdit->setMinimumHeight(40);
    
    // 强制立即处理UI事件,确保用户消息立即显示
    QApplication::processEvents();
    
    // 发送到AI
    sendMessageToAI(message);
}

void AIChatDialog::onTextChanged()
{
    bool hasText = !m_inputLineEdit->toPlainText().trimmed().isEmpty();
    m_sendButton->setEnabled(hasText && !m_isWaitingForResponse);
    
    // 动态调整输入框高度
    QTextDocument *doc = m_inputLineEdit->document();
    int docHeight = doc->size().height() + m_inputLineEdit->contentsMargins().top() + m_inputLineEdit->contentsMargins().bottom();
    docHeight = qBound(40, docHeight + 10, 120); // 限制在40-120像素之间
    m_inputLineEdit->setMinimumHeight(docHeight);
}

void AIChatDialog::sendMessageToAI(const QString& message)
{
    if (m_isWaitingForResponse) {
        return;
    }
    
    m_isWaitingForResponse = true;
    m_sendButton->setEnabled(false);
    
    // 创建请求
    QNetworkRequest request;
    QString fullUrl = m_baseUrl + "/chat/completions";
    request.setUrl(QUrl(fullUrl));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_apiKey).toUtf8());
    request.setRawHeader("Accept", "text/plain"); // 流式响应使用text/plain
    request.setRawHeader("User-Agent", "openai-python/1.0.0"); // 模拟OpenAI客户端
    
    // 添加调试信息
    qDebug() << "发送请求到:" << fullUrl;
    qDebug() << "API Key前缀:" << m_apiKey.left(20) + "...";
    
    // 创建请求体 - 完全按照Python脚本的格式
    QJsonObject json;
    json["model"] = m_model;
    json["stream"] = true; // 使用流式请求，与Python脚本保持一致
    
    QJsonArray messages;
    // System message
    QJsonObject systemMessage;
    systemMessage["role"] = "system";
    systemMessage["content"] = "你是SophNet智能助手";
    messages.append(systemMessage);
    
    // User message
    QJsonObject userMessage;
    userMessage["role"] = "user";
    userMessage["content"] = message;
    messages.append(userMessage);
    
    json["messages"] = messages;
    
    QJsonDocument doc(json);
    QByteArray requestData = doc.toJson(QJsonDocument::Compact);
    
    qDebug() << "请求数据:" << requestData;
    
    // 发送请求
    m_currentReply = m_networkManager->post(request, requestData);
    // 连接流式响应信号
    connect(m_currentReply, &QNetworkReply::readyRead, this, &AIChatDialog::onNetworkReplyReadyRead);
    connect(m_currentReply, &QNetworkReply::finished, this, &AIChatDialog::onNetworkReplyFinished);
    
    // 显示正在思考的消息
    m_currentAIResponse.clear();
    appendAIMessage("正在思考中...");
    
    // 设置超时 (600秒)
    QTimer::singleShot(600000, this, [this]() {
        if (m_currentReply && m_currentReply->isRunning()) {
            qDebug() << "请求超时，正在取消...";
            m_currentReply->abort();
            showErrorMessage("请求超时（60秒），请检查网络连接后重试");
        }
    });
}

void AIChatDialog::onNetworkReplyReadyRead()
{
    if (!m_currentReply) {
        return;
    }
    
    QByteArray data = m_currentReply->readAll();
    QString response = QString::fromUtf8(data);
    
    qDebug() << "收到流式数据块，长度:" << data.size();
    qDebug() << "数据内容:" << response.left(200) << "...";
    
    // 处理SSE格式的流式响应
    QStringList lines = response.split('\n');
    for (const QString& line : lines) {
        if (line.startsWith("data: ")) {
            QString jsonStr = line.mid(6); // 移除 "data: " 前缀
            jsonStr = jsonStr.trimmed();
            
            qDebug() << "处理JSON字符串:" << jsonStr;
            
            if (jsonStr == "[DONE]") {
                qDebug() << "流式响应结束";
                break;
            }
            
            if (jsonStr.isEmpty()) {
                continue;
            }
            
            QJsonParseError error;
            QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8(), &error);
            if (error.error == QJsonParseError::NoError && doc.isObject()) {
                QJsonObject obj = doc.object();
                qDebug() << "成功解析JSON对象";
                
                if (obj.contains("choices")) {
                    QJsonArray choices = obj["choices"].toArray();
                    if (!choices.isEmpty()) {
                        QJsonObject choice = choices[0].toObject();
                        if (choice.contains("delta")) {
                            QJsonObject delta = choice["delta"].toObject();
                            if (delta.contains("content")) {
                                QString content = delta["content"].toString();
                                qDebug() << "提取到内容片段:" << content;
                                
                                if (!content.isEmpty()) {
                                    m_currentAIResponse += content;
                                    qDebug() << "累积响应内容长度:" << m_currentAIResponse.length();
                                    qDebug() << "当前累积内容:" << m_currentAIResponse;
                                    
                                    // 不进行实时更新，只累积内容
                                } else {
                                    qDebug() << "内容片段为空，跳过";
                                }
                            } else {
                                qDebug() << "Delta对象不包含content字段";
                            }
                        } else {
                            qDebug() << "Choice对象不包含delta字段";
                        }
                    } else {
                        qDebug() << "Choices数组为空";
                    }
                } else {
                    qDebug() << "JSON对象不包含choices字段";
                }
            } else {
                qDebug() << "JSON解析错误:" << error.errorString() << "内容:" << jsonStr;
            }
        }
    }
}

void AIChatDialog::onNetworkReplyFinished()
{
    if (!m_currentReply) {
        return;
    }
    
    // 获取HTTP状态码
    int httpStatus = m_currentReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    
    qDebug() << "流式网络请求完成 - HTTP状态码:" << httpStatus;
    qDebug() << "网络错误:" << m_currentReply->error() << m_currentReply->errorString();
    
    if (m_currentReply->error() != QNetworkReply::NoError || httpStatus != 200) {
        // 处理错误
        QByteArray responseData = m_currentReply->readAll();
        QString errorMessage;
        
        if (httpStatus == 400) {
            errorMessage = "请求格式错误 (HTTP 400)";
            if (!responseData.isEmpty()) {
                QString responseStr = QString::fromUtf8(responseData);
                qDebug() << "HTTP 400 错误响应:" << responseStr;
                
                QJsonParseError parseError;
                QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);
                if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
                    QJsonObject obj = doc.object();
                    if (obj.contains("message")) {
                        errorMessage = QString("API错误: %1").arg(obj["message"].toString());
                    }
                } else {
                    errorMessage += QString("\n原始响应: %1").arg(responseStr);
                }
            }
        } else {
            errorMessage = QString("网络错误: %1 (HTTP %2)")
                          .arg(m_currentReply->errorString()).arg(httpStatus);
            if (!responseData.isEmpty()) {
                QString responseStr = QString::fromUtf8(responseData.left(500));
                errorMessage += QString("\n响应: %1").arg(responseStr);
            }
        }
        
        qDebug() << "流式请求错误:" << errorMessage;
        showErrorMessage(errorMessage);
        setConnectionStatus(false);
    } else {
        // 流式请求成功完成
        setConnectionStatus(true);
        qDebug() << "流式响应完成，累积内容长度:" << m_currentAIResponse.length();
        
        // 显示最终的完整响应内容
        if (!m_currentAIResponse.isEmpty()) {
            qDebug() << "执行最终的强制更新，内容:" << m_currentAIResponse;
            
            // 使用QTextCursor方式清除"正在思考中..."消息
            QString currentText = m_chatDisplay->toPlainText();
            int lastThinkingIndex = currentText.lastIndexOf("正在思考中...");
            
            if (lastThinkingIndex != -1) {
                qDebug() << "找到'正在思考中...'位置:" << lastThinkingIndex;
                
                // 使用QTextCursor定位并删除"正在思考中..."所在的块
                QTextCursor cursor = m_chatDisplay->textCursor();
                
                // 从头开始查找"正在思考中..."
                cursor.movePosition(QTextCursor::Start);
                QTextDocument* doc = m_chatDisplay->document();
                cursor = doc->find("正在思考中...", cursor);
                
                if (!cursor.isNull()) {
                    qDebug() << "通过QTextCursor找到'正在思考中...'";
                    
                    // 选择整个块（段落）
                    cursor.movePosition(QTextCursor::StartOfBlock);
                    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
                    
                    // 删除选中的内容
                    cursor.removeSelectedText();
                    
                    // 删除空行
                    cursor.deletePreviousChar();
                    
                    qDebug() << "已删除'正在思考中...'消息";
                }
            }
            
            // 添加最终的完整AI响应
            appendAIMessage(m_currentAIResponse);
        } else {
            showErrorMessage("没有收到AI响应内容");
        }
    }
    
    m_currentReply->deleteLater();
    m_currentReply = nullptr;
    m_isWaitingForResponse = false;
    onTextChanged(); // 更新发送按钮状态
}

void AIChatDialog::onRetryConnection()
{
    setConnectionStatus(true);
    m_retryButton->setVisible(false);
}

void AIChatDialog::appendUserMessage(const QString& message)
{
    QTextCursor cursor = m_chatDisplay->textCursor();
    cursor.movePosition(QTextCursor::End);
    
    // 创建右对齐的块格式
    QTextBlockFormat blockFormat;
    blockFormat.setAlignment(Qt::AlignRight);
    cursor.insertBlock(blockFormat);
    
    // 插入用户消息的HTML
    QString html = QString(
        "<span style=\"background-color: #ecf0f1; color: #2c3e50; padding: 10px 15px; "
        "border-radius: 18px; display: inline-block; max-width: 65%; word-wrap: break-word; "
        "font-size: 14px; line-height: 1.5; box-shadow: 0 2px 5px rgba(52, 152, 219, 0.3);\">"
        "%1</span>"
    ).arg(message.toHtmlEscaped());
    
    cursor.insertHtml(html);
    m_chatDisplay->setTextCursor(cursor);
    scrollToBottom();
}

void AIChatDialog::appendAIMessage(const QString& message)
{
    // 检测是否为Markdown内容
    if (isMarkdownContent(message)) {
        // 使用Markdown渲染
        appendAIMessageWithMarkdown(message);
    } else {
        // 使用普通文本渲染
        QTextCursor cursor = m_chatDisplay->textCursor();
        cursor.movePosition(QTextCursor::End);
        
        // 创建左对齐的块格式
        QTextBlockFormat blockFormat;
        blockFormat.setAlignment(Qt::AlignLeft);
        cursor.insertBlock(blockFormat);
        
        // 插入AI消息的HTML
        QString html = QString(
            "<span style=\"background-color: #ffffffff; color: #2c3e50; padding: 10px 15px; "
            "border-radius: 18px; display: inline-block; max-width: 65%; word-wrap: break-word; "
            "font-size: 14px; line-height: 1.5; box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);\">"
            "%1</span>"
        ).arg(message.toHtmlEscaped());
        
        cursor.insertHtml(html);
        m_chatDisplay->setTextCursor(cursor);
    }
    scrollToBottom();
}

void AIChatDialog::updateAIResponse()
{
    qDebug() << "调用updateAIResponse()，当前累积内容长度:" << m_currentAIResponse.length();
    qDebug() << "当前累积内容:" << m_currentAIResponse;
    
    // 移除最后一个"正在思考中..."消息并添加实际响应
    QString currentHtml = m_chatDisplay->toHtml();
    int lastThinkingIndex = currentHtml.lastIndexOf("正在思考中...");
    
    qDebug() << "搜索'正在思考中...'的位置:" << lastThinkingIndex;
    
    if (lastThinkingIndex != -1) {
        // 找到"正在思考中..."的开始位置
        int divStart = currentHtml.lastIndexOf("<div", lastThinkingIndex);
        int divEnd = currentHtml.indexOf("</div>", lastThinkingIndex) + 6;
        
        qDebug() << "找到div范围: start=" << divStart << ", end=" << divEnd;
        
        if (divStart != -1 && divEnd != -1) {
            // 替换整个消息
            QString beforeDiv = currentHtml.left(divStart);
            QString afterDiv = currentHtml.mid(divEnd);
            
            // 创建新的AI响应HTML
            QString aiResponseHtml = QString(
                "<div style=\"margin: 10px 0; text-align: left;\">"
                "<div style=\"background-color: #ecf0f1; color: #2c3e50; padding: 10px 15px; "
                "border-radius: 18px; display: inline-block; max-width: 65%; word-wrap: break-word; "
                "font-size: 14px; line-height: 1.5; box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);\">"
                "%1</div></div>"
            ).arg(m_currentAIResponse.toHtmlEscaped());
            
            QString newHtml = beforeDiv + aiResponseHtml + afterDiv;
            qDebug() << "设置新的HTML内容，AI响应长度:" << aiResponseHtml.length();
            m_chatDisplay->setHtml(newHtml);
        } else {
            qDebug() << "无法找到有效的div范围，直接添加新消息";
            appendAIMessage(m_currentAIResponse);
        }
    } else {
        // 如果没找到"正在思考中..."，直接添加新消息
        qDebug() << "没有找到'正在思考中...'，直接添加新消息";
        appendAIMessage(m_currentAIResponse);
    }
    
    scrollToBottom();
    qDebug() << "完成updateAIResponse()";
}

void AIChatDialog::updateAIResponseWithContent(const QString& content)
{
    // 移除最后一个"正在思考中..."消息并添加AI响应
    QString currentHtml = m_chatDisplay->toHtml();
    int lastThinkingIndex = currentHtml.lastIndexOf("正在思考中...");
    
    if (lastThinkingIndex != -1) {
        // 找到"正在思考中..."的开始位置
        int divStart = currentHtml.lastIndexOf("<div", lastThinkingIndex);
        int divEnd = currentHtml.indexOf("</div>", lastThinkingIndex) + 6;
        
        if (divStart != -1 && divEnd != -1) {
            // 替换整个消息
            QString beforeDiv = currentHtml.left(divStart);
            QString afterDiv = currentHtml.mid(divEnd);
            
            // 创建新的AI响应HTML
            QString aiResponseHtml = QString(
                "<div style=\"margin: 10px 0; text-align: left;\">"
                "<div style=\"background-color: #ecf0f1; color: #2c3e50; padding: 10px 15px; "
                "border-radius: 18px; display: inline-block; max-width: 65%; word-wrap: break-word; "
                "font-size: 14px; line-height: 1.5; box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);\">"
                "%1</div></div>"
            ).arg(content.toHtmlEscaped());
            
            QString newHtml = beforeDiv + aiResponseHtml + afterDiv;
            m_chatDisplay->setHtml(newHtml);
        }
    } else {
        // 如果没找到"正在思考中..."，直接添加新消息
        appendAIMessage(content);
    }
    
    scrollToBottom();
}

void AIChatDialog::showErrorMessage(const QString& error)
{
    QString html = QString(
        "<div style=\"margin: 10px 0; text-align: center;\">"
        "<span style=\"background-color: #e74c3c; color: white; padding: 8px 12px; "
        "border-radius: 18px; display: inline-block; font-style: italic;\">"
        "错误: %1</span></div>"
    ).arg(error.toHtmlEscaped());
    
    m_chatDisplay->append(html);
    scrollToBottom();
}

void AIChatDialog::setConnectionStatus(bool connected)
{
    m_isConnected = connected;
    if (connected) {
        m_statusLabel->setText("已连接");
        m_statusLabel->setProperty("status", "connected");
        m_retryButton->setVisible(false);
    } else {
        m_statusLabel->setText("连接失败");
        m_statusLabel->setProperty("status", "disconnected");
        m_retryButton->setVisible(true);
    }
    
    // 重新应用样式
    m_statusLabel->style()->unpolish(m_statusLabel);
    m_statusLabel->style()->polish(m_statusLabel);
    m_statusLabel->update();
}

void AIChatDialog::scrollToBottom()
{
    QScrollBar* scrollBar = m_chatDisplay->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void AIChatDialog::appendAIMessageWithMarkdown(const QString& markdown)
{
    // 创建一个临时的QTextDocument来处理Markdown
    QTextDocument tempDoc;
    tempDoc.setMarkdown(markdown);
    QString htmlContent = tempDoc.toHtml();
    
    // 移除HTML文档的头部和尾部，只保留body内容
    int bodyStart = htmlContent.indexOf("<body");
    int bodyContentStart = htmlContent.indexOf(">", bodyStart) + 1;
    int bodyEnd = htmlContent.lastIndexOf("</body>");
    
    QString bodyContent;
    if (bodyStart != -1 && bodyEnd != -1 && bodyContentStart < bodyEnd) {
        bodyContent = htmlContent.mid(bodyContentStart, bodyEnd - bodyContentStart).trimmed();
    } else {
        // 如果无法提取body内容，使用原始内容
        bodyContent = markdown.toHtmlEscaped();
    }
    
    QTextCursor cursor = m_chatDisplay->textCursor();
    cursor.movePosition(QTextCursor::End);
    
    // 创建左对齐的块格式
    QTextBlockFormat blockFormat;
    blockFormat.setAlignment(Qt::AlignLeft);
    cursor.insertBlock(blockFormat);
    
    // 将Markdown渲染的内容包装在聊天气泡样式中
    QString html = QString(
        "<span style=\"background-color: #ecf0f1; color: #2c3e50; padding: 12px 16px; "
        "border-radius: 12px; display: inline-block; max-width: 75%; word-wrap: break-word; "
        "font-family: 'Microsoft YaHei', Arial, sans-serif; line-height: 1.5; "
        "box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);\">"
        "%1</span>"
    ).arg(bodyContent);
    
    cursor.insertHtml(html);
    m_chatDisplay->setTextCursor(cursor);
}

bool AIChatDialog::isMarkdownContent(const QString& content)
{
    // 检测常见的Markdown标记
    if (content.contains(QRegularExpression("^#{1,6}\\s+")) ||          // 标题 # ## ### 等
        content.contains(QRegularExpression("\\*\\*.*\\*\\*")) ||         // 粗体 **text**
        content.contains(QRegularExpression("\\*.*\\*")) ||               // 斜体 *text*
        content.contains(QRegularExpression("`.*`")) ||                  // 行内代码 `code`
        content.contains(QRegularExpression("```[\\s\\S]*```")) ||        // 代码块 ```code```
        content.contains(QRegularExpression("^\\s*[-*+]\\s+")) ||        // 无序列表 - * +
        content.contains(QRegularExpression("^\\s*\\d+\\.\\s+")) ||      // 有序列表 1. 2.
        content.contains(QRegularExpression("^>\\s+")) ||                // 引用 >
        content.contains(QRegularExpression("\\[.*\\]\\(.*\\)")) ||      // 链接 [text](url)
        content.contains("---") ||                           // 分割线
        content.contains("|")) {                             // 表格分隔符
        return true;
    }
    return false;
}

bool AIChatDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_inputLineEdit && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        
        // 处理 Enter 键
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            // Shift+Enter: 插入换行符
            if (keyEvent->modifiers() & Qt::ShiftModifier) {
                m_inputLineEdit->insertPlainText("\n");
                return true; // 事件已处理
            }
            // 单独 Enter: 发送消息
            else {
                onSendMessage();
                return true; // 事件已处理
            }
        }
    }
    
    // 调用基类的事件过滤器
    return QDialog::eventFilter(obj, event);
}
