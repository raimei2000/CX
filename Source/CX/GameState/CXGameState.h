// CXGameState.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CXGameState.generated.h"


class ACXPlayerState;

UENUM(BlueprintType)
enum class EGameState : uint8
{
    Default,
	Waiting,
    Playing,
	Finished
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameplayTimerUpdateSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameStateUpdatedSignature, EGameState, NewGameState);

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

    FOnGameStateUpdatedSignature OnGameStateUpdate;

    FOnGameplayTimerUpdateSignature OnTimerUpdated;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_UpdateGameState)
    EGameState CurrentGameState = EGameState::Default;

	UPROPERTY(ReplicatedUsing = OnRep_UpdateTimer)
    int32 RemainingTime = 0;

private:
    UFUNCTION()
    void OnRep_UpdateGameState();

    UFUNCTION()
    void OnRep_UpdateTimer();
};
