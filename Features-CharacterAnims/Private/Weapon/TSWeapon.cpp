#include "Weapon/TSWeapon.h"

ATSWeapon::ATSWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(Root);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 무기는 기본적으로 충돌 없음 (오버랩 등으로 처리)
	LeftHandSocketName = FName("LeftHandSocket"); // 기본 왼손 소켓 이름
	WeaponType = ETSWeaponType::OneHanded; // 기본값
}

void ATSWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

FTransform ATSWeapon::GetLeftHandSocketTransform() const
{
	if (WeaponMesh && WeaponMesh->DoesSocketExist(LeftHandSocketName))
	{
		return WeaponMesh->GetSocketTransform(LeftHandSocketName, RTS_World); // 월드 좌표계에서 소켓 트랜스폼 반환
	}
	return FTransform::Identity; // 소켓이 없으면 기본 트랜스폼 반환
}
bool ATSWeapon::HasLeftHandSocket() const
{
	return WeaponMesh && WeaponMesh->DoesSocketExist(LeftHandSocketName); // 소켓 존재 여부 반환
}


