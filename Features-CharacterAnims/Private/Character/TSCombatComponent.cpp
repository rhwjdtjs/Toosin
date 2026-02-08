#include "Character/TSCombatComponent.h"

UTSCombatComponent::UTSCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;;
}


void UTSCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UTSCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
void UTSCombatComponent::LightAttack()
{
	UE_LOG(LogTemp, Log, TEXT("[CombatComponent] Light Attack Triggered"));
}

void UTSCombatComponent::HeavyAttack()
{
	UE_LOG(LogTemp, Log, TEXT("[CombatComponent] Heavy Attack Triggered"));
}
