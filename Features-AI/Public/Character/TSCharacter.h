#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Toosin/Public/Character/TSCombatComponent.h" // 컴포넌트 헤더 추가
#include "Toosin/Public/Headers/ATSWeaponTypes.h"
#include "TSCharacter.generated.h"
// 캐릭터 기본 스탯 구조체
USTRUCT(BlueprintType)
struct FTSCharacterStats : public FTableRowBase {
 
    
    GENERATED_BODY()

public:
      UPROPERTY(EditAnywhere, BlueprintReadWrite)
      float MaxHealth = 100.f; // 최대 체력

     UPROPERTY(EditAnywhere, BlueprintReadWrite)
     float MaxStamina = 100.f; // 최대 스태미나

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
      float WalkSpeed = 400.f; // 걷기 속도

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
      float RunSpeed = 600.f; // 달리기 속도

     UPROPERTY(EditAnywhere, BlueprintReadWrite)
      float Defense = 5.f; // 방어력 (받는 데미지에서 차감)

     UPROPERTY(EditAnywhere, BlueprintReadWrite)
      float BaseAttackPower = 10.f; // 기본 공격력 (무기 데미지에 합산)
};

UCLASS()
class TOOSIN_API ATSCharacter : public ACharacter {

    GENERATED_BODY()

public:
      ATSCharacter();           

      virtual void Tick(float DeltaTime) override; // 틱 함수
      virtual void SetupPlayerInputComponent(class UInputComponent *PlayerInputComponent) override; // 입력 설정 함수
      virtual float TakeDamage(float DamageAmount, struct FDamageEvent const &DamageEvent,class AController *EventInstigator, AActor *DamageCauser) override; // 데미지 처리 함수
      void InitializeStats(); // 데이터 테이블에서 스탯 초기화 함수

      // [라운드 리셋용]
      UFUNCTION(BlueprintCallable, Category = "Stats")
      void ResetStats();

     UFUNCTION(BlueprintCallable, Category = "Toosin|State")
     FORCEINLINE ETSCharacterState GetCharacterState() const {return CurrentState;} // 현재 상태 반환 함수
     UFUNCTION(BlueprintCallable, Category = "Toosin|State")
     FORCEINLINE ETSWeaponType GetWeaponType() const { return CurrentWeaponType; }
     UFUNCTION(BlueprintCallable, Category = "Toosin|State")
     void SetCharacterState(ETSCharacterState NewState); // 현재 상태 설정 함수
     UFUNCTION(BlueprintCallable, Category = "Toosin|State")
     void SetWeaponType(ETSWeaponType NewType); // 현재 무기 타입 설정 함수

      UFUNCTION(BlueprintCallable, Category = "Toosin|Weapon")
    void EquipWeapon(class ATSWeapon *NewWeapon); // 무기 장착 함수
    FORCEINLINE class ATSWeapon *GetCurrentWeapon() const {return CurrentWeapon;} // 현재 장착된 무기 반환 함수

    // 입력 처리 함수
    void Move(const FInputActionValue &Value);
    void Look(const FInputActionValue &Value);
    void LightAttack();
    void HeavyAttack();
    void Dodge();
    void GuardStart();
    void GuardEnd();

    void Die();

    // [피격 리액션] 캐릭터별 피격 몽타주 재생
    void PlayHitReaction(AActor *DamageCauser);

    // [피격 리액션 종료] 몽타주 끝나면 Stunned → Idle 복구
    UFUNCTION()
    void OnHitReactionEnded(UAnimMontage *Montage, bool bInterrupted);

    // [넉백 적용] 공통 넉백 함수 (Launch + 자동 정지)
    void ApplyKnockback(AActor *Source);

    // [넉백 정지] 잔여 속도 제거
    void StopKnockback();

     // [무기 콜리전 제어 함수] (AnimNotifyState에서 호출)
    void EnableWeaponCollision();
    void DisableWeaponCollision();

    protected:
    virtual void BeginPlay() override;
    void PerformCombo(int32 SectionIndex); 
    void ResetAttackCooldown();            
    public:
    // 카메라
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|Camera")
    class USpringArmComponent *CameraBoom; // 카메라 붐

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|Camera")
    class UCameraComponent *FollowCamera; // 팔로우 카메라

    // [피격 몽타주] 캐릭터별 피격 리액션 몽타주 (에디터에서 설정)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Toosin|Animation")
    class UAnimMontage *HitReactionMontage; // 피격 리액션 몽타주

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Toosin|Animation")
    class UAnimMontage *ParriedMontage; // 패링당했을 때 리액션 몽타주 (튕겨나감)
    
    // [패링당함 리액션] 공격자가 패링당했을 때 호출
    void PlayParriedReaction(AActor* Parrier);

