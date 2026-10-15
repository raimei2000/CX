// CXItemSpawner.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CXItemSpawner.generated.h"

class UBillboardComponent;

UCLASS()
class CX_API ACXItemSpawner : public AActor
{
	GENERATED_BODY()
	
public:
	ACXItemSpawner();

protected:
	virtual void BeginPlay() override;

	void SpawnItem();

	UFUNCTION()
    void OnItemDestroyed(AActor* DestroyedActor);

public:
	virtual void Tick(float DeltaTime) override;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Spawner)
    TObjectPtr<UBillboardComponent> BillboardComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Spawner)
	TSubclassOf<AActor> ItemClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Spawner)
	float SpawnInterval = 2.f;

protected:
	UPROPERTY()
    TObjectPtr<AActor> SpawnedItem;

	FTimerHandle SpawnTimerHandle;
};
