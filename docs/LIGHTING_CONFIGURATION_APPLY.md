# Applying lighting configuration to the existing owners

This packet reconstructs `004BACD0` and its seven required leaf helpers using
borrowed fields of the actual ambient owner and directional light tail. It adds
no light owner, fallback storage, cached colors or implicit initialization.
The optional configuration is a set of references to its current fields.

The native null path initializes and reuses three process fallback colors. It
leaves **effective specular `+194..1A3` and specular scale `+1DC` untouched**.
The configured path sets both scales and recomputes both effective colors.
Neither path clears the light's unknown scalar `+1D4`, ownership fields, node
prefix or pool slab ID `+1EC`.

All eight native spans (1,739 bytes) and five global spans (81 bytes) match both
the installed executable and the live saved Ghidra program. The binary SHA-256
is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The [audit](../reports/lighting_configuration_apply_audit.json) records exact
spans, hashes, native ABI, source pointers, numeric checks and limitations.

| Entry | End, exclusive | Native ABI and role |
| --- | --- | --- |
| `004BACD0` | `004BB155` | ECX ambient, EDX directional, stack optional configuration; RET 4 |
| `004B4D80` | `004B4DF3` | ECX output float3, stack first/second angle; EAX output; RET 8 |
| `004B62E0` | `004B6369` | ECX light, stack float4 pointer; copy base diffuse and scale; RET 4 |
| `00B7AF20` | `00B7AF3E` | ECX ambient, stack float4 pointer; copy `+18`; RET 4 |
| `00B7AF40` | `00B7AF5E` | ECX ambient, stack float4 pointer; copy `+28`; RET 4 |
| `00B7AF60` | `00B7AF88` | ECX ambient, stack float4 pointer/index; copy cube face; RET 8 |
| `00B7AF90` | `00B7B003` | ECX light, stack float32 scale; update `+1D8`, recompute `+184`; RET 4 |
| `00B7B010` | `00B7B083` | ECX light, stack float32 scale; update `+1DC`, recompute `+194`; RET 4 |

## Required bindings

`lighting_ambient_fields(ConcreteSystemAmbientLight&)` references the same
`ambient_18`, `ambient_mode3_28` and six `ambient_cube_38` words read through that
owner's existing `lighting` view. `LightingDirectionalFields` binds directly to
the directional owner's `NativeLightTailStorage`: `diffuse_184`, `specular_194`,
`base_diffuse_1a4`, `diffuse_mode3_1b4`, `base_specular_1c4`,
`diffuse_scale_1d8`, `specular_scale_1dc` and `direction_1e0`. Constructing either
view writes no owner field.

| Configuration source | Meaning established by this call |
| --- | --- |
| `F48..F57` | Ambient float4; fourth word is the optional RGB multiplier |
| `F58..F67` | Raw mode-3 ambient float4 |
| `F68..FC7` | Six float4 cube faces; fourth word always scales RGB |
| `FC8..FD7` | Base diffuse float4 |
| `FD8..FE7` | Raw mode-3 diffuse float4 |
| `FE8..FF7` | Base specular float4 |
| `FFC` / `1000` | Diffuse / specular scales |
| `1004` / `1008` | Second / first angle passed to `004B4D80` |

`FF8` is not read by this routine. Names for the angles describe their parameter
positions; interpreting them as yaw/elevation follows the recovered formula,
not an authored map-field schema.

`LightingConfigurationGlobals` requires references to actual process storage:

| Guard DWORD | Color DWORDs | First-use values |
| --- | --- | --- |
| `00E18B18` | `00E18B08..17` | Current `00CE3800` in RGB, current `00D7A24C` in alpha |
| `00E18B04` | `00E18AF4..B03` | Captured current `00D7A24C` in all four words |
| `00E18AF0` | `00E18AE0..EF` | Raw zero RGB, current `00D7A24C` in alpha |

It also requires current direction words `00F8758C/90/94`, the mutable byte
`00F88A0C`, and source words `00CE3800` and `00D7A24C` (installed `.5f` and `1.f`).
The saved executable's fallback colors, guards and direction region are zero
filled; this is startup-file evidence, not permission to replace the current
process words with zeros. Initializing or loading that actual process storage
is the integrator's responsibility.

