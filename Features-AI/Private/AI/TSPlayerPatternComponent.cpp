#include "Toosin/Public/AI/TSPlayerPatternComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

UTSPlayerPatternComponent::UTSPlayerPatternComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SampleTimer = 0.0f;
}

void UTSPlayerPatternComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
    
    // 초기 적 타겟 검색 (1v1 가정)
    // 실제로는 GameMode나 Controller에서 설정해주는 것이 정확함
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), FoundActors);
    for (AActor* Actor : FoundActors)
    {
        if (Actor != OwnerCharacter)
        {
            EnemyTarget = Actor;
            break;
        }
    }
}

void UTSPlayerPatternComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// [이동 패턴 샘플링]
    // 일정 간격으로 적과의 거리 및 횡이동 여부를 체크
    SampleTimer += DeltaTime;
    if (SampleTimer >= SampleInterval)
    {
        SampleTimer = 0.0f;
        
        if (OwnerCharacter && EnemyTarget)
        {
            // 1. 거리 측정
            float Distance = OwnerCharacter->GetDistanceTo(EnemyTarget);
            CurrentRoundData.AccumulatedDistance += Distance;
            CurrentRoundData.DistanceSampleCount++;
            
            // 2. 횡이동(Side Step) 감지
            // 캐릭터의 Velocity와 적을 향한 방향(Forward)의 내적을 이용
            FVector Velocity = OwnerCharacter->GetVelocity();
            if (Velocity.SizeSquared() > 100.0f) // 움직이고 있을 때만
            {
                FVector ToEnemy = (EnemyTarget->GetActorLocation() - OwnerCharacter->GetActorLocation()).GetSafeNormal();
                FVector MoveDir = Velocity.GetSafeNormal();
                
                // 내적이 0에 가까우면 수직(횡) 이동
                float Dot = FVector::DotProduct(ToEnemy, MoveDir);
                if (FMath::Abs(Dot) < 0.5f) // 대략 60~120도 사이
                {
                    CurrentRoundData.SideStepCount++;
                }
            }
        }
    }
}

void UTSPlayerPatternComponent::RegisterAttack(bool bIsHeavy, int32 ComboCount)
{
    if (bIsHeavy)
    {
        CurrentRoundData.HeavyAttackCount++;
    }
    else
    {
        CurrentRoundData.LightAttackCount++;
    }

    if (ComboCount > 0)
    {
        CurrentRoundData.TotalComboSegments = FMath::Max(CurrentRoundData.TotalComboSegments, ComboCount);
        // 콤보 시퀀스 카운트는 1타(시작)일 때만 증가시키거나, 별도 로직으로 처리
        // 여기서는 단순화하여 호출될 때마다 세지 않고, 1타일 때만 시퀀스 시작으로 간주
        if (ComboCount == 1)
        {
            CurrentRoundData.ComboSequenceCount++;
        }
    }
    
    // 디버그 로그
    UE_LOG(LogTemp, Log, TEXT("[AI_Learning] Attack Registered (Heavy: %d, Combo: %d)"), bIsHeavy, ComboCount);
}

void UTSPlayerPatternComponent::RegisterDefense(bool bIsGuard, bool bIsParry, bool bIsHit)
{
    if (bIsGuard)
    {
        CurrentRoundData.GuardSuccessCount++;
        UE_LOG(LogTemp, Log, TEXT("[AI_Learning] Guard Registered"));
    }
    else if (bIsParry)
    {
        CurrentRoundData.ParrySuccessCount++;
        UE_LOG(LogTemp, Log, TEXT("[AI_Learning] Parry Registered"));
    }
    else if (bIsHit)
    {
        CurrentRoundData.HitCount++;
        UE_LOG(LogTemp, Log, TEXT("[AI_Learning] Hit Registered"));
    }
}

void UTSPlayerPatternComponent::RegisterDodge()
{
    CurrentRoundData.DodgeCount++;
    UE_LOG(LogTemp, Log, TEXT("[AI_Learning] Dodge Registered"));
}

void UTSPlayerPatternComponent::RegisterMovement(float Distance, bool bSideStep)
{
    CurrentRoundData.AccumulatedDistance += Distance;
    CurrentRoundData.DistanceSampleCount++;
    
    if (bSideStep)
    {
        CurrentRoundData.SideStepCount++;
    }
}

void UTSPlayerPatternComponent::ResetRoundData()
{
    CurrentRoundData.Reset();
    UE_LOG(LogTemp, Warning, TEXT("[AI_Learning] Data Reset for New Round"));
}
