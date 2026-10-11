#include "RidgefireGameMode.h"
#include "RidgefireWeaponProgression.h"
#include "RidgefireProgression.h"
#include "Variant_Shooter/AI/ShooterNPC.h"
#include "Variant_Shooter/ShooterPlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "TimerManager.h"

int32 ARidgefireGameMode::SpawnWaveEnemies(int32 Count,int32 WaveNumber,int32& Turrets,int32& Sprinters,int32& Brutes)
{
    int32 Spawned=0;
    for(int32 Index=0;Index<Count;++Index) {
        const int32 Ordinal=bLegacySetupFixture ? Index : EncounterSpawned;
        FTransform Transform=SentinelEntrances[Ordinal%SentinelEntrances.Num()];
        Transform.AddToTranslation(FVector(FMath::RandRange(-130.0f,130.0f),FMath::RandRange(-130.0f,130.0f),30));
        FActorSpawnParameters Parameters;Parameters.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        auto* Enemy=GetWorld()->SpawnActor<AShooterNPC>(SentinelClass,Transform,Parameters);
        if(!Enemy) {UE_LOG(LogTemp,Warning,TEXT("RIDGEFIRE ENCOUNTER spawn failed wave=%d ordinal=%d"),WaveNumber,Ordinal);continue;}
        const int32 Threat=bLegacySetupFixture ? WaveNumber : EncounterPlan.Threat;
        const float Health=bLegacySetupFixture ? 1.0f : EncounterPlan.HealthScale;
        Enemy->CurrentHP=(120+Threat*12)*Health;Enemy->SetActorScale3D(FVector(1.35f));
        const bool bBoss=!bLegacySetupFixture && EncounterPlan.bChampionEncounter && Ordinal==0;
        const bool bSprinter=bLegacySetupFixture ? WaveNumber>=2 && Index==5 : EncounterPlan.Encounter>=2 && Ordinal%6==2;
        const bool bBrute=bLegacySetupFixture ? WaveNumber>=3 && Index==Count-1 : EncounterPlan.Encounter>=3 && Ordinal%9==8;
        if(bBoss) {
            Enemy->EnableRidgefireBrute();Enemy->CurrentHP=EncounterPlan.ChampionHealth;
            Enemy->EnableRidgefireChampion(!AreArenaRelaysSecured(),RunPlan.Visited.Contains(1));Champion=Enemy;++Brutes;
        } else if(!bLegacySetupFixture && bFoundryArenaActive && Ordinal%8==3) {
            Enemy->EnableFoundryTurret();++Turrets;
        } else if(bBrute) {
            Enemy->EnableRidgefireBrute();Enemy->CurrentHP=(260+Threat*25)*Health;++Brutes;
        } else if(bSprinter) {
            Enemy->EnableRidgefireSprinter();
            if(!bLegacySetupFixture) Enemy->CurrentHP=(55+Threat*4)*Health;
            Enemy->SetActorScale3D(FVector(1.05f));++Sprinters;
        }
        if(!bLegacySetupFixture) {
            Enemy->SetRidgefireDamageScale(EncounterPlan.DamageScale);
            Enemy->ConfigurePressureTactics(RunPlan.Seed,RunPlan.Current,EncounterPlan.Encounter,Ordinal,EncounterPlan.Players);
        }
        const float MinimumZ=ArenaGroundZ+Enemy->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()+5;
        if(Enemy->GetActorLocation().Z<MinimumZ) Enemy->SetActorLocation(FVector(Enemy->GetActorLocation().X,Enemy->GetActorLocation().Y,MinimumZ),false,nullptr,ETeleportType::TeleportPhysics);
        ActiveWaveSentinels.Add(Enemy);Enemy->ForceNetUpdate();Enemy->OnPawnDeath.AddDynamic(this,&ARidgefireGameMode::HandleSentinelDeath);
        ++EnemiesRemaining;++Spawned;if(!bLegacySetupFixture) ++EncounterSpawned;
    }
    return Spawned;
}
void ARidgefireGameMode::SpawnEncounterReinforcements()
{
    if(!HasAuthority() || bLegacySetupFixture || bRunOver || bArmoryOpen || bFrontEnd || bLobbyOpen || bRouteVoteOpen || PendingArena!=INDEX_NONE) {
        GetWorldTimerManager().ClearTimer(EncounterTimer);return;
    }
    if(PendingEncounterEnemies<=0) {if(EnemiesRemaining==0) FinishClearedWave();return;}
    const int32 Count=FMath::Min3(EncounterPlan.BatchSize,PendingEncounterEnemies,EncounterPlan.AliveLimit-EnemiesRemaining);
    if(Count<=0) {GetWorldTimerManager().SetTimer(EncounterTimer,this,&ARidgefireGameMode::SpawnEncounterReinforcements,2.0f,false);return;}
    int32 Turrets=0,Sprinters=0,Brutes=0;
    const int32 Spawned=SpawnWaveEnemies(Count,CurrentWave,Turrets,Sprinters,Brutes);
    PendingEncounterEnemies-=Spawned;
    ConsecutiveWaveSpawnFailures=Spawned>0 ? 0 : ConsecutiveWaveSpawnFailures+1;
    if(ConsecutiveWaveSpawnFailures>=3) {
        bRunOver=true;GetWorldTimerManager().ClearTimer(WaveTimer);GetWorldTimerManager().ClearTimer(EncounterTimer);
        UE_LOG(LogTemp,Error,TEXT("RIDGEFIRE ENCOUNTER reinforcement spawning failed; ending run for restart"));RefreshRidgefireRunStates();return;
    }
    UE_LOG(LogTemp,Display,TEXT("RIDGEFIRE ENCOUNTER wave=%d fight=%d/%d party=%d live=%d pending=%d hp_scale=%.3f damage_scale=%.3f"),CurrentWave,ArenaWave,EncounterPlan.Encounters,EncounterPlan.Players,EnemiesRemaining,PendingEncounterEnemies,EncounterPlan.HealthScale,EncounterPlan.DamageScale);
    if(PendingEncounterEnemies>0) GetWorldTimerManager().SetTimer(EncounterTimer,this,&ARidgefireGameMode::SpawnEncounterReinforcements,Spawned>0 ? EncounterPlan.BatchInterval : 2.0f,false);
    RefreshRidgefireRunStates();
}
void ARidgefireGameMode::FinishClearedWave()
{
    if(bRunOver || bWaveClearRewarded || EnemiesRemaining>0 || PendingEncounterEnemies>0 || CurrentWave<=0) return;
    bWaveClearRewarded=true;GetWorldTimerManager().ClearTimer(EncounterTimer);
    AwardWildcardRerollToken();
    if(!bLegacySetupFixture) AwardSharedCash(RidgefireWeaponProgression::EncounterCash);
    for(FConstPlayerControllerIterator It=GetWorld()->GetPlayerControllerIterator();It;++It)
        if(auto* Player=Cast<AShooterPlayerController>(It->Get())) Player->GrantPersonalXP(30);
    if(auto* Store=GetGameInstance()->GetSubsystem<URidgefireProgressionStore>()) Store->Flush();
    bRegroupOpen=!bLegacySetupFixture;RegroupReady.Reset();++RegroupEpoch;
    const float Delay=bLegacySetupFixture ? 5.0f : RidgefireWeaponProgression::RegroupSeconds;
    UE_LOG(LogTemp,Display,TEXT("RIDGEFIRE: Wave %d cleared. Regroup %.0f seconds; shared cash %d."),CurrentWave,Delay,bLegacySetupFixture ? 0 : RidgefireWeaponProgression::EncounterCash);
    GetWorldTimerManager().SetTimer(WaveTimer,this,&ARidgefireGameMode::BeginNextWave,Delay,false);
    if(bRegroupOpen) GetWorldTimerManager().SetTimer(RegroupStateTimer,this,&ARidgefireGameMode::RefreshRidgefireRunStates,1.0f,true);
    RefreshRidgefireRunStates();
}
