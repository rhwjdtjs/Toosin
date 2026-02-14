#pragma once

#include "CoreMinimal.h"
#include "TSAITypes.generated.h"

/**
 * 📊 플레이어 전투 패턴 데이터 구조체
 * 라운드마다 누적되어 AI 학습(SaveGame)에 사용됩니다.
 */
USTRUCT(BlueprintType)
struct FPlayerPatternData
{
	GENERATED_BODY()

public:
	// [공격 성향]
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LightAttackCount = 0; // 경공격 횟수

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 HeavyAttackCount = 0; // 강공격 횟수

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalComboSegments = 0; // 콤보 누적 횟수 (예: 1타=1, 3타=3)

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ComboSequenceCount = 0; // 콤보 시도 횟수 (평균 콤보 길이 계산용)

	// [방어 성향]
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 HitCount = 0; // 피격 횟수 (유효타 허용)

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GuardSuccessCount = 0; // 가드 성공 횟수

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ParrySuccessCount = 0; // 패링 성공 횟수

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DodgeCount = 0; // 회피 사용 횟수

	// [이동 성향]
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AccumulatedDistance = 0.0f; // 적과의 거리 누적값

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DistanceSampleCount = 0; // 거리 샘플링 횟수 (평균 거리 계산용)

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SideStepCount = 0; // 좌우 이동(횡이동) 빈도

	// [통계 계산 헬퍼 함수]
	float GetAverageComboLength() const
	{
		return (ComboSequenceCount > 0) ? (float)TotalComboSegments / (float)ComboSequenceCount : 0.0f;
	}

	float GetAverageDistance() const
	{
		return (DistanceSampleCount > 0) ? AccumulatedDistance / (float)DistanceSampleCount : 0.0f;
	}
    
    // 공격적 성향 비율 (0.0 ~ 1.0)
    float GetAggressiveness() const
    {
        int32 TotalActions = LightAttackCount + HeavyAttackCount + GuardSuccessCount + SideStepCount;
        if (TotalActions == 0) return 0.5f;
        return (float)(LightAttackCount + HeavyAttackCount) / (float)TotalActions;
    }

	// 데이터 초기화
	void Reset()
	{
		LightAttackCount = 0;
		HeavyAttackCount = 0;
		TotalComboSegments = 0;
		ComboSequenceCount = 0;
		HitCount = 0;
		GuardSuccessCount = 0;
		ParrySuccessCount = 0;
		DodgeCount = 0;
		AccumulatedDistance = 0.0f;
		DistanceSampleCount = 0;
		SideStepCount = 0;
	}

    // 데이터 합산 (누적용)
    void operator+=(const FPlayerPatternData& Other)
    {
        LightAttackCount += Other.LightAttackCount;
        HeavyAttackCount += Other.HeavyAttackCount;
        TotalComboSegments += Other.TotalComboSegments;
        ComboSequenceCount += Other.ComboSequenceCount;
        HitCount += Other.HitCount;
        GuardSuccessCount += Other.GuardSuccessCount;
        ParrySuccessCount += Other.ParrySuccessCount;
        DodgeCount += Other.DodgeCount;
        AccumulatedDistance += Other.AccumulatedDistance;
        DistanceSampleCount += Other.DistanceSampleCount;
        SideStepCount += Other.SideStepCount;
    }
};
