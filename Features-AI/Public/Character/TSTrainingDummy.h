#pragma once

#include "CoreMinimal.h"
#include "Toosin/Public/Character/TSCharacter.h" // ACharacter -> ATSCharacter 변경
#include "TSTrainingDummy.generated.h"

/**
 * 🥋 훈련용 더미 클래스 (Training Dummy)
 * ATSCharacter를 상속받아 기본 전투 기능을 공유하며,
 * Behavior Tree를 통해 제어되는 AI 캐릭터입니다.
 */
UCLASS()
class TOOSIN_API ATSTrainingDummy : public ATSCharacter
{
	GENERATED_BODY()

public:
	ATSTrainingDummy();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	// ATSCharacter의 TakeDamage, Die 등을 그대로 사용하므로 오버라이드 불필요할 수 있음.
	// 커스텀 로직이 필요한 경우만 오버라이드.
};
