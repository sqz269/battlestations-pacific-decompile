# Hull-list sentinel and header constructor

These complete helpers allocate the existing `10h` hull-list sentinel and
initialize an existing raw `0Ch` list header. Correct established STL helper
names are retained as descriptive hypotheses, not recovered symbols. No hull
handle construction, payload initialization or outer manager class is invented.

| Native entry | Inclusive byte body | Original/Source ABI | Coverage |
| --- | --- | --- | --- |
| `STL_List_BuyHeadNode_0041bf50` | `0041BF50..0041BF69`, 26 bytes | cdecl no arguments/receiver; EAX head; plain RET | complete |
| `STL_List_Construct_0041E1B0` | `0041E1B0..0041E1C5`, 22 bytes | ECX raw header; EAX same header; ESI preserved; no stack arguments; plain RET | complete |

Public Source APIs are `buy_native_hull_list_head_0041bf50()` returning the
existing `Cc13AvoidZoneHullListNode*`, and
`construct_native_hull_list_0041e1b0(void*)` returning the same raw header.
The first is cdecl, the second one-argument Win32 fastcall. Both are naked
kernels; current CRT/provider-frame/global binding qualifications still apply.

## Actual storage and complete schedules

The existing `Cc13AvoidZoneHullListNode` in `cc13_avoid1.hpp` supplies a `10h`
record: next `+0h`, previous `+4h`, and the existing two-word hull handle at
`+8h/+Ch`. Size/offset assertions reconcile the producer with that schema.
Only the two link words are written here. The payload words are untouched,
uninitialized and unread: no `new Node{}`, payload constructor, zero-fill,
default value or test of those uninitialized bytes is added.

The head's exact instruction schedule is:

1. PUSH `10h`, then `0041BF52 CALL 00BF681B`; ADD ESP,4 at `0041BF57` proves
   the one cdecl allocation argument.
2. TEST EAX and, if nonzero, write next=self at `0041BF5E`.
3. Compute ECX=EAX+4 with DWORD wrap; TEST ECX and, if nonzero, write
   previous=self at `0041BF67`.
4. RET at `0041BF69`, with the original allocation still in EAX.

The second guard is on the computed slot address; it is not a blanket
"allocation failed, return null" policy. The Source kernel retains each
register operation, guard and store. The genuine provider returns valid raw
nonnull storage or applies its existing retry/throw policy; no alternate
provider returning malformed/wrapped storage is installed.

The header constructor retains its exact schedule:
PUSH ESI; capture ECX; `0041E1B3 CALL 0041BF50`; write header+4 with returned
EAX at `0041E1B8`; zero count header+8 at `0041E1BB`; return the original
header through EAX at `0041E1C2`; POP ESI and RET. Header+0 is untouched.
No header store precedes allocation, so an allocator exception does not perform
these header writes; provider side effects are not rolled back.

The raw header is an existing writable `0Ch` extent, not a replacement list
class or newly constructed manager. The borrowed original header pointer is
retained across allocation in ESI. No extra null guard, cleanup, payload
lifetime or exception frame is added.

## Genuine provider and all native callers

The head redirects its sole external call to a concrete noinline cdecl adapter
which invokes `singleton_lifetime_allocate({object,bytes,bytes})`. Its actual
captured byte count is `16`, so native/source request sizes are `16/16`.
The existing provider and native `00BF681B` body were read in this worker
stream: malloc, new-handler retry, then `std::bad_alloc` on exhausted allocation.
The source allocation matches the existing real `singleton_lifetime_free`
service. No fake allocation callback, forced success or new failure policy is
introduced. Current CRT heap/handler/exception identity, provider private frames,
fault/SEH addresses, original object binding and gameplay are not transplanted.

All live head references were inspected:

* `0042153E CALL 0041BF50` in manager construction `00421500`: returned EAX
  is stored at manager+70h, followed by count zero at manager+74h. The list
  header starts at manager+6Ch, consistent with the existing schema.
* The other reference is the owned standalone header constructor, verified in
  its complete schedule above.

No live references were found to standalone `0041E1B0`. The Source API is
available for the parallel manager-lifetime implementation; that consumer and
its shared wiring are outside this packet's ownership.

## Prototype corrections, verification and transfer

Old saved prototypes were
`undefined STL_List_BuyHeadNode_0041bf50(void)` and
`undefined STL_List_Construct_0041E1B0(void)`. Their concrete result/receiver
contracts above follow from full assembly and actual result stores, rather
than the untyped snapshot declarations. Existing library names and earlier
constructor ledger evidence remain intact. Primary owns saved Ghidra
prototype/comment annotation and refreshed exports; the worker makes no writes.

Strict MSVC x86 `/std:c++17 /EHsc /MD /W4 /WX /fp:strict /permissive- /O2 /Gy`
compilation passed without warnings. Actual COFF comparison passed all 48
owned bytes, with only the two verified CALL rel32 operands masked: head+3h
to the concrete allocation adapter, constructor+4h to the actual Source head.
Normalized hashes are head
`fe87e1226c574ab0e6bc4d89e898d921e38bc676411fb859d8891e932f7e17b8` and
constructor `587bc090c93254b62a3c5df1a92f85bc4597d6fcf1f6d0ea7aac7b44255baa4c`.
Ignored `local/goal15e_hull_head/compare_hull_head_bytes.py` accepts the primary
registered COFF object as optional argv1 and reuses captured live Native bytes.

One tiny real-heap Source constructor/head probe passed self-links, header
proxy/count/result and matching free; it does not read or require zero payload
words. Artifacts are `local/goal15e_hull_head/hull_head_probe.cpp` and
`probe.cmd` in that directory. It uses the real provider, `/MD` and an embedded
manifest. All three report rows and three unique markdown call claims passed.
No forced OOM, new test framework/target,
Native differential, original ABI equivalence or gameplay validation is claimed.
Primary registers `src/native_hull_list_head.cpp`, integrates and builds with
the real archive. The lease is released on final transfer.
