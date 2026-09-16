# Raw particle parameter builder

This packet gives the existing builder a genuine raw pooled-text context. It
adds AF4060 and composes the complete eight previously reconstructed bodies with
the application's actual string publications and the fixed source CRT allocator.
The original 68-byte legacy binding and its overloads remain available.

## Native storage and ABI

The caller owns the 10h-byte builder: records pointer at +0, signed count at +4,
signed capacity at +8, and kind at +C. Each key occupies 2Ch bytes. Construction
clears only the first three words and reserves 32 keys; it leaves kind alone.
The two cleanup leaves free the captured records pointer and then clear the
current first three words, again retaining kind.

| Body | Complete span | Original arguments and return |
|---|---|---|
| AFBED0 | AFBED0–AFBF13, 68 bytes | ECX builder; EAX builder; RET |
| AF4110 | AF4110–AF4129, 26 bytes | ECX builder; RET |
| AF4060 | AF4060–AF4079, 26 bytes | ECX vector; RET |
| AFC360 | AFC360–AFC46B, 268 bytes | ECX builder; two stack floats; RET8 |
| AFC470 | AFC470–AFCC9E, 2,095 bytes | ECX builder; stack line header; AL bool; RET4 |
| AFBDB0 | AFBDB0–AFBE74, 197 bytes | ECX builder; stack signed capacity; RET4 |
| AFC260 | AFC260–AFC333, 212 bytes | ECX builder; stack key pointer; EAX index; RET4 |
| AFBC90 | AFBC90–AFBD02, 115 bytes | ECX tangent; two stack floats; RET8 |
| AFBD10 | AFBD10–AFBD82, 115 bytes | ECX tangent; two stack floats; RET8 |

`NativeParticleParameterBuilderRawContext` borrows `NativeStringRawPoolContext`,
the actual `CameraAxesCrtAccess`, and ten native constant pointer cells. It owns
no string facade, parameter pool, allocation callback, or numeric constants.
Runtime parameter conversion, coefficients, and integration remain outside this
raw overload packet. Layer parsing can consume AFC1B0/AFC1C0 directly.

## Shared implementation and ownership

One incoming, outgoing, and endpoint assembly implementation serves both context
types. Its private numeric view holds addresses of the original binding's pointer
members. Each assembly load dereferences that cell at the original load point;
callback-driven pointer rebinding remains observable. The view adds only integer
loads. The existing x87 instructions, spills, register saves, stack arguments,
and call order are retained. The endpoint insertion bridge selects a concrete
legacy or raw context; it does not fabricate a legacy owner.

Reserve and insertion share typed implementations. Raw allocation/free use
`singleton_lifetime_allocate/free`, the existing fixed source CRT providers.
Minimum capacity, saturated multiplication, forward eleven-word copies, current
count/source reloads, sorted insertion, unknown coefficient bytes, and final
publication order remain unchanged.

Parsing shares its control flow with separate typed text policies. Raw token
operations use the real pooled-text overloads, which reload the actual pool
publication through 419CC0. Hermite obtains the second pair token before the
first, converts second then first, and releases first then second. Normal return
disarms a temporary's native state before its explicit release; a failure from
that release can propagate without retrying the disarmed object.

Native FH3 ownership is explicit:

- AFBED0 / DF30BC: state 0 owns the builder vector through CBB140 → AF4060.
- AFBDB0 / DF3090: state 0 owns the replacement allocation through CBB120 → BF6989.
- AFC470 / DF3114: state 0 owns type; state 1 owns incoming-y then type; state 2
  owns outgoing-y then type. Count, ordinary x/y, pair-x, and Const-value tokens
  have no native unwind action. Source guards do not invent one.
- Cleanup guards are `noexcept`; a secondary C++ exception during true unwind
  terminates rather than replacing the active exception.

The current CRT `atof` is an explicit provider boundary. Raw parsing consumes
its ST(0) return directly with FSTP32, preserving the native pair reload/store and
Const FLD32/FSTP32. First-time validation uses UCOMISS and last-time validation
uses FLD64/FLD32/FUCOMIP. Kind and partial vector mutations survive a false parse;
endpoints append, and Const writes the current first key after conversion.

## Evidence and validation

The companion report records all nine complete live/PE spans and hashes, exact
calls, ten numeric cells, prior annotations, and complete FH3 funclet/map spans.
Parent-serialized AF4060 and CBB120 repairs are recorded in
`native_particle_layer_builder_flow_orch4_m13.json` and
`native_particle_layer_builder_bodies_orch4_m13.json`. No live Ghidra writes were
made by this worker. Existing library names and legacy evidence are retained.

The focused ignored probe in `local/parameter_builder_probe` executes eleven
copied original spans and rebuilt source against the same genuine raw string
pool, fixed allocation/free, pooled-text providers, and current CRT. Forty
observations cover four x87 rounding modes, endpoints, Const rounding/underflow,
sorted Linear, Hermite, endpoint failures, NaNs, count failure, and unknown kind.
It compares defined key fields, count/capacity/kind, x87 exception flags and stack
depth, and all 9,098,372 owner bytes before the embedded critical section.
Indeterminate key scratch is excluded; source and original allocations need not
have the same pointer value.

The existing legacy original/source fixture also passes 14 observations with
two constant sets, including conversion and actual parameter-pool state. It
checks compatibility; it does not establish raw runtime conversion. The final
Win32 Release build uses the repository's strict flags and passes all three
CTests. Independent source/evidence review found no issue in numeric-view loads,
stack offsets, x87 ordering, or raw parser state transitions.

The copied-original probe exercises normal native control flow, not FH3 exception
transport. Cleanup states have assembly/map and source-review evidence. Native
SEH ABI, malformed extreme inputs, game integration, and gameplay remain
unvalidated. These borrowed C++ interfaces are not binary replacements.
