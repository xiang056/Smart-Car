# STM32F407 Discovery 智能小車專案規劃

## 📋 項目概述

| 項目 | 詳情 |
|------|------|
| **硬體平台** | STM32F407VG Discovery Board |
| **核心功能** | 藍牙遠端控制 + 實時顯示 |
| **開發環境** | STM32CubeIDE (硬體配置+編譯) + VS Code (業務邏輯開發) |
| **開發模式** | 純裸機 (無 RTOS) |
| **目標交付** | 分階段迭代，每個功能獨立驗證 |

---

## 🔌 硬體連接規劃

### 1. 完整 PIN 分配表

> **硬體變更紀錄（2026-06-04）**：HC-05 改為 HM-10 BLE；HC-SR04 與紅外線感測器因車體空間不足取消。

| 功能模組 | 連接器 | STM32 GPIO | 通訊協議 | 說明 |
|---------|--------|-----------|--------|------|
| **HM-10 BLE** | TX → PC7 | USART6_RX | UART 9600bps | 接收手機命令 |
| | RX → PC6 | USART6_TX | | 發送狀態反饋 |
| | VCC | 3.3V | | 直接接 STM32 |
| | GND | GND | | 共地 |
| **OLED I2C** | SDA | PB7 | I2C1_SDA | 顯示速度/方向/狀態 |
| | SCL | PB6 | I2C1_SCL | |
| | VCC | 3.3V | | 直接接 STM32 |
| | GND | GND | | |
| **L298N 馬達驅動** | IN1 (左輪) | PE5 | GPIO | 方向控制 |
| | IN2 (左輪) | PE6 | GPIO | |
| | ENA (左輪) | PE9 | TIM1_CH1 PWM | 速度控制（拔跳帽） |
| | IN3 (右輪) | PE11 | GPIO | 方向控制 |
| | IN4 (右輪) | PE13 | GPIO | |
| | ENB (右輪) | PE14 | TIM1_CH4 PWM | 速度控制（拔跳帽） |
| | GND | GND | | 共地 |
| **SG90 伺服馬達** | Signal | PA6 | TIM3_CH1 PWM | 50Hz，脈寬 0.5~2.5ms |
| | VCC | 5V | | |
| | GND | GND | | |

### 2. 電源分配

```
STM32F407 Discovery（內部）
├─ 3.3V 邏輯電源（STM32 內核 + OLED）
├─ 5V 輔助電源（經由 ST-Link 或外部電源）
│
外部電源轉換模組
├─ 輸入：電池或外部 5V
├─ 輸出 5V：HC-05、L298N、SG90、HC-SR04、紅外線感測器
└─ 輸出 3.3V：OLED（如果需要降壓）
```

### 3. 關鍵連線注意

#### ⚠️ HM-10 BLE 電源
HM-10 為 3.3V 模組，直接接 STM32 3.3V 腳，TX/RX 電壓相容，無需分壓。

#### ⚠️ L298N 電源隔離
- L298N 有獨立的馬達電源腳（+12V/+5V）和邏輯電源腳（+5V GND）
- **必須共地**（STM32 GND = L298N GND = 電池 GND）
- GPIO 控制線直接接 L298N 邏輯腳

#### ⚠️ SG90 PWM 頻率
- 50Hz（週期 20ms）
- 0.5ms → 0° | 1.5ms → 90° | 2.5ms → 180°
- 使用 STM32 Timer 的 PWM 輸出模式

---

## 🏗️ 軟體架構規劃

### 目錄結構
```
SmartCar/
├── Core/
│   ├── Inc/
│   │   ├── main.h
│   │   ├── motor_driver.h        # 馬達驅動層
│   │   ├── servo_driver.h        # 伺服馬達驅動
│   │   ├── oled_driver.h         # OLED 顯示驅動（待實作）
│   │   └── bluetooth.h           # BLE 通訊協議層
│   └── Src/
│       ├── main.c                # 狀態機主迴圈
│       ├── motor_driver.c
│       ├── servo_driver.c
│       ├── oled_driver.c         # 待實作
│       └── bluetooth.c
│
├── Drivers/
│   ├── CMSIS/                    # STM32 CMSIS 核心文件（CubeIDE 生成）
│   └── STM32F4xx_HAL_Driver/    # STM32F4 HAL 庫（CubeIDE 生成）
│
└── .ioc                          # STM32CubeIDE 配置文件
```

