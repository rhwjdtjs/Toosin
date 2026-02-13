#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ATSEnemyController.generated.h"

/**
 * 🤖 적 AI 컨트롤러
 * Behavior Tree를 실행하고 Blackboard 값을 관리합니다.
 */
UCLASS()
class TOOSIN_API AATSEnemyController : public AAIController
{
	GENERATED_BODY()
	
public:
	AATSEnemyController();

protected:
	virtual void OnPossess(APawn* InPawn) override;

	// [설정]
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	class UBehaviorTree* BehaviorTreeAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	class UBlackboardData* BlackboardAsset;
	
public:
	// 공격 대상(플레이어) 감지 및 설정 시도
	void DetectPlayer();
};
