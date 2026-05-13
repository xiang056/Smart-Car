# Windows Git 完全教程 - 從零開始

## 第一部分：安裝 Git

### 步驟 1：下載 Git

1. 打開瀏覽器，訪問官方網站：https://git-scm.com/
2. 點擊 **"Download for Windows"**
3. 自動下載 **Git-[版本]-64-bit.exe**（大約 50MB）

### 步驟 2：安裝 Git

1. 雙擊下載的 exe 檔
2. **開始安裝**：
   - ✅ 點擊 "Next" 確認路徑（通常 `C:\Program Files\Git`）
   - ✅ 保持預設選項
   - ✅ 編輯器選擇 **Visual Studio Code**（或 Notepad++）
   
3. **關鍵步驟 - 換行符配置**：
   - 選 **"Checkout as-is, commit as-is"**（推薦給 Windows 開發者）
   - 或選 **"Checkout Windows-style, commit Unix-style"**（也可以）
   - 點擊 "Next"

4. **完成安裝**：
   - 點擊 "Install"
   - 等待 1-2 分鐘
   - ✅ 完成後勾選 "Launch Git Bash"

### 步驟 3：驗證安裝

1. 打開 **Git Bash**（Windows 開始菜單搜尋 "Git Bash"）
2. 輸入命令：
```bash
git --version
```

**預期輸出**：
```
git version 2.x.x.windows.x
```

如果看到版本號，✅ 安裝成功！

---

## 第二部分：Git 基礎概念

### 核心概念圖

```
你的電腦（本地）
    ↓
  Working Directory（工作區）
    ↓ git add
  Staging Area（暫存區）
    ↓ git commit
  Local Repository（本地倉庫）
    ↓ git push
Remote Repository（遠程倉庫，例如 GitHub）
```

### 重要概念速查

| 名詞 | 意思 | 比喻 |
|------|------|------|
| **Repository (Repo)** | 倉庫 | 你的項目歷史檔案櫃 |
| **Commit** | 提交 | 保存一個版本的快照 |
| **Branch** | 分支 | 平行的開發線（如主線 main、功能線 feature） |
| **Push** | 推送 | 上傳本地更改到遠程倉庫 |
| **Pull** | 拉取 | 下載遠程更改到本地 |
| **Merge** | 合併 | 將一個分支的代碼合入另一個分支 |
| **Stage** | 暫存 | 標記檔案為「準備好提交」 |
| **Untracked** | 未追蹤 | 新檔案，Git 還不知道要管理它 |

---

## 第三部分：全局配置（首次設置）

### 步驟 1：配置使用者資訊

打開 **Git Bash**，執行：

```bash
git config --global user.name "你的名字"
git config --global user.email "你的郵箱@example.com"
```

**例子**：
```bash
git config --global user.name "Edward"
git config --global user.email "edward@example.com"
```

### 步驟 2：驗證配置

```bash
git config --global --list
```

**預期輸出**：
```
user.name=Edward
user.email=edward@example.com
core.editor=...
```

### 步驟 3：配置編輯器（可選）

設置 VS Code 為預設編輯器：

```bash
git config --global core.editor "code --wait"
```

---

## 第四部分：STM32 項目的 Git 工作流

### 方案選擇

選一個適合你的方案：

| 方案 | 難度 | 優點 | 適用場景 |
|------|------|------|---------|
| **本地 Git**（推薦入門） | ⭐ | 無需網路，即學即用 | 個人學習、單人項目 |
| **GitHub**（推薦長期） | ⭐⭐ | 雲備份，容易分享 | 多人協作、備份 |
| **GitLab/Gitea**（自託管） | ⭐⭐⭐ | 完全控制，私密 | 企業項目 |

**我建議你先用「本地 Git」學習基礎，再升級到 GitHub。**

---

## 第五部分：本地 Git 工作流（推薦開始）

### 場景：STM32 Smart Car 項目

#### 初始化項目（第一次）

```bash
# 1. 進入你的項目資料夾
cd C:\Users\你的用戶名\Documents\SmartCar

# 2. 初始化 Git 倉庫
git init

# 預期輸出：
# Initialized empty Git repository in C:/Users/.../SmartCar/.git/
```

✅ 現在你的項目是一個 Git 倉庫了！

