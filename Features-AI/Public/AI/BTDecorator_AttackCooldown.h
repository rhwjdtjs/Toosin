#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_AttackCooldown.generated.h"

/**
 * ⏲️ 공격 쿨타임 체크 데코레이터
 * LastAttackTime 키를 확인하여, 일정 시간이 지났는지 검사
 */
UCLASS()
class TOOSIN_API UBTDecorator_AttackCooldown : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_AttackCooldown();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

public:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector LastAttackTimeKey;

	UPROPERTY(EditAnywhere, Category = "Condition")
	float CooldownTime = 2.0f;
};
