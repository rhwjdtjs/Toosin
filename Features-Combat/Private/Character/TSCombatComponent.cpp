#include "Toosin/Public/Character/TSCombatComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Toosin/Public/Character/TSCharacter.h"

UTSCombatComponent::UTSCombatComponent() {
    PrimaryComponentTick.bCanEverTick = true;

    bIsGuarding = false;
    bIsParryWindow = false;
    bIsGuardOnCooldown = false;

    DefaultFOV = 90.0f;

    ParryTimeDilation = 0.05f;
    ParryEffectDuration = 0.2f;
}

void UTSCombatComponent::BeginPlay() {
    Super::BeginPlay();

    OwnerCharacter = Cast<ATSCharacter>(GetOwner());
    if (OwnerCharacter && OwnerCharacter->FollowCamera) {
        float CurrentFOV = OwnerCharacter->FollowCamera->FieldOfView;
        if (CurrentFOV > 10.0f) {
            DefaultFOV = CurrentFOV;
        } else {
            DefaultFOV = 90.0f;
        }
    }
}

void UTSCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) {
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UTSCombatComponent::GuardStart() {
    if (!OwnerCharacter) return;

    if (bIsGuardOnCooldown) {
        UE_LOG(LogTemp, Warning, TEXT("[TSCombatComp] 가드 쿨타임 중 - 가드 불가"));
        return;
    }

    ETSCharacterState State = OwnerCharacter->GetCharacterState();
    if (State == ETSCharacterState::Idle || State == ETSCharacterState::Moving) {
        bIsGuarding = true;

        bIsParryWindow = true;
        GetWorld()->GetTimerManager().SetTimer(ParryTimerHandle, this, &UTSCombatComponent::CloseParryWindow, ParryInputWindow, false);

        OwnerCharacter->SetCharacterState(ETSCharacterState::Blocking);
        OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = OwnerCharacter->GetWalkSpeed() * 0.5f;

        UE_LOG(LogTemp, Warning, TEXT("[TSCombatComp] Guard Start (Parry Window Open)"));
    }
}

void UTSCombatComponent::GuardEnd() {
    if (!OwnerCharacter) return;

    if (bIsGuarding) {
        bIsGuarding = false;
        bIsParryWindow = false;
        GetWorld()->GetTimerManager().ClearTimer(ParryTimerHandle);

        if (OwnerCharacter->GetCharacterState() == ETSCharacterState::Blocking) {
            OwnerCharacter->SetCharacterState(ETSCharacterState::Idle);
        }
        OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = OwnerCharacter->GetWalkSpeed();

        // [가드 에임 리셋]
        GuardAimYaw = 0.f;
        GuardAimPitch = 0.f;

        bIsGuardOnCooldown = true;
        GetWorld()->GetTimerManager().SetTimer(GuardCooldownTimerHandle, this, &UTSCombatComponent::ResetGuardCooldown, GuardCooldownTime, false);

        UE_LOG(LogTemp, Warning, TEXT("[TSCombatComp] Guard End (쿨타임 %.1f초 시작)"), GuardCooldownTime);
    }
}

void UTSCombatComponent::CloseParryWindow() {
    if (bIsParryWindow) {
        bIsParryWindow = false;
        UE_LOG(LogTemp, Warning, TEXT("[TSCombatComp] Parry Window Closed -> Normal Guard"));
    }
}

void UTSCombatComponent::OnParrySuccess(AActor *Attacker) {
    if (!OwnerCharacter) return;

    UE_LOG(LogTemp, Warning, TEXT("[TSCombatComp] Parry SUCCESS!"));

    UGameplayStatics::SetGlobalTimeDilation(GetWorld(), ParryTimeDilation);

    GetWorld()->GetTimerManager().SetTimer(ParryEffectTimerHandle, this, &UTSCombatComponent::ResetParryEffect, ParryEffectDuration, false);
}

void UTSCombatComponent::ResetParryEffect() {
    UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
}

void UTSCombatComponent::ResetGuardCooldown() {
    bIsGuardOnCooldown = false;
    UE_LOG(LogTemp, Warning, TEXT("[TSCombatComp] 가드 쿨타임 해제 - 가드/패링 다시 가능"));
}

// ========== [가드 에임 업데이트] ==========
// 공격자의 방향을 기준으로 Yaw/Pitch 계산 → AnimBP에서 AimOffset과 연동
void UTSCombatComponent::UpdateGuardAim(AActor *Attacker) {
    if (!OwnerCharacter || !Attacker) return;

    // 공격자를 바라보는 방향 계산
    FVector Direction = Attacker->GetActorLocation() - OwnerCharacter->GetActorLocation();
    FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(OwnerCharacter->GetActorLocation(), Attacker->GetActorLocation());

    // 캐릭터의 현재 회전 기준으로 상대적 방향 계산
    FRotator DeltaRot = (LookAtRot - OwnerCharacter->GetActorRotation()).GetNormalized();

    // Yaw: -90~90 범위로 클램프 (좌우)
    GuardAimYaw = FMath::Clamp(DeltaRot.Yaw, -90.f, 90.f);
    // Pitch: -45~45 범위로 클램프 (상하)
    GuardAimPitch = FMath::Clamp(DeltaRot.Pitch, -45.f, 45.f);

    UE_LOG(LogTemp, Log, TEXT("[TSCombatComp] Guard Aim Updated - Yaw:%.1f, Pitch:%.1f"), GuardAimYaw, GuardAimPitch);
}

// ========== [피격 처리] ==========
float UTSCombatComponent::ProcessDamage(float DamageAmount, FDamageEvent const &DamageEvent, AController *EventInstigator, AActor *DamageCauser, bool bIsGuardPenetrated) {
    // 1. 패링 성공 체크 (Just Guard)
    if (bIsParryWindow && !bIsGuardPenetrated) {
        OnParrySuccess(DamageCauser);
        return 0.0f;
    }

    // 2. 가드 관통 체크 (무기가 바디에 직접 맞은 경우)
    if (bIsGuardPenetrated) {
        UE_LOG(LogTemp, Warning, TEXT("[TSCombatComp] 가드 관통! 무기가 바디에 직접 맞음 → 풀 데미지 적용"));

        // 가드 상태 강제 해제 (관통당했으므로)
        if (bIsGuarding) {
            bIsGuarding = false;
            bIsParryWindow = false;
            if (OwnerCharacter && OwnerCharacter->GetCharacterState() == ETSCharacterState::Blocking) {
                OwnerCharacter->SetCharacterState(ETSCharacterState::Idle);
            }
        }

        // 피격 몽타주 재생
        if (OwnerCharacter) {
            OwnerCharacter->PlayHitReaction(DamageCauser);
        }

        // 방어력 차감만 적용 (가드 경감 없음)
        float DefenseStat = OwnerCharacter ? OwnerCharacter->GetDefense() : 0.f;
        float FinalDamage = FMath::Max(DamageAmount - DefenseStat, 1.0f);

        UE_LOG(LogTemp, Warning, TEXT("[TSCombatComp] 가드 관통 피격 - 원래:%.1f, 방어력:%.1f → 최종:%.1f"), DamageAmount, DefenseStat, FinalDamage);
        return FinalDamage;
    }

    // 3. 가드 성공 (무기↔무기 충돌로 여기 도달)
    if (bIsGuarding) {
        UE_LOG(LogTemp, Warning, TEXT("[TSCombatComp] 가드 성공 - 데미지 경감 적용"));

        // 가드 히트 몽타주 재생 (경직/밀려남)
        if (OwnerCharacter && GuardHitMontage) {
            OwnerCharacter->PlayAnimMontage(GuardHitMontage);
        }

        float GuardedDamage = DamageAmount * 0.2f;
        UE_LOG(LogTemp, Warning, TEXT("[TSCombatComp] 가드 데미지: %.1f → %.1f (80%% 경감)"), DamageAmount, GuardedDamage);
        return GuardedDamage;
    }

    // 4. 일반 피격 - 방어력 차감
    float DefenseStat = 0.f;
    if (OwnerCharacter) {
        DefenseStat = OwnerCharacter->GetDefense();
     }
     float FinalDamage = FMath::Max(DamageAmount - DefenseStat, 1.0f);

     // 피격 몽타주 재생
     if (OwnerCharacter) {
        OwnerCharacter->PlayHitReaction(DamageCauser);
   }

    UE_LOG(LogTemp, Warning, TEXT("[TSCombatComp] 일반 피격 - 원래 데미지:%.1f, 방어력:%.1f → 최종 데미지:%.1f"), DamageAmount, DefenseStat, FinalDamage);

     return FinalDamage;
}
