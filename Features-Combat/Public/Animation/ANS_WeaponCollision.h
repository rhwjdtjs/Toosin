#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_WeaponCollision.generated.h"

/**
 * ⚔️ 무기 콜리전 제어용 노티파이 스테이트
 * 몽타주 트랙에 배치하여, 해당 구간 동안 무기의 콜리전을 활성화합니다.
 */
UCLASS()
class TOOSIN_API UANS_WeaponCollision : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	// 노티파이 시작 시 호출 (콜리전 켜기)
	virtual void NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	// 노티파이 종료 시 호출 (콜리전 끄기)
	virtual void NotifyEnd(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, const FAnimNotifyEventReference& EventReference) override;
	
};
