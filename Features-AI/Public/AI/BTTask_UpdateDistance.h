#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_UpdateDistance.generated.h"

/**
 * 📏 플레이어와의 거리를 계산하여 Blackboard에 업데이트하는 태스크
 */
UCLASS()
class TOOSIN_API UBTTask_UpdateDistance : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_UpdateDistance();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
