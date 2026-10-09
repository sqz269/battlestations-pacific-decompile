# Mission Lua variant owned cleanup: ABI readiness

The saved `006EE040..006EE0A1` body is fully accounted for: **98 bytes, 29 operations, three blocks, three edges, three calls and one plain RET**. Fresh live bytes agree with the file-backed original executable and the primary Astra gate. The existing Source pool providers are present and registered. Admission remains held because the embedded cleanup, actual selected variant ownership/lifetime and Native child/EH contracts remain incomplete.

This is a read-only evidence packet. It changes only this document and its JSON report. There is no Source, Ghidra, ledger or CMake edit; no build, test, probe or new consumer; and zero new Source, Original ABI or gameplay credit. “Owned cleanup” and the field roles below remain provisional descriptions, not recovered symbols or a copy-provider claim.

## Evidence boundary

Baseline: `572b6612cf6c2c8092922f75a8b104d408a8e374`. The whole-body SHA-256 is `329d7b7b6a88be900256ee4dc587853dae8370b8c9cf05d39ba36070edb30f7a`. The current program is `/battlestationspacific.exe` in the existing `C:/Users/sqz269/bsp.gpr`; the observed total remains 64,729 functions. All bytes are backed by `.text` at RVA/file offset `002EE040`. The report records the original-image identity, complete instruction bytes, physical offsets, Root gate pins and conditional instruction-by-instruction stack replay.

The primary read the entire 98-byte original body before delegating this Source contract audit. Native bodies `00884AF0`, `00419CC0`, `00BD1510`, handler `00C82FE8`, callers, neighbors, global words and pool data were not opened. Subordinate Native questions were metadata only. Current Source providers and accepted prior reports are separate evidence domains.

## Entry, frame and embedded call

Let `S` be entry ESP and `N` entry ECX. All address and integer arithmetic is modulo 2^32. The body loads no external stack arguments and ends with plain RET.

| Site | Exact owned effect |
| --- | --- |
| `006EE040..04E` | Push full state `-1` at `S-4`, handler `00C82FE8` at `S-8`, capture prior `FS:[0]` into EAX and push at `S-0C`, then publish `S-0C` to `FS:[0]`. |
| `006EE055..059` | Push incoming ECX at `S-10`, save incoming ESI at `S-14`, capture ECX in ESI and rewrite current `[S-10]` with ESI. |
| `006EE05D..060` | Form ECX from current `ESI+10h`, **then** store full DWORD zero into current `[ESP+10h]`, conventionally `S-4`. |
| `006EE068` | Always call `00884AF0`, with caller ESP `S-14`, child entry ESP `S-18` and no argument pushes. |

At the embedded call EAX is the prior FS word, EDX is incoming EDX, and arithmetic flags are incoming flags: the owned PUSH/MOV/LEA prefix does not define them. The formed `N+10h` ECX value precedes the state store, so an aliasing store can change backing without changing that register value. LEA itself does not read a receiver field. A conventional normal frame requires this child to return without argument cleanup. Correct later receiver reads require child preservation of ESI; EBX, EDI and EBP also require compatible preservation because this body does not save them.

The local state sequence is full DWORD `-1 -> 0 -> -1`. The handler remains unopened. Registration syntax and state writes do not establish unwind actions, nonthrowing behavior, termination, rollback or Original EH identity. Stack/FS writes can alias other live backing even though this body has no explicit receiver-field stores.

## Late fields and the two pool calls

After the embedded child, `006EE06D` loads **current** `[N+8]` into ECX as `P`. TEST at `006EE070` establishes the condition, and the full DWORD state-reset MOV at `006EE072` preserves it for JE at `006EE07A`. `P==0` skips both pool calls but never skips the embedded call. An aliasing state store can change backing after `P` has been captured; it does not recapture P or replace the TEST condition.

For `P!=0`, `006EE07C` loads **current** `[N+4]` into EAX as `L`, after the state reset. `006EE07F` pushes literal `1` **before** `006EE081` adds one to EAX. Thus even a push aliasing `[N+4]` does not change the captured L used by ADD. The result is `Z=(L+1) mod 2^32`, without an overflow/type/length check. The body pushes Z and captured P, then calls `00419CC0`. On return it moves EAX to ECX and calls `00BD1510`, with no intervening argument push or stack adjustment.

| Word address | Parent write | At getter child entry `S-24` |
| --- | --- | --- |
| `S-20` | captured P | `[ESP+4]` |
| `S-1C` | wrapped Z | `[ESP+8]` |
| `S-18` | literal 1 | `[ESP+0C]` |

The current Source header describes the Native getter as no-argument/plain-RET and the return helper as ECX pool plus stack `(block,size,unused)`, RET `0Ch`. Under this **inherited** split, both children enter at `S-24`; the pending word addresses are unchanged, while each CALL writes a different return address at `S-24`. The getter can change pending word values through aliases. The parent does not refresh them, so the return helper sees their then-current contents, not necessarily the originally pushed P/Z/1.

The owned nonzero path establishes only the conventional **net 12-byte child cleanup requirement**. It does not itself determine each child's convention or word semantics. A different getter pop would change the second call's positions. The inherited no-argument getter does not acquire semantic arguments merely because these three words are physically present.

At getter entry ECX=P, EAX=Z and EDX is the embedded child's residual. ADD supplies arithmetic flags: CF iff `L==FFFFFFFF`, OF iff `L==7FFFFFFF`, AF iff `(L&0F)==0F`, ZF iff Z is zero, SF=bit31(Z), PF=even low-byte parity. PUSH and CALL preserve those flags. At return-helper entry ECX and EAX both hold getter result G; EDX and flags are getter residuals. No pool-null guard is added.

