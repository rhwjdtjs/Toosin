#include "Character/TSAnimInstance.h"
#include "Character/TSCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "KismetAnimationLibrary.h"
#include "Weapon/TSWeapon.h"
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
    if (TSCharacter == nullptr) TSCharacter = Cast<ATSCharacter>(TryGetPawnOwner()); //캐릭터 소유자 다시 가져오기
	if (TSCharacter && CharacterMovement) //캐릭터 및 무브먼트 컴포넌트가 유효한 경우
    {
		GroundSpeed = UKismetMathLibrary::VSizeXY(CharacterMovement->Velocity); // 수평 속도 크기 계산

        if (GroundSpeed > 3.0f) UpdateLocomotionDirection(DeltaTime); //로코모션 방향 설정
        else
        {
            Direction = 0.0f;
            LocomotionDirection = 0.0f;
            SmoothedLocomotionDirection = 0.0f;
        }
        CharacterState = TSCharacter->GetCharacterState();
        WeaponType = TSCharacter->GetWeaponType();
        UpdateIKHandTransform(TSCharacter); //왼손 IK 트랜스폼 업데이트
    }
}
float UTSAnimInstance::Snap4Way(float Angle)
{
    float Normalized = Angle;
    if (Normalized < 0.0f)
    {
        Normalized += 360.0f;
    }
    // 4방향 스냅 (대각선 입력 시 떨림 방지를 위해 전방 영역을 더욱 넓게 잡음 ±60도)
    // WA(315도), WD(45도) 입력이 경계선에서 떨리지 않고 확실하게 '전방(0)'으로 판정되도록 수정
    if (Normalized >= 300.0f || Normalized < 60.0f)
    {
        return 0.0f;    // 앞 (0도) -- [300 ~ 60 범위]
    }
    else if (Normalized >= 60.0f && Normalized < 120.0f)
    {
        return 90.0f;   // 우 (90도) -- [60 ~ 120 범위]
    }
    else if (Normalized >= 120.0f && Normalized < 240.0f)
    {
        return 180.0f;  // 뒤 (180도) -- [120 ~ 240 범위] (후방도 넓게 잡음)
    }
    else // 240.0f ~ 300.0f
    {  
        return 270.0f;  // 좌 (270도) -- [240 ~ 300 범위]
    }
}
void UTSAnimInstance::UpdateLocomotionDirection(float DeltaTime)
{
    FRotator BaseRotation = TSCharacter->GetActorRotation();
    FVector Velocity = CharacterMovement->Velocity;
    FVector Acceleration = CharacterMovement->GetCurrentAcceleration();

    // 가속도(입력)가 있으면 그것을 기준으로 방향 계산 (마우스 회전 시 벨로시티 랙으로 인한 튐 방지)
    FVector DirectionVector = (Acceleration.SizeSquared() > 0.1f) ? Acceleration : Velocity;
    Direction = UKismetAnimationLibrary::CalculateDirection(DirectionVector, BaseRotation);
    // 4방향 스냅 적용
    LocomotionDirection = Snap4Way(Direction); // 스냅된 목표 방향
    // [Hysteresis] 좌측(300~360)에서 전방(0)으로 올 때, 0이 아닌 360을 유지하도록 강제
    // (WA -> W 전환 시 360->0 튀는 현상 방지)
    if (FMath::IsNearlyZero(LocomotionDirection) && SmoothedLocomotionDirection > 180.0f) // 0도 근처에서 좌측에서 올 때
    {
        LocomotionDirection = 360.0f; // 360도로 강제 설정
    }
    FRotator CurrentRot = FRotator(0.0f, SmoothedLocomotionDirection, 0.0f); // 현재 스무딩된 회전
    FRotator TargetRot = FRotator(0.0f, LocomotionDirection, 0.0f); // 목표 회전
    FRotator ResultRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, 10.0f); // 보간 속도 10.0
    SmoothedLocomotionDirection = ResultRot.Yaw;
    if (SmoothedLocomotionDirection < 0.0f)
    {
        SmoothedLocomotionDirection += 360.0f; // 다시 0~360 범위로 복구
    }
}
void UTSAnimInstance::UpdateIKHandTransform(ATSCharacter* InCharacter)
{
    IKLeftHandAlpha = 0.f;
    IKLeftHandEffectorLocation = FVector::ZeroVector;

    if (!InCharacter) return;

    ATSWeapon* Weapon = InCharacter->GetCurrentWeapon(); // 현재 장착된 무기 가져오기
    if (!Weapon) return;

    if (!(WeaponType == ETSWeaponType::TwoHandedSword ||
        WeaponType == ETSWeaponType::TwoHandedAxe ||
        WeaponType == ETSWeaponType::Polearm))
    {
        return;
    }

	if (!Weapon->HasLeftHandSocket()) return; // 왼손 소켓이 없으면 종료

	USkeletalMeshComponent* CharMesh = InCharacter->GetMesh(); // 캐릭터 메쉬 가져오기
	if (!CharMesh) return; // 안전 검사

    // 1) 무기 소켓 월드 트랜스폼
    const FTransform SocketWorld = Weapon->GetLeftHandSocketTransform(); // 월드라고 가정

    // 2) 'hand_r' 기준 Bone Space로 변환 (ABP 설정과 1:1 매칭)
    FVector BoneSpacePos;
    FRotator BoneSpaceRot; // 필요 없으면 무시 가능

	CharMesh->TransformToBoneSpace( // Bone Space 변환 함수
		FName("hand_r"), // 우측 손 뼈대 이름
		SocketWorld.GetLocation(), // 소켓 월드 위치
		SocketWorld.Rotator(), // 소켓 월드 회전
		BoneSpacePos, // out 위치
		BoneSpaceRot // out 회전
    );

	IKLeftHandEffectorLocation = BoneSpacePos + IKLeftHandEffectorLocationOffset; // 오프셋 적용
	IKLeftHandAlpha = 1.f; // IK 적용
}

