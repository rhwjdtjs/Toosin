#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Toosin/Public/Headers/ATSWeaponTypes.h"
#include "TSAnimInstance.generated.h"

UCLASS()
class TOOSIN_API UTSAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeInitializeAnimation() override; //애니메이션	초기화
	virtual void NativeUpdateAnimation(float DeltaTime) override; //애니메이션	업데이트

protected:
	UPROPERTY(BlueprintReadOnly, Category="Toosin|Character")
	class ATSCharacter* TSCharacter; //캐릭터 참조
	UPROPERTY(BlueprintReadOnly, Category="Toosin|Movement")
	class UCharacterMovementComponent* CharacterMovement; //캐릭터 무브먼트 컴포넌트 참조

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|Movement")
	float GroundSpeed; //지상 속도
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|Movement")
	float LocomotionDirection;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|Movement")
	float SmoothedLocomotionDirection; // RInterpTo로 보간된 값 (AnimGraph용)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|Movement")
	float Direction; //이동 방향

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|State")
	ETSCharacterState CharacterState; //캐릭터 상태
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Toosin|State")
	ETSWeaponType WeaponType; //무기 타입
	UPROPERTY(BlueprintReadOnly, Category = "Toosin|IK")
	float IKLeftHandAlpha = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Toosin|IK")
	FVector IKLeftHandEffectorLocation = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Toosin|IK")
	FVector IKLeftHandEffectorLocationOffset = FVector::ZeroVector;
private:
	float Snap4Way(float Angle); //4방향 스냅 함수
	void UpdateLocomotionDirection(float DeltaTime); //로코모션 방향 설정 함수
	void UpdateIKHandTransform(ATSCharacter* InCharacter); //왼손 IK 트랜스폼 업데이트 함수
};
