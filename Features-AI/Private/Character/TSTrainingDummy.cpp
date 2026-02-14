#include "Toosin/Public/Character/TSTrainingDummy.h"
#include "Toosin/Public/AI/ATSEnemyController.h" 

ATSTrainingDummy::ATSTrainingDummy()
{
 	// Tick 함수 실행 여부 설정
	PrimaryActorTick.bCanEverTick = true;

	// AI Controller Class 설정
	AIControllerClass = AATSEnemyController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// 스탯 커스텀 설정 (필요 시)
	// CharacterStatRowName = FName("Dummy"); 
}

void ATSTrainingDummy::BeginPlay()
{
	Super::BeginPlay();
	
	// ATSCharacter::BeginPlay()에서 무기 생성 및 스탯 초기화가 이미 처리됨.
	// 추가적인 초기화가 필요하면 여기에 작성.
}

void ATSTrainingDummy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
