# STM32CubeIDE 外設配置清單 - STM32F407 Discovery

## ⚠️ 重要提示

**PA9 (UART1 TX) 在 STM32F407 Discovery 板上有 4.7µF 電容連接，會影響高速 UART 信號。**
- 如果要用 UART1 + PA9/PA10，需要移除該電容或改用低波特率 (≤9600)
- **推薦方案**：使用 **UART6 (PC6/PC7)** ✅（無電容干擾）

---

## 1️⃣ 時鐘配置（System Clock）

| 設定項 | 數值 | 說明 |
|-------|------|------|
| **HSE（外部晶振）** | 8 MHz | Discovery 板上已提供 |
| **PLL 倍頻** | x42 | 8 MHz × 42 = 336 MHz → ÷2 = 168 MHz |
| **系統時鐘（SYSCLK）** | 168 MHz | 最大工作頻率 |
| **AHB 分頻器** | 1 | 168 MHz（用於 AHB 總線） |
| **APB1 分頻器** | 4 | 168 ÷ 4 = 42 MHz（UART、I2C、Timer 2~7、12~14） |
| **APB2 分頻器** | 2 | 168 ÷ 2 = 84 MHz（SPI、Timer 1、8~11） |

**CubeIDE 設定路徑**：
1. 開啟 `.ioc` 檔
2. 左側面板 → System Core → RCC
3. 配置如上

---

## 2️⃣ GPIO 快速對應表

| 功能 | 腳位 | 模式 | 說明 |
|------|------|------|------|
| **紅外線感測器** | PC13 | GPIO_Input | 數位讀取（HIGH=無障礙, LOW=有障礙） |
| **HC-SR04 Trig** | PD13 | GPIO_Output | 觸發脈衝（≥10µs） |
| **馬達方向控制 IN1** | PE5 | GPIO_Output | L298N 前馬達方向 A |
| **馬達方向控制 IN2** | PE6 | GPIO_Output | L298N 前馬達方向 B |
| **馬達方向控制 IN3** | PE11 | GPIO_Output | L298N 後馬達方向 A |
| **馬達方向控制 IN4** | PE13 | GPIO_Output | L298N 後馬達方向 B |

**CubeIDE 設定**：
1. 點擊 GPIO 腳位（例如 PC13）
2. 下拉選單選 "GPIO_Input" 或 "GPIO_Output"
3. 設定 Pull-up / Pull-down（通常無需）

---

## 3️⃣ UART6（HC-05 藍牙）✅ 推薦

| 設定項 | 數值 | 備註 |
|-------|------|------|
| **UART 外設** | USART6 | 不受 Discovery 板干擾 |
| **TX 腳** | **PC6** | 連接 HC-05 RX（需要分壓 1kΩ） |
| **RX 腳** | **PC7** | 連接 HC-05 TX（直接相容 3.3V） |
| **Baud Rate** | 9600 | HC-05 預設 |
| **Data Bits** | 8 | 標準 |
| **Stop Bits** | 1 | 標準 |
| **Parity** | None | 無校驗 |
| **Flow Control** | None | 無硬體流控 |
| **中斷優先級** | 2 | 高優先級（接收命令優先） |

**CubeIDE 設定步驟**：
1. System Core → USART6
2. 「Mode」→ Asynchronous
3. 設定參數如上表
4. NVIC 設定 → USART6 global interrupt 勾選 ✓
5. NVIC Priority → Preemption Priority = 2, Sub Priority = 0

**電路圖**（分壓電阻）：
```
STM32 PC6 (TX) ──── 1kΩ ──── HC-05 RX
                      │
                      └─── 1kΩ ──── GND
                      
HC-05 TX ──────────── STM32 PC7 (RX)   [直接相連，無需分壓]
```

---

## 4️⃣ I2C1（OLED 顯示）

| 設定項 | 數值 | 備註 |
|-------|------|------|
| **I2C 外設** | I2C1 | 標準 I2C 總線 |
| **SCL 腳** | **PB6** | I2C 時鐘線 |
| **SDA 腳** | **PB7** | I2C 數據線 |
| **I2C 速度模式** | Standard (100 kHz) | 推薦，穩定性佳 |
| **時鐘源** | APB1 = 42 MHz | 自動計算 |
| **中斷** | 可選（輪詢足夠） | - |

**CubeIDE 設定步驟**：
1. System Core → I2C1
2. 「Mode」→ I2C
3. 參數設定：
   - Speed = 100 kHz (Standard mode)
   - Duty Cycle = 2:1（或 16:9，自動選擇）
4. GPIO 設定：
   - PB6 → Alternate Function Open Drain（GPIO_AF4_I2C1）
   - PB7 → Alternate Function Open Drain（GPIO_AF4_I2C1）
   - Pull-up = Internal Pull-up
