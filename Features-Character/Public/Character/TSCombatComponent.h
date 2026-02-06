#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TSCombatComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TOOSIN_API UTSCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UTSCombatComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void LightAttack(); // 가벼운 공격
	void HeavyAttack(); // 강한 공격
};
