# UE5 設置指南 - 卡牌對戰遊戲

## 逐步設置流程

### 步驟 1：編譯 C++ 代碼

1. 打開 **Windows Explorer**
2. 導航到 `d:\UnrealEngineProjects\CardGame`
3. 右鍵點擊 `CardGame.sln`
4. 選擇 **Open with Visual Studio 2022** (或更新版本)
5. 在 Visual Studio 中：
   - 右鍵點擊 **CardGame** 項目
   - 選擇 **Build** (構建)
   - 等待構建完成（應顯示 "Build succeeded"）

### 步驟 2：打開 UE5 編輯器

1. 在 File Explorer 中雙擊 `CardGame.uproject`
2. 或在 Epic Games Launcher 中選擇 **CardGame** 項目並點擊 **Open**
3. 編輯器會詢問是否重建 C++ 代碼，選擇 **Yes**
4. 等待編輯器完全加載

### 步驟 3：創建 GameMode Blueprint

1. 在 **Content Browser** 中創建一個新文件夾 `Blueprints`
2. 在 `Blueprints` 文件夾中右鍵
3. 選擇 **Blueprint Class**
4. 搜索 `CardBattle` 並選擇它作為父類
5. 命名為 `BP_CardBattle`
6. 雙擊打開 `BP_CardBattle`
7. 在右上角點擊 **Compile** 和 **Save**
8. 關閉該藍圖

### 步驟 4：創建 Player Pawn Blueprint

1. 同樣在 `Blueprints` 文件夾中創建新 Blueprint Class
2. 搜索 `CardGamePlayer` 並選擇它
3. 命名為 `BP_CardGamePlayer`
4. 打開並編輯（暫時保留默認設置）
5. **Compile** 和 **Save**
6. 關閉該藍圖

### 步驟 5：創建測試 Actor Blueprint

1. 創建新 Blueprint Class 基於 `CardGameTester`
2. 命名為 `BP_CardGameTester`
3. 打開並編輯
4. **Compile** 和 **Save**

### 步驟 6：設置 Project Settings

1. 在菜單欄選擇 **Edit** → **Project Settings**
2. 搜索 **Maps & Modes**
3. 在 **Default GameMode** 下拉菜單中選擇 `BP_CardBattle`
4. 關閉 Project Settings（自動保存）

### 步驟 7：創建測試 Level

1. 文件菜單 → **New Level**
2. 選擇 **Basic** 模板
3. **Save** 這個 Level，命名為 `TestLevel`

### 步驟 8：在 Level 中放置 Tester Actor

1. 在 Place Actors 面板中搜索 `BP_CardGameTester`
2. 拖放到 Level 中任何位置
3. 選擇該 Actor（在世界中選中它）

### 步驟 9：配置自動遊戲啟動

#### 方法 A：使用 Level Blueprint（推薦新手）

1. 在工具欄中點擊 **Blueprints** → **Open Level Blueprint**
2. 右鍵空白區域 → **Event Begin Play**
3. 從返回值拖出連線 → 搜索 `Cast to CardGameTester`
4. 從 Cast 的結果拖出 → 搜索 `Start Test Game`
5. 連接 Event Begin Play 的執行針腳到 Start Test Game
6. **Compile** 和 **Save**

#### 方法 B：使用 Actor Blueprint（推薦進階）

1. 創建新 Blueprint Class 基於 **Actor**
2. 命名為 `BP_GameStarter`
3. 打開 Event Graph
4. **Event Begin Play** → 搜索 `Get Game Mode`
5. 從 Game Mode 拖出 → **Cast to CardBattle**
6. 從結果拖出 → 搜索 `Start Game`
7. 連接執行針腳
8. **Compile** 和 **Save**
9. 在 Level 中放置這個 Actor

### 步驟 10：測試遊戲

1. 在 Level 編輯器中按 **Play** 按鈕
2. 檢查 **Output Log**（View → Developer Tools → Output Log）
3. 應該看到以下日誌：
   ```
   ===== STARTING TEST GAME =====
   Game initialized. Player 0 hand size: 10, Player 1 hand size: 10
   Player X goes first
   ========== GAME STATE ==========
   ```

4. 遊戲應該會自動運行並顯示每一步
5. 最後顯示獲勝者

## 進階設置

### 自定義回合時間

在 `BP_CardBattle` 的 Details 面板中：
1. 搜索 `Turn Time Limit`
2. 修改數值（默認 30 秒）
3. **Compile** 保存

### 調整自動出牌延遲

在 `BP_CardGameTester` 中：
1. 打開 Event Graph
2. 在 Event Begin Play 後添加：
   - 節點：**Set Auto Play Delay**
   - 參數：**Delay** = 你想要的秒數（推薦 1-2 秒）

