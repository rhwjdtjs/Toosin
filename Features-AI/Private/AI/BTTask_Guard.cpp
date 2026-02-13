#include "Toosin/Public/AI/BTTask_Guard.h"
#include "AIController.h"
#include "Toosin/Public/Character/TSCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_Guard::UBTTask_Guard()
{
	NodeName = TEXT("Guard");
	bNotifyTick = true; // TickTask 활성화
}

EBTNodeResult::Type UBTTask_Guard::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	ATSCharacter* AIChar = Cast<ATSCharacter>(AIController ? AIController->GetPawn() : nullptr);

    if (AIChar)
    {
        // [회전 보정] 가드 시작 시 타겟 방향 바라보기 (엉뚱한 곳 가드 방지)
        UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
        AActor* TargetActor = Cast<AActor>(Blackboard ? Blackboard->GetValueAsObject(TEXT("PlayerActor")) : nullptr);
        if (TargetActor)
        {
            FVector Direction = TargetActor->GetActorLocation() - AIChar->GetActorLocation();
            Direction.Z = 0.0f; // 높이 무시
            
            // 즉시 회전 (몸 돌리기)
            FRotator NewRot = FRotationMatrix::MakeFromX(Direction).Rotator();
            AIChar->SetActorRotation(NewRot);

            // 시선 고정 (컨트롤러 회전 + 지속적인 추적)
            if (AIController)
            {
                AIController->SetControlRotation(NewRot);
                AIController->SetFocus(TargetActor);
            }
        }

        AIChar->GuardStart();
        // UE_LOG(LogTemp, Warning, TEXT("[AI] Guard Started"));
        return EBTNodeResult::InProgress; // 태스크 계속 유지
    }

	return EBTNodeResult::Failed;
}

void UBTTask_Guard::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	bool bPlayerAttacking = Blackboard ? Blackboard->GetValueAsBool(TEXT("bPlayerAttacking")) : false;

	// 플레이어 공격이 멈추면 가드 해제하고 태스크 종료
	if (!bPlayerAttacking)
	{
		AAIController* AIController = OwnerComp.GetAIOwner();
		ATSCharacter* AIChar = Cast<ATSCharacter>(AIController ? AIController->GetPawn() : nullptr);
		
		if (AIChar)
		{
			AIChar->GuardEnd();
			UE_LOG(LogTemp, Warning, TEXT("[BTTask_Guard] Guard Ended - Player stopped attacking"));
		}

		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
	else
	{
		// [가드 재시도 로직]
		// 플레이어는 공격 중인데, AI가 가드 상태가 아니라면(예: 피격 모션 끝나고 Idle로 돌아왔을 때)
		// 다시 가드를 올려야 함.
		AAIController* AIController = OwnerComp.GetAIOwner();
		ATSCharacter* AIChar = Cast<ATSCharacter>(AIController ? AIController->GetPawn() : nullptr);
		
		if (AIChar && AIChar->GetCharacterState() != ETSCharacterState::Blocking)
		{
			// 혹시 Stunned 상태가 아니라면 (즉, 움직일 수 있는 상태라면) 가드 시도
			if (AIChar->GetCharacterState() != ETSCharacterState::Stunned && AIChar->GetCharacterState() != ETSCharacterState::Dead)
			{
				AIChar->GuardStart();
				// UE_LOG(LogTemp, Warning, TEXT("[BTTask_Guard] Retry Guard (Recovered from Hit?)"));
			}
		}
	}
}

EBTNodeResult::Type UBTTask_Guard::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	ATSCharacter* AIChar = Cast<ATSCharacter>(AIController ? AIController->GetPawn() : nullptr);

	if (AIChar)
	{
		AIChar->GuardEnd();
		UE_LOG(LogTemp, Warning, TEXT("[BTTask_Guard] Aborted - Forcing Guard End"));
	}

	return Super::AbortTask(OwnerComp, NodeMemory);
}
