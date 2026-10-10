# CC12 game-state enqueue owner and read-order review

`004D3ED0` reads the caller's pointer argument and its DWORD value **after**
optional map growth, optional block allocation, block publication and destination
selection. It captures the incoming queue, keeps its computed offset/index
across allocation, and reloads the map pointer after allocation. Its own
`RET 4` is directly proved. These orders are not established by the existing
by-value C++ queue interface.

There is **no production C++ binding GO**. The next smallest separately leased
Native body is `004D20D0`, the growth routine. Allocator behavior and production
raw queue ownership remain explicit dependencies.

The independently frozen Source baseline is
**ea1b54c1b8445c49a3ab06c4fa8ada44fec77470**. It includes nine previously
accepted overlays plus the accepted startup changes in `game_hosts.hpp` and
`game_hosts.cpp`. Both new files match Root's complete
`Source3352-particle-startup` manifest. Separate pending header work is outside
this epoch. This packet owns only the enqueue body, this document, its report,
and its ignored local evidence directory.

## Complete body, fields and control flow

The body is exactly `004D3ED0..004D3F44`: **117 bytes, 46 instructions**. It
matches the retained original PE and verified live byte query. The direct live
disassembly matches the saved exported listing instruction-for-instruction.
The two `show` calls selected saved exports despite the `--live` option; their
markers are retained and their evidence is labeled accordingly. A separate
typed `ghidra disasm` supplied the live listing. Every Ghidra query verified
the configured project/program, x86 language and `00400000` image base:
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`.

Let Q be entry ECX, S be entry ESP, and `u32` mean wrapped 32-bit arithmetic.
The pointer argument occupies `[S+4]`. Q is the raw embedded queue supplied
by the accepted wrapper as incoming game A+5D8h.

| Queue offset | Local use |
| --- | --- |
| `+04` | Raw map pointer; loaded before the slot test, reloaded after allocation before publication, and reloaded for final destination selection. |
| `+08` | Block-map capacity; read for the growth gate and again after optional growth. |
| `+0C` | Head offset; read before and after optional growth. No local head store. |
| `+10` | Element count; read for the gate and post-growth offset, then incremented in memory at the end. |
| `+00` | No local operand; delegated preservation is not established. |

The complete local order is:

1. `004D3ED1` captures Q in ESI. Read count C0 and head H0. If
   `u32(H0+C0) & 3` is zero, compare capacity B0 with
   `u32(C0+4) >> 2`. Call `004D20D0` only when B0 is unsigned-less-than-or-equal
   to that value, using ECX=Q and the literal argument 1.
2. `004D3EF4..004D3F08` rereads capacity B1, head H1 and count C1 after the
   optional growth. Keep T=`u32(H1+C1)` in EBX and K=`T>>2` in EDI. If K is
   greater than or equal to B1, subtract B1 **once**. This is not a general
   modulo operation or a bounds check.
3. `004D3F0A..004D3F20` reads the current map and tests slot K. If that slot is
   zero, call the named `operator_new` at `00BF681B` with `0x10`. After it
   returns, reload Q+4 and store its EAX result into the reloaded map's slot K.
   Q, T and K are retained across the call under the declared nonvolatile ABI.
4. `004D3F23..004D3F33` reloads the map again and reads its slot K. Compute
   D=`u32(block + (T&3)*4)`. Test D; the following EDI/EBX pops preserve those
   test flags. Branch on whether the **computed destination** is zero.
5. Only when D is nonzero, `004D3F35` loads the pointer argument from `[S+4]`,
   `004D3F39` reads one DWORD through it, and `004D3F3B` stores that DWORD at D.
   Neither the argument pointer nor the pointee was read earlier in this body.
6. `004D3F3D` increments the **current** DWORD `[Q+10h]`, then the function
   restores ESI and executes `RET 4`. EAX remains computed D on normal return;
   no recovered return-type or caller-use meaning is inferred.

The element width is **four bytes**. `0x10` is the allocation size of a block
holding four such slots. The body performs no local block-wide clear.

There are three explicit data-write sites: map slot `004D3F20`, selected DWORD
`004D3F3B`, and count increment `004D3F3D`. There are 14 explicit read operands,
one address-only LEA, five conditional branches, two calls, five push sites,
three pop sites and one `RET 4`. Implicit call/stack writes are separate from
the three explicit stores. No current-game publication or callback-slot operand
occurs locally; delegated writes and aliases remain possible.

The live metadata records 17 call xrefs and 11 function callers. Only this
body was decoded. `004D20D0`, `00BF681B`, callers, data tables and other providers
were not opened for Native analysis, and no Ghidra annotation or listing was
changed.

## Stack contract and read timing

The body saves ESI, then later EBX and EDI. After those two latter registers
are popped, ESP is S-4, so the operand `[ESP+8]` at `004D3F35` is the incoming
pointer slot S+4. The late pointer load and pointee read occur only on the
nonzero-D arm, after every optional call and any new-block publication.

The leaf's `RET 4` directly closes the cleanup assumption in the preceding
`004D7920` review. Two provider assumptions remain: growth removes its own
DWORD argument, while `operator_new` leaves its size argument for the explicit
`ADD ESP,4` at `004D3F1D`. Ordinary nonvolatile-register preservation is also
required across those providers. Their actual bodies and exception paths were
not inspected.

The audit enumerates all **24 syntactic normal-return paths**, covering every
instruction and every stack join. Each reaches `RET 4` with ESP restored to S;
return leaves the caller at S+8. Twelve syntactic paths copy a value and twelve
take the zero-destination skip. These counts describe the control-flow graph
under declared normal provider return, not proof that all combinations are
feasible for a valid deque or that exceptional paths were covered.

For the accepted title route, the wrapper supplies Q=A+5D8h and a pointer to
the caller's live slot containing 4. The leaf does not reload a newer game or
queue. If a provider can reenter or alter aliased state, its effects precede
the value read. This packet establishes that ordering; it does not establish
that such reentry happens or cannot happen. The accepted title function still
reloads game B after the wrapper returns before clearing B's hold byte.

## Failure, malformed-state and alias boundaries

The zero-D arm skips the pointer load, pointee read and value store, yet still
increments the current count. This is **not** an allocation-null check. If an
allocator were to return zero, a nonzero within-block offset would still form
a nonzero D. A nonzero block address could also wrap when the offset is added.
The actual allocator's null/throw contract remains uninspected; no new fallback
or null guard is justified by this review.

Growth or allocation that does not return prevents later local steps. A fault
during the late argument/pointee access or value store prevents the following
increment. The new block is already published before that late read. There is
no local rollback or cleanup path; external exception behavior has not been
analyzed or executed.

The capacity subtraction assumes suitable queue bounds if K is to be a valid
index. Zero capacity, wrapped counters, invalid map/slot pointers or overlapping
storage receive no local repair. The final `ADD [Q+10h],1` operates on the then
current count, not necessarily C1: provider effects and earlier stores through
aliases can change it. Likewise, head/capacity having no local stores does not
prove they are preserved by the whole call. Neither a pre-allocation map cache
nor an early value snapshot is established as equivalent.

## Current Source and ownership comparison

The existing `enqueue_state_request_004d3ed0` at
`src/game_frame_control.cpp:96` accepts `std::uint32_t` **by value**. Its value
is available before its growth and allocation work. Native delays both the
pointer-argument load and pointee read. General equivalence therefore needs
the actual alias, provider and read-order contract; the semantic helper alone
does not supply it.

`GameStateRequestQueue` at `include/bsp/game_frame_control.hpp:29` owns a
`vector<unique_ptr<GameStateRequestBlock>>` and separate head/count fields.
The helper grows that vector, creates a C++ block and writes the selected value.
`GameStateRequestBlock` has `entries{}`, and `new GameStateRequestBlock()`
initializes the block. The Native body requests 16 bytes and has only the
selected element store; it does not clear the whole block locally. Initialization
by the allocator is a separate unknown. This review grants no raw-layout,
allocation, failure or byte-for-byte equivalence to the C++ interface.

`MenuPathHost` and `MenuTitleHandoverHost` operate on fixed `MenuImpl::requests`
and mirror its count into their borrowed `GameStateSlot`. Current
`GameStartupHost` still owns a separate `GameStateSlot` at
`include/bsp/game_hosts.hpp:739` and passes it into `GameMenuHost` at
`src/game_hosts.cpp:2313`. No association between that semantic queue and the
incoming raw Q has been established.

Retained raw lifecycle Source is available: game construction initializes the
map/capacity/head/count words and hold byte; game teardown passes game+5D8h to
`destroy_native_game_pointer_blocks_004c4a50`, which uses a supplied free
provider for raw blocks and map storage. `GameNativeGameRuntime` shares raw
construction/lifetime publication cells. These are existing lifecycle
boundaries, not a newly bound raw producer or fresh Native provider proof.

The accepted startup overlay adds particle-owner preparation, construction and
containment. Exact whole `game_hosts.hpp` and `game_hosts.cpp` bytes match the
Root `Source3352-particle-startup` manifest and its primary report. This packet
does not replay that packet's build or award new startup credit. No pending
separate header change is adopted.

Current destructor/shutdown code still deletes menu before frontend
(`src/game_hosts.cpp:1595` and `:2502`). Four automatic `MenuPressStartHost`
objects remain at menu-source lines 1554, 1564, 1785 and 1848. Their borrowed
`MenuImpl` and current publications are insufficient deferred-lifetime proofs.
Accepted distinct profile/storage/settings slots, current versus captured
owners, and outer post-callback uses remain part of any eventual composition.
This packet adds no owner, retry, cancellation, queue repair, callback drain
or cleanup reordering.

## Evidence and replay

The single retained root includes the complete 4,206-file Source Git and
working archives, manifests and current tree, with eleven precisely qualified
overlays. Current Git and working bytes were independently matched. The selected
closure has 393 files from 20 roots and 725 quoted include edges; no project
include is missing. Eight full-corpus queries produce 297 matches. Twenty-seven
Source excerpts and three documentation excerpts are backed by whole files.
System/SDK includes remain listed external dependencies, not a compiler closure.

Four whole prior wrapper/startup worker and primary reports, and their documents,
are retained as exact Git artifacts. Their nested Root proof trees are not
walked or replayed. The accepted 19-byte wrapper is retained for byte identity
against its report and the original PE without new decoding. Only the enqueue
body's 117 bytes receive fresh Native instruction analysis.

The JSON report pins this document, immutable freeze, audit and six helper
scripts. Default verification opens only the new retained root and report,
forbids writes, subprocesses, network and dynamic loading, and recomputes the
Source closure/searches, accepted overlays, full instruction facts, late-read
ordering and 24 declared stack paths. Git/current-source checks use a separate
mode and require exactly the two owned output paths.

```powershell
python local/cc12_game_state_enqueue_owner_review/verify.py
python local/cc12_game_state_enqueue_owner_review/verify.py --original-worktree
python local/cc12_game_state_enqueue_owner_review/relocate_verify.py
```

The relocation helper physically copies every pin and the exact report into
a fresh directory, then runs the guarded verifier there. Its receipt is retained;
it refuses to overwrite an existing destination. Preserve every referenced
relative path under `local/cc12_game_state_enqueue_owner_review/` plus the report.
Old absolute paths in accepted dependencies are provenance only.

The next bounded body is **004D20D0**, to establish map growth, allocation and
movement, raw fields, failure order and cleanup. The `operator_new` contract
and matching destruction ownership remain separate where needed. A production
binding additionally requires the actual raw queue and current/captured
storage, menu, profile, settings/archive, Lua and shutdown owners.

Validation here is evidence replay and artifact/Git identity. There are no C++
edits, compiler runs, new C++ tests, Native/game/SDK/OS execution or Ghidra writes.
No activation, ABI-drop-in, build, fixture, startup or gameplay equivalence is
claimed.
