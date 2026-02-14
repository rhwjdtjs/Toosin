#include "Toosin/Public/Game/ATSArenaGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/Character.h"
#include "Toosin/Public/Character/TSCharacter.h"
#include "Components/CapsuleComponent.h"
#include "AIController.h"
#include "GameFramework/PlayerController.h"
#include "Toosin/Public/AI/ATSEnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Toosin/Public/AI/TSAILearningSaveGame.h" // 추가
#include "Toosin/Public/AI/TSPlayerPatternComponent.h" // 추가
#include "EngineUtils.h" // TActorIterator용

ATSArenaGameMode::ATSArenaGameMode()
{
	// Default Pawn Class 설정 (필요 시)
	// DefaultPawnClass = ATSCharacter::StaticClass(); 
}

void ATSArenaGameMode::StartPlay()
{
	Super::StartPlay();

	// 게임 시작 시 라운드 바로 시작
	StartRound();
}

void ATSArenaGameMode::StartRound()
{
	if (bIsRoundActive) return;

	bIsRoundActive = true;
	UE_LOG(LogTemp, Warning, TEXT("[Arena] Round Started!"));

	// [락온 설정] 플레이어와 적을 찾아서 서로(혹은 플레이어만) 락온
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)
	{
		ATSCharacter* PlayerChar = Cast<ATSCharacter>(PC->GetPawn());
		
	// [적 AI 스폰/리셋]
	TArray<AActor*> Enemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATSCharacter::StaticClass(), Enemies);

    // 플레이어 제외한 캐릭터(=적) 찾기
    AActor* EnemyActor = nullptr;
    for (AActor* Actor : Enemies)
    {
        // 소멸 예정이거나, 죽은 상태인 적은 무시 (새로 스폰하기 위해)
        if (Actor != PlayerChar)
        {
            if (Actor->IsActorBeingDestroyed()) continue;
            
            // 만약 죽어있는 상태라면? -> 그냥 Destroy 시키고 새로 뽑거나, 아니면 여기서 부활시킬 수도 있음.
            // 유저 요청: "죽으면 스폰 안됨" -> 기존 액터가 사라져서 그런듯.
            // 여기서 살아있는 적만 EnemyActor로 취급.
            if (ATSCharacter* TSChar = Cast<ATSCharacter>(Actor))
            {
               if (TSChar->GetCharacterState() == ETSCharacterState::Dead) 
               {
                   TSChar->Destroy(); // 확실하게 제거
                   continue;
               }
            }
            
            EnemyActor = Actor;
            break;
        }
    }

    // 적이 없으면 스폰
    if (!EnemyActor && EnemyClass)
    {
        // 스폰 위치 찾기
        FVector SpawnLoc = FVector(1000.f, 0.f, 100.f); 
        FRotator SpawnRot = FRotator(0.f, 180.f, 0.f);
        TArray<AActor*> SpawnPoints;
        UGameplayStatics::GetAllActorsWithTag(GetWorld(), EnemySpawnTag, SpawnPoints);
        if (SpawnPoints.Num() > 0)
        {
            SpawnLoc = SpawnPoints[0]->GetActorLocation() + FVector(0, 0, 110.0f); // [수정] 높이 보정 110.0f로 통일
            SpawnRot = SpawnPoints[0]->GetActorRotation();
        }

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        EnemyActor = GetWorld()->SpawnActor<APawn>(EnemyClass, SpawnLoc, SpawnRot, SpawnParams);
        
         // [중요] 스폰 직후 Controller가 없을 수 있으므로 SpawnDefaultController 호출
        if (APawn* EnemyPawn = Cast<APawn>(EnemyActor))
        {
            EnemyPawn->SpawnDefaultController();
        }
        
        UE_LOG(LogTemp, Warning, TEXT("[Arena] Spawned new Enemy: %s"), *EnemyActor->GetName());
    }

    // 적 상태 초기화
    if (ATSCharacter* EnemyChar = Cast<ATSCharacter>(EnemyActor))
    {
        EnemyChar->ResetStats();
        EnemyChar->SetCharacterState(ETSCharacterState::Idle);

        // [중요] 콜리전 강제 설정 (스폰 직후 꺼짐 방지)
        EnemyChar->GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));
        EnemyChar->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        
        // [카메라 충돌 방지] 프로필 설정 후 다시 무시하도록 설정 (확대/겹침 방지)
        EnemyChar->GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
        EnemyChar->GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

        // AI 타겟 설정 (플레이어)
        if (AATSEnemyController* AIC = Cast<AATSEnemyController>(EnemyChar->GetController()))
        {
             AIC->GetBlackboardComponent()->ClearValue(TEXT("TargetActor")); 
             // 쿨타임 초기화
             AIC->GetBlackboardComponent()->SetValueAsFloat(TEXT("LastAttackTime"), GetWorld()->GetTimeSeconds());
        }
    }
    
    // 플레이어 락온 및 카메라 리셋
    if (PlayerChar)
    {
        if (EnemyActor) PlayerChar->SetLockOnTarget(EnemyActor);

        // [카메라 리셋] 플레이어 컨트롤러의 회전도 스폰 포인트 방향으로 강제 설정
        // 이걸 해야 게임 처음 시작할 때처럼 정면을 바라봄
        PC->SetControlRotation(PlayerChar->GetActorRotation());
        
        // [AI 학습] 플레이어 데이터 리셋 (새 라운드 수집 시작)
        if (PlayerChar->GetPlayerPatternComponent())
        {
            PlayerChar->GetPlayerPatternComponent()->ResetRoundData();
        }
    }
    
	// [AI 학습] 저장된 데이터 로드 및 AI에게 전달
    if (UTSAILearningSaveGame* LoadGameInstance = Cast<UTSAILearningSaveGame>(UGameplayStatics::LoadGameFromSlot(UTSAILearningSaveGame::SaveSlotName, UTSAILearningSaveGame::UserIndex)))
    {
        // 월드에 있는 적 컨트롤러 찾기
        for (TActorIterator<AATSEnemyController> It(GetWorld()); It; ++It)
        {
            AATSEnemyController* AIController = *It;
            if (AIController)
            {
                // AIController에 데이터 전달
                UE_LOG(LogTemp, Warning, TEXT("[AI_Learning] AI에게 학습 데이터 전달 (Rounds: %d)"), LoadGameInstance->TotalRoundsPlayed);
                AIController->UpdatePlayerData(LoadGameInstance->AccumulatedData);
            }
        }
    }
	}
}

