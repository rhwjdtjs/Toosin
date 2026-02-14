#include "Toosin/Public/Character/TSCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Weapon/TSWeapon.h"
#include "Toosin/Public/Game/ATSArenaGameMode.h"

ATSCharacter::ATSCharacter() {
    PrimaryActorTick.bCanEverTick = true;

    // 카메라 붐 설정
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent); // 루트 컴포넌트에 부착
    CameraBoom->TargetArmLength = 400.0f; // 카메라와 캐릭터 간 거리 설정
    CameraBoom->bUsePawnControlRotation = true; // 카메라 로테이션이 폰의 컨트롤 로테이션을 따르도록 설정

    // Combat Component 생성
    CombatComponent = CreateDefaultSubobject<UTSCombatComponent>(TEXT("CombatComponent"));

    // Pattern Component 생성 (AI 학습용 데이터 수집)
    PlayerPatternComponent = CreateDefaultSubobject<UTSPlayerPatternComponent>(TEXT("PlayerPatternComponent"));

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera")); // 팔로우 카메라 생성
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // 카메라 붐에 부착
    FollowCamera->bUsePawnControlRotation = false; // 카메라 자체 회전은 사용하지 않음 컨트롤러 회전 설정
    bUseControllerRotationYaw = true; // 컨트롤러 회전 사용
    GetCharacterMovement()->bOrientRotationToMovement = false; // 이동 방향에 따라 회전하지 않음

    // 기본 값 설정
    MaxHealth = 100.f;
    CurrentHealth = MaxHealth;
    MaxStamina = 100.f;
    CurrentStamina = MaxStamina;
    BaseAttackPower = 10.f; // 기본 공격력 기본값

    // 기본 Row Name
    CharacterStatRowName = FName(TEXT("Default"));
    CurrentState = ETSCharacterState::Idle; // 초기 상태
    CurrentWeaponType = ETSWeaponType::OneHanded; // 초기 무기 타입

    bCanRecoverStamina = true; // 스태미나 회복 가능 초기화

    // [Collision Settings]
    // 카메라가 캐릭터 메쉬나 캡슐에 충돌하여 줌인되는 현상 방지
    GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
}

void ATSCharacter::BeginPlay() {
    Super::BeginPlay();
    ComboCount = 0; // 콤보 카운트 초기화
    bHasNextComboInput = false; // 다음 콤보 입력 없음
    bAttackCooldown = false; // 공격 쿨다운 없음
    InitializeStats();
    if (UAnimInstance *AnimInstance = GetMesh()->GetAnimInstance()) {
        AnimInstance->OnMontageEnded.AddDynamic(this, &ATSCharacter::OnAttackMontageEnded); // 애니메이션 몽타주 종료 이벤트 바인딩
    }
    if (APlayerController *PlayerController = Cast<APlayerController>(Controller)) {
        if (UEnhancedInputLocalPlayerSubsystem *Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer())) // 입력 서브시스템 가져오기
        {
            if (DefaultMappingContext)
                Subsystem->AddMappingContext(DefaultMappingContext, 0); // 기본 매핑 컨텍스트 추가
         
        }
    }
    if (DefaultWeaponClass) {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.Instigator = GetInstigator();

        ATSWeapon *SpawedWeapon = GetWorld()->SpawnActor<ATSWeapon>(DefaultWeaponClass, GetActorTransform(), SpawnParams); // 무기 생성
        if (SpawedWeapon) {
            EquipWeapon(SpawedWeapon); // 무기 장착
        }
    }
}

