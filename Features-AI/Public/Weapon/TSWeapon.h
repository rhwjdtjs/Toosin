#pragma once

#include "Components/BoxComponent.h" // BoxComponent 헤더 추가
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Headers/ATSWeaponTypes.h"
#include "TSWeapon.generated.h"

UCLASS()
class TOOSIN_API ATSWeapon : public AActor {
    GENERATED_BODY()

public:
    ATSWeapon();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|Components")
    class USceneComponent *Root;
    // 무기 메쉬
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|Weapon")
    class UStaticMeshComponent *WeaponMesh; // 무기 메쉬 컴포넌트

    // 무기 타입 (양손검, 도끼 등)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Toosin|Weapon")
    ETSWeaponType WeaponType; // 무기 타입

    // 무기 충돌용 박스 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|Weapon")
    class UBoxComponent *CollisionBox;

    // 충돌 활성화/비활성화 함수 (AnimNotify에서 호출 예정)
    UFUNCTION(BlueprintCallable, Category = "Toosin|Weapon")
    void EnableCollision();

    UFUNCTION(BlueprintCallable, Category = "Toosin|Weapon")
    void DisableCollision();

    // [가드 전용] 무기↔무기(WorldDynamic) 오버랩만 활성화 (Pawn 응답은 꺼둠)
    UFUNCTION(BlueprintCallable, Category = "Toosin|Weapon")
    void EnableGuardCollision();

    UFUNCTION(BlueprintCallable, Category = "Toosin|Weapon")
    void DisableGuardCollision();

    // 충돌 이벤트 처리 함수
    UFUNCTION()
    void OnBoxOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);

    // 한 번 공격에 중복 피격 방지용
    TArray<AActor *> IgnoreActors;

    // 왼손 IK 소켓 이름 ( 기본값: "LeftHandSocket")
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Toosin|Weapon")
    FName LeftHandSocketName; // 왼손 IK 소켓 이름

    // 왼손 소켓 Transform 반환
    UFUNCTION(BlueprintCallable, Category = "Toosin|Weapon")
    FTransform GetLeftHandSocketTransform() const;
    bool HasLeftHandSocket() const;
    // 무기 타입 반환
    UFUNCTION(BlueprintCallable, Category = "Toosin|Weapon")
    FORCEINLINE ETSWeaponType GetWeaponType() const { return WeaponType; }

    // 공격 몽타주
    UPROPERTY(EditDefaultsOnly, Category = "Toosin|Animation")
    class UAnimMontage *LightAttackMontage; // 일반 공격(콤보) 몽타주

    UPROPERTY(EditDefaultsOnly, Category = "Toosin|Animation")
    class UAnimMontage *HeavyAttackMontage; // 강공격 몽타주

    FORCEINLINE class UAnimMontage *GetLightAttackMontage() const { return LightAttackMontage; }
    FORCEINLINE class UAnimMontage *GetHeavyAttackMontage() const { return HeavyAttackMontage; }

    // [무기 튕김 몽타주] 가드 성공 시 공격자(가해자)에게 재생
    UPROPERTY(EditDefaultsOnly, Category = "Toosin|Animation")
    class UAnimMontage *WeaponDeflectMontage; // 무기 튕김 몽타주

    // ========== [데미지 시스템] ==========

    // 무기 고유 데미지 (무기마다 다른 기본 공격력)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Toosin|Damage")
    float WeaponDamage = 15.f;

    // 경공격 데미지 배율
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Toosin|Damage")
    float LightAttackMultiplier = 1.0f;

    // 강공격 데미지 배율 (경공격 대비 1.8배)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Toosin|Damage")
    float HeavyAttackMultiplier = 1.8f;

    // 콤보 단계별 데미지 배율 (예: 1타=1.0, 2타=1.1, 3타=1.3)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Toosin|Damage")
    TArray<float> ComboMultipliers;

    // 현재 공격이 강공격인지 여부 (ATSCharacter에서 공격 시 설정)
    bool bIsHeavyAttack = false;

    // 최종 데미지 계산 함수
    // 공식: (BaseAttackPower + WeaponDamage) × AttackTypeMultiplier × ComboMultiplier
    float CalculateDamage(float BaseAttackPower, int32 ComboCount) const;
};
