#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Toosin/Public/AI/TSAITypes.h"
#include "TSAILearningSaveGame.generated.h"

/**
 * 💾 AI 학습 데이터 저장용 세이브 게임
 * 플레이어의 패턴 데이터를 영구 저장합니다.
 */
UCLASS()
class TOOSIN_API UTSAILearningSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UTSAILearningSaveGame();

	// 저장 슬롯 이름 (상수)
	static const FString SaveSlotName;
	static const int32 UserIndex;

	/** 누적된 플레이어 전투 패턴 데이터 */
	UPROPERTY(VisibleAnywhere, Category = "AI Learning")
	FPlayerPatternData AccumulatedData;

	/** 총 플레이한 라운드 수 (학습 성숙도 체크용) */
	UPROPERTY(VisibleAnywhere, Category = "AI Learning")
	int32 TotalRoundsPlayed;

    // [AI 가중치 - 학습 결과]
    // 저장된 데이터를 바탕으로 계산된 AI 행동 가중치들
    
    UPROPERTY(VisibleAnywhere, Category = "AI Learning")
    float AI_Aggressiveness; // AI 공격성 (0.0 수비적 ~ 1.0 공격적)

    UPROPERTY(VisibleAnywhere, Category = "AI Learning")
    float AI_ReactionTime; // AI 반응 속도 (초, 낮을수록 빠름)

    UPROPERTY(VisibleAnywhere, Category = "AI Learning")
    float AI_GuardProbability; // 가드 확률 (0.0 ~ 1.0)
};
