#include "Toosin/Public/AI/BTTask_ExecuteAttack.h"
#include "AIController.h"
#include "Toosin/Public/Character/TSCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Toosin/Public/AI/ATSEnemyController.h"

UBTTask_ExecuteAttack::UBTTask_ExecuteAttack()
{
	NodeName = TEXT("Execute Attack");
    bNotifyTick = true; // [중요] Tick 활성화하여 몽타주 끝날 때까지 대기
}

EBTNodeResult::Type UBTTask_ExecuteAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	ATSCharacter* AIChar = Cast<ATSCharacter>(AIController->GetPawn());
	if (!AIChar) return EBTNodeResult::Failed;

	// [중요] 경직(Stunned) 상태이거나 사망 상태면 공격 불가
	if (AIChar->GetCharacterState() == ETSCharacterState::Stunned || AIChar->GetCharacterState() == ETSCharacterState::Dead)
	{
		return EBTNodeResult::Failed;
	}

	// [공격 전 회전 보정] 공격 시작 순간에만 타겟을 바라봄 (에임핵 방지)
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	
	// [사거리 체크] 삭제됨 - 아래에서 대쉬 공격(원거리) 분기 처리를 위해 제거
    /*
    if (Blackboard)
    {
        AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TEXT("PlayerActor")));
        // ...
            if (Distance > 300.0f) return EBTNodeResult::Failed;
    }
    */

    // [공격 전 회전 보정] ...
	AActor* TargetActor = Cast<AActor>(Blackboard ? Blackboard->GetValueAsObject(TEXT("PlayerActor")) : nullptr);
	if (TargetActor)
	{
		// [수정] "공격을 잘못해" -> 에임 트래킹 문제 해결
        // 공격 직전에 회전을 고정(SetActorRotation)하고 포커스를 끄면(ClearFocus),
        // 선딜레이(Windup) 동안 플레이어가 조금만 움직여도 빗나감.
        // 따라서 포커스를 유지하여 공격 애니메이션 중에도 회전하게 함 (언리얼 기본 ACharacter 회전 설정 따름)
        
        /* 기존 코드: 회전 고정 및 트래킹 해제 (너무 정직함)
		FVector Direction = TargetActor->GetActorLocation() - AIChar->GetActorLocation();
		Direction.Z = 0.0f; // 높이 무시
		FRotator NewRot = FRotationMatrix::MakeFromX(Direction).Rotator();
		AIChar->SetActorRotation(NewRot);
		
		AIController->ClearFocus(EAIFocusPriority::Gameplay);
        */
        
        // [변경] 트래킹 유지 (SetFocus가 되어 있다면 계속 봄)
        AIController->SetFocus(TargetActor); 
	}

	// [공격 패턴 결정 로직]
	// 1. 필요한 데이터 및 몽타주 가져오기
	TArray<UAnimMontage*>& Montages = AIChar->EnemyAttackMontages;
	if (Montages.Num() == 0) return EBTNodeResult::Failed;

	// AI 성향 데이터 가져오기 (Controller에서)
	float ReactionTime = 0.3f;       // 기본값
	float HeavyPref = 0.5f;          // 기본값 (가드율 대응)
	
	if (AATSEnemyController* EnemyController = Cast<AATSEnemyController>(AIController))
	{
		ReactionTime = EnemyController->AI_ReactionTime;
		HeavyPref = EnemyController->AI_HeavyAttackPreference;
	}

	// 2. 공격 속도 보정 (ReactionTime이 빠를수록 공격도 빨라짐)
	// 예: 0.3초(보통) -> 1.0배
	//     0.15초(고수) -> 1.0 + (0.15 * 2.5) = 1.375배 (약 40% 빨라짐)
	float PlayRate = 1.0f + (0.3f - ReactionTime) * 2.5f; 
	PlayRate = FMath::Clamp(PlayRate, 1.0f, 1.5f); // 최대 1.5배속 제한

	// 3. 몽타주 인덱스 선택
	int32 SelectedIndex = 0;
    
    // 타겟 거리 계산
    float Distance = 0.0f;
    // [Fix] 변수명 충돌 방지 (TargetActor -> DistanceTarget)
    if (AActor* DistanceTarget = Cast<AActor>(Blackboard ? Blackboard->GetValueAsObject(TEXT("PlayerActor")) : nullptr))
    {
        Distance = AIChar->GetDistanceTo(DistanceTarget);
    }

	// [거리별 공격 패턴 분리]
    // User Request: 
    // - Index 0~2: 근접 콤보 (0: 1타, 1: 2타, 2: 3타 풀콤보)
    // - Index 3~4: 원거리 돌진/도약 공격
    
	// 1. [원거리] 돌진 공격 (Index 3, 4)
	if (Distance > 300.0f && Distance < 800.0f)
	{
        // 몽타주가 충분히 있어야 돌진 공격 가능
        if (Montages.Num() > 3)
        {
            int32 DashStartIndex = 3;
            int32 DashEndIndex = Montages.Num() - 1; 

            // 거리가 멀면(450 이상) Index 4(긴 돌진) 확률 높임
            if (Distance > 450.0f && Montages.IsValidIndex(4))
            {
                 SelectedIndex = (FMath::RandRange(0, 100) < 70) ? 4 : 3;
            }
            else
            {
                 // 300~450 사이면 3번 (짧은 돌진) 위주
                 if (DashStartIndex > DashEndIndex) DashStartIndex = DashEndIndex;
                 SelectedIndex = FMath::RandRange(DashStartIndex, DashEndIndex);
            }
            UE_LOG(LogTemp, Warning, TEXT("[AI_Learning] Ranged Attack (Dash) - Dist: %.0f, Index: %d"), Distance, SelectedIndex);
        }
        else
        {
            // 돌진 몽타주 없으면 접근 유도 (공격 실패)
            return EBTNodeResult::Failed;
        }
	}
    // 2. [초장거리] 800 이상이면 공격 안 함 (접근 유도)
    else if (Distance >= 800.0f)
    {
        return EBTNodeResult::Failed;
    }
    // 3. [근거리] 기본 콤보 공격 (Index 0 ~ 2)
	else
	{
        // HeavyPref (가드 파괴 필요성)에 따라 콤보 길이 결정
        // - 가드 잘함 (>0.6): 3타 풀콤보(Index 2)로 스태미나 압박
        // - 공격적임 (<0.4): 1타 견제(Index 0)로 치고 빠지기
        
        float RandVal = FMath::FRand();
        
        if (HeavyPref > 0.6f) 
        {
            // [가드 파괴/압박] 3타 콤보(Index 2) 우선
            if (RandVal < 0.7f && Montages.IsValidIndex(2)) SelectedIndex = 2; // 70% 확률로 3타
            else if (Montages.IsValidIndex(1)) SelectedIndex = 1;
            else SelectedIndex = 0;
            UE_LOG(LogTemp, Warning, TEXT("[AI_Learning] Close Range Pressure (Guard Rate High) -> Use 3-Hit Combo (Index 2)"));
        }
        else if (HeavyPref < 0.4f)
        {
            // [스피드/견제] 1타(Index 0) 우선
            if (RandVal < 0.7f) SelectedIndex = 0; // 70% 확률로 1타
            else if (Montages.IsValidIndex(1)) SelectedIndex = 1; // 가끔 2타
            else if (Montages.IsValidIndex(2)) SelectedIndex = 2;
            UE_LOG(LogTemp, Warning, TEXT("[AI_Learning] Close Range Poke (Aggressive Player) -> Use 1-Hit (Index 0)"));
        }
        else
        {
            // [밸런스] 랜덤 섞기
            int32 MaxIndex = FMath::Min(2, Montages.Num() - 1);
            SelectedIndex = FMath::RandRange(0, MaxIndex);
        }
	}

    // [최종 실행] 유효성 검사 후 실행
    bool bPlayedSuccessfully = false;
    if (Montages.IsValidIndex(SelectedIndex))
    {
        UAnimMontage* AttackMontage = Montages[SelectedIndex];
        if (AttackMontage)
        {
            AIChar->SetCharacterState(ETSCharacterState::Attacking);
            float Duration = AIChar->PlayAnimMontage(AttackMontage, PlayRate);
            bPlayedSuccessfully = (Duration > 0.0f);
            
            // [학습/로직] 공격 성공 시 마지막 공격 시간 기록
            if (bPlayedSuccessfully && Blackboard)
            {
                Blackboard->SetValueAsFloat(TEXT("LastAttackTime"), AIChar->GetWorld()->GetTimeSeconds());
            }
            
            UE_LOG(LogTemp, Warning, TEXT("[AI_Learning] Attack Executed: %s (Index: %d, Rate: %.2f)"), *AttackMontage->GetName(), SelectedIndex, PlayRate);
        }
    }
    else
    {
        // 예외: 인덱스가 없으면 기본 0번 실행
        if (Montages.Num() > 0)
        {
             AIChar->SetCharacterState(ETSCharacterState::Attacking);
             float Duration = AIChar->PlayAnimMontage(Montages[0], PlayRate);
             bPlayedSuccessfully = (Duration > 0.0f);
        }
    }

    if (bPlayedSuccessfully)
    {
        return EBTNodeResult::InProgress; // 몽타주가 끝날 때까지 TickTask에서 대기
    }
    else
    {
        return EBTNodeResult::Failed;
    }
}

void UBTTask_ExecuteAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    ATSCharacter* AIChar = Cast<ATSCharacter>(AIController ? AIController->GetPawn() : nullptr);
    if (AIChar)
    {
        // 공격 상태가 아니게 되면 (몽타주 종료 또는 인터럽트) 태스크 종료
        // TSCharacter::OnAttackMontageEnded에서 State를 Idle로 변경해주므로 그것을 감지
        if (AIChar->GetCharacterState() != ETSCharacterState::Attacking)
        {
            FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
            // UE_LOG(LogTemp, Warning, TEXT("[ExecuteAttack] Attack Finished"));
        }
    }
    else
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
    }
}
