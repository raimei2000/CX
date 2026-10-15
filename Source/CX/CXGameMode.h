// CXGameMode.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CXGameMode.generated.h"

UCLASS(MinimalAPI)
class ACXGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACXGameMode();

protected:
	void StartWaitingCountdown();

	void HandleWaitingTimerTick();
	
	void StartPlayingState();

	void HandlePlayingTimerTick();

	void FinishGame();

    virtual void BeginPlay() override;

	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	virtual void PostLogin(APlayerController* NewPlayer) override;

private:
	bool IsPlayerStartAvailable(const APlayerStart* PlayerStart) const;

private:
	UPROPERTY(EditDefaultsOnly)
	FVector VehicleSpawnCheckHalfExtent = FVector(250.0, 120.0, 80.0);

	UPROPERTY(EditDefaultsOnly)
	FVector VehicleSpawnCheckOffset = FVector(0.0, 0.0, 80.0);

	FTimerHandle GameTimerHandle;

	bool bWaitingCountdownStarted = false;
};