### 模塊化設計原則

| 模塊 | 責任 | 狀態 |
|------|------|------|
| **Motor Driver** | PWM 速度/方向控制（左右輪獨立） | ✅ 完成 |
| **Servo Driver** | TIM3 50Hz PWM 角度映射（0~180°） | ✅ 完成（待接入主迴圈） |
| **Bluetooth** | HM-10 BLE 命令解析 + telemetry | ✅ 完成 |
| **OLED Driver** | SSD1306 I2C 初始化、文字/數字顯示 | ❌ 待實作 |
| **State Machine** | main.c 主迴圈，F/B/L/R/S 控制 | ✅ 完成 |

---

## 🎮 控制協議（藍牙 Bluetooth）

### 命令格式
接收單個字符，即時執行：

| 命令 | 功能 | PWM 速度 | 伺服角度 |
|------|------|---------|---------|
| `F` | 前進 | 100% | 90°（直行） |
| `B` | 後退 | 100% | 90°（直行） |
| `L` | 左轉 | 50% | 45°（左） |
| `R` | 右轉 | 50% | 135°（右） |
| `S` | 停止 | 0% | 90°（直行） |
| `?` | 查詢狀態 | - | - |

### 狀態反饋格式（可選，後續實現）
```
格式：$STATUS,SPEED,ANGLE,DISTANCE,OBSTACLE#
例：$STATUS,80,90,45,0#   （速度 80%, 角度 90°, 距離 45cm, 無障礙）
```

---

## 🔧 STM32CubeIDE 配置清單

### 必需配置項

#### 時鐘設定
- **系統時鐘**：168 MHz（STM32F407 最大頻率）
- **AHB 分頻**：1（168 MHz）
- **APB1 分頻**：4（42 MHz，用於 UART、I2C、Timer）
- **APB2 分頻**：2（84 MHz，用於 SPI、高速外設）

#### UART1（HC-05 通訊）
- **Baud Rate**：9600 bps
- **Data Bits**：8
- **Stop Bits**：1
- **Parity**：None
- **Flow Control**：None
- **GPIO**：PA9(TX), PA10(RX)
- **中斷**：啟用 RX 中斷（用於接收命令）

#### I2C1（OLED 顯示）
- **Speed Mode**：Standard（100 kHz）或 Fast（400 kHz）
- **GPIO**：PB6(SCL), PB7(SDA)
- **時鐘頻率**：42 MHz（來自 APB1）
- **中斷**：可選（輪詢模式即可）

#### TIM1（PWM 輸出：馬達 + 伺服）
- **時鐘源**：内部時鐘（84 MHz from APB2）
- **預分頻（PSC）**：計算見下
- **自動重裝（ARR）**：見下
- **通道配置**：
  - CH1（PE9）：馬達速度 PWM（頻率 20 kHz）
  - CH4（PE14）：馬達速度 PWM（頻率 20 kHz）
  - 另有 CH2/CH3 可用於伺服（需要單獨配置為 50Hz）

**PWM 頻率計算**（馬達 20 kHz）：
- 時鐘：84 MHz
- PSC = 1（無分頻）
- ARR = 84,000,000 / (20,000 × 1) - 1 = 4199
- 脈寬範圍：0~4199（對應 0~100%）

#### TIM2（Input Capture：超聲波回波測量）
- **時鐘源**：内部時鐘（84 MHz）
- **通道配置**：PD14(CH3) 設為 Input Capture
- **捕捉邊沿**：上升和下降邊沿
- **中斷**：啟用 Capture 中斷（計算脈寬 = 距離）

#### TIM3（PWM：伺服馬達 50Hz）
- **時鐘源**：内部時鐘（42 MHz from APB1）
- **預分頻（PSC）**：839（42,000,000 / 50,000 = 840）
- **自動重裝（ARR）**：999（脈寬範圍 0~999 對應 0~20ms）
- **通道配置**：PD12(CH1)
- **脈寬映射**：
  - 0° → CCR = 25（0.5ms）
  - 90° → CCR = 75（1.5ms）
  - 180° → CCR = 125（2.5ms）

