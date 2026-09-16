# Raw runtime particle parameters

This packet connects the complete AFBF60 conversion closure to the application's
actual F8D344 pool and fixed source CRT segment storage. The existing legacy
68-byte binding, builder raw context, and public legacy entry signatures remain
compatible. Runtime definitions move to `native_particle_parameter_runtime_loading.cpp`;
there is one coefficient kernel and one of each integration kernel.

## Storage and public contract

`NativeParticleParameterRuntimeRawContext` contains the same actual
`NativeWeakHandlePool&` consumed by B00090 returns, and four live numeric pointer
members: CF1450 tangent scale (double), D5DCA0 default slope (float), D7A280
integration half (double), and D7A348 integration quarter (double). No strings,
CRT square-root access, host owner adapter, or allocation callback is needed by
this closure. Raw segment allocation/free use the existing fixed
`singleton_lifetime_allocate/free` CRT providers.

The builder remains the caller-owned 10h image. Runtime payloads occupy the
first 0Ch bytes of actual 10h pool slots: multiplier +0, payload/segment pointer
+4, byte count +8, byte capacity +9, and type word +A. Construction clears +4,
+8, +9 and +A; multiplier +0 and physical pool slab index +C remain untouched.
Hermite rows are 1Ch bytes; Linear rows are 14h bytes. Const stores its value in
+4. The caller owns the resulting slot and retains ownership of the builder.

| Body | Complete bytes | Original ABI |
|---|---:|---|
| AFBF60–AFC1A6 | 583 | ECX builder; EAX runtime slot/null; RET |
| AFBF20–AFBF51 | 50 | ECX builder; EDX retained owner internally; RET |
| AFF9B0–AFF9C6 | 23 | ECX payload; EAX same payload; RET |
| AFFD70–AFFDE0 | 113 | ECX payload; stack capacity low byte; RET4 |
| B000A0–B00113 | 116 | ECX payload; stack Hermite row; RET4 |
| B00120–B00194 | 117 | ECX payload; stack Linear row; RET4 |
| B004A0–B004A9 | 10 | ignores incoming ECX size; ECX=F8D344; tail 9242F0 |
| AFB3A0–AFB543 | 420 | ECX first key; stack next key; RET4 |
| AFFCB0–AFFD19 | 106 | ECX payload; stack time; RET4/ST0 |
| AFFD20–AFFD6F | 80 | ECX payload; stack time; RET4/ST0 |

## Numeric and ownership behavior

The private numeric view holds addresses of the original context's pointer
members. Assembly dereferences those cells at each native constant load. It
does not snapshot their values or construct legacy bindings for raw callers.
All address-tagged instructions in the moved coefficient/integration kernels
match their prior source. The binding adds integer loads only.

Public integration entries create that view with integer instructions, retain
the previous EAX/EDX behavior, and preserve caller-owned x87 values. They add no
floating-point return spill. Append retains the native FLD32/FSTP32 time
argument, integration call, and FSTP32 result. Linear slope and Const use explicit
native x87 stores without an extra C++ float-return spill.

Conversion reloads current builder counts/records at the loop boundaries and
mutates Hermite coefficients in place. Segment capacity is truncated to a byte;
conversion does not cap the loop to that byte. Allocation captures type/stride,
frees a captured nonnull pointer, clears current +4, then reloads current
capacity before allocating. Append uses captured count for its initial
destination, ordered payload copies, then current count/pointer for its final
accumulated field and count increment. Unknown type abandons its newly allocated
slot and returns null. No transactional rollback or malformed-input policy is
added.

AFBF60 uses handler CBB168 and descriptor DF30F0. DF30E8 state 0 invokes CBB160
→ B00090 only while constructing the allocated payload. The state is disarmed
after payload construction and the kind load, before coefficient/segment work.
The source guard uses the genuine pool-return provider, and is `noexcept` so a
secondary C++ exception during true unwind terminates. The current payload
constructor contains only stores and cannot throw a C++ exception. Later
allocation failure retains the native partial slot/key effects.

## Evidence and validation

The report preserves all ten complete live/PE spans and hashes (1,618 bytes),
exact calls including the EH tail, four numeric cells, FH3 bytes/maps, prior
annotations/ledger records, and before/after source hashes. AFBF60's unlisted
AFBFE8–AFBFEF bytes follow an unconditional jump to AFBFF0; this is unreachable
alignment, not an omitted executing tail. No Ghidra writes or flow repairs were
needed. Correct existing names and all prior ledger evidence are retained.

The ignored `local/parameter_runtime_probe` fixture executes 21 copied original
spans and the rebuilt library against genuine raw string and parameter pools.
Its 64 observations cover four x87 rounding modes and two numeric constant sets:
Const subnormal/NaN, sorted Linear, Hermite, zero-segment Linear/Hermite, unknown
kind, and small Linear values. Checks compare mutated coefficients, complete
segments, conversion x87 exception flags/TOP, physical slot identity, the full
904h parameter slab/freelist, and the 9,098,372-byte raw string owner image before
its critical section. Only the separate segment allocation address is normalized
in slab comparison; segment bytes are compared independently. Direct integration
checks preserve a lower-stack sentinel and EAX, and compare values/status at
three times.

Because shared definitions moved, the archived m13 raw builder 40-case and
legacy 14-case original/source probes were rebuilt and rerun. Both pass. Strict
Win32 Release compilation and all three repository CTests pass. No new permanent
tests were added. The new source was appended under a short CMake lease, released
before building.

These are source and bounded original-code fixture results. Copied native FH3
transport, SEH equivalence, malformed overflow/invalid-pointer execution and
gameplay are unvalidated. The interfaces are borrowed C++ composition APIs, not
drop-in replacements at the original machine addresses.
