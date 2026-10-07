# StartSpeed retained F payload

Packet `cc11_scene_start_speed_float_payload`, baseline `b06eee8d1`, owns
`00822C20` and three files: the scene-contents source, this doc and
`reports/scene_start_speed_float_payload_cc11.json`. There is no header, API,
layout, policy or math change.

`SceneReaderBinding::instantiate_entity` previously required StartSpeed's raw
diagnostic tokens and reparsed them into the retained entity record. It now
calls a private `retain_start_speed` helper in the same created-instance gate.
Successful nonempty explicit F uses the owning float payload, even if its
diagnostics are later empty. The helper's immediate result controls the same
summary increment. It is connected production code, not an unused host API.

## Native data contract

Read-only `bsp.py` queries verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. `00823590` belongs to `00822C20`'s stored body
`00822C20..00824B57`; its descriptive name is an existing hypothesis. The
complete relevant data slice was read in assembly. `EDI` is zero from
`008234D1`; the intervening instructions through this arm do not assign EDI,
and the prior cruise-speed audit establishes its preserved-register context.
`00823537/3A/3D` select holder kind1; its `+8h` is the scene property bag.

| Step | Evidence |
| --- | --- |
| StartSpeed name | Verified literal `00CFCCFC`, passed at `0082358B` |
| Find the record | `00823590 -> 008F2260`, ECX receives the bag at `00823588` |
| Absent record | `00823595/97` compare with zero and skip to `008235FC` |
| Choose value representation | Type word `+4h` compared with zero at `00823599`, branch `0082359C` |
| Int0 value | `0082359E` CVTSI2SS from the dword at record `+0Ch` |
| Other type value | `008235A5` MOVSS from float32 at record `+0Ch` |
| Retain float32 for seed | `008235AA` MOVSS to the stack |

The F type1 producer/storage contract is the accepted parser audit:
`008F5BE0..008F5C91`, heap producer `008F3770` and scalar constructor
`008EF170`. The latter is not an existing-record-copy helper. Separate clone
`008F4F60` and assignment `008F0700` support retained scalar copying. The
complete `008F2260` lookup listing was read in the preceding convoy packet;
native map/string allocation and dotted namespace/scratch providers remain
external. No field contract is inferred merely from a consumer argument name.

The adjacent `00823576` lookup reads ShipYardLaunch's byte at `0082357F`.
Its source path is unchanged. This packet mechanically checks both find rows
with `00822C20` as the containing function; reference-speed calls, division,
setters, controller math, whole-body/holder behavior and native ABI are not
newly reconstructed. Existing `CRUISE_SPEED_SETTING.md` owns that wider audit.

## Source wiring and preserved boundaries

The merged group/authored bag is retained only in the existing
`created.instance != nullptr` block. `retain_start_speed` writes the same
`GameSceneEntityRecord.start_speed_*` fields. `GameUnits`' actual
`StartSpeedSeedBinding::find_start_speed_00823590` reads those fields, and
`create_units` runs the existing start-speed arm for `runs_ship_base()` before
authored commands are latched. No GameUnits file was changed.

Only letter F plus `has_float` selects the new payload. Every other case keeps
the existing nonempty-diagnostic predicate, exact uppercase-I type0 mapping,
non-I legacy Float mapping and raw scanners. When diagnostics exist, the
incidental integer read still occurs even for F. Failed raw conversions skip
their assignments, rather than reset an existing field; a nonempty malformed
raw value still meets the old presence/counter predicate. Missing or raw-empty
input skips retention and counter increment without clearing an old record.
These permissive paths are preserved SOURCE policy, not expanded native typed
or graceful-error admission. ShipYardLaunch and all other properties remain
untouched.

The owning payload comes from one modern MSVC direct `std::sscanf("%f")` read
into float, on admitted finite ordinary C-locale decimal-prefix tokens in
closed, NUL-free input below the verified `400h` limit. Historical VS2005 CRT
`BF7533`, numerical/FP-status/error/extended-ST0 parity remain unverified.
No new scalar, x87 or ABI provider was recovered.

`has_float` is SOURCE availability, not native declaration identity or a
persistent success/replay action. Ordinary compatible group capture and
authored overwrite carry the owning metadata; declaration conflicts,
duplicates, implicit typing, enum identity at `+28h`, Lua/context providers,
structural aliases, missing/forward/cyclic parents, fault and allocator paths
retain their existing limits. Prior group-merge tail/body qualification is
unchanged. The containing convoy free-tail gap was separately repaired by
primary after that packet; this packet makes no flow repair.

Empty F remains a distinct unresolved parser dependency. Native guard failure
has a positive-zero temporary, but an existing type1 record is written only
on successful read; an absent record can be created with zero. A group copy
of such a zero is real value data. The source lacks that declaration context.
Bare F semicolon still has no payload and follows the old SOURCE skip. No
empty-F default, reset or replay metadata is introduced.

## Focused source evidence

One ignored probe includes the actual scene-contents production TU and freshly
compiles `scene_file.cpp`. It invokes the called retention helper, actual
PropertyLibrary methods and the existing primitive
`scene_start_speed_value_00823599` read service. It does not instantiate the
full emitter, GameUnits adapter or seed/controller math. Active enlarged
property paths are fresh; no enlarged record crosses an old active interface.
The prior immutable 3-library/74-Game-object snapshot is reused for linkage,
with all 77 hashes verified before and after linking. Uninvoked Game objects
are dependencies, not game execution proof.

Compile/run both exit 0 with Win32 `/W4 /WX /fp:strict`; the executable contains
RT_MANIFEST type24/id1 with `asInvoker`. Source discovery of the same bounded
15 library inputs yields 22 groups, without claiming VFS/enum/Lua loading.
Installed Ship's `StartSpeed = F 0.0` retains positive zero and matches the
prior SOURCE raw path. Fresh JM06 parsing yields 96 authored entities, with
sixteen DestroyerGen records authoring F10. Their actual merged bags retain
the owning values and incidental integer10. These are authored-source counts,
not sixteen emitter admissions or generated ships.

The scenario checks owning authored overwrite, a prefix value, diagnostics
cleared after parsing, signed zero and an unchanged group snapshot. It checks
the raw F/I/non-I paths, failed-conversion no-reset, missing/raw-empty skip,
untouched ShipYardLaunch and seven immediate counter eligibilities. Raw/empty
checks are explicitly SOURCE-only. Mode, Hidden, difficulty, class/type
admission, original CRT, native ABI and game behavior are untested here.

`git diff --check` and both direct-call verifier rows pass. No tracked test,
full build, game run, metadata or Ghidra mutation was performed. Primary owns
integration, the full build and independent production-TU rerun.
