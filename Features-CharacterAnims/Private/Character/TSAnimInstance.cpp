#include "Character/TSAnimInstance.h"
#include "Character/TSCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "KismetAnimationLibrary.h"

void UTSAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

	TSCharacter = Cast<ATSCharacter>(TryGetPawnOwner()); //캐릭터 소유자 가져오기
    if (TSCharacter)
    {
		CharacterMovement = TSCharacter->GetCharacterMovement(); //캐릭터 무브먼트 컴포넌트 가져오기
    }
}

void UTSAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
    Super::NativeUpdateAnimation(DeltaTime);

    if (TSCharacter == nullptr)
    {
		TSCharacter = Cast<ATSCharacter>(TryGetPawnOwner()); //캐릭터 소유자 다시 가져오기
    }

	if (TSCharacter && CharacterMovement) //캐릭터 및 무브먼트 컴포넌트가 유효한 경우
    {
        // 1. 속력(GroundSpeed) 업데이트 (XY 평면 속도)
		GroundSpeed = UKismetMathLibrary::VSizeXY(CharacterMovement->Velocity); // 수평 속도 크기 계산

        if (GroundSpeed > 3.0f)
        {
            FRotator BaseRotation = TSCharacter->GetActorRotation();
            FVector Velocity = CharacterMovement->Velocity;

            // ★ 중요: UKismetMathLibrary가 아니라 UKismetAnimationLibrary 입니다!
            Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, BaseRotation);

            // 4방향 스냅 적용
            LocomotionDirection = Snap4Way(Direction);
        }
        else
        {
            Direction = 0.0f;
            LocomotionDirection = 0.0f;
        }

        // 3. 캐릭터 상태 및 무기 업데이트
        CharacterState = TSCharacter->GetCharacterState();
        WeaponType = TSCharacter->GetWeaponType();
    }
}
float UTSAnimInstance::Snap4Way(float Angle)
{
    // 0~360 범위로 변환 (음수 제거하여 블렌드 스페이스 보간 문제 해결)
    float Normalized = Angle;
    if (Normalized < 0.0f)
    {
        Normalized += 360.0f;
    }

    // 4방향 스냅 (대각선 입력 시 떨림 방지를 위해 전방/후방 영역을 조금 더 넓게 잡음 ±50도)
    // WA(315도), WD(45도) 입력이 확실하게 '전방(0)'으로 판정되도록 수정

    if (Normalized >= 310.0f || Normalized < 50.0f)
    {
        return 0.0f;    // 앞 (0도)
    }
    else if (Normalized >= 50.0f && Normalized < 130.0f)
    {
        return 90.0f;   // 우 (90도)
    }
    else if (Normalized >= 130.0f && Normalized < 230.0f)
    {
        return 180.0f;  // 뒤 (180도)
    }
    else // 230.0f ~ 310.0f
    {
        return 270.0f;  // 좌 (270도)
    }
}
