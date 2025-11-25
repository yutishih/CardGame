# 卡牌對戰系統 - 快速參考

## 🎮 核心概念

| 概念 | 說明 |
|------|------|
| **卡牌** | 1-30 編號的牌（FCard 結構） |
| **牌組** | 30 張完整的牌（UCardDeck 類） |
| **玩家** | 單一的遊戲參與者（APlayer 類） |
| **對戰** | 完整的遊戲流程（ACardBattle GameMode） |

## 📊 遊戲狀態機

```
Idle
  ↓ [StartGame]
Started
  ↓
DetermineFirstPlayer (隨機選擇先手)
  ↓
WaitingForPlayer0 ↔ WaitingForPlayer1
  ↓
ResolveRound (比較出牌數值)
  ↓
[雙方手牌都空?]
  ├─ Yes → GameOver
  └─ No → 下一回合
```

## 🔑 關鍵 API

### 遊戲控制
```cpp
StartGame()                    // 開始遊戲
EndGame()                      // 結束遊戲
PlayerPlayCard(PID, Index)    // 玩家出牌
```

### 狀態查詢
```cpp
GetBattleState()              // 遊戲狀態
GetCurrentTurnPlayerId()      // 當前玩家
GetPlayerHand(PID)            // 玩家手牌
GetPlayerScore(PID)           // 玩家分數
GetRemainingTurnTime()        // 剩餘時間
GetLastRoundInfo()            // 上一回合
GetWinner()                   // 獲勝者
```

## ⏱️ 時間流程

| 事件 | 觸發條件 | 結果 |
|------|--------|------|
| **回合開始** | 進入 WaitingForPlayer 狀態 | 計時器重置為 30 秒 |
| **計時進行中** | 每幀 Tick 更新 | CurrentTurnRemainingTime 遞減 |
| **玩家出牌** | 調用 PlayerPlayCard | 立即生效 |
| **時間到期** | CurrentTurnRemainingTime ≤ 0 | 系統隨機出牌 |
| **雙方都出牌** | 兩張卡都已出 | 進入 ResolveRound |

## 📈 分數計算

```
出牌比較:
  ├─ Player0 數值 > Player1 → Player0 得分 = Player0數值 + Player1數值
  ├─ Player1 數值 > Player0 → Player1 得分 = Player0數值 + Player1數值
  └─ 相等 → 雙方都不得分

最終判定:
  ├─ Player0 總分 > Player1 → 玩家0獲勝
  ├─ Player1 總分 > Player0 → 玩家1獲勝
  └─ 相等 → 平手
```

## 🔄 常見操作流程

### 啟動遊戲
```
1. 創建 ACardBattle GameMode
2. 調用 StartGame()
   ├─ InitializeGame()
   ├─ DetermineFirstPlayer()
   └─ GoToNextTurn()
3. 遊戲進入 WaitingForPlayer0 或 WaitingForPlayer1
```

### 玩家出牌
```
1. 玩家選擇手牌位置（0 到 HandSize-1）
2. 調用 PlayerPlayCard(PlayerId, CardIndex)
3. 出牌被記錄
4. 檢查雙方是否都已出牌
   ├─ Yes → ResolveRound()
   └─ No → 切換到另一玩家
```

### 回合結算
```
1. ResolveRound() 比較出牌
2. 更新獲勝玩家的分數
3. CheckGameOver() 檢查遊戲是否結束
   ├─ Yes → DetermineWinner()
   └─ No → GoToNextTurn()
```

## 🎯 牌組和手牌管理

### 牌組（UCardDeck）
```cpp
Initialize()      // 創建30張牌並打亂
DrawCards(N, Out) // 抽取N張牌
GetRemainingCount() // 獲取剩餘牌數
Reset()           // 重置牌組
```

### 玩家手牌（APlayer）
```cpp
DrawCardsToHand(N) // 從牌組抽N張到手牌
PlayCard(Index)    // 根據索引出牌
PlayCardRandom()   // 隨機出牌
GetHand()          // 獲取當前手牌
GetHandSize()      // 獲取手牌數量
HasCards()         // 檢查是否有手牌
```

## 🧪 測試和調試

### 使用 CardGameTester
```cpp
Tester->StartTestGame()        // 開始自動遊戲
Tester->SimulateGameRound()    // 執行一步
Tester->StopTestGame()         // 停止遊戲
Tester->SetAutoPlayDelay(2.0f) // 設置延遲
Tester->IsTestingGame()        // 檢查是否在測試中
```

### 查看日誌
- 所有重要事件都會輸出到 Output Log
- 按 `Windows` → `Developer Tools` → `Output Log` 查看

## 💾 關鍵數據結構

### FCard
```cpp
USTRUCT(BlueprintType)
struct FCard {
    int32 CardValue;  // 1-30
};
```

### FRoundInfo
```cpp
USTRUCT(BlueprintType)
struct FRoundInfo {
    FCard Player0Card;    // 玩家0的牌
    FCard Player1Card;    // 玩家1的牌
    int32 WinnerID;       // 獲勝者 (0, 1 或 -1)
};
```

### EBattleState
```cpp
enum class EBattleState : uint8 {
    Idle = 0,
    Started = 1,
    WaitingForPlayer0 = 2,
    WaitingForPlayer1 = 3,
    RoundEnd = 4,
    GameOver = 5
};
```

## 🐛 常見問題排查

| 問題 | 原因 | 解決 |
|------|------|------|
| 遊戲不啟動 | GameMode 未設置 | 檢查 Project Settings |
| 無法出牌 | 不是當前玩家 | 檢查 GetCurrentTurnPlayerId |
| CardIndex 超界 | 索引超出手牌範圍 | 使用 GetPlayerHand 檢查大小 |
| 遊戲未結束 | 仍有玩家有手牌 | 等待所有牌出完 |
| 時間不計時 | Tick 未啟用 | 檢查 PrimaryActorTick.bCanEverTick |

## 📱 Blueprint 集成

### 從 Blueprint 調用
```
Cast to CardBattle (Get Game Mode)
  ↓
Call StartGame / PlayerPlayCard / etc.
```

### 監聽遊戲狀態
```
Event Tick
  ├─ Get Game Mode (CardBattle)
  ├─ Get Battle State
  ├─ Get Player Score
  └─ Update UI
```

## 🔗 類依賴圖

```
ACardBattle (GameMode)
  ├─ [2] APlayer
  │   ├─ TArray<FCard> Hand
  │   └─ UCardDeck Deck
  └─ [2] UCardDeck
      └─ TArray<FCard> Deck

ACardGamePlayer (Pawn)
  └─ ACardBattle* BattleGameMode

ACardGameTester (Actor)
  └─ ACardBattle* BattleGameMode
```

## 📐 性能指標

- **初始化時間**: < 1ms
- **每幀計算**: O(1)
- **牌組打亂**: O(n) 其中 n=30
- **出牌操作**: O(n) 其中 n=手牌數
- **記憶體使用**: ~1KB 每個玩家

## 🎁 提供的工具

| 工具 | 類 | 用途 |
|------|------|------|
| 卡牌系統 | Card.h/cpp | 管理卡牌和牌組 |
| 玩家系統 | Player.h/cpp | 管理玩家狀態 |
| 遊戲邏輯 | CardBattle.h/cpp | 核心遊戲流程 |
| 玩家控制 | CardGamePlayer.h/cpp | 玩家輸入示例 |
| 測試工具 | CardGameTester.h/cpp | 自動測試和調試 |

---

**版本**: 1.0  
**更新**: 2025-11-26
