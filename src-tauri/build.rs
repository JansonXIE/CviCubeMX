use std::collections::HashMap;
use std::fs;
use std::path::Path;

fn main() {
    // 将 AI 配置在构建期烧入二进制（供 ai_chat.rs 的 option_env! 读取），
    // 避免硬编码敏感信息到源码中。
    //
    // 注意：cargo/tauri 默认 **不会** 读取 .env 文件，option_env! 只能看到
    // 编译进程继承的 shell 环境变量。为避免开发者必须手动 export 才能编译出
    // 带 key 的二进制（否则 api_key 为空，请求会被 API 以 HTTP 400 "缺少授权参数" 拒绝），
    // 这里在构建期解析项目根目录的 .env，并通过 cargo:rustc-env 注入。
    //
    // 优先级：shell 环境变量 > .env 文件。CI（GitHub Actions）通过 secrets 注入
    // shell 环境变量且没有 .env 文件，本地开发则回退到 .env。
    inject_ai_config();

    // 环境变量变化时强制重新编译，确保 option_env! 读取到最新注入值。
    println!("cargo:rerun-if-env-changed=AI_API_KEY");
    println!("cargo:rerun-if-env-changed=AI_BASE_URL");
    println!("cargo:rerun-if-env-changed=AI_MODEL");
    // .env 文件变化时也重新编译（build.rs 的 CWD 为 src-tauri，.env 在上一级）。
    println!("cargo:rerun-if-changed=../.env");

    tauri_build::build()
}

/// 解析 .env 并通过 cargo:rustc-env 注入 AI 配置。
fn inject_ai_config() {
    let dotenv = parse_dotenv(Path::new("../.env"));

    for key in ["AI_API_KEY", "AI_BASE_URL", "AI_MODEL"] {
        // shell 环境变量优先，其次 .env 文件；两者都没有则不注入，
        // 交由 ai_chat.rs 中的代码默认值兜底。
        let value = std::env::var(key).ok().or_else(|| dotenv.get(key).cloned());
        if let Some(v) = value {
            println!("cargo:rustc-env={}={}", key, v);
        }
    }
}

/// 极简 .env 解析：忽略空行与 `#` 注释，按第一个 `=` 分割，去除值两端引号。
fn parse_dotenv(path: &Path) -> HashMap<String, String> {
    let mut map = HashMap::new();
    let Ok(content) = fs::read_to_string(path) else {
        return map;
    };
    for line in content.lines() {
        let line = line.trim();
        if line.is_empty() || line.starts_with('#') {
            continue;
        }
        if let Some((k, v)) = line.split_once('=') {
            let k = k.trim().to_string();
            let v = v.trim().trim_matches('"').trim_matches('\'').to_string();
            map.insert(k, v);
        }
    }
    map
}
