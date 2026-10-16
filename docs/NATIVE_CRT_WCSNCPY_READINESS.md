# Native CRT `_wcsncpy` readiness

Addresses: `00BFAEEF`–`00BFAF2B` (inclusive).

**Source ready: false. Source authorized: false. Source credit: 0.**
This bounded packet stops for specialist review of incoming direction flag and ES
padding state. It adds readiness evidence only. The entire Original body is 61
bytes / 31 instructions, SHA-256
`9789205272f4c057c2bc61d248b3465e80f51678077db71845ac3abb10ff9293`.
Installed PE bytes, live bytes, the complete listing and all 31 typed instruction
owners/default/effective flows agree. The body has no call, listing gap, thunk,
no-return override or flow repair. The captured project is `bsp`, marker
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`.

| Coverage | Result |
| --- | --- |
| Whole Original body | Complete static read; no unread body range |
| Ordinary replacement | Blocked pending specialist DF/ES contract |
| Source/compiler/Main/runtime | None |

The typed metadata shows a three-argument cdecl `wchar_t*` signature. That label
does not establish genuine wide-object lifetimes, aligned storage, SDK identity,
or a physical ABI bridge. Only one indexed caller label was inspected:
`00770E50 BSP_WideString_Substring`; its body and callsite were not read.

## Captured homes and zero count

The count DWORD is read once from original ESP+0C before either pointer home.
After two register saves, the destination DWORD home at original ESP+04 is
captured in EDI and ESI. The zero-count branch follows those operations. It skips
the source home at original ESP+08 and all payload reads/writes, then returns the
captured destination through EAX. Null source/destination values need no pointed
storage on that branch. The native physical argument/return stack still exists.
The source home is read once only for positive count. No pointer/count home is
reread in the copy or padding loop.

## WORD copy and overlap

Each copy reads one WORD through DS using MOVZX, writes that captured low WORD
through DS, advances both cursors by two with two INC instructions apiece, and
then tests the captured WORD for zero. DF does not control these INC operations.
A nonzero WORD decrements count and loops while count remains. Exhausting count
does not read an extra source WORD. A copied zero branches before the loop count
decrement; the later DEC accounts for the already copied zero.

No disjointness assumption is recovered. The read occurs before its paired
write. Actual overlapping bytes can be changed by earlier writes and affect
later source WORD reads, even for a one-byte offset. A later source NUL is not
required if count exhausts first. Once zero is encountered, there are no further
source payload reads; padding can still overwrite aliased source storage.

## Exact padding handoff

Let c include the copied zero WORD, r=count-c, and P be EDI immediately after that
copy. r=0 returns before both REP instructions. Otherwise XOR EAX,EAX sets the
padding value to zero; SHR ECX,1 yields q=floor(r/2) and carry t=r&1.

`00BFAF20 REP STOSD ES:EDI` performs q **DWORD** stores. Its initial EDI offset is
P. With s=+1 for DF=0 or s=-1 for DF=1, the store offsets are P+s*4*j for
j=0..q-1, and ending EDI is P+s*4*q. REP consumes ECX to zero and does not change
the SHR carry. ADC ECX,ECX therefore makes ECX=t.

`00BFAF24 REP STOSW ES:EDI` begins at P+s*4*q and performs t **WORD** stores.
These are ES destinations; ordinary copy MOV stores use DS. There is no CLD or
STD anywhere in the body. Incoming DF/ES are not established, and no equal
DS/ES interpretation is proved. These offset facts do not recover the general
physical segmentation, frame alias or fault contract. A forward scalar WORD
padding loop is not authorized as an equivalent whole-routine mechanism.

The captured destination in ESI is returned through EAX on every normal path.
No allocation, ownership transfer, provider or hidden call occurs in this body.

## Specialist boundary and future gates

Root must first decide the incoming DF/ES boundary with the specialist. Only
after that could an ordinary raw-storage interface be considered. Its contract
would need the exact ordered WORD copy, actual DWORD/WORD padding widths, genuine
live reached readable/writable storage, nonwrapping cursor progression within or
one-past actual allocations, no one-past read, unaligned/interior storage and
overlap including self-induced later-read changes. Do not invent wchar_t objects,
positive count, nonoverlap, an INT_MAX cap, a full-NUL source extent or ownership.

Private addressed MOVZX16/MOV16 operations and separately assessed padding are
possible future mechanisms, not accepted Source here. External mutation,
concurrency, fault/MMIO/lifetime changes and physical callee frame/home/saved
register alias protocols remain outside any prospective ordinary domain. Caller
stack payload must not be conflated with those native private-frame protocols.
No flags/register/segment/stack/fault ABI, SDK/Unicode/caller/startup/game/runtime
claim is made.

Verification follows `docs/WORKER_VERIFICATION_CHECKLIST.md`: boundary before
attribution, complete Original coverage, and register provenance from the whole
listing. The report-call checker is mechanical only; its zero call rows cannot
prove this read/write or DF/ES contract. Local evidence is under
`local/NativeCrtWcsncpyReadiness01`. The initial preservation-only filename
preflight failure is retained with its unchanged tool and corrected pre-pinned
stage; it occurred before function metadata/body queries. No Source, compiler,
build, tests, probes, emulation, target execution or Ghidra mutation was performed.