5. NVIC 設定（可選）：
   - I2C1 event interrupt
   - I2C1 error interrupt

**硬體連線**：
```
STM32 PB6 ──── 10kΩ pull-up ──── 3.3V  ──── OLED SCL
STM32 PB7 ──── 10kΩ pull-up ──── 3.3V  ──── OLED SDA

OLED GND ──────────────────────────────────── STM32 GND
OLED VCC ──────────────────────────────────── 3.3V
```

---

## 5️⃣ TIM1（PWM 輸出：馬達速度控制）

### 配置 1：馬達速度 PWM（20 kHz）

| 設定項 | 數值 | 計算方法 |
|-------|------|---------|
| **Timer 外設** | TIM1 | 進階 Timer |
| **時鐘源** | Internal Clock | APB2 Timer = 168 MHz（APB2 = 84 MHz × 2）|
| **模式** | PWM generation (CH1, CH4) | 雙通道 PWM |
| **預分頻 (PSC)** | **0** | 不分頻，Timer 直接用 168 MHz |
| **自動重裝 (ARR)** | **8399** | 頻率 = 168M / (0+1) / (8399+1) = 20 kHz |
| **CH1 （PE9）** | PWM Mode 1 | 馬達前馬達 EN 腳 |
| **CH4 （PE14）** | PWM Mode 1 | 馬達後馬達 EN 腳 |
| **比較值 CCR1/CCR4 範圍** | 0 ~ 8399 | 對應 0% ~ 100% |

**計算說明**（20 kHz）：
```
TIM1 時鐘 = 168 MHz（APB2 Timer，因 APB2 分頻=/2，所以 ×2）
PSC = 0（無分頻）
ARR = 8399（168M ÷ 20k = 8400）
頻率 = 168,000,000 / (8399+1) = 20,000 Hz = 20 kHz ✓
```

**CubeIDE 設定步驟**：
1. Timers → TIM1
2. 「Mode」→ Combined PWM
3. 參數設定：
   - Prescaler (PSC) = 0
   - Auto Reload Register (ARR) = 8399
   - **CH1** (PE9):
     - PWM Generation = PWM Mode 1
     - Pulse (CCR1) = 4200（預設 50%）
   - **CH4** (PE14):
     - PWM Generation = PWM Mode 1
     - Pulse (CCR4) = 4200（預設 50%）
4. NVIC → TIM1 update interrupt（可選，用於定時任務）

**CubeIDE 視覺配置**：
1. 在 `.ioc` 檔中，點擊 PE9 腳
2. 下拉選單 → TIM1_CH1
3. 同理 PE14 → TIM1_CH4

---

## 6️⃣ TIM3（PWM 輸出：伺服馬達 50Hz）

| 設定項 | 數值 | 計算方法 |
|-------|------|---------|
| **Timer 外設** | TIM3 | 通用 Timer |
| **時鐘源** | Internal Clock | APB1 Timer = 84 MHz（APB1 = 42 MHz × 2）|
| **模式** | PWM generation (CH1) | 單通道 PWM |
| **預分頻 (PSC)** | **839** | PSC = (84,000,000 / 100,000) - 1 = 839 |
| **自動重裝 (ARR)** | **1999** | ARR = 2000 - 1 = 1999 |
| **CH1 (PA6)** | PWM Mode 1 | SG90 伺服馬達 signal 腳 |
| **比較值 CCR1 範圍** | 50 ~ 250 | 對應 0° ~ 180° |

**脈寬映射**（20ms 週期）：
```
0°   → CCR = 50  (0.5ms)   → 脈寬 / 週期 = 0.5 / 20 = 2.5% duty
90°  → CCR = 150 (1.5ms)   → 脈寬 / 週期 = 1.5 / 20 = 7.5% duty
180° → CCR = 250 (2.5ms)   → 脈寬 / 週期 = 2.5 / 20 = 12.5% duty
```

**驗證計算**：
- 時鐘：84 MHz（APB1 Timer = 42 MHz × 2）
- PSC = 839 → Timer 頻率 = 84M / 840 = 100 kHz
- ARR = 1999 → PWM 週期 = 100k / 2000 = 50 Hz（20ms）✓
- CCR = 50 → 脈寬 = 50 / 2000 × 20ms = 0.5ms ✓

**CubeIDE 設定步驟**：
1. Timers → TIM3
2. 「Mode」→ PWM Generation
3. 參數設定：
   - Prescaler (PSC) = 839
   - Auto Reload Register (ARR) = 1999
   - **CH1** (PA6):
     - PWM Generation = PWM Mode 1
     - Pulse (CCR1) = 150（預設 90°）
4. GPIO 設定：
   - PA6 → Alternate Function (GPIO_AF2_TIM3)