#### 創建 .gitignore 檔（忽略不需要的檔案）

```bash
# 在項目根目錄創建 .gitignore 檔
# Windows 用戶可以用記事本或 VS Code
```

**貼入以下內容**（STM32 項目標準）：

```
# STM32CubeIDE 生成檔案
.mxproject
*.ioc.bak
*.log

# 編譯產物
*.o
*.elf
*.bin
*.hex
*.map
Build/
Debug/
Release/

# IDE 相關
.cproject
.project
.settings/
*.launch

# OS 相關
.DS_Store
Thumbs.db
*.swp
*~

# Python 快取（如果有）
__pycache__/
*.pyc

# 編輯器設定（個人用，不上傳）
.vscode/settings.json
.idea/

# 依賴檔案（如果用 npm/pip）
node_modules/
venv/
```

**保存檔案**：
- 檔案名：**.gitignore**（注意開頭的 `.`）
- 位置：`C:\Users\你的用戶名\Documents\SmartCar\.gitignore`

#### 首次提交

```bash
# 1. 檢查狀態
git status

# 預期輸出：
# On branch master
# No commits yet
# Untracked files:
#   .gitignore
#   Core/
#   Drivers/
#   ...

# 2. 將所有檔案加入暫存區
git add .

# 3. 提交（保存快照）
git commit -m "Initial commit: STM32 Smart Car project setup"

# 預期輸出：
# [master (root-commit) abc1234] Initial commit: STM32 Smart Car project setup
#  X files changed, Y insertions(+)
#  create mode 100644 .gitignore
#  ...
```

✅ 你的第一個 commit 完成了！

#### 日常工作流

**修改代碼後**：

```bash
# 1. 查看改動
git status

# 輸出例：
# On branch master
# Changes not staged for commit:
#   modified:   Core/Src/main.c
#   modified:   Core/Inc/config.h
# Untracked files:
#   test_config.h

# 2. 添加具體修改（三種方式任選一）

# 方式 A：添加單個檔案
git add Core/Src/main.c

# 方式 B：添加多個檔案
git add Core/Src/*.c Core/Inc/*.h

# 方式 C：添加所有變更（推薦）
git add .

# 3. 查看暫存內容（可選）
git diff --staged

# 4. 提交
git commit -m "feat: 實現馬達控制驅動層"

# 或使用更詳細的訊息
git commit -m "feat: 實現馬達控制驅動層

- 添加 motor_driver.c 和 motor_driver.h
- 支援 PWM 速度控制（0-100%）
- 支援方向控制（前進/後退）
- 包含安全檢查邏輯"
```

#### 查看提交歷史

```bash
# 查看所有提交
git log

# 簡化版（單行顯示）
git log --oneline

# 預期輸出：
# abc1234 (HEAD -> master) feat: 實現馬達控制驅動層
# def5678 Initial commit: STM32 Smart Car project setup

# 查看某個提交的詳細更改
git show abc1234

# 查看特定檔案的歷史
git log --oneline Core/Src/main.c
```

---

## 第六部分：分支管理（多功能同時開發）

### 為什麼需要分支？

想象你的項目有這些並行任務：
- 主線（main）：穩定版本
- 馬達功能（feature/motor）：開發中
- 藍牙通訊（feature/bluetooth）：開發中
- 緊急修復（hotfix/sensor-bug）：修復 bug

**分支讓你同時做這些工作而不互相干擾！**

### 分支操作

#### 創建分支

```bash
# 1. 基於當前分支（main）創建新分支
git branch feature/motor-driver

# 2. 切換到新分支
git checkout feature/motor-driver

# 一行操作（創建並切換）
git checkout -b feature/motor-driver

# 或使用新語法（Git 2.23+）
git switch -c feature/motor-driver

# 驗證（* 表示當前分支）
git branch
# 輸出：
#   main
# * feature/motor-driver
```

#### 在新分支上開發

```bash
# 編輯檔案（例如寫馬達驅動代碼）
# 然後提交
git add .
git commit -m "feat: 實現 PWM 馬達速度控制"
```

#### 切換回主線

```bash
git checkout main
# 或
git switch main
```

**注意**：切換前要提交所有更改，否則會出錯！

#### 合併分支

當馬達功能完成，合併回主線：

