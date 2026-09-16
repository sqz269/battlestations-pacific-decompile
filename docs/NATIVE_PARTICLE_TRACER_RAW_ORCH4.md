# Raw Tracer particle definition loading

Addresses: 00B0AD50, 00B0A920.

| Entry | Original span | ABI | Coverage |
|---|---|---|---|
| B0AD50 parser | B0AD50..B0B5CB, 2172 bytes | ECX actual E8h definition, stack actual 1Ch TextBuffer; RET4/AL true | complete |
| B0A920 texture list | B0A920..B0AB8A, 619 bytes | ECX definition, stack filename; RET4 | complete |

These raw overloads use `NativeParticleTracerRawContext`. The existing host-binding
interfaces remain separate. Both bodies, both FH3 function-info structures and
unwind maps, and all eleven cleanup actions match the installed PE and live BSP
program: nineteen spans, 3,059 bytes. All 104 original call sites are recorded.
The B0ADBD listing gap is the three-byte alignment instruction `8D 49 00` after
an unconditional jump. No listing mutation was required.

## Shared services and retained invocation

The parser borrows the actual raw string publication/gate/manager cells, shared
F8C2C8 line scratch, existing builder constants/CRT, actual F8D344 runtime pool,
current F8C26C atlas publication, and B015C0/B00980 raw contexts. It introduces
no string adapter, callback allocator, replacement atlas or converted item.
Constructor-established layouts and all 24 property offsets are documented in
[the earlier Object/Tracer analysis](NATIVE_PARTICLE_OBJECT_TRACER_LOADING.md).

`NativeParticleTracerRawAcquired` is a caller-retained, single-use invocation.
Its actual four-byte temporary headers outlive its deque of B015C0 child frames.
Every common-property invocation gets a fresh child; a failed child's nested
texture/cache obligations remain available after parent cleanup. Destruction adds
no rollback or reference release. The caller must retain the frame until those
existing obligations are resolved. Source metadata allocation and provider-domain
validation are explicit C++ boundaries rather than native instruction behavior.

The builder's unwritten initial kind is supplied by the caller and persists across
lines. No valid kind is substituted after an ignored parser failure. Native parser
locals otherwise become initialized at their producing operations.

## Parsing, arithmetic and publication

Entry clears only definition byte +64. Opening-brace search is followed by another
line read even after EOF; a closing brace leaves subsequent input unread. Unknown
lines and property names still follow their native parsing/allocation schedule.
The source calls raw B015C0 first, then the derived branches; numeric branches
construct the genuine builder, append zero endpoints, ignore AFC470's Boolean
result, and call raw B00980 before the Tracer-specific numeric mapping.

`atof` returns through x87 and spills to float at the original boundary. The common
parameter argument is loaded and spilled again. First-value properties use the
x87 result/store. Runtime conversion precedes the live percentage-cell lookup and
`FLD32 / FMUL64 / FSTP32`; the scale is not narrowed to float. `TileLength` publishes
the converted pointer without touching its multiplier. Existing runtime pointers
are overwritten without release.

B0A920 resets only +9C. A zero frame count performs one atlas lookup; positive
counts append contiguous borrowed items, stopping on a miss. Each lookup reloads
the actual manager publication. The +98/+9C/+A0 descriptor grows by unsigned
`2*capacity+2` only when larger. Capacity is published before the fixed CRT
allocation; multiplying by four saturates to FFFFFFFF on overflow. Copies reread
the descriptor/count, the old allocation is freed before pointer publication, and
the final append rereads current fields. Reset preserves capacity and stale cells.

## Ownership and exception evidence

B0AD50's DF3D08 map is: 0 line→-1; 1 common suffix→0; 2 name→0;
3 head→2; 4 tail→2; 5 tracer texture→2; 6 scalar suffix→2; 7 builder→2;
8 outer suffix→7; 9 inner suffix→8. Keyword, Boolean-value and numeric-value
tokens have no parent unwind state. Normal inline returns retain their captured
pointer and clear only where the body does. The final line header remains stale
after its normal return. The builder destructor runs while state seven remains
installed, then normal name cleanup selects state zero.

B0A920 owns only the successfully constructed frame-name header (DF3C58 state
zero). It disarms that state before normal return. True C++ unwind follows the
recovered map and terminates on a second cleanup exception; original FH3/SEH
transport and unrestricted constructor/cleanup failures remain unvalidated.

## Validation

A clean strict MSVC Win32 build and both existing CTests passed. An ignored,
manifested Win32 probe executes both complete copied bodies and compares two
cohesive sequences against the linked library, each observing 9,105,313 bytes:
complete definition/TextBuffer/items, initialized string arena/ring/counters,
initialized runtime slab and produced segments, shared scratch, initialized
texture-array cells, borrowed identity, and x87 status. Only pointer allocation
addresses are normalized; opaque Windows critical sections and uninitialized
array capacity are excluded.

The sequences cover all 24 Tracer keys, common Stops/Shader, pointer overwrites,
Const/Linear/Hermite builders, ignored malformed syntax, texture growth/reset,
brace/EOF, and a signaling-NaN scale. A separate source-only missing-profile
failure checks retained child state and parent cleanup. Both copied bodies call
existing source implementations for seventeen helper targets, fixed CRT allocation
and free bridges, and three current CRT functions. Those dependencies were not
executed as original code by this probe. FH3 handler references fail the probe if
invoked; they were never invoked. No permanent tests were added.

The report preserves the prior host-overload ledger records and Ghidra comments,
and records hashes, calls, EH evidence, comparison scope and all ten checklist
rules: `reports/native_particle_tracer_raw_orch4.json`. New C++ interfaces and
descriptive names do not establish binary substitution or gameplay correctness.
