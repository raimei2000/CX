// CXMineItem.cpp


#include "Item/CXMineItem.h"

#include "CXPawn.h"

#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystem.h"
#include "Sound/Soundbase.h"
#include "Kismet/GameplayStatics.h"

ACXMineItem::ACXMineItem()
{
	bReplicates = true;
	SetNetUpdateFrequency(10);
	SetMinNetUpdateFrequency(5);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->SetupAttachment(SceneRoot);
	
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CollisionSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Vehicle, ECollisionResponse::ECR_Overlap);

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetupAttachment(SceneRoot);
	StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ACXMineItem::OnOverlapSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ACXPawn* Vehicle = Cast<ACXPawn>(OtherActor))
	{
		Vehicle->STCHitByMine();
		MulticastPlayExplosionCosmetic();

		Destroy();
	}
}

void ACXMineItem::MulticastPlayExplosionCosmetic_Implementation()
{
	if (HasAuthority() == true) { return; }

	UWorld* World = GetWorld();
	if (IsValid(ExplosionEffect))
	{
		UGameplayStatics::SpawnEmitterAtLocation(World, ExplosionEffect, GetActorTransform());
	}

	if (IsValid(ExplosionSound))
	{
		UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, GetActorLocation());
	}
}

void ACXMineItem::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		CollisionSphere->OnComponentBeginOverlap.AddUniqueDynamic(this, &ACXMineItem::OnOverlapSphereBeginOverlap);
	}
}