### 添加調試信息顯示

1. 創建新 Blueprint 基於 HUD
2. 在 Draw HUD 事件中獲取遊戲狀態並繪製文本
3. 在 GameMode 中設置為 HUD Class

## UI 集成（可選）

### 基本 UI 框架

```
UMG Widget Blueprint: WBP_GameUI
├─ Canvas Panel
│  ├─ Player 0 Info Panel
│  │  ├─ Hand Size Text
│  │  ├─ Score Text
│  │  └─ Card Grid (放置手牌)
│  ├─ Player 1 Info Panel
│  ├─ Center Info
│  │  ├─ Current Player Text
│  │  ├─ Remaining Time Text
│  │  └─ Last Round Info
│  └─ Game Over Panel
│     ├─ Winner Text
│     ├─ Final Scores
│     └─ Play Again Button
```

### 實現步驟

1. **創建 UI Widget**：
   - Content Browser → 新建 Widget Blueprint
   - 命名為 `WBP_GameUI`

2. **添加元素**（使用 UMG Designer）：
   - Text Block 显示玩家信息
   - Horizontal Box 放置卡牌
   - Button 用於出牌

3. **在 GameMode 中顯示 UI**：
   - 在 C++ 或藍圖中 Create Widget
   - 添加到 Viewport

## 故障排查

### 問題：藍圖找不到 CardBattle 類

**解決方案**：
1. 確保 C++ 代碼已編譯成功
2. 編輯器 → **Edit** → **Plugins**
3. 搜索 "CardGame"，確保已啟用
4. 重啟編輯器

### 問題：遊戲不啟動

**解決方案**：
1. 檢查 Project Settings 中 Default GameMode 設置
2. 查看 Output Log 中的錯誤信息
3. 確認 Level 中有 GameMode Actor

### 問題：無法編譯 C++ 代碼

**解決方案**：
1. 關閉 UE5 編輯器
2. 刪除 `Intermediate` 和 `Binaries` 文件夾
3. 在 File Explorer 中右鍵 `.uproject` → **Generate Visual Studio project files**
4. 打開 `.sln` 並重新編譯

### 問題：卡牌位置索引錯誤

**解決方案**：
1. 使用 `GetPlayerHand()` 檢查實際手牌大小
2. 確保 CardIndex 在 0 到 (HandSize - 1) 之間
3. 查看日誌中的警告信息

## 性能優化建議

1. **禁用不必要的功能**：
   - 在編輯器中禁用物理模擬
   - 降低 LOD 設置

2. **使用遊戲模擬**：
   - 測試時使用 `ACardGameTester` 而不是手動操作
   - 可以快速測試數百局遊戲

3. **監控性能**：
   - **Ctrl+Shift+,** 打開性能分析面板
   - 查看 FPS 和 CPU 使用率

## 下一步

✅ 基礎設置完成後，你可以：

1. **添加 UI**：
   - 創建卡牌視覺表示
   - 顯示分數和計時器
   - 添加遊戲結果屏幕

2. **添加輸入**：
   - 配置鼠標點擊出牌
   - 添加觸屏支持
   - 配置鍵盤快捷鍵

3. **增強遊戲**：
   - 添加音效
   - 添加動畫
   - 實現 AI 對手

4. **測試和優化**：
   - 進行平衡性測試
   - 優化性能
   - 收集反饋

## 命令行快速測試

如果你喜歡使用命令行，可以運行：

```batch
# 在項目目錄中運行（Windows PowerShell）
"C:\Program Files\Epic Games\UE_5.x\Engine\Binaries\Win64\UnrealEditor.exe" CardGame.uproject -game

# 或使用 Unreal Automation Tool
cd d:\UnrealEngineProjects\CardGame
```

## 必要的插件

確保以下插件已啟用（通常預設啟用）：
- ✅ Enhanced Input Plugin
- ✅ Online Subsystem
- ✅ Niagara Particle System（如果添加特效）

## 文件檢查清單

在開始遊戲前確認以下文件存在：

- ✅ `Source/CardGame/Card.h` 和 `.cpp`
- ✅ `Source/CardGame/Player.h` 和 `.cpp`
- ✅ `Source/CardGame/CardBattle.h` 和 `.cpp`
- ✅ `Source/CardGame/CardGamePlayer.h` 和 `.cpp`
- ✅ `Source/CardGame/CardGameTester.h` 和 `.cpp`
- ✅ `CardGame.sln` 已編譯
- ✅ 藍圖類已創建
- ✅ Project Settings 已配置

---

**祝你玩得愉快！** 🎮

如有問題，查看 Output Log 或參考其他文檔文件。
