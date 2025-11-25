# 卡牌對戰遊戲系統文檔

## 項目概述

這是一個基於 Unreal Engine 的兩人卡牌對戰遊戲。遊戲系統已全部實現，包括卡牌管理、玩家管理、遊戲流程控制和分數計算。

## 核心系統架構

### 1. 卡牌系統 (Card.h/Card.cpp)

#### FCard 結構
- 代表單一卡牌
- 每張卡牌有一個 1-30 的數值
- `CardValue`: 卡牌的數值

#### UCardDeck 類
- 管理一個完整的卡牌組 (30 張牌)
- 初始化: 創建 30 張卡牌 (編號 1-30)
- 打亂: 使用 Fisher-Yates 算法隨機打亂牌組
- 抽牌: `DrawCards()` 從牌組中抽取指定數量的牌

### 2. 玩家系統 (Player.h/Player.cpp)

#### APlayer 類
- 代表遊戲中的一個玩家
- **主要屬性**:
  - `PlayerID`: 玩家編號 (0 或 1)
  - `Hand`: 玩家的手牌列表
  - `Score`: 玩家的累計分數
  - `Deck`: 玩家的牌組

- **主要方法**:
  - `Initialize(PlayerId)`: 初始化玩家
  - `DrawCardsToHand(NumberOfCards)`: 從牌組抽牌到手牌
  - `PlayCard(CardIndex)`: 根據索引出牌
  - `PlayCardRandom()`: 系統自動隨機出牌
  - `AddScore(Points)`: 增加分數
  - `HasCards()`: 檢查是否還有手牌

### 3. 遊戲對戰系統 (CardBattle.h/CardBattle.cpp)

#### EBattleState 枚舉
遊戲的各種狀態：
- `Idle`: 空閒狀態
- `Started`: 遊戲已開始
- `WaitingForPlayer0`: 等待玩家 0 出牌
- `WaitingForPlayer1`: 等待玩家 1 出牌
- `RoundEnd`: 回合結束
- `GameOver`: 遊戲結束

#### FRoundInfo 結構
記錄每回合的信息：
- `Player0Card`: 玩家 0 出的牌
- `Player1Card`: 玩家 1 出的牌
- `WinnerID`: 本回合的獲勝者 (0, 1 或 -1)

#### ACardBattle 類
核心遊戲模式，負責整個遊戲流程

**遊戲流程**:

1. **遊戲初始化** (`StartGame()`)
   - 創建 2 個玩家和 2 個牌組
   - 每個牌組初始化 30 張牌 (1-30)
   - 每個玩家從牌組抽 10 張牌

2. **隨機決定先手** (`DetermineFirstPlayer()`)
   - 隨機選擇 0 或 1 作為先手玩家

3. **遊戲循環**
   - 輪流出牌（每回合 30 秒限制）
   - 時間到未出牌自動隨機出牌
   - 當雙方都出牌後進入結算

4. **回合結算** (`ResolveRound()`)
   - 比較雙方卡牌數值
   - 數值大的玩家獲得本回合的分數 = 雙方卡牌數值之和
   - 如果平手，不計分

5. **遊戲結束檢查** (`CheckGameOver()`)
   - 當雙方手牌都空時，遊戲結束
   - 計算最終分數並確定獲勝者

## 使用方法

### 基本使用

```cpp
// 在 GameMode 中調用
void StartGame();        // 開始遊戲
void EndGame();         // 結束遊戲
void PlayerPlayCard(int32 PlayerId, int32 CardIndex); // 玩家出牌
```

### 查詢遊戲狀態

```cpp
EBattleState GetBattleState() const;           // 獲取當前遊戲狀態
int32 GetCurrentTurnPlayerId() const;          // 獲取當前出牌玩家
const TArray<FCard>& GetPlayerHand(int32 PlayerId) const; // 獲取玩家手牌
int32 GetPlayerScore(int32 PlayerId) const;   // 獲取玩家分數
float GetRemainingTurnTime() const;            // 獲取回合剩餘時間
const FRoundInfo& GetLastRoundInfo() const;    // 獲取上一回合信息
int32 GetWinner() const;                       // 獲取遊戲最終獲勝者
```

## 遊戲規則

### 牌組構成
- 每個牌組 30 張牌，編號 1-30
- 遊戲開始時每個玩家從牌組中抽 10 張牌

### 出牌規則
- 每回合只能出一張牌
- 手牌順序由玩家決定 (通過 CardIndex 參數)
- 如果 30 秒內未出牌，系統自動隨機出牌

### 分數規則
- 比較雙方卡牌數值
- 數值大的玩家獲得分數 = 雙方卡牌數值之和
- 平手不計分

### 勝負判定
- 當雙方手牌都空時，遊戲結束
- 累計分數最高的玩家獲勝
- 分數相同則為平手

## 代碼結構

```
Source/CardGame/
├── Card.h/Card.cpp          - 卡牌和牌組系統
├── Player.h/Player.cpp      - 玩家系統
├── CardBattle.h/CardBattle.cpp - 遊戲對戰邏輯
├── CardGame.h/CardGame.cpp  - 遊戲模塊
└── CardGame.Build.cs        - 構建配置
```

## 下一步

後續可以開發的功能：
1. UI 系統 - 顯示手牌、分數、計時器等
2. 輸入系統 - 玩家操作卡牌出牌
3. 網絡同步 - 多人在線遊戲支持
4. 音效和視覺效果 - 增強遊戲體驗
5. 牌組構建系統 - 玩家自定義牌組

## 編譯和運行

1. 打開 CardGame.sln
2. 編譯解決方案
3. 在 UE5 編輯器中設置 ACardBattle 為 GameMode
4. 在 Level 中放置遊戲模式並調用 `StartGame()` 開始遊戲