The direct read footprint is unconditional DWORD `N+8..0B` after the embedded call and conditional DWORD `N+4..7` after the state reset. This selects a 12-byte prefix; it is not an allocation size or a complete class declaration. There are **zero explicit N-field stores**, no pointer/length clearing and no established idempotence. The embedded child can reach beyond this prefix.

## Normal tail

At the balanced join, `006EE092` captures the **current** saved-chain word `[ESP+8]=[S-0C]` into ECX before `006EE096` pops ESI from current `[S-14]`. `006EE097` restores FS from captured ECX. `006EE09E` adds `10h` to ESP, reaching S; `006EE0A1` is plain RET, leaving caller ESP `S+4`.

EAX and EDX are residuals of the last executed child: embedded cleanup on the zero branch, return helper on the nonzero branch. The body does not guarantee a common semantic return or `EAX=N`. Final ECX is the late saved-chain value. Final arithmetic flags come from `ADD ESP,10h`, not TEST or length ADD: for `x=S-10h` and result `r=S`, CF iff `x>=FFFFFFF0`, OF iff `7FFFFFF0<=x<=7FFFFFFF`, AF=0, ZF iff r=0, SF=bit31(r), PF=even low-byte parity. RET preserves them.

These are static normal-boundary statements conditional on compatible child returns, preservation and live frame backing. Late frame reads, restoration and RET may fault; there is no exceptional-path execution proof.

## Current Source providers and actual storage

`native_string_pool_owner.hpp/.cpp` provides two ordinary C++ singleton getter overloads, taking an actual publication reference and either a semantic lifetime-domain reference or the actual raw manager-slot reference. The slow path captures the manager/lock, double-checks publication, allocates and constructs, publishes, obtains the current registration manager, registers the current publication and releases the captured lock/depth in `__finally`; it returns a fresh publication read. Its explicit arguments and current Source exception policy do not provide the unopened Native no-argument ABI.

`native_string_pool_storage.hpp/.cpp` provides `return_native_string_pool_00bd1510(actual_pool, block, size, actual_live_gate_reference) noexcept`. Size >=150 uses current `std::free` without reading pool/gate. For smaller sizes it first reads the live gate and can return early; otherwise it uses the captured critical section, depth increment, raw small return, depth decrement and leave. The fourth Source argument is the **actual gate reference**, not the Native pending literal 1. Current CRT, Win32 synchronization and noexcept behavior remain Source policies.

The borrowed `ActualNativeStringPoolStorage` bridge retains publication and gate references plus either lifetime domain or raw manager slot. Its release gets the pool before returning the block. Its inherited `noexcept` means a C++ getter failure may terminate; this cannot be substituted for an unknown Native EH contract. Likewise the owned nonzero branch always calls getter first even when the eventual return-helper path would ignore pool/gate. The getter cannot be removed based on size or gate state.

The reconstructed `GameNativeStringProcess` owns publication, gate and raw manager slots. Its raw context and storage bridge borrow those same slots; the process object is retained until termination. Another existing consumer, `TitleFileBlock::return_caller_name`, captures data, checks null, computes uint32 length+1, calls the raw getter then the return helper with the actual gate, and does not reset its header. This establishes a real **other** Source domain and consumer. It does not bind the selected variant N or its embedded `N+10h` object to that domain, allocation, registration, deletion or exceptional lifetime. Original global words were not read.

Current `cmake/startup.cmake` registers both pool implementation files and the game process-domain sources. The older pool-owner JSON still says registration was pending; that line is historical and is not current absence evidence.

A complete search of current tracked `src/` and `include/` text finds no provider keyed to `006EE040` or `00884AF0`. The existing `008849B0` initializer writes a 20-byte prefix, and raw link-repair/small-link helpers have their own narrow contracts. They do not establish a complete selected variant, embedded destructor, production lifetime, unwind map or ABI-compatible composition. No new type or adapter is supplied here.

## Accepted context and verification limits

The accepted selected-parent audit covers 637 saved bytes/215 operations plus 54 selected continuation bytes/19 operations: 691 bytes/234 operations, without extent mutation or ABI admission. Its accepted retirement boundary reloads current Z, forms `Z+0Ch`, and calls this child without arguments; it reloads current Z again after return before outer free. The selected receiver and subsequent free target cannot be frozen across this child. That caller, its continuation and its historical 74-input array were not reopened/replayed here.

The accepted 99-byte/38-operation pair audit retains its full DWORD guard, late pair-field timing, returning-assertion possibility and unresolved composition boundary. It was not reopened or migrated. Its historical 82-input array was not replayed. Later current Source providers must not be hidden by repeating older Source-absence statements; presence alone does not bind either the pair or selected parent.

All **80** Source input identities from the latest accepted normal-build receipt match this baseline's canonical Git blobs and normalized working files. That prior build ran `scripts/build.ps1` from `2026-10-09T16:28:10Z` to `16:28:29Z`, passed three existing checks and recorded 16 whole objects/18 positive public Core roots. Its pending-entity provider has 12 functions, 18 sections, 6063 bytes and exactly two 44-byte EH sections; the separate eight EH payloads belong to the Legacy object. These compiled records remain prior evidence: no object recapture, fixture rerun, new build or application execution occurred here.

The JSON report contains exact current provider excerpts, canonical input pins, the 80-input identity replay and complete owned instruction/stack records. Remaining work is separately owned evidence for `00884AF0`, actual selected variant production backing/lifetime, Native pool ABI and exception contracts, and handler behavior. This packet alone admits none of those dependencies and provides no gameplay proof.
