use futures_util::StreamExt;
use reqwest::Client;
use serde::{Deserialize, Serialize};
use std::path::PathBuf;
use tauri::{AppHandle, Emitter, Manager};
use uuid::Uuid;

/// AI API 配置
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct AiApiConfig {
    pub api_key: String,
    pub base_url: String,
    pub model: String,
}

impl Default for AiApiConfig {
    fn default() -> Self {
        // 敏感信息从编译时环境变量读取，避免硬编码到源码中被泄露。
        // GitHub Actions 构建 release 时通过 secrets 注入 AI_API_KEY / AI_BASE_URL / AI_MODEL。
        // 本地开发可在 src-tauri/.env 或 shell 中设置这些变量（参见 .env.example）。
        Self {
            api_key: option_env!("AI_API_KEY").unwrap_or("").to_string(),
            base_url: option_env!("AI_BASE_URL")
                .unwrap_or("https://www.sophnet.com/api/open-apis/v1")
                .to_string(),
            model: option_env!("AI_MODEL")
                .unwrap_or("DeepSeek-V4-Pro:2Hz6HohcSFaiFG98gqzSwW")
                .to_string(),
        }
    }
}

/// SSE chunk解析结果
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct SseChunk {
    pub content: String,
}

/// AI chunk 推送 payload (通过 Tauri Event System发送给前端)
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct AiChunkPayload {
    pub content: String,
    pub done: bool,
}

/// 聊天消息（与前端 ChatMessage 保持字段一致）
#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct ChatMessage {
 pub id: String,
 pub role: String,
 pub content: String,
 pub timestamp: u64,
 pub is_markdown: bool,
}

/// 聊天会话完整数据
#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct ChatSession {
 pub id: String,
 pub title: String,
 pub messages: Vec<ChatMessage>,
 pub created_at: String,
 pub updated_at: String,
}

/// 聊天会话列表元数据
#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct ChatSessionMeta {
 pub id: String,
 pub title: String,
 pub message_count: usize,
 pub created_at: String,
 pub updated_at: String,
}

impl From<&ChatSession> for ChatSessionMeta {
 fn from(session: &ChatSession) -> Self {
 Self {
 id: session.id.clone(),
 title: session.title.clone(),
 message_count: session.messages.len(),
 created_at: session.created_at.clone(),
 updated_at: session.updated_at.clone(),
 }
 }
}

/// SSE 行解析:解析 "data: ..." 行
/// - data: {"choices":[{"delta":{"content":"..."}}]} → SseChunk { content }
/// - data: [DONE] → None (流结束)
/// - 非 "data: " 开头 → None (跳过)
pub fn parse_sse_line(line: &str) -> Option<SseChunk> {
    if !line.starts_with("data: ") {
        return None;
    }

    let data = line[6..].trim();

    if data == "[DONE]" {
        return None;
    }

    if data.is_empty() {
        return None;
    }

    //解析 JSON: {"choices":[{"delta":{"content":"..."}}]}
    if let Ok(parsed) = serde_json::from_str::<serde_json::Value>(data) {
        if let Some(choices) = parsed.get("choices").and_then(|c| c.as_array()) {
            if let Some(first_choice) = choices.first() {
                if let Some(delta) = first_choice.get("delta") {
                    let content = delta
                        .get("content")
                        .and_then(|c| c.as_str())
                        .unwrap_or("")
                        .to_string();
                    return Some(SseChunk { content });
                }
            }
        }
    }

    None
}

/// 配置文件路径
fn config_file_path(app: &AppHandle) -> Result<PathBuf, String> {
    let app_dir = app
        .path()
        .app_data_dir()
        .map_err(|e| format!("获取应用数据目录失败: {}", e))?;
    std::fs::create_dir_all(&app_dir).map_err(|e| format!("创建应用数据目录失败: {}", e))?;
    Ok(app_dir.join("ai_config.json"))
}

/// 聊天会话目录路径
fn sessions_dir(app: &AppHandle) -> Result<PathBuf, String> {
 let app_dir = app
 .path()
 .app_data_dir()
 .map_err(|e| format!("获取应用数据目录失败: {}", e))?;
 let sessions_dir = app_dir.join("chat_sessions");
 std::fs::create_dir_all(&sessions_dir)
 .map_err(|e| format!("创建会话目录失败: {}", e))?;
 Ok(sessions_dir)
}

