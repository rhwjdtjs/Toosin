#include "Toosin/Public/Animation/ANS_WeaponCollision.h"
#include "Toosin/Public/Character/TSCharacter.h"
#include "Toosin/Public/Weapon/TSWeapon.h"

void UANS_WeaponCollision::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (MeshComp && MeshComp->GetOwner())
	{
		if (ATSCharacter* Character = Cast<ATSCharacter>(MeshComp->GetOwner()))
		{
			// 캐릭터에게 무기 콜리전을 켜라고 요청
			Character->EnableWeaponCollision();
		}
	}
}

void UANS_WeaponCollision::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (MeshComp && MeshComp->GetOwner())
	{
		if (ATSCharacter* Character = Cast<ATSCharacter>(MeshComp->GetOwner()))
		{
			// 캐릭터에게 무기 콜리전을 끄라고 요청
			Character->DisableWeaponCollision();
		}
	}
}