---

## 7️⃣ TIM2（Input Capture：超聲波 HC-SR04）

| 設定項 | 數值 | 備註 |
|-------|------|------|
| **Timer 外設** | TIM2 | 通用 Timer（32 位） |
| **時鐘源** | Internal Clock | APB1 Timer = 84 MHz（APB1 = 42 MHz × 2）|
| **模式** | Input Capture | 測量脈寬 |
| **預分頻 (PSC)** | **83** | PSC = (84M / 1M) - 1 = 83（時鐘 = 1 MHz）|
| **CH3 (PA2)** | Input Capture | HC-SR04 Echo 腳 |
| **捕捉邊沿** | Rising & Falling | 上升邊 + 下降邊 |
| **中斷優先級** | 3 | 次高優先級 |

**距離計算**：
```
Echo 脈寬（µs）= CCR 計數差
距離（cm）= 脈寬（µs）/ 58
（聲速 = 343 m/s，往返 ÷2）
```

**CubeIDE 設定步驟**：
1. Timers → TIM2
2. 「Mode」→ Input Capture direct (CH3)
3. 參數設定：
   - Prescaler (PSC) = 83
   - Counter Period = 0xFFFFFFFF（32 位最大）
   - **CH3** (PA2):
     - Input Capture = Input Capture direct
     - Capture Polarity = Rising & Falling
4. NVIC 設定：
   - TIM2 global interrupt ✓
   - Priority = 3

**GPIO 設定**：
- PA2→ Alternate Function (GPIO_AF1_TIM2)

---

## 8️⃣ SPI（可選：邏輯分析儀調試）

如果需要用邏輯分析儀監測信號，連接到以下腳：
- MISO/MOSI（未使用，可跳過）
- SCK（用作時序參考，可選）

通常無需在 CubeIDE 中專門配置（邏輯分析儀只是被動監聽）。

---

## 9️⃣ 中斷優先級分配（NVIC）

| 中斷源 | 優先級 | 說明 |
|-------|-------|------|
| USART6 (UART RX) | **2** | 最高（接收藍牙命令優先） |
| TIM2 Capture (超聲波) | **3** | 次高（測距定時） |
| TIM1 Update | 4 | 低 |
| SysTick | 15 | 最低（系統 tick） |

**CubeIDE 設定路徑**：
1. System Core → NVIC
2. 表格中勾選中斷
3. Priority 欄設定 Preemption Priority

---

## 🔟 完整檔案樹（CubeIDE 生成後）

```
YourProject/
├── Core/
│   ├── Inc/
│   │   ├── main.h
│   │   ├── stm32f4xx_it.h       ← 中斷處理
│   │   └── stm32f4xx_hal_conf.h ← HAL 配置
│   │
│   └── Src/
│       ├── main.c
│       ├── stm32f4xx_it.c       ← UART6, TIM2, TIM1 中斷回調
│       └── syscalls.c
│
├── Drivers/
│   ├── CMSIS/                    ← STM32 核心文件（自動生成）
│   └── STM32F4xx_HAL_Driver/    ← HAL 庫（自動生成）
│
├── .ioc                          ← CubeIDE 圖形配置檔（重要！）
├── CMakeLists.txt / Makefile     ← 編譯配置
└── STM32F407VGTx_FLASH.ld      ← 連接指令稿
```

---

## 1️⃣1️⃣ STM32CubeIDE 生成代碼後的修改清單

### a) main.c - HAL 初始化

生成後的 main.c 會自動包含：
```c
int main(void) {
    HAL_Init();                          // HAL 初始化
    SystemClock_Config();                // 時鐘配置
    
    MX_GPIO_Init();                      // GPIO 初始化
    MX_USART6_UART_Init();              // UART6 初始化
    MX_I2C1_Init();                     // I2C1 初始化
    MX_TIM1_Init();                     // TIM1 初始化
    MX_TIM2_Init();                     // TIM2 初始化
    MX_TIM3_Init();                     // TIM3 初始化
    
    // 啟動外設
    HAL_UART_Receive_IT(&huart6, &rx_buffer, 1);  // UART6 中斷接收
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);     // TIM1 CH1 PWM
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);     // TIM1 CH4 PWM
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);     // TIM3 CH1 PWM
    HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_3);   // TIM2 CH3 Input Capture
    
    while (1) {
        // 你的代碼放這裡
    }
}
```

✅ **CubeIDE 自動生成，無需手動修改**

### b) stm32f4xx_it.c - 中斷回調

需要添加以下回調函數（CubeIDE 已框架，填空即可）：

