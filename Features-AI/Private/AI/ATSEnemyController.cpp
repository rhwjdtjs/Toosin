#include "Toosin/Public/AI/ATSEnemyController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

AATSEnemyController::AATSEnemyController()
{
	
}

void AATSEnemyController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (BehaviorTreeAsset && BlackboardAsset)
	{
		// Blackboard 초기화 (TObjectPtr 호환성 문제 해결을 위해 로컬 변수 사용)
		UBlackboardComponent* BlackboardComp = nullptr;
		if (UseBlackboard(BlackboardAsset, BlackboardComp))
		{
			// 필요한 경우 멤버 변수에 재할당 (UseBlackboard가 내부적으로 설정하므로 필수는 아닐 수 있음)
			Blackboard = BlackboardComp;
			// Behavior Tree 실행
			RunBehaviorTree(BehaviorTreeAsset);
			UE_LOG(LogTemp, Warning, TEXT("[AIController] BT Started for %s"), *InPawn->GetName());
			
            // [중요] BT 시작 시 LastAttackTime을 현재 시간으로 설정하여
            // 스폰 직후 바로 공격하는 것을 방지 (쿨타임 적용)
            if (Blackboard)
            {
                Blackboard->SetValueAsFloat(TEXT("LastAttackTime"), GetWorld()->GetTimeSeconds());
            }

			// 초기 플레이어 감지 시도
			DetectPlayer();
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[AIController] MISSING ASSETS! BehaviorTree: %s, Blackboard: %s"), 
			BehaviorTreeAsset ? TEXT("Valid") : TEXT("NULL"), 
			BlackboardAsset ? TEXT("Valid") : TEXT("NULL"));
	}
}

void AATSEnemyController::DetectPlayer()
{
	// 간단하게 플레이어 0번을 타겟으로 설정 (1v1 아레나 가정)
	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (PlayerCharacter && Blackboard)
	{
		Blackboard->SetValueAsObject(TEXT("PlayerActor"), PlayerCharacter);
        
        // [주시 설정] 항상 플레이어를 바라보도록 설정 (몸을 돌리지 않음)
        SetFocus(PlayerCharacter);
        
		UE_LOG(LogTemp, Warning, TEXT("[AIController] Player Detected: %s"), *PlayerCharacter->GetName());
	}
}
