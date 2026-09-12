# Icon rotation and temporary-state frame tail

Addresses: `00AB1150`, `00AB1110`, `00AB27F0`. Names are descriptive hypotheses.
This extends the existing `GuiIconRuntime` over its same `GuiLayoutWidget` and
`GuiIconWidget`. No additional elapsed clock, texture state, geometry owner,
virtual-profile token interpreter or substitute callback graph is introduced.
The public methods have a new semantic C++ ABI, not native layout/vtable ABI.

| Native routine | Original ABI | Coverage |
| --- | --- | --- |
| AB1150..AB11FC | ECX Icon; float seconds stack; RET4 at AB11FA | Complete derived continuation AB1161..AB11FC; AA87B0 at AB115C is delegated to the canonical frame integrator |
| AB1110..AB1149 | ECX Icon; immediate index DWORD, expiry index DWORD, float seconds; RET0Ch at AB1147 | Complete on the existing concrete Icon current88/current80 profile; index low16 bits are consumed |
| AB27F0..AB281E | ECX Icon; float rotation stack; RET4 at AB281C | Complete on the existing concrete Icon current8C/current80 profile |

The configured wrappers verified `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe` before every live batch. The worker only read
Ghidra and refreshed exports. AB27F0 was initially undefined: live bytes matched
the whole 47-byte disk routine, with CC padding at AB281F. The preceding saved
function ends AB27E6; AB27E7..AB27EF is outside this body. The primary agent
subsequently defined AB27F0 through its locked repair path; readback established
the exact inclusive AB27F0..AB281E body. AB1150 and AB1110 have zero listing gaps.

## Producers and actual targets

The constructor AB5C60 stores table D5C4C0 at AB5C91. Verified table cells are
D5C500=current40/AB1150, D5C504=current44/AB27F0,
D5C544=current84/AB1110, D5C548=current88/AB1710,
D5C54C=current8C/AB10D0 and D5C540=current80/AB3CB0.
Only the established canonical Icon profile is supported by these methods.
Other derived classes sharing some table entries are not thereby implemented.

| Native field | Existing retained field | Producer evidence |
| --- | --- | --- |
| +128 float | `state_seconds_remaining_128` (formerly `field_128`) | Constructor AB5D0C stores zero; AB1110 stores its third argument with MOVSS at AB1137, after current88 returns |
| +12C int16 | `expiry_state_12c` (formerly `field_12c`) | Constructor AB5CFF clears it; AB113F stores the second argument's low16 after the timer store |
| +130 float | `auto_rotate` | Constructor AB5D14 stores zero; property descriptor AB350D targets +130, with string D5C3E8=`AutoRotate`, float tag2 and zero default before visitor current0C at AB352F |
| +48 float | existing base `transform.rotate` | AA7936 MOVSS stores the supplied rotation, then AA793B calls AA7220 on the same owner |
| +134 byte | existing `cached_prefers_bilinear` | Constructor AB5D06 clears it; existing AB3CB0 updates the material-filter cache |

AB1110 first forwards the immediate index to current88 with mode0 and ratio1.
Only on its successful return are countdown and expiry state stored. The low16
state carrier is a semantic interface restriction, not an exact DWORD ABI.
The current88 body AB1710 is reused: it clamps the ratio, applies its existing
state/mode/ratio predicate and dispatches current80/AB3CB0 when required.
No new state vector or timer setter bypasses this producer ordering.

## Ordering and floating-point details

The canonical integrator must keep one active-owner lifetime guard across the
whole operation, invoke AA87B0 exactly once, and then invoke
`update_after_base40_00ab1150`. AB1151/AB1157 spill a copy of original seconds
through x87 for the base argument. The original caller float remains available
to the derived tail, rather than the parent's accumulated +80 elapsed clock.
The tail intentionally never calls the base itself. Its `zero_00d7a218` argument
must alias the canonical frame service's live constant storage; current bytes
are positive zero. Ownership, the concrete Icon type, and the runtime service
storage must survive every callback. Canonical retirement/reentry guards are
provided by the integrating frame runtime, not a second Icon lifetime domain.

