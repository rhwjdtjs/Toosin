#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Toosin/Public/AI/TSAITypes.h"
#include "TSPlayerPatternComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TOOSIN_API UTSPlayerPatternComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UTSPlayerPatternComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// [데이터 수집 API]
	// 공격 시 호출 (ComboCount: 현재 콤보 차수)
	void RegisterAttack(bool bIsHeavy, int32 ComboCount);

	// 피격/방어 시 호출
	void RegisterDefense(bool bIsGuardSuccess, bool bIsParrySuccess, bool bIsHit);
	
	// 회피 시 호출
	void RegisterDodge();

    // 이동 분석 시 호출
    void RegisterMovement(float Distance, bool bSideStep);

	// 현재 라운드 데이터 반환
	FORCEINLINE const FPlayerPatternData& GetCurrentRoundData() const { return CurrentRoundData; }
    
    // 데이터 초기화 (새 라운드 시작 시)
    void ResetRoundData();

private:
	// 이번 라운드에서 수집된 데이터
	FPlayerPatternData CurrentRoundData;

	// [이동 분석용]
	float SampleTimer;
	const float SampleInterval = 0.5f; // 0.5초마다 거리/이동 샘플링
    
    UPROPERTY()
    class ACharacter* OwnerCharacter;
    
    UPROPERTY()
    class AActor* EnemyTarget; // 거리 측정 대상 (적)
};