```c
// UART6 接收完成中斷
void USART6_IRQHandler(void) {
    HAL_UART_IRQHandler(&huart6);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART6) {
        // 處理接收的藍牙命令
        // 重新啟動接收
        HAL_UART_Receive_IT(&huart6, &rx_buffer, 1);
    }
}

// TIM2 輸入捕捉中斷
void TIM2_IRQHandler(void) {
    HAL_TIM_IRQHandler(&htim2);
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3) {
        // 計算 HC-SR04 回波脈寬
        uint32_t pulse_width = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3);
        uint16_t distance = pulse_width / 58;  // cm
    }
}
```

---

## 1️⃣2️⃣ CubeIDE 配置檢查清單

開啟 `.ioc` 檔後，按順序檢查：

### System Core
- [ ] RCC: HSE = 8 MHz, PLLx42, SysClk = 168 MHz
- [ ] AHB: 分頻 1（168 MHz）
- [ ] APB1: 分頻 4（42 MHz）
- [ ] APB2: 分頻 2（84 MHz）
- [ ] NVIC: 中斷優先級設定正確

### Connectivity (左側面板)
- [ ] USART6:
  - [ ] Mode = Asynchronous
  - [ ] Baud = 9600
  - [ ] PC6 (TX), PC7 (RX) 設定為 Alternate Function
  - [ ] 中斷 Priority = 2
  
- [ ] I2C1:
  - [ ] Mode = I2C
  - [ ] Speed = 100 kHz
  - [ ] PB6 (SCL), PB7 (SDA) 設定為 Open Drain
  - [ ] Pull-up = Internal
  
### Timers
- [ ] TIM1:
  - [ ] PSC = 0, ARR = 8399
  - [ ] PE9 (CH1), PE14 (CH4) → PWM Mode 1

- [ ] TIM3:
  - [ ] PSC = 839, ARR = 1999
  - [ ] PA6 (CH1) → PWM Mode 1, CCR1 = 150

- [ ] TIM2:
  - [ ] PSC = 83, ARR = 0xFFFFFFFF
  - [ ] PA2 (CH3) → Input Capture (Rising & Falling)
  - [ ] 中斷 Priority = 3

### GPIO
- [ ] PC13 (IR Sensor) → GPIO_Input
- [ ] PD13 (HC-SR04 Trig) → GPIO_Output
- [ ] PE5, PE6, PE11, PE13 (馬達方向) → GPIO_Output

### Project Settings
- [ ] Compiler Optimizations: -O2（平衡速度與大小）
- [ ] HAL Library: Enable
- [ ] Code Generation: ✓ Generate peripheral initialization as separate files（可選）

---

## 1️⃣3️⃣ 驗證清單（編譯前）

```bash
✓ 所有必要外設已配置
✓ 時鐘設定正確（SystemClock_Config 生成）
✓ GPIO 腳位無衝突
✓ 中斷優先級已設定（NVIC）
✓ .ioc 檔已保存
```

**編譯命令**：
```bash
# STM32CubeIDE 內建編譯，或命令行：
arm-none-eabi-gcc -c main.c -o main.o ...
arm-none-eabi-gcc *.o -o firmware.elf ...
arm-none-eabi-objcopy -O binary firmware.elf firmware.bin
```

**燒入命令**（使用 st-link）：
```bash
st-flash write firmware.bin 0x08000000
```

---

## 1️⃣4️⃣ 常見配置錯誤排查

| 問題 | 原因 | 解決方案 |
|------|------|---------|
| UART 無法接收 | PC6/PC7 未設定為 Alternate Function | 檢查 `.ioc` GPIO 配置，手動設定 AF |
| OLED 無顯示 | I2C 時鐘 > 400 kHz 或上拉不足 | 降到 100 kHz，確認 10kΩ 上拉電阻 |
| PWM 信號異常 | PSC/ARR 計算錯誤 | 重新驗證頻率計算，檢查 CubeIDE 數值 |
| 超聲波無反應 | TIM2 Input Capture 邊沿配置錯誤 | 改為 "Rising & Falling" 雙邊沿 |
| 編譯失敗 | HAL 庫缺失 | Regenerate Code，確保 `STM32F4xx_HAL_Driver` 包含 |

---

## 1️⃣5️⃣ 推薦工作流程

1. **第一步**：完成上述 CubeIDE 配置
2. **第二步**：生成代碼（Project → Generate Code）
3. **第三步**：測試基礎外設：
   - LED 閃爍（GPIO）
   - UART 迴環（發送 → 接收）
   - I2C 掃描（檢測 OLED 0x3C 地址）
   - PWM 波形（用邏輯分析儀確認）
4. **第四步**：VS Code + Claude Code 開發業務邏輯層（馬達驅動、藍牙協議等）

---

**文檔版本**：v1.0  
**更新時間**：2026-03-31  
**硬體平台**：STM32F407VG Discovery  
**開發環境**：STM32CubeIDE v1.x + VS Code