After all base callbacks, AB1161 reloads AutoRotate. The UCOMISS/LAHF/TEST
sequence skips only ordered equality with the live zero; NaN follows rotation.
The x87 multiply rate*original_seconds stays unspilled until the subsequent
add of live base Rotate. AB118F spills the sum to float, then AB1193/AB1197
spill the outgoing argument again. The runtime uses explicit x87 instructions
to preserve this sequencing under Win32 `/fp:strict`.

Actual current44/AB27F0 first performs another float argument spill and calls
base AA7930, which MOVSS-stores Rotate and recomposes through the same owner.
It then runs the existing actual AB2600 filter query. That query and the
existing scale48 route share one retained implementation, preserving the
HasTexture/ShaderName/platform/rotation/scale short circuits and the selected
state/texture lookup. Width is read before height from the same captured
texture. After callbacks, +134 is reloaded. A changed choice calls actual
current8C/AB10D0, which reloads current state and reaches existing current80
geometry. It does not write the cache itself or suppress a -1 state.

After current44 returns, AB119C reloads +128. COMISS/JBE skips nonpositive and
unordered countdowns. For a positive countdown, x87 subtracts original seconds,
spills the result to float, reloads it and stores +128 before FLDZ/FCOMIP.
Only ordered result <=0 expires; unordered subtraction results stay stored and
do not select a state. Expiry reloads +12C and calls actual current88 with
mode0/ratio1. The unconditional positive-zero store occurs only after that
callback returns, so even a new timer set by that callback is overwritten.
Exceptions propagate with completed effects intact; a failed expiry rebuild
leaves the already-subtracted countdown instead of executing the zero store.

## Calls and correction of earlier scope

| Site | Containing function | Actual target / contract |
| --- | --- | --- |
| AB115C | AB1150 | AA87B0, same-owner base exactly once under the integrator's guard |
| AB119A | AB1150 | current44/AB27F0, x87-computed float rotation |
| AB11EB | AB1150 | current88/AB1710, low16 +12C, mode0, ratio1 |
| AB112A | AB1110 | current88/AB1710, immediate low16 index, mode0, ratio1 |
| AB27FB | AB27F0 | AA7930, base rotate then actual owner recomposition |
| AB2802 | AB27F0 | AB2600, live filter query |
| AB2819 | AB27F0 | current8C/AB10D0, conditional current-state rebuild |
| AA793B | AA7930 | AA7220, existing same-owner transform publication |
| AB10E0 | AB10D0 | current80/AB3CB0, existing retained geometry |
| AB17A3 | AB1710 | current80/AB3CB0, existing state selection rebuild |

Earlier `GUI_ICON_RUNTIME.md` attributed AutoRotate to AB6430 and rejected
nonzero rates. AB1150 establishes its actual frame consumer, so this rejection
is removed. `GUI_ICON_WIDGET.md`'s unidentified +128/+12C pair is the temporary
state countdown and expiry state established above. These timing fields are
independent of `DelayedTextureLoad`: that separately unsupported AB6430 path
remains explicitly rejected. No delayed texture-loading claim is made here.

## Verification and remaining gates

The changed production source compiled with MSVC Win32 `/std:c++17 /EHsc /W4
/WX /O2 /fp:strict`. One ignored, manifested numerical fixture included the
actual production kernels and passed an unspilled-product cancellation witness
((1+2^-23)*(1-2^-23)-1 gives -2^-46), expiry equality, NaN rate/timer/delta,
signed-zero preservation, negative original delta and infinity subtraction.
It uses no geometry callbacks. It does not exercise the complete retained Icon
runtime or a native function-byte differential. There are no permanent tests.
`verify_report_calls.py` passed 13 numeric rows with zero failures; six indirect
targets are separately grounded in the actual table cells, and two symbolic
texture slots retain their existing resource-service contracts.

The parent is responsible for the combined `scripts/build.ps1`, canonical
base-plus-tail composition and existing checks. Complete real resource-backed
Icon geometry/filter callbacks, whole-frame mutation/exception behavior,
rendering and game execution remain integration validation gates. Required
texture callbacks preserve their borrowed state storage and FP environment.
Unmasked floating-point exception timing, native SEH/OOM, arbitrary owner/list
mutation and physical object/vtable ABI compatibility are not claimed.
