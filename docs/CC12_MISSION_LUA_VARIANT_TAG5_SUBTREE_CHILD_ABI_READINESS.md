# Raw mission Lua variant tag-5 recursive child: 006EE890

The full `006EE890..006EE925` body is 150 bytes / 50 instructions. It checks a
byte at node `+31h`, recursively processes the pointer at `+8`, captures the
current pointer at `+0` after recursion, invokes a child at node `+1Ch`, performs
an optional pool return, frees the current node, then checks and iterates the
saved `+0` pointer. Its sole normal exit is `RET 4` at `006EE923`.

The live listing omits the 11-byte continuation after `_free`. Those four
instructions contain the next-node test and loop backedge, so the pseudocode's
one-node return does not describe the complete normal path. All 150 live Ghidra
bytes match the installed PE, and every byte is independently decoded.

**Source closure remains held**, chiefly at the 101-byte `00884AF0` child and
the unopened local EH contract. Incoming EAX is discarded. Incoming EDX has no
owned read or write, but can pass through recursion into external children.
The parent's duplicate K in EDX is not locally consumed as a node argument;
the node is read from the pushed stack DWORD. "Subtree" and node roles remain
descriptive hypotheses, not a recovered class or ownership proof.

This packet adds only this document and its JSON report. There are no Source,
CMake, ledger, Ghidra, build, test, probe, Original execution or credit changes.

## Complete extent and target

- Published baseline: `aa9c7182e35dc2c23ab781c51f7003306f8ced5a`.
- Worker: `agent/cc12_lua_variant_copy_child_abi`; packet:
  `cc12_lua_variant_tag5_subtree_child_ABI_readiness`.
- Metadata checked before body capture: 150-byte extent, 46 listed instructions,
  5 blocks, 6 edges and 5 calls. The extent passes the 300-byte gate; the listed
  instruction/flow counts omit the normal loop tail.
- Full independent decode: 50 instructions / 150 bytes, five call sites and one
  `C2 04 00` return. There is one recursive call to the owned entry and four
  external child calls. No new function body is opened by recognizing recursion.
- Whole-span SHA-256:
  `d7415860864ad8bebea6795a8bc3d9a751855a72ca9cda6833f0f4c16c06d1d6`.
