#include "Toosin/Public/AI/ATSEnemyController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Toosin/Public/Character/TSCharacter.h"
#include "Toosin/Public/Weapon/TSWeapon.h"

AATSEnemyController::AATSEnemyController()
{
	
}

void AATSEnemyController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (BehaviorTreeAsset && BlackboardAsset)
	{
		// Blackboard 초기화 (TObjectPtr 호환성 문제 해결을 위해 로컬 변수 사용)
		UBlackboardComponent* BlackboardComp = nullptr;
		if (UseBlackboard(BlackboardAsset, BlackboardComp))
		{
			// 필요한 경우 멤버 변수에 재할당 (UseBlackboard가 내부적으로 설정하므로 필수는 아닐 수 있음)
			Blackboard = BlackboardComp;
			// Behavior Tree 실행
			RunBehaviorTree(BehaviorTreeAsset);
			UE_LOG(LogTemp, Warning, TEXT("[AIController] BT Started for %s"), *InPawn->GetName());
			
            // [중요] BT 시작 시 LastAttackTime을 현재 시간으로 설정하여
            // 스폰 직후 바로 공격하는 것을 방지 (쿨타임 적용)
            if (Blackboard)
            {
                Blackboard->SetValueAsFloat(TEXT("LastAttackTime"), GetWorld()->GetTimeSeconds());
            }

	// 초기 플레이어 감지 시도
			DetectPlayer();
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[AIController] MISSING ASSETS! BehaviorTree: %s, Blackboard: %s"), 
			BehaviorTreeAsset ? TEXT("Valid") : TEXT("NULL"), 
			BlackboardAsset ? TEXT("Valid") : TEXT("NULL"));
	}
}

// [중요] 매 프레임 플레이어 상태 확인하여 Blackboard 업데이트
void AATSEnemyController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (Blackboard)
    {
        AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TEXT("PlayerActor")));
        if (ATSCharacter* PlayerChar = Cast<ATSCharacter>(TargetActor))
        {
            // [사망 체크] 플레이어가 죽으면 주시 해제 및 정지
            if (PlayerChar->GetCharacterState() == ETSCharacterState::Dead)
            {
                ClearFocus(EAIFocusPriority::Gameplay);
                StopMovement();
                // Blackboard->SetValueAsObject(TEXT("PlayerActor"), nullptr); // 필요 시 타겟 해제
                return;
            }

            // [개선] 단순히 State만 보는게 아니라, 실제 몽타주가 재생 중인지도 확인 (애니메이션 끝났는데 State만 남은 경우 방지)
            // 주의: 무기 콜리전만 체크하면 공격 선딜(Windup) 구간에서 가드를 풀어버리는 문제 발생 -> 다시 몽타주 재생 여부로 판단
            bool bIsMontage = PlayerChar->GetMesh()->GetAnimInstance()->IsAnyMontagePlaying();
            bool bIsAttacking = (PlayerChar->GetCharacterState() == ETSCharacterState::Attacking) && bIsMontage;
            
            // [거리 체크 & 공격성 추가] 
            // 플레이어가 공격 중이라도, 거리가 멀면(350cm 이상) 위협이 되지 않으므로 가드하지 않음
            // (이전의 20% 랜덤 공격 무시 코드는 삭제함: 불확실성이 크고 AI가 가끔 멍청해 보이는 원인이 됨)
            // 대신 거리 기반으로 확실하게 350cm 이상이면 가드를 풀고 접근하거나 돌진 공격을 하도록 유도함.
            // Pawn이 없으면 거리를 무한대로 설정하여 오작동(거리 0으로 인식해 가드하는 버그) 방지
            float DistanceToPlayer = GetPawn() ? GetPawn()->GetDistanceTo(PlayerChar) : FLT_MAX;
            
            // [거리 체크 - 히스테리시스 적용] 
            // 가드 떨림(Flickering) 방지: 경계선(350)에서 넉백 등으로 왔다갔다 할 때 가드 On/Off 반복되는 현상 해결
            // 1. 위협 상태면(True) -> 확실히 멀어져야(> 450) 무시
            // 2. 무시 상태면(False) -> 확실히 가까워져야(< 300) 다시 위협으로 간주
            
            // 기존 Blackboard 값 참조
            bool bPrevAttacking = Blackboard->GetValueAsBool(TEXT("bPlayerAttacking"));
            bool bDistanceCondition = false;
            
            if (bPrevAttacking) // 현재 가드 중(위협 느낌) -> 관성 유지 (잘 안 풂)
            {
                 // 이미 막고 있으면 450까지는 계속 막음
                 if (DistanceToPlayer <= 450.0f) bDistanceCondition = true;
            }
            else // 현재 무시 중 -> 관성 유지 (잘 안 켬)
            {
                 // 안 막고 있으면 300 안으로 들어와야 막음
                 if (DistanceToPlayer <= 300.0f) bDistanceCondition = true;
            }
            
            // 거리 조건 불만족 시 위협 무시
            if (!bDistanceCondition)
            {
                bIsAttacking = false;
            }
            
            // Blackboard 값 업데이트 (Behavior Tree의 Service/Decorator가 이걸 감지해야 함)
            // bool bPrevAttacking = Blackboard->GetValueAsBool(TEXT("bPlayerAttacking")); // 위에서 이미 선언됨
            if (bIsAttacking != bPrevAttacking)
            {
                Blackboard->SetValueAsBool(TEXT("bPlayerAttacking"), bIsAttacking);
                // UE_LOG(LogTemp, Log, TEXT("[AIController] Player Attacking State Changed: %s"), bIsAttacking ? TEXT("TRUE") : TEXT("FALSE"));
            }
        } // End of PlayerChar check
        
    }
}

