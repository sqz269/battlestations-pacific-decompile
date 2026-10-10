# CC12 locale pool constructor unwind contract

The three actions named by `00D85670` have complete stored bodies of 8, 11 and
11 bytes. Each reloads `DWORD[EBP-10h]` into ECX, optionally adds a member offset,
and tail-jumps to one cleanup target. This closes action-byte, resource-selection
and dispatch-target evidence. Target cleanup effects and actual FH3 frame setup
remain incomplete dependencies. No C++ implementation or GPR repair is added.

The full record is
[the contract report](../reports/cc12_locale_pool_constructor_unwind_contract.json).
This packet consumes the accepted
[profile](CC12_LOCALE_PROFILE_PRIMARY.md) and
[lifecycle](CC12_LOCALE_LIFECYCLE_PRIMARY.md) contracts, the original
[readiness proof](CC12_LOCALE_POOL_PROFILE_UNWIND_READINESS.md), and the separate
[borrowed recycling helper](CC12_LOCALE_BORROWED_NODE_RECYCLING_HELPER.md).

## Gate and native actions

Before body reads, live prototypes and complete typed ownership admitted the
exact ranges below; their owned sizes equal their hull sizes and each is below
64 bytes. The worker extended its lease to those ranges before reading assembly.
All 30 bytes match the retained original PE, SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Eight instructions account for every admitted byte. There are no gaps, padding,
unowned continuations or fallthroughs into the following handler window.

| Action range | Bytes | ECX passed to tail target | Target and existing stored name |
| --- | ---: | --- | --- |
| `00C5EEA0..00C5EEA7` | 8 | `DWORD[EBP-10h]` | `00403970`, `BSP_AllocatorListElement_UnlinkBase` |
| `00C5EEA8..00C5EEB2` | 11 | `DWORD[EBP-10h] + 0Ch` | `00402F70`, `BSP_RecursiveCriticalSection_Destroy` |
| `00C5EEB3..00C5EEBD` | 11 | `DWORD[EBP-10h] + 28h` | `00435560`, `FUN_00435560` |

The additions are 32-bit native address arithmetic. In particular, action B3
passes the address of the member at `pool+28h`, not the pointer stored in that
member. Each action independently reloads the saved pointer; the table does not
justify caching one value across cleanup calls. Existing target names are
descriptive analysis labels, not independent evidence of target effects.

The actual opcode at each terminal is `E9 rel32`, a direct JMP. The actions have
no prologue, stack adjustment, PUSH, CALL or RET. They leave ESP and its existing
return destination untouched before dispatch; target return and clobber behavior
remain outside this body contract. MOV/JMP preserve flags; the two ADDs update
arithmetic flags. The action instructions themselves leave EAX, EDX, EBX, ESI,
EDI and EBP untouched, while ECX is their output. This is not a preservation
claim for the target routines or the whole exception path.

Stored `undefined Unwind@...(void)` prototypes omit the required EBP-based frame
input. All three terminal instructions retain `CALL_RETURN` overrides and
effective `CALL_TERMINATOR`; their default flow is `UNCONDITIONAL_JUMP` and both
flow views name the same direct target. All three target metadata records say
`no_return=false`. These flags are preserved; no body, flow, signature or
no-return repair is performed.

## Constructor frame and declared states

Let `S` be ESP at entry to `00436230`. Its retained 211-byte hull establishes:

- The initial state `-1` is pushed at `S-4`; handler `C5EEBE` is at `S-8`.
  The saved previous FS head and new FS registration are at `R=S-0Ch`.
- The constructor never establishes EBP. After saving its registers,
  `00436265` writes ESI, carrying the incoming pool ECX, to `[ESP+0Ch]=S-10h`.
  This saved pool slot is `R-4`.
- At `00436278`, after pushing the section argument, `[ESP+1Ch]=S-4` receives
  the zero in EBX. State 0 is armed before the section initialization call.
