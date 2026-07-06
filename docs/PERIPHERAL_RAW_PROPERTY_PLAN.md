# 外设通用键值属性编辑器（方案 B）实现规划书

> 目标：在保留现有 5 个「友好属性」编辑器（`status` / `clock-frequency` / `#pwm-cells` / `current-speed` / `ch-remap`）的基础上，新增一个**通用原始属性（Raw Property）编辑器**，让客户可以对任意外设节点查看 / 修改 / 新增 / 删除 DTS 属性，并对关键属性做只读保护，方便客户完成 DTS 配置。

---

## 1. 背景与现状

CviCubeMX 外设配置页（`PeripheralPage` → `PeripheralTree` → `ConfigDialog`）当前只能编辑 5 个硬编码属性，依赖 `PeripheralInfo` 上的 `has_xxx` 布尔标志驱动 UI。这导致：

| 外设 | DTS 实际属性 | 弹窗显示 |
|------|------|------|
| GPIO | 常无可配属性 | **空白** |
| SARADC | 常仅 `status` | **空白 / 极少** |
| I2C | `status` + `clock-frequency` | 频率（+ status） |
| PWM | `status` + `#pwm-cells` | pwm-cells（+ status） |
| SPI | 仅 `status` | 仅使能 |
| UART | `status` + `current-speed` | 使能 + 波特率 |

**根因**：解析器只提取 5 个已知属性；前端只在 `has_xxx == true` 时渲染表单，因此无法「新增」原本不存在的属性，也无法覆盖 GPIO/SARADC/SPI 的真实可配项。

---

## 2. 方案总览

新增一条独立的「原始属性」链路，全部为**增量**，不改动现有 5 个命令的行为与已通过的测试：

```
DTS 文件
  │  load_content / load_file
  ▼
DtsParser ──────────────┐
  │ get_raw_properties() │  新增：解析节点全部属性
  ▼                      │
RawProperty[] ───────────┘
  │
  ├─ set_raw_property()   （dts_writer，upsert，保护校验）
  └─ delete_raw_property()（dts_writer，删除，保护校验）
  │
Tauri 命令（3 个）→ peripheralStore（3 个 action）→ ConfigDialog 高级折叠区
```

### 关键约束（所有 subagent 必须遵守）

1. **不破坏现有测试**：Rust 199 个、前端 165 个测试必须继续全绿。
2. **增量暴露**：新能力通过新命令暴露，不修改 `PeripheralInfo` 已有字段语义。
3. **行级写入**：复用现有 `update_property_in_node` 的行级替换思想，保留注释与格式。
4. **值格式三分类**：`cell`（`<...>`）、`string`（`"..."`）、`bool`（无值，如 `wakeup-source;`）。
5. **保护属性**：`compatible` / `reg` / `interrupts` / `interrupt-parent` / `clocks` / `clock-names` / `#address-cells` / `#size-cells` / `dmas` / `dma-names` 只读，禁止修改/删除。

---

## 3. 数据结构（Rust，`src-tauri/src/peripheral.rs`）

```rust
/// A single raw property parsed from a DTS node.
#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub struct RawProperty {
    /// e.g. "clock-frequency", "compatible", "#pwm-cells"
    pub key: String,
    /// Raw inner value WITHOUT surrounding `<>`/`""`. Empty for bool props.
    pub value: String,
    /// "cell" (<...>), "string" ("..."), or "bool" (no value).
    pub kind: String,
    /// Whether this property is protected (read-only in UI).
    pub protected: bool,
}

/// Properties that must never be edited/deleted via the raw editor.
pub const PROTECTED_PROPERTIES: &[&str] = &[
    "compatible", "reg", "interrupts", "interrupt-parent",
    "clocks", "clock-names", "#address-cells", "#size-cells",
    "dmas", "dma-names",
];

pub fn is_protected_property(key: &str) -> bool {
    PROTECTED_PROPERTIES.contains(&key)
}
```

---

## 4. 涉及文件清单