```bash
# 確保在 main 分支
git checkout main

# 合併 feature/motor-driver
git merge feature/motor-driver

# 預期輸出：
# Updating abc1234..def5678
# Fast-forward
#  Core/Src/motor_driver.c | 150 +++++++++
#  Core/Inc/motor_driver.h |  25 +++++
#  2 files changed, 175 insertions(+)
```

#### 刪除已合併的分支

```bash
# 刪除本地分支
git branch -d feature/motor-driver

# 強制刪除（即使未合併）
git branch -D feature/motor-driver
```

### 分支命名規範（推薦）

| 前綴 | 用途 | 例子 |
|------|------|------|
| `feature/` | 新功能 | `feature/motor-driver` |
| `bugfix/` | 修復 bug | `bugfix/uart-timeout` |
| `hotfix/` | 緊急修復（在 main 上） | `hotfix/critical-sensor` |
| `docs/` | 文檔 | `docs/api-reference` |
| `refactor/` | 重構 | `refactor/timer-config` |

---

## 第七部分：撤銷和恢復

### 場景 1：修改了檔案，還沒 add

```bash
# 查看改動
git status

# 撤銷修改（恢復到上一個 commit）
git checkout -- Core/Src/main.c

# 或使用新語法
git restore Core/Src/main.c
```

### 場景 2：已 add，還沒 commit

```bash
# 取消暫存
git reset HEAD Core/Src/main.c

# 或
git restore --staged Core/Src/main.c
```

### 場景 3：已 commit，要撤銷

```bash
# 查看最後 3 個提交
git log --oneline -3

# 撤銷最後一個提交（保留改動）
git reset --soft HEAD~1

# 撤銷最後一個提交（丟棄改動）
git reset --hard HEAD~1

# ⚠️ 危險操作，請確認後再執行！
```

### 場景 4：已 push 到遠程，還要撤銷

```bash
# 這個比較複雜，後面講 GitHub 時會說
# 簡單方法：新建一個 commit 來恢復
git revert abc1234  # 反向提交，保留歷史

# 然後 push
git push
```

---

## 第八部分：升級到 GitHub（雲備份）

### 為什麼要用 GitHub？

1. **雲備份**：你的代碼不會丟失（電腦損壞也沒事）
2. **分享和協作**：可以與他人共享代碼
3. **版本管理**：完整的歷史記錄
4. **免費私密倉庫**：GitHub 提供免費的私密倉庫

### 步驟 1：註冊 GitHub 帳號

1. 打開 https://github.com/
2. 點擊 "Sign up"
3. 填寫郵箱、密碼、用戶名
4. 驗證郵箱

✅ 帳號創建完成

### 步驟 2：配置 SSH 密鑰（安全連線）

**為什麼需要 SSH？** 避免每次 push 都要輸密碼

#### 生成密鑰

打開 **Git Bash**：

```bash
# 生成 SSH 密鑰對
ssh-keygen -t ed25519 -C "你的郵箱@example.com"

# 或舊版 SSH（如果上面不行）
ssh-keygen -t rsa -b 4096 -C "你的郵箱@example.com"

# 提示「Enter file in which to save the key」：
# 直接按 Enter（使用預設路徑 ~/.ssh/id_ed25519）

# 提示「Enter passphrase」：
# 按 Enter（不設密碼，方便開發）
# 或輸入密碼（更安全，但要記住）
```

**預期輸出**：
```
Your identification has been saved in /c/Users/你的用戶名/.ssh/id_ed25519
Your public key has been saved in /c/Users/你的用戶名/.ssh/id_ed25519.pub
...
```

#### 複製公鑰

```bash
# 顯示公鑰內容
cat ~/.ssh/id_ed25519.pub

# 複製整個輸出（從 ssh-ed25519 開始到郵箱結束）
```

#### 在 GitHub 上添加公鑰

1. 登錄 GitHub
2. 點擊右上角頭像 → **Settings**
3. 左側菜單 → **SSH and GPG keys**
4. 點擊 **"New SSH key"**
5. **Title**：填寫 "Windows Development PC" 或你的電腦名
6. **Key**：貼入剛才複製的公鑰（整個內容）
7. 點擊 **"Add SSH key"**

✅ SSH 配置完成