fn validate_session_id(session_id: &str) -> Result<(), String> {
 if session_id.is_empty()
 || !session_id
 .chars()
 .all(|c| c.is_ascii_alphanumeric() || c == '-' || c == '_')
 {
 return Err(format!("非法会话 ID: {}", session_id));
 }
 Ok(())
}

fn session_file_path(app: &AppHandle, session_id: &str) -> Result<PathBuf, String> {
 validate_session_id(session_id)?;
 Ok(sessions_dir(app)?.join(format!("{}.json", session_id)))
}

fn now_rfc3339() -> String {
 chrono::Utc::now().to_rfc3339()
}

fn write_chat_session(app_handle: &AppHandle, session: &ChatSession) -> Result<(), String> {
 let path = session_file_path(app_handle, &session.id)?;
 let json = serde_json::to_string_pretty(session)
 .map_err(|e| format!("序列化聊天会话失败: {}", e))?;
 std::fs::write(&path, json).map_err(|e| format!("写入聊天会话失败: {}", e))?;
 Ok(())
}

fn read_saved_or_default_config(app_handle: &AppHandle) -> Result<AiApiConfig, String> {
    let path = config_file_path(app_handle)?;
    if !path.exists() {
        return Ok(AiApiConfig::default());
    }

    let json = std::fs::read_to_string(&path).map_err(|e| format!("读取配置文件失败: {}", e))?;
    serde_json::from_str(&json).map_err(|e| format!("解析配置文件失败: {}", e))
}

fn hide_api_key(mut config: AiApiConfig) -> AiApiConfig {
    config.api_key.clear();
    config
}

/// 列出聊天会话元数据
#[tauri::command]
pub fn list_chat_sessions(app_handle: AppHandle) -> Result<Vec<ChatSessionMeta>, String> {
 let dir = sessions_dir(&app_handle)?;
 let entries = std::fs::read_dir(&dir).map_err(|e| format!("读取会话目录失败: {}", e))?;
 let mut sessions = Vec::new();

 for entry in entries.flatten() {
 let path = entry.path();
 if path.extension().and_then(|ext| ext.to_str()) != Some("json") {
 continue;
 }

 let Ok(json) = std::fs::read_to_string(&path) else {
 continue;
 };
 let Ok(session) = serde_json::from_str::<ChatSession>(&json) else {
 continue;
 };
 sessions.push(ChatSessionMeta::from(&session));
 }

 sessions.sort_by(|a, b| b.updated_at.cmp(&a.updated_at));
 Ok(sessions)
}

/// 获取单个聊天会话
#[tauri::command]
pub fn get_chat_session(app_handle: AppHandle, session_id: String) -> Result<ChatSession, String> {
 let path = session_file_path(&app_handle, &session_id)?;
 if !path.exists() {
 return Err(format!("会话不存在: {}", session_id));
 }
 let json = std::fs::read_to_string(&path).map_err(|e| format!("读取聊天会话失败: {}", e))?;
 serde_json::from_str(&json).map_err(|e| format!("解析聊天会话失败: {}", e))
}

/// 创建聊天会话
#[tauri::command]
pub fn create_chat_session(app_handle: AppHandle) -> Result<ChatSession, String> {
 let now = now_rfc3339();
 let session = ChatSession {
 id: Uuid::new_v4().to_string(),
 title: "新对话".to_string(),
 messages: Vec::new(),
 created_at: now.clone(),
 updated_at: now,
 };
 write_chat_session(&app_handle, &session)?;
 Ok(session)
}

/// 保存聊天会话
#[tauri::command]
pub fn save_chat_session(app_handle: AppHandle, mut session: ChatSession) -> Result<(), String> {
 validate_session_id(&session.id)?;
 session.updated_at = now_rfc3339();
 write_chat_session(&app_handle, &session)
}

/// 删除聊天会话
#[tauri::command]
pub fn delete_chat_session(app_handle: AppHandle, session_id: String) -> Result<(), String> {
 let path = session_file_path(&app_handle, &session_id)?;
 if path.exists() {
 std::fs::remove_file(&path).map_err(|e| format!("删除聊天会话失败: {}", e))?;
 }
 Ok(())
}