| 文件 | 改动 | 负责 subagent |
|------|------|------|
| `src-tauri/src/peripheral.rs` | 新增 `RawProperty` / 保护清单 / 3 个 Tauri 命令 | A5(结构) / A3(命令) |
| `src-tauri/src/dts_parser.rs` | 新增 `get_raw_properties` / `parse_raw_properties_from_node` | A1 |
| `src-tauri/src/dts_writer.rs` | 新增 `set_raw_property` / `delete_raw_property` | A2 |
| `src-tauri/src/lib.rs` | 注册 3 个新命令 | A3 |
| `src/stores/peripheralStore.ts` | 新增 `RawProperty` 类型 + 3 个 action | B1 |
| `src/components/ConfigDialog.tsx` | 新增「高级：原始属性」折叠区 | B2 |
| `src-tauri/tests/raw_property_test.rs` | 新建 Rust 集成测试 | C1 |
| `src/__tests__/rawProperty.test.ts` | 新建前端 store 测试 | C2 |

---

## 5. 验收标准（Definition of Done）

1. `cd src-tauri && cargo test` 全绿（原 199 + 新增）。
2. `npx vitest run` 全绿（原 165 + 新增）。
3. `npx tauri build --no-bundle` 通过。
4. 手动验证：GPIO/SARADC 弹窗能看到并编辑真实属性；可对任意外设新增/删除自定义属性；保护属性只读；现有 5 个友好属性行为不变。

---

## 6. 分 Subagent 执行 Prompt

> 建议执行顺序：**A1 → A2 → A5/A3 →（Rust 编译通过后）B1 → B2 →（前端编译通过后）C1 / C2**。
> A1、A2、A5 之间存在依赖（写入器依赖结构，命令依赖两者），前端 B1 依赖 A3 的命令名。测试 C1/C2 可在对应实现完成后并行。
> 每个 Prompt 均可整段复制给 `runSubagent`（`Explore` 用于只读探查，实现类任务使用默认编码 agent）。

---

### Subagent A1 — Rust 解析器：`get_raw_properties`

```
你是 Rust 后端工程师，在 CviCubeMX（Tauri v2 + Rust）项目中工作。

任务：在 src-tauri/src/dts_parser.rs 新增解析节点「全部属性」的能力，不改动现有 parse_node 逻辑。

前置阅读：
- src-tauri/src/dts_parser.rs（了解 find_node_position_in_str、load_content、get_file_content）
- src-tauri/src/peripheral.rs（RawProperty 结构与 is_protected_property 由 subagent A5 提供；若尚未存在，请先在 peripheral.rs 添加，见文档第 3 节）

实现两个方法（impl DtsParser 内）：
1. pub fn get_raw_properties(&self, node_name: &str) -> Result<Vec<RawProperty>, String>
   - 用 find_node_position_in_str 定位节点；找不到返回 Err(format!("未找到节点: {}", node_name))。
   - 取节点切片，调用 parse_raw_properties_from_node。
2. pub fn parse_raw_properties_from_node(node_content: &str) -> Vec<RawProperty>
   实现要点：
   a. 只扫描节点最外层 { 与配对 } 之间的 body（用括号计数定位配对括号）。
   b. 先移除注释：多行 /* ... */ 与行内 // ...。
   c. 再用括号计数移除 body 内的嵌套子节点 { ... } 块（避免把子节点属性当作父节点属性）。
   d. 将剩余 body 的换行折叠为空格（处理跨行的 <...> 多行 cell 值），再按 ';' 切分为语句。
   e. 每条语句 trim 后跳过空串；按以下三种形态匹配（顺序：cell → string → bool）：
      - cell:   ^\s*([#\w\-,]+)\s*=\s*<(.*)>\s*$   -> kind="cell",  value=$2.trim()
      - string: ^\s*([#\w\-,]+)\s*=\s*"(.*)"\s*$   -> kind="string",value=$2
      - bool:   ^\s*([#\w\-,]+)\s*$                 -> kind="bool",  value=""
   f. protected = crate::peripheral::is_protected_property(key)。
   g. 保持属性在 DTS 中的出现顺序（用 Vec 顺序 push）。

约束：
- 不修改现有 parse_node / parse_dts_file 及任何现有测试。
- 使用已引入的 regex crate。
- 运行 `cd src-tauri && cargo build` 确认编译通过。

交付：修改后的 dts_parser.rs；简述你如何处理子节点剥离与多行 cell 值。
```

