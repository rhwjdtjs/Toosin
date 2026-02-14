#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_MoveToOptimalRange.generated.h"

/**
 * 🏃‍♂️ 최적의 공격 거리로 이동하는 태스크
 */
UCLASS()
class TOOSIN_API UBTTask_MoveToOptimalRange : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_MoveToOptimalRange();

	UPROPERTY(EditAnywhere, Category = "AI")
	float AcceptableRadius = 50.0f; // 목표 지점 허용 오차

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
};