///发送 AI 消息 (SSE 流式推送)
#[tauri::command]
pub async fn send_ai_message(
    app_handle: AppHandle,
    message: String,
    mut config: AiApiConfig,
) -> Result<(), String> {
    if config.api_key.is_empty() {
        config.api_key = read_saved_or_default_config(&app_handle)?.api_key;
    }

    let client = Client::new();

    // 构建请求 URL
    let url = format!("{}/chat/completions", config.base_url);

    // 构建请求体
    let request_body = serde_json::json!({
    "model": config.model,
    "stream": true,
    "messages": [
    {
    "role": "system",
    "content": "你是SophNet智能助手, 尽量使用markdown格式回复问题。"
    },
    {
    "role": "user",
    "content": message
    }
    ]
    });

    //发送 HTTP POST 请求
    let response = client
        .post(&url)
        .header("Content-Type", "application/json")
        .header("Authorization", format!("Bearer {}", config.api_key))
        .header("Accept", "text/plain")
        .json(&request_body)
        .send()
        .await
        .map_err(|e| format!("HTTP 请求失败: {}", e))?;

    // 检查 HTTP 状态码
    let status = response.status();
    if !status.is_success() {
        let error_body = response
            .text()
            .await
            .unwrap_or_else(|_| "无法读取错误响应体".to_string());
        return Err(format!(
            "API 错误 (HTTP {}): {}",
            status.as_u16(),
            error_body
        ));
    }

    // 流式读取 SSE 响应
    let mut stream = response.bytes_stream();

    let mut buffer = String::new();

    while let Some(chunk) = stream.next().await {
        let chunk = chunk.map_err(|e| format!("读取流失败: {}", e))?;
        let text = String::from_utf8_lossy(&chunk);
        buffer.push_str(&text);

        //处理缓冲区中的完整行
        while let Some(newline_pos) = buffer.find('\n') {
            let line = buffer[..newline_pos].trim_end_matches('\r').to_string();
            buffer = buffer[newline_pos + 1..].to_string();

            if let Some(sse_chunk) = parse_sse_line(&line) {
                // 推送 content chunk 到前端
                let payload = AiChunkPayload {
                    content: sse_chunk.content,
                    done: false,
                };
                app_handle
                    .emit("ai-chunk", payload)
                    .map_err(|e| format!("推送 ai-chunk事件失败: {}", e))?;
            }
        }
    }

    // 推送 done 信号到前端
    let done_payload = AiChunkPayload {
        content: "".to_string(),
        done: true,
    };
    app_handle
        .emit("ai-chunk", done_payload)
        .map_err(|e| format!("推送 ai-done事件失败: {}", e))?;

    Ok(())
}

/// 保存 AI 配置到文件
#[tauri::command]
pub fn save_ai_config(app_handle: AppHandle, mut config: AiApiConfig) -> Result<(), String> {
    if config.api_key.is_empty() {
        config.api_key = read_saved_or_default_config(&app_handle)?.api_key;
    }

    let path = config_file_path(&app_handle)?;
    let json =
        serde_json::to_string_pretty(&config).map_err(|e| format!("序列化配置失败: {}", e))?;
    std::fs::write(&path, json).map_err(|e| format!("写入配置文件失败: {}", e))?;
    Ok(())
}

/// 加载 AI 配置从文件
#[tauri::command]
pub fn load_ai_config(app_handle: AppHandle) -> Result<AiApiConfig, String> {
    read_saved_or_default_config(&app_handle).map(hide_api_key)
}

// ==================== Unit Tests ====================

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_parse_sse_line_content() {
        let line = "data: {\"choices\":[{\"delta\":{\"content\":\"Hello\"}}]}";
        let chunk = parse_sse_line(line);
        assert!(chunk.is_some());
        assert_eq!(chunk.unwrap().content, "Hello");
    }

    #[test]
    fn test_parse_sse_line_done() {
        let line = "data: [DONE]";
        let chunk = parse_sse_line(line);
        assert!(chunk.is_none());
    }

    #[test]
    fn test_parse_sse_line_invalid() {
        let line = "event: ping";
        let chunk = parse_sse_line(line);
        assert!(chunk.is_none());
    }
}
