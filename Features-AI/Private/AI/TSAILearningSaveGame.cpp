#include "Toosin/Public/AI/TSAILearningSaveGame.h"

const FString UTSAILearningSaveGame::SaveSlotName = TEXT("TS_AILearningData");
const int32 UTSAILearningSaveGame::UserIndex = 0;

UTSAILearningSaveGame::UTSAILearningSaveGame()
{
	TotalRoundsPlayed = 0;
    
    // 기본 AI 성향 초기화
    AI_Aggressiveness = 0.5f;
    AI_ReactionTime = 0.3f; // 0.3초 반응
    AI_GuardProbability = 0.5f; // 50% 가드율
}
