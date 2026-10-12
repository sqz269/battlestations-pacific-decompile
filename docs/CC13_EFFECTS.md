# CC13 effect instances (band 00865000-00875FFF)

Packet cc13-effects1, 2026-10-12. Worktree `agent/cc13-effects1`. This band holds the gameplay-effect
manager, the effect components and their instance objects (sound, splash, flare, waterdrops, light,
thunderstorm, model events), and at its end the fixed-step tick registries. Most of the spine
was named and reconstructed by earlier packets (POINT_EFFECT_*, LIVE_EFFECT_*,
GAMEPLAY_EFFECT_*, REGISTERED_MODEL_EFFECT). This packet names what was left raw and reconstructs
the instance virtuals and the sound instance constructor.

Status terms: **named** = ledger + Ghidra; **reconstructed, build-tested** = C++ in
`src/effects_cc13.cpp` that compiles under `./scripts/build.ps1`. Nothing here has a fixture or
a game run, and nothing is ABI-compatible. The game host does not call any of it.

## Counts (`tools/progress_counts.py --range 00865000-00875FFF`)

| | total | raw | ledger | tagged | recon |
| --- | --- | --- | --- | --- | --- |
| before (index at 760c88179) | 402 | 78 | 222 | 73 | 211 |
| after (this branch, index rebuilt) | 418 | 0 | 359 | 92 | 211 |

The total grows by 16: functions defined in Ghidra (see below). The ledger and tag columns
also include records merged from `origin/main` between the two index builds. The raw count
from this packet alone is 78 to 0.

## What was done

- **94 names** through `ledger add-name` (shards `config/names/00860000.jsonl`,
  `00870000.jsonl`) and `tools/ghidra_annotate.py --apply`. Prior Ghidra values are in
  `local/ghidra-annotations-20261012T152006Z.json` in the worktree. 78 were the raw functions;
  16 were vtable targets with no Ghidra function.
- **16 functions defined** with `tools/ghidra_define_function.py` under the write lock, record
  `reports/cc13_effects1_functions.json`: 0086B9A0, 00871FF0, 00872080, 008720A0, 00872100,
  00872630, 00872650, 00872670, 00872690, 00872720, 00872770, 00872E30, 00872E70, 00872F40,
  00874AC0, 00874B00. Each one starts after INT3 padding, except 00872690, which follows 00872670's `RET 4`. Each is referenced by an .rdata vtable
  slot, except 00872E70, which only appears in the 00D7xxxx address table. The ends were read from the disk
  listing. `ghidra proto 00BF9DC8` showed `void __cdecl _free(void * _Memory)` before and after.
  'Non-Returning Functions - Discovered' was not touched.
- **Template pipeline**: `st1_shapes.py 53 54`, `st1_auto/rules/family`, then
  `st1_build.py --range 00865000 00876000` (dry run). The result was one `stl_probable` row,
  0086D180, from the untrusted `scl_97plus` rule. That function is the water tracer Lua reader, so
  nothing was written or applied. The raw template copies here (refcount release, vector
  push/reserve/destroy, tree iterator wrappers) do not match any known shape, so they were
  named by hand with an address suffix, following the band's existing `BSP_NativeDwordVector_*`
  names.
- **Reconstructed, build-tested**: 24 functions in `src/effects_cc13.cpp` /
  `include/bsp/effects_cc13.hpp` (registered in `cmake/startup.cmake`).

## Instance vtables

Each instance has a primary table at +0 and a secondary at +8. The secondary is the primary
minus one dword, and the constructors write both. The point-effect child update `00867790` calls
through the primary: +08h IsComplete, +28h Update(dt, reference node), +30h Stop. It skips
Update for types 1 and 4 (model events). The base table is 00D0C888, where slot +08h is
`__purecall`.

| instance | secondary table | IsComplete +08h | Update +28h | Stop +30h | destructor body |
| --- | --- | --- | --- | --- | --- |
| waterdrops | 00D0DBB4 | 00872120 | 00873100 | 00872BF0 | 00872C10 |
| light | 00D0DBEC | 008721A0 (named earlier: `BSP_EffectNode_IsNonpositive20`) | 008738B0 | 00872D30 | 00872CB0 |
| splash | 00D0DC24 | 008721C0 | 00872E30 | 00872DF0 | 00872D60 |
| flare | 00D0DC5C | 008720F0 | 008720A0 | 008720D0 | (scalar dtor 00873060) |
| sound | 00D0DD74 | 00872650 | 00872F40 | 00872630 | 00873DE0 |
| thunderstorm | 00D0DDCC | 00872110 (false) | 00873C80 | 00872100 (RET) | 00873FF0 |
| model event | 00D0DE14 | 00872010 | 00872740 | 00871FE0 | 00874430 |
| type4 event | 00D0DE4C | 00872020 | 00872790 | 00872060 | 00874540 |

The sound instance overrides more slots: +0Ch ScaleVolume 00872670, +10h SetPitch 00872690,
+1Ch SetParameterFraction 008726C0, +20h SetLevel 00872FB0, +24h SampleFlag15 00872720. The
model event overrides +14h (00872770, model +1DCh = value) and +18h (00871FF0,
`BSP_ParticleModel_SetEmitterValue`). The type4 event overrides +14h (00872080,
`BSP_SkinedWaterTracer_SetAlpha`). These slot meanings come from the bodies; the base
defaults 00858170..00858210 lie outside this band and were not read.