## Order and floating-point behavior

Null configuration first visits all six cube faces, then ambient, mode-3 ambient
and directional mode-3 diffuse. Every use tests mask `0x1` in the guard's low
byte. First use ORs `0x1` into the complete DWORD, preserving every other bit.
The cube loop reads `00CE3800` even when already guarded. Later calls reuse the
current fallback words; editing a guarded fallback changes the next result.

The diffuse guard test occurs before the directional mode-3 stores. Its alpha
constant was already captured, including when the half-color guard was set.
The specular guard test occurs before the three default-direction stores.
Those captured branch decisions and the source-word read positions are retained.
Default direction uses raw copies, so signaling NaNs and negative zero retain
their bit patterns. Base diffuse is recomputed with the existing `+1D8` scale;
null configuration neither resets that scale nor updates effective specular.

On nonnull configuration, any nonzero `00F88A0C` enables ambient scaling. The
ambient multiplier is first copied with raw MOVSS semantics. Each cube multiplier
instead passes through FLD/FSTP float32 before multiplication, which can quiet a
signaling NaN. Both paths perform the fourth alpha multiplication even though
the final alpha word is replaced with the current `00D7A24C`. Discarding this
operation would lose its floating-point exceptions; the fixture specifically
observes overflow from the otherwise discarded ambient alpha square.

The scaler leaves retain x87 operand ordering and float32 spills for each
channel. They multiply all four words, with x/y destination stores while the
alpha product is still on the x87 stack. `004B62E0` first spills the existing
scale, copies base words sequentially, then rereads the original source for
multiplication. The separate scale setters store their raw input bits before
using them. Their caller first spills the configuration scales through x87.

The angle helper executes three FCOS and two FSIN instructions, with the native
intermediate float32 spills and repeated cosine calculation. For ordinary finite
angles it corresponds to `(sin(second)*cos(first), sin(first),
cos(second)*cos(first))`, but generic library trigonometry is not substituted.
The caller first spills `1004`, then `1008`, calls the helper with the latter as
its first argument, and spills each returned direction component into the light.
The implementation preserves the caller's x87 precision and rounding controls.

## Focused verification

One native/host timeline passes four calls: first null initialization, another
null call after changing guarded fallback words and direction, configured input
with `00F88A0C=0`, and configured input with `00F88A0C=FF`. The fixture uses the
existing concrete ambient constructor and its actual read/write views, plus the
directional worker's real `NativeLightTailStorage` type placed at `slot+174`.

Each checkpoint compares the entire ambient native prefix (`0x98` bytes), the
entire `0x1F0` light slot, all fallback words/guards and current source globals:
729 bytes per call, **2,916 identical bytes** in total. It includes signaling and
quiet NaNs, negative zero, subnormals, mutable fallback colors, preserved guard
high bits, unchanged null specular storage and the untouched pool slab word.

| Call | x87 control | Native and host status, mask `47FF` |
| --- | --- | --- |
| Null, first use | `027F` | `0000` |
| Null, guarded fallback changed | `067F` | `0003` |
| Configured, ambient scale off | `0A7F` | `0033` |
| Configured, ambient scale on | `0E7F` | `003B` |

The final status adds overflow from the discarded alpha square. Every control
word remains unchanged after the call. The native run uses only the eight
verified native bodies and five data spans in sparse reserved image space, with
64 recorded absolute operands rebased. There are no native hooks, imports,
unresolved-call stubs, full PE load or game entrypoint invocation; other bytes
on committed pages are INT3, and other pages remain inaccessible.

Strict MSVC Win32 `/W4 /WX` compilation and the focused comparison pass.
`./scripts/build.ps1` and the existing `reconstructed_math` CTest pass; the primary
integrator registers the new source in shared CMake and binds the real runtime
owners/globals. This is reconstructed and focused native-fixture behavior,
not a drop-in native ABI, authored configuration schema, visual/render result or
game validation. Unmasked floating-point traps, concurrent external mutation,
invalid cube indices and all possible source/destination aliases were not
executed. The original game and Ghidra program were not mutated by this packet.
