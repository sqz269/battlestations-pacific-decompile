# Type10/11 host, environment, and mapped-path TEXT correction

The final unadopted correction passes passive review with **Source credit 0**.
It supplies proposed checks for uppercase Python environment keys, the exact
PowerShell executable and complete external environment, and the loaded Capstone
module path. It supplies no Root acceptance or execution admission.

The immutable evidence family is
`J:/PROG/battlestations-pacific-decompile-cc12_type5_complete_guard_materialization_TEXT/local/t1011hostEnvTEXT`.
Only these final inputs are presented for further Root review:

- Seven payloads in `candidate_TEXT_v3/`.
- `proposed_dependency_selection_template_v4.json`.
- `private_proposed_environment_profiles_UNADOPTED.json` and
  `private_proposed_external_environment_v3_UNADOPTED.json`.
- The unchanged `candidate_materialization_draft119/`.
- `FINAL_TEXT_REVIEW_INPUTS_NOT_ROOT_SELECTION.json`, which identifies those
  inputs without filling any Root selections.

Earlier `candidate_TEXT` and `candidate_TEXT_v2` are consumed historical
derivations, preserved without overwrite. They are not the final proposal.

## Changes

Both illustrative Python profiles use uppercase keys, including `SYSTEMROOT`.
Their original values are unchanged: 10 root-process keys and 12 recipe-child
keys. The launcher and bootstrap reject non-uppercase Python profile keys and
non-string values before accepting the original exact environment comparison.

Two new concrete fields, `external_powershell_host` and
`private_external_environment`, remain **null**. All nine preceding concrete
Root fields remain null, and both Root adoption flags remain false. When Root
later makes a fresh selection, the proposed launcher checks the current
`Get-Process -Id $PID` executable path, byte length, and SHA-256 against the
selected host pin. It compares the full actual external environment against its
separate pinned profile using ordinal key-set and exact value equality. The
check runs before interpreter dispatch and inside both dependency bookends.

The external profile admits 14 named safe context keys and rejects case aliases
or extra actual keys. These include `PATHEXT`, `PSMODULEPATH`, `USERPROFILE`, and
`BSP_COORDINATION_DIR`. Its illustrative 14-key profile stays separate from the
Python profiles. Root reported that its own clean PowerShell launch adds
`PATHEXT` and expands `PSMODULEPATH`; this peer did not repeat that launch.
Consequently, the illustration is not evidence of a reproducible host profile.
Root must independently choose and pin the actual complete profile, including
its exact casing, values, user context, and coordination context.

After Capstone loads, the proposed bootstrap retains the packaged-file hash and
requested-path checks, then queries the already loaded `_cs._handle` using
`GetModuleFileNameW`. The signature uses a pointer-sized module handle, a wide
character output pointer, and 32-bit count/return values. A null/non-integer
handle, zero result, truncated result, non-absolute path, differing resolved
path, or differing current file pin rejects admission. The fixed buffer holds
32,768 wide characters. This follows the documented loaded-module and return
contracts in [Microsoft's GetModuleFileNameW reference](https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-getmodulefilenamew).
The API library request uses the documented
[`LOAD_LIBRARY_SEARCH_SYSTEM32` flag](https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-loadlibraryexa).

The mapped-path observation remains private bootstrap state. The original
`runtime_description()` AST is unchanged, so importing Capstone cannot change
the static dependency-guard value inside `recipe.selection()` and invalidate the
exact Root TEXT receipt. The first derivation included that changing observation
in the static value; it was rejected during review and corrected in a new
derivation. No selected body was executed during that review.

## Preservation and verification

Only `dependency_bootstrap.py.txt` and
`external_preinterpreter_guard.ps1.txt` differ from the sealed Root payloads.
The driver and all four recipe/reader payloads are byte-identical to Root's
complete guard draft. Independent AST comparison retained every original guard
and all current guards:

| Role | Original explicit guards | Current guards, preserved |
|---|---:|---:|
| Recipe | 49 | 50 |
| Primary gate | 87 | 89 |
| Independent code reader | 86 | 88 |
| Independent recorded reader | 58 | 58 |
| Total | 280 | 285 |

`launch`, `launch_ast`, `compile_plan`, and `link_plan` are AST-identical to the
original draft. The Native constants and six ordered launch keys are unchanged.
All 119 materialization files are byte-identical to the sealed Root draft; the
only differences from the older original119 are the same pre-existing four
programs, manifest, and receipt. Source/header/probe/case bytes were not edited.
All original selection values remain unchanged, including Source12/case28 pins,
dependency pins, package maps, and literal membership predicates.

The PowerShell parser reported zero syntax errors. All 24 original `Require`
calls and eight `Check` calls remain in order; the proposal has 36 and 11,
respectively. Mandatory parameters, original throw statements, the `Members`
body, and the complete outer dispatch `try/catch/finally` body are preserved.
Python's literal membership predicate, recipe environment expression, child
dispatch body, and timeout behavior are also AST-identical. The mapped-path
check follows decoder import and precedes every reviewed `capstone.Cs` call.

Before optional own analysis imports, the peer freshly froze 2,102 interpreter,
stdlib, package, startup, metadata, and native companion files in 3,588 membership
records across 16 scopes. Its final own Python review ran with `-I -B -E -S`, an
empty bytecode prefix, and 44 verified loaded file origins. Original Root's exact
10,594 files and the separate original119 were independently verified and copied.
The Root seal remains 2,029,480 bytes / SHA-256
`2ac0ce5032ef89c871fd335c13d370c812592d62732c9a576abdc4b6c5a2b646`.
Own pre/post/final checks verified all 12,815 original/copied input pairs, exact
family membership, all analysis scopes, and the full private own environment.

## Holds and limits

All 10,422 original dependency files still match the sealed Root pins. The
`Main_include_guard` scope has grown from 2,016 to 2,022 files, adding
`game_native_game_application_frame.hpp`, `native_entity_destroy_state.hpp`,
`native_global_config_sample_dispatch.hpp`, `native_mission_clock_reset.hpp`,
`native_subtree_invalidation.hpp`, and `native_world_current_tick.hpp`.
That membership drift remains an execution hold. The old snapshot was not
repinned or adopted.

The descendant-timeout cleanup limitation remains explicit. An inner 540-second
timeout can terminate outer Python before the 600-second PowerShell wait expires.
The outer process can then be closed while its descendants remain unproven; the
existing final tree-kill branch can be skipped. A nonzero result blocks dependent
actions, but it does not prove descendant cleanup. This correction does not waive
that limitation or change the timeout body.

No selected helper, driver, parser, function, or Win32 query was executed. There
was no compiler, Native/Ghidra, provider, original-PE, target, or prior runtime
replay; no Root receipt or materialization; and no Main write. The future
`local/type1011p1` family remained absent. The new mapped-path guard is proposed
code, not a captured runtime observation, and does not prove mapped image bytes
or every Windows module's identity. Own utility failures and corrected utilities
remain in the evidence family; the final reviews and bookends passed.

The peer seal lists 13,047 artifacts plus itself: exactly **13,048 files**.
`seal.json` is 2,616,714 bytes, SHA-256
`843709f896d31e6a9213464ce8c34b710413d8c07dfca90e896bcd094b607b8f`.
See [the structured correction report](../reports/cc12_type10_type11_host_ENV_guard_TEXT_correction.json)
for final payload pins, preserved checks, holds, and the evidence boundary.
