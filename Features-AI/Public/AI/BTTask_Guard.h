#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Guard.generated.h"

/**
 * 🛡️ 가드 실행 및 해제 태스크
 */
UCLASS()
class TOOSIN_API UBTTask_Guard : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_Guard();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
