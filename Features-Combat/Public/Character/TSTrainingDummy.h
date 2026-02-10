#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Weapon/TSWeapon.h" // 무기 헤더 추가
#include "TSTrainingDummy.generated.h"

/**
 * 🥋 훈련용 더미 클래스 (Training Dummy)
 * 플레이어의 방어 및 패링 연습을 위해 주기적으로 공격하는 AI입니다.
 */
UCLASS()
class TOOSIN_API ATSTrainingDummy : public ACharacter
{
	GENERATED_BODY()

public:
	// 생성자 (Constructor)
	ATSTrainingDummy();

protected:
	// 게임 시작 시 호출 (Called when the game starts or when spawned)
	virtual void BeginPlay() override;

public:	
	// 매 프레임 호출 (Called every frame)
	virtual void Tick(float DeltaTime) override;

	// 데미지 처리 함수 (플레이어로부터 공격받을 때 호출됨)
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	// 공격 함수 (Attack Function)
	void AttackPlayer();

	// 공격 타이머 핸들 (Attack Timer Handle)
	FTimerHandle AttackTimerHandle;

	// 공격 주기 (Attack Interval)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackInterval = 3.0f; // 3초마다 공격

	// 공격 데미지 (Attack Damage)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackDamage = 10.0f;

	// 공격 몽타주 (Attack Montage) -> 블루프린트에서 할당
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	UAnimMontage* AttackMontage;

	// 공격 범위 (Attack Range)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackRange = 150.0f;

	// 현재 체력 (Current Health)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float CurrentHealth;

	// 최대 체력 (Max Health)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHealth = 100.0f;

	// 더미가 사용할 무기 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TSubclassOf<ATSWeapon> DummyWeaponClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	ATSWeapon* CurrentWeapon;

	// 공격 활성화/비활성화 (애니메이션 노티파이 대신 타이머로 흉내내기 위함)
	void EnableWeaponCollision();
	void DisableWeaponCollision();
	FTimerHandle CollisionTimerHandle;
};
