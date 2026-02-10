#include "Toosin/Public/Weapon/TSWeapon.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"
#include "Toosin/Public/Character/TSCharacter.h"
#include "Toosin/Public/Character/TSCombatComponent.h"

ATSWeapon::ATSWeapon() {
    PrimaryActorTick.bCanEverTick = false;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
    WeaponMesh->SetupAttachment(Root);
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 무기 메쉬는 단순 외형이므로 충돌 꺼둠

    // 충돌 박스 생성 및 설정
    CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
    CollisionBox->SetupAttachment(WeaponMesh);
    CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 기본은 꺼둠
    CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap); // 캐릭터 충돌 감지
    CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap); // 무기끼리 충돌 감지
    CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
    CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Ignore);

    CollisionBox->SetGenerateOverlapEvents(true); // 오버랩 이벤트 생성 활성화

    LeftHandSocketName = TEXT("LeftHandSocket");

    // [콤보 배율 기본값 초기화] 1타=1.0, 2타=1.1, 3타=1.3
    ComboMultipliers.Add(1.0f);
    ComboMultipliers.Add(1.1f);
    ComboMultipliers.Add(1.3f);
}

void ATSWeapon::BeginPlay() {
    Super::BeginPlay();

    // 오버랩 이벤트 바인딩
    if (CollisionBox) {
        CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &ATSWeapon::OnBoxOverlap);
    }

    // 주인이 있다면 주인과의 충돌 무시
    if (GetOwner()) {
        CollisionBox->IgnoreActorWhenMoving(GetOwner(), true);
        if (UPrimitiveComponent *OwnerRoot = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent())) {
            CollisionBox->IgnoreComponentWhenMoving(OwnerRoot, true);
        }
    }
}

void ATSWeapon::EnableCollision() {
    if (CollisionBox) {
        CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        IgnoreActors.Empty();

        FVector BoxExtent = CollisionBox->GetScaledBoxExtent();
        FVector BoxLoc = CollisionBox->GetComponentLocation();
        UE_LOG(LogTemp, Warning, TEXT("[TSWeapon] Collision Enabled. Box Extent: %s, Location: %s"), *BoxExtent.ToString(), *BoxLoc.ToString());
    }
}

void ATSWeapon::DisableCollision() {
    if (CollisionBox) {
        CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        IgnoreActors.Empty();
        UE_LOG(LogTemp, Warning, TEXT("[TSWeapon] Collision Disabled"));
    }
}

