// CXItemSpawner.cpp


#include "Item/CXItemSpawner.h"

#include "Kismet/GameplayStatics.h"
#include "Components/BillboardComponent.h"

ACXItemSpawner::ACXItemSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

    BillboardComponent = CreateDefaultSubobject<UBillboardComponent>(TEXT("BillboardComponent"));
}

void ACXItemSpawner::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		SpawnItem();
	}
}

void ACXItemSpawner::SpawnItem()
{
    if (HasAuthority() == false || IsValid(ItemClass) == false) { return; }

    UWorld* World = GetWorld();
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;

    SpawnedItem = World->SpawnActorDeferred<AActor>(ItemClass, GetActorTransform(), this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	SpawnedItem->OnDestroyed.AddDynamic(this, &ACXItemSpawner::OnItemDestroyed);
	UGameplayStatics::FinishSpawningActor(SpawnedItem, GetActorTransform());
}

void ACXItemSpawner::OnItemDestroyed(AActor* DestroyedActor)
{
    if (HasAuthority() == false) { return; }

	SpawnedItem = nullptr;
    GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &ACXItemSpawner::SpawnItem, SpawnInterval, false);
}
