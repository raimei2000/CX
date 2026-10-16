// PlayerState.cpp


#include "CXPlayerState.h"

#include "Net/UnrealNetwork.h"

static int32 GMyTestCoin = 0;

static FAutoConsoleVariableRef CVarAddCoinServer(
    TEXT("cheat.addcoin.server"),
    GMyTestCoin,
    TEXT("Increase Coin"),
    FConsoleVariableDelegate::CreateLambda([](IConsoleVariable* CVar)
        {
            UE_LOG(LogTemp, Warning, TEXT("Console Var changed! current: %d"), CVar->GetInt());
            UE_LOG(LogTemp, Warning, TEXT("Console Var changed! GMyTestCoin: %d"), GMyTestCoin);

            GWorld->GetFirstPlayerController()->GetPlayerState<ACXPlayerState>()->CTSAddCoin(GMyTestCoin);
            GMyTestCoin = 0;
        }),
    ECVF_Cheat
);

static FAutoConsoleVariableRef CVarAddCoinClient(
    TEXT("cheat.addcoin.client"),
    GMyTestCoin,
    TEXT("Increase Coin"),
    FConsoleVariableDelegate::CreateLambda([](IConsoleVariable* CVar)
        {
            UE_LOG(LogTemp, Warning, TEXT("Console Var changed! current: %d"), CVar->GetInt());
            UE_LOG(LogTemp, Warning, TEXT("Console Var changed! GMyTestCoin: %d"), GMyTestCoin);

            GWorld->GetFirstPlayerController()->GetPlayerState<ACXPlayerState>()->Coins += GMyTestCoin;
            GMyTestCoin = 0;
        }),
    ECVF_Cheat
);

void ACXPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(ThisClass, Coins, COND_OwnerOnly);
    DOREPLIFETIME_CONDITION(ThisClass, BonusMaxSpeed, COND_OwnerOnly);
}

void ACXPlayerState::CTSAddCoin_Implementation(const int32& Amount)
{
    AddCoins(Amount);
}

void ACXPlayerState::AddCoins(int32 Amount)
{
    // 매크로를 써서 서버에서만 빌드되도록 할 수도 있음.
    if (HasAuthority() == false || Amount <= 0) { return; }

    UE_LOG(LogTemp, Warning, TEXT("%s - AddCoins: %d > %d"), *GetPlayerName(), Coins, Amount);
    Coins += Amount;
}

bool ACXPlayerState::TryRemoveCoins(int32 Amount)
{
    if (HasAuthority() == false || Amount <= 0 || Coins < 5) { return false; }

    Coins -= Amount;
    return true;
}

void ACXPlayerState::AddBonusMaxSpeed(float Amount)
{
    if (HasAuthority() == false || Amount <= 0.f) { return; }

    BonusMaxSpeed += Amount;
}

void ACXPlayerState::ReduceBonusMaxSpeed(float Amount)
{
    if (HasAuthority() == false || Amount <= 0.f) { return; }

    BonusMaxSpeed -= Amount;
    BonusMaxSpeed = FMath::Max(-20, BonusMaxSpeed); // min cap -20
}

FString ACXPlayerState::GetPlayerStateInfo() const
{
    return FString::Printf(TEXT("Coins: %d, BonusMaxSpeed: %.2f"), Coins, BonusMaxSpeed);
}