void ATSArenaGameMode::EndRound(AActor* Winner)
{
	if (!bIsRoundActive) return;

	bIsRoundActive = false;
	FString WinnerName = Winner ? Winner->GetName() : TEXT("None");
	UE_LOG(LogTemp, Warning, TEXT("[Arena] Round Ended! Winner: %s"), *WinnerName);

    // [AI 학습] 데이터 저장
    // 1. 플레이어 컴포넌트에서 이번 라운드 데이터 가져오기
    AController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PlayerController)
    {
        ATSCharacter* PlayerChar = Cast<ATSCharacter>(PlayerController->GetPawn());
        if (PlayerChar && PlayerChar->GetPlayerPatternComponent())
        {
            FPlayerPatternData RoundData = PlayerChar->GetPlayerPatternComponent()->GetCurrentRoundData();
            
            // 2. SaveGame 로드 또는 생성
            UTSAILearningSaveGame* SaveGameInstance = Cast<UTSAILearningSaveGame>(UGameplayStatics::LoadGameFromSlot(UTSAILearningSaveGame::SaveSlotName, UTSAILearningSaveGame::UserIndex));
            if (!SaveGameInstance)
            {
                SaveGameInstance = Cast<UTSAILearningSaveGame>(UGameplayStatics::CreateSaveGameObject(UTSAILearningSaveGame::StaticClass()));
            }
            
            // 3. 누적 및 저장
            SaveGameInstance->AccumulatedData += RoundData;
            SaveGameInstance->TotalRoundsPlayed++;
             // 분석 결과 저장 (선택 사항, 저장 시점에 계산해서 넣어도 됨)
            // SaveGameInstance->AI_Aggressiveness = ... 
            
            UGameplayStatics::SaveGameToSlot(SaveGameInstance, UTSAILearningSaveGame::SaveSlotName, UTSAILearningSaveGame::UserIndex);
            UE_LOG(LogTemp, Warning, TEXT("[AI_Learning] 학습 데이터 저장 완료 (Total Rounds: %d)"), SaveGameInstance->TotalRoundsPlayed);
        }
    }

	// 일정 시간 후 라운드 리셋
	GetWorldTimerManager().SetTimer(RoundTimerHandle, this, &ATSArenaGameMode::ResetRound, RoundEndDelay, false);
}

void ATSArenaGameMode::ResetRound()
{
	UE_LOG(LogTemp, Warning, TEXT("[Arena] Resetting Round..."));

	// 1. 플레이어와 적을 스폰 위치로 이동 및 상태 초기화
	TArray<AActor*> ActorsToReset;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATSCharacter::StaticClass(), ActorsToReset);

	for (AActor* Actor : ActorsToReset)
	{
		ATSCharacter* Character = Cast<ATSCharacter>(Actor);
		if (!Character || Character->IsActorBeingDestroyed()) continue; // [수정] 소멸 중인 액터 무시

		// 상태 초기화 (체력, 스태미나 등)
		Character->ResetStats(); 
		Character->SetCharacterState(ETSCharacterState::Idle);
		
		// 죽은 경우 물리 시뮬레이션 끄고 캡슐 콜리전 켜기
		Character->GetMesh()->SetSimulatePhysics(false);
        
        // [중요] 콜리전 프로필 복구 (Pawn)
        Character->GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));
		Character->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        
        // [카메라 충돌 방지] 프로필 변경 후 다시 적용
        Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
        Character->GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

		Character->GetMesh()->AttachToComponent(Character->GetCapsuleComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
		Character->GetMesh()->SetRelativeLocation(FVector(0,0,-90)); 
		Character->GetMesh()->SetRelativeRotation(FRotator(0,-90,0)); 

		// 위치 리셋
		FName SpawnTag = Character->IsPlayerControlled() ? PlayerSpawnTag : EnemySpawnTag;
		
		TArray<AActor*> SpawnPoints;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), SpawnTag, SpawnPoints);
		
		if (SpawnPoints.Num() > 0)
		{
			AActor* SpawnPoint = SpawnPoints[0];
            
            // [위치 보정] 바닥에 끼는 문제 해결을 위해 Z축을 90.0f 올림 (캡슐 반높이 고려)
			FVector NewLoc = SpawnPoint->GetActorLocation() + FVector(0, 0, 90.0f);
			Character->SetActorLocationAndRotation(NewLoc, SpawnPoint->GetActorRotation(), false, nullptr, ETeleportType::TeleportPhysics);
		}

        // [AI 초기 공격 방지] 쿨타임 강제 적용
        if (AATSEnemyController* AIC = Cast<AATSEnemyController>(Character->GetController()))
        {
            if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
            {
                BB->SetValueAsFloat(TEXT("LastAttackTime"), GetWorld()->GetTimeSeconds());
            }
        }
	}

	// 다시 라운드 시작
	StartRound();
}
