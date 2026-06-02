# Smart Car V1 — 開發紀錄

## 今日修改（2026-06-02）

### STM32 韌體（Smart_Car_V1）

#### 問題診斷與修復

| 問題 | 根本原因 | 修復方式 |
|------|----------|----------|
| BLE 指令收不到 | `HAL_UART_RxCpltCallback` / `HAL_UART_ErrorCallback` 已在 `stm32f4xx_it.c` 定義，誤重複加到 `main.c` | 移除 `main.c` 重複定義 |
| Telemetry 傳不回 App | USART6 MSP（PC6/PC7 AF8）設定正確，問題在 HM-10 TX→STM32 RX 方向 | 加 echo debug 確認 RX 路徑通暢 |
| App 看不到距離更新 | HM-10 BLE 連線與 UART 是獨立的，已確認 STM32 TX→HM-10 RX→BLE→App 路徑正常 | 移除 echo debug，恢復正常 telemetry |
| 馬達完全不動 | L298N 電源誤接 GND | 接正確電源後馬達正常 |
| F/B 方向相反 | 實體馬達接線導致 IN1/IN2 邏輯反向 | 交換 `motor_driver.c` 中 FORWARD/BACKWARD 的 GPIO 設定 |
| L/R 無法轉向 | 轉速比 30% 低於馬達起步力矩 | 改為 pivot turn：外輪全速，內輪停止 |
| F 按久自動停 | 超聲波未接但 PA2 浮空，隨機觸發 obs_cnt → 進入避障狀態，之後 B/L/R 被擋 | 加 `avoidance_off` 旗標暫時停用避障 |

#### 程式修改清單

- **`Core/Src/motor_driver.c`**：FORWARD/BACKWARD GPIO 邏輯對調（PE5/PE6 與 PE11/PE13）
- **`Core/Src/main.c`**：
  - 移除開機馬達自動測試
  - CAR_LEFT / CAR_RIGHT 改為 pivot turn（外輪全速，內輪停）
  - 加入 `avoidance_off` 旗標（目前 = 1，暫停避障）
- **`Core/Src/bluetooth.c`**：加入/移除 echo debug（最終已移除）
- **`Core/Inc/motor_driver.h`**：MOTOR_TURN_RATIO 從 3 改為 6（後改為 pivot turn 故此值暫無作用）

### Flutter App（smart_car_app）

- **`lib/main.dart`**：
  - 加入 debug `_send()` 訊息（DBG: not connected / txChar null / dup / Sent / Write error）
  - 加入/移除 `RX: $text` BLE 原始資料顯示（debug 用，已移除）
  - `_lastCmd` 初始值從 `'S'` 改為 `'S'`（維持不變）
  - 連線斷開時重置 `_lastCmd = ''`

---

## 功能完成狀態

### ✅ 已完成

- **BLE 遙控**：HM-10（MLT-BT05）← → STM32 USART6 雙向通訊正常
- **前進（F）/ 後退（B）**：雙輪同步，方向正確，長按持續動作
- **左轉（L）/ 右轉（R）**：pivot turn，需左右各一組馬達（OUT1/2 + OUT3/4）才能實現轉向
- **停止（S）**：隨時生效
- **Telemetry**：STM32 每 300ms 傳 `$STATUS,speed,angle,dist,obstacle#` 到 App
- **距離顯示**：App 解析 telemetry 並顯示超聲波距離（需接 HC-SR04 才有讀值）
- **Flutter App**：BLE 掃描、連線、發指令、顯示狀態列

### ⚠️ 待確認 / 未完成

| 項目 | 狀態 | 備註 |
|------|------|------|
| 右側馬達（OUT3/4）接線 | ❌ 尚未接 | L/R 轉向需要左右兩組馬達分開 |
| 避障（HC-SR04）| ⚠️ 暫停 | `avoidance_off = 1`，需接感測器後測試再開啟 |
| IR 感測器 | ⚠️ 暫停 | 避障停用中，待硬體接好後一起啟用 |
| 舵機 | ❌ 未接 | 目前設計不需要舵機（差速轉向），可移除相關 init |
| L298N 電容 | ⚠️ 待安裝 | 100µF 並聯在 VS-GND，減少啟動電流峰值 |
| iOS 測試 | ❌ 待辦 | 需要 Mac 才能編譯 iOS App |
| 避障邏輯重新啟用 | ❌ 待辦 | 接好感測器後，將 `avoidance_off` 改回 `0` |

---

## 硬體接線確認

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
| TX | PC7 | BLE → STM32（指令接收）|
| RX | PC6 | STM32 → BLE（Telemetry）|
| VCC | 3.3V | |
| GND | GND | |

### 下一步
1. 把右側馬達接到 OUT3/OUT4
2. 並聯 100µF 電容到 L298N VS-GND
3. 接好 HC-SR04 後將 `avoidance_off = 0`，測試避障
