# Native Server build diagnosis — October 11, 2026

The original Linux Development build failed in two game unity translation units.
The completed engine objects reduced an exact incremental rerun to 4.84 seconds;
it reproduced the same namespace-shadow, helper-redefinition and absent-editor-hook
errors. Captured evidence: `linux-server-build-unity-failure.log` and
`linux-server-repro.log`.

Ranked causes checked against the diagnostics:

1. Unity compilation combines otherwise file-local anonymous namespace helpers.
   The failing generated unit includes the menu colors and separate HUD local
   names, and both tests defining `GameWorld()`.
2. Encounter tests are gated by development automation but observe a sentry event
   declared under `WITH_EDITOR`. The non-editor Server target lacks that member.
3. An incorrect SDK or engine dependency could create platform failures. The
   verified v26/clang 20.1.8 toolchain and successful engine compilation do not
   support this cause.

The repair disables unity only for the game's Server target, preserving Windows
Game/Editor build settings. Encounter tests with editor event hooks now require
both development automation and editor support. No gameplay body, replicated
field, RPC, input mapping or protocol version changed.

The actual compiler invocation is the regression seam; a runtime unit test cannot
detect translation-unit name collisions or a missing compile-time editor member.
After the repair, all separate game sources compiled. The accelerator exhausted
Windows commit memory at the final step; the resumed `-NoUBA` actions linked the
server and wrote its receipt successfully. Build result: **Succeeded**, native ELF
binary 273,974,200 bytes. `linux-server-build-direct.log` captures that final pass.
No temporary game instrumentation was added.

The earlier Windows evidence retains its date and scope. Two Source input hashes
now differ from VS57: the Server-only build rule and editor-only test guard. Saved
passes must not be advertised as exact-current-input reruns. Native packaging and
real client joins are separate acceptance steps.
