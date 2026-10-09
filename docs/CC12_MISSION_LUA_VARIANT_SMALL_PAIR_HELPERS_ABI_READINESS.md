# Raw mission Lua variant small helpers: 006ED9D0 and 006ED9F0

Both helpers consume one raw node pointer in ECX, follow a single link while
the next node's byte at `+31h` is zero, and return the last selected node in
EAX. EDX contains the nonzero-flag child that stopped the traversal. Neither
uses a stack argument, changes pointed data, calls a child, or saves registers.
Each returns with plain `RET`. The accepted caller actually stores EAX into
its retained header, so the decompiler's void presentation is insufficient.

The complete admitted spans are 28 and 27 bytes. The first includes three
unlisted bytes skipped by an owned unconditional jump: full raw coverage is
11 instructions, while ordinary entry flow uses ten instructions / 25 bytes.
The second contains ten listed/raw instructions, including a reachable
six-byte identity LEA. No outside-body read or Ghidra repair is needed.

## Scope and physical evidence

Packet `cc12_mission_lua_variant_small_pair_helpers_ABI_readiness`, fixed
published baseline `b5b3773626bd37c32fae56f6336de65e04ec7f89`. Only this
document and `reports/cc12_mission_lua_variant_small_pair_helpers_ABI_readiness.json`
are changed. The two entry addresses and these exact outputs were leased.

| Admitted span | Full bytes / raw instructions | Saved instructions | SHA-256 |
| --- | --- | --- | --- |
| `006ED9D0..006ED9EB` | 28 / 11 | 10 | `ee04b97e773723b563e7d596fdf33d6845f5ea3c1ed6c9749abf22bcdc7612f7` |
| `006ED9F0..006EDA0A` | 27 / 10 | 10 | `dc6c4ac378bb555bc0e54cf1e9e9175b202005683c502c7f171c9086305a050e` |

All 55 bytes match the live program and configured installed PE. Their
RVA/file offsets are `002ED9D0` and `002ED9F0`. The whole installed PE hash
is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Every saved instruction start matches the independent decode; the sole
additional decoded instruction is the three-byte omitted span below.

Each metadata record reports four blocks, five edges, ten instructions,
zero calls and zero parameters, with a saved `undefined ...(void)` prototype.
The decompiler infers a fastcall ECX parameter but presents no result. No
prototype, no-return flag, flow property, function extent or AddressSet was
changed. The configured existing `C:/Users/sqz269/bsp.gpr` and program
`/battlestationspacific.exe` were reused; each live CLI batch checked project,
program, `x86:LE:32:default` and image base `00400000`. Live and saved counts
are 64,729. This is not a stronger server-side absolute GPR path claim.

No inter-body padding, Native caller, neighbor, profile, string, table,
handler or CRT body was opened. Both bodies have no external transfer.

## Complete 006ED9D0 schedule

Let N0 be incoming ECX, `r(V)=DWORD[V+8]`, and `s(V)=byte[V+31h]`.

1. `006ED9D0`: EAX=N0.
2. `006ED9D2`: EDX=`r(EAX)`; this is the first link read.
3. `006ED9D5`: compare `s(EDX)` with zero, without a pointer/null guard.
4. `006ED9D9`: nonzero branches directly to `006ED9EB`.
5. `006ED9DB`: the zero path jumps unconditionally to `006ED9E0`.
6. `006ED9E0`: EAX=EDX, retaining the node whose flag was tested zero.
7. `006ED9E2`: EDX=`r(EAX)`, a fresh read through that selected node.
8. `006ED9E5`: compare the new `s(EDX)` with zero.
9. `006ED9E9`: zero repeats `006ED9E0`; nonzero falls through.
10. `006ED9EB`: plain `RET`.

The raw bytes at `006ED9DD..006ED9DF` are `8D 49 00`, independently decoded
as `LEA ECX,[ECX]`. They lie within the explicitly admitted 28-byte span
but have no saved listing entry. The entry's direct-return branch, its
unconditional jump, and the loop backedge all bypass them. Thus this local
entry-flow model reaches 25 bytes / ten instructions and excludes exactly
those three bytes. It neither proves that no external entry can target them
nor attributes the saved listing omission to an unqueried analysis property.
Their full-span bytes remain pinned; they are not silently discarded or
promoted to ordinary executed code.

