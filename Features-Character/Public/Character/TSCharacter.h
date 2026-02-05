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

	UFUNCTION(BlueprintCallable, Category = "State")
	FORCEINLINE ETSCharacterState GetCharacterState() const { return CurrentState; } //현재 상태 반환 함수
	UFUNCTION(BlueprintCallable, Category = "State")
	void SetCharacterState(ETSCharacterState NewState); //현재 상태 설정 함수
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Dodge();
	void GuardStart();
	void GuardEnd();
	void LightAttack();
	void HeavyAttack();
	void Die();

protected:
	virtual void BeginPlay() override;
public:
    //카메라
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class USpringArmComponent* CameraBoom; //카메라 붐

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* FollowCamera; //팔로우 카메라

    //전투

    //데이터 테이블 설정
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats Data")
	UDataTable* CharacterStatsTable; //캐릭터 스탯 데이터 테이블

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats Data")
	FName CharacterStatRowName; //데이터 테이블의 행 이름

    //입력 액션
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputMappingContext* DefaultMappingContext; //기본 입력 매핑 컨텍스트

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* MoveAction; //이동 액션
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction; //시점 조작 액션
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* DodgeAction; //회피 액션
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* GuardAction; //가드 액션
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LightAttackAction; //일반공격 액션
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* HeavyAttackAction; //강공격 액션

protected:
    //스탯
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float MaxHealth; //최대 체력
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float CurrentHealth; //현재 체력
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float MaxStamina; //최대 스태미나
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float CurrentStamina; //현재 스태미나
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float WalkSpeed; //걷기 속도

    //상태머신
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	 ETSCharacterState CurrentState;

};
