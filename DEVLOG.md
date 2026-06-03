# Smart Car V1 — 開發紀錄

## 2026-06-03 更新

### STM32 韌體修改

| 問題 | 根本原因 | 修復方式 |
|------|----------|----------|
| 避障邏輯誤觸發（F 按久停） | 超聲波未接，PA2 浮空亂讀 → obs_cnt 到 8 進入避障 | 加 `avoidance_off` 旗標測試後移除，恢復正常邏輯 |
| IR 感測器浮空假觸發 | PC13 浮空讀 LOW，立即 obs_cnt=8 | 暫時 comment out IR 檢查 |
| L/R 停止輪仍在轉 | MOTOR_STOP 用 coast 模式（IN1=IN2=LOW, ENA=LOW），輪子靠慣性滑行 | 嘗試主動剎車（IN1=IN2=HIGH）但引發新 bug，最終還原 coast |
| Telemetry BLE 封包超過 20 bytes | `$STATUS,100,90,300,1#\r\n` = 24 bytes > HM-10 MTU | 改短格式 `S,%u,%u,%u,%u\n`（最長 15 bytes）|
| 超聲波距離一直為 0 | TIM2 counter 可能未正確計時，t2-t1=0 | 改用 DWT（CPU cycle counter）計時，不依賴 TIM2 |
| 超聲波 Trig 脈衝不穩定 | 使用 TIM2 counter 計 10µs，TIM2 若未啟動則死鎖 | 改用 `HAL_Delay(1)` + 前置 LOW 2ms 確保乾淨觸發 |
| Echo PIN 衝突 | PA2 同時是 TIM2_CH3 / USART2_TX（ST-Link）| 改用 PC2（無衝突） |
| 避障距離門檻太窄 | 僅偵測 < 20cm，容易漏觸發 | 放寬至 < 40cm |

### Flutter App 修改

- **Telemetry regex 更新**：`\$STATUS,...#` → `S,(\d+),(\d+),(\d+),(\d+)` 配合新格式

### 驗證確認

- ✅ F / B 雙輪方向正確，長按持續動
- ✅ L / R pivot turn（一輪全速，一輪停）
- ✅ 避障邏輯正確（dist=10 強制測試通過）：F 觸發 → 停 → 後退 → 轉向 → 確認
- ✅ B 後退不觸發避障（正確設計）
- ✅ Telemetry 格式在 BLE MTU 內

---

## 2026-06-02 修改

### STM32 韌體（Smart_Car_V1）

| 問題 | 根本原因 | 修復方式 |
|------|----------|----------|
| BLE 指令收不到 | `HAL_UART_RxCpltCallback` 已在 `stm32f4xx_it.c` 定義，誤重複加到 `main.c` | 移除重複定義 |
| 馬達完全不動 | L298N 電源誤接 GND | 接正確電源 |
| F/B 方向相反 | 馬達實體接線導致 IN1/IN2 邏輯反向 | 交換 FORWARD/BACKWARD GPIO |
| L/R 無法轉向 | 轉速比 30% 低於馬達起步力矩 | 改為 pivot turn |

---

## 功能完成狀態

### ✅ 已完成

- **BLE 遙控**：HM-10（MLT-BT05）雙向通訊正常
- **F / B**：雙輪同步，方向正確
- **L / R**：pivot turn，左右各一組馬達
- **停止（S）**：隨時生效
- **避障邏輯**：9 狀態機完整（BRAKE → BACKUP → TURN → CHECK）
- **Telemetry**：每 300ms 傳 `S,speed,angle,dist,obs\n`，BLE 封包 ≤ 15 bytes
- **Flutter App**：BLE 掃描、連線、指令、距離顯示

### ⚠️ 待解決

| 項目 | 狀態 | 備註 |
|------|------|------|
| HC-SR04 距離讀值 | ❌ 未解決 | 5V 電源確認、PA2→PC2 換腳均無效；ECHO 訊號未到 STM32，疑模組故障或接線接觸不良 |
| IR 感測器 | ⚠️ 暫停 | Comment out 中，待確認接線後重新啟用 |
| iOS 測試 | ❌ 待辦 | 需要 Mac |

---

## 硬體接線

### L298N ↔ STM32
| L298N | STM32 | 功能 |
|-------|-------|------|
| IN1 | PE5 | 左輪方向 |
| IN2 | PE6 | 左輪方向 |
| ENA | PE9（拔跳帽）| 左輪 PWM |
| IN3 | PE11 | 右輪方向 |
| IN4 | PE13 | 右輪方向 |
| ENB | PE14（拔跳帽）| 右輪 PWM |

### HM-10 ↔ STM32
| HM-10 | STM32 | 功能 |
|-------|-------|------|
| TX | PC7 | BLE → STM32 |
| RX | PC6 | STM32 → BLE |
| VCC | 3.3V | |
| GND | GND | |

### HC-SR04 ↔ STM32
| HC-SR04 | STM32 | 功能 |
|---------|-------|------|
| VCC | **5V** | 必須 5V |
| TRIG | PD13 | 觸發脈衝（共用橘色 LED）|
| ECHO | PC2 | 回波輸入 |
| GND | GND | |
