#include "Toosin/Public/AI/BTTask_BackStep.h"
#include "AIController.h"
#include "Toosin/Public/Character/TSCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

UBTTask_BackStep::UBTTask_BackStep()
{
	NodeName = TEXT("Back Step");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_BackStep::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	ATSCharacter* AIChar = Cast<ATSCharacter>(AIController->GetPawn());
	if (!AIChar) return EBTNodeResult::Failed;

	// 블랙보드에서 플레이어 찾기
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AActor* TargetActor = Cast<AActor>(Blackboard ? Blackboard->GetValueAsObject(TEXT("PlayerActor")) : nullptr);

	if (!TargetActor) return EBTNodeResult::Failed;

	// 플레이어 -> AI 방향 벡터 (회피 방향)
	FVector Direction = AIChar->GetActorLocation() - TargetActor->GetActorLocation();
	Direction.Z = 0.0f; // 높이 무시
	Direction.Normalize();

	// 후퇴 목표 위치
	FVector Destination = AIChar->GetActorLocation() + (Direction * BackStepDistance);

	// 네비게이션 시스템을 통해 갈 수 있는 위치인지 확인 및 보정
	UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (NavSystem)
	{
		FNavLocation ProjectedLoc;
		if (NavSystem->ProjectPointToNavigation(Destination, ProjectedLoc))
		{
			Destination = ProjectedLoc.Location;
		}
	}

	// 이동 명령 (보통 후퇴는 빠르게)
	// AIController->StopMovement(); // 기존 이동 중지
	
	// [이동 설정 변경] 후퇴 시 플레이어를 바라보게 함 (Strafe)
	AIChar->bUseControllerRotationYaw = true;
	AIChar->GetCharacterMovement()->bOrientRotationToMovement = false;
	AIController->SetFocus(TargetActor);

	// MoveToLocation 사용 (bUsePathfinding = true)
	EPathFollowingRequestResult::Type Result = AIController->MoveToLocation(Destination, 50.0f, true, true, false, true);

	if (Result == EPathFollowingRequestResult::RequestSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AI] BackStep Started - Distance: %.1f"), BackStepDistance);
		return EBTNodeResult::InProgress; // 이동 완료 대기
	}

	return EBTNodeResult::Failed;
}

void UBTTask_BackStep::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController && (AIController->GetMoveStatus() == EPathFollowingStatus::Idle))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

void UBTTask_BackStep::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

	// [이동 설정 복구] 태스크 종료 시 다시 진행 방향을 보게 설정
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController)
	{
		ATSCharacter* AIChar = Cast<ATSCharacter>(AIController->GetPawn());
		if (AIChar)
		{
			AIChar->bUseControllerRotationYaw = false;
			AIChar->GetCharacterMovement()->bOrientRotationToMovement = true;
			AIController->ClearFocus(EAIFocusPriority::Gameplay); // Focus 해제 (선택사항, 계속 주시하려면 유지)
		}
	}
}
