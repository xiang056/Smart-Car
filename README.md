# Smart Car V1 — STM32F407 BLE 遙控小車

STM32F407VG Discovery Board 裸機開發的藍牙遙控小車，搭配 Flutter App 進行雙向通訊與即時狀態顯示。

---

## 硬體規格

| 元件 | 型號 | 說明 |
|------|------|------|
| 主控板 | STM32F407VG Discovery | 168 MHz Cortex-M4 |
| 馬達驅動 | L298N | 雙 H 橋，PWM 差速控制 |
| 藍牙模組 | HM-10 (MLT-BT05) | BLE 4.0，UART 9600 bps |
| 電源 | 外部 5V | L298N + HM-10 供電 |

### 接線

**L298N ↔ STM32**

| L298N | STM32 | 功能 |
|-------|-------|------|
| IN1 | PE5 | 左輪方向 A |
| IN2 | PE6 | 左輪方向 B |
| ENA | PE9 | 左輪 PWM（TIM1_CH1，拔跳帽） |
| IN3 | PE11 | 右輪方向 A |
| IN4 | PE13 | 右輪方向 B |
| ENB | PE14 | 右輪 PWM（TIM1_CH4，拔跳帽） |

**HM-10 ↔ STM32**

| HM-10 | STM32 | 功能 |
|-------|-------|------|
| TX | PC7 | USART6_RX |
| RX | PC6 | USART6_TX |
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
    ├── main.c            # 9 狀態機主迴圈
    ├── motor_driver.c    # TIM1 PWM 雙輪控制
    ├── bluetooth.c       # UART 中斷接收 + telemetry 發送
    └── stm32f4xx_it.c    # 中斷向量（UART RxCplt / Error）
```

### 開發環境
- **STM32CubeIDE** — HAL 初始化、周邊配置、編譯燒錄
- **VS Code** — 業務邏輯開發
- **純裸機（無 RTOS）**

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

---

## 狀態機

小車共 9 個狀態，**L / R 指令根據當前狀態自動切換差速或原地轉向**：

| state | 名稱 | 左輪 | 右輪 |
|-------|------|------|------|
| 0 | STOP | 停 | 停 |
| 1 | FORWARD | 全速前進 | 全速前進 |
| 2 | BACKWARD | 全速後退 | 全速後退 |
| 3 | LEFT | 停 | 全速前進（pivot） |
| 4 | RIGHT | 全速前進 | 停（pivot） |
| 5 | FORWARD_LEFT | 40% 前進 | 100% 前進 |
| 6 | FORWARD_RIGHT | 100% 前進 | 40% 前進 |
| 7 | BACKWARD_LEFT | 40% 後退 | 100% 後退 |
| 8 | BACKWARD_RIGHT | 100% 後退 | 40% 後退 |

> 行進中收到 L/R → 差速曲線轉向（內輪 40%）  
> 靜止時收到 L/R → 原地 pivot 轉向

---

## 周邊配置（STM32CubeIDE）

| 周邊 | 用途 | 參數 |
|------|------|------|
| TIM1 | 馬達 PWM | APB2 84 MHz，PSC=0，ARR=8399 → 10 kHz |
| TIM2 | Input Capture（保留） | — |
| TIM3 | 保留 | — |
| USART6 | HM-10 BLE | 9600 bps，8N1，RX 中斷 |
| I2C1 | 保留 | 100 kHz |
| GPIO PE5/6/11/13 | 馬達方向 | Output Push-Pull |
| GPIO PD13 | 保留 | — |

---

## 開發紀錄

詳細 debug 過程與設計決策見 [DEVLOG.md](DEVLOG.md)。

### 重要設計取捨

- **HC-SR04 超聲波避障**：已完整實作並驗證（BRAKE → BACKUP → TURN → CHECK 四段狀態機），因車體空間限制從最終版移除
- **差速轉向**：行進中轉向採內輪減速（40%）而非原地 pivot，提升操控流暢度

---

## 配套 Flutter App

[smart-car-app](https://github.com/xiang056/smart-car-app) — BLE 掃描、連線、方向控制、即時狀態顯示
