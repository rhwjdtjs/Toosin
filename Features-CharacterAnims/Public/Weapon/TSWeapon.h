#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Headers/ATSWeaponTypes.h"
#include "TSWeapon.generated.h"

UCLASS()
class TOOSIN_API ATSWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	ATSWeapon();

protected:
	virtual void BeginPlay() override;

public:	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|Components")
	class USceneComponent* Root;
		//무기 메쉬
		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|Weapon")
		class UStaticMeshComponent* WeaponMesh; // 무기 메쉬 컴포넌트

		// 무기 타입 (양손검, 도끼 등)
		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Toosin|Weapon")
		ETSWeaponType WeaponType; // 무기 타입

		//왼손 IK 소켓 이름 ( 기본값: "LeftHandSocket")
		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Toosin|Weapon")
		FName LeftHandSocketName; // 왼손 IK 소켓 이름

		// 왼손 소켓  Transform 반환
		UFUNCTION(BlueprintCallable, Category = "Toosin|Weapon")
		FTransform GetLeftHandSocketTransform() const;
		bool HasLeftHandSocket() const;
		//무기 타입 반환
		UFUNCTION(BlueprintCallable, Category = "Toosin|Weapon")
		FORCEINLINE ETSWeaponType GetWeaponType() const { return WeaponType; }

		// 공격 몽타주
		UPROPERTY(EditDefaultsOnly, Category = "Toosin|Animation")
		class UAnimMontage* LightAttackMontage; // 일반 공격(콤보) 몽타주

		UPROPERTY(EditDefaultsOnly, Category = "Toosin|Animation")
		class UAnimMontage* HeavyAttackMontage; // 강공격 몽타주

		FORCEINLINE class UAnimMontage* GetLightAttackMontage() const { return LightAttackMontage; }
		FORCEINLINE class UAnimMontage* GetHeavyAttackMontage() const { return HeavyAttackMontage; }

};