---

### Subagent A2 — Rust 写入器：`set_raw_property` / `delete_raw_property`

```
你是 Rust 后端工程师，在 CviCubeMX 项目工作。

任务：在 src-tauri/src/dts_writer.rs 新增两个方法，复用现有 update_property_in_node 的行级替换思想。

前置阅读：
- src-tauri/src/dts_writer.rs（重点看 update_property_in_node、find_node_position_in_content 的用法）
- src-tauri/src/peripheral.rs（is_protected_property）

实现（impl DtsWriter 内）：
1. pub fn set_raw_property(parser, peripheral: &str, key: &str, value: &str, kind: &str) -> Result<(), String>
   - if is_protected_property(key) { return Err(format!("属性 {} 受保护，不可修改", key)); }
   - 依据 kind 构造新行 new_line：
       cell   -> format!("\n\t\t{} = <{}>;", key, value)
       string -> format!("\n\t\t{} = \"{}\";", key, value)
       bool   -> format!("\n\t\t{};", key)
       其它 kind -> Err("不支持的属性类型")
   - 构造匹配「已存在同名属性行」的正则（用 regex::escape(key)），需同时匹配 cell/string/bool 三形态，例如：
       format!(r#"\s*{}\s*(=\s*(<[^>]*>|"[^"]*"))?\s*;"#, regex::escape(key))
   - 复用 Self::update_property_in_node(parser, peripheral, &regex, new_line, /*should_add=*/true)
     （存在则替换、不存在则插入，避免同名重复）。

2. pub fn delete_raw_property(parser, peripheral: &str, key: &str) -> Result<(), String>
   - 保护校验同上（受保护返回 Err）。
   - 定位节点，构造匹配「整行含前导换行与缩进」的正则，例如：
       format!(r#"\n[ \t]*{}\s*(=\s*(<[^>]*>|"[^"]*"))?\s*;"#, regex::escape(key))
   - 在节点切片内 replace_all 为 ""，再 splice 回文件内容并 parser.load_content(new_content)。
   - 目标：删除后不留空行、不破坏相邻属性。
   - 若 update_property_in_node 为私有且不便复用删除逻辑，可在本方法内直接实现 find + 切片 + replace + load_content。

约束：
- 不改动现有 update_status / update_clock_frequency 等方法与其测试。
- 运行 `cd src-tauri && cargo build` 确认编译通过。

交付：修改后的 dts_writer.rs；说明删除时如何保证不产生空行。
```

---

### Subagent A5 — Rust 数据结构与保护清单

```
你是 Rust 后端工程师，在 CviCubeMX 项目工作。

任务：在 src-tauri/src/peripheral.rs 顶部区域（PeripheralInfo 附近）新增 RawProperty 结构、保护属性常量与判断函数。

前置阅读：src-tauri/src/peripheral.rs（确认已 use serde::{Deserialize, Serialize}）。

新增内容（见规划书第 3 节，逐字实现）：
- #[derive(Debug, Clone, PartialEq, Serialize, Deserialize)] struct RawProperty { key, value, kind, protected }
- pub const PROTECTED_PROPERTIES: &[&str] = &[ "compatible","reg","interrupts","interrupt-parent","clocks","clock-names","#address-cells","#size-cells","dmas","dma-names" ];
- pub fn is_protected_property(key: &str) -> bool { PROTECTED_PROPERTIES.contains(&key) }

约束：
- 全部 pub，供 dts_parser / dts_writer / 集成测试 use。
- 不改动 PeripheralInfo 现有字段。
- 运行 `cd src-tauri && cargo build` 确认编译通过。

注意：本任务应在 A1/A2 之前或同时完成，因为它们依赖 RawProperty 与 is_protected_property。

交付：修改后的 peripheral.rs 片段。
```

---