#### 驗證連線

```bash
ssh -T git@github.com

# 預期輸出：
# Hi 你的用戶名! You've successfully authenticated, 
# but GitHub does not provide shell access.
```

### 步驟 3：在 GitHub 上創建倉庫

1. 登錄 GitHub，點擊 **"+"** → **"New repository"**
2. **Repository name**：`SmartCar`（或你的項目名）
3. **Description**：`STM32F407 Smart Car Project`（可選）
4. **Visibility**：
   - **Public**：公開（免費）
   - **Private**：私密（免費）
   - 選 **Private**（保護你的代碼）
5. ❌ **不要** 勾選 "Initialize this repository with..."（因為你已有本地倉庫）
6. 點擊 **"Create repository"**

✅ 遠程倉庫創建完成

### 步驟 4：連接本地倉庫到 GitHub

GitHub 會顯示指令，你在 **Git Bash** 執行：

```bash
# 1. 進入你的項目目錄
cd C:\Users\你的用戶名\Documents\SmartCar

# 2. 添加遠程倉庫（GitHub 會給你這個命令）
git remote add origin git@github.com:你的用戶名/SmartCar.git

# 3. 驗證（應該看到 origin）
git remote -v
# 輸出：
# origin  git@github.com:你的用戶名/SmartCar.git (fetch)
# origin  git@github.com:你的用戶名/SmartCar.git (push)

# 4. 重命名主分支為 main（GitHub 預設）
git branch -M main

# 5. 推送本地倉庫到 GitHub
git push -u origin main

# 預期輸出：
# Enumerating objects: 3, done.
# Counting objects: 100% (3/3), done.
# ...
# To github.com:你的用戶名/SmartCar.git
#  * [new branch]      main -> main
# Branch 'main' set up to track remote-tracking branch 'main' from 'origin'.
```

✅ 推送完成！你的代碼現在在 GitHub 上了！

### 步驟 5：驗證（在 GitHub 網頁上查看）

1. 登錄 GitHub
2. 進入你的倉庫頁面（`github.com/你的用戶名/SmartCar`）
3. 應該看到你的所有檔案和提交歷史

---

## 第九部分：日常 GitHub 工作流

### 推送新提交

```bash
# 1. 修改檔案
# 2. 提交
git add .
git commit -m "feat: 添加 OLED 顯示驅動"

# 3. 推送到 GitHub
git push

# 或明確指定分支
git push origin main
```

### 拉取遠程更新

```bash
# 如果有人修改了遠程倉庫（或你在其他電腦修改了）
git pull

# 或更明確的做法
git fetch              # 下載最新資訊
git merge origin/main  # 合併
```

### 從 GitHub 克隆（在新電腦上）

```bash
# 1. 進入目錄
cd C:\Users\你的用戶名\Documents

# 2. 克隆倉庫
git clone git@github.com:你的用戶名/SmartCar.git

# 或用 HTTPS（不用 SSH）
git clone https://github.com/你的用戶名/SmartCar.git

# 3. 進入項目
cd SmartCar

# 完成！你現在有整個項目了
```

---

## 第十部分：Git 常用命令速查表

| 命令 | 功能 | 例子 |
|------|------|------|
| `git init` | 初始化新倉庫 | `git init` |
| `git add` | 暫存檔案 | `git add .` |
| `git commit` | 提交快照 | `git commit -m "desc"` |
| `git push` | 推送到遠程 | `git push origin main` |
| `git pull` | 拉取遠程更新 | `git pull` |
| `git status` | 查看狀態 | `git status` |
| `git log` | 查看歷史 | `git log --oneline` |
| `git branch` | 管理分支 | `git branch -a` |
| `git checkout` | 切換分支 | `git checkout feature/motor` |
| `git merge` | 合併分支 | `git merge feature/motor` |
| `git reset` | 撤銷提交 | `git reset --soft HEAD~1` |
| `git diff` | 查看差異 | `git diff` |
| `git stash` | 臨時保存 | `git stash` |

---

## 第十一部分：STM32 項目的 Git 最佳實踐

### 推薦目錄結構

