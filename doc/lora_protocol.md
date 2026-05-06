# LoRa 通信协议

## 1. 文档状态

本文档用于描述本项目当前使用的 LoRa 协议格式与业务命令约定。

当前项目目标：

- 已有通用帧封装/解包、CRC 校验、TLV 解析能力。
- 当前业务命令聚焦 `lora_chat`。

---

## 2. 帧格式

### 2.1 固定头

`FrameHead_t`（紧凑排列）字段如下：

| 字节索引 | 字段 | 长度（字节） | 字节序 | 说明 |
|---:|---|---:|---|---|
| 0 | Head | 1 | - | 固定值 `0x4C` |
| 1~2 | Flag | 2 | 小端 | 标志位 |
| 3~4 | Length | 2 | 小端 | 见长度规则 |
| 5~8 | ContentCrc | 4 | 小端 | 内容 CRC32（content crc），用于校验解密后内容区是否可用 |
| 9~12 | TimeStamp | 4 | 小端 | Unix 时间戳 |
| 13~14 | SerialIndex | 2 | 小端 | 递增序号 |
| 15 | CommandId | 1 | - | 命令 ID |

### 2.2 长度规则

协议实现中：

- `total_frame_length = head.length + 11`
- `head.length` 不包含前 9 字节（`Head + Flag + Length + ContentCrc`），也不包含末尾 `CRC16`（2 字节）

等价写法：

- `head.length = TimeStamp(4) + SerialIndex(2) + CommandId(1) + TLV/填充区`

### 2.3 Content CRC（CRC32）

- 字段名：`ContentCrc`
- 位置：紧跟 `Length` 之后（固定头第 5~8 字节）
- 计算范围：内容区字节（即 `TimeStamp` 到 TLV/填充区末尾，不含末尾 `CRC16`）
- 字段用途：在接收端解密后对内容区做 CRC32 校验，防止错误密钥解出伪随机数据并继续解析导致异常

### 2.4 尾部

| 字段 | 长度 | 说明 |
|---|---:|---|
| CRC16 | 2 字节 | 对除最后 2 字节 CRC 之外的全部字节做 `crc16_ccitt` |

CRC 字节按小端内存顺序写入。

---

## 3. Flag 定义（16 位）

`FrameHeadFlag_t` 位分配：

| 位 | 名称 | 说明 |
|---:|---|---|
| 0~11 | `networkId` | 12 位网络标识 |
| 12 | `override` | 预留标志位（本项目当前未使用 anti-replay 语义） |
| 13 | `ack` | 应答帧标志 |
| 14 | `reserved` | 保留位 |
| 15 | `encrypt` | 是否加密 |

---

## 4. TLV 编码

命令字节（`CommandId`）之后是 TLV 序列。

### 4.1 TLV 结构

- `T`：1 字节类型
- `L`：可变长度字段
  - 当 `L <= 127`：占 1 字节
  - 当 `L > 127`：占 2 字节，第一字节最高位为 1（`0x80 | (len >> 8)`），第二字节为低 8 位
- `V`：`L` 字节值

### 4.2 解析约定

- `L = 1/2/4` 时，可直接映射为整数值字段
- 其他长度按字节数组处理
- 若下一个类型字节为 `0xFF`，视为填充结束标记

---

## 5. 加密

当 `flag.encrypt = 1` 时：

- 算法：AES-256-CBC（`ctaes`）
- 明文前缀：前 9 字节（`Head + Flag + Length + ContentCrc`）
- 加密区间：`[9 .. frame_len-3]`
- 加密长度必须是 16 的整数倍
- 采用 `0xFF` 填充后再加密

密钥来源：通过 `DeviceSettingsGetLoraSecretKey()` 取密钥材料并经 SHA-256 派生；IV 使用工程中的固定常量。

---

## 6. 接收校验流程

建议/现行处理顺序：

1. 校验帧头（`head == 0x4C`）
2. 校验长度（`head.length + 11 == frame_len`）
3. 校验 CRC16
4. 校验网络 ID（`frame.networkId == local networkId`）
5. 按需解密（`encrypt = 1` 时）
6. 校验内容 CRC32（对解密后的内容区计算 CRC32，并与 `ContentCrc` 比较）

说明：本文档不再定义 anti-replay 相关规则。

---

## 7. 命令集合

### 7.1 Command ID `0x01`：LoRa Chat（`lora_chat`）

用途：节点之间广播聊天消息。每个节点发送本机聊天内容，网络内其他节点接收后展示。

#### 发送与接收语义

- 发送方式：广播发送（非点对点定向）。
- 业务含义：发送端将当前聊天消息打包为 TLV 并发出。
- 接收行为：接收端解析 `CommandId=0x01` 后，将消息写入聊天记录。

#### TLV Type 定义

| Type | 名称 | 长度 | 说明 |
|---:|---|---:|---|
| `0x01` | Username | 变长（建议 1~32） | 用户名，UTF-8 字节串，不含结尾 `\0` |
| `0x02` | Text | 变长 | 聊天文本内容，UTF-8 字节串，不含结尾 `\0` |
| `0x03` | AvatarColor | 3 | 头像颜色，固定 3 字节 RGB（顺序：R、G、B） |

#### 必填字段约束

- 一个 `lora_chat` 帧必须包含且仅包含一组 `Username`、`Text`、`AvatarColor`。
- 任一必填 Type 缺失时，接收端应丢弃该聊天消息。
- `AvatarColor` 长度不是 3 字节时，接收端应丢弃该聊天消息。

---

## 8. 字节序与逻辑顺序

逻辑顺序：

`Head | Flag(2) | Length(2) | ContentCrc(4) | TimeStamp(4) | SerialIndex(2) | CommandId | TLVs | CRC16(2)`

除 TLV 值本身外，多字节整数字段均为小端。

---

## 9. 实现说明

- 本协议文档命名统一为 LoRa 协议。
- 本项目协议业务命令当前定义为 `lora_chat (Command ID = 0x01)`。
- 历史的继电器控制/读取命令描述已从本文档移除。