### Subagent A3 — Tauri 命令注册

```
你是 Rust/Tauri 工程师，在 CviCubeMX 项目工作。

任务：新增 3 个 Tauri 命令并在 lib.rs 注册，模式与现有命令（set_peripheral_status 等）完全一致，含写回文件。

前置阅读：
- src-tauri/src/peripheral.rs（现有 #[tauri::command] 命令写法、DtsState、写回文件逻辑）
- src-tauri/src/lib.rs（invoke_handler! 注册列表）
- 依赖：RawProperty(A5)、DtsParser::get_raw_properties(A1)、DtsWriter::set_raw_property/delete_raw_property(A2)

在 peripheral.rs 追加：
- #[tauri::command] pub fn get_peripheral_raw_properties(state, peripheral: String) -> Result<Vec<RawProperty>, String>
    锁 parser，调用 parser.get_raw_properties(&peripheral)。（只读，不写回文件）
- #[tauri::command] pub fn set_peripheral_raw_property(state, peripheral: String, key: String, value: String, kind: String) -> Result<(), String>
    调用 DtsWriter::set_raw_property(...)，成功后若 get_file_path() 有值则 std::fs::write 写回。
- #[tauri::command] pub fn delete_peripheral_raw_property(state, peripheral: String, key: String) -> Result<(), String>
    调用 DtsWriter::delete_raw_property(...)，成功后写回文件。

在 lib.rs 的 invoke_handler! 列表追加：
    peripheral::get_peripheral_raw_properties,
    peripheral::set_peripheral_raw_property,
    peripheral::delete_peripheral_raw_property,

约束：
- 参数命名与前端 invoke 保持一致（peripheral / key / value / kind）。
- 运行 `cd src-tauri && cargo build` 确认编译通过。

交付：peripheral.rs 与 lib.rs 的改动片段。
```

---

### Subagent B1 — 前端 store：类型与 actions

```
你是前端工程师（React + TypeScript + Zustand），在 CviCubeMX 项目工作。

任务：在 src/stores/peripheralStore.ts 新增 RawProperty 类型与 3 个 action，风格与现有 action 一致（成功后调用 fetchDtsContent 刷新 DTS 预览）。

前置阅读：src/stores/peripheralStore.ts（现有 invoke 用法、setPeripheralStatus 的写法、fetchDtsContent）。
依赖命令名（由 A3 定义）：get_peripheral_raw_properties / set_peripheral_raw_property / delete_peripheral_raw_property。

新增：
export interface RawProperty {
  key: string;
  value: string;
  kind: 'cell' | 'string' | 'bool';
  protected: boolean;
}

在 PeripheralState 接口追加：
  getRawProperties: (name: string) => Promise<RawProperty[]>;
  setRawProperty: (name: string, key: string, value: string, kind: string) => Promise<void>;
  deleteRawProperty: (name: string, key: string) => Promise<void>;

实现：
  getRawProperties: async (name) =>
    await invoke<RawProperty[]>('get_peripheral_raw_properties', { peripheral: name }),
  setRawProperty: async (name, key, value, kind) => {
    await invoke('set_peripheral_raw_property', { peripheral: name, key, value, kind });
    await get().fetchDtsContent();
  },
  deleteRawProperty: async (name, key) => {
    await invoke('delete_peripheral_raw_property', { peripheral: name, key });
    await get().fetchDtsContent();
  },

约束：
- 不改动现有 action 与 PeripheralInfo 类型。
- 运行 `npx tsc --noEmit` 或 `npm run build` 确认类型通过。

交付：修改后的 peripheralStore.ts 片段。
```

---

### Subagent B2 — 前端 UI：ConfigDialog 高级属性折叠区