## Complete 006ED9F0 schedule

Use `l(V)=DWORD[V+0]` and the same byte test `s(V)`.

1. `006ED9F0`: EAX=incoming ECX.
2. `006ED9F2`: EDX=`l(EAX)`.
3. `006ED9F4`: compare `s(EDX)` with zero.
4. `006ED9F8`: nonzero branches directly to `006EDA0A`.
5. `006ED9FA`: `LEA EBX,[EBX]`, encoded as `8D 9B 00 00 00 00`.
6. `006EDA00`: EAX=EDX.
7. `006EDA02`: EDX=`l(EAX)`, freshly read from that selected node.
8. `006EDA04`: compare the new `s(EDX)` with zero.
9. `006EDA08`: zero repeats `006EDA00`; nonzero falls through.
10. `006EDA0A`: plain `RET`.

All 27 bytes are reachable in the permissive local flow model. The six-byte
LEA executes once when the initial child flag is zero; the backedge targets
the following instruction and does not repeat it. LEA reads and writes the
EBX register as an identity operation. It does not access memory at EBX or
change flags. Its encoding and presence are retained separately from the
semantic fact that EBX's value is preserved.

## Entry, results, flags and backing

The only semantic incoming register is ECX. Incoming EAX is replaced at the
first instruction, and incoming EDX at the second, before either old value
is used. ECX retains its incoming value on ordinary paths. ESI, EDI and EBP
are untouched. EBX's value is preserved by both bodies, with the explicit
reachable identity operation in the second. Neither function pushes, pops,
adjusts ESP, or reads a public stack argument. With S=entry ESP, plain RET
consumes only the current return address and leaves ESP=`S+4`.

On direct return, EAX is N0 and EDX is its first loaded child. On a return
after one or more loop iterations, EAX is the last child previously tested
zero, and EDX is its freshly loaded child whose flag tests nonzero. The
helper never tests N0's own flag. It returns the predecessor of the stopping
child, not the stopping child. A valid ordered-tree interpretation would
make these right/left endpoint traversals, but ordering, a tree type and
minimum/maximum semantic identities remain unproved.

The returning comparison is always `CMP byte[EDX+31h],0` with a nonzero
byte. RET preserves its arithmetic flags: ZF=0, CF=0, OF=0; SF and PF follow
the tested byte. No incoming arithmetic condition is used. There is no x87
operation, local EH, callback, allocation, release or direct memory store.

The initial N0 directly needs only the link DWORD: a 12-byte highest-address
span for `006ED9D0`, or four bytes for `006ED9F0`. Every tested child needs
the readable byte at `+31h`, giving a 50-byte highest-address span. A child
selected by a zero flag must additionally provide the next link DWORD. These
are accessed offsets, not a proved allocation/object size, ownership model,
implicit type construction or blanket initialization requirement.

There is no null, range, alignment, membership, cycle or lifetime guard. Any
nonzero flag ends traversal; it need not equal one. Each loop can fail to
terminate on a zero-flag cycle. A link or flag may overlap other data or even
stack backing, but the helper makes no writes to alter such aliases locally.
The exact sequence of link read then child-flag read is preserved; no
consistency guarantee for external concurrent mutation or faults is added.
A normal return still requires a usable current return address. There is no
rollback or exception policy supplied by the body.

## Accepted caller delivery

Only the accepted `006EE5D0` receipt was used. Its 637 saved bytes plus
54-byte continuation remain a 691-byte / 234-operation candidate; the 13
following INT3 bytes remain separate. No Native caller bytes were reopened.

| Accepted site | Helper and delivery | Result use |
| --- | --- | --- |
| `006EE6B3` | `006ED9F0`, ECX=replacement Y, no stack arguments; EBX=retained H, ESI=P, EDI=Y, EBP=R | Store EAX to `[retained H+0]` without a post-child header reload |
| `006EE6D5` | `006ED9D0`, ECX=replacement Y, no stack arguments; the same nonvolatile roles | Store EAX to `[retained H+8]` without a post-child header reload |

