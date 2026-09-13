# Native resource fallback item

`native_resource_fallback_item.cpp` reconstructs the complete **58-byte**
`B86930/B86990` constructor/scalar packet and supplies a finite `D631C0`
implementation of existing `NativeRefCountedDeleteCalls`. The owner storage
is an eight-byte borrowed prefix; the implementation adds no allocating
producer, reference count, registration or runtime wiring.

| Entry | Complete inclusive native range | Original ABI | Coverage |
| --- | --- | --- | --- |
| Construct `B86930` | `B86930-B86945`, 22 bytes | ECX storage; EAX same; RET | Complete |
| Scalar delete `B86990` | `B86990-B869B3`, 36 bytes | ECX item; stack flags DWORD; EAX same; RET4 | Complete |

Both source entry points have new C++ ABIs. The finite provider is a source
binding, not an original vtable installed into the game. Raw profiles other
than `D631C0` are outside its domain and cause `std::invalid_argument` before
the owner is changed. That binding error is a source policy, not recovered
native behavior.

The [report](../reports/native_resource_fallback_item.json) records the
PE/live-matched ranges, original registers and call sites, current source
dependencies, test scope and retained evidence. Every live batch verified
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, bridge8089.
The installed PE SHA256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

## Writes and deletion

Construction writes `CEB130` at00, count1 at04, then `D631C0` at00. The source
uses volatile DWORD stores in that order. EAX captures native incomingECX
before any store; the source likewise returns the original supplied pointer.
Only those eight bytes are written.

The scalar captures the original receiver, writes `D5C104` at00, calls the
existing concrete `destroy_native_ref_counted_base_00bd30f0`, and conditionally
calls `singleton_lifetime_free` when flags bit0 is set. That existing service
is the `BF65AC` CRT-free boundary. The native free call at `B869A6` returns
through `ADD ESP,4` at `B869AB`; the scalar returns the original pointer even
after free. Count04 and neighboring bytes remain untouched. There is no
scalar null test, decrement, payload cleanup, rollback or local FH3 frame.

The existing `BD30E0` source checks for a null owner, captures its current
profile and asks the provider for slot04(flags1). This packet maps the exact
captured `D631C0` identity to the concrete scalar. It does not reread another
profile, perform another decrement or supply a generic destructor callback.
`BD30F0` and `BD30E0` source files were reused unchanged.

## Producer evidence and limits

The sole live direct constructor call is `B7EA81` in `B7E970`. It follows
`PUSH8; CALL BF681B; ADD ESP,4` at `B7EA68..B7EA6F`; ECX receives EAX at
`B7EA7F`. The constructed pointer is captured in ESI, survives payload skip,
and is pushed into the current resource's virtual0C at `B7EAA7`. No local
item release follows that append. This confirms the storage extent and
initial profile; this packet does not implement that producer or append.

The matched nine-slot table is `D631C0..D631E3`. Its first two entries are
`BD30E0/B86990`. The remaining entries and their type-token initialization
are outside this provider, as are general item-base helpers `B868B0/B86890`.
The thirteen other registered parser-result profiles, `B88430`, manager/cache,
array helpers and hierarchy storage remain separate contracts. The hierarchy
record's raw parent DWORD is not this item vtable.

The native dispatch FH3 map frees its allocation only during construction,
then protects its child handle. It does not provide constructed-item rollback
around skip/append. This source adds neither that producer nor new unwind
behavior. No native exception-unwind, allocator-runtime, full item interface,
drop-in binary compatibility or gameplay claim follows from the two bodies.

## Verification and retained proof

One ignored capsule was frozen against the original bytes before production
edits. It records construction, six flag patterns, nonzero sentinel counts,
neighbor guards, final-delete dispatch and a null invoker. Only the two native
scalar CALL displacements are relocated: one to the original seven-byte base
body and one to a shared test-only free observer. Native invoker dispatch uses
a relocated two-entry table; production retains the original profile identity.
The original input source, bytes, object, executable and nine output rows are
hashed in `local/resource-fallback-bf/original-baseline-frozen.json`.

The source comparison uses the actual current fallback and refbase archive
members and an identical free observer. It also checks that an unsupported
profile is rejected before mutation. The observer records the free schedule,
owner identity and current fields; it does not exercise the real CRT free or
prove an actual use-after-free lifetime. Original outcomes are not regenerated
to match source. No tracked tests or additional framework were added.

The strict MSVC Win32 build and both existing CTests passed, as did the
current/copied capsule comparison. The first full build hit an unrelated
CMake `generate.stamp` timestamp-access error; the unchanged-source retry
passed. That failed log is retained. The final commit is gated again with
`MSBUILDDISABLENODEREUSE=1`. Exact-commit logs, current and copied object/archive provenance,
compiler include/command closure, linker maps, source/header bytes and probe
outcomes are retained under `local/resource-fallback-bf/`. The final
`delivery.json` records results and commit identity; `whole-local-manifest.json`
hashes the entire worktree `local/` except itself. The earlier BF discovery
worktree and its475-file manifest are preserved separately and unchanged.

The evidence establishes the complete source bodies and finite provider with
the documented free-service substitution. No game, UI, Ghidra mutation,
export refresh or runtime owner integration was performed by this packet.

## BF integration correction, 2026-09-13

The isolated BF integration on 2026-09-13 defined and range-verified all eleven previously missing functions, including `CD82D0-CD82E5`, against the installed PE and live Ghidra bytes. The seven nondeleting leaves, token startup entry and two compiler exception handlers remain analysis-only. Evidence is retained in `reports/native_resource_lifetime_bf_function_definitions.json`. The local `B7D68C` call override was cleared and its nine-byte fall-through gap decoded; the stored `B7D640-B7D69E` body now covers all95 bytes with zero remaining call gaps. Existing full-function documentation was archived before and after repair. The repair changes analysis metadata, not the installed game. See `reports/native_resource_lifetime_bf_flow_repairs.json`.

The unchanged report now passes all6 direct-call rows with zero failures. Historical missing-function flags and worker-stage limitations above describe the retained original observations. New source comprises nine complete bodies (494 original bytes) across the three BF source packets; pool wrappers remain build-only. Combined candidate validation is recorded separately; this correction does not claim gameplay validation.
