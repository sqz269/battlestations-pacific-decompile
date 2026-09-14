# Validated first-viewport admission in raw camera construction

Addresses: 00B71A80

The raw camera constructor now takes its prepared viewport admission before native effects and registers the successful first viewport before publishing camera+180. It requires the same installed resolver used by the camera frame. Legacy raw calls share the unchanged continuation; the semantic body stays separate. No native retain, count or cleanup is added.

Exact source `0307b963f66a80caa7f9308cecbfc2d5d6b780da` passed strict MSVC Win32 compilation, both existing CTests and eight seed checks. All 2470 tracked build inputs, both modified source/header files and the three linked libraries stayed unchanged. The fixture compiles only its own source against those current libraries. Independent source/fixture review and the source/library/recipe/log archive are pinned in `reports/native_camera_viewport_admission_bi_validation.json`.

The existing successful original/source CW027F helper pair still agrees at 15 camera checkpoints, 16740 camera bytes per path, 11 ordered events and 36 normalized helper bytes; 8880 mapped original code bytes remain unchanged. The first view is absent during B1F850 callbacks, and the subsequent camera callback sees the exact live registered viewport with actual count 1. Both prepared camera companion constructions still attempt zero guarded host allocations.

Two source-only failure observations bracket publication. Renderer call 2 fails inside viewport initialization: the record cancels and its raw allocation frees. Renderer call 3 fails after publication: camera cleanup ends, but its independently captured viewport remains live at count 1 until the fixture explicitly releases it later. Caller camera-slot return and raw-name cleanup are checked in both cases. These failures do not execute original FH3. No repository test was added.

This adds zero native bodies or bytes. The existing BSP_Camera_Construct name, original thiscall signature and complete 604-byte body were verified live; no Ghidra mutation was needed. Full B3C800 construction, persistent cockpit storage and quiescence, original replacement ABI/FH3 and gameplay remain open. The next design packet is NATIVE_COCKPIT_CONSTRUCTION_BLOCK_BJ.