void ATSCharacter::InitializeStats() {
    if (CharacterStatsTable) {
        // 캐릭터 스탯 데이터 테이블에서 Row 찾기
        static const FString ContextString(TEXT("Character Stats Context")); // 컨텍스트 문자열
        FTSCharacterStats *Stats = CharacterStatsTable->FindRow<FTSCharacterStats>(CharacterStatRowName, ContextString, true); // Row 찾기
        if (Stats) {
            MaxHealth = Stats->MaxHealth; // 최대 체력 설정
            MaxStamina = Stats->MaxStamina; // 최대 스태미나 설정
            WalkSpeed = Stats->WalkSpeed; // 걷기 속도 설정
            Defense = Stats->Defense; // 방어력 설정
            BaseAttackPower = Stats->BaseAttackPower; // 기본 공격력 설정
            CurrentHealth = MaxHealth; // 현재 체력 초기화
            CurrentStamina = MaxStamina; // 현재 스태미나 초기화
            GetCharacterMovement()->MaxWalkSpeed = WalkSpeed; // 최대 걷기 속도 설정

            UE_LOG(LogTemp, Warning, TEXT("[TSCharacter] 스탯 초기화 완료 - HP:%.0f, Stamina:%.0f, Defense:%.1f, AttackPower:%.1f"), MaxHealth, MaxStamina, Defense, BaseAttackPower);
        }
    }
}

void ATSCharacter::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent) {
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    if (UEnhancedInputComponent *EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) // 향상된 입력 컴포넌트로 캐스팅
    {
        if (MoveAction)
            EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATSCharacter::Move); // 이동 입력 바인딩
        if (LookAction)
            EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATSCharacter::Look); // 시점 입력 바인딩
        if (LightAttackAction)
            EnhancedInputComponent->BindAction(LightAttackAction, ETriggerEvent::Started, this, &ATSCharacter::LightAttack); // 경공격 입력 바인딩
        if (HeavyAttackAction)
            EnhancedInputComponent->BindAction(HeavyAttackAction, ETriggerEvent::Started, this, &ATSCharacter::HeavyAttack); // 중공격 입력 바인딩
        if (DodgeAction)
            EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Started, this, &ATSCharacter::Dodge); // 회피 입력 바인딩
        if (GuardAction) // 방어 입력 바인딩
        {
            EnhancedInputComponent->BindAction(GuardAction, ETriggerEvent::Started, this, &ATSCharacter::GuardStart); // 누르는 순간 가드 시작 (Just Guard 타이머 시작)
            EnhancedInputComponent->BindAction(GuardAction, ETriggerEvent::Completed, this, &ATSCharacter::GuardEnd); // 떼면 가드 종료
        }
    }
}

void ATSCharacter::Move(const FInputActionValue &Value) {
    if (CurrentState == ETSCharacterState::Attacking || CurrentState == ETSCharacterState::Stunned || CurrentState == ETSCharacterState::Dead) return; // 공격 중, 기절 중, 사망 상태에서는 이동 불가

    FVector2D MovementVector = Value.Get<FVector2D>(); // 입력값을 2D 벡터로 변환
    if (Controller != nullptr) // 컨트롤러 유효성 검사
    {
        const FRotator Rotation = Controller->GetControlRotation(); // 컨트롤러 회전 값 가져오기
        const FRotator YawRotation(0, Rotation.Yaw, 0); // Yaw() 회전 값만 사용
        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X); // 전방 방향 벡터
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y); // 우측 방향 벡터

        AddMovementInput(ForwardDirection, MovementVector.Y); // 전방 이동
        AddMovementInput(RightDirection, MovementVector.X);   // 우측 이동
    }
}

void ATSCharacter::Look(const FInputActionValue &Value) {
    FVector2D LookAxisVector = Value.Get<FVector2D>(); // 입력값을 2D 벡터로 변환
    if (Controller != nullptr) // 컨트롤러 유효성 검사
    {
        AddControllerYawInput(LookAxisVector.X); // 좌우 시점 이동
        AddControllerPitchInput(LookAxisVector.Y); // 상하 시점 이동
    }
}

void ATSCharacter::Dodge() {
    if (CurrentState == ETSCharacterState::Attacking || CurrentState == ETSCharacterState::Idle || CurrentState == ETSCharacterState::Moving) // 공격 중, 대기 중, 이동 중
    {
        UE_LOG(LogTemp, Warning, TEXT("Dodge Action!")); // 회피 액션!
        // 회피 구현 예정
        
        // [AI 학습] 회피 기록
        if (PlayerPatternComponent)
        {
            PlayerPatternComponent->RegisterDodge();
        }
    }
}

