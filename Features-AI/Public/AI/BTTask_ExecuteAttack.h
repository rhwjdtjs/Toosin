#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ExecuteAttack.generated.h"

/**
 * ⚔️ 공격을 실행하는 태스크 (Light/Heavy 랜덤)
 */
UCLASS()
class TOOSIN_API UBTTask_ExecuteAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_ExecuteAttack();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
