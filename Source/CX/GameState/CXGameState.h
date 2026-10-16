// CXGameState.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CXGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameplayStateSignature);

class ACXPlayerState;

UENUM(BlueprintType)
enum class EGameState : uint8
{
	Waiting,
    Playing,
	Finished
};

/**
 * 
 */
UCLASS()
class CX_API ACXGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    void SetGameState(EGameState NewGameState);

	void SetRemainingTime(int32 NewRemainingTime);

    FORCEINLINE int32 GetRemainingTime() const { return RemainingTime; }

    FORCEINLINE EGameState GetCurrentGameState() const { return CurrentGameState; }

    UPROPERTY(Replicated)
	TArray<TObjectPtr<ACXPlayerState>> Winners;

    FOnGameplayStateSignature OnGameStateUpdate;

    FOnGameplayStateSignature OnTimerUpdated;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_UpdateGameState)
    EGameState CurrentGameState = EGameState::Waiting;

	UPROPERTY(ReplicatedUsing = OnRep_UpdateTimer)
    int32 RemainingTime = 0;

private:
    UFUNCTION()
    void OnRep_UpdateGameState();

    UFUNCTION()
    void OnRep_UpdateTimer();
};
