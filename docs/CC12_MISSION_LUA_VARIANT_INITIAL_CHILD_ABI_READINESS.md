# Raw mission Lua variant initial child: 00886920

The full `00886920..008869C7` body is 168 bytes and independently decodes to
59 instructions, six calls and two plain returns. It reads the receiver's tag
once. Tags 1 and 5 with a nonzero captured payload take different child/free
schedules; every normal path eventually writes `FFFFFFFFh` to the receiver tag.
Only those two nonzero-payload paths locally clear the receiver payload word.

**Source closure remains held at `006EE960` and integration contracts.** The
owned body discards incoming EAX, uses incoming ECX as the receiver, and only
saves/restores incoming EDI and ESI before repurposing them. It never reads or
writes EDX, so incoming EDX can reach children unchanged. That is pass-through
evidence, not proof of a hidden argument or of transitive independence.

The live listing omits 53 bytes after two `_free` calls. The omitted bytes
include another `_free`, payload/receiver stores, and a normal return. The
pseudocode's early returns are therefore not the complete normal behavior.
No Ghidra repair or mutation was performed. This document/report adds no Source,
CMake, ledger, build, test, probe, Original execution or reconstruction credit.

## Full-body and target evidence

- Published baseline: `ef03278921416a877cb96569f64da20c7b136204`.
- Worker: `agent/cc12_lua_variant_copy_child_abi`; packet:
  `cc12_lua_variant_initial_child_abi_readiness`.
- Metadata was checked before body capture: 168-byte extent, 46 listed
  instructions, 9 blocks, 11 edges and 5 listed calls. The extent is below
  the 300-byte gate; the metadata instruction/call counts are incomplete.
- All 168 live bytes match the installed PE at file offset `00486920h`.
  Capstone 5.0.7 decodes every byte, with 59 instructions and six direct calls
  to four distinct targets. Both actual exits, `00886987` and `008869C7`, are
  plain one-byte `RET`. No local switch or outside-body data read was needed.
- Whole-span SHA-256:
  `0fc4cfcda1e7bce2c2b169965d909e30e68d836732d836d0bbd84fc07be93938`.
