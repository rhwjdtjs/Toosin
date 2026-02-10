#include "Toosin/Public/Character/TSCombatComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Toosin/Public/Character/TSCharacter.h"

UTSCombatComponent::UTSCombatComponent() {
  PrimaryComponentTick.bCanEverTick = true;

  bIsGuarding = false;
  bIsParryWindow = false;
  bIsGuardOnCooldown = false; // 가드 쿨타임 초기화

  DefaultFOV = 90.0f; // 기본값 안전 초기화

  // 패링 효과 초기값 설정
  ParryTimeDilation = 0.05f;  // 아주 느리게 (0.2 -> 0.05)
  ParryEffectDuration = 0.2f; // 짧고 굵게
}

void UTSCombatComponent::BeginPlay() {
  Super::BeginPlay();

  // 소유자 캐릭터 캐싱
  OwnerCharacter = Cast<ATSCharacter>(GetOwner());
  if (OwnerCharacter && OwnerCharacter->FollowCamera) {
    float CurrentFOV = OwnerCharacter->FollowCamera->FieldOfView;
    if (CurrentFOV > 10.0f) {
      DefaultFOV = CurrentFOV;
    } else {
      DefaultFOV = 90.0f; // 비정상 값이면 90으로
    }
  }
}

void UTSCombatComponent::TickComponent(
    float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction *ThisTickFunction) {
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UTSCombatComponent::GuardStart() {
  if (!OwnerCharacter)
    return;

  // [가드 쿨타임 체크] 쿨타임 중이면 가드 자체가 불가
  if (bIsGuardOnCooldown) {
    UE_LOG(LogTemp, Warning,
           TEXT("[TSCombatComp] 가드 쿨타임 중 - 가드 불가 (%.1f초 대기)"),
           GuardCooldownTime);
    return;
  }

  // 이동, 대기 중일 때만 방어 가능
  ETSCharacterState State = OwnerCharacter->GetCharacterState();
  if (State == ETSCharacterState::Idle || State == ETSCharacterState::Moving) {
    bIsGuarding = true;

    // [Just Guard] 패링 윈도우 시작
    bIsParryWindow = true;
    GetWorld()->GetTimerManager().SetTimer(
        ParryTimerHandle, this, &UTSCombatComponent::CloseParryWindow,
        ParryInputWindow, false);

    OwnerCharacter->SetCharacterState(ETSCharacterState::Blocking);
    OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed =
        OwnerCharacter->GetWalkSpeed() * 0.5f; // 이속 감소

    UE_LOG(LogTemp, Warning,
           TEXT("[TSCombatComp] Guard Start (Parry Window Open)"));
  }
}

void UTSCombatComponent::GuardEnd() {
  if (!OwnerCharacter)
    return;

  if (bIsGuarding) {
    bIsGuarding = false;
    bIsParryWindow = false;
    GetWorld()->GetTimerManager().ClearTimer(ParryTimerHandle); // 타이머 취소

    if (OwnerCharacter->GetCharacterState() == ETSCharacterState::Blocking) {
      OwnerCharacter->SetCharacterState(ETSCharacterState::Idle);
    }
    OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed =
        OwnerCharacter->GetWalkSpeed(); // 이속 복구

    // [가드 쿨타임 시작] 가드 해제 후 일정 시간 동안 가드+패링 모두 불가
    bIsGuardOnCooldown = true;
    GetWorld()->GetTimerManager().SetTimer(
        GuardCooldownTimerHandle, this, &UTSCombatComponent::ResetGuardCooldown,
        GuardCooldownTime, false);

    UE_LOG(LogTemp, Warning,
           TEXT("[TSCombatComp] Guard End (쿨타임 %.1f초 시작)"),
           GuardCooldownTime);
  }
}

void UTSCombatComponent::CloseParryWindow() {
  if (bIsParryWindow) {
    bIsParryWindow = false;
    UE_LOG(LogTemp, Warning,
           TEXT("[TSCombatComp] Parry Window Closed -> Normal Guard"));
  }
}

void UTSCombatComponent::OnParrySuccess(AActor *Attacker) {
  if (!OwnerCharacter)
    return;

  UE_LOG(LogTemp, Warning, TEXT("[TSCombatComp] Parry SUCCESS!"));

  // 1. 스태미나 회복 (접근 처리가 필요하지만 일단 생략하거나 추후 Character에
  // 함수 추가) OwnerCharacter->RestoreStamina(30.0f); // 구현 필요

  // 2. 시간 느려짐
  UGameplayStatics::SetGlobalTimeDilation(GetWorld(), ParryTimeDilation);

  // 3. 카메라 줌 (제거됨 - 사용자 요청)
  /*
  if (OwnerCharacter->FollowCamera)
  {
          OwnerCharacter->FollowCamera->SetFieldOfView(ParryZoomFOV);
  }
  */

  // 4. 이펙트 복구 타이머
  GetWorld()->GetTimerManager().SetTimer(ParryEffectTimerHandle, this,
                                         &UTSCombatComponent::ResetParryEffect,
                                         ParryEffectDuration, false);

  // 5. 패링 몽타주? (이미 시도할 때 재생했으므로, 성공 모션이 따로 있다면
  // 여기서 재생) if (ParrySuccessMontage)
  // OwnerCharacter->PlayAnimMontage(ParrySuccessMontage);
}

void UTSCombatComponent::ResetParryEffect() {
  UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);

  // 카메라 복구 로직 제거
  /*
  if (OwnerCharacter && OwnerCharacter->FollowCamera)
  {
          // DefaultFOV가 정상 범위일 때만 복구
          if (DefaultFOV > 10.0f)
          {
                  OwnerCharacter->FollowCamera->SetFieldOfView(DefaultFOV);
          }
          else
          {
                  OwnerCharacter->FollowCamera->SetFieldOfView(90.0f); //
  비정상이면 90으로 강제 복구
          }
  }
  */
}

