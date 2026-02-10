#include "Toosin/Public/Weapon/TSWeapon.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Toosin/Public/Character/TSCharacter.h" 
#include "Toosin/Public/Character/TSCombatComponent.h"

ATSWeapon::ATSWeapon()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(Root);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 무기 메쉬는 단순 외형이므로 충돌 꺼둠 (카메라 줌인, 캐릭터 이동 방해 방지)

	// 충돌 박스 생성 및 설정
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(WeaponMesh);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 기본은 꺼둠
	CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap); // 캐릭터 충돌 감지
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap); // 무기끼리 충돌 감지
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore); // 카메라는 확실히 무시
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Ignore); // 카메라는 확실히 무시
	
	CollisionBox->SetGenerateOverlapEvents(true); // 오버랩 이벤트 생성 활성화


	LeftHandSocketName = TEXT("LeftHandSocket");
}

void ATSWeapon::BeginPlay()
{
	Super::BeginPlay();

	// 오버랩 이벤트 바인딩
	if (CollisionBox)
	{
		CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &ATSWeapon::OnBoxOverlap);
	}

	// 주인이 있다면 주인과의 충돌 무시 (이동 시 걸리적거림 방지)
	if (GetOwner())
	{
		CollisionBox->IgnoreActorWhenMoving(GetOwner(), true);
		if (UPrimitiveComponent* OwnerRoot = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent()))
		{
			CollisionBox->IgnoreComponentWhenMoving(OwnerRoot, true);
		}
	}
}

void ATSWeapon::EnableCollision()
{
	if (CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		IgnoreActors.Empty(); // 이전 공격의 무시 리스트 초기화
		
		// [Debug] 콜리전 박스 상태 로깅
		FVector BoxExtent = CollisionBox->GetScaledBoxExtent();
		FVector BoxLoc = CollisionBox->GetComponentLocation();
		UE_LOG(LogTemp, Warning, TEXT("[TSWeapon] Collision Enabled. Box Extent: %s, Location: %s"), *BoxExtent.ToString(), *BoxLoc.ToString());
	}
}

void ATSWeapon::DisableCollision()
{
	if (CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		IgnoreActors.Empty();
		UE_LOG(LogTemp, Warning, TEXT("[TSWeapon] Collision Disabled"));
	}
}

void ATSWeapon::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// [Debug] 오버랩 발생 확인
	if (OtherActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("[TSWeapon] Overlap Event! OtherActor: %s"), *OtherActor->GetName());
	}

	if (!OtherActor || OtherActor == this || OtherActor == GetOwner()) return; // 자신이나 주인은 무시

	// 이미 이번 공격에 맞은 대상이면 무시
	if (IgnoreActors.Contains(OtherActor)) return;
	IgnoreActors.Add(OtherActor);

	// 1. 캐릭터 피격 처리 (Pawn)
	// ATSCharacter 뿐만 아니라 모든 ACharacter (더미 포함) 상대로 데미지 적용
	if (OtherActor->IsA(ACharacter::StaticClass()))
	{
		UE_LOG(LogTemp, Warning, TEXT("[TSWeapon] Hit Character: %s"), *OtherActor->GetName());

		UGameplayStatics::ApplyDamage(OtherActor, 10.0f, GetInstigatorController(), this, UDamageType::StaticClass());
	}
	
	
	// 2. 무기끼리 충돌 (패링) 처리
	// 상대 무기와 부딪혔을 때, 상대방이 '패링 중'이라면?
	ATSWeapon* OtherWeapon = Cast<ATSWeapon>(OtherActor);
	if (OtherWeapon)
	{
		// 이 무기의 주인이 패링 중인지 확인? 아니면 상대 무기의 주인이 패링 중인지?
		// 상황: Dummy가 공격(이 무기) -> Player가 패링 중 -> Player의 무기(OtherWeapon)에 닿음.
		// Player의 무기에 닿았을 때 Player가 패링 상태라면 패링 성공 처리.
		
		ATSCharacter* OtherChar = Cast<ATSCharacter>(OtherWeapon->GetOwner());
		if (OtherChar && OtherChar->GetCombatComponent() && OtherChar->GetCombatComponent()->bIsParryWindow)
		{
			// 주인(공격자)이 아닌 경우에만 패링 성공 처리
			if (OtherChar != GetOwner())
			{
				UE_LOG(LogTemp, Warning, TEXT("[TSWeapon] Weapon Clashed! Parry Triggered on %s"), *OtherChar->GetName());
				OtherChar->GetCombatComponent()->OnParrySuccess(GetOwner()); 
			}
		}
	}
}

FTransform ATSWeapon::GetLeftHandSocketTransform() const
{
	return WeaponMesh->GetSocketTransform(LeftHandSocketName);
}

bool ATSWeapon::HasLeftHandSocket() const
{
	return WeaponMesh->DoesSocketExist(LeftHandSocketName);
}
