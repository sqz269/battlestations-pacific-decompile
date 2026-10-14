# Actual native font runtime probe

`probe_actual_native_fonts` exercises `GameNativeFontApplication` using the
running application's actual renderer, device, owner registry, pooled strings,
texture cache, VFS and loaded Lua tables. It deliberately has an isolated empty
`FontRegistryStartup`; it does not replace the startup registry, export a
`NativeFontResourceOwners` association or construct Text.

## Integration contract

The helper is declared in `include/bsp/game_native_font_probe.hpp`:

```cpp
GameNativeFontProbeSummary probe_actual_native_fonts(
    GameHostLog&, GameNativeRendererApplication&, GameVfsHost&,
    GameScriptHost&, const std::string& language_font_path);
```

The primary integrator owns the opt-in `BSP_NATIVE_FONT_PROBE=1` startup hook
after genuine renderer/device/VFS and phase-7 Lua/font initialization. Pass the
existing script host's `files()`/`globals()` and the VFS language path through
this interface. The helper itself uses the original `kFontRootPrefix`
(`Fonts\\`) and `kFontDescriptorPath` (`Fonts/Fonts.lua`) constants.

Serialize the call with frame submission, device reset, cache mutation and
singleton draining. The source does not launch a second game process or install
providers. Production and probe sources require permanent `bsp_game` CMake
registration by the primary integrator.

## Actual observations

The genuine producer executes its retained native cache operation for each GFX
and alpha image, then the VFS DAT read, established scalar DAT decoder and
`NativeFontResources` adoption. An empty descriptor result is a probe failure.
Null images or unsupported actual texture profiles are probe failures; there is
no semantic wrapper or success fallback.

For every completed operation, the probe verifies:

- stable descriptor identity between the isolated registry and native resource;
- complete persistent cache operations, cleared transferred owning slots and
  decoded unique ownership, and no remaining pooled-name guards;
- distinct sole `FontData` allocations, nonempty original glyphs, fallback
  `0x0091`, matching known fallback fields and actual GFX/alpha image aliases;
- image identity authenticated first by the canonical actual owner registry,
  profile `0x00D61948`, `0x00D61870` or `0x00D618B0`, and the companion's exact
  alias of the image's live atomic reference count at raw offset `+04`.

Per-font logs include original record count, unique glyph count, signed height,
sole decoded owner address, actual image addresses, native profiles and raw
reference counts. A small observation vector aggregates expected font-owned
slots for shared image identities. It does not retain images, replace their
reference counts or copy glyph/metric data.

The helper explicitly calls `retire_fonts()` while descriptors and renderer are
live. For each unique image it checks the native count decreases by the number
of returned owning font slots. Canonical owner metadata is queried before any
possible post-retirement read: a predicted zero count must have no surviving
owner, while a surviving owner must report the expected actual count. Total
registry size is logged without requiring equality because a lazily created
cache fallback may remain owned by the application.

## Failure and lifetime policy

The registry and producer are held in one heap state. On success the producer
retires first and is destroyed before its descriptor registry. On any entered
failure the heap state is released from automatic destruction, the actual
exception and operation/cache/loader state are logged, the log is closed and
`std::_Exit(1)` applies the application's existing no-unwind process-retention
policy. The caller's renderer, VFS and script graphs remain live until process
exit. No failed frame is replayed, retargeted, implicitly retired or unwound.

Diagnostic native sites identify the producer's observed body region and the
existing cache/loader's retained site; they do not claim to recover an exact
original nested exception PC. Failure logging preserves path strings, transfer
slots, decoded/unpublished/published identities, loader HRESULTs and registration
state. A logging failure falls back to stderr and still exits without unwinding.

## Evidence and limits

This source packet was strict-compiled with MSVC Win32 C++17, `/W4 /WX /EHsc
/permissive- /O2 /MD /fp:strict`; the local object and log are under
`local/cc12_actual_font_probe/`. The worker's standard Win32 build verifies its
already registered sources and existing checks: `scripts/build.ps1` passed,
including both existing CTest cases (11.04 seconds). New probe/producer registration
and combined linkage belong to the primary integrator; separate strict
compilation does not establish that the helper is linked into the worker game.

Live execution is pending the primary's serialized startup launcher. This
document and the machine report do not claim a runtime PASS. A later PASS proves
only actual resource production, owner observations and retirement in that
run. A meaningful runtime failure is evidence of the genuine service boundary,
not a reason to substitute wrapper textures or fabricated providers.

No new Ghidra analysis or annotation is introduced by this diagnostic helper.
Its dependency's reconstructed AD4C30 producer still has an explicit typed
transport boundary: original register ABI, raw tree/pool layout, exceptional
timing and AD51D0 destructive reload are not established. The probe leaves
canonical GUI/Material/Text production and frontend binding unactivated. Native
Text rendering, world rendering, ABI compatibility and gameplay remain unproven.