void ATSCharacter::GuardStart() {
    if (CombatComponent) {
        CombatComponent->GuardStart();
    }
}

void ATSCharacter::GuardEnd() {
    if (CombatComponent) {
        CombatComponent->GuardEnd();
    }
}

void ATSCharacter::LightAttack() {
    if (CurrentState == ETSCharacterState::Blocking || CurrentState == ETSCharacterState::Dead || CurrentState == ETSCharacterState::Stunned) return;
    if (bAttackCooldown) return; // (쿨타임 중이면 공격 불가)

    if (!CurrentWeapon) return;

    // 공격 중이 아니라면 -> 1타 시작
    if (CurrentState != ETSCharacterState::Attacking) {
        GetCharacterMovement()->StopMovementImmediately(); // (미끄러짐 방지)

        CurrentState = ETSCharacterState::Attacking;
        ComboCount = 1;
        bHasNextComboInput = false; // 다음 콤보 입력 없음

        bUseControllerRotationYaw = false; // (공격 중 회전은 몽타주에 맡김)

        // 경공격 플래그 설정 (무기 데미지 계산에서 사용)
        if (CurrentWeapon) {
            CurrentWeapon->bIsHeavyAttack = false; // 경공격임을 표시
        }

        // [AI 학습] 경공격 기록
        if (PlayerPatternComponent)
        {
            PlayerPatternComponent->RegisterAttack(false, ComboCount);
        }

        PerformCombo(ComboCount); // 콤보 실행
    }
    // 이미 공격 중이라면 -> 입력 예약 (버퍼링)
    else {
        bHasNextComboInput = true; // 다음 콤보 입력 예약
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Cyan, TEXT("Input Buffered!")); // 디버그 메시지
    }
}

void ATSCharacter::HeavyAttack() {
    if (CurrentState == ETSCharacterState::Blocking || CurrentState == ETSCharacterState::Stunned) return;

    // 강공격은 콤보 없이 단발 실행
    if (CurrentState == ETSCharacterState::Attacking) return;

    if (CurrentWeapon) {
        if (UAnimMontage *Montage = CurrentWeapon->GetHeavyAttackMontage()) {
            CurrentState = ETSCharacterState::Attacking; // 상태를 공격으로 설정
            CurrentWeapon->bIsHeavyAttack = true; // 강공격 플래그 설정 (데미지 배율 적용용)
            
            // [AI 학습] 강공격 기록
            if (PlayerPatternComponent)
            {
               PlayerPatternComponent->RegisterAttack(true, 1);
            }
            
            PlayAnimMontage(Montage); // 강공격 몽타주 재생
            UE_LOG(LogTemp, Warning, TEXT("[TSCharacter] 강공격 시작 (bIsHeavyAttack = true)"));
        }
    }
}

float ATSCharacter::TakeDamage(float DamageAmount, FDamageEvent const &DamageEvent, AController *EventInstigator, AActor *DamageCauser) {
    // 부모 클래스 처리
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    if (CurrentState == ETSCharacterState::Dead) return 0.0f;

    // CombatComponent를 통해 데미지 로직 처리 (패링/방어)
    if (CombatComponent) {
        ActualDamage = CombatComponent->ProcessDamage(ActualDamage, DamageEvent, EventInstigator, DamageCauser);
    }

    // 0 데미지면(패링 등) 리턴
    if (ActualDamage <= 0.0f) return 0.0f;
    // 일반 피격 처리
    CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.0f, MaxHealth);

    UE_LOG(LogTemp, Warning, TEXT("[TSCharacter] Final Damage: %f, CurrentHealth: %f"), ActualDamage, CurrentHealth);
    // 사망 처리
    if (CurrentHealth <= 0.0f)
    {
        Die();
        
        // [아레나 모드] 라운드 종료 알림
        if (ATSArenaGameMode* ArenaGM = Cast<ATSArenaGameMode>(GetWorld()->GetAuthGameMode()))
        {
            // 승자는 데미지를 준 사람 (또는 null)
            ArenaGM->EndRound(DamageCauser);
        }
    }
    return ActualDamage;
}

