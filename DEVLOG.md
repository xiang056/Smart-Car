# Smart Car V1 — 開發紀錄

---

## 2026-06-04 重構：硬體精簡 + 差速轉向 + App 對齊

### 硬體決策

| 決策 | 原因 |
|------|------|
| HC-SR04 超聲波取消 | 車體空間不足；避障邏輯已實作並驗證，作為設計取捨移除 |
| IR 感測器取消 | 車體空間不足 |
| SG90 伺服馬達取消 | 差速輪驅本身已能轉向，伺服無實質作用 |
| HC-05 → HM-10 BLE | 3.3V 直接相容，不需電阻分壓；BLE 而非 Classic BT |

---

### STM32 韌體變更

#### 刪除模組
| 檔案 | 原因 |
|------|------|
| `Core/Src/ultrasonic_driver.c` | 硬體取消 |
| `Core/Inc/ultrasonic_driver.h` | 硬體取消 |
| `Core/Src/ir_sensor_driver.c` | 硬體取消 |
| `Core/Inc/ir_sensor_driver.h` | 硬體取消 |
| `Core/Src/servo_driver.c` | 硬體取消 |
| `Core/Inc/servo_driver.h` | 硬體取消 |

#### 狀態機重構（`main.c`）

原本 9 個狀態（含避障）全部移除，改為以下 9 個狀態：

```c
typedef enum {
    CAR_STOP,            // 0 靜止
    CAR_FORWARD,         // 1 直線前進
    CAR_BACKWARD,        // 2 直線後退
    CAR_LEFT,            // 3 原地左轉（pivot）
    CAR_RIGHT,           // 4 原地右轉（pivot）
    CAR_FORWARD_LEFT,    // 5 前進左彎（差速）
    CAR_FORWARD_RIGHT,   // 6 前進右彎（差速）
    CAR_BACKWARD_LEFT,   // 7 後退左彎（差速）
    CAR_BACKWARD_RIGHT,  // 8 後退右彎（差速）
} CarState_t;
```

**差速轉向邏輯**：L/R 指令根據當前狀態決定行為

| 當前狀態 | 收到 L | 收到 R |
|---------|--------|--------|
| STOP | pivot 左轉 | pivot 右轉 |
| FORWARD / FORWARD_* | 前進左彎（內輪 40%） | 前進右彎（內輪 40%） |
| BACKWARD / BACKWARD_* | 後退左彎（內輪 40%） | 後退右彎（內輪 40%） |

```c
// 指令解析邏輯
case BT_CMD_LEFT:
    if (state == CAR_FORWARD || state == CAR_FORWARD_LEFT || state == CAR_FORWARD_RIGHT)
        state = CAR_FORWARD_LEFT;
    else if (state == CAR_BACKWARD || state == CAR_BACKWARD_LEFT || state == CAR_BACKWARD_RIGHT)
        state = CAR_BACKWARD_LEFT;
    else
        state = CAR_LEFT;
    break;
```

#### `motor_driver.h` 更新
- 移除未使用的 `MOTOR_TURN_RATIO`
- 新增 `MOTOR_PWM_INNER (MOTOR_PWM_MAX * 4 / 10)` 作為差速內輪速度常數
- 修正：原 `#define PWM_INNER` 錯誤放在 `while(1)` 迴圈內，移至 header 並正確命名

#### Telemetry 簡化
感測器移除後，telemetry 格式從 4 欄縮減為 2 欄：

| 版本 | 格式 | 最大長度 |
|------|------|---------|
| 舊 | `S,speed,angle,dist,obs\n` | 15 bytes |
| 新 | `S,speed,state\n` | 10 bytes |

`state` 值即 `CarState_t` 枚舉值（0–8），App 端直接對應顯示。

#### `stm32f4xx_it.c` 清理
- 移除 `#include "ultrasonic_driver.h"`
- 移除 `HAL_TIM_IC_CaptureCallback`（超聲波已移除）

---

### Flutter App 變更（`lib/main.dart`）

#### Telemetry 解析更新
```dart
// 舊
RegExp(r'S,(\d+),(\d+),(\d+),(\d+)')  // 解析 dist
// 新
RegExp(r'S,(\d+),(\d+)')               // 解析 state
```

#### `_DistanceCard` → `_StateCard`
超聲波距離卡片移除，改為即時顯示車輛行駛狀態。支援全部 9 個狀態（0–8）：

| state | 標籤 | 顏色 |
|-------|------|------|
| 0 | STOP | white38 |
| 1 | FORWARD | cyan |
| 2 | BACKWARD | orange |
| 3 | LEFT | cyanAccent |
| 4 | RIGHT | cyanAccent |
| 5 | FWD LEFT | cyan |
| 6 | FWD RIGHT | cyan |
| 7 | BWD LEFT | orange |
| 8 | BWD RIGHT | orange |

原先 `clamp(0, 4)` 導致複合狀態（5–8）全部顯示為 RIGHT，已修正為 `clamp(0, 8)`。

#### BLE 寫入延遲修正
```dart
// 舊：等待 GATT ACK，增加控制延遲
_txChar!.write(cmd, withoutResponse: false)
// 新：無回應寫入，符合 HM-10 UART 特性
_txChar!.write(cmd, withoutResponse: true)
```

#### 移除 debug 訊息
`_send()` 中的 `DBG: not connected`、`DBG: txChar null`、`DBG: dup` 等開發期 debug 字串全部移除。

---

## 功能完成狀態

### ✅ 已完成

| 功能 | 說明 |
|------|------|
| BLE 遙控 | HM-10（USART6）中斷驅動，雙向通訊 |
| 前進 / 後退 | 雙輪同步，全速 |
| 差速轉向 | 行進中 L/R：內輪 40%，外輪 100% |
| 原地轉向 | 靜止時 L/R：一輪停，一輪全速（pivot） |
| 停止 | S 指令隨時生效 |
| Telemetry | 每 300ms 傳送 `S,speed,state\n`，≤ 10 bytes |
| Flutter App | BLE 掃描、連線、方向控制、狀態顯示（9 態） |
| 避障邏輯 | 已實作並驗證（BRAKE→BACKUP→TURN→CHECK），因空間取捨移除 |

### ⚠️ 已知限制

| 項目 | 說明 |
|------|------|
| 複合指令 UX | 按住 F 再按 L，放開 L 時車停（送 S）而非回到前進；需 App 端追蹤多鍵狀態才能修，暫不處理 |
| iOS 測試 | 尚未在 iOS 驗證，需要 Mac |

---

## 硬體接線（最終版）

### L298N ↔ STM32
| L298N | STM32 | 功能 |
|-------|-------|------|
| IN1 | PE5 | 左輪方向 A |
| IN2 | PE6 | 左輪方向 B |
| ENA | PE9（拔跳帽）| 左輪 PWM（TIM1_CH1） |
| IN3 | PE11 | 右輪方向 A |
| IN4 | PE13 | 右輪方向 B |
| ENB | PE14（拔跳帽）| 右輪 PWM（TIM1_CH4） |

### HM-10 ↔ STM32
| HM-10 | STM32 | 功能 |
|-------|-------|------|
| TX | PC7 | BLE RX → STM32（USART6_RX） |
| RX | PC6 | STM32 TX → BLE（USART6_TX） |
| VCC | 3.3V | |
| GND | GND | |

---

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