void ATSWeapon::OnBoxOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult) {
    if (OtherActor) {
        UE_LOG(LogTemp, Warning, TEXT("[TSWeapon] Overlap Event! OtherActor: %s, OtherComp: %s"), *OtherActor->GetName(), *OtherComp->GetName());
    }

    if (!OtherActor || OtherActor == this || OtherActor == GetOwner()) return;

    // 이미 이번 공격에 맞은 대상이면 무시
    if (IgnoreActors.Contains(OtherActor)) return;
    IgnoreActors.Add(OtherActor);

    // ========== 1. 무기끼리 충돌 체크 (가드/패링) ==========
    // 무기↔무기 충돌을 먼저 체크 (가드 성공 판정)
    ATSWeapon *OtherWeapon = Cast<ATSWeapon>(OtherActor);
    if (OtherWeapon) {
        ATSCharacter *OtherChar = Cast<ATSCharacter>(OtherWeapon->GetOwner());
        if (OtherChar && OtherChar != GetOwner()) {
            // [패링 체크] 상대가 패링 윈도우 중이면 패링 성공
            if (OtherChar->GetCombatComponent() && OtherChar->GetCombatComponent()->bIsParryWindow) {
                IgnoreActors.Add(OtherChar);
                IgnoreActors.Add(OtherWeapon);

                UE_LOG(LogTemp, Warning, TEXT("[TSWeapon] 패링 성공! %s의 패링 윈도우에 걸림"), *OtherChar->GetName());
                OtherChar->GetCombatComponent()->OnParrySuccess(GetOwner());

                // [가해자 무기 튕김] 공격한 쪽에 WeaponDeflectMontage 재생
                ATSCharacter *AttackerChar = Cast<ATSCharacter>(GetOwner());
                if (AttackerChar && WeaponDeflectMontage) {
                    AttackerChar->PlayAnimMontage(WeaponDeflectMontage);
                    UE_LOG(LogTemp, Warning, TEXT("[TSWeapon] 공격자 무기 튕김 몽타주 재생"));
                }
                return; // 패링 성공이면 데미지 없이 종료
            }

            // [가드 체크] 상대가 가드 중이면 가드 성공 (무기↔무기 충돌)
            if (OtherChar->GetCombatComponent() && OtherChar->GetCombatComponent()->bIsGuarding) {
                IgnoreActors.Add(OtherChar);

                UE_LOG(LogTemp, Warning, TEXT("[TSWeapon] 가드 성공 (무기↔무기 충돌) - %s"), *OtherChar->GetName());

                // [가드 에임 업데이트] 공격 방향으로 가드 포즈 조절
                OtherChar->GetCombatComponent()->UpdateGuardAim(GetOwner());

                // 데미지 계산 후 가드 경감 적용
                float BasePower = 0.f;
                ATSCharacter *OwnerChar = Cast<ATSCharacter>(GetOwner());
                if (OwnerChar) {
                    BasePower = OwnerChar->GetBaseAttackPower();
                }
                int32 CurrentCombo = OwnerChar ? OwnerChar->GetComboCount() : 0;
                float FinalDamage = CalculateDamage(BasePower, CurrentCombo);

                // 가드 성공: ProcessDamage에서 경감 처리 (bIsGuardPenetrated = false)
                UGameplayStatics::ApplyDamage(OtherChar, FinalDamage, GetInstigatorController(), this, UDamageType::StaticClass());

                // [가해자 무기 튕김] 가드 성공 시에도 공격한 쪽 무기 튕김
                ATSCharacter *AttackerChar = Cast<ATSCharacter>(GetOwner());
                if (AttackerChar && WeaponDeflectMontage) {
                    AttackerChar->PlayAnimMontage(WeaponDeflectMontage);
                    UE_LOG(LogTemp, Warning, TEXT("[TSWeapon] 가드 성공 - 공격자 무기 튕김 몽타주 재생"));
                }
                return; // 가드 처리 완료
            }
        }
        return; // 무기끼리 충돌했지만 가드/패링 아니면 무시
    }

    // ========== 2. 캐릭터 바디 피격 처리 ==========
    if (OtherActor->IsA(ACharacter::StaticClass())) {
        UE_LOG(LogTemp, Warning, TEXT("[TSWeapon] 캐릭터 바디 피격 감지: %s"), *OtherActor->GetName());

        // 데미지 계산
        float BasePower = 0.f;
        ATSCharacter *OwnerChar = Cast<ATSCharacter>(GetOwner());
        if (OwnerChar) {
            BasePower = OwnerChar->GetBaseAttackPower();
        }
        int32 CurrentCombo = OwnerChar ? OwnerChar->GetComboCount() : 0;
        float FinalDamage = CalculateDamage(BasePower, CurrentCombo);

        UE_LOG(LogTemp, Warning, TEXT("[TSWeapon] 데미지 계산 결과 - Base:%.1f, WeaponDmg:%.1f, Heavy:%s, Combo:%d → Final:%.1f"), BasePower, WeaponDamage, bIsHeavyAttack ? TEXT("Yes") : TEXT("No"), CurrentCombo, FinalDamage);

        // [가드 관통 체크] 상대가 가드 중인데 바디에 직접 맞은 경우
        ATSCharacter *TargetChar = Cast<ATSCharacter>(OtherActor);
        if (TargetChar && TargetChar->GetCombatComponent() && TargetChar->GetCombatComponent()->bIsGuarding) {
            // 무기가 아닌 바디에 맞았으므로 가드 관통 → 풀 데미지
            UE_LOG(LogTemp, Warning, TEXT("[TSWeapon] 가드 관통! 무기가 바디에 직접 맞음 → 풀 데미지 적용"));
            TargetChar->GetCombatComponent()->ProcessDamage(FinalDamage, FDamageEvent(), GetInstigatorController(), this, true); // bIsGuardPenetrated = true

            // 가드 관통 시 직접 체력 차감
            float ActualDamage = FinalDamage; // 가드 경감 없이 풀 데미지
            // TakeDamage를 통하지 않고 직접 처리하면 중복 방지
            UGameplayStatics::ApplyDamage(OtherActor, FinalDamage, GetInstigatorController(), this, UDamageType::StaticClass());
            return;
        }

        // 일반 피격
        UGameplayStatics::ApplyDamage(OtherActor, FinalDamage, GetInstigatorController(), this, UDamageType::StaticClass());
    }
}

FTransform ATSWeapon::GetLeftHandSocketTransform() const {
    return WeaponMesh->GetSocketTransform(LeftHandSocketName);
}

bool ATSWeapon::HasLeftHandSocket() const {
    return WeaponMesh->DoesSocketExist(LeftHandSocketName);
}

// ========== [데미지 계산 함수] ==========
float ATSWeapon::CalculateDamage(float BaseAttackPower, int32 ComboCount) const {
    float TypeMultiplier = bIsHeavyAttack ? HeavyAttackMultiplier : LightAttackMultiplier;

    float ComboMult = 1.0f;
    if (!bIsHeavyAttack && ComboCount > 0) {
        int32 ComboIndex = ComboCount - 1;
        if (ComboMultipliers.IsValidIndex(ComboIndex)) {
            ComboMult = ComboMultipliers[ComboIndex];
        } else if (ComboMultipliers.Num() > 0) {
            ComboMult = ComboMultipliers.Last();
        }
    }

    float FinalDamage = (BaseAttackPower + WeaponDamage) * TypeMultiplier * ComboMult;
    FinalDamage = FMath::Max(FinalDamage, 1.0f);

    UE_LOG(LogTemp, Log, TEXT("[TSWeapon::CalculateDamage] (%.1f + %.1f) × %.2f × %.2f = %.1f"), BaseAttackPower, WeaponDamage, TypeMultiplier, ComboMult, FinalDamage);

    return FinalDamage;
}
