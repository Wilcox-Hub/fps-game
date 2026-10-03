# Diagnostic and implementation corrections

The focused real owner-controller recoil API reproduced twice before the fix. The command was the supporting Invoke-RecoilCheck.ps1 with RunName recoil-baseline, then recoil-baseline-repeat. A positive 0.30 kick produced +0.30 degrees with legacy pitch scale +1 and -0.75 degrees with scale -2.5. The minimised test retains only an owned pawn/controller, pitch scale, physical kick and the normal engine rotation update. Ranked predictions considered inherited look-input scaling, visual-only weapon animation and a negative firing-call amount. The camera-direction test distinguished the first: physical recoil is now accumulated independently of input inversion, through the ordinary engine rotation update.

The first build waited on the owner's open Editor DLL; the owner closed it. A later compile required regenerating reflection headers because staged header timestamps predated generated headers; the guarded installer now updates changed-file timestamps. An incorrect new helper include was corrected. These failed attempts are not current build evidence.

The first network run caught EnableRidgefireSprinter resetting CurrentHP after initial scaling. Normal spawning now applies its role/progression HP multiplier after role activation. Legacy fixtures retain their old role values. Final role checks cover sprinters, brutes, turrets and ordinary sentries.

The opt-in restart fixture originally called host firing after Super::PlayerTick, before the outer controller tick cleared RotationInput. The client owner RPC raised aim, while that artificial host timing discarded its first kick. The fixture now schedules real DoStartFiring in the timer phase, allowing the next normal rotation update to consume it; camera direction is checked on both peers. No direct camera-setting bypass was added to production recoil.

The first Shipping attempt encountered a short-lived Unreal build mutex conflict during test startup. It is recorded as a failed attempt, not an SDK failure or a package. The complete package has a separate success log and startup/runtime hash.