Component tables (slot 5 = ReadLua): the water tracer 00D0D5D4 has `0086D180` as its reader
and `0086BF10` as its destructor. The thunderstorm 00D0DA38 has `0086F3B0` as its reader and
`0086FBC0` as its destructor. The tracer's secondary base sits at +80h
(00D0D590): adjustor `0086B9A0` and destructor `0086B9B0`.

## Reconstruction (`src/effects_cc13.cpp`)

| address | function | notes |
| --- | --- | --- |
| 00873AE0 | `construct_effect_sound_instance_00873ae0` | prefix, tables 00D0DD78/00D0DD74, type 2; sample pick and pitch, both via `EffectPresentationRandom`; x87 tail read from the listing (00873C3E..00873C5D: `u - float(2s)*0.5 + 1.0` with qword constants, FSTP float) |
| 00872630 / 00872650 / 00872670 / 00872690 | sound Stop / IsComplete / ScaleVolume / SetPitch | |
| 008726C0 | sound SetParameterFraction | sound vtable +28h range `{id, lo, hi}`, x87 `lo + (hi-lo)*t`, then +2Ch |
| 00872720 | sound SampleFlag15 | byte +15h of 00A81860(sound +4Ch) |
| 00872F40 | sound Update | set_3d(node world position, point +5Ch); dt and reference node unused |
| 00872FB0 | sound SetLevel | SSE clamp to [0, 1], FUCOMPI; NaN is stored (traced in the comment) |
| 00873DE0 | sound destructor body | stop(1), release, clear |
| 008721C0 / 00872DF0 / 00872E30 | splash IsComplete / Stop / Update | Stop zeroes the timer before 00B9E4A0, which the decompiler dropped |
| 008720F0 / 008720A0 / 008720D0 | flare IsComplete / Update / Stop | |
| 00872120 / 00872BF0 | waterdrops IsComplete / Stop | |
| 00872D30 | light Stop | |
| 00872110 / 00872100 | thunderstorm IsComplete / Stop | constexpr |
| 00865FE0 | `point_effect_is_finished_00865fe0` | pending count +1Ch <= 0, byte +0Ah clear, all child slots null |

All of these are 'reconstructed, build-tested' only. The calls the image makes outside the band
go through `EffectInstanceServices`, one virtual per image address. Payload virtuals go through
`EffectTrackedSound`.

## RNG sites

| site | function | entry | ECX (image stream) | draw | host handling |
| --- | --- | --- | --- | --- | --- |
| 00873B63 | 00873AE0 sound instance construct | 00BD2FC0 raw word | `00873B61 XOR ECX,ECX`, stream 0 | sample index = word % sample count | `EffectPresentationRandom::next_u32(EffectRandomSite::sound_sample_00873b63)`; not wired |
| 00873C39 | 00873AE0 | 00BD2F10 U(lo, hi) | `00873C26 XOR ECX,ECX`, stream 0 | U(0, 2*PitchRnd); pitch = 1 + U - PitchRnd | `uniform(EffectRandomSite::sound_pitch_00873c39, 0, 2s)`; not wired |
| 00874313 | 008742A0 model event construct | 00BD2FC0 raw word | `00874306 XOR ECX,ECX` (both paths), stream 0 | particle variant = word % [def+2Ch] | existing `src/registered_model_effect.cpp` draws `RandomStream::primary`; that is a library, not wired into the host. If it is ever wired, it must move to a presentation stream (not this packet's file) |

Both sound draws are skipped when the definition has no samples (00873B5B). A rel32 scan of the
whole band for calls into 00BD2E00..00BD3100 finds only these three sites; the other hits are
00BD30F0 `BSP_RefCounted_DestroyBase`. The lightning storm (00873EB0 builds it, 00873C80 ticks
it) calls code outside the band that was not scanned.

## Gameplay left for cc9

- `008746A0` `BSP_SimObject_RunFixedSteps_Provisional`: for each count it calls vtable+4(0.05),
  vtable+0Ch(), vtable+8(0.05). Its callers are 006E16D0, 006E1930, 0074D230 and 007D1360
  `BSP_Plane_ApplyControlStateMessage`, so it steps unit simulation. It is named only.
- The tick-registry singleton holders (00874740, 00874AC0, 00874B00, 00875420, 00875820) are
  frame-loop infrastructure and are named only. Nothing in the band applies damage.

## Open items

- The base slot defaults 00858170..00858210 (band 0085) and the meanings of base slots +14h/+18h
  and +2Ch are not established.
- Waterdrops Update 00873100, light Update 008738B0 and thunderstorm Update 00873C80 are named
  but not reconstructed. They are x87-heavy and call the weather renderer 00BC1xxx setters
  (docs/CC13_WEATHER_RENDER.md).
- `008679D0` `BSP_PointEffect_RetireChild` has one caller, 00857160. Ghidra has no function
  that contains the call site 008573ED (verify_report_calls), so that row was left out of the
  report.
- 00866688, 0086FEFF and 00872EFA are data-referenced mid-body addresses (resource or EH
  tables), not function starts. They were not defined.
- The image's 00D7C000..00D7FFFF table lists addresses inside functions across the band (for
  example 00865013). Its purpose was not determined.

## Handoff

The band has no raw functions left. Next steps: reconstruct the three x87 Update bodies above,
read the 00858170..00858210 base defaults (band 0085, segment 53), and settle whether
`registered_model_effect.cpp` should adopt a presentation stream before any host binding.
