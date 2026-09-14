# Actual renderer bridges for effect admission

Addresses: `00B18D60`, `00B407A0`, `00B5F160`, caller adaptation at `00B2ED43`.

The active B2BB90 fixture investigation found two callable-table assumptions in
the canonical effect preparation path. The new overloads consume the existing
raw renderer and the borrowed, mutable D5F0A8 profile. They dispatch the existing
substantive B319B0 texture cache and B1FF50 capability getter. They never call a
numeric game address as host code or replace an unknown table entry.

| Routine | Coverage | Native schedule retained |
| --- | --- | --- |
| B18D60, 304 bytes | Complete normal actual overload | Construct fields and `error.tga`; capture current renderer, profile and slot64; state3 call; publish returned owner to98; release temporary; sample/increment serial and clear flags. |
| B407A0, 122 bytes | Complete normal actual overload | Base first; clear C4/138/13C, publish D61A00, clear fourteen140 DWORDs; leave C8..137 untouched. |
| B5F160, 653 bytes | Complete normal raw-profile route | Six conditional state groups precede current renderer/profile/slot104 capture, B1FF50, returned byte3D and the final removals. |
| B2EBB0 | Existing body, constructor continuation changed | Adopt the construction frame before allocating the raw effect and calling B407A0. Preserve failed raw storage with its child frame. |

`NativeMaterialEffectConstructionAccess` gains the actual texture cache and
borrowed renderer profile. The explicit third-argument constructors require a
fresh `NativeMaterialEffectConstructionFrame`. It owns the real temporary header
and a `NativeTextureCacheAcquired` allocated before the native texture call.
The texture cache must share the exact string and current-renderer publication
domains. The returned owner transfers to98 with no additional retain.

The frame records reached sites, captured receiver/profile/target, exception
state and publication status. A failed frame remains alive with the partially
constructed storage and child acquisitions. The actual B2EBB0 caller does not
delete that storage or simulate native constructor/member unwind. This is a
source exception boundary: original FH3 cleanup, hardware faults and secondary
exceptions are not proved. Failed frames cannot be replayed or silently retired.

`NativeMaterialEffectProgramsContext` gains a trailing borrowed actual renderer
profile. B5F6A0, including B17DD0's canonical path, uses the explicit raw pruning
overload. An unsupported profile/target is rejected when its native read/call is
reached, after earlier removals. The original standalone two-argument callable
pruner and constructors remain explicit compatibility interfaces. Canonical
program finalization now requires the actual profile field.

The compiler provider is consumed without edits from corrected `03e003dc`
(same released `b0a4ddb4` blobs), dependency commit `d10b8140`. The platform message
provider is consumed without edits from published `a234503d`, dependency commit
`0dfab46d`. Neither dependency's ledger or Ghidra metadata is changed here.
The six changed reconstruction files are committed at
`26f83bd6390dc51010c8f2eaf50cde8f7465ea3e`.

## Focused evidence

`local/debug24-active-runtime/effect_bridges_probe.cpp` adapts the immutable
compiler-runtime World fixture (manifest `21c0fb55`, source `39a9c24a`). It uses the
same raw string pool, actual texture constructor/cache, real HAL/COM texture,
canonical texture/state references, and substantive platform adapter. B30B40
pumps messages even on a hot alias hit; the fixture retains the real adapter
and real XLive pretranslation binding. Its three completed null-online calls do not exercise the
nonnull online/input/device getter route. No game process is pumped.

One fixture verifies base and derived construction with a real hot `error.tga`
owner: references advance1→2→3, serial wrapsFFFFFFFF→0→1, required fields change,
and native unwritten fields remain A5 preimages. An unsupported renderer slot
and an unsupported inner cache acquire slot retain the exact parent/child
frames, temporary string and prior writes without publishing/releasing a
fallback. The inner case enters actual B319B0/B30B40 and retains its failed child.

Both capability branches compare the full original653-byte B5F160 body against
the raw source overload, including state row order and real owner cleanup.
This is a **relocated native-profile observation**: the original invocation's
receiver profile pointer and selected slot104 point to private copies of the
verified native profile and exact original seven-byte B1FF50. Every other one
of the81 profile words is checked unchanged. The original renderer-global
operand is relocated to the fixture's actual publication cell. Its33 direct
B5EE00 calls use a disclosed ABI adapter to the substantive source child.
The source invocation restores numeric D5F0A8 and borrows unchanged original
profile bytes. This is not an unmodified native-address or whole-original-parent
test. A prior fixed-address attempt refused an occupied region; no existing
mapping was overwritten. That refusal log is retained.

Win32 `/W4 /WX` build, two existing CTests and eight native seeds pass. The probe
uses `/MD`, `/fp:strict`, `/MANIFEST:EMBED`, an exact link map, recorded include
dependencies and loaded module paths. Seventeen current live/installed-PE spans cover
4822 bytes, including original constructor/loader FH3 maps and cleanup thunks.
The final archive manifest pins source, native inputs, compiler
reads/commands/objects, link inputs and loaded runtime modules.

## Active B2 boundary

No active B2 record or draw is executed. A warm renderer19E4 alone does not
bypass the fresh per-record B4C700 generated-model factory. Canonical effect
admission requires a completed B46950 operation and initialized C8/100 pass
slots; the frozen compiler World contains only scalar effect input storage.
Its hot shader/texture setup does not establish that effect owner, declaration,
layout or generated-model cache. The first descriptor input boundary is
B45F5E→B43B00, including B43B4E→B69D40 for the actual Lua/VFS shader script.
Existing producer bodies are present, but their complete input/domain setup is
not proved by this fixture. A separate confirmed composition gap is
B463B6→B45E00: a present primary C8 still reaches the old callable B44B10
constructor in `native_material_pass_copy.cpp`. It needs the existing actual
compiler pass constructor and a persistent secondary-operation frame in a
separate packet. No fake effect registration, manually initialized
pass slots, draw callback, native scratch default or cold loader shortcut is
introduced. Original constructor FH3, full B2 invocation, teardown and gameplay
remain unproved. Parent integration preserves its newer descriptor teardown
interfaces independently of this narrow patch.
