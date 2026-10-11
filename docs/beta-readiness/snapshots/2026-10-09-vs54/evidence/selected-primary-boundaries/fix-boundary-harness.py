from pathlib import Path
base=Path(__file__).resolve().parent;repo=Path('C:/Users/YaYa/Documents/Codex/2026-09-22/connect-this-github-repo-to-a')
rel='unreal/Ridgefire/Source/TP_FirstPerson/Tests/RidgefireCombatReliabilityTests.cpp'
s=(repo/rel).read_text(encoding='utf-8-sig')
a='        if (Mode->GetWave() >= FirstWave+(bShotClear ? 1 : 3))'
assert s.count(a)==1
s=s.replace(a,'        // Boundaries are exercised after real-time input/reload checks; compress only\n        // the controlled frozen-enemy encounter timers, including their warning lead.\n        if (!bShotClear && CombatStage>=3) UGameplayStatics::SetGlobalTimeDilation(World,8.f);\n'+a)
p=base/'source'/rel;p.parent.mkdir(parents=True,exist_ok=True);p.write_text(s,encoding='utf-8',newline='\n')
print('Existing boundary harness adapts to longer warned arrivals; real input timing retained')
