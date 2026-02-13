#include "Toosin/Public/AI/BTDecorator_AttackCooldown.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UBTDecorator_AttackCooldown::UBTDecorator_AttackCooldown()
{
	NodeName = TEXT("Can Attack? (Cooldown)");
    
    // 키 필터링 (Float 타입만 허용)
    LastAttackTimeKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_AttackCooldown, LastAttackTimeKey));
}

bool UBTDecorator_AttackCooldown::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard) return false;

    // 마지막 공격 시간 가져오기
	float LastTime = Blackboard->GetValueAsFloat(LastAttackTimeKey.SelectedKeyName);
	float CurrentTime = GetWorld()->GetTimeSeconds();

    // 0.0f이면 (아직 공격 안함) -> 공격 가능
    if (LastTime <= 0.0f) return true;

    // 경과 시간 체크
	return (CurrentTime - LastTime) >= CooldownTime;
}
