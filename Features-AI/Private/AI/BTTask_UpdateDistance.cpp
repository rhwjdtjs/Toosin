#include "Toosin/Public/AI/BTTask_UpdateDistance.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Actor.h"

UBTTask_UpdateDistance::UBTTask_UpdateDistance()
{
	NodeName = TEXT("Update Distance");
}

EBTNodeResult::Type UBTTask_UpdateDistance::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	APawn* ControlledPawn = AIController->GetPawn();
	if (!ControlledPawn) return EBTNodeResult::Failed;

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TEXT("PlayerActor")));

	if (TargetActor)
	{
		float Dist = ControlledPawn->GetDistanceTo(TargetActor);
		Blackboard->SetValueAsFloat(TEXT("CurrentDistance"), Dist);
		// UE_LOG(LogTemp, Warning, TEXT("Distance: %f"), Dist);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}