```
你是前端工程师（React + TypeScript + Tailwind），在 CviCubeMX 项目工作。

任务：在 src/components/ConfigDialog.tsx 现有表单下方，新增可折叠的「⚙️ 高级：原始属性编辑」区块，复用 peripheralStore 的 3 个新 action（getRawProperties / setRawProperty / deleteRawProperty）。

前置阅读：
- src/components/ConfigDialog.tsx（现有弹窗结构、样式风格、handleSave 流程）
- src/stores/peripheralStore.ts（RawProperty 类型与新 action，由 B1 提供）

需求：
1. 弹窗打开时（useEffect 依赖 peripheral）调用 getRawProperties(peripheral.name)，存入本地 state：rawProps: RawProperty[]。
2. 折叠区默认收起，标题「⚙️ 高级：原始属性编辑」，点击展开/收起。
3. 每行渲染：key 输入框 + kind 下拉(cell/string/bool) + value 输入框(kind==='bool' 时禁用并置空) + 删除按钮。
   - protected===true 的行：key/kind/value 全部只读(disabled)，显示 🔒 图标，隐藏删除按钮。
4. 底部「+ 新增属性」按钮：向本地 state 追加一条空白可编辑行 { key:'', value:'', kind:'cell', protected:false }。
5. 保存策略（在现有 handleSave 里，友好属性保存之后追加）：
   - 计算被删除的行（原 rawProps 中存在、当前列表已移除且非 protected）→ 调用 deleteRawProperty。
   - 对当前列表中 key 非空、非 protected、且（新增或 value/kind 有变更）的行 → 调用 setRawProperty(name, key, kind==='bool'?'':value, kind)。
   - 遇错 console.error 并保留弹窗打开。
6. 复用现有 Tailwind 配色（slate/indigo 系），与现有表单块视觉一致。

约束：
- 不破坏现有 5 个友好属性表单及其保存逻辑。
- GPIO/SARADC 打开弹窗时，即便无友好属性，也能在高级区看到真实属性（如 compatible/reg 只读、status 可改），弹窗不再空白。
- 运行 `npm run build` 确认编译通过。

交付：修改后的 ConfigDialog.tsx；描述保存时新增/修改/删除的判定逻辑。
```

---

### Subagent C1 — Rust 集成测试

```
你是 Rust 测试工程师，在 CviCubeMX 项目工作。

任务：新建 src-tauri/tests/raw_property_test.rs，覆盖解析、upsert、删除、保护、子节点隔离、多行 cell、回归等场景。

前置阅读：
- src-tauri/tests/dts_parser_test.rs（了解 use cvicubemx_lib::... 的引用方式与断言风格）
- 已实现的 dts_parser.rs(A1) / dts_writer.rs(A2) / peripheral.rs(A5)

请逐字实现下列 15 个测试（A1~A15，见规划书附录“测试用例”），确保：
- 引用：use cvicubemx_lib::dts_parser::DtsParser; use cvicubemx_lib::dts_writer::DtsWriter; use cvicubemx_lib::peripheral::is_protected_property;
- 覆盖点：三种 kind 解析、protected 标记、声明顺序、已存在 cell 属性替换(不重复)、新增 string/cell/bool、删除属性、删除不留空行、保护属性拒绝改/删、子节点属性隔离、多行 cell 值、缺失节点报错、回归(raw 编辑不破坏友好属性解析)。

运行 `cd src-tauri && cargo test --test raw_property_test` 应全绿；随后 `cargo test` 整体全绿（原 199 不回归）。

交付：raw_property_test.rs 全文与测试运行结果摘要。
```

---

### Subagent C2 — 前端 store 测试

```
你是前端测试工程师（Vitest），在 CviCubeMX 项目工作。

任务：新建 src/__tests__/rawProperty.test.ts，验证 peripheralStore 的 3 个新 action。

前置阅读：
- src/__tests__/ 下现有测试对 @tauri-apps/api/core 的 invoke mock 方式
- src/stores/peripheralStore.ts（B1 新增的 action）

实现（见规划书附录“前端测试”B1~B4）：
- 使用 vi.mock('@tauri-apps/api/core', () => ({ invoke: (...a)=>invokeMock(...a) }))。
- beforeEach 重置 mock 与 store 状态。
- B1: getRawProperties 以 { peripheral } 调用 'get_peripheral_raw_properties'。
- B2: setRawProperty 第 1 次调用 'set_peripheral_raw_property'（含 peripheral/key/value/kind），第 2 次调用 'get_dts_content' 并更新 dtsContent。
- B3: deleteRawProperty 调用 'delete_peripheral_raw_property'（含 peripheral/key）后刷新。
- B4: setRawProperty 在后端 reject 时 rejects。

运行 `npx vitest run src/__tests__/rawProperty.test.ts` 应全绿；随后 `npx vitest run` 整体全绿（原 165 不回归）。

交付：rawProperty.test.ts 全文与运行结果摘要。
```