// ========== [피격 리액션 몽타주 재생] ==========
// Stunned 상태 설정 + 넉백 + 몽타주 종료 시 복구
void ATSCharacter::PlayHitReaction(AActor *DamageCauser) {
    if (CurrentState == ETSCharacterState::Dead) return;

    // 공격 중이었다면 공격 중단
    if (CurrentState == ETSCharacterState::Attacking) {
        ResetCombo();
    }

    // 가드 중이었다면 가드 해제
    if (CurrentState == ETSCharacterState::Blocking && CombatComponent) {
        CombatComponent->GuardEnd();
    }

    // [경직] Stunned 상태로 전환 → 이동/공격/가드 불가
    SetCharacterState(ETSCharacterState::Stunned);

    // [넉백]
    ApplyKnockback(DamageCauser);

    // 기존 몽타주 확실하게 정지 (블렌드 이슈 방지)
    StopAnimMontage();

    // [몽타주 재생 + 종료 콜백]
    if (HitReactionMontage) {
        float Duration = PlayAnimMontage(HitReactionMontage);
        if (Duration > 0.0f) {
            UE_LOG(LogTemp, Warning, TEXT("[TSCharacter] 피격 리액션 몽타주 재생 성공: %s (길이: %.2f)"), *HitReactionMontage->GetName(), Duration);

            // 몽타주 끝나면 OnHitReactionEnded 호출
            if (UAnimInstance *AnimInst = GetMesh()->GetAnimInstance()) {
                FOnMontageEnded EndDelegate;
                EndDelegate.BindUObject(this, &ATSCharacter::OnHitReactionEnded);
                AnimInst->Montage_SetEndDelegate(EndDelegate, HitReactionMontage);
            }
        } else {
            UE_LOG(LogTemp, Error, TEXT("[TSCharacter] 피격 리액션 몽타주 재생 실패! (Duration 0)"));
             SetCharacterState(ETSCharacterState::Idle); // 실패 시 복구
        }
    } else {
        UE_LOG(LogTemp, Warning, TEXT("[TSCharacter] HitReactionMontage 미설정 - 즉시 Idle 복구"));
        SetCharacterState(ETSCharacterState::Idle);
    }
}

// ========== [패링당함 리액션] ==========
// ========== [패링당함 리액션] ==========
// ========== [패링당함 리액션] ==========
void ATSCharacter::PlayParriedReaction(AActor* Parrier)
{
    UE_LOG(LogTemp, Warning, TEXT("[TSCharacter] PlayParriedReaction Called on: %s"), *GetName());

    if (CurrentState == ETSCharacterState::Dead) return;

    // 공격 중이었다면 공격 중단
    if (CurrentState == ETSCharacterState::Attacking) {
        ResetCombo();
    }

    // [경직] Stunned 상태로 전환
    SetCharacterState(ETSCharacterState::Stunned);

    // [넉백] 패링한 사람으로부터 밀려남
    if (Parrier) {
        ApplyKnockback(Parrier);
    }

    // 기존 몽타주 정지
    StopAnimMontage();

    // [몽타주 재생]
    if (ParriedMontage) {
        float Duration = PlayAnimMontage(ParriedMontage);
        if (Duration > 0.0f) {
            UE_LOG(LogTemp, Warning, TEXT("[TSCharacter] 패링 리액션 몽타주 재생 성공: %s (Duration: %.2f)"), *ParriedMontage->GetName(), Duration);
            
            // 몽타주 종료 시 상태 복구 (타이머 사용 - 더 안전함)
            FTimerHandle ResetStateHandle;
            GetWorldTimerManager().SetTimer(ResetStateHandle, [this]()
            {
                if (CurrentState == ETSCharacterState::Stunned)
                {
                    SetCharacterState(ETSCharacterState::Idle);
                }
            }, Duration, false);
        }
        else {
             UE_LOG(LogTemp, Error, TEXT("[TSCharacter] 몽타주 재생 실패! (Duration 0) - 슬롯 설정(DefaultGroup 등)이나 블렌드를 확인하세요."));
             SetCharacterState(ETSCharacterState::Idle);
        }
    }
    else {
        UE_LOG(LogTemp, Error, TEXT("[TSCharacter] ParriedMontage가 비어있습니다! 블루프린트에서 설정해주세요."));
        SetCharacterState(ETSCharacterState::Idle);
    }
}

