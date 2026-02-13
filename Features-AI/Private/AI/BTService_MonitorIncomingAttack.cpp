#include "Toosin/Public/AI/BTService_MonitorIncomingAttack.h"
#include "AIController.h"
#include "Toosin/Public/Character/TSCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"

UBTService_MonitorIncomingAttack::UBTService_MonitorIncomingAttack()
{
	NodeName = TEXT("Monitor Incoming Attack");
	Interval = 0.1f; // 0.1초마다 체크
	RandomDeviation = 0.05f;
}

void UBTService_MonitorIncomingAttack::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard) return;

	AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TEXT("PlayerActor")));
	ATSCharacter* TargetChar = Cast<ATSCharacter>(TargetActor);

	// [플레이어 감지 로직 강화]
	if (!TargetChar)
	{
		TargetActor = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
		if (TargetActor)
		{
			Blackboard->SetValueAsObject(TEXT("PlayerActor"), TargetActor);
			TargetChar = Cast<ATSCharacter>(TargetActor);
			UE_LOG(LogTemp, Warning, TEXT("[BTService] Player Auto-Detected & Set: %s"), *TargetActor->GetName());
		}
	}

	if (TargetChar)
	{

		// 플레이어가 죽었는가?
		if (TargetChar->GetCharacterState() == ETSCharacterState::Dead)
		{
			Blackboard->ClearValue(TEXT("PlayerActor"));
			Blackboard->ClearValue(TEXT("TargetActor")); // 필요하다면
			Blackboard->SetValueAsBool(TEXT("bPlayerAttacking"), false);
			UE_LOG(LogTemp, Warning, TEXT("[BTService] Player is Dead. AI stops tracking."));
			return;
		}

		// 플레이어가 공격 중인가?
		bool bIsAttacking = (TargetChar->GetCharacterState() == ETSCharacterState::Attacking);
		
		// Blackboard 업데이트
		Blackboard->SetValueAsBool(TEXT("bPlayerAttacking"), bIsAttacking);

		// [디버그 로그]
		// if (bIsAttacking) UE_LOG(LogTemp, Warning, TEXT("[BTService] Player is Attacking!"));
        
        // [거리 계산 추가]
        if (AAIController* AIController = OwnerComp.GetAIOwner())
        {
            if (APawn* ControlledPawn = AIController->GetPawn())
            {
                float Distance = ControlledPawn->GetDistanceTo(TargetChar);
                Blackboard->SetValueAsFloat(TEXT("CurrentDistance"), Distance);
                
                // [거리 체크] 거리가 멀면(예: 250 이상) 공격 중이라도 가드하지 않음
                if (Distance > 250.0f)
                {
                    bIsAttacking = false; 
                    // UE_LOG(LogTemp, Warning, TEXT("[BTService] Player Attacking but too far (%.0f)"), Distance);
                }
            }
        }
        
        // Blackboard 업데이트
        Blackboard->SetValueAsBool(TEXT("bPlayerAttacking"), bIsAttacking);
	}
}
