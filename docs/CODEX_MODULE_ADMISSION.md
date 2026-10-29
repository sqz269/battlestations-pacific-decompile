# Existing module compilation admission

Four existing C++ modules were absent from the root and included CMake
source lists and generated Win32 projects. They are now registered directly
in `bsp_core`, using the same root registry as the recent Dyn packets.
`cmake/startup.cmake` remains under the other coordinator's ownership; the
obsolete queue for editing it was withdrawn. No source/header body, original
installation, provider, host call, switch or reconstruction ledger is changed.

| Existing source | Historical rows / game-scope rows in baseline census |
|---|---|
| `src/sim_damage_seg54_argon.cpp` | 40 / 23 |
| `src/powerup_seg58_argon.cpp` | 60 / 48 |
| `src/warning_string_seg64_argon.cpp` | 47 / 8 |
| `src/hardware_options_seg66_argon.cpp` | 70 / 5 |

The counts are baseline audit categories, not newly reconstructed or active
functions. Same-address alternatives already exist in active scene, fixed-step,
powerup and mission-frame sources. Their contracts must be reconciled before
substitution; distinct C++ namespaces do not add native-address coverage.
Hardware/vendor portions remain outside reconstruction credit.

The inspected source retains raw-pointer views and explicit required virtual
providers. No fake storage, fabricated process cell, default-return provider
or stub was introduced to make a link succeed. This admission does not review
every historical body or convert its historical evidence into current ABI or
gameplay acceptance.

The combined MSVC Win32 Release build and both existing math/tool suites
passed. The generated `bsp_core.vcxproj` includes all four sources, and each
compiled object is present exactly once in `bsp_core.lib`. The durable report
records source/header/object/library/map hashes and each module's game-map
presence. Receipt: `local/codex_module_admission_build.log`; report:
`reports/codex_module_admission.json`.

All four module names are absent from the resulting game map. The existing
suites passed in 14.87 seconds. This batch closes the compile-registration
gap while their original host call paths remain unintegrated.

Static-archive inclusion establishes compilation, not extraction into a game
caller. The missing raw storage, process clocks, message resolver/vector
custody and virtual providers remain separate integration work. No new tests,
game runs, switch changes, native ABI or activation acceptance are added here.