```
SmartCar/
├── .git/                    ← Git 自動管理，不用碰
├── .gitignore              ← 忽略規則
├── README.md               ← 項目說明
├── .ioc                    ← CubeIDE 配置檔
│
├── Core/
│   ├── Inc/
│   │   ├── main.h
│   │   ├── config.h
│   │   └── ...
│   └── Src/
│       ├── main.c
│       ├── stm32f4xx_it.c
│       └── ...
│
├── Drivers/
│   ├── CMSIS/
│   └── STM32F4xx_HAL_Driver/
│
├── docs/
│   ├── SmartCar_ProjectPlan.md
│   ├── STM32CubeIDE_Config_Checklist.md
│   └── ...
│
└── scripts/                 ← 編譯/燒入腳本（可選）
    ├── build.sh
    └── flash.sh
```

### 提交訊息規範

使用清晰的提交訊息，便於日後查找：

```bash
# ✅ 好的提交訊息
git commit -m "feat: 實現馬達 PWM 控制驅動"
git commit -m "fix: 修復 UART 接收超時問題"
git commit -m "docs: 更新 GPIO 配置文檔"
git commit -m "refactor: 優化計時器初始化邏輯"

# ❌ 不好的提交訊息
git commit -m "更新檔案"
git commit -m "bug fix"
git commit -m "..."
```

**提交訊息格式**（可選，但推薦）：
```
<type>: <subject>

<body>

<footer>
```

- **type**：feat（功能）, fix（修復）, docs（文檔）, refactor（重構）, test（測試）
- **subject**：簡短說明（不超過 50 字）
- **body**：詳細說明（可選）
- **footer**：相關 issue 編號（可選）

### 何時提交？

```
✅ 完成一個小功能
✅ 修復一個 bug
✅ 更新文檔
❌ 不要每一行代碼都提交
❌ 不要等到 1000 行改動才提交一次
```

**經驗法則**：當你能用一句話描述改動時，就該提交了。

---

## 第十二部分：常見問題排查

### Q1: 提交時提示「Please tell me who you are」

**原因**：未配置 Git 使用者資訊

**解決**：
```bash
git config --global user.name "你的名字"
git config --global user.email "你的郵箱@example.com"
```

### Q2: Push 時提示「Permission denied」

**原因**：SSH 密鑰未配置

**解決**：
1. 重新生成 SSH 密鑰（參考第八部分步驟 2）
2. 或改用 HTTPS（GitHub 會要求輸入 Personal Access Token）

### Q3: 合併分支時出現衝突（Conflict）

**原因**：兩個分支修改了同一檔案的同一行

**解決**：
1. 打開衝突檔案（VS Code 會高亮衝突區域）
2. 手動選擇要保留的代碼
3. `git add` 和 `git commit`

```
<<<<<<< HEAD
你的代碼
=======
其他人的代碼
>>>>>>> feature/other
```

編輯為：
```
最終要保留的代碼
```

### Q4: 誤刪檔案，想恢復

```bash
# 查看刪除前的版本
git log --diff-filter=D --summary | grep delete

# 恢復檔案
git checkout HEAD~1 -- 檔案路徑

# 或恢復整個舊版本
git reset --hard abc1234
```

### Q5: 想改變最後一個提交訊息

```bash
# 修改最後一個提交訊息
git commit --amend -m "新的提交訊息"

# ⚠️ 如果已 push，需要強制推送
git push --force-with-lease
```

---

## 快速開始清單

### 今天完成

- [ ] 下載並安裝 Git
- [ ] 配置 Git 使用者名和郵箱
- [ ] 在 SmartCar 項目目錄初始化本地倉庫（`git init`）
- [ ] 創建 .gitignore 檔
- [ ] 進行首次提交（`git commit`）

### 本周完成

- [ ] 在 GitHub 註冊帳號
- [ ] 配置 SSH 密鑰
- [ ] 在 GitHub 創建遠程倉庫
- [ ] 推送本地倉庫到 GitHub（`git push`）
- [ ] 練習創建分支和合併

### 長期習慣

- [ ] 每完成一個小功能就提交一次
- [ ] 定期推送到 GitHub（至少每天一次）
- [ ] 寫清楚的提交訊息
- [ ] 使用分支進行功能開發

---

**文檔版本**：v1.0  
**更新時間**：2026-03-31  
**適用系統**：Windows  
**目標受眾**：Git 零基礎使用者
