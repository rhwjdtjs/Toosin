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
        // 공격 콜리전: Pawn(바디) + WorldDynamic(무기) 모두 Overlap 활성화
        CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
        CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
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

void ATSWeapon::EnableGuardCollision() {
    if (CollisionBox) {
        CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        // Pawn(바디) 응답은 끄고, WorldDynamic(무기끼리)만 Overlap 유지
        CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
        CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
        IgnoreActors.Empty();
        UE_LOG(LogTemp, Warning, TEXT("[TSWeapon] Guard Collision Enabled (Weapon-to-Weapon Only)"));
    }
}

void ATSWeapon::DisableGuardCollision() {
    if (CollisionBox) {
        CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        // Pawn 응답 원래대로 복구 (다음 공격 시 EnableCollision에서 전부 켤 수 있도록)
        CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
        IgnoreActors.Empty();
        UE_LOG(LogTemp, Warning, TEXT("[TSWeapon] Guard Collision Disabled"));
    }
}

void ATSWeapon::OnBoxOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult) {
    if (!OtherActor || OtherActor == this || OtherActor == GetOwner()) return;

    // [컴포넌트 필터] 전투 관련 컴포넌트만 처리 (무기 BoxComponent / 캐릭터 CapsuleComponent)
    // 헬멧, 갑옷 등 메쉬 컴포넌트는 무시 (WorldDynamic이라 가드 콜리전에 걸림)
    bool bIsWeaponComp = Cast<UBoxComponent>(OtherComp) != nullptr;
    bool bIsBodyComp = Cast<UCapsuleComponent>(OtherComp) != nullptr;
    if (!bIsWeaponComp && !bIsBodyComp) return;

    // 이미 이번 공격에 맞은 대상이면 무시
    if (IgnoreActors.Contains(OtherActor)) return;
    IgnoreActors.Add(OtherActor);

    UE_LOG(LogTemp, Warning, TEXT("[TSWeapon] Overlap - Actor: %s, Comp: %s"), *OtherActor->GetName(), *OtherComp->GetName());

    // ========== 1. 무기끼리 충돌 체크 (가드/패링) ==========
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

                return;
            }

            // [가드 체크] 상대가 가드 중이면 가드 성공
            if (OtherChar->GetCombatComponent() && OtherChar->GetCombatComponent()->bIsGuarding) {
                IgnoreActors.Add(OtherChar);

                UE_LOG(LogTemp, Warning, TEXT("[TSWeapon] 가드 성공 (무기↔무기 충돌) - %s"), *OtherChar->GetName());

                OtherChar->GetCombatComponent()->UpdateGuardAim(GetOwner());

                float BasePower = 0.f;
                ATSCharacter *OwnerChar = Cast<ATSCharacter>(GetOwner());
                if (OwnerChar) BasePower = OwnerChar->GetBaseAttackPower();
                int32 CurrentCombo = OwnerChar ? OwnerChar->GetComboCount() : 0;
                float FinalDamage = CalculateDamage(BasePower, CurrentCombo);

                UGameplayStatics::ApplyDamage(OtherChar, FinalDamage, GetInstigatorController(), this, UDamageType::StaticClass());

                return;
            }
        }
        return;
    }

    // ========== 2. 캐릭터 바디 피격 처리 ==========
    if (OtherActor->IsA(ACharacter::StaticClass())) {
        ATSCharacter* OwnerChar = Cast<ATSCharacter>(GetOwner());
        
        // [중요] 공격 중이 아니면(예: 가드 중, 아이들 등) 데미지 판정 무시
        if (OwnerChar && OwnerChar->GetCharacterState() != ETSCharacterState::Attacking)
        {
            return;
        }

        float BasePower = 0.f;
        if (OwnerChar) BasePower = OwnerChar->GetBaseAttackPower();
        int32 CurrentCombo = OwnerChar ? OwnerChar->GetComboCount() : 0;
        float FinalDamage = CalculateDamage(BasePower, CurrentCombo);

        ATSCharacter *TargetChar = Cast<ATSCharacter>(OtherActor);

        // [가드 중 피격] → 에임 방향 기준 각도 판정
        if (TargetChar && TargetChar->GetCombatComponent() && TargetChar->GetCombatComponent()->bIsGuarding) {
            // 무기 오버랩에서 중복 처리 방지
            if (TargetChar->GetCurrentWeapon()) IgnoreActors.Add(TargetChar->GetCurrentWeapon());

            // [가드 아크 판정] 공격자가 가드 커버 범위 안인가?
            if (TargetChar->GetCombatComponent()->IsAttackInGuardArc(GetOwner())) {
                // 가드 성공 (에임 방향 안) → 데미지 경감
                UE_LOG(LogTemp, Warning, TEXT("[TSWeapon] 가드 성공 (아크 범위 내) - %s"), *TargetChar->GetName());
                UGameplayStatics::ApplyDamage(OtherActor, FinalDamage, GetInstigatorController(), this, UDamageType::StaticClass());

            } else {
                // 가드 관통 (뒤쪽/옆구리 공격) → 풀 데미지
                UE_LOG(LogTemp, Warning, TEXT("[TSWeapon] 가드 관통! (아크 범위 밖 = 뒤치기) - %s"), *TargetChar->GetName());
                TargetChar->GetCombatComponent()->ProcessDamage(FinalDamage, FDamageEvent(), GetInstigatorController(), this, true);
                UGameplayStatics::ApplyDamage(OtherActor, FinalDamage, GetInstigatorController(), this, UDamageType::StaticClass());
            }
            return;
        }

        // 일반 피격
        UE_LOG(LogTemp, Warning, TEXT("[TSWeapon] 일반 피격 - %s, 데미지: %.1f"), *OtherActor->GetName(), FinalDamage);
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
