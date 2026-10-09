# MCommandBuilding kind predicate Source candidate

The complete 006F58E0..006F591A predicate is reconstructed as one direct naked
MSVC Win32 Source leaf. It is an **uncompiled candidate awaiting Root admission**:
this packet adds zero accepted Source, build, Original-ABI, startup or gameplay
credit. The existing provisional name BSP_MCommandBuilding_IsKindOf is retained.

## Complete Native gate

Only this address was opened for Native analysis in this packet. All 59 bytes /
21 operations match the Original PE and live Ghidra; every saved/live instruction
start matches the complete PE decode. Both returns are within the reviewed body:
false at 006F5910, true at 006F5918, with end exclusive 006F591B. Original body
SHA-256 is e114eda484401d63249b769c0ea91d93b99d264ed57dc1b970deb4114471bbb6.

Every live question used bsp.py ghidra, which verifies project bsp, program
/battlestationspacific.exe, x86 language and image base before querying. Local
configuration selects C:/Users/sqz269/bsp.gpr. Original image SHA-256 matches the
already pinned Root Native gate; function count remains 64729. No Ghidra mutation,
new function, vtable-cell analysis, caller analysis or sibling-body expansion occurred.
Full bytes, all 21 operations, file offset, live commands and saved-listing pin
are preserved in reports/cc12_native_mcommandbuilding_kind_source.json.

## Exact entry and behavior

ECX holds the same actual receiver. One raw query DWORD is at entry ESP+4.
The initial MOV overwrites incoming EAX; EDX is neither read nor written.
The function compares the query against 1C,1B,5,4,2,1,0 in that exact order,
then performs exactly one late CMP EAX,[ECX+C4] if all fixed words failed.
Every match reaches MOV EAX,1 / RET4. Failure reaches XOR EAX,EAX / RET4.
It returns full EAX 0 or 1; a caller may consume only AL.

ECX, EDX, EBX, EBP, ESI and EDI remain unchanged. Incoming flags are not consumed:
each branch uses its local comparison. There are no calls, globals, writes, x87
operations, EH setup, allocation, callbacks or copied class-id caches. Fixed
matches avoid the receiver read entirely. The full raw DWORD is compared;
there is no sign conversion, boolean query projection or base-leaf substitution.

## Actual Source interface and storage

The public entry is native_mcommandbuilding_is_kind_006f58e0 in
include/bsp/native_mcommandbuilding_kind.hpp and
src/native_mcommandbuilding_kind.cpp. It follows the existing native_ship_kind
API convention: uint32 return, fastcall receiver in ECX, explicit unused EDX,
third argument as the stacked uint32 query, and a naked body containing the
complete original instruction sequence. A Win32 pointer-size assertion prevents
accidentally admitting this entry as a 64-bit implementation.

An ordinary caller supplies a stable nonnull actual receiver with readable
backing through +C8 and a genuine aligned uint32_t at the actual +C4 offset.
When constructing this borrowed view over raw storage, establish that DWORD's
lifetime in place. A copied id passed through another object does not satisfy
the actual-storage contract. Fixed paths do not read the receiver; that fact
does not establish constructor/profile/ownership/lifetime admission.

The header and CPP were reviewed completely against all 21 original operations.
There is no eager receiver read, null guard, wrapper, dispatcher or consumer.
Actual emitted bytes, branch encoding, symbol retention and complete COFF review
remain for Root's normal MSVC Win32 build. No compiled equivalence is claimed here.

## Source117 context and timestamp boundary

Both reports/cc12_native_tick_subnode_requeue_primary_review.json and
reports/cc12_native_return_one_byte_primary_review.json pin the Root build at
2026-10-09T20:20:30.934115Z through 20:20:47.377222Z. Current replay verifies all
117 raw Root inputs and four artifact hashes/sizes, plus all 117 worker inputs
after LF normalization. The sole worker raw difference is CRLF-only in
reports/cc12_pending_registry_tick_registration_constructor_readiness.json.

Those frozen reports record three existing checks passed, 35 captured/replayed
whole objects and 39 positive Core roots. These are inherited Source117 evidence,
not a build or object recapture of this candidate. The new header/CPP/doc have
their own exact pins in the candidate report and are absent from Source117's
input list. Existing ship-kind header/CPP template pins are unchanged from the
previous readiness review; their actual selected Source convention was reused.

Artifact equality is a timestamped observation of the referenced Root directory.
A later Root build may replace those artifacts; it does not retroactively compile
this candidate. No historical Source113 or Source109 artifact set is presented
as the current Source117 result.

## Integration and remaining boundary

Root owns CMake registration, the normal build, whole emitted-object review,
Source admission, ledger changes, Ghidra annotation and integration. This worker
changes only the four leased files and adds no build, test or probe.

Completing this direct predicate does not identify the actual receiver/profile
at 008761E0, install its current-token virtual dispatcher, prove a class constructor
or supply model-numbering services. Native profile addresses remain numeric DATA.
The separate 006F5890 worker's leaf and other class predicates are not replaced
or credited. Invalid placement, faults, concurrent mutation, structural reentry,
private EH and game binding remain unproved.
