# 卡牌對戰遊戲 - 集成指南

## 快速開始

### 第一步：編譯項目
1. 打開 `CardGame.sln`
2. 右鍵選擇 CardGame 項目 → 生成
3. 等待編譯完成

### 第二步：在 UE5 編輯器中設置遊戲

#### 設置 GameMode
1. 打開 UE5 編輯器
2. 創建一個新的 Blueprint 類：
   - 基於 `ACardBattle`
   - 命名為 `BP_CardBattle`
3. 在 Project Settings → Maps & Modes 中：
   - 設置 `Default GameMode` = `BP_CardBattle`

#### 設置玩家控制器
1. 創建 Blueprint 類基於 `ACardGamePlayer`：
   - 命名為 `BP_CardGamePlayer`
2. 在 `BP_CardBattle` 中：
   - 設置 `Player Pawn Class` = `BP_CardGamePlayer`

#### 創建測試 Level
1. 創建一個新的 Level
2. 在 Level 中放置一個 Actor
3. 在該 Actor 的藍圖中添加一個 Event Begin Play：
   ```
   Cast to CardBattle → StartGame
   ```

### 第三步：運行遊戲

在 UE5 編輯器中按 Play，遊戲應該會：
1. 自動初始化兩個玩家和牌組
2. 隨機決定先手
3. 等待玩家出牌

## 系統架構詳解

### 1. 卡牌系統

**FCard 結構**
```cpp
USTRUCT(BlueprintType)
struct FCard {
    int32 CardValue;  // 1-30
};
```

**UCardDeck 類**
- 管理 30 張牌的完整牌組
- 自動打亂和抽牌
- 追蹤已抽取的牌數

### 2. 玩家系統

**APlayer 類**
- 管理單一玩家的狀態
- 手牌管理
- 分數計算
- 支持主動出牌和系統隨機出牌

### 3. 遊戲對戰系統

**ACardBattle 類** (GameMode)
- 管理整個遊戲流程
- 回合管理和計時
- 自動結算和獲勝者判定
- 提供 Blueprintable 接口供 UI 使用

## 遊戲流程詳解

### 遊戲初始化序列
```
StartGame()
  ├─ InitializeGame()
  │  ├─ 創建 2 個 APlayer 實例
  │  ├─ 創建 2 個 UCardDeck 實例
  │  ├─ 每個玩家抽 10 張牌
  │  └─ 重置狀態
  ├─ DetermineFirstPlayer()  // 隨機決定先手
  └─ GoToNextTurn()          // 進入第一個回合
```

### 出牌流程
```
PlayerPlayCard(PlayerId, CardIndex)
  ├─ 檢查是否是該玩家的回合
  ├─ 從手牌中移除卡牌
  ├─ 記錄出牌
  ├─ 如果雙方都出牌
  │  └─ ResolveRound()   // 結算回合
  │     ├─ 比較卡牌數值
  │     ├─ 計算分數
  │     └─ 檢查遊戲是否結束
  └─ 否則切換到另一玩家
```

### 時間管理
- 每回合 30 秒限制
- 通過 `Tick()` 函數每幀更新時間
- 時間到自動隨機出牌
- 時間歸零後切換回合

### 遊戲結束檢查
```
CheckGameOver()
  ├─ 檢查 Player 0 是否有手牌
  ├─ 檢查 Player 1 是否有手牌
  └─ 如果都沒有 → 計算最終分數並確定獲勝者
```

## API 參考

### ACardBattle 公共接口

#### 遊戲控制
```cpp
void StartGame()              // 開始遊戲
void EndGame()                // 結束遊戲
void PlayerPlayCard(int32 PlayerId, int32 CardIndex)  // 玩家出牌
```

#### 狀態查詢
```cpp
EBattleState GetBattleState() const
int32 GetCurrentTurnPlayerId() const
const TArray<FCard>& GetPlayerHand(int32 PlayerId) const
int32 GetPlayerScore(int32 PlayerId) const
float GetRemainingTurnTime() const
const FRoundInfo& GetLastRoundInfo() const
int32 GetWinner() const
```

### APlayer 公共接口

```cpp
void Initialize(int32 PlayerId)           // 初始化玩家
void SetDeck(UCardDeck* InDeck)           // 設置牌組
const TArray<FCard>& GetHand() const      // 獲取手牌
FCard PlayCard(int32 CardIndex)           // 出牌
FCard PlayCardRandom()                    // 隨機出牌
int32 GetHandSize() const                 // 獲取手牌數
int32 GetScore() const                    // 獲取分數
void AddScore(int32 Points)               // 增加分數
bool HasCards() const                     // 檢查是否有手牌
```

### UCardDeck 公共接口

```cpp
void Initialize()                         // 初始化牌組 (30張牌)
void DrawCards(int32 NumberOfCards, TArray<FCard>& OutCards)  // 抽牌
int32 GetRemainingCardsCount() const      // 獲取剩餘牌數
void Reset()                              // 重置牌組
```

## 調試

### 查看日誌
遊戲會輸出詳細的日誌信息到 Output Log：
- 遊戲初始化信息
- 每回合的出牌記錄
- 分數計算
- 最終結果

### 常見問題

**Q: 遊戲沒有開始**
- A: 確保在 Level 中調用了 `StartGame()`
- 檢查 GameMode 是否正確設置

**Q: 玩家無法出牌**
- A: 檢查當前是否是該玩家的回合 (GetBattleState)
- 確保 CardIndex 在有效範圍內 (0 到 HandSize-1)

**Q: 遊戲沒有結束**
- A: 檢查是否有玩家仍有手牌
- 查看日誌中的遊戲狀態

## 下一步開發

### 建議的功能擴展

1. **UI 系統**
   - 顯示玩家手牌
   - 顯示分數和剩餘時間
   - 顯示出牌歷史
   - 顯示遊戲結果

2. **輸入系統**
   - 整合增強型輸入系統
   - 支持鼠標點擊
   - 支持觸屏操作

3. **視覺效果**
   - 卡牌動畫
   - 分數飄動效果
   - 勝負提示特效

4. **AI 對手**
   - 簡單 AI (隨機出牌)
   - 中等 AI (策略性出牌)
   - 困難 AI (最優策略)

5. **進階規則**
   - 技能卡牌
   - 特殊效果
   - 多局對戰系統

## 技術細節

### 內存管理
- 所有動態對象使用 `NewObject<>()` 創建
- UObject 自動管理垃圾回收
- 遊戲結束時自動清理

### 線程安全
- 所有操作在主遊戲線程中執行
- Tick 函數中的時間更新是線程安全的

### 性能
- 優化的 Fisher-Yates 打亂算法
- 常數級別的手牌操作
- 最小化每幀計算

## 文件結構

```
Source/CardGame/
├── Card.h/Card.cpp
│   └── FCard 結構和 UCardDeck 類
├── Player.h/Player.cpp
│   └── APlayer 類
├── CardBattle.h/CardBattle.cpp
│   └── ACardBattle GameMode 類
├── CardGamePlayer.h/CardGamePlayer.cpp
│   └── ACardGamePlayer 玩家 Pawn 類
├── CardGame.h/CardGame.cpp
│   └── 遊戲模塊
└── CardGame.Build.cs
    └── 構建配置
```

## 許可和支持

本代碼基於 Unreal Engine 開發，遵循 Epic Games 許可協議。