void ATSCharacter::Die() {
    SetCharacterState(ETSCharacterState::Dead); // 사망 상태로 설정

    // [중요] 죽는 순간 무기 콜리전 끄기 (자해가 일어나는 경우 방지)
    DisableWeaponCollision();

    // [중요] 넉백 등으로 인한 잔여 속도가 랙돌에 전달되면 날아갈 수 있음 -> 속도 제거
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->StopMovementImmediately();
        GetCharacterMovement()->DisableMovement();
    }

    // [캡슐 콜리전 끄기] 메쉬와 충돌하여 폭발(Explosion)하는 현상 방지
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

    // [모든 부착물 콜리전 끄기] 헬멧, 갑옷 등이 본체와 충돌해서 날아가는 것 방지
    TArray<USceneComponent*> AttachedComps;
    GetMesh()->GetChildrenComponents(true, AttachedComps);
    for (USceneComponent* Child : AttachedComps)
    {
        if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(Child))
        {
            Prim->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
    }

    // [Ragdoll 설정]
    GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
    GetMesh()->SetSimulatePhysics(true);
    
    // [카메라 줌인 방지] 랙돌 메쉬가 카메라를 막지 않도록 설정
    GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

    // 랙돌이 너무 가벼워서 날아가는 경우를 대비해 댐핑 추가 (선택 사항)
    GetMesh()->SetLinearDamping(0.5f);
    GetMesh()->SetAngularDamping(0.5f);
}

// ========== [피격 리액션 종료] ==========
void ATSCharacter::OnHitReactionEnded(UAnimMontage *Montage, bool bInterrupted) {
    if (CurrentState == ETSCharacterState::Stunned) {
        SetCharacterState(ETSCharacterState::Idle);
        UE_LOG(LogTemp, Warning, TEXT("[TSCharacter] 피격 경직 해제 → Idle 복구 (interrupted: %s)"), bInterrupted ? TEXT("Yes") : TEXT("No"));
    }
}

// ========== [넉백 적용 - 공통 함수] ==========
// ========== [넉백 적용 - 공통 함수] ==========
void ATSCharacter::ApplyKnockback(AActor *Source) {
    if (!Source) return;

    // [이동 가능한 피격] 넉백 강도가 약하면(예: 300 이하) 넉백(밀림)을 적용하지 않음
    // -> 이러면 하체는 계속 움직일 수 있음 (Layered Blend Per Bone 적용 시)
    if (KnockbackStrength <= 300.0f) 
    {
        UE_LOG(LogTemp, Warning, TEXT("[TSCharacter] 넉백 생략 (강도 %.0f <= 300) - 이동 가능"), KnockbackStrength);
        return; 
    }

    // 소스가 무기면 소유자(캐릭터) 위치 사용
    AActor *AttackSource = Source->GetOwner() ? Source->GetOwner() : Source;
    FVector KnockDir = GetActorLocation() - AttackSource->GetActorLocation();
    KnockDir.Z = 0.f;
    float Distance = KnockDir.Size();
    KnockDir.Normalize();

    // 거리 기반 스케일: 100유닛 이하 → 풀 넉백, 멀어질수록 감소 (최소 30%)
    float DistScale = FMath::Clamp(1.0f - (Distance - 100.f) / 400.f, 0.3f, 1.0f);
    FVector KnockVelocity = KnockDir * KnockbackStrength * DistScale;
    KnockVelocity.Z = KnockbackUpForce;

    // [이동 정지 후 날리기]
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->StopMovementImmediately(); // 기존 이동 멈춤
    }
    LaunchCharacter(KnockVelocity, true, true);
    
    UE_LOG(LogTemp, Warning, TEXT("[TSCharacter] 넉백 - 거리:%.0f, 스케일:%.1f%%, 강도:%.0f"), Distance, DistScale * 100.f, KnockbackStrength * DistScale);

    // 잔여 속도 제거 타이머 (뒤뚱거림 방지)
    GetWorldTimerManager().ClearTimer(KnockbackStopTimerHandle);
    GetWorldTimerManager().SetTimer(KnockbackStopTimerHandle, this, &ATSCharacter::StopKnockback, KnockbackStopDelay, false);
}

