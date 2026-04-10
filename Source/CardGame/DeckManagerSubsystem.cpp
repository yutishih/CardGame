#include "DeckManagerSubsystem.h"
#include "DeckSaveGame.h"
#include "Kismet/GameplayStatics.h"

void UDeckManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadDeck();
}

int32 UDeckManagerSubsystem::GetCardCount(FName RowName) const
{
	int32 Count = 0;
	for (const FName& Name : PlayerDeck)
	{
		if (Name == RowName) ++Count;
	}
	return Count;
}

void UDeckManagerSubsystem::SetPlayerDeck(const TArray<FName>& NewDeck)
{
	PlayerDeck = NewDeck;
}

bool UDeckManagerSubsystem::AddCard(FName RowName)
{
	if (PlayerDeck.Num() >= MaxDeckSize)
	{
		UE_LOG(LogTemp, Warning, TEXT("DeckManager: 牌組已達上限 %d 張"), MaxDeckSize);
		return false;
	}

	if (GetCardCount(RowName) >= MaxCopiesPerCard)
	{
		UE_LOG(LogTemp, Warning, TEXT("DeckManager: %s 已達同張上限 %d 張"), *RowName.ToString(), MaxCopiesPerCard);
		return false;
	}

	PlayerDeck.Add(RowName);
	UE_LOG(LogTemp, Log, TEXT("DeckManager: 加入 %s，目前 %d 張"), *RowName.ToString(), PlayerDeck.Num());
	return true;
}

bool UDeckManagerSubsystem::RemoveCard(FName RowName)
{
	// 從後往前找，移除最後一個相符的
	for (int32 i = PlayerDeck.Num() - 1; i >= 0; --i)
	{
		if (PlayerDeck[i] == RowName)
		{
			PlayerDeck.RemoveAt(i);
			UE_LOG(LogTemp, Log, TEXT("DeckManager: 移除 %s，目前 %d 張"), *RowName.ToString(), PlayerDeck.Num());
			return true;
		}
	}
	return false;
}

void UDeckManagerSubsystem::ClearDeck()
{
	PlayerDeck.Empty();
}

void UDeckManagerSubsystem::SaveDeck()
{
	UDeckSaveGame* SaveGame = Cast<UDeckSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UDeckSaveGame::StaticClass())
	);

	if (SaveGame)
	{
		SaveGame->PlayerDeckRowNames = PlayerDeck;
		UGameplayStatics::SaveGameToSlot(SaveGame, UDeckSaveGame::SaveSlotName, UDeckSaveGame::SaveUserIndex);
		UE_LOG(LogTemp, Log, TEXT("DeckManager: 牌組已儲存，共 %d 張"), PlayerDeck.Num());
	}
}

void UDeckManagerSubsystem::LoadDeck()
{
	if (UGameplayStatics::DoesSaveGameExist(UDeckSaveGame::SaveSlotName, UDeckSaveGame::SaveUserIndex))
	{
		UDeckSaveGame* SaveGame = Cast<UDeckSaveGame>(
			UGameplayStatics::LoadGameFromSlot(UDeckSaveGame::SaveSlotName, UDeckSaveGame::SaveUserIndex)
		);

		if (SaveGame)
		{
			PlayerDeck = SaveGame->PlayerDeckRowNames;
			UE_LOG(LogTemp, Log, TEXT("DeckManager: 讀取牌組成功，共 %d 張"), PlayerDeck.Num());
			return;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("DeckManager: 無存檔，使用空牌組"));
}