#### GPIO（紅外線感測器、觸發）
- PC13：輸入，GPIO_Input（IR 感測器）
- PD13：輸出，GPIO_Output（HC-SR04 Trig）

### CubeIDE 生成後的修改
- 在 `main.c` 中初始化外設
- 啟用中斷優先級：
  - UART1_RX：優先級 2（高，即時命令）
  - TIM2_Capture：優先級 3（次高，測距）
  - 其他：優先級 4~7（低）

---

## 📊 狀態機設計（Core Logic）

### 主狀態枚舉
```c
typedef enum {
    STATE_IDLE,           // 待命狀態
    STATE_MOVING_FWD,     // 前進
    STATE_MOVING_BWD,     // 後退
    STATE_TURNING_LEFT,   // 左轉
    STATE_TURNING_RIGHT,  // 右轉
    STATE_AVOIDING,       // 自動避障（後期功能）
    STATE_STOPPED         // 停止
} CarState_t;
```

### 轉態圖
```
[IDLE]
  ↓ (接收命令: F/B/L/R/S)
[MOVING_FWD] ← →[MOVING_BWD]
  ↓ (S 命令)
[STOPPED] → [IDLE]

(同時響應轉向命令)
[MOVING_FWD] → [TURNING_LEFT] / [TURNING_RIGHT] → [MOVING_FWD]
```

### 主循環流程（偽代碼）
```
while(1) {
    // 1. 讀取藍牙命令（UART 中斷服務）
    if (bluetooth_cmd_available) {
        char cmd = bluetooth_cmd;
        process_command(cmd);
    }
    
    // 2. 執行當前狀態邏輯
    switch (current_state) {
        case STATE_MOVING_FWD:
            motor_set_speed(SPEED_MAX);
            servo_set_angle(90);  // 直行
            break;
        // ... 其他狀態
    }
    
    // 3. 讀取感測器數據
    distance = ultrasonic_measure();
    obstacle = ir_sensor_read();
    
    // 4. 更新 OLED 顯示
    oled_display_status(speed, angle, distance, obstacle);
    
    // 5. 延遲以降低 CPU 使用率
    HAL_Delay(50);  // 20Hz 主循環
}
```

---

## 🚀 開發進度規劃（分階段）

### 第 1 階段：硬體驗證 & 基礎驅動（Week 1~2）

| 任務 | 完成條件 | 優先級 |
|------|---------|--------|
| STM32CubeIDE 新建項目 + HAL 庫配置 | 成功編譯 | P0 |
| LED 閃爍測試（驗證系統時鐘） | LED 正常閃爍 1Hz | P0 |
| UART1 初始化 + 迴環測試（HC-05 未連接） | 串口助手可收發字符 | P0 |
| I2C1 初始化 + OLED 地址掃描 | 偵測到 0x3C 地址 | P0 |
| Timer PWM 輸出驗證（馬達 + 伺服） | 邏輯分析儀顯示正確 PWM | P1 |

### 第 2 階段：逐模塊集成（Week 3~4）

| 任務 | 完成條件 | 依賴 |
|------|---------|------|
| Motor Driver 實現（前進/後退/停止） | 馬達按命令動作 | 階段 1 完成 |
| Servo Driver 實現（0°~180° 角度映射） | 伺服馬達轉向正確 | 階段 1 完成 |
| Ultrasonic Driver 實現 | 距離測量誤差 < 2cm | 階段 1 完成 |
| IR Sensor Driver 實現 | 感測器狀態讀取正確 | 階段 1 完成 |
| OLED Driver 實現（顯示文字/數字） | OLED 能顯示速度/角度 | 階段 1 完成 |

### 第 3 階段：藍牙控制集成（Week 5）

| 任務 | 完成條件 |
|------|---------|
| HC-05 與手機配對 | 手機能發現並連接 HC-05 |
| Bluetooth Protocol 實現 | 接收 F/B/L/R/S 命令，馬達響應 |
| 狀態機主循環 | 小車完整動作迴圈 |

### 第 4 階段：測試 & 優化（Week 6）

| 任務 | 完成條件 |
|------|---------|
| 全功能集成測試 | 藍牙控制 + 顯示 + 感測器同時工作 |
| 響應延遲優化 | 命令延遲 < 100ms |
| 電源和發熱測試 | 長時間運行穩定，無異常斷電 |
| 文檔和代碼清理 | 代碼註釋完整，模塊化清晰 |

