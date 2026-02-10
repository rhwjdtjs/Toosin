#include "Toosin/Public/Character/TSTrainingDummy.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Toosin/Public/Character/TSCharacter.h" 
#include "Weapon/TSWeapon.h" // 무기 헤더

// 생성자: 기본값 초기화
ATSTrainingDummy::ATSTrainingDummy()
{
 	// Tick 함수 실행 여부 설정
	PrimaryActorTick.bCanEverTick = true;

	// 캡슐 콜리전 설정 (충돌 감지용)
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore); // 카메라 무시

	// 메쉬 설정
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore); // 카메라 무시

	// 최대 체력 및 현재 체력 초기화
	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;

	// 더미는 움직이지 않도록 설정
	GetCharacterMovement()->MaxWalkSpeed = 0.0f; 
}

// 게임 시작 시 호출
void ATSTrainingDummy::BeginPlay()
{
	Super::BeginPlay();
	
	// 현재 체력을 최대 체력으로 설정
	CurrentHealth = MaxHealth;

	// 무기 생성 및 장착
	if (DummyWeaponClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;

		CurrentWeapon = GetWorld()->SpawnActor<ATSWeapon>(DummyWeaponClass, GetActorTransform(), SpawnParams);
		if (CurrentWeapon)
		{
			// 무기 부착 (소켓 이름: Sword - 마네킹 손에 있는 소켓)
			// *실제 에디터에서 마네킹 스켈레톤에 'Sword' 소켓이 있는지 확인 필요. (만약 없으면 hand_r 등에 붙임)
			FName SocketName = TEXT("Sword"); // or "Sword"
			CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);
			UE_LOG(LogTemp, Warning, TEXT("[Dummy] Weapon Equipped: %s"), *CurrentWeapon->GetName());
		}
	}

	// 일정 시간(AttackInterval)마다 AttackPlayer 함수를 반복 호출하는 타이머 설정
	GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &ATSTrainingDummy::AttackPlayer, AttackInterval, true);
}

// 매 프레임 호출 (현재 사용하지 않음)
void ATSTrainingDummy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// 플레이어를 공격하는 함수
void ATSTrainingDummy::AttackPlayer()
{
	UE_LOG(LogTemp, Warning, TEXT("[Dummy] Attempting Attack..."));

	// 1. 공격 몽타주가 있다면 재생
	if (AttackMontage)
	{
		float Duration = PlayAnimMontage(AttackMontage);
		UE_LOG(LogTemp, Warning, TEXT("[Dummy] Montage Played. Duration: %f"), Duration);

		// 2. 무기 충돌 활성화
		// (원래는 몽타주의 AnimNotifyState_Collision 등으로 제어해야 정확하지만, 
		//  코드레벨 테스트를 위해 공격 시작 0.2초 후 켜지고 0.5초 후 꺼지게 대충 설정)
		if (CurrentWeapon)
		{
			// 0.4초는 너무 늦음(로그상 몽타주 길이가 0.4초였음). 0.1초로 수정.
			GetWorldTimerManager().SetTimer(CollisionTimerHandle, this, &ATSTrainingDummy::EnableWeaponCollision, 0.1f, false);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Dummy] Attack Montage is NULL!"));
	}
}

void ATSTrainingDummy::EnableWeaponCollision()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->EnableCollision();
		// 0.4초 뒤에 끄기
		GetWorldTimerManager().SetTimer(CollisionTimerHandle, this, &ATSTrainingDummy::DisableWeaponCollision, 0.4f, false);
	}
}

void ATSTrainingDummy::DisableWeaponCollision()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->DisableCollision();
	}
}


// 데미지를 받는 함수 (플레이어가 때렸을 때)
float ATSTrainingDummy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// 부모 클래스의 TakeDamage 호출하여 기본 로직 수행
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// 체력 감소
	CurrentHealth -= ActualDamage;
	
	// 로그 출력: 누가, 얼마만큼의 데미지를, 남은 체력은 얼마인지
	UE_LOG(LogTemp, Warning, TEXT("[Dummy] Hit by %s! Damage: %f, Health: %f"), *DamageCauser->GetName(), ActualDamage, CurrentHealth);

	// 체력이 0 이하라면 사망 처리 (필요시 구현)
	if (CurrentHealth <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Dummy] Destroyed!"));
		CurrentHealth = MaxHealth; // 테스트를 위해 체력 리셋
	}

	return ActualDamage;
}