- Installed PE SHA-256:
  `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
- Each live CLI query verifies project `bsp`, program
  `/battlestationspacific.exe`, `x86:LE:32:default`, and base `00400000` before
  access. The configured existing `C:/Users/sqz269/bsp.gpr` exists and was reused.
  Live/snapshot counts remain 64,729. No project save or reimport occurred.
- Native children received metadata queries only. No Native child, caller,
  handler, profile, string, RTTI or adjacent body was opened. Raw captures use
  the shared exports resolved by `tools/workspace.py`.

The complete independent decode recovers these omissions; no bytes are invented
from inferred control flow:

| Omitted span | Bytes / instructions | Actual effect |
| --- | --- | --- |
| `0088695D..00886987` | 43 / 11 | Push captured payload; zero its `+4` and `+8`; call `_free` again; clean both free arguments; clear receiver `+8`; write receiver `+4=FFFFFFFFh`; restore registers/local stack; return. |
| `008869B1..008869BA` | 10 / 2 | Clean one free argument and clear receiver `+8`, then fall into the listed final tag store/return. |

Exact no-return flags were not queried or changed. The listing ends after free
calls at `00886958` and `008869AC`; that observation does not establish which
saved analysis property caused the omissions. The raw continuations and both
normal returns are present in matching live and installed bytes.

## Entry machine state and frame

Let S be entry ESP and D entry ECX. The body reserves eight uninitialized local
bytes, saves ESI and EDI, copies D into EDI, and reads `[D+4]` into EAX at
`00886927`. It subtracts one and branches for tag 1; otherwise it subtracts four
more and branches away unless the original tag was 5. These are exact DWORD
equalities, not a range check or a second tag read.

| Machine input | Owned classification |
| --- | --- |
| ECX | Semantic input D; copied to EDI before ECX is reused. |
| EAX | First owned use of its value is after definition from `[D+4]`; incoming EAX is discarded. |
| EDX | No owned read or write anywhere in all 168 bytes; forwarded to the first reached child. Further values depend on child clobbers/preservation. |
| EDI | Incoming value is read by PUSH solely for preservation; then overwritten with D. No owned receiver/source operation uses incoming EDI. |
| ESI | Incoming value is similarly saved, then replaced by `[D+8]` on selected branches. |
| EBX / EBP | Untouched locally. Whole-call preservation depends on child contracts. |
| Arithmetic flags | Entry `SUB ESP,8` defines them before any owned consumption; subsequent decisions use local SUB/TEST results. No incoming carry/flag dependency is encoded. |
| x87 state | No owned x87 instruction, environment access or reset. This does not establish children preserve or ignore it. |

The saved EDI at `S-16` and saved ESI at `S-12` are restored before return.
The local area is `S-8..S-1`, and steady body ESP is `S-16`. There are no public
stack arguments. Both returns restore ESP to S before plain RET. No local FS
exception record, handler, unwind-state store or owned FS access is present.

The body does not specify a uniform EAX result: no-child tag-1/tag-5 null-payload
paths retain zero from tag arithmetic; other tags retain `(tag-5) mod 2^32`.
Paths which call free leave its EAX result unaltered locally. This is not a
receiver-return contract. The accepted parent does not use that result; it next
loads the current source tag into EAX.

The parent `00886C10` previously supplied ECX=D, ESI=D, EDI=captured source Q,
EAX=old FS head and incoming EDX, with no pushed arguments. This child saves Q
in its EDI preservation slot and replaces EDI with D before every child call;
it does not consume Q as an owned source-pointee input. It also discards the old
FS value in EAX. EDX remains unresolved transitively, particularly through
`006EE960`. Incoming saved-register words remain present in the frame; this is
not a claim about an arbitrary child's access beyond its declared arguments.
The parent body was not reopened.

There is a useful conditional narrowing for the accepted outer wrapper:
`00886DA0` previously stores `FFFFFFFFh` at `D+4`. If that word remains intact
until this child's entry, the child takes the no-call path and rewrites the same
tag, without consuming EDX locally or clearing the payload. The condition can
fail with frame/storage alias mutation; it does not establish general EDX
independence, close later parent children, or replace this body's full contract.

## Ordered tag-5 path

1. `00886938` captures P=`[D+8]` in ESI. Zero P takes the final tag store with
   no child or receiver-payload write. Nonzero P is retained across all children,
   subject to their required nonvolatile preservation.
2. Load A=`[P+4]`, then B=`[A]`. There is no A/null/extent check before that
   second dereference. Push A, P, B, P; form the address of the eight-byte local
   area; push that address; set ECX=P; call `006EE960` at `0088694F`.
3. At that child's entry the five DWORDs after its return address are
   `(&local8, P, B, P, A)`. The local bytes have not been initialized by this
   body and are never read by it afterward. The child's writes/extent and
   meaning of this output area remain unproved. Normal stack balance requires
   removal of all 20 argument bytes, consistent with `RET 14h`; this is a
   caller requirement, not an established child ABI.
4. After the child returns, `00886954` reloads current C=`[P+4]`. Push C and
   call `_free` at `00886958`. The free receives C, which may differ from A.
   Do not cache the original A across `006EE960` for this call.
5. Without cleaning the first free argument yet, `0088695D` pushes captured P.
   It then stores zero at `P+4`, followed by zero at `P+8`, and calls `_free`
   again at `0088696C`. The second call receives P at the top of stack, while
   the earlier C word remains below it. `00886971` cleans both argument DWORDs.
6. Only after the second free returns, store zero at `D+8`, then `FFFFFFFFh`
   at `D+4`, restore EDI/ESI/local stack, and execute `RET` at `00886987`.

At `006EE960`, EAX holds `&local8`, ECX=P, ESI=P, EDI=D and EDX is still incoming
EDX. Flags come from the nonzero-P TEST. No other child has run on this branch.
If the child ignores or preserves EDX, that still does not prove every later
reached child ignores the value. The full body and dependencies remain unopened.

## Ordered tag-1 path

1. `00886988` captures P=`[D+8]`. Zero P takes the final tag store without a
   child or receiver-payload write. For nonzero P, load B=`[P+4]` and test it.
2. If B is nonzero, load L=`[P]`; push literal 1; calculate
   N=`(L+1) mod 2^32`; push N and B. These three words are prepared **before**
   the call to `00419CC0` at `0088699F`.
3. The accepted Source/ledger contract identifies `00419CC0` as a no-argument
   pool getter with plain RET. The pending words belong to the following call,
   not three invented getter parameters. At getter entry, ECX=B, EAX=N,
   ESI=P, EDI=D, and EDX is unchanged incoming EDX. The current owned body
   supplies those machine values but does not establish which the getter uses.
4. Copy getter EAX to ECX, then call `00BD1510` at `008869A6` with the prepared
   stack `(B,N,1)`. Its accepted contract is ECX=pool, three argument DWORDs,
   `RET 0Ch`; the third word is historically recorded as unread. No local pool
   null check, block/length reload or independent repush occurs between calls.
5. Whether B was zero or the pool-return call completed, push captured P and
   call `_free` at `008869AC`. Clean that argument, clear `D+8`, then join the
   final `D+4=FFFFFFFFh` store and plain return at `008869C7`.

The pool getter can change headers or globals after B and N have been captured;
the next call still receives the pending stack words, subject to stack aliasing.
Neither receiver payload nor captured P is reloaded before its free. The owner
and gate used by current Source facilities are separate binding requirements.

## Footprint, mutation and failure limits

The complete direct receiver footprint is a DWORD tag read at `D+4`, an optional
DWORD payload read at `D+8`, a final DWORD tag store on every normally returning
path, and a zero DWORD at `D+8` only after the selected nonzero-P child/free path.
Other tags do not read or clear the payload. No direct `D+0`, `D+Ch`, `D+10h` or
larger receiver access is encoded. This is not a complete class-size claim.

Tag 1 directly reads payload DWORDs `P+4` and, only when B is nonzero, `P+0`.
It does not clear payload fields before freeing P. Tag 5 reads `P+4`, dereferences
its captured A, reloads `P+4` after the child, then clears `P+4` and `P+8` after
the first free but before freeing P. None of those selected payload extents
proves the complete child's layout or the total transitive footprint.

The receiver, payload, nested pointer and stack slots have no disjointness,
alignment, validity or bounds checks. A zero/unmapped D faults at the initial
tag read before any child. P is null-checked; its nested pointer A is not.
Invalid aliases can make C equal P or receiver storage, so subsequent stores
may target already-freed storage. Such inputs are not repaired. Partial aliasing
also makes exact store order observable, including P-field clearing before
receiver clearing. No global ownership or deallocation safety is inferred from
names such as "pool", "free" or the probable STL tag on the unresolved child.

Calls can change D/P contents, but the initial tag decision and captured P are
not refreshed. The tag-5 nested free pointer is refreshed exactly once after
`006EE960`; the tag-1 block/size pair is captured before the getter. Final stores
overwrite any earlier child changes to those receiver words. Saved frame values
are read during epilogue; arbitrary stack aliases or incompatible callees can
invalidate normal restoration and balance.

No local EH/cleanup frame protects this schedule. If a child fails before later
stores, those later owned effects are not reached by normal continuation. The
parent's existing frame and any child EH remain external contexts. This packet
does not establish exceptional cleanup, rollback, leak freedom, Native CRT
semantics or safe repeated retirement. In particular, tag-5 P-field zeroing occurs
only after the first free returns, and D+8 is cleared only after the final free.

## Current Source and next dependency

Exact address searches and targeted reconstruction records find no current
Source for `00886920` or `006EE960`. The latter is metadata-bounded at 201 bytes,
76 listed instructions and six listed calls. A fresh bounded Astra packet can
check the complete body, EDX and other incoming machine state, the five arguments,
local-output contract, cleanup/return, mutations and direct children. Its listing
counts are metadata, not a future completeness guarantee.

| Direct child | Metadata bytes / instructions / calls | Current Source evidence |
| --- | --- | --- |
| `006EE960` | 201 / 76 / 6 | No address-bound Source; first next dependency. |
| `00419CC0` | 192 / 51 / 5 | `native_string_pool_get_or_create_00419cc0`, two current publication/domain overloads; accepted no-native-argument/plain-RET contract. |
| `00BD1510` | 95 / 32 / 2 | `return_native_string_pool_00bd1510`, explicit owner/block/size/live-gate Source interface; accepted Native `RET 0Ch` contract. |
| `00BF65AC` | 5 / 1 / 0 | Metadata identifies the `_free` thunk; admitted `singleton_lifetime_free` supplies the typed host CRT boundary. |

The current pool getter and return helper were inspected only as Source, through
their actual admission reports and targeted ledger records. The owner/storage
reports contain no canonical Source artifact hashes to replay; current Git blob
and physical file pins are recorded without treating those historical fixture
claims as new evidence for the current revisions. Their published ABI/game
validation limits remain in force. The free helper's current pin is replayed
through the accepted parent report.

All parent current-input pins are replayed against their stated historical Git
revision, with any current drift listed separately. The parent's stale historical
`00426060` Source hashes remain qualified and are not used to revalidate that
Original ABI. The JSON also embeds the full decode, both listing gaps, six call
boundaries, register/frame/footprint contracts and this document's hash.

Only static readiness evidence is added. No existing fixture or build was rerun,
and application startup, binary replacement and gameplay remain unproved.
