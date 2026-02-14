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

	AAIController* AIController = OwnerComp.GetAIOwner();
	ATSCharacter* AIChar = Cast<ATSCharacter>(AIController ? AIController->GetPawn() : nullptr);

	if (!AIChar) return;

	// 1. 플레이어 공격이 멈추면 가드 해제하고 태스크 종료
	if (!bPlayerAttacking)
	{
		AIChar->GuardEnd();
		// UE_LOG(LogTemp, Warning, TEXT("[BTTask_Guard] Guard Ended - Player stopped attacking"));
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// 2. 가드 중 로직 (Active Guard & Guard Restore)
	if (bPlayerAttacking) 
	{
		// [가드 상태 복구] 피격(Stunned) 등으로 가드가 풀렸다가 Idle로 돌아오면 다시 가드
		if (AIChar->GetCharacterState() == ETSCharacterState::Idle)
		{
			AIChar->GuardStart();
		}

		// [Active Guard] 거리 유지 및 회피 이동
		AActor* TargetActor = Cast<AActor>(Blackboard ? Blackboard->GetValueAsObject(TEXT("PlayerActor")) : nullptr);
		if (TargetActor)
		{
			float Distance = AIChar->GetDistanceTo(TargetActor);
			
			// 피격 중(몽타주 재생 중)이면 이동 금지
			bool bIsMontagePlaying = AIChar->GetMesh()->GetAnimInstance()->IsAnyMontagePlaying();

			// 너무 가까우면 (200cm 이내) 거리 벌리기 시도
			if (Distance < 200.0f && !bIsMontagePlaying && AIChar->GetCharacterState() == ETSCharacterState::Blocking)
			{
				// 이동 빈도 조절
				if (FMath::RandRange(0, 100) < 2) 
				{
					if (FMath::RandBool()) 
					{
						FVector DirToTarget = (TargetActor->GetActorLocation() - AIChar->GetActorLocation()).GetSafeNormal();
						FVector BackStepPos = AIChar->GetActorLocation() - (DirToTarget * 100.0f); 
						AIController->MoveToLocation(BackStepPos, -1.0f, false, true, false, false); 
					}
					else
					{
						FVector DirToTarget = (TargetActor->GetActorLocation() - AIChar->GetActorLocation()).GetSafeNormal();
						FVector RightDir = FVector::CrossProduct(DirToTarget, FVector::UpVector);
						FVector StrafePos = AIChar->GetActorLocation() + (RightDir * (FMath::RandBool() ? 150.0f : -150.0f));
						AIController->MoveToLocation(StrafePos, -1.0f, false, true, false, false);
					}
				}
			}
            
            // [추가] "내가 공격 중이지만 거리가 멀어졌다" -> 가드 풀고 반격 기회
            // 단, 피격 중(Stunned)이거나 다른 몽타주 실행 중이면 절대 풀면 안 됨
            // Controller에서는 350 이상이면 bPlayerAttacking을 false로 만듦.
            // 여기서도 350~360 정도를 기준으로 잡아서 확실히 멀어지면 풀도록 함.
            else if (Distance > 360.0f && !bIsMontagePlaying && AIChar->GetCharacterState() == ETSCharacterState::Blocking)
            {
                 // 안전 확인 후 가드 해제 -> 태스크 종료 -> BT 재평가 -> ExecuteAttack 실행
                 AIChar->GuardEnd();
                 FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
                 // UE_LOG(LogTemp, Warning, TEXT("[BTTask_Guard] Distance > 360, Breaking Guard to Attack"));
                 return;
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