// [가드 쿨타임 해제] 쿨타임이 끝나면 다시 가드+패링 가능
void UTSCombatComponent::ResetGuardCooldown() {
  bIsGuardOnCooldown = false;
  UE_LOG(LogTemp, Warning,
         TEXT("[TSCombatComp] 가드 쿨타임 해제 - 가드/패링 다시 가능"));
}

float UTSCombatComponent::ProcessDamage(float DamageAmount,
                                        FDamageEvent const &DamageEvent,
                                        AController *EventInstigator,
                                        AActor *DamageCauser) {
  // 1. 패링 성공 체크 (Just Guard)
  if (bIsParryWindow) {
    OnParrySuccess(DamageCauser);
    return 0.0f; // 데미지 무효
  }

  // 2. 방어 체크
  if (bIsGuarding) {
    // 전방 체크 로직 추가 가능 (내적 계산)
    UE_LOG(LogTemp, Warning,
           TEXT("[TSCombatComp] 가드 성공 - 데미지 경감 적용"));

    // 가드 히트 몽타주 재생 (경직)
    if (OwnerCharacter && GuardHitMontage) {
      OwnerCharacter->PlayAnimMontage(GuardHitMontage);
    }

    // 가드 시 데미지 80% 경감 (20%만 받음)
    float GuardedDamage = DamageAmount * 0.2f;
    UE_LOG(LogTemp, Warning,
           TEXT("[TSCombatComp] 가드 데미지: %.1f → %.1f (80%% 경감)"),
           DamageAmount, GuardedDamage);
    return GuardedDamage;
  }

  // 3. 일반 피격 - 방어력(Defense) 차감 적용
  // 방어력만큼 데미지를 줄이되, 최소 1 데미지는 보장
  float DefenseStat = 0.f;
  if (OwnerCharacter) {
    DefenseStat = OwnerCharacter->GetDefense(); // 캐릭터의 방어력 스탯 가져오기
  }
  float FinalDamage =
      FMath::Max(DamageAmount - DefenseStat, 1.0f); // 최소 1 데미지 보장

  UE_LOG(LogTemp, Warning,
         TEXT("[TSCombatComp] 일반 피격 - 원래 데미지:%.1f, 방어력:%.1f → 최종 "
              "데미지:%.1f"),
         DamageAmount, DefenseStat, FinalDamage);

  return FinalDamage;
}