    // [넉백] 피격 시 밀림 설정 (에디터/블루프린트에서 조절 가능)
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Toosin|Combat")
    float KnockbackStrength = 800.f; // 수평 밀림 강도

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Toosin|Combat")
    float KnockbackUpForce = 50.f; // 위로 뜨는 힘

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Toosin|Combat")
    float KnockbackStopDelay = 0.15f; // 넉백 후 속도 정지까지 딜레이 (초)

    FTimerHandle KnockbackStopTimerHandle;

     // 데이터 테이블 설정
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Toosin|Stats Data")
    UDataTable *CharacterStatsTable; // 캐릭터 스탯 데이터 테이블

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Toosin|Stats Data")
    FName CharacterStatRowName; // 데이터 테이블의 행 이름

    // 입력 액션
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Toosin|Input")
     class UInputMappingContext *DefaultMappingContext; // 기본 입력 매핑 컨텍스트

      UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Toosin|Input")
      class UInputAction *MoveAction; // 이동 액션
      UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Toosin|Input")
      class UInputAction *LookAction; // 시점 조작 액션
      UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Toosin|Input")
      class UInputAction *LightAttackAction; // 일반공격 액션
      UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Toosin|Input")
      class UInputAction *HeavyAttackAction; // 강공격 액션
      UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Toosin|Input")
      class UInputAction *DodgeAction; // 회피 액션
      UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Toosin|Input")
      class UInputAction *GuardAction; // 가드 액션

protected:
      // 스탯
      UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|Stats")
      float MaxHealth; // 최대 체력
      UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|Stats")
      float CurrentHealth; // 현재 체력
      UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|Stats")
      float MaxStamina; // 최대 스태미나
      UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|Stats")
      float CurrentStamina; // 현재 스태미나

      UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|Stats")
  float Defense; // 방어력
      UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|Stats")
      float BaseAttackPower; // 기본 공격력
      UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|Movement")
      float WalkSpeed; // 걷기 속도

      // 상태머신
     UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|State")
      ETSCharacterState CurrentState;
      UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|State")
      ETSWeaponType CurrentWeaponType;
      UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|Combat")
      bool bHasNextComboInput; // 추가 (입력 버퍼)

      // [공격 후딜레이(쿨타임) 시스템]
      UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|Combat")
      bool bAttackCooldown; // 추가 (쿨타임 상태)

      UPROPERTY(EditDefaultsOnly, Category = "Toosin|Combat")
      float AttackCooldownTime = 0.5f; // 추가 (공격 후 0.5초간 공격 불가)

      FTimerHandle CooldownTimerHandle; // 추가 (타이머 핸들)
      // 콤보 시스템
      UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|Combat")
     int32 ComboCount; // 현재 콤보 횟수

     UFUNCTION(BlueprintCallable, Category = "Toosin|Combat")
      void ResetCombo(); // 콤보 초기화 (AnimNotify에서 호출)

      UFUNCTION(BlueprintCallable, Category = "Toosin|Combat")
      void ContinueCombo(); // 콤보 체크 (AnimNotify에서 호출)
     UFUNCTION()
     void OnAttackMontageEnded(UAnimMontage *Montage, bool bInterrupted);

      UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|Weapon")
  class ATSWeapon *CurrentWeapon;
      UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Toosin|Weapon")
      TSubclassOf<ATSWeapon> DefaultWeaponClass;

      // [컴포넌트 추가]
      UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|Components")
      class UTSCombatComponent *CombatComponent;

  // 입력 액션
      UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Toosin|Input")

  // [스태미나 회복 관련]
      bool bCanRecoverStamina = true;    // 스태미나 회복 가능 여부
      float StaminaRecoveryRate = 10.0f; // 초당 스태미나 회복량

 public:
      // Getter for CombatComponent
      FORCEINLINE class UTSCombatComponent *GetCombatComponent() const { return CombatComponent; }

      // [락온 시스템]
      UFUNCTION(BlueprintCallable, Category = "Toosin|Combat")
      void SetLockOnTarget(AActor* Target);

      UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|Combat")
      AActor* LockedTarget;

      // [AI 전투 패턴] 랜덤 공격용 몽타주 배열
      UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Toosin|AI")
      TArray<class UAnimMontage*> EnemyAttackMontages;

      // Getter for WalkSpeed
      FORCEINLINE float GetWalkSpeed() const { return WalkSpeed; }
     // Getter for Defense (방어력)
      FORCEINLINE float GetDefense() const { return Defense; }
      // Getter for BaseAttackPower (기본 공격력)
      FORCEINLINE float GetBaseAttackPower() const { return BaseAttackPower; }
      // ComboCount를 외부에서 참조 가능하게 (무기 데미지 계산용)
      FORCEINLINE int32 GetComboCount() const { return ComboCount; }
};