---

## 7. 附录：测试用例源码

### 7.1 Rust 集成测试（`src-tauri/tests/raw_property_test.rs`）

```rust
use cvicubemx_lib::dts_parser::DtsParser;
use cvicubemx_lib::dts_writer::DtsWriter;
use cvicubemx_lib::peripheral::is_protected_property;

const SAMPLE: &str = r#"
&i2c0 {
	compatible = "snps,designware-i2c";
	reg = <0x0 0x04000000 0x0 0x1000>;
	status = "okay";
	clock-frequency = <100000>;
	wakeup-source;
};
&spi0 {
	compatible = "snps,dw-apb-ssi";
	status = "disabled";
};
&saradc {
	compatible = "cvitek,saradc";
	status = "okay";
};
"#;

// A1. 解析出全部属性，含三种 kind
#[test]
fn test_parse_all_raw_properties() {
    let mut p = DtsParser::new();
    p.load_content(SAMPLE);
    let props = p.get_raw_properties("i2c0").unwrap();

    let keys: Vec<&str> = props.iter().map(|x| x.key.as_str()).collect();
    assert!(keys.contains(&"compatible"));
    assert!(keys.contains(&"reg"));
    assert!(keys.contains(&"status"));
    assert!(keys.contains(&"clock-frequency"));
    assert!(keys.contains(&"wakeup-source"));

    let comp = props.iter().find(|x| x.key == "compatible").unwrap();
    assert_eq!(comp.kind, "string");
    assert_eq!(comp.value, "snps,designware-i2c");

    let freq = props.iter().find(|x| x.key == "clock-frequency").unwrap();
    assert_eq!(freq.kind, "cell");
    assert_eq!(freq.value.trim(), "100000");

    let wake = props.iter().find(|x| x.key == "wakeup-source").unwrap();
    assert_eq!(wake.kind, "bool");
    assert_eq!(wake.value, "");
}

// A2. 保护属性标记正确
#[test]
fn test_protected_flag() {
    let mut p = DtsParser::new();
    p.load_content(SAMPLE);
    let props = p.get_raw_properties("i2c0").unwrap();
    assert!(props.iter().find(|x| x.key == "compatible").unwrap().protected);
    assert!(props.iter().find(|x| x.key == "reg").unwrap().protected);
    assert!(!props.iter().find(|x| x.key == "status").unwrap().protected);
    assert!(is_protected_property("clocks"));
    assert!(!is_protected_property("foo-bar"));
}

// A3. 保持声明顺序
#[test]
fn test_preserve_order() {
    let mut p = DtsParser::new();
    p.load_content(SAMPLE);
    let props = p.get_raw_properties("i2c0").unwrap();
    let keys: Vec<&str> = props.iter().map(|x| x.key.as_str()).collect();
    assert_eq!(keys[0], "compatible");
    assert_eq!(keys[1], "reg");
    assert_eq!(keys[2], "status");
}

// A4. upsert：修改已存在的 cell 属性（不重复插入）
#[test]
fn test_set_existing_cell_property() {
    let mut p = DtsParser::new();
    p.load_content(SAMPLE);
    DtsWriter::set_raw_property(&mut p, "i2c0", "clock-frequency", "400000", "cell").unwrap();
    let c = p.get_file_content();
    assert!(c.contains("clock-frequency = <400000>;"));
    assert_eq!(c.matches("clock-frequency").count(), 1); // 未重复
}

// A5. upsert：新增不存在的 string 属性
#[test]
fn test_add_new_string_property() {
    let mut p = DtsParser::new();
    p.load_content(SAMPLE);
    DtsWriter::set_raw_property(&mut p, "spi0", "spi-max-frequency", "abc", "string").unwrap();
    let c = p.get_file_content();
    assert!(c.contains("spi-max-frequency = \"abc\";"));
}

// A6. upsert：新增 cell 属性到 SPI（真实场景 spi-max-frequency）
#[test]
fn test_add_cell_property_to_spi() {
    let mut p = DtsParser::new();
    p.load_content(SAMPLE);
    DtsWriter::set_raw_property(&mut p, "spi0", "spi-max-frequency", "50000000", "cell").unwrap();
    assert!(p.get_file_content().contains("spi-max-frequency = <50000000>;"));
}

// A7. 新增 bool 属性
#[test]
fn test_add_bool_property() {
    let mut p = DtsParser::new();
    p.load_content(SAMPLE);
    DtsWriter::set_raw_property(&mut p, "saradc", "cvitek,use-vref", "", "bool").unwrap();
    assert!(p.get_file_content().contains("cvitek,use-vref;"));
}

// A8. 删除非保护属性
#[test]
fn test_delete_property() {
    let mut p = DtsParser::new();
    p.load_content(SAMPLE);
    DtsWriter::delete_raw_property(&mut p, "i2c0", "wakeup-source").unwrap();
    let c = p.get_file_content();
    assert!(!c.contains("wakeup-source"));
    assert!(c.contains("clock-frequency = <100000>;"));
}

// A9. 删除后不留空行/不破坏相邻属性
#[test]
fn test_delete_no_dangling_blank_line() {
    let mut p = DtsParser::new();
    p.load_content(SAMPLE);
    DtsWriter::delete_raw_property(&mut p, "i2c0", "clock-frequency").unwrap();
    let c = p.get_file_content();
    assert!(!c.contains("clock-frequency"));
    assert!(c.contains("status = \"okay\";"));
    assert!(c.contains("wakeup-source;"));
    assert!(!c.contains("\n\n\n"));
}

// A10. 保护属性拒绝修改
#[test]
fn test_reject_set_protected() {
    let mut p = DtsParser::new();
    p.load_content(SAMPLE);
    let r = DtsWriter::set_raw_property(&mut p, "i2c0", "compatible", "x", "string");
    assert!(r.is_err());
    assert!(p.get_file_content().contains("snps,designware-i2c"));
}

// A11. 保护属性拒绝删除
#[test]
fn test_reject_delete_protected() {
    let mut p = DtsParser::new();
    p.load_content(SAMPLE);
    assert!(DtsWriter::delete_raw_property(&mut p, "i2c0", "reg").is_err());
    assert!(p.get_file_content().contains("reg = <0x0 0x04000000"));
}

// A12. 不误入子节点
#[test]
fn test_ignore_child_node_properties() {
    let content = r#"
&spi0 {
	status = "okay";
	flash@0 {
		compatible = "jedec,spi-nor";
		reg = <0>;
	};
};
"#;
    let mut p = DtsParser::new();
    p.load_content(content);
    let props = p.get_raw_properties("spi0").unwrap();
    let keys: Vec<&str> = props.iter().map(|x| x.key.as_str()).collect();
    assert!(keys.contains(&"status"));
    assert_eq!(keys.iter().filter(|k| **k == "compatible").count(), 0);
}

// A13. 多行 cell 值解析
#[test]
fn test_parse_multiline_cell() {
    let content = r#"
sysdma_remap {
	ch-remap = <0 5 2 3
				42 42 4 7>;
	status = "okay";
};
"#;
    let mut p = DtsParser::new();
    p.load_content(content);
    let props = p.get_raw_properties("sysdma_remap").unwrap();
    let ch = props.iter().find(|x| x.key == "ch-remap").unwrap();
    assert_eq!(ch.kind, "cell");
    assert!(ch.value.contains("0 5 2 3"));
    assert!(ch.value.contains("42 42 4 7"));
}

// A14. 找不到节点返回错误
#[test]
fn test_missing_node_error() {
    let mut p = DtsParser::new();
    p.load_content(SAMPLE);
    assert!(p.get_raw_properties("not_exist").is_err());
}

// A15. 回归：raw 编辑不破坏现有友好属性解析
#[test]
fn test_regression_friendly_props_intact() {
    let mut p = DtsParser::new();
    p.load_content(SAMPLE);
    DtsWriter::set_raw_property(&mut p, "i2c0", "foo", "1", "cell").unwrap();
    let i2c0 = p.get_peripheral("i2c0").unwrap();
    assert_eq!(i2c0.status, "okay");
    assert_eq!(i2c0.clock_frequency, 100000);
    assert!(p.get_file_content().contains("foo = <1>;"));
}
```

