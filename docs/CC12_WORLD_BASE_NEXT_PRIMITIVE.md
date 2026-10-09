# World base next primitive (CC12)

The next concrete storage edge is the **World-specific root and 97-header
profile**, not another header constructor or a full World constructor. Main
`0b2eede10` has the actual header initializer, raw clear and destructor callback,
plus the matrix-sentinel producer and post-base chain-header allocation helper.
The existing general Source iterators still reject the exact World callback
profile. A dedicated two-function composition is now **IMPLEMENTED_UNBUILT**
after the integrator accepted its bounded scope. Header13's separate Source
admission remains pending. No new original-function or accepted-fragment
credit, build acceptance, full World ownership or gameplay claim is added.

## Exact missing edge

The complete `004CB030` constructor is 126 bytes /39 instructions. At
`004CB057..004CB05F` it zeroes the root DWORDs at World+0C/+10/+14. Its
`004CB076` call to `00BF7CD1` then passes actual World+18, stride0C, count61h=97,
constructor `004B7EC0`, destructor `004C2D30`. The final array element starts at
World+498 and ends at +4A4. The total root-plus-array writable span is
`[World+0C,World+4A4)`, 1176 bytes /98 contiguous 12-byte headers.

Current `construct_native_game_array_00bf7cd1` in
`src/native_game_array_elements.cpp` supports only the vector10, participant118
and list0C callback pairs used by `004DDB90`; the World pair is rejected before
operation state changes. `destroy_native_game_array_00bf7c6e` in
`src/native_game_array_lifetime.cpp` similarly rejects destructor `004C2D30`.
Their recorded-failure state is diagnostic and does not reproduce native
reverse-unwind cleanup. Extending either by merely accepting another address
pair would not supply the missing lifetime contract.

The actual initializer Source writes three zero DWORDs at offsets0/4/8, using
the literal 13-byte native leaf and an explicit unused EDX register formal.
The actual destructor Source is the five-byte tail to raw clear70. Clear70
unlinks and canonically frees owned nodes while preserving the 12-byte header
and borrowed payloads; it is not an empty callback. These are physically
different from `construct_world_object_004cb030` in `src/world_construct.cpp`,
which returns layout metadata and sentinel token1 without constructing an
actual World or any embedded header.

## Current dependency admission

Current tracked reports explicitly record clear70 Source1 and callback5
Source1, with their independent validation and named dependency holds cleared.
Header13 remains Source0 with `complete_linked_helper_validation_pending=true`.
The recent `Main/local/h13p4/Root_linked_manual_gate_receipt.json` is accepted
for **one fresh fixture's launch admission only**, and explicitly retains
Source0 until capture/provider reading, final bookends, sealing and separate
Source publication. This is not evidence of final Header13 admission.

The r03 independent peer audit is historical static evidence from a terminally
sealed family. It neither accepts r04 nor supersedes the current Header13 hold.
The peer owns that work; no peer Source, fixture/probe, AF3160 address or
admission metadata is changed here. The current Header13 hold is not generalized
into a rollback of the separately accepted clear70/callback5 contracts.

## Implemented bounded composition

The new files are
[`native_world_parent_headers.hpp`](../include/bsp/native_world_parent_headers.hpp)
and [`native_world_parent_headers.cpp`](../src/native_world_parent_headers.cpp),
with normal C++ actual-storage interfaces:

- `initialize_native_world_parent_headers` initializes the actual root at
  World+0C through header13, then initializes all
  97 actual category headers at World+18+i*0C in ascending order through the
  same physical Source leaf.
- `clear_native_world_parent_headers` clears the same 97 actual headers in
  descending order through callback5,
  then clear the actual root at World+0C through callback5.

Both functions take only the actual borrowed World pointer and are noexcept.
They assert Win32 pointer width, the exact stride, adjacency and array endpoint
at compile time. Both pass an explicit unused EDX value0 to the existing raw
callbacks. No new assembly or allocation service is added.

This is a fixed storage profile, with no dynamic callback facade, projected
owner, table token, replacement node allocator or full-World destructor. It
adds an unbuilt constructor-storage/cleanup fragment, not another
original-function reconstruction. Source admission remains qualified by the
current Header13 dependency and the integrator's own build decision.

The cleanup order comes from the actual World EH map: state1 -> state0 invokes
`00C6563B` (97 elements, stride0C, `004C2D30`, through `00BF7C6E`); state0 -> -1
invokes `00C65630` on World+0C. The complete native vector constructor77B22 walks
forward; the destructor75B20 subtracts the stride before each reverse callback.
The fixed Source callbacks are noexcept. On valid, exclusively supplied fresh
storage, initialization itself has no allocating or throwing callee. A future
sentinel-allocation exception can therefore clear the completed array and root
through real callbacks. This does not reconstruct general CRT/FH3/SEH behavior,
partial progress for arbitrary throwing constructors, unexpected destructor
exceptions, or faulting/malformed storage. Cleanup requires all98 headers to be
initialized and coherent; owned nodes must be distinct canonical allocation
bases and must not alias the headers or borrowed payloads, exactly as required
by `native_parent_header_clear.hpp`. Reverse order matters even if the fresh
constructor-failure case contains only empty headers: each actual callback is
still invoked, with no replacement empty teardown. Raw clear's real failure
boundary is retained through its existing noexcept interface; this composition
does not catch failures, store diagnostic progress, or claim to repair them.
The ordinary C++/canonical-free contract requires DF0; the raw Header13 leaf's
separate DF1 tolerance does not broaden this composed interface.

The actual full base constructor still needs a real current World method table
and its class/normal-destruction dependencies. Its write of Native table address
CE7784 cannot become a fabricated Source table. The new composition writes
none of the vptr, chain-header pointers, active/reference fields, sentinel,
matrix count or ready byte and does not establish full World lifetime.

The [report](../reports/cc12_world_base_next_primitive.json) records the exact
Native spans, current Source comparisons and dependency status snapshot.
Exactly the two dedicated Source files and this document/report are changed.
No CMake/config/ledger/Ghidra mutation, build, test, probe, fixture or target
execution has occurred. Root will review before any registration/build; the
current dependent-helper hold is not bypassed by this implementation.
