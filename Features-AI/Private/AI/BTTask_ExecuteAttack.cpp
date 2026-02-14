#include "Toosin/Public/AI/BTTask_ExecuteAttack.h"
#include "AIController.h"
#include "Toosin/Public/Character/TSCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_ExecuteAttack::UBTTask_ExecuteAttack()
{
	NodeName = TEXT("Execute Attack");
}

EBTNodeResult::Type UBTTask_ExecuteAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	ATSCharacter* AIChar = Cast<ATSCharacter>(AIController->GetPawn());
	if (!AIChar) return EBTNodeResult::Failed;

	// [중요] 경직(Stunned) 상태이거나 사망 상태면 공격 불가
	if (AIChar->GetCharacterState() == ETSCharacterState::Stunned || AIChar->GetCharacterState() == ETSCharacterState::Dead)
	{
		return EBTNodeResult::Failed;
	}

	// [공격 전 회전 보정] 공격 시작 순간에만 타겟을 바라봄 (에임핵 방지)
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	
    // [사거리 체크] 너무 멀면 공격하지 않음 (300.0f 이상이면 실패)
    if (Blackboard)
    {
        AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TEXT("PlayerActor")));
        if (TargetActor)
        {
            float Distance = AIChar->GetDistanceTo(TargetActor);
            if (Distance > 300.0f) return EBTNodeResult::Failed;
        }
    }

    // [공격 전 회전 보정] ...
	AActor* TargetActor = Cast<AActor>(Blackboard ? Blackboard->GetValueAsObject(TEXT("PlayerActor")) : nullptr);
	if (TargetActor)
	{
		FVector Direction = TargetActor->GetActorLocation() - AIChar->GetActorLocation();
		Direction.Z = 0.0f; // 높이 무시
		FRotator NewRot = FRotationMatrix::MakeFromX(Direction).Rotator();
		AIChar->SetActorRotation(NewRot);
		
		// 주시 해제 (공격 중 회전 방지)
		AIController->ClearFocus(EAIFocusPriority::Gameplay);
	}

	// [랜덤 공격 패턴]
	if (AIChar->EnemyAttackMontages.Num() > 0)
	{
		int32 RandIndex = FMath::RandRange(0, AIChar->EnemyAttackMontages.Num() - 1);
		UAnimMontage* AttackMontage = AIChar->EnemyAttackMontages[RandIndex];
		
        if (AttackMontage)
        {
            // [중요] 공격 상태로 전환해야 무기 데미지가 들어감 (TSWeapon::OnBoxOverlap 참조)
            AIChar->SetCharacterState(ETSCharacterState::Attacking);
            
            AIChar->PlayAnimMontage(AttackMontage);
            UE_LOG(LogTemp, Warning, TEXT("[AI] Random Attack Executed: %s"), *AttackMontage->GetName());
        }
	}
	else
	{
		// 몽타주 배열이 비어있으면 기존 방식 (50% 확률로 경공격 또는 강공격)
		if (FMath::RandBool())
		{
			AIChar->LightAttack();
			UE_LOG(LogTemp, Warning, TEXT("[AI] Light Attack Executed"));
		}
		else
		{
			AIChar->HeavyAttack();
			UE_LOG(LogTemp, Warning, TEXT("[AI] Heavy Attack Executed"));
		}
	}

	return EBTNodeResult::Succeeded;
}
