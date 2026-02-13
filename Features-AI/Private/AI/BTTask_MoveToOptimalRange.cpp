#include "Toosin/Public/AI/BTTask_MoveToOptimalRange.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Actor.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "Toosin/Public/Character/TSCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

UBTTask_MoveToOptimalRange::UBTTask_MoveToOptimalRange()
{
	NodeName = TEXT("Move To Optimal Range");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_MoveToOptimalRange::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TEXT("PlayerActor")));
	
	// [이동 설정 변경] 추격 시 진행 방향을 보게 함 (Run Forward) -> 자연스러운 달리기
	if (ATSCharacter* AIChar = Cast<ATSCharacter>(AIController->GetPawn()))
	{
		AIChar->bUseControllerRotationYaw = false;
		AIChar->GetCharacterMovement()->bOrientRotationToMovement = true;
	}
	AIController->ClearFocus(EAIFocusPriority::Gameplay); // 주시 해제 (진행 방향 보게)

	if (TargetActor)
	{
		// [ML 지원] Blackboard에서 최적 공격 거리 가져오기 (기본값 200)
		float OptimalRange = 200.0f;
        // KeyExists는 없으므로 GetValueAsFloat 사용 (키가 없으면 0.0f 반환 가능성 있음 -> 0이면 기본값 사용)
		float BBRange = Blackboard->GetValueAsFloat(TEXT("OptimalAttackRange"));
        if (BBRange > 10.0f) // 유효한 값이면 적용
        {
            OptimalRange = BBRange;
        }

		// AcceptableRadius를 OptimalRange 기준으로 설정 (약간의 오차 허용)
		float TargetRadius = OptimalRange - 50.0f; // 공격 사거리보다 살짝 안쪽으로 파고들기
		if (TargetRadius < 50.0f) TargetRadius = 50.0f;

		// MoveToActor는 내부적으로 네비게이션을 사용
		EPathFollowingRequestResult::Type Result = AIController->MoveToActor(TargetActor, TargetRadius);
		
		if (Result == EPathFollowingRequestResult::RequestSuccessful)
		{
			return EBTNodeResult::InProgress; // 이동 완료 대기
		}
		else if (Result == EPathFollowingRequestResult::AlreadyAtGoal)
		{
			return EBTNodeResult::Succeeded; // 이미 목표 지점에 있음
		}
	}

	return EBTNodeResult::Failed;
}

void UBTTask_MoveToOptimalRange::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

    AAIController* AIController = OwnerComp.GetAIOwner(); // [수정] AIController 선언
	if (AIController && (AIController->GetMoveStatus() == EPathFollowingStatus::Idle))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

EBTNodeResult::Type UBTTask_MoveToOptimalRange::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController)
	{
		AIController->StopMovement();
		if (ATSCharacter* AIChar = Cast<ATSCharacter>(AIController->GetPawn()))
		{
			AIChar->bUseControllerRotationYaw = true;
			AIChar->GetCharacterMovement()->bOrientRotationToMovement = false;
			AIController->ClearFocus(EAIFocusPriority::Gameplay);
		}
	}
	return Super::AbortTask(OwnerComp, NodeMemory);
}

void UBTTask_MoveToOptimalRange::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
	
	// 필요시 설정 복구 (다른 태스크가 오버라이드 하겠지만 안전장치)
}