- The accepted normal stdcall return restores the one section argument. After
  table pointer, count and capacity are zeroed, `004362A3` writes byte 2 to
  `[ESP+18h]=S-4`. The earlier DWORD zero makes the state value 2. This precedes
  the capacity-32 store and the 80h allocation call. There is no normal state-1
  store in the retained constructor.

For an action's `[EBP-10h]` load to select that saved pool, its dispatcher must
supply **`EBP=S=R+0Ch`**. This is the required frame relation established by the
constructor and action bytes. The unread FH3 dispatcher has not been shown to
supply it. The original ESP-based constructor's incoming EBP must not be
silently equated with the action frame EBP. A normalized C++ member-call
interface would therefore not establish the original funclet ABI.

The retained 36-byte `FuncInfo` at `00D85688` has `maxState=3` and unwind map
`00D85670`. Its exact 24-byte map declares:

| State | To state | Action | Selected address under the required frame relation |
| ---: | ---: | --- | --- |
| 2 | 1 | `00C5EEB3` | pool member at `+28h` |
| 1 | 0 | `00C5EEA8` | section member at `+0Ch` |
| 0 | -1 | `00C5EEA0` | pool/list base |

Thus state 1 is a declared intermediate cleanup state even though the normal
constructor stores only -1, 0 and 2. No action itself writes the state. This
table describes the declared chain; dispatcher invocation, action completion,
exceptions during cleanup, SEH coverage and double-failure behavior are unproved.
In particular, this packet cannot establish how target `435560` treats a null
table pointer paired with capacity 32 if allocation fails.

The `C5EEBE` evidence remains a fixed 32-byte window decoded only through its
10-byte MOV/JMP prefix, which names `D85688` and `BF6B43`. It remains function
absent; its remaining 22 bytes are not decoded or assigned a body. Prior
constructor, destructor `435A50`, and profile `CE4120 -> 4360E0` normal-return
contracts retain their previously accepted flow defects and limitations.

## Portable evidence and limits

Capture HEAD is `503e3e09dd741c3bd21cd9c54eb9f5057df09490`, the worker merge of
Main `646420dba` and helper candidate `9bf12e5b8`. This is a separate Source
epoch. Its 4,217 Source files are physically byte-equal to actual committed Git
blobs; retained commit and tree objects also prove each path's membership.
Eight named dependency blobs and four tool/config blobs have the same proof.
Relative to the frozen readiness Source, changes are the existing render-context
and game-grid pairs, the helper's two added files, and its CMake registration.
No build or artifact credit is transferred between epochs.

Three complete typed captures verify `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`, with before/after metadata equal at modification 79.
The ten retained live-query outputs use the identity-checking BSP tools. A
preliminary detailed prototype query was also read; the finite proof uses the
retained concise prototype query. The installed executable was not reopened.
Fresh native bytes total 30; the preceding 1,335 retained native bytes retain
their original provenance and query epochs.

`local/cc12_locale_pool_constructor_unwind_contract/evidence.zip` has 4,263
payload files / 64,862,148 uncompressed bytes, ZIP size 34,694,289 bytes and
SHA-256 `fcae4292adc84d276553b3640e8d40a6a89bb6ad90b0a5dbb9888207bd05f50b`.
Its guarded read-only replay verifies the old nested proof, current manifests,
Git blobs and path trees, PE slices, complete action decoding, constructor frame
arithmetic, descriptor rows and typed metadata equality. An independent physical
copy passes the same replay with external reads, process/network calls, dynamic
loading and directory access denied after the guard is installed.

The next bounded gate is metadata/prototype/typed-flow review for cleanup target
`00435560`, with `00403970` and `00402F70` retained as named incomplete cleanup
dependencies. Any body range needs a new Root-approved lease and explicit size
admission. Actual FH3 dispatcher frame setup is a separate proposal. No target
or destructor body was expanded here. Canonical pool storage/lifetime, slab
production, full owning unwind, original ABI compatibility, payload-before-pool
teardown and application/game activation remain open. No build, test, probe,
game execution, ledger write, GPR mutation or OS change is added.