- Installed PE SHA-256:
  `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
  RVA and file offset are `002EE890h`.
- Capstone 5.0.7 consumes the entire selected span, ending at `006EE925`.
  Every direct branch target is an owned instruction start. The ordinary local
  graph, allowing recursive/external calls to return, includes all 50 instructions.
  This is static flow evidence, not proof of recursion termination or execution.
- Each live CLI query verifies existing project `bsp`, program
  `/battlestationspacific.exe`, language `x86:LE:32:default` and base `00400000`.
  The configured `C:/Users/sqz269/bsp.gpr` exists and was reused. Live/snapshot
  function counts both remain 64,729.
- No outside data, handler, profile, table, string, Native caller, Native child
  or adjacent body was read. `006EE5D0` remains outside this packet. External
  children received metadata only. Shared exports resolve through
  `tools/workspace.py`, without a worktree junction.

The missing continuation is wholly inside the admitted extent:

| Address | Bytes | Instruction |
| --- | --- | --- |
| `006EE906` | `83 c4 04` | `ADD ESP,4` — clean the free argument. |
| `006EE909` | `80 7e 31 00` | `CMP byte [ESI+31h],0` — test the saved next node after free returns. |
| `006EE90D` | `8b ee` | `MOV EBP,ESI` — change current node; preserve CMP flags. |
| `006EE90F` | `74 a6` | `JE 006EE8B7` — repeat when that byte is zero. |

No no-return property was queried or changed. The listing stops after the
`006EE901` free call, but the exact cause of that saved analysis omission is
not established. No flow/listing repair or annotation was performed.

## Entry registers, stack argument and frame

Let S be entry ESP, R entry ECX, and K the DWORD read from `[S+4]` at `006EE8A7`.
R is retained in EBX and forwarded to recursive calls. No owned instruction
dereferences storage relative to R. The explicit node comes from the stack,
independently of EDX and the parent's other delivered register values.

| Incoming state | Owned classification |
| --- | --- |
| ECX | Captured as opaque recursive context R at `006EE8B1`; recursively forwarded in ECX. No owned receiver-pointee access. |
| EAX | Immediately replaced by `FS:[0]` at `006EE890`; no incoming EAX value is consumed. |
| EDX | No read or write in any of the 50 instructions. A recursive call and subsequent external children may receive the entry value or a value left by intervening children. No hidden-input or independence claim follows. |
| EBX / EBP | Saved, then replaced by R / K before children. Incoming values are used only for preservation. |
| ESI | Saved, then replaced by K; later holds the next pointer captured after recursion. |
| EDI | Saved but not locally replaced until after the first recursive call. It can reach that owned recursive entry unchanged; no owned semantic read precedes its later `LEA` definition. |
| Arithmetic flags | The first conditional use follows the owned byte CMP at `006EE8AB`. PUSH/MOV operations between that CMP and JNZ preserve its flags. No incoming arithmetic flag is consumed. |
| x87 state | No owned x87 instruction, environment access or reset. External-child behavior remains unproved. |

The parent `006EE960` sets ECX=R, EDX=K and pushes the same K. This body reads K
from its stack slot and does not use the duplicate EDX locally. At the recursive
call, EAX is the freshly loaded `+8` pointer and that same value is pushed, while
EDX is **not** rewritten to match it. Recursive entry again discards EAX and
loads its pushed node. EDX can therefore differ from deeper explicit node words;
matching EDX and K at the outer caller does not prove an EDX node parameter.

The local exception registration and saved registers are:

| Address | Role |
| --- | --- |
| `S-4` | Unwind state, initially `FFFFFFFFh`. |
| `S-8` | Handler address `00C830A8h`. |
| `S-Ch` | Saved FS head; registration installed at this address. |
| `S-10h` | Saved EBX. |
| `S-14h` | Saved EBP. |
| `S-18h` | Saved ESI. |
| `S-1Ch` | Saved EDI; steady body ESP. |
| `S+4` | Initial node argument; overwritten later with current node `+Ch`. |

The initial byte test occurs after the EBX/EBP saves but before ESI/EDI saves.
The local frame is installed and state remains -1 during that test. Exceptional
restoration at such a fault is a handler question, not proved by normal pops.

## Sentinel condition, recursion and local iteration

`006EE8AB` compares byte `[K+31h]` with zero. Any nonzero byte takes the normal
epilogue without child calls, link/payload reads or free. This is a byte flag
condition; there is no node==null or node==header comparison. Zero/unmapped K
ordinarily faults at `K+31h` rather than acting as an empty input.

For a flag-zero current node N, EBP and ESI initially identify N. One iteration
has this exact schedule:

1. At `006EE8B7`, read Q=`[N+8]` into EAX; push Q; set ECX=R from EBX; call
   `006EE890` recursively at `006EE8BD`. This call's four-byte callee cleanup is
   established by the same complete owned body. Each invocation has its own
   exception frame and node argument. No null/right-link validity guard is added.
2. Only after recursion returns, `006EE8C2` reloads `[N+0]` through the retained
   ESI and captures it as L in ESI. EBP still holds N. Changes to `[N]` during
   recursion affect L; later child changes to `[N]` do not trigger another reload.
3. Form V=`N+Ch` in EDI and overwrite the original argument slot `[S+4]` with V
   at `006EE8C7`. This slot is not subsequently reread by owned normal code.
   Set ECX=`V+10h`=`N+1Ch`, set state zero, and call `00884AF0` at `006EE8D6`
   with no pushed argument. The child's footprint and lifetime behavior are unopened.
4. After that child returns, read B=`[V+8]`=`[N+14h]` into EAX and TEST it.
   State remains zero through this read/test, then becomes -1 at `006EE8E0`.
5. If B is nonzero, read W=`[V+4]`=`[N+10h]` into EDI. Push literal 1, compute
   Z=`(W+1) mod 2^32`, push Z and B. Call `00419CC0` at `006EE8F4`; move its
   EAX to ECX; call `00BD1510` at `006EE8FB` using those pending `(B,Z,1)` words.
   There is no local null-pool check, size clamp or payload reload between calls.
6. With B zero or after a normal pool-return call, push **captured N** in EBP
   and call `_free` at `006EE901`. The omitted tail cleans that argument, then
   reads byte `[L+31h]`, copies L into EBP, and loops if that byte is zero.
   Otherwise it restores the frame and returns.

For a valid stable graph, this means the `+8` branch is handled before the
current node, and the saved `+0` branch after the current node is freed. It is
not a proof that the input graph is a tree, that nodes are unique, or that every
node is freed once. There is no visited set, cycle/shared-link check, recursion
bound or proof of progress. A captured L equal to N or an already retired node
can be dereferenced after its storage was freed; the body does not repair it.

The current node flag is not rechecked after recursion or children. The next
node flag is checked **after** current-node free returns. Links, flags and values
can change across callbacks, so neither an entry snapshot nor a preloaded next
flag preserves the observed order.

## Child stack boundaries and late data

There are five static call sites: self at `006EE8BD`, `00884AF0` at `006EE8D6`,
getter `00419CC0` at `006EE8F4`, pool return `00BD1510` at `006EE8FB`, and free
`00BF65AC` at `006EE901`.

The three pool-return words are pushed before the getter. Existing admitted
Source/ledger evidence describes the getter as no native arguments/plain RET,
and the following pool return as ECX=pool plus three DWORDs/`RET 0Ch` (the last
word historically unread). They must not become invented getter arguments from
the pseudocode. The body provides no cleanup between those calls. The free is
followed by explicit four-byte caller cleanup. `00884AF0` receives no pushed
argument and must preserve zero-argument stack balance for the observed schedule.

The block B is fetched after `00884AF0`; W is fetched afterward only if B was
nonzero. B/Z are then captured on the stack before the getter, while N and L are
retained in nonvolatile registers. The getter, return helper or free can alter
aliased storage, but N is not reloaded for free and L is not reloaded for the
next-node test. Only the next flag is read late. Whole-call preservation still
depends on compatible external child ABIs.

## Direct footprint, EH and normal result

The owned node-relative reads are `+31h` (one byte), `+8` and `+0` (DWORD links),
`+14h` (DWORD B), and `+10h` (DWORD W only when B is nonzero). The initial or next
nonzero-flag path reads only that flag. The highest directly consumed byte is
node `+31h`; this is a minimum address extent, not a complete 50-byte class or
allocation-size claim. No owned node-field store, link clearing, count update,
receiver-header write or prior-output read is encoded. Children may do all of
those things, and stack/FS stores can alias node storage under raw malformed inputs.

The body itself mutates FS registration, unwind-state and saved-frame words,
and the original argument slot with V=`N+Ch`. It never reloads K from that
overwritten slot for iteration. A cached original node must not replace the
captured ESI/EBP schedule, and a const argument-slot assumption would omit a
real write observable through aliases or exceptional machinery.

State is -1 during recursion, next-link capture and argument-slot publication;
zero is installed at `006EE8CE` before `00884AF0`, retained through the subsequent
block read/test, and reset to -1 before W/size reads, pool calls, free and the
next-node test. The handler `00C830A8` was not opened. Its cleanup, ownership and
second-failure behavior are unproved; a numeric state and saved V do not justify
invented RAII or rollback.

On normal exit, the body loads the current saved FS head at `[S-Ch]`, restores
EDI/ESI/EBP, reinstalls that FS word, restores EBX, discards the three exception
words, and executes `RET 4`. This closes the accepted parent's four-byte cleanup
requirement. There is no uniform receiver/node-return contract: the initial
nonzero-flag path leaves the old FS head in EAX; paths which process nodes leave
the final free's EAX value unchanged locally. The parent ignores that result and
performs its own header reload/store sequence after full return.

Null/invalid node, link or value storage has no general guard. Failure during a
child, pool return, free or late flag read can occur after earlier recursive work
and release effects. Normal FS restoration does not prove exceptional cleanup,
leak freedom, safe repeated retirement, alias safety or Native allocator/EH parity.

## Current Source and next dependency

Exact searches find no current Source or reconstruction record for `006EE890`
or `00884AF0`. Self-recursion stays within the complete body evidence; it does
not supply a new independent Source implementation.

| External child | Metadata bytes / instructions / calls | Current status |
| --- | --- | --- |
| `00884AF0` | 101 / 29 / 3 | First small next body: node `+1Ch` receiver, delivered registers, no-argument return, exact fields/reloads and local EH. |
| `00419CC0` | 192 / 51 / 5 | Admitted pool getter Source with explicit publication/lifetime contexts; historical ABI evidence, no new Native ABI check here. |
| `00BD1510` | 95 / 32 / 2 | Admitted pool-return Source with actual owner/block/size/live gate; host/Native boundary remains. |
| `00BF65AC` | 5 / 1 / 0 | Admitted typed host CRT free service; no new Original binary binding proof. |

`00884AF0` metadata lists `00886920`, the pool getter and pool return as children.
That links back into the existing readiness graph; it does not close the graph
or prove an independent leaf. No body from that next layer was read here. The
101-byte next packet still requires its own lease and complete-byte/ABI review.
Handler `00C830A8` remains a separately bounded future scope.

All 60 parent input pins replay exactly and match this baseline, including the
current Source/header/report files for the three admitted services. Their
previously inspected Source contracts are reused only after that byte check.
The pool reports' missing per-file historical hashes and existing CRT/locking/EH
limits remain explicit. No old fixture claim is promoted to a new current or
Original ABI result. The inherited stale string receipt and the unopened
637-byte `006EE5D0` remain qualified and outside this body scope.

The JSON embeds the full 150-byte decode, missing continuation, recursive/local
flow boundaries, exact register/frame/reload schedule and canonical/physical
pins. No application startup, binary replacement or gameplay proof is added.