Each site is reached only after its endpoint comparison succeeds and the
replacement's `+31h` byte tests zero in that accepted schedule. The helper
itself does not repeat the replacement's own flag test. The raw plain RET
and register preservation meet the local no-argument caller requirement;
the exact EAX result is now directly evidenced. Incoming EAX/EDX values not
specified by the caller receipt need no invented interpretation, since
both are discarded locally. Faults or infinite traversal prevent the
caller's subsequent store; no whole-loop or runtime equivalence follows.

## Smallest candidate Source interface

For either entry, the smallest placement contract is:

```cpp
void* __fastcall helper(void* actual_node);
```

This is a provisional Source interface suggestion, not an implementation or
Native class declaration. The one pointer occupies ECX, the physical EAX
result is exposed, and no stack argument or dummy EDX placement is needed.
No raw `noexcept` promise is warranted. A future complete physical emission
review must separately account for the first body's skipped three bytes
and the second's reachable six-byte LEA; operation spelling alone does not
guarantee the same encoding. No Source implementation is added in this packet.

## Current Source comparison and receipt versions

The newly admitted link-repair Source remains the actual two naked fastcall
implementations with receiver/unused-EDX/stack-pivot placement, pointer result,
three RET4 arms each, no calls and no raw `noexcept`. Its current header/body
hashes match the accepted Source candidate. The latest primary report records
exact emitted 78/82 bytes, 30 operations each, zero relocations/local EH and
unique Core definitions. They are absent from the application map and have
no production consumers. This is current Source build evidence, not execution
or a dependency that makes these newly audited small helpers implemented.

The companion owned-message Source was inspected directly. It captures a
pointer-slot address before profile publication, then reads its value; keeps
the strlen argument across allocation; increments the 32-bit size without an
overflow guard; publishes nullable malloc output before branching; rereads the
same slot after publication for `strcpy_s`; ignores that result; uses a DWORD
read-modify-write zero on the initial-null path; and finally writes ownership
DWORD one. It returns the receiver with RET4 and has no raw `noexcept`.

Its latest primary emission is **80 Source bytes / 33 operations for 78
Original bytes**, using qualified current CRT calls. The selected Source
context provides the strlen thunk and UCRT heap/string imports. The new
constructor itself is unselected in the application map, so those existing
bindings do not prove an actually selected new caller operand or Native CRT,
allocator, failure, fault, alias, EH, type or throw identity. It creates no
producer, default storage, callable Native profile or production owner.

The registered batch ran `2026-10-09T13:39:47Z..13:40:03Z` and passed its three
existing checks. Its receipts retain 61 repository input pins, four artifact
pins, nine complete objects and ten unique Core roots. All 61 current
canonical pins replay, including the separately recorded CRLF physical form
of one readiness report. The four current Core/executable/map/test-log files
also match their recorded hashes and stayed stable during these reads.
Hashing those artifacts did not rerun the build/tests or reparse the graphs.

The latest review reports all 20 typed-owner code/relocation contracts and
all eight EH payload/relocation contracts unchanged. The prior default
migration's constructor 60-byte/flags-1/max-state-3 terminate context and
private helper 36-byte/flags-5/max-state-0 context therefore remain qualified
current Source evidence, not Original EH proof.

All 140 prior worker input pins replay historically. Against this packet's
baseline, exactly five changed paths are CMake and the names/reconstruction
shards for `006e` and `00bf`. The older 57-input default-primary receipt
replays historically but its CMake pin is no longer current. Both earlier
candidate documents changed during admission; the four candidate Source
header/body pins remain unchanged. Both latest primary document pins match
current files. Historical candidate, pre-registration, old CMake/ledger and
current admission domains are kept separate.

The report pins 150 current canonical/physical repository inputs, the live
and PE captures, CFG records, Source/context replays and this document.
No Source/CMake/ledger/Ghidra edits, build, test, probe, new function/byte
credit, Original ABI or game validation is claimed. The 691-byte parent,
`006EDA20`'s assertion provider, Native EH and production storage remain open.