void AATSEnemyController::DetectPlayer()
{
	// 간단하게 플레이어 0번을 타겟으로 설정 (1v1 아레나 가정)
	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (PlayerCharacter && Blackboard)
	{
		Blackboard->SetValueAsObject(TEXT("PlayerActor"), PlayerCharacter);
        
        // [주시 설정] 항상 플레이어를 바라보도록 설정 (몸을 돌리지 않음)
        SetFocus(PlayerCharacter);
        
		UE_LOG(LogTemp, Warning, TEXT("[AIController] Player Detected: %s"), *PlayerCharacter->GetName());
	}
}

void AATSEnemyController::UpdatePlayerData(const FPlayerPatternData& NewData)
{
    PlayerData = NewData;
    
    // [데이터 분석 및 가중치 조정]
    
    // 1. 플레이어의 공격성 (Aggressiveness)
    // - 공격 횟수가 많고 거리가 가까울수록 공격적
    float PlayerAggro = PlayerData.GetAggressiveness();
    
    // -> AI 대응: 플레이어가 공격적이면 반격 위주(ReactionTime 감소), 수비적이면 압박(Aggressiveness 증가)
    if (PlayerAggro > 0.7f) {
        AI_ReactionTime = 0.15f; // 빠르게 반응 (초고수 모드)
        AI_GuardProbability = 0.8f; // 잘 막음
        UE_LOG(LogTemp, Warning, TEXT("[AI_Learning] Player is Aggressive! -> AI Reacts Faster"));
    } else {
        AI_ReactionTime = 0.3f; // 보통
        AI_GuardProbability = 0.5f;
    }
    
    // 2. 플레이어의 가드 실력 (Guard Rate)
    // - 가드 성공 횟수가 많으면 강공격(Heavy) 비중을 높여서 가드를 뚫거나 흔듦
    int32 TotalDefense = PlayerData.GuardSuccessCount + PlayerData.ParrySuccessCount + PlayerData.HitCount;
    float GuardRate = (TotalDefense > 0) ? (float)(PlayerData.GuardSuccessCount + PlayerData.ParrySuccessCount) / (float)TotalDefense : 0.0f;
    
    if (GuardRate > 0.6f) {
        AI_HeavyAttackPreference = 0.8f; // 강공격 위주
        UE_LOG(LogTemp, Warning, TEXT("[AI_Learning] Player Guards Well! -> Use Heavy Attacks"));
    } else {
        AI_HeavyAttackPreference = 0.3f; // 경공격 위주 (콤보)
    }
    
    // 3. 거리 조절 (Spacing)
    // "왜 계속 붙기만 해?" -> AI가 일정 거리를 유지하거나 왔다갔다 하도록 변경
    // 너무 가까우면(200 이하) 부담스러우므로, 기본 거리를 300~450 사이에서 유동적으로 가져감
    
    // 공격적(Aggressive)일수록 더 붙음 (250), 수비적이면 거리를 둠 (450)
    float BaseRange = (PlayerAggro > 0.6f) ? 300.0f : 400.0f; 
    
    // 랜덤 변화 (숨쉬기)
    float RandomOffset = FMath::RandRange(-50.0f, 50.0f);
    float OptimalRange = BaseRange + RandomOffset;

    // [Blackboard 업데이트]
    if (Blackboard)
    {
        Blackboard->SetValueAsFloat(TEXT("OptimalAttackRange"), OptimalRange); // 이동 목표 거리 (이 값 - 50 까지 접근)
        
        Blackboard->SetValueAsFloat(TEXT("AI_ReactionTime"), AI_ReactionTime);
        Blackboard->SetValueAsFloat(TEXT("AI_GuardProbability"), AI_GuardProbability);
        Blackboard->SetValueAsFloat(TEXT("AI_HeavyAttackPreference"), AI_HeavyAttackPreference);
    }
}
