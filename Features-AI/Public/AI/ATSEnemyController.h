#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Toosin/Public/AI/TSAITypes.h" // 추가
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
    virtual void Tick(float DeltaTime) override; // 추가

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
    
    // [AI 학습] 데이터 업데이트 및 성향 조정
    void UpdatePlayerData(const FPlayerPatternData& NewData);
    
    // [AI 성향 - Blackboard용]
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Learning")
    float AI_Aggressiveness = 0.5f; // 0.0(수비적) ~ 1.0(공격적)

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Learning")
    float AI_ReactionTime = 0.3f; // 반응 속도 (초)

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Learning")
    float AI_GuardProbability = 0.5f; // 가드 확률
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Learning")
    float AI_HeavyAttackPreference = 0.5f; // 강공격 선호도 (0.0=경공격 위주, 1.0=강공격 위주)

private:
   FPlayerPatternData PlayerData;
};
