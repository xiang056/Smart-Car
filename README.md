# Smart Car V1 — STM32F407 BLE 遙控小車

STM32F407VG Discovery Board 裸機開發的藍牙遙控小車，搭配 Flutter App 進行雙向通訊與即時狀態顯示。

---

## App 畫面

| STOP | FORWARD | FWD LEFT | BWD RIGHT |
|------|---------|----------|-----------|
| ![stop](docs/Screenshot_2026-06-10-20-58-51-97.png) | ![forward](docs/Screenshot_2026-06-10-20-59-44-26.png) | ![fwd_left](docs/Screenshot_2026-06-10-20-59-03-14.png) | ![bwd_right](docs/Screenshot_2026-06-10-21-00-02-09.png) |

---

## 硬體規格

| 元件 | 型號 | 說明 |
|------|------|------|
| 主控板 | STM32F407VG Discovery | 168 MHz Cortex-M4，裸機（無 RTOS） |
| 馬達驅動 | L298N | 雙 H 橋，PWM 差速控制，5V 供電 |
| 藍牙模組 | HM-10（MLT-BT05） | BLE 4.0，UART 9600 bps，3.3V |
| 馬達 | TT 馬達 × 4 | 左側兩顆並聯 / 右側兩顆並聯 |
| 底盤 | 4 輪壓克力底盤 | |
| 電池 | 6V AA × 4 | L298N VS 供電 |
| 電容 | 100µF 50V | 並聯 L298N VS-GND，抑制啟動電流 |

### 接線

**L298N ↔ STM32**

| L298N | STM32 | 功能 |
|-------|-------|------|
| IN1 | PE5 | 左輪方向 A |
| IN2 | PE6 | 左輪方向 B |
| ENA | PE9（拔跳帽）| 左輪 PWM（TIM1_CH1）|
| IN3 | PE11 | 右輪方向 A |
| IN4 | PE13 | 右輪方向 B |
| ENB | PE14（拔跳帽）| 右輪 PWM（TIM1_CH4）|
| VS | 外部 6V | 馬達電源 |
| VSS | 5V | 邏輯電源 |

**HM-10 ↔ STM32**

| HM-10 | STM32 | 功能 |
|-------|-------|------|
| TX | PC7 | USART6_RX（BLE → STM32 指令）|
| RX | PC6 | USART6_TX（STM32 → BLE telemetry）|
| VCC | 3.3V | |
| GND | GND | |

---

## 軟體架構

```
Core/
├── Inc/
│   ├── motor_driver.h    # PWM 差速驅動介面
│   └── bluetooth.h       # BLE 命令 / telemetry 介面
└── Src/
    ├── main.c            # 9 狀態機主迴圈 + 換向保護
    ├── motor_driver.c    # TIM1 PWM 雙輪獨立控制
    ├── bluetooth.c       # UART 中斷接收 + telemetry 發送
    └── stm32f4xx_it.c    # 中斷向量（UART RxCplt / Error）
```

### 開發環境
- **STM32CubeIDE** — HAL 初始化、周邊配置、編譯燒錄
- **VS Code + Claude Code** — 業務邏輯開發
- **裸機（無 RTOS）**

---

## 控制協議

### 指令（App → STM32，單 ASCII 字元）

| 指令 | 動作 |
|------|------|
| `F` | 前進 |
| `B` | 後退 |
| `L` | 左轉 |
| `R` | 右轉 |
| `S` | 停止 |

### Telemetry（STM32 → App，每 300ms）

```
S,<speed%>,<state>\n
```

| 欄位 | 說明 |
|------|------|
| speed% | 0 = 停止，100 = 行進中 |
| state | 0–8，對應下方狀態表 |

格式最長 `S,100,8\n` = 9 bytes，遠低於 HM-10 BLE MTU 20 bytes。

---

## 狀態機

小車共 **9 個狀態**，L / R 指令根據當前狀態自動切換差速或原地轉向：

| state | 名稱 | 左輪 | 右輪 |
|-------|------|------|------|
| 0 | STOP | 停 | 停 |
| 1 | FORWARD | 全速前進 | 全速前進 |
| 2 | BACKWARD | 全速後退 | 全速後退 |
| 3 | LEFT | 停 | 全速前進（pivot）|
| 4 | RIGHT | 全速前進 | 停（pivot）|
| 5 | FORWARD_LEFT | 60% 前進 | 100% 前進 |
| 6 | FORWARD_RIGHT | 100% 前進 | 60% 前進 |
| 7 | BACKWARD_LEFT | 60% 後退 | 100% 後退 |
| 8 | BACKWARD_RIGHT | 100% 後退 | 60% 後退 |

**轉向邏輯：**
- 行進中收到 L/R → 差速曲線轉向（內輪 60%）
- 靜止時收到 L/R → 原地 pivot 轉向（一輪停，一輪全速）
- 反向 pivot 瞬切（LEFT 時收到 R）→ 先停止再轉向

**換向保護：**
- FORWARD ↔ BACKWARD 切換時自動插入 100ms 停止，防止馬達反轉瞬間電流過大卡住

---

## 周邊配置

| 周邊 | 用途 | 參數 |
|------|------|------|
| TIM1 | 馬達 PWM | APB2 168 MHz，PSC=0，ARR=33599 → **5 kHz** |
| USART6 | HM-10 BLE | 9600 bps，8N1，RX 中斷驅動 |
| GPIO PE5/6/11/13 | 馬達方向 | Output Push-Pull |

> PWM 頻率從 20 kHz 降至 5 kHz，L298N 切換損耗降低，馬達有效電壓提升。

---

## Flutter App

[smart-car-app](https://github.com/xiang056/smart-car-app)

- BLE 掃描、連線、方向控制
- 狀態卡片即時顯示 9 個行駛狀態（含顏色與圖示）
- `withoutResponse: true` 寫入，降低控制延遲
- Android 與 iOS（需 Mac 編譯）

---

## 開發紀錄

詳細 debug 過程與設計決策見 [DEVLOG.md](DEVLOG.md)。

### 重要設計取捨

| 功能 | 決策 | 原因 |
|------|------|------|
| HC-SR04 超聲波避障 | 移除 | 車體空間不足；避障邏輯已實作驗證（BRAKE→BACKUP→TURN→CHECK） |
| IR 感測器 | 移除 | 車體空間不足 |
| SG90 舵機 | 移除 | 差速轉向已足夠，舵機無實質作用 |
| HC-05 → HM-10 | 改用 HM-10 | BLE 相容 iOS，3.3V 直接相容 |
