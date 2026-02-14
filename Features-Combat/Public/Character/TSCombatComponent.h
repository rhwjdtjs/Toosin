#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "TSCombatComponent.generated.h"

// 전방 선언
class ATSCharacter;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TOOSIN_API UTSCombatComponent : public UActorComponent {
    GENERATED_BODY()

public:
    UTSCombatComponent();

    // 컴포넌트 소유자 (캐릭터)
    UPROPERTY()
    class ATSCharacter *OwnerCharacter;

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|Combat")
    bool bIsGuarding; // 가드 중인지 여부

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|Combat")
    bool bIsParryWindow; // 패링 유효 시간인지 여부 (저스트 가드)

    // [설정값]
    UPROPERTY(EditDefaultsOnly, Category = "Toosin|Combat")
    float DefaultFOV;

    UPROPERTY(EditDefaultsOnly, Category = "Toosin|Combat")
    float ParryInputWindow = 0.25f; // 입력 후 0.25초간 패링 인정

    UPROPERTY(EditDefaultsOnly, Category = "Toosin|Combat")
    float ParryTimeDilation = 0.05f; // 패링 성공 시 시간 느려짐 배율

    UPROPERTY(EditDefaultsOnly, Category = "Toosin|Combat")
    float ParryEffectDuration = 0.2f; // 시간 느려짐 지속 시간

    FTimerHandle ParryTimerHandle;       // 패링 윈도우 타이머
    FTimerHandle ParryEffectTimerHandle; // 이펙트 복구 타이머

    // [가드 쿨타임] 패링 스팸 방지 - 가드 해제 후 일정 시간 동안 패링 불가
    UPROPERTY(EditDefaultsOnly, Category = "Toosin|Combat")
    float GuardCooldownTime = 1.0f; // 가드 해제 후 쿨타임 (초)

    bool bIsGuardOnCooldown = false;       // 가드 쿨타임 중 여부
    FTimerHandle GuardCooldownTimerHandle; // 가드 쿨타임 타이머

    UPROPERTY(EditDefaultsOnly, Category = "Toosin|Animation")
    class UAnimMontage *GuardHitMontage; // 가드 중 피격 몽타주 (밀려남)

    // [가드 에임 오프셋] 가드 중 상대 방향에 따라 무기 방향 조절
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|Combat")
    float GuardAimYaw = 0.f; // 가드 에임 좌우 방향

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|Combat")
    float GuardAimPitch = 0.f; // 가드 에임 상하 방향

    // [가드 아크] 가드 커버 범위 (에임 기준 좌우 반각, 기본 60도 = 총 120도 커버)
    UPROPERTY(EditDefaultsOnly, Category = "Toosin|Combat")
    float GuardArcHalfAngle = 60.f;

    // [AnimInstance 연동] 스레드 안전성 확보를 위한 Getter
    UFUNCTION(BlueprintCallable, Category = "Toosin|Combat")
    FORCEINLINE float GetGuardAimYaw() const { return GuardAimYaw; }

    UFUNCTION(BlueprintCallable, Category = "Toosin|Combat")
    FORCEINLINE float GetGuardAimPitch() const { return GuardAimPitch; }

    // [전투 함수]
    UFUNCTION(BlueprintCallable, Category = "Toosin|Combat")
    void GuardStart();

    UFUNCTION(BlueprintCallable, Category = "Toosin|Combat")
    void GuardEnd();

    // [저스트 가드 / 패링 윈도우 종료]
    void CloseParryWindow();

    UFUNCTION(BlueprintCallable, Category = "Toosin|Combat")
    void OnParrySuccess(AActor *Attacker); // 패링 성공 시 호출

    void ResetParryEffect();   // 이펙트 복구
    void ResetGuardCooldown(); // 가드 쿨타임 해제

    // [가드 에임] 공격자 방향 기반 가드 에임 업데이트
    void UpdateGuardAim(AActor *Attacker);

    // [가드 아크 판정] 공격자가 가드 커버 범위 안에 있는지 체크
    bool IsAttackInGuardArc(AActor *Attacker) const;

    // [피격 처리]
    // 데미지 계산 및 리액션을 처리하고 최종 데미지를 반환
    // bIsGuardPenetrated: true면 가드 관통 (무기가 바디에 직접 맞은 경우)
    float ProcessDamage(float DamageAmount, struct FDamageEvent const &DamageEvent, class AController *EventInstigator, AActor *DamageCauser, bool bIsGuardPenetrated = false);
  GENERATED_BODY()

public:
  UTSCombatComponent();

  // 컴포넌트 소유자 (캐릭터)
  UPROPERTY()
  class ATSCharacter *OwnerCharacter;

protected:
  virtual void BeginPlay() override;

public:
  virtual void
  TickComponent(float DeltaTime, ELevelTick TickType,
                FActorComponentTickFunction *ThisTickFunction) override;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|Combat")
  bool bIsGuarding; // 가드 중인지 여부

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|Combat")
  bool bIsParryWindow; // 패링 유효 시간인지 여부 (저스트 가드)

  // [설정값]
  UPROPERTY(EditDefaultsOnly, Category = "Toosin|Combat")
  float DefaultFOV;

  UPROPERTY(EditDefaultsOnly, Category = "Toosin|Combat")
  float ParryInputWindow = 0.25f; // 입력 후 0.25초간 패링 인정

  UPROPERTY(EditDefaultsOnly, Category = "Toosin|Combat")
  float ParryTimeDilation = 0.05f; // 패링 성공 시 시간 느려짐 배율

  UPROPERTY(EditDefaultsOnly, Category = "Toosin|Combat")
  float ParryEffectDuration = 0.2f; // 시간 느려짐 지속 시간

  FTimerHandle ParryTimerHandle;       // 패링 윈도우 타이머
  FTimerHandle ParryEffectTimerHandle; // 이펙트 복구 타이머

  // [가드 쿨타임] 패링 스팸 방지 - 가드 해제 후 일정 시간 동안 패링 불가
  UPROPERTY(EditDefaultsOnly, Category = "Toosin|Combat")
  float GuardCooldownTime = 1.0f; // 가드 해제 후 쿨타임 (초)

  bool bIsGuardOnCooldown = false;       // 가드 쿨타임 중 여부
  FTimerHandle GuardCooldownTimerHandle; // 가드 쿨타임 타이머

  UPROPERTY(EditDefaultsOnly, Category = "Toosin|Animation")
  class UAnimMontage *GuardHitMontage; // 가드 중 피격 몽타주 (밀려남)

  // [전투 함수]
  UFUNCTION(BlueprintCallable, Category = "Toosin|Combat")
  void GuardStart();

  UFUNCTION(BlueprintCallable, Category = "Toosin|Combat")
  void GuardEnd();

  // [저스트 가드 / 패링 윈도우 종료]
  void CloseParryWindow();

  UFUNCTION(BlueprintCallable, Category = "Toosin|Combat")
  void OnParrySuccess(AActor *Attacker); // 패링 성공 시 호출

  void ResetParryEffect();   // 이펙트 복구
  void ResetGuardCooldown(); // 가드 쿨타임 해제

  // [피격 처리]
  // 데미지 계산 및 리액션을 처리하고 최종 데미지를 반환
  float ProcessDamage(float DamageAmount,
                      struct FDamageEvent const &DamageEvent,
                      class AController *EventInstigator, AActor *DamageCauser);
};