### 7.2 前端测试（`src/__tests__/rawProperty.test.ts`）

```ts
import { describe, it, expect, vi, beforeEach } from 'vitest';

const invokeMock = vi.fn();
vi.mock('@tauri-apps/api/core', () => ({ invoke: (...a: any[]) => invokeMock(...a) }));

import { usePeripheralStore } from '../stores/peripheralStore';

describe('raw property store actions', () => {
  beforeEach(() => {
    invokeMock.mockReset();
    usePeripheralStore.setState({ peripherals: [], dtsContent: '' });
  });

  // B1. getRawProperties 透传命令名与参数
  it('getRawProperties invokes correct command', async () => {
    invokeMock.mockResolvedValueOnce([
      { key: 'status', value: 'okay', kind: 'string', protected: false },
    ]);
    const res = await usePeripheralStore.getState().getRawProperties('i2c0');
    expect(invokeMock).toHaveBeenCalledWith('get_peripheral_raw_properties', { peripheral: 'i2c0' });
    expect(res[0].key).toBe('status');
  });

  // B2. setRawProperty 透传全部参数并刷新 DTS 预览
  it('setRawProperty invokes set + fetchDtsContent', async () => {
    invokeMock.mockResolvedValueOnce(undefined)      // set_peripheral_raw_property
             .mockResolvedValueOnce('NEW DTS CONTENT'); // get_dts_content
    await usePeripheralStore.getState().setRawProperty('spi0', 'spi-max-frequency', '50000000', 'cell');
    expect(invokeMock).toHaveBeenNthCalledWith(1, 'set_peripheral_raw_property', {
      peripheral: 'spi0', key: 'spi-max-frequency', value: '50000000', kind: 'cell',
    });
    expect(invokeMock).toHaveBeenNthCalledWith(2, 'get_dts_content');
    expect(usePeripheralStore.getState().dtsContent).toBe('NEW DTS CONTENT');
  });

  // B3. deleteRawProperty 透传命令并刷新
  it('deleteRawProperty invokes delete + fetchDtsContent', async () => {
    invokeMock.mockResolvedValueOnce(undefined).mockResolvedValueOnce('DTS');
    await usePeripheralStore.getState().deleteRawProperty('i2c0', 'wakeup-source');
    expect(invokeMock).toHaveBeenNthCalledWith(1, 'delete_peripheral_raw_property', {
      peripheral: 'i2c0', key: 'wakeup-source',
    });
  });

  // B4. 错误冒泡
  it('setRawProperty propagates backend error', async () => {
    invokeMock.mockRejectedValueOnce('属性 compatible 受保护，不可修改');
    await expect(
      usePeripheralStore.getState().setRawProperty('i2c0', 'compatible', 'x', 'string')
    ).rejects.toBeTruthy();
  });
});
```

---

## 8. 待确认事项

1. **保护属性清单**是否符合 SDK 实际？当前为保守清单（`compatible/reg/interrupts/clocks/#*-cells/dmas/dma-names`）。若客户需改 `interrupts` 等，可调整。
2. **`kind` 分类是否够用**？当前 cell/string/bool 三类，`phandle`（`<&ref>`）、`bytestring`（`[00 11]`）会被归入 cell 原样保留；如需专门处理请告知。
3. **UI 摆放**：当前放入 `ConfigDialog` 的可折叠「高级」区。若希望独立 Tab / 独立弹窗，调整 B2 Prompt 即可。
