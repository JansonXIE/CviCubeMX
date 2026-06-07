use serde::{Deserialize, Serialize};
use tauri::{AppHandle, Emitter, Manager};
use reqwest::Client;
use std::path::PathBuf;
use futures_util::StreamExt;
use tokio::io::AsyncBufReadExt;

/// AI API 配置
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct AiApiConfig {
    pub api_key: String,
    pub base_url: String,
    pub model: String,
}

impl Default for AiApiConfig {
    fn default() -> Self {
        Self {
            api_key: "your_api_key_here".to_string(),
            base_url: "https://www.sophnet.com/api/open-apis/v1".to_string(),
            model: "DeepSeek-V3.2-Exp:6P2FGzuj1EOFpP2DCX2miK".to_string(),
        }
    }
}

/// SSE chunk 解析结果
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct SseChunk {
    pub content: String,
}

/// AI chunk 推送 payload (通过 Tauri Event System 发送给前端)
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct AiChunkPayload {
    pub content: String,
    pub done: bool,
}

/// SSE 行解析: 解析 "data: ..." 行
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

    // 解析 JSON: {"choices":[{"delta":{"content":"..."}}]}
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
    std::fs::create_dir_all(&app_dir)
        .map_err(|e| format!("创建应用数据目录失败: {}", e))?;
    Ok(app_dir.join("ai_config.json"))
}

/// 发送 AI 消息 (SSE 流式推送)
#[tauri::command]
pub async fn send_ai_message(
    app_handle: AppHandle,
    message: String,
    config: AiApiConfig,
) -> Result<(), String> {
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

    // 发送 HTTP POST 请求
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
        return Err(format!("API 错误 (HTTP {}): {}", status.as_u16(), error_body));
    }

    // 流式读取 SSE 响应
    let mut stream = response.bytes_stream();

    let mut buffer = String::new();

    while let Some(chunk) = stream.next().await {
        let chunk = chunk.map_err(|e| format!("读取流失败: {}", e))?;
        let text = String::from_utf8_lossy(&chunk);
        buffer.push_str(&text);

        // 处理缓冲区中的完整行
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
                    .map_err(|e| format!("推送 ai-chunk 事件失败: {}", e))?;
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
        .map_err(|e| format!("推送 ai-done 事件失败: {}", e))?;

    Ok(())
}

/// 保存 AI 配置到文件
#[tauri::command]
pub fn save_ai_config(app_handle: AppHandle, config: AiApiConfig) -> Result<(), String> {
    let path = config_file_path(&app_handle)?;
    let json = serde_json::to_string_pretty(&config)
        .map_err(|e| format!("序列化配置失败: {}", e))?;
    std::fs::write(&path, json)
        .map_err(|e| format!("写入配置文件失败: {}", e))?;
    Ok(())
}

/// 加载 AI 配置从文件
#[tauri::command]
pub fn load_ai_config(app_handle: AppHandle) -> Result<AiApiConfig, String> {
    let path = config_file_path(&app_handle)?;
    if !path.exists() {
        // 配置文件不存在，返回默认配置
        return Ok(AiApiConfig::default());
    }
    let json = std::fs::read_to_string(&path)
        .map_err(|e| format!("读取配置文件失败: {}", e))?;
    let config: AiApiConfig = serde_json::from_str(&json)
        .map_err(|e| format!("解析配置文件失败: {}", e))?;
    Ok(config)
}