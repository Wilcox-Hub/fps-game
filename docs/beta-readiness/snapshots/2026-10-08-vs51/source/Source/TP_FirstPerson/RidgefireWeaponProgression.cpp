#include "RidgefireWeaponProgression.h"

namespace RidgefireWeaponProgression
{
int32 Price(int32 Offer) { const int32 Prices[]={300,600,0,650,800};return Offer>=0 && Offer<5 ? Prices[Offer] : MAX_int32; }
int32 UpgradePrice(int32 Level) { return Level>=0 && Level<3 ? 450+350*Level : MAX_int32; }
FRidgefireWeaponRoll Upgrade(FRidgefireWeaponRoll W,int32 Behavior)
{
    if(W.UpgradeLevel>=3 || Behavior<0 || Behavior>1 || (W.UpgradeLevel>0 && static_cast<int32>(W.UpgradeBehavior)!=Behavior+1)) return W;
    W.DamageScale=FMath::Clamp(W.DamageScale*1.5f,0.05f,40.0f); W.RefireDelay=FMath::Max(0.055f,W.RefireDelay*0.92f);
    W.MagazineSize=FMath::Min(100,W.MagazineSize+FMath::Max(1,W.MagazineSize/4));
    ++W.UpgradeLevel;W.UpgradeBehavior=static_cast<ERidgefireUpgradeBehavior>(Behavior+1);
    return W;
}
float ReloadSeconds(const FRidgefireWeaponRoll& W) { return 1.05f; }
float SpreadDegrees(const FRidgefireWeaponRoll& W)
{
 return W.Mode==ERidgefireFireMode::Scatter ? 8.0f : W.Mode==ERidgefireFireMode::Roulette ? 15.0f : W.Mode==ERidgefireFireMode::Auto ? 0.30f : W.Mode==ERidgefireFireMode::Burst ? 0.20f : 0.12f;
}
float RecoilDegrees(const FRidgefireWeaponRoll& W)
{
 const float Family=W.Mode==ERidgefireFireMode::Phasma ? 0.85f : W.Mode==ERidgefireFireMode::Pierce ? 0.70f
  : W.Mode==ERidgefireFireMode::Mortar ? 0.95f : W.Mode==ERidgefireFireMode::Scatter ? 0.65f : W.Mode==ERidgefireFireMode::Burst ? 0.38f : 0.28f;
 const float Rate=FMath::Clamp(0.16f/FMath::Max(0.055f,W.RefireDelay),1.0f,1.9f);
 const float Power=FMath::Clamp(FMath::Sqrt(FMath::Max(0.05f,W.DamageScale)),0.9f,1.5f);
 return FMath::Min(1.8f,Family*Rate*Power);
}
const TCHAR* RecoilLabel(float D) { return D<=0.20f ? TEXT("Low") : D<=0.35f ? TEXT("Medium") : D<=0.60f ? TEXT("High") : TEXT("Very High"); }
float ConditionMultiplier(const FRidgefireWeaponRoll& W,float Distance,bool bSprint,float HP,int32 Ammo,int32 Streak,bool bReload)
{
    switch(W.Condition) {
    case ERidgefireCondition::Close:return Distance<240 ? 4.0f : 0.35f;
    case ERidgefireCondition::Longshot:return Distance>700 ? 3.5f : 0.45f;
    case ERidgefireCondition::Sprint:return bSprint ? 2.8f : 0.5f;
    case ERidgefireCondition::LowHealth:return HP<0.28f ? 4.5f : 0.6f;
    case ERidgefireCondition::FullHealth:return HP>0.85f ? 3.0f : 0.4f;
    case ERidgefireCondition::LastShot:return Ammo==0 ? 7.0f : 0.65f;
    case ERidgefireCondition::Streak:return Streak>=3 ? 3.0f : 0.65f;
    case ERidgefireCondition::AfterReload:return bReload ? 5.0f : 0.55f;
    case ERidgefireCondition::Desperate:return Ammo<=2 ? 4.0f : 0.55f;
    default:return 1;
    }
}
static float ReferenceDamage(const FRidgefireWeaponRoll& W,int32 Ammo,bool bReload,float Outgoing)
{
    float Mode=1;
    if(W.Mode==ERidgefireFireMode::Scatter) Mode=FMath::Sqrt(static_cast<float>(W.Pellets));
    if(W.Mode==ERidgefireFireMode::Burst) Mode=2.16f;
    if(W.Mode==ERidgefireFireMode::Echo) Mode=1.72f;
    if(W.Mode==ERidgefireFireMode::Roulette) Mode=(0.1f+0.4f+1+2+5+12)/6;
    float Payload=W.Payload==ERidgefirePayload::Burn ? 1.28f : W.Payload==ERidgefirePayload::Leaden ? 0.38f : W.Payload==ERidgefirePayload::Glass ? (0.12f+0.3f+0.65f+1.4f+3+8)/6 : 1;
    float Direct=42*W.DamageScale*Mode*Payload*ConditionMultiplier(W,750,false,1,Ammo,0,bReload)*Outgoing;
    if(W.UpgradeBehavior==ERidgefireUpgradeBehavior::EchoPulse) Direct*=1.35f;
    // Arsenal damage is per direct impact; splash is reduced, distance-scaled, and can hit multiple targets.
    return Direct+(W.Mode==ERidgefireFireMode::Mortar ? 95*W.DamageScale : 0);
}
FRidgefireWeaponStats Stats(const FRidgefireWeaponRoll& W,float Outgoing,float ReloadModifier,float RecoilModifier)
{
    FRidgefireWeaponStats S;const float Delay=FMath::Max(0.055f,W.RefireDelay);const int32 Mag=FMath::Clamp(W.MagazineSize,1,100);
    S.Damage=ReferenceDamage(W,Mag-1,true,Outgoing);S.RPM=60/Delay;S.Reload=ReloadSeconds(W)*ReloadModifier;
    S.ConeDegrees=SpreadDegrees(W);S.RecoilDegrees=RecoilDegrees(W)*RecoilModifier;
    float MagazineDamage=0;for(int32 Shot=0;Shot<Mag;++Shot) MagazineDamage+=ReferenceDamage(W,Mag-1-Shot,Shot==0,Outgoing);
    S.BurstDPS=MagazineDamage/(Mag*Delay);
    S.SustainedDPS=MagazineDamage/((Mag-1)*Delay+FMath::Max(Delay,S.Reload));
    return S;
}
TArray<FString> EnemyComparison(const FRidgefireWeaponRoll& W,const FRidgefireEncounterPlan& P,float Outgoing)
{
    TArray<FString> Rows;
    const auto Solo=FRidgefireEncounterPlan::Build(1,0,P.Encounter,1);
    auto Shots=[&W,Outgoing](float HP,float Guard=1.0f) {
        const int32 Mag=FMath::Clamp(W.MagazineSize,1,100);
        double PerMagazine=0;for(int32 Shot=0;Shot<Mag;++Shot) PerMagazine+=ReferenceDamage(W,Mag-1-Shot,Shot==0,Outgoing)*Guard;
        if(!FMath::IsFinite(PerMagazine) || PerMagazine<=0) return MAX_int32;
        const int32 Cycles=FMath::Max(0,FMath::CeilToInt(HP/PerMagazine)-1);
        double Remaining=HP-Cycles*PerMagazine;int32 Count=Cycles*Mag;
        for(int32 Shot=0;Shot<Mag && Remaining>0.00001;++Shot) {Remaining-=ReferenceDamage(W,Mag-1-Shot,Shot==0,Outgoing)*Guard;++Count;}
        return Count;
    };
    const float Standard=(120+P.Threat*12)*P.HealthScale,Armored=(260+P.Threat*25)*P.HealthScale;
    Rows.Add(FString::Printf(TEXT("Standard: %d shots / solo %d / HP %.0f (party %d)"),Shots(Standard),Shots((120+P.Threat*12)*Solo.HealthScale),Standard,P.Players));
    Rows.Add(FString::Printf(TEXT("Armored brute: %d / solo %d / HP %.0f; armor is extra HP"),Shots(Armored),Shots((260+P.Threat*25)*Solo.HealthScale),Armored));
    Rows.Add(FString::Printf(TEXT("Champion: %d / solo %d; guarded %d (75%% reduction)"),Shots(P.ChampionHealth),Shots(Solo.ChampionHealth),Shots(P.ChampionHealth,0.25f)));
    return Rows;
}
}
