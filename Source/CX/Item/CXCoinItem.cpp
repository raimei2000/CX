// CXCoinItem.cpp


#include "Item/CXCoinItem.h"

#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystem.h"
#include "Sound/Soundbase.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerState/CXPlayerState.h"
#include "CXPawn.h"

ACXCoinItem::ACXCoinItem()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetNetUpdateFrequency(1.f);
    SetMinNetUpdateFrequency(1.f);

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
    OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
    OverlapSphere->SetupAttachment(SceneRoot);
	OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &ACXCoinItem::OnOverlapSphereBeginOverlap);
    OverlapSphere->SetSphereRadius(100.f);
    OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    OverlapSphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Vehicle, ECollisionResponse::ECR_Overlap);
    OverlapSphere->SetGenerateOverlapEvents(true);

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
    ItemMesh->SetupAttachment(SceneRoot);
    ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ACXCoinItem::BeginPlay()
{
	Super::BeginPlay();
	
    if (HasAuthority())
    {
        OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &ACXCoinItem::OnOverlapSphereBeginOverlap);
    }
}

void ACXCoinItem::OnOverlapSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (HasAuthority() == false) { return; }

    ACXPawn* VehiclePawn = Cast<ACXPawn>(OtherActor);
    if (IsValid(VehiclePawn) == false) { return; }

    ACXPlayerState* PS = Cast<ACXPlayerState>(VehiclePawn->GetPlayerState());
    PS->AddCoins(10);

    MulticastPickupEffects();

    Destroy();
}

void ACXCoinItem::MulticastPickupEffects_Implementation()
{
    UWorld* World = GetWorld();
    
    if (IsValid(PickupParticle))
    {
        UGameplayStatics::SpawnEmitterAtLocation(World, PickupParticle, GetActorLocation());
    }

    if (IsValid(PickupSound))
    {
        UGameplayStatics::PlaySoundAtLocation(World, PickupSound, GetActorLocation());
    }
}

void ACXCoinItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

