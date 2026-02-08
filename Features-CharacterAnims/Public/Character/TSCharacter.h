#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Engine/DataTable.h"
#include "InputActionValue.h"
#include "Toosin/Public/Headers/ATSWeaponTypes.h"
#include "TSCharacter.generated.h"

//캐릭터 기본 스탯 구조체
USTRUCT(BlueprintType)
struct FTSCharacterStats : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHealth = 100.f; //최대 체력

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxStamina = 100.f; //최대 스태미나

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WalkSpeed = 400.f; //걷기 속도

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RunSpeed = 600.f; //달리기 속도
};


UCLASS()
class TOOSIN_API ATSCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ATSCharacter(); //생성자
	virtual void Tick(float DeltaTime) override; //틱 함수
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override; //입력 설정 함수
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override; //데미지 처리 함수
	void InitializeStats(); //데이터 테이블에서 스탯 초기화 함수

	UFUNCTION(BlueprintCallable, Category = "Toosin|State")
	FORCEINLINE ETSCharacterState GetCharacterState() const { return CurrentState; } //현재 상태 반환 함수
	UFUNCTION(BlueprintCallable, Category = "Toosin|State")
	FORCEINLINE ETSWeaponType GetWeaponType() const { return CurrentWeaponType; }
	UFUNCTION(BlueprintCallable, Category = "Toosin|State")
	void SetCharacterState(ETSCharacterState NewState); //현재 상태 설정 함수
	UFUNCTION(BlueprintCallable, Category = "Toosin|State")
	void SetWeaponType(ETSWeaponType NewType); //현재 무기 타입 설정 함수

	UFUNCTION(BlueprintCallable, Category = "Toosin|Weapon")
	void EquipWeapon(class ATSWeapon* NewWeapon); //무기 장착 함수
	FORCEINLINE class ATSWeapon* GetCurrentWeapon() const { return CurrentWeapon; } //현재 장착된 무기 반환 함수

	//입력 처리 함수
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void LightAttack();
	void HeavyAttack();
	void Dodge();
	void GuardStart();
	void GuardEnd();
	void Die();

protected:
	virtual void BeginPlay() override;
	void PerformCombo(int32 SectionIndex); //추가
	void ResetAttackCooldown(); //추가
public:
	//카메라
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|Camera")
	class USpringArmComponent* CameraBoom; //카메라 붐

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|Camera")
	class UCameraComponent* FollowCamera; //팔로우 카메라

	//전투

	//데이터 테이블 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Toosin|Stats Data")
	UDataTable* CharacterStatsTable; //캐릭터 스탯 데이터 테이블

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Toosin|Stats Data")
	FName CharacterStatRowName; //데이터 테이블의 행 이름

	//입력 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Toosin|Input")
	class UInputMappingContext* DefaultMappingContext; //기본 입력 매핑 컨텍스트

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Toosin|Input")
	class UInputAction* MoveAction; //이동 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Toosin|Input")
	UInputAction* LookAction; //시점 조작 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Toosin|Input")
	UInputAction* LightAttackAction; //일반공격 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Toosin|Input")
	UInputAction* HeavyAttackAction; //강공격 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Toosin|Input")
	UInputAction* DodgeAction; //회피 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Toosin|Input")
	UInputAction* GuardAction; //가드 액션
protected:
	//스탯
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|Stats")
	float MaxHealth; //최대 체력
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|Stats")
	float CurrentHealth; //현재 체력
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|Stats")
	float MaxStamina; //최대 스태미나
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|Stats")
	float CurrentStamina; //현재 스태미나
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|Movement")
	float WalkSpeed; //걷기 속도

	//상태머신
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|State")
	ETSCharacterState CurrentState;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|State")
	ETSWeaponType CurrentWeaponType;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|Combat")
	bool bHasNextComboInput; //추가 (입력 버퍼)

	// [공격 후딜레이(쿨타임) 시스템]
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|Combat")
	bool bAttackCooldown; //추가 (쿨타임 상태)

	UPROPERTY(EditDefaultsOnly, Category = "Toosin|Combat")
	float AttackCooldownTime = 0.5f; //추가 (공격 후 0.5초간 공격 불가)

	FTimerHandle CooldownTimerHandle; //추가 (타이머 핸들)
	// 콤보 시스템
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|Combat")
	int32 ComboCount; // 현재 콤보 횟수

	UFUNCTION(BlueprintCallable, Category = "Toosin|Combat")
	void ResetCombo(); // 콤보 초기화 (AnimNotify에서 호출)

	UFUNCTION(BlueprintCallable, Category = "Toosin|Combat")
	void ContinueCombo(); // 콤보 체크 (AnimNotify에서 호출)
	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|Weapon")
	class ATSWeapon* CurrentWeapon;
	// 기본 무기 클래스 (에디터에서 BP_GreatSword 지정)
	UPROPERTY(EditDefaultsOnly, Category = "Toosin|Weapon")
	TSubclassOf<ATSWeapon> DefaultWeaponClass;
};
