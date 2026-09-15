# Native shadow pointer vector

Addresses: **00AE1440..00AE149E (95 bytes)** and **00AE19B0..00AE19FF (80 bytes)**.
This packet supplies both complete normal bodies, totaling **175 native bytes**.
The descriptive pointer-vector names are hypotheses about opaque DWORD storage;
they do not identify an owning node type or reconstruct an enclosing caller.

## Actual storage and ABI

Both native entries take the actual three-DWORD header in ECX: data at +0,
signed count at +4, signed capacity at +8. The original public stack word is a
signed requested count/capacity, consumed by `RET 4`. The source APIs use naked
Win32 `__fastcall` with an unused EDX argument so that the third C++ parameter
occupies that same public stack slot. Neither body rewrites the incoming word.
The caller supplies valid header and backing storage for every reached access.
Elements remain opaque words; these routines do not establish their ownership.

## Complete reserve and resize behavior

Reserve captures the request in EDI and clamps that register to a signed minimum
of four. It compares current capacity, then computes the allocation byte count
with the original DWORD `EDI*4` wrap. It tests current count after allocation.
The forward copy tests each computed destination for null, reloads current data
before the source load, and compares the cursor with a fresh count each time.
After copying, it captures current old data and frees it. Only after free returns
and the outgoing stack word is removed does it publish new data, then capacity.
Reserve leaves count untouched and does not initialize uncopied words.

Resize captures the signed public request and calls reserve only when it exceeds
current capacity. It then captures current count for growth. Each growth
iteration reloads data, forms the DWORD address, skips only a computed null, and
stores zero. Shrink repeatedly writes the decremented actual count and compares
it with the captured request. Every normal path ends with the final requested
count store. Nongrowth does not alter data/capacity or free/destroy elements.
Signed comparisons, wrapped arithmetic and abnormal-input paths remain literal.
The six-byte `8D 9B 00 00 00 00` LEA at AE19CA and `8B FF` MOV at AE19EE are
explicit byte emissions. Both returns explicitly consume four bytes.

## Concrete allocation boundary

The private cdecl allocation adapter passes the already-wrapped DWORD to
`singleton_lifetime_allocate({SingletonAllocationKind::object, wrapped, wrapped})`.
That existing provider uses real `malloc`, retries through `_callnewh`, and throws
`bad_alloc` when the handler declines. The free site calls the existing
`singleton_lifetime_free`, which uses matching `free`. No host array owner,
replacement allocator, overflow check, zero-size fallback or callback stub is
introduced. The adapter receives no additional native body credit.

The original BF55BE/BF681B/BF6989 CRT boundaries are represented by the established
source CRT domain. Original allocator hooks, incidental provider register values,
OOM/fault behavior and native exception-unwind equivalence are not established.
The routines add no EH frame or rollback. Allocation failure exits before their
publication stores; provider side effects are not rolled back. Current memory
reads and free-before-publication order are retained across the concrete calls.

## Evidence and validation

Source starts at `d854e0f04072cc7ccd455af91639394fb5a16fdf`. Accepted EO evidence
and the primary review pin the complete bodies and concrete providers. Fresh
BSP queries use `C:/Users/sqz269/bsp.gpr` as the configured target and verify
project name `bsp`, program `/battlestationspacific.exe`, language and image base
before each endpoint call. They confirm 38/31 instructions and zero listing gaps.
All 175 live bytes match the original PE SHA256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The primary repaired AE148C's old CALL_RETURN override and recreated the complete
reserve body before this packet; this worker makes no Ghidra changes.

The report retains three internal direct-call rows and five external caller
rows. Reserve's external callers double current capacity and select signed
minimum four. All three observed resize callers pass zero; AE1C56 follows its
separate node-release loop, AE1C65 precedes backing free, and AE1D91 uses the
owner+44 header during destruction. Those enclosing bodies gain no credit.

The final coupled EP+EQ source `4d0a0cbd5a27bceec34d39576228dc2b309397e6` passes the Win32 build and both existing math CTests. All 2,647 tracked inputs plus the actual verified seed remained unchanged with clean exact HEAD before and after. No new tests or runtime fixtures were added.

The complete 95-byte reserve and 80-byte resize match their native bodies after only three CALL-rel32 operands. Both RET4 instructions and exact alignment bytes are preserved. The full 58-byte allocation adapter passes kind 3 and the same wrapped DWORD as both native/host request sizes to the actual existing CRT provider. Primary and independent reviews cover all three code sections,89 instructions, actual call relocations and exact current-library membership.

The first parallel build stopped on CMake generate.stamp timestamp restoration with Access is denied before tests. The full attempt is archived. An otherwise identical local wrapper using explicit --parallel 1 passed; all four libraries and both complete packet objects remained byte-identical. Normal file ACL/attributes and multiple shared-stamp project rules support a contention hypothesis; the denying handle is unproven. No tracked build-script/source or permission changes were made.

Both descriptive Ghidra names, original native prototypes and evidence comments are saved with previous values preserved and exports refreshed. All eight direct report call rows pass. The final report identifies the sealed actual-input/output/native/generated archive. Static source CRT compatibility, runtime behavior and gameplay validation remain separate.

AE15E0/AE0A50, actual node-release closure in AE1C20, AD7A30 and the larger shadow
update remain incomplete. This packet provides no parent or missing-child body
credit, and makes no renderer, game, concurrent-mutation or fault validation claim.
