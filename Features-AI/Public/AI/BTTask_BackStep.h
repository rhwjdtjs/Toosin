#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_BackStep.generated.h"

/**
 * 🏃‍♂️ 플레이어 반대 방향으로 후퇴하는 태스크
 */
UCLASS()
class TOOSIN_API UBTTask_BackStep : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_BackStep();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

public:
	// 후퇴 거리 (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float BackStepDistance = 400.0f;
};
