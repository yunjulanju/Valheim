// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/MonsterAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include <BehaviorTree/BehaviorTreeComponent.h>

AMonsterAIController::AMonsterAIController()
{
    // Perception Component 생성
    AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));

    // Sight Config 설정
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 500;          // 시야 범위
    SightConfig->LoseSightRadius = 600;       // 잃는 범위    
    SightConfig->PeripheralVisionAngleDegrees = 100; // 시야각
    SightConfig->SetMaxAge(2.f);                 // 기억 유지 시간
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

    // Perception에 Sight 등록
    AIPerception->ConfigureSense(*SightConfig);
    AIPerception->SetDominantSense(SightConfig->GetSenseImplementation());

    // 감지 콜백 바인딩
    AIPerception->OnTargetPerceptionUpdated.AddDynamic(this, &AMonsterAIController::OnTargetPerceptionUpdated);
}

void AMonsterAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
	
    RunBehaviorTree(BT);
}

void AMonsterAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{

}

void AMonsterAIController::StopAI()
{
    //StopTree(EBTStopMode::Safe);
    //CleanupBrainComponent();

    UBehaviorTreeComponent* BTC = Cast<UBehaviorTreeComponent>(GetBrainComponent());
    if (nullptr == BTC)
    {
        return;
    }
    UE_LOG(LogTemp, Warning, TEXT("StopTree"));
    BTC->StopTree(EBTStopMode::Safe);
}