// ========== [넉백 정지] ==========
void ATSCharacter::StopKnockback() {
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (!MoveComp) return;

    // 넉백으로 날아간 후 착지 시 미끄러짐 방지
    // 수평 속도만 제거 (Z는 유지 → 자연스러운 착지)
    FVector Vel = MoveComp->Velocity;
    Vel.X = 0.f;
    Vel.Y = 0.f;
    MoveComp->Velocity = Vel;

    // 가속도도 초기화 (잔여 이동 입력 방지)
    MoveComp->StopActiveMovement();

    // 착지 상태 보장 (Falling 모드에서 걸림 방지)
    if (MoveComp->IsFalling()) {
        MoveComp->SetMovementMode(MOVE_Walking);
    }

    // [회전 제어 복구] 뒤뚱거림 방지 (ResetCombo와 동일한 처리)
    bUseControllerRotationYaw = true; 
    UE_LOG(LogTemp, Warning, TEXT("[TSCharacter] 넉백 정지 - 회전 제어 복구"));
}

void ATSCharacter::SetCharacterState(ETSCharacterState NewState) {
    CurrentState = NewState; // 상태설정
}

void ATSCharacter::ResetStats()
{
    CurrentHealth = MaxHealth;
    CurrentStamina = MaxStamina;
    
    // 필요한 경우 물리/캡슐 콜리전도 초기화
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GetMesh()->SetSimulatePhysics(false);
    GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh")); // 원래 프로필로 복구
    
    // 랙돌에서 복구 시 상대 위치 맞춤은 GameMode에서 처리
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->StopMovementImmediately();
        GetCharacterMovement()->SetMovementMode(MOVE_Walking);
    }
    
    // [회전 복구] 8방향 로코모션(스트레이핑)을 위해 마우스 회전(ControllerYaw)을 켜고, 방향 회전(Orient)은 끔
    bUseControllerRotationYaw = true; 
    GetCharacterMovement()->bOrientRotationToMovement = false;

    UE_LOG(LogTemp, Warning, TEXT("[TSCharacter] Stats Reset. HP: %.0f"), CurrentHealth);
}
void ATSCharacter::SetWeaponType(ETSWeaponType NewType) {
    CurrentWeaponType = NewType; // 상태설정
}
void ATSCharacter::EquipWeapon(ATSWeapon *NewWeapon) {
    if (NewWeapon) {
        CurrentWeapon = NewWeapon; //
        CurrentWeaponType = NewWeapon->GetWeaponType(); // 현재 무기 타입 설정
        FName SocketName = FName("Sword"); // 우측 손 소켓 이름
        NewWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepRelative, true), SocketName); // 무기 부착
        NewWeapon->SetOwner(this);
    }
}
void ATSCharacter::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);

    // 스태미나 자동 회복 로직
    if (bCanRecoverStamina && CurrentState != ETSCharacterState::Blocking && CurrentState != ETSCharacterState::Attacking) {
        if (CurrentStamina < MaxStamina) {
            CurrentStamina += StaminaRecoveryRate * DeltaTime; // 초당 회복량만큼 증가
            CurrentStamina = FMath::Clamp(CurrentStamina, 0.0f, MaxStamina); // 최대치 넘지 않게 조정
        }
    }

    // [락온 시스템] 캐릭터가 타겟을 바라보도록 회전 (카메라는 자유롭게)
    if (LockedTarget)
        {
            FVector TargetLoc = LockedTarget->GetActorLocation();
            FVector MyLoc = GetActorLocation();
            
		// 높이 차이는 무시하고 수평 회전만 계산
		TargetLoc.Z = MyLoc.Z; 

            FRotator TargetRot = FRotationMatrix::MakeFromX(TargetLoc - MyLoc).Rotator();
		FRotator CurrentRot = GetActorRotation();

		// 부드럽게 캐릭터 회전 (InterpSpeed 조절 가능)
            FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, 10.0f);
            
		SetActorRotation(NewRot);
    }
}
void ATSCharacter::PerformCombo(int32 SectionIndex) {
    if (UAnimMontage *Montage = CurrentWeapon->GetLightAttackMontage()) // 현재 무기의 경공격 몽타주 가져오기
    {
        FName SectionName = *FString::Printf(TEXT("Combo%d"), SectionIndex); // 섹션 이름 생성

        // 몽타주 재생 (해당 섹션으로 점프)
        PlayAnimMontage(Montage, 1.0f, SectionName); // 재생 속도 1.0f
    }
}
void ATSCharacter::ContinueCombo() {
    if (bHasNextComboInput) // (미리 입력해둔 게 있다면)
    {
        bHasNextComboInput = false; // 예약 소비)
        ComboCount++;

        if (ComboCount > 3) {
            // 3타 넘어가면 콤보 끝 (여기서 리셋하지 않고 몽타주 종료에 맡김)
            return;
		}

		// [AI 학습] 콤보 연계 기록
        if (PlayerPatternComponent)
        {
            PlayerPatternComponent->RegisterAttack(false, ComboCount);
        }

		PerformCombo(ComboCount); // 추가 (다음 콤보 실행)
	}
}
void ATSCharacter::OnAttackMontageEnded(UAnimMontage *Montage, bool bInterrupted) {
    UE_LOG(LogTemp, Warning, TEXT("[TSCharacter] OnAttackMontageEnded: Interrupted? %s"), bInterrupted ? TEXT("Yes") : TEXT("No"));

    if (bInterrupted) return; // 몽타주가 중단되었으면 무시
    // 공격 몽타주가 끝났을 때
    if (CurrentState == ETSCharacterState::Attacking) {
        // 콤보 상태 초기화
        ComboCount = 0;
        bHasNextComboInput = false; // 추가
        bUseControllerRotationYaw = true; // 추가 (다시 마우스 회전 가능)
        SetCharacterState(ETSCharacterState::Idle);

        // [핵심] 공격 후 딜레이(쿨타임) 적용 -> 바로 다시 공격 못하게 함
        bAttackCooldown = true; // 추가
        GetWorldTimerManager().SetTimer(CooldownTimerHandle, this, &ATSCharacter::ResetAttackCooldown, AttackCooldownTime, false); // 추가

        UE_LOG(LogTemp, Warning, TEXT("Combo Finished. Cooldown Started.")); // 추가
    }
}
void ATSCharacter::ResetAttackCooldown() // 추가
{
    bAttackCooldown = false; // 추가 (쿨타임 해제)
    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Green, TEXT("Attack Ready")); // 추가
}
void ATSCharacter::ResetCombo() {
    // 피격 등으로 강제 취소될 때 사용
    ComboCount = 0;
    bHasNextComboInput = false; // 추가
    bAttackCooldown = false; // 추가

    bUseControllerRotationYaw = true;
    SetCharacterState(ETSCharacterState::Idle);

    // [중요] 몽타주 중단 시 무기 콜리전도 강제로 껴야 함
    DisableWeaponCollision();
    StopAnimMontage(); // 추가 (몽타주 즉시 정지)
}

void ATSCharacter::EnableWeaponCollision() {
    if (CurrentWeapon) {
        CurrentWeapon->EnableCollision();
    }
}

void ATSCharacter::DisableWeaponCollision() {
    if (CurrentWeapon) {
        CurrentWeapon->DisableCollision();
    }
}

void ATSCharacter::SetLockOnTarget(AActor* Target)
{
    LockedTarget = Target;
    if (LockedTarget)
    {
        UE_LOG(LogTemp, Warning, TEXT("[TSCharacter] Lock-on Target Set: %s"), *LockedTarget->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[TSCharacter] Lock-on Target Cleared"));
    }
}
