#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ATSArenaGameMode.generated.h"

/**
 * 🏟️ 아레나 게임 모드
 * 1v1 라운드 기반의 전투를 관리합니다.
 */
UCLASS()
class TOOSIN_API ATSArenaGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATSArenaGameMode();

	virtual void StartPlay() override;

	// [라운드 관리]
	UFUNCTION(BlueprintCallable, Category = "Arena")
	void StartRound();

	UFUNCTION(BlueprintCallable, Category = "Arena")
	void EndRound(AActor* Winner);

	UFUNCTION(BlueprintCallable, Category = "Arena")
	void ResetRound();

	// [라운드 설정]
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arena")
	float RoundStartDelay = 3.0f; // 라운드 시작 전 대기 시간

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arena")
	float RoundEndDelay = 3.0f; // 라운드 종료 후 재시작 대기 시간

	// [스폰 포인트 태그]
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arena")
	FName PlayerSpawnTag = TEXT("Spawn_Player");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arena")
	FName EnemySpawnTag = TEXT("Spawn_Enemy");

    // [적 클래스]
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Arena")
    TSubclassOf<class APawn> EnemyClass;

protected:
	FTimerHandle RoundTimerHandle;
	
	// 현재 라운드 진행 중 여부
	bool bIsRoundActive = false;
};
