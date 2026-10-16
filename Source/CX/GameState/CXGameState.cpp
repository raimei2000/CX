// CXGameState.h


#include "GameState/CXGameState.h"

#include "Net/UnrealNetwork.h"

void ACXGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ThisClass, CurrentGameState);
    DOREPLIFETIME(ThisClass, RemainingTime);
    DOREPLIFETIME(ThisClass, Winners);
}

void ACXGameState::SetGameState(EGameState NewGameState)
{
    if (HasAuthority() == false) { return; }
    CurrentGameState = NewGameState;
}

void ACXGameState::SetRemainingTime(int32 NewRemainingTime)
{
    if (HasAuthority() == false) { return; }
    RemainingTime = FMath::Max(0.f, NewRemainingTime);
}

void ACXGameState::OnRep_UpdateTimerUI()
{
    OnTimerUpdated.Broadcast();
}
