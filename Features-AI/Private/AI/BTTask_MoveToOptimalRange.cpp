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
	
	// [이동 설정 변경] 거리에 따라 이동 모드 변경
    // 멀면(> 500): 자연스럽게 달리기 (OrientRotationToMovement)
    // 가까우면(<= 500): 플레이어 주시하며 횡이동 (Strafing / Lock-on Move)
    if (ATSCharacter* AIChar = Cast<ATSCharacter>(AIController->GetPawn()))
	{
         float DistToTarget = TargetActor ? AIChar->GetDistanceTo(TargetActor) : 0.0f;
         
         if (DistToTarget > 500.0f)
         {
             // Run Mode
             AIChar->bUseControllerRotationYaw = false; 
             AIChar->GetCharacterMovement()->bOrientRotationToMovement = true; 
             AIController->ClearFocus(EAIFocusPriority::Gameplay);
         }
         else
         {
             // Combat Mode (Strafe)
             AIChar->bUseControllerRotationYaw = true; 
             AIChar->GetCharacterMovement()->bOrientRotationToMovement = false; 
             if (TargetActor) AIController->SetFocus(TargetActor);
         }
	}

	if (TargetActor)
	{
		// [ML 지원] Blackboard에서 최적 공격 거리 가져오기 (기본값 200)
		float OptimalRange = 200.0f;
		float BBRange = Blackboard->GetValueAsFloat(TEXT("OptimalAttackRange"));
        if (BBRange > 10.0f) OptimalRange = BBRange;

		// AcceptableRadius를 OptimalRange 기준으로 설정 (약간의 오차 허용)
		float TargetRadius = OptimalRange - 50.0f; // 공격 사거리보다 살짝 안쪽으로 파고들기
		if (TargetRadius < 50.0f) TargetRadius = 50.0f;

		// [이동 개선] MoveToLocation (정적 위치) 대신 MoveToActor (동적 추적) 사용
        // 플레이어가 이동해도 계속 따라가도록 함
        
        // Strafe 모드일 때만 오프셋 적용하고 싶지만, MoveToActor는 오프셋 지원이 제한적임.
        // 하지만 "Tracking"이 더 중요하므로 MoveToActor 사용.
        
		EPathFollowingRequestResult::Type Result = AIController->MoveToActor(TargetActor, TargetRadius, true, true, true, 0, true);
		
		if (Result == EPathFollowingRequestResult::RequestSuccessful)
		{
			return EBTNodeResult::InProgress;
		}
		else if (Result == EPathFollowingRequestResult::AlreadyAtGoal)
		{
			return EBTNodeResult::Succeeded;
		}
	}

	return EBTNodeResult::Failed;
}

void UBTTask_MoveToOptimalRange::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

    AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController)
	{
        // [이동 중 공격 기회 포착]
        // 돌진 공격(Index 3, 4) 사거리(350~700)에 들어오면, 이동을 멈추고 공격 태스크로 넘김
        if (APawn* Pawn = AIController->GetPawn())
        {
            UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
            AActor* TargetActor = Cast<AActor>(Blackboard ? Blackboard->GetValueAsObject(TEXT("PlayerActor")) : nullptr);
            
            if (TargetActor)
            {
                float Dist = Pawn->GetDistanceTo(TargetActor);
                
                // [수정] 무한 멈춤 방지: 쿨타임 중일 때는 이동을 멈추지 않음!
                // 쿨타임인데 멈추면 -> BT에서 Attack 태스크 진입 불발(Decorator) -> 다시 Move 태스크 진입 -> 다시 멈춤 -> 무한루프
                
                float LastAttackTime = Blackboard->GetValueAsFloat(TEXT("LastAttackTime"));
                float CurrentTime = Pawn->GetWorld()->GetTimeSeconds();
                float CooldownDuration = 1.0f; // 기본 쿨타임 (안전장치)
                
                // 플레이어 공격패턴 데이터에서 가져와도 되지만, 여기선 간단히 1~2초 간격 유지 보장
                bool bIsCoolingDown = (CurrentTime - LastAttackTime) < CooldownDuration;

                // 350 ~ 430 사이면 공격 시도 (돌진 공격 사거리)
                if (Dist > 260.0f && Dist < 330.0f)
                {
                    // 쿨타임이 아닐 때만 멈춰서 공격! 쿨타임이면 계속 무빙(InProgress)
                    if (!bIsCoolingDown)
                    {
                         AIController->StopMovement();
                         FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
                         return;
                    }
                }
            }
        }

        // 기본 완료 조건 (도착)
		if (AIController->GetMoveStatus() == EPathFollowingStatus::Idle)
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
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