---

## 🐛 已知風險 & 緩解方案

| 風險 | 影響 | 緩解方案 |
|------|------|---------|
| L298N 發熱過高 | 馬達驅動可靠性降低 | 加散熱器 / 限制 PWM duty cycle |
| HC-SR04 測距誤差 | 避障效果差 | 增加軟體濾波（移動平均），用 3~5 次測量取中位數 |
| 藍牙命令延遲 | 控制不流暢 | 使用中斷接收，避免輪詢 |
| 馬達速度不一致（左右輪） | 小車偏向 | 后期實現差速控制或編碼器反饋 |
| I2C 時序不穩定 | OLED 顯示花屏 | 降低 I2C 時鐘（100 kHz standard mode），確保上拉電阻 10kΩ |

---

## 📝 代碼規範

### 命名習慣
```c
// 模塊前綴 + 功能名稱
void motor_set_speed(uint16_t speed);           // 馬達相關
void servo_set_angle(uint16_t angle);           // 伺服相關
uint16_t ultrasonic_get_distance(void);         // 超聲波相關
uint8_t ir_sensor_read(void);                   // 紅外線相關
void oled_display_string(uint8_t x, uint8_t y, const char *str);  // OLED 相關

// 全局配置宏
#define MOTOR_SPEED_MAX    4199     // PWM ARR 值
#define SERVO_ANGLE_MIN    25       // 0°
#define SERVO_ANGLE_MID    75       // 90°
#define SERVO_ANGLE_MAX    125      // 180°
```

### 註釋標準
```c
/**
 * @brief  初始化馬達驅動
 * @param  speed: PWM 占空比（0~4199，0%~100%）
 * @return HAL_OK 或 HAL_ERROR
 * @note   必須在 main 中優先調用，位於其他模塊之前
 */
HAL_StatusTypeDef motor_init(void);
```

---

## 🔗 相關資源

- **STM32F407 參考手冊**：https://www.st.com/resource/en/reference_manual/dm00031020-stm32f405-415-stm32f407-417-stm32f427-437-and-stm32f429-439-advanced-arm-based-32-bit-mcus-stmicroelectronics.pdf
- **HAL 庫文檔**：STM32CubeIDE 內置 Help
- **SSD1306 OLED 驅動**：https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf
- **HC-05 手冊**：Google 搜索 "HC-05 datasheet PDF"
- **L298N 馬達驅動**：Google 搜索 "L298N datasheet"

---

## ✅ 檢查清單（開發前）

- [ ] STM32CubeIDE 已安裝，新建 STM32F407VG 項目
- [ ] 硬體焊接完成，多表檢查所有連線
- [ ] 邏輯分析儀準備就緒（調試 PWM/UART）
- [ ] 手機藍牙終端 App 已安裝（例：BluetoothTerminal）
- [ ] 代碼倉庫初始化（Git）

---

## 📞 常見問題解決

### Q1: OLED 顯示為空或顯示花屏
- 檢查 I2C 時序（100 kHz，確認上拉電阻 10kΩ）
- 驗證 SSD1306 地址（0x3C 或 0x3D，用掃描程序確認）
- 檢查 OLED 初始化序列（VCC、GND、RST 腳）

### Q2: HC-05 無法收到命令
- 確認藍牙已配對（PIN：1234）
- 檢查 UART 波特率（9600 bps）
- 驗證 RX/TX 接線無誤，考慮接 USB 串口轉換器測試

### Q3: 馬達不動或不聽命令
- 確認 L298N VCC/GND 接線正確
- 用邏輯分析儀查看 PWM 信號是否正確
- 檢查馬達電源電壓（應≥6V，L298N 推薦 12V）
- 試試手動旋轉馬達（排除機械卡頓）

### Q4: 超聲波距離測量不準
- 確認 HC-SR04 工作電壓（5V）
- 檢查 Trig 脈衝寬度（應≥10µs）
- 考慮加軟體濾波（3~5 次測量取中位數）
- 避免測量時有其他超聲波設備干擾

---

**文檔版本**：v1.0  
**最後更新**：2026-03-31  
**維護者**：Edward
