# Unit numbering update: virtual query ABI readiness

008761E0..0087622B is **Source-held**. Its complete 76 bytes / 28 operations
are understood, and two advertised slot +5C providers have current exact byte
evidence. The actual production receiver/profile and executable current-target
binding remain unresolved. This read-only packet adds zero Source, build,
original-ABI, startup or gameplay credit.

## Exact update contract

The Native entry receives R in ECX and one raw number DWORD at entry ESP+4,
preserves ESI/EDI and returns with RET4. There is no EH setup or stable
return-value contract. Every operation of the pinned Root record was read.

1. Capture the number in EDI and R in ESI. Equal R+35C returns immediately
   without a model read or store.
2. Test **pre-store** R+360; store the captured number at R+35C without changing
   flags; return if the pre-store model was null.
3. Read current [R] into EAX and current [EAX+5C] into EDX; push6 and CALL EDX
   at 00876206. ECX remains R; ESI/EDI hold receiver/captured number.
4. Test **AL only**. On zero, reread current vtable and slot target, push1B,
   explicitly restore ECX=R, and call fresh EDX at 00876215. Again test AL only.
5. On either true result reread **current** R+360, push captured EDI, and call
   actual 00711BE0. There is no second null guard or number recapture.

The number store precedes every slot read. No evidence permits assuming
nonaliasing or hoisting the second slot fetch, late model read or receiver+C4
fallback across unresolved dispatch. The two selected leaves are pure; their
purity does not establish a production receiver/profile domain or universal
dispatch purity. Invalid storage, arbitrary aliases, faults, concurrency and
structural reentry remain outside this readiness evidence.

## Bounded actual query providers

| Advertised profile / actual slot cell | Exact target range | Complete body | Fixed words before late actual R+C4 |
| --- | --- | --- | --- |
| 00D09678 / 00D096D4 | 006DFE50..006DFE85 | 54 bytes / 19 operations | 6,5,4,2,1,0 |
| 00CFF3F8 / 00CFF454 | 006F5890..006F58C5 | 54 bytes / 19 operations | 1B,5,4,2,1,0 |

Both actual slot cells match the Original PE and live Ghidra. Both complete
bodies match PE/live bytes and every saved/live instruction start. Their first
instruction overwrites EAX from [ESP+4]. **EDX is neither read nor written**.
ECX is unchanged and used only by the final fallback CMP EAX,[ECX+C4].
ESI/EDI/EBX/EBP are untouched; both exits return full EAX 0/1 with RET4. There
are no calls, writes or globals. Fixed matches avoid the receiver read, and
incoming flags are not consumed.

These are target-local facts. Query6/1B does not select the receiver's dynamic
target, prove either profile, or prove the other 86 catalogued bodies. No other
Native body was reviewed.

## Existing Source providers

The exact naked 006DFE50 leaf exists at src/native_ship_kind.cpp:7. Its header
explicitly leaves whole-class/native virtual routing, constructors and lifetime
admission unbound. The other eight entries were not revalidated here. No actual
006F5890 Source leaf was found by the bounded searches.

The unit_kind_query table and pure functions take supplied class ids. They do
not obtain the current actual slot target. Its dynamic-id helper also takes an
already supplied id; it does not itself preserve the original late R+C4 read.
NativeUnitPartAttachmentAccess::query_kind_5c is a borrowed callback requirement;
its declarations and uses are not a concrete production provider.

The full public numbering header and complete relevant 00711BE0 Source
body at src/native_model_numbering.cpp:286 were inspected: model+160 holder,
holder+0C node, then the existing node-numbering provider. Existing ledger status
is complete_supported_normal_caller_original_loop_and_real_d3d9_runtime_fixture_tested.
That historical status is retained, with no runtime-suite replay. Its new C++
interface adds actual numbering services and operation; it is not a raw Native ABI
replacement. The same real string/renderer/layout/stream/atlas/type/storage domains,
real node_virtual0c dispatch, operation failure rules and lifetimes remain required.

## Current evidence and limits

Every live question used bsp.py ghidra, verifying project bsp, program
/battlestationspacific.exe, x86 language and image base before querying. Config
selects C:/Users/sqz269/bsp.gpr, whose exact identity is pinned in the Root gate.
Function count remains 64729. Original image SHA-256: b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6.
Root capture SHA-256: b381b76eb8966f0479ab5f0532986cab8f005c79547ab97a8db9a06615d5516d.
Selected wrapper SHA-256: 28efa0cd1b6d198e789096fa4465491c591750b1ebcc5284498997cb9241b906.

The Source113 context was checked against
reports/cc12_native_tick_subnode_reparent_primary_review.json: all **113 Root raw
inputs and four artifact hashes/sizes match**, and all 113 worker inputs match
after LF normalization. The sole raw worker difference is CRLF-only in
reports/cc12_pending_registry_tick_registration_constructor_readiness.json.
The pinned primary report records three passing existing checks, 33 captured/replayed
whole objects and 37 positive Core definitions. These are inherited primary context,
not a worker build or fresh object recapture. Source113 excludes the separately
inspected numbering/kind/attachment providers; their current pins and Root/worker
LF equality are recorded separately.

The explicit leases covered 008761E0, two complete 54-byte query targets and two 4-byte
slot cells. No Ghidra, C++, CMake, ledger or GPR mutation, build, test, probe,
caller-body expansion or runtime review occurred. Other workers' Native entries
and the Native numbering hierarchy were not analyzed.

## Independent bounded leaf prerequisite

The complete 006F5890 body is ready for a separately leased direct Source-leaf
packet: ECX is the same actual receiver, EDX is unused, the raw query DWORD is
at entry ESP+4, the fixed 1B/5/4/2/1/0 comparisons precede one late actual+C4
read, EAX returns 0/1, and both exits use RET4. It can follow the existing
ship-kind entry pattern with genuine stable aligned receiver storage through
+C8 for the fallback. No callback, dynamic classifier, copied id or base
substitution is needed for this exact leaf.

Completing that leaf would still leave the production receiver/profile,
current-target dispatch and numbering-service binding below unresolved.

## Smallest concrete production prerequisite

Evidence and bind one actual production receiver/profile path into 008761E0,
including genuine R+0,+C4,+35C,+360 storage and lifetime. Sole indexed caller 0088FE30
is an unreviewed lead for a separately leased packet. Bind the **freshly captured
current target** for that admitted profile to its exact executable Source provider,
preserving receiver, stacked query, AL result and callee cleanup. If the profile is
proven to be 00D09678, reuse existing 006DFE50 Source; query6 alone is insufficient.
An admitted 006F5890 target needs its own actual Source leaf first.

Bind the same current model and inherited 00711BE0 services/operation, then submit
the 76-byte Source candidate with both fresh slot reads and late model read intact.
A guessed bool(__thiscall*) cast, table copy, cached id, declared callback or cast
of numeric Native table DATA does not satisfy this prerequisite. Other profiles
need exact providers only when admitted; reconstructing all 88 is not required.

Machine-readable report:
reports/cc12_native_unit_numbering_update_virtual_query_readiness.json
SHA-256: 810c0f3ce466f5d46c294b20d57dc6ebafd7de55e11b978185f013cc05b4522d.
