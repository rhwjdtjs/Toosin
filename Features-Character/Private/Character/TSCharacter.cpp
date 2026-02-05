#include "Toosin/Public/Character/TSCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/InputComponent.h"
#include "Components/CapsuleComponent.h"

ATSCharacter::ATSCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// 카메라 설정
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom")); //카메라 붐 생성
	CameraBoom->SetupAttachment(RootComponent); //루트 컴포넌트에 부착
	CameraBoom->TargetArmLength = 400.0f; //카메라 거리 설정
	CameraBoom->bUsePawnControlRotation = true; //회전에 따라 카메라 회전

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera")); //팔로우 카메라 생성
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); //카메라 붐에 부착
	FollowCamera->bUsePawnControlRotation = false; //카메라 자체 회전 비활성화

	// 캐릭터 무브먼트 설정
	bUseControllerRotationYaw = true; //컨트롤러 회전에 따라 캐릭터 회전 활성화
	GetCharacterMovement()->bOrientRotationToMovement = false; //이동 방향으로 회전 활성화

	//기본값 설정
	MaxHealth = 100.f;
	CurrentHealth = MaxHealth;
	MaxStamina = 100.f;
	CurrentStamina = MaxStamina;

	//기본 Row Name 설정
	CharacterStatRowName = FName(TEXT("Default"));
	CurrentState = ETSCharacterState::Idle; //초기 상태 설정
}

void ATSCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	//스탯 초기화
	InitializeStats(); //Data Table에서 스탯 불러오기

	if(APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer())) //로컬 플레이어 서브시스템 가져오기
		{
			if(DefaultMappingContext)
				Subsystem->AddMappingContext(DefaultMappingContext, 0); //입력 매핑 컨텍스트 추가
		}
	}
}

void ATSCharacter::InitializeStats()
{
	if (CharacterStatsTable)
	{
		// 데이터 테이블에서 Row 찾기
		static const FString ContextString(TEXT("Character Stats Context")); //컨텍스트 문자열
		FTSCharacterStats* Stats = CharacterStatsTable->FindRow<FTSCharacterStats>(CharacterStatRowName, ContextString, true); //Row 찾기
		if (Stats)
		{
			MaxHealth = Stats->MaxHealth; //스탯 설정
			MaxStamina = Stats->MaxStamina; //스탯 설정
			WalkSpeed = Stats->WalkSpeed; //스탯 설정

			// 적용
			CurrentHealth = MaxHealth; //현재 체력 초기화
			CurrentStamina = MaxStamina; //현재 스태미나 초기화
			GetCharacterMovement()->MaxWalkSpeed = WalkSpeed; //걷기 속도 적용

			UE_LOG(LogTemp, Warning, TEXT("Stats Loaded from DataTable: Health=%f, Speed=%f"), MaxHealth, WalkSpeed); //로그 출력
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Stats Row Not Found: %s"), *CharacterStatRowName.ToString()); //Row를 찾지 못했을 때 로그 출력
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("CharacterStatsTable is NULL! Using default hardcoded values.")); //데이터 테이블이 없을 때 로그 출력
	}
}

void ATSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) //Enhanced Input 컴포넌트로 캐스팅
	{
		if (MoveAction) EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATSCharacter::Move); //이동 액션 바인딩
		if (LookAction) EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATSCharacter::Look); //시점 조작 액션 바인딩
		if (DodgeAction) EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Started, this, &ATSCharacter::Dodge); //회피 액션 바인딩
		if (GuardAction) //방어 액션 바인딩
		{
			EnhancedInputComponent->BindAction(GuardAction, ETriggerEvent::Started, this, &ATSCharacter::GuardStart); //가드 시작
			EnhancedInputComponent->BindAction(GuardAction, ETriggerEvent::Completed, this, &ATSCharacter::GuardEnd); //가드 종료
		}
		if (LightAttackAction) EnhancedInputComponent->BindAction(LightAttackAction, ETriggerEvent::Started, this, &ATSCharacter::LightAttack); //일반공격 액션 바인딩
		if (HeavyAttackAction) EnhancedInputComponent->BindAction(HeavyAttackAction, ETriggerEvent::Started, this, &ATSCharacter::HeavyAttack); //강공격 액션 바인딩
	}
}
void ATSCharacter::Move(const FInputActionValue& Value)
{
	if (CurrentState == ETSCharacterState::Attacking || CurrentState == ETSCharacterState::Stunned || CurrentState == ETSCharacterState::Dead) //공격 상태이거나, 스턴 상태이거나 , 죽은상태에서는
		return; //움직일 수 없게 리턴한다.

	FVector2D MovementVector = Value.Get<FVector2D>(); //입력값을 2D 벡터로 가져온다.

	if (Controller != nullptr) //컨트롤러가 유효하다면
	{
		const FRotator Rotation = Controller->GetControlRotation(); //컨트롤러의 회전값을 가져온다.
		const FRotator YawRotation(0, Rotation.Yaw, 0); //Yaw(수평) 회전값만 사용
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X); //앞 방향 벡터 계산
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y); //오른쪽 방향 벡터 계산

		AddMovementInput(ForwardDirection, MovementVector.Y); //앞뒤 이동
		AddMovementInput(RightDirection, MovementVector.X); //좌우 이동
	}
}

void ATSCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>(); //입력값을 2D 벡터로 가져온다.
	if (Controller != nullptr) //컨트롤러가 유효하다면
	{
		AddControllerYawInput(LookAxisVector.X); //수평 시점 조작
		AddControllerPitchInput(LookAxisVector.Y); //수직 시점 조작
	}
}

void ATSCharacter::Dodge()
{
	if (CurrentState == ETSCharacterState::Attacking || CurrentState == ETSCharacterState::Idle || CurrentState == ETSCharacterState::Moving) //공격 상태이거나, 대기 상태이거나 , 이동 상태일 때
	{
		UE_LOG(LogTemp, Warning, TEXT("Dodge Action!")); //로그 출력
		// 스태미나 소모 및 몽타주 재생 로직 추가 예정
	}
}

void ATSCharacter::GuardStart()
{
	if (CurrentState == ETSCharacterState::Idle || CurrentState == ETSCharacterState::Moving) //대기 상태이거나 , 이동 상태일 때
	{
		SetCharacterState(ETSCharacterState::Blocking); //상태를 가드 상태로 변경
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed * 0.5f; //가드 시 걷기 속도 절반으로 감소
		UE_LOG(LogTemp, Warning, TEXT("Guard Start")); //로그 출력
	}
}

void ATSCharacter::GuardEnd()
{
	if (CurrentState == ETSCharacterState::Blocking) //가드 상태일 때
	{
		SetCharacterState(ETSCharacterState::Idle); //상태를 대기 상태로 변경
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed; //걷기 속도 원래대로 복구
		UE_LOG(LogTemp, Warning, TEXT("Guard End")); //로그 출력
	}
}

void ATSCharacter::LightAttack()
{
	if (CurrentState == ETSCharacterState::Blocking) return; //가드 상태일 때는 공격 불가
	UE_LOG(LogTemp, Warning, TEXT("Light Attack!")); //로그 출력
}

void ATSCharacter::HeavyAttack()
{
	if (CurrentState == ETSCharacterState::Blocking) return; //가드 상태일 때는 공격 불가
	UE_LOG(LogTemp, Warning, TEXT("Heavy Attack!")); //로그 출력
}

float ATSCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (CurrentState == ETSCharacterState::Dead) return 0.0f; //이미 죽은 상태라면 데미지 무시

	if (CurrentState == ETSCharacterState::Blocking) //가드 상태라면
	{
		UE_LOG(LogTemp, Warning, TEXT("Blocked Damage!")); //로그 출력
		CurrentStamina -= DamageAmount * 0.5f; // 예시: 스태미나로 데미지 받음
		return 0.0f;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.0f, MaxHealth); //체력 감소 및 클램프

	if (CurrentHealth <= 0.0f)
	{
		Die(); //사망 처리
	}
	return ActualDamage; //실제 데미지 반환
}

void ATSCharacter::Die()
{
	SetCharacterState(ETSCharacterState::Dead); //상태를 죽은 상태로 변경
	GetMesh()->SetSimulatePhysics(true); //메쉬에 물리 시뮬레이션 활성화
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision); //캡슐 콜리전 비활성화
}

void ATSCharacter::SetCharacterState(ETSCharacterState NewState)
{
	CurrentState = NewState; //상태 변경
}
void ATSCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}