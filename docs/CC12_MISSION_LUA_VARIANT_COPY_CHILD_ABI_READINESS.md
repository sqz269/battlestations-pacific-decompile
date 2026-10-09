# Raw mission Lua variant copy child: 00886C10

The complete `00886C10..00886D80` body is 369 bytes / 114 instructions. With
entry ECX as destination D and the captured stack DWORD as source Q, identity
returns immediately after local exception-frame setup. Otherwise the function
calls `00886920`, then reads the current source tag at `Q+4`, stores it at `D+4`,
and selects one of seven cases. The selected cases include x87 float transfer,
byte/word/DWORD transfer, two allocations with different follow-up schedules,
and a child call with ECX=D and no pushed argument. Every normal exit returns D
in EAX with `RET 4`.

**Source closure remains held.** Incoming EAX is overwritten before any owned
use. No owned instruction consumes incoming EDX before defining it, but the
first unresolved child receives incoming EDX unchanged. Neither the accepted
callers nor the generic child prototype proves independence from that register.
Four direct children have no current address-bound Source record; the first
child's full ABI and lifetime contract is the first small next dependency.

This packet adds only this document and its JSON report. No Source, CMake,
ledger, Ghidra, probe, test, build, Original execution or reconstruction credit
is added. The descriptive word "copy" is provisional.

## Scope, complete bytes and target

- Baseline: `34ad122c6cb3894c962821550711613a021c1966`.
- Owner/packet: `agent/cc12_lua_variant_copy_child_abi` /
  `cc12_lua_variant_copy_child_abi_readiness`.
- Primary explicitly admitted the entire 369-byte body beyond the predecessor
  packet's 300-byte gate. No prefix was accepted as a complete function.
- The primary separately admitted exactly 28 local switch-data bytes at
  `00886D84..00886D9F`; these are data, excluded from the 369 function bytes.
  The three padding bytes `00886D81..00886D83` were not inspected.
- CLI queries reused the configured existing `C:/Users/sqz269/bsp.gpr` and
  verified project `bsp`, program `/battlestationspacific.exe`, language
  `x86:LE:32:default`, and image base `00400000` before each query. The configured
  project file exists. Live and snapshot function counts both remain 64,729.
- All 369 live bytes equal the installed PE at file offset `00486C10h`.
  Independent Capstone 5.0.7 decoding consumes the entire span, reaches the
  inclusive end `00886D80`, and exactly matches all 114 Ghidra instruction starts.
  There are no listing gaps or omitted bytes after calls. Metadata has 16 blocks,
  18 edges and seven direct calls to six distinct children.
- Whole-body SHA-256:
  `054dbc13c176b2c13f71f7cd7524900cafb55be2764ebba5360a1231925f8ec1`.
- Installed PE SHA-256:
  `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
- The eight actual `RET 4` sites are `00886C69`, `00886C84`, `00886CC1`,
  `00886CDB`, `00886CF6`, `00886D13`, `00886D2E`, and `00886D7E`.
  No no-return flag was queried or changed; no flow/listing repair was needed.
- Children received metadata queries only. No Native child, caller, handler,
  profile, string, RTTI or adjacent function body was read. Captures use the
  shared export directory resolved by `tools/workspace.py`, without a junction.

The seven little-endian table targets are all instruction starts inside the
owned body. All 28 live table bytes equal the installed PE, with SHA-256
`805970604922d282b9f3b13821ca02c27ad4fcfe3cb95f6043056bf26ea5a97e`.

## Entry, frame and register contract

Let S be entry ESP. At `00886C10`, EAX becomes the old `FS:[0]` head. The function
pushes state -1, handler `00C97416`, that old head, incoming ESI and incoming EDI.
It captures Q from `[S+4]` at `00886C27` and D from ECX at `00886C2B`, in EDI and
ESI respectively. `CMP ESI,EDI` controls the identity bypass. The source DWORD
is captured once; later allocation cases overwrite its original argument slot.

| Entry-relative address | Owned role |
| --- | --- |
| `S-4` | Unwind state, initially `FFFFFFFFh`. |
| `S-8` | Handler word `00C97416h`. |
| `S-12` | Saved FS head; local record installed into `FS:[0]`. |
| `S-16` | Saved incoming ESI. |
| `S-20` | Saved incoming EDI; steady body ESP. |
| `S+4` | Initial source DWORD; overwritten by allocation result in tags 1 and 5. |

At the first child call `00886C35 -> 00886920`, ECX=D, ESI=D, EDI=Q, EAX is the
old FS head, EDX is incoming EDX, and no argument DWORD is pushed. EBX and EBP
are untouched. CMP has already replaced incoming arithmetic flags. The child's
use of EDX, EAX, EDI or other delivered machine state is unopened; a declared
`void` prototype does not close its ABI. Returning compatibly with this caller
requires the child to preserve live ESI/EDI and balance its zero-argument stack.

Owned instructions never read incoming EAX. Incoming EDX is passed through but
not locally consumed; the tag-4 path first defines EDX at `00886D16`, then stores
that new value. EBX and EBP have no owned reads or writes, but transitive child
requirements remain outside this result. The body saves/restores ESI and EDI;
whole-call nonvolatile preservation still requires compatible children.
Even if the first child were later shown to ignore EDX, a value it preserves
could reach another child; each reached boundary still needs its own contract.

All normal epilogues restore `FS:[0]`, incoming EDI and ESI, discard the three
exception words, return D in EAX, and consume four argument bytes. Identity
performs no receiver/source-pointee access or child call. Two equal zero words
therefore take that local bypass; this is not a general null-input guarantee.
Restoration reads the current saved frame slots. If aliased stores or callees
overwrite them, original register/FS values and normal frame balance are not
guaranteed; no frame-alias exclusion is enforced by this body.

The accepted wrapper `00886DA0` supplies D in ECX, late-loads `[wrapper S+4]`
into EAX and also pushes it, while passing EDX unchanged. This child's first
instruction discards that delivered EAX copy and `00886C27` independently reads
the pushed word. The accepted reserve caller `006B87F0` supplied its source in
EDX as well, but the wrapper's earlier destination stores may change aliased
storage or its argument slot. Equality of the eventual stack source and EDX
must not be inferred for arbitrary aliasing. Neither caller body was reopened.

## Full branch and child schedule

After a nonidentity comparison, `00886920` is called before any owned tag or
payload access. Only after its normal return does `00886C3A` load current
`[Q+4]` into EAX. `00886C40` stores that DWORD to `D+4`, then unsigned `JA` at
`00886C43` sends any tag above 6 to the common return. Negative signed DWORDs
also take that default path. This preserves the newly stored tag and performs
no further local payload store. A tag in 0..6 indexes the admitted local table.

| Tag | Target | Exact owned operation after tag publication |
| --- | --- | --- |
| 0 | `00886C6C` | `FLD m32real [Q+8]`; set EAX=D; `FSTP m32real [D+8]`; return. |
| 1 | `00886C87` | Allocate 8 bytes through `00BF681B`; clean four bytes; store allocation A into `[S+4]`; set state 0. If A is zero, store zero at `D+8` and return. Otherwise load current `[Q+8]` into EDI, push it, set ECX=A, call `00426060`, store its EAX result at `D+8`, and return. |
| 2 | `00886CDE` | Read one byte at `Q+8`, write one byte at `D+8`, return. |
| 3 | `00886CF9` | Read two bytes at `Q+8`, write two bytes at `D+8`, return. |
| 4 | `00886D16` | Read DWORD at `Q+8` into EDX, write it to `D+8`, return. |
| 5 | `00886D31` | Allocate 12 bytes through `00BF681B`; clean four bytes; store A in `[S+4]`; set state 1. If A is nonzero call `006EEAF0` with ECX=A and no pushed argument; otherwise set EAX=0. Store result R at `D+8`, then read current `[Q+8]`, push that DWORD, set ECX=R, reset state to -1, and call `00781F20`; common return follows. |
| 6 | `00886C50` | Set ECX=D, call `0074FB30` with no pushed argument, return. No owned source-payload read follows the tag read. |

There are two allocation call sites (`00886C89`, `00886D33`), and one each for
`00886920` (`00886C35`), `0074FB30` (`00886C52`), `00426060` (`00886CA7`),
`006EEAF0` (`00886D4D`), and `00781F20` (`00886D67`). The allocation calls have
explicit caller cleanup. The one-argument calls require four-byte callee cleanup
for the observed frame to balance; the zero-argument calls require plain-return
balance. Those are caller requirements where child ABI is unresolved.

Tag 0 really is x87 load/store, not a DWORD MOV. It uses the current x87 state
without saving, initializing or changing the control word. Successful FLD raises
stack depth by one and FSTP lowers it again; the body has no unconditional x87
reset or exception clearing. Bit preservation for all encodings, exceptional
floating values, full x87 status/stack equivalence and fault parity are not
established by the pseudocode's apparent four-byte assignment. No float probe
or exceptional-value execution was performed.

## Footprint, aliasing, failure and lifetime

The complete **owned** receiver-relative footprint is a DWORD store at `D+4`
on every nonidentity path after the first child returns, plus case-dependent
stores at `D+8` of width 1, 2 or 4. These cover at most `D+4..D+Bh`. There is no
owned receiver-relative read, or direct access to `D+0..3`, `D+Ch..D+13h`, or
anything beyond. This is not the total footprint: children can read/write the
receiver, source, allocation, global or aliased storage. It does not establish
a 12-byte complete class or allocation size. The accepted reserve's 20-byte
pitch and wrapper's four preliminary stores remain separate caller evidence.

The owned source-pointee reads are current `[Q+4]` after the first child, and
current `[Q+8]` on the payload-reading cases. Width is 4 for tag 0, 1, 4 and 5;
1 for tag 2; and 2 for tag 3. Tags 6 and default do not locally read `Q+8`.
The string branch replaces EDI with the loaded payload word only after allocation;
the original Q pointer is otherwise retained, subject to compatible callees.
No source-relative write is encoded, but destination and stack writes can alias it.

The ordered effects are essential:

1. Identity is tested once, before child effects. The child can change the
   contents of D or Q before the tag load. The loaded tag is then captured in
   EAX and stored before payload access; it is not reread after later children.
2. An overlap can make the tag store change the later source-payload read.
   Tags 2 and 3 only replace one or two payload bytes locally; other bytes retain
   the values left by earlier effects, not necessarily their entry values.
3. Both allocations can mutate aliased source storage before payload load.
   They overwrite `[S+4]` with A before that payload load, while EDI retains Q.
   A reconstruction must not reload Q from the now-overwritten argument slot.
4. Tag 1 reads the current source-payload word after allocation, then passes it
   to the string constructor. The final `D+8` store occurs after the constructor
   returns, and stores its EAX result rather than an independently cached A.
5. Tag 5 stores R at `D+8` before loading `[Q+8]`. Overlap can make the pushed
   source word equal a newly published value. It passes R in ECX even if R is
   zero; the zero-allocation branch still makes the `00781F20` call. Tag 1's
   zero-allocation branch instead returns after publishing zero.

There is no general null, overlap, alignment, extent or payload-validity check.
If either distinct pointer is invalid, first-child behavior precedes the owned
tag read/store, so the exact first failure cannot be assigned without that child.
A tag above 6 is preserved, not rejected or normalized. Allocation sizes are
constant 8 and 12; the receiver's full allocation requirement is not derived
from them. No owned reference-count update or direct deallocation is encoded.

The initial child may retire old payload state, but that interpretation is
provisional. The body establishes only that it runs before copying the tag.
It does not prove deep copy, move, reference retention, safe self/overlap copy,
rollback, leak freedom or strong exception guarantees.

The handler word `00C97416` was not opened. State is -1 through the first child,
tag store and allocator call; 0 is installed after the tag-1 allocation returns,
and 1 after the tag-5 allocation returns. Both store A at `[S+4]` before the state
change. Tag 1 keeps state 0 through normal epilogue. Tag 5 keeps state 1 through
construction, pointer publication and source-word reload, then writes -1 at
`00886D5F` before its final child call. Normal FS restoration bypasses any need
to interpret those states. Exceptional cleanup, allocation ownership, SEH/C++
unwind and second-failure behavior remain unproved; generic RAII must not be
substituted for that missing contract.

## Current Source and smallest next packets

Exact address searches and targeted reconstruction records find no current
Source for `00886C10`, `00886920`, `0074FB30`, `006EEAF0` or `00781F20`.
The admitted initializer `008849B0` remains a separate five-DWORD leaf, including
`+8=0`; it cannot replace this branch/child operation or establish a callable
profile. Its current source and admission receipt are pinned.

Two existing Source facilities are relevant but do not close this packet:

- `00426060` has an actual-header string-copy implementation and an accepted
  historical report describing ECX destination, stack source, EAX destination,
  `RET 4`, and no local EH. Current source and header are pinned. Their bytes
  differ from the original report's recorded artifact hashes; the current raw
  pool overload uses `memmove`, while the storage overload uses `memcpy`.
  This packet neither replays the old fixture against the current files nor
  claims new Native ABI or pool/EH closure. No Native string body was reopened.
- `00BF681B` has the admitted typed `singleton_lifetime_allocate` host CRT
  service (malloc/new-handler retry/throw). Its typed request and host allocation
  do not establish an Original binary allocator binding. The owned caller's
  explicit null branches are retained as observed code, without asserting
  their reachability under every allocator contract.

| Next body | Metadata bytes / instructions / calls | Bounded question |
| --- | --- | --- |
| `00886920` | 168 / 46 / 5 | First priority: incoming EDX/EAX/EDI use, receiver footprint, lifetime effects, full normal return and handler boundary before the source-tag load. |
| `0074FB30` | 17 / 6 / 1 | Tag-6 call with ECX=D and no pushed DWORD; exact effects and its one child. |
| `006EEAF0` | 43 / 15 / 1 | Tag-5 12-byte construction, result pointer, initialization and its one child. |
| `00781F20` | 54 / 25 / 2 | Tag-5 pushed source word, ECX=result, null behavior, reload/alias schedule and cleanup ABI. |

All four bodies are below 300 bytes by metadata, but each needs a fresh lease,
complete bytes/extent check and bounded child admission. Their small size is
not proof that their dependency graphs are closed. Keep handler `00C97416`,
the wrapper handler, actual callable profiles and raw owner/backing integration
as distinct future scopes. Register ABI and x87 work retains Astra routing.

The JSON report embeds every decoded instruction and table target, child
metadata, exact call/footprint/EH contracts, historical pin replay, current Git
blob and physical SHA-256 domains, and the document hash. Validation here is
static complete-byte/metadata/Source evidence only. Startup and gameplay remain
unproved.
