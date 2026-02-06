#include "Weapon/TSWeapon.h"

ATSWeapon::ATSWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(Root);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 무기는 기본적으로 충돌 없음 (오버랩 등으로 처리)
	LeftHandSocketName = FName("LeftHandSocket");
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
		return WeaponMesh->GetSocketTransform(LeftHandSocketName, RTS_World);
	}
	return FTransform::Identity;
}
bool ATSWeapon::HasLeftHandSocket() const
{
	return WeaponMesh && WeaponMesh->DoesSocketExist(LeftHandSocketName);
}


