// Copyright Epic Games, Inc. All Rights Reserved.


#include "CXUI.h"

#include "UI/ChatUI.h"
#include "GameState/CXGameState.h"
#include "PlayerState/CXPlayerState.h"

#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"

void UCXUI::OnNewChatMessage(const FString& InSender, const FString& InMessage)
{
	UI_Chat->OnNewChatMessage(InSender, InMessage);
}

void UCXUI::UpdateSpeed(float NewSpeed)
{
	// format the speed to KPH or MPH
	float FormattedSpeed = FMath::Abs(NewSpeed) * (bIsMPH ? 0.022f : 0.036f);

	// call the Blueprint handler
	OnSpeedUpdate(FormattedSpeed);
}

void UCXUI::UpdateGear(int32 NewGear)
{
	// call the Blueprint handler
	OnGearUpdate(NewGear);
}

void UCXUI::UpdateTime()
{
	if (ACXGameState* CXGS = Cast<ACXGameState>(UGameplayStatics::GetGameState(this)))
	{
		switch (CXGS->GetCurrentGameState())
		{
		case EGameState::Waiting:
		{
			UI_Timer->SetText(FText::FromString(FString::Printf(TEXT("Waiting... Game starts in %d seconds"), CXGS->GetRemainingTime())));

			break;
		}
		case EGameState::Playing:
		{
			UI_Timer->SetText(FText::FromString(FString::Printf(TEXT("Time Remaining: %d seconds"), CXGS->GetRemainingTime())));

			break;
		}
		case EGameState::Finished:
		{
			UI_Timer->SetText(FText::FromString(FString::Printf(TEXT("Game Finished!"))));
			ShowResult();

			break;
		}
		}
	}
}

void UCXUI::ShowResult()
{
	if (ACXGameState* CXGS = Cast<ACXGameState>(UGameplayStatics::GetGameState(this)))
	{
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
		{
			if (ACXPlayerState* CXPS = Cast<ACXPlayerState>(PC->PlayerState))
			{
				if (CXGS->Winners.Contains(CXPS))
				{
					UI_WinText->SetVisibility(ESlateVisibility::Visible);
					UI_LoseText->SetVisibility(ESlateVisibility::Hidden);
				}
				else
				{
					UI_WinText->SetVisibility(ESlateVisibility::Hidden);
					UI_LoseText->SetVisibility(ESlateVisibility::Visible);
				}
            }
		}
	}
}
