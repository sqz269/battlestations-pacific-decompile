# Preset iterator equality

Whole `00699EF0..00699F18` is41B/18 instructions, retaining the established
`STL_inst_00699ef0` library identity. The Source name is descriptive and
provisional. This packet owns only this ordinary entry.

| Entry | Coverage |
| --- | --- |
| 00699EF0 | Complete41B/18 instructions |

Native ECX points to the actual left8B iterator; EDX is unused. One stacked
right-iterator pointer is consumed by RET4. Owner/node offsets0/4 reuse the
published real `NativeInputPresetTreeIterator` carrier, without a node overlay
or a container substitute. Native AL reports equality; the upper24 EAX bits
come from left.node and are incidental. The Source bool API exposes AL only.

Capture left.owner before conditionally reading right.owner. A zero left
owner short-circuits that right-owner access; otherwise compare cached left
owner to CURRENT right.owner. On failure invoke the actual published
diagnostic, which may return or throw. After a normal return read CURRENT
left.node before CURRENT right.node and compare them. No owner revalidation,
early return, new pointer guard, callback, pool, context, repair or catch is
added. A thrown diagnostic prevents subsequent node reads.

| Scope | Native site and target |
| --- | --- |
| Owned diagnostic | 00699f02 CALL 00bf6713 |
| Readonly producer successor | 006a1750 CALL 0069c0f0 |
| Readonly producer | 006a175e CALL 00699ef0 |

The producer forms two actual private8B owner/node iterators, passes the
right pointer on the stack and left in ECX, then tests AL at006A1763. Native
stack-argument load timing and Source private argument/frame aliases remain
qualified. The old AV helpers and map-index/hint/predecessor Source are not
changed or promoted.

The complete PE/live/body/current-start proof, accepted provider hashes,
actual CMake COFF, build result and remaining limits are in the companion
report. Original CRT/handler selection, FH3/SEH/RTTI, faults, heap, register
residue, binary ABI and game equivalence are not established. Valid reached
aligned carrier storage is required through the diagnostic's return. The
published diagnostic uses current SDK policy and promises neither noexcept
nor noreturn. No new tests, probes or Native execution are added.

Default MSVC Win32 build and all three existing suites pass in7.58seconds.
Those suites do not exercise this new iterator entry. The complete actual
CMake object contains58B/28 instructions: public44B/21 and an emitted unused
raw-read helper14B/7. Its sole REL32 binds the published diagnostic, and the
public entry returns with RET4 without local EH or code padding. A zero
left owner jumps past the right-owner load; after a returning diagnostic
left.node is read before right.node. The compiler loads the right argument
before the owner load, unlike Native; private argument timing and full EAX
are explicitly outside the Source contract. Frozen-pin primary review is
pending; preliminary primary12 checks passed without findings.
