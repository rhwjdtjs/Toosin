#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_MonitorIncomingAttack.generated.h"

/**
 * 🛡️ 플레이어의 공격 상태를 모니터링하여 방어 여부 결정
 */
UCLASS()
class TOOSIN_API UBTService_MonitorIncomingAttack : public UBTService
{
	GENERATED_BODY()
	
public:
	UBTService_MonitorIncomingAttack();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
