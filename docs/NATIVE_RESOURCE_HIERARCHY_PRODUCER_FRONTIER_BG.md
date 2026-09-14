# Native hierarchy producer frontier BG

`B7EB90` is **not ready for a faithful raw source implementation**. The pool,
name storage, numeric reserve and comparison bodies already exist; BG append
has source on an unmerged branch. The remaining obstacle is the actual node
handle and stream-reader contract, not the already established 84h payload.
The typed `StructuredNode`/`HierarchyItem` parser cannot fill that gap: it uses
`unique_ptr`, `std::string`, readiness checks, full-read rejection and early exits.

The [report](../reports/native_resource_hierarchy_producer_frontier_bg.json)
maps all 27 direct producer calls to concrete source or a named missing raw body,
with stack word counts and register provenance for every call. The only live
incoming call is `B7F165` inside `B7F100`: ECX is its saved manager, and its one
stack argument is the current child wrapper. Producer `RET4` consumes it.
All queries used `bsp.py ghidra`, whose client verifies
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, bridge8089 before querying.
No Ghidra annotations, exports, prototypes, functions or saved analysis changed.

## Ready source and missing raw contracts

| Native entry | Actual source status and required contract |
| --- | --- |
| B87A90 | Existing `allocate_static_native_hierarchy_slot_00b87a90()` selects the bound actual109022C companion; native ECX84h is discarded. Startup/binding remains explicit. |
| B7D640 | Existing `reserve_native_hierarchy_reference_array_00b7d640(void*,int32_t)` accepts actual0Ch header and signed capacity. |
| 41DD40 | Existing raw-header resize overload takes `NativeStringRawPoolContext`; actual name header, length and preserve=true. |
| 419CC0 / BD1510 | Existing raw getter and return bodies consume actual publications01090AA8/01090AA0 and return gate01090AA4. Getter has **zero native arguments**; three words queued at B7ED93..99 belong to BD1510, whose RET0C consumes them. |
| 425850 | Existing `equal_native_string_header_00425850(const void*,const char*)` consumes child node+10h; RET4. |
| BF7FBF / BF7680 | Existing CRT boundaries for case-insensitive comparison and copy. Original copy contains overlap handling; original CRT/locale/exception identity remains outside this discovery. |
| B87AE0 | `append_native_hierarchy_item_00b87ae0(void*,uint32_t)` exists in **unmerged** commit351572b8, outside base dbae27fb/frozen root b538f53e. It consumes ECX=[manager+24], record stack word, and appends through resource+1Ch. |
| 715BF0 | Only typed source exists. Raw body is independently ready: valid wrapper; null contained node returns false; otherwise node+20h !=0. No attachment or sign check. |
| BEA680 | Missing actual24h child allocation/constructor path through BEA250, including native counted tag and path storage. Typed allocation checks/readiness/unique ownership differ. |
| BE9A00 | Missing raw DWORD wrapper through BF0280, actual stream virtual34, nonnull actual-count pointer and unchecked budget debit. No numeric lookup or conversion occurs. |
| BEA010 | Missing actual8h result header via BE9FE0/BF0510 and stream virtual48. `read_string(std::string&)` is not this output contract. |
| B936E0 / B932E0 / B93310 | Missing raw ECX-wrapper/EDX-destination bodies:16/4/6 sequential BE99D0 reads, each FSTP32; no early failure exit. BE99D0 calls BF02C0, stream virtual44, float temporary round-trip and actual-count debit. |
| B7D220 | Typed x87 `sphere_to_box_00b7d160` exists, but raw wrapper does not. Native ECX output, stack sphere, RET4; B7D160 writes a temporary and wrapper copies six values through x87. |
| BE9C40 | Missing raw detach: parent budget debit, BF03E0 stream virtual1C relative seek, zero remaining, decrement reader+60h path index, clear node+8. No duplicate-detach guard. |
| BE9ED0 | Missing actual intrusive handle release: InterlockedDecrement(node+4), call node virtual0 at zero, then clear wrapper. Typed unique ownership does not supply the same lifecycle. |

The report records the exact blocking addresses and distinguishes inspected
transitive paths from unresolved concrete stream/vtable and deleting-destructor
bodies. No fabricated callbacks, globals or host-node representation are proposed.
Raw XML/node-handle terminology must not erase the observed binary reader:
wrapper points to a24h node with native tag header+10/+14, reader+8 and
remaining+20. This producer never performs a host XML lookup.

## Producer details that matter for the next source packet

Parent at+0 begins FFFFFFFF; name+4/+8 and numeric header+4C/+50/+54 begin zero.
The matrix area is unwritten. Existing BE payload/default details still apply.
Resource values are raw DWORDs captured in EDI before reserve. The inline append
uses capacity+4 with signed minimum8, reloads current count/data after reserve,
skips only a null computed destination, then increments current count. It never
calls B7D9D0, looks up a resource, or retains a resource-item pointer.

Matrix/sphere/box call sites explicitly load EDX with record+0C/+5C/+6C and ECX
with the current child wrapper. Sphere conversion writes a separate box temporary,
then six x87 loads/stores copy it into record+6C. Duplicate fields preserve native
ordering: scalar/name/matrix fields overwrite; Resource appends; sphere recomputes
box while an explicit box only replaces box. Final B7EE78 passes the same raw
record to the resource selected by a fresh manager+24 load.

FH3 info DFB168 and map DFB158 have exactly two actions: state1 to0 cleans the
temporary name through CC1FF8/41DD20; state0 to-1 releases the current child
wrapper through CC1FF0/BE9ED0. Name cleanup is armed after the string reader
returns and disarmed before normal return-block cleanup. Child cleanup is disarmed
before normal release. No map action destroys the record or returns its slot.
Unmerged cleanup4876b34b implements B88320 elsewhere; it is not producer rollback.
C++ cleanup structure alone would not establish native FH3/SEH compatibility.

## Minimum next packet and verification

A concrete independent source packet can own only715BF0 plus
`include/bsp/native_structured_node_predicate.hpp` and
`src/native_structured_node_predicate.cpp`, exposing
`bool native_structured_node_has_remaining_00715bf0(const void*) noexcept`.
It loads the actual wrapper and node+20 directly with the null-node branch above.
It needs no allocator, stream callback or invented owner. This is a proposal;
this discovery worker leased onlyB7EB90 and its two new evidence files.

A full B7EB90 packet should own its new producer header/source only after the
raw wrapper/stream/name/lifecycle dependencies in the report have real source
contracts and reviewed BG append is integrated. The report does not call those
missing dependencies ready merely because their functions have descriptive names.

The complete inclusive B7EB90-B7EE90 interval is769 bytes. Live and installed PE
SHA-256 both equal `3d29d9f744a2052f8886ff8e64278ee79670a6fa6faf90500b51c2165be3b5c4`.
EH handler, map/info and action bytes also match the PE. The installed executable
was hashed twice: `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The report retains the complete local evidence inventory, two hash passes,
query commands/exit statuses and failure history. CC2000 has no Ghidra function;
a read-only bytes query supplied its ten bytes after disassembly failed.

This is discovery only: no source implementation, compilation, test/probe,
original EH execution, native ABI replacement or game/runtime validation.
`verify_report_calls.py` passed all28 rows (27 producer calls plus one incoming), with0 failures.
