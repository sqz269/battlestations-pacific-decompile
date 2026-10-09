# Raw mission Lua variant pair child: 006EDA20

The complete saved body `006EDA20..006EDA82` is 99 bytes / 38 instructions.
Every live byte matches the installed PE, and every independent decoded
instruction has a saved listing entry. There are **one physical CALL and one
external tail JMP** to `00BF6713`, plus two plain `RET` instructions. The saved
metadata's call count of two is retained separately from the physical CALL count.

The receiver is an address of two adjacent DWORDs. Word `+0` is read as a zero
guard; all explicit pair writes target word `+4`. The latter can be changed
repeatedly during traversal. No direct semantic use of incoming EDX is present,
although the unopened assertion child can receive incoming registers. This
resolves the owned mutation width and local register-use question; it does not
prove child effects, valid tree structure, termination or an Original ABI bridge.

## Scope and evidence

Packet `cc12_lua_variant_pair_advance_child_ABI_readiness`, fixed published
baseline `cf257e747e130480f385233b5d4b2abcaebbb0ec`. Only this document and
`reports/cc12_mission_lua_variant_pair_advance_child_ABI_readiness.json` are
changed. The sole Native body admitted and leased is `006EDA20`.

The configured existing `C:/Users/sqz269/bsp.gpr` and program
`/battlestationspacific.exe` were reused. Each live CLI batch verifies project,
program, `x86:LE:32:default` and image base `00400000`; configuration/existence
does not establish a stronger server-side absolute-project-path identity.
Live and saved function counts are both 64,729.

- Body SHA-256: `2f2b51ef06aedbbaca0ccb3c723439fe65cb551e56a110f29bbcf8efcaec210e`.
- PE RVA and file offset: `002EDA20`; span length: 99 bytes.
- Installed PE SHA-256: `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
- Saved metadata: 38 instructions, 13 basic blocks, 18 edges, call count two.
- Complete local flow model reaches all 38 instructions / 99 bytes when the
  ordinary CALL may return. Both loops and the tail transfer remain explicit.
- No omitted listing instruction, hidden continuation, adjacent byte read,
  indirect transfer or jump-table read is present in this packet.

The sole external target `00BF6713` was queried only for metadata: 16 bytes /
9 instructions / one recorded child call. Its body, child bodies, callers,
handlers, profile/table/string data and neighboring bytes were not opened.
No Ghidra body, prototype, name, no-return flag, flow or AddressSet was changed.

## Entry frame and child transfers

Let `S` be entry ESP and `B` entry ECX. `B+0` and `B+4` denote actual memory
words, not captured immutable values. Let `a(X)=[X+0]`, `p(X)=[X+4]`,
`b(X)=[X+8]`, and `s(X)=byte[X+31h]`. These offset names are descriptive;
the packet does not establish a C++ object type or tree invariant.

`PUSH ESI` at `006EDA20` places the saved incoming ESI at `S-4`.
`MOV ESI,ECX` retains `B`; there are no locals or public stack arguments.
Normal owned work runs at ESP=`S-4`. Each local return pops the current
saved slot into ESI, then plain `RET` consumes the entry return address,
leaving ESP=`S+4` if the stack remains valid. EBX/EDI/EBP are not written
locally. Preservation across the external CALL remains a child contract.

| Site | Physical transfer and state | Returning continuation |
| --- | --- | --- |
| `006EDA28` | `CALL 00BF6713` when current DWORD `[B]` is zero. No arguments are pushed. ECX and ESI are `B`; EAX/EDX and EBX/EDI/EBP retain incoming values locally. Child entry ESP=`S-8`, return=`006EDA2D`, next stack word is saved ESI. | Requires a balanced no-argument return and usable ESI=`B`. Reload `[B+4]`; do not recheck `[B]`. |
| `006EDA37` | `JMP 00BF6713` when the freshly loaded node's `s` byte is nonzero. The preceding POP has already restored current saved ESI and ESP=`S`. EAX is that node; ECX/EDX are incoming values or prior assertion-child results. No local ECX reset occurs. | A compatible returning target consumes the original caller's return address. There is no local post-tail continuation or local return here. |

There is no null test for `B`, the node at `[B+4]`, or any subsequently loaded
link before dereferencing it. A returning first assertion may change either
pair word or reachable memory. It can leave word `+0` zero: that condition is
not retried. An assertion that does not return prevents its continuation;
no no-return policy is inferred from its name, current comments or decompiler.

Incoming EAX is not semantically read by owned instructions before replacement
at `006EDA2D`; it still reaches the optional first child unchanged locally.
Incoming EDX is only passed through locally until `006EDA73` defines it.
All owned EDX-based addressing follows that definition. Incoming condition
flags are overwritten by the first CMP; there is no x87 or local EH sequence.
No ABI semantics for otherwise passed-through registers are imposed on the
unopened child.

## Ordered traversal and mutation

The initial guard is `CMP DWORD [B],0`; after the optional returning CALL,
`006EDA2D` reads the **current** `[B+4]` into EAX (`N`). `006EDA30` tests
`s(N)`. Any nonzero byte takes the restored-frame assertion tail. With
`s(N)==0`, `006EDA3C` reads `Q=b(N)` into ECX, then tests `s(Q)`.

When `s(Q)==0`, the descent arm performs these exact operations:

1. `006EDA45`: read `A=a(Q)` into EAX; test `s(A)` at `006EDA47`.
2. If nonzero, proceed directly to the final store. Otherwise the physical
   `LEA ECX,[ECX]` at `006EDA4D` is executed before the first loop iteration.
3. `006EDA50`: assign ECX=EAX; `006EDA52`: read `EAX=a(ECX)`; then test
   that new EAX node's `s` byte. Zero repeats `006EDA50` via `006EDA58`.
4. `006EDA5A`: write current ECX to `[B+4]`; restore ESI and `RET` at
   `006EDA5E`. No intermediate pair write occurs in this descent arm.

Thus the register retained in ECX is the last selected zero-flag node;
the EAX used for the stopping comparison is its loaded nonzero-flag child.
This is consistent with a successor traversal's right-subtree/left-chain
arm, but a valid tree and a unique successor are not established.

When `s(Q)!=0`, the ascent arm instead preserves this schedule:

1. `006EDA5F`: read `P=p(N)` into EAX; `006EDA62`: test `s(P)`. Nonzero
   skips the loop and writes P at the final store.
2. `006EDA68`: reload **current** `[B+4]` into ECX; compare it with freshly
   read `b(P)` at `006EDA6B`. Inequality skips the intermediate store.
3. Equality executes `006EDA70`: write P to `[B+4]`; `006EDA73`: copy
   P to EDX; `006EDA75`: read **current** `p(EDX)` into EAX, after the
   pair write. No earlier capture of this parent link is interchangeable.
4. `006EDA78`: test the new EAX node's `s` byte. Zero branches back to
   `006EDA68` through `006EDA7C`, reloading the pair word again.
5. All ascent exits reach `006EDA7E`: write current EAX to `[B+4]`,
   restore ESI, and `RET` at `006EDA82`.

The two physical backedges are `006EDA58 -> 006EDA50` and
`006EDA7C -> 006EDA68`. There is no iteration bound, cycle detection,
progress check or rollback. All byte tests distinguish zero from any nonzero
value; no flag is required to equal one. The body does not read a color byte.

## Return values, flags and alias boundaries

The decompiler's void signature does not hide the actual surviving registers:

| Local exit | EAX | ECX | EDX | Last arithmetic flag producer |
| --- | --- | --- | --- | --- |
| Descent `006EDA5E` | Nonzero-flag stopping child | Final node stored in pair word `+4` | No local write; incoming/prior-child value | Byte CMP at `006EDA47` or `006EDA54` |
| Ascent `006EDA82`, initial parent flag nonzero | Initial loaded parent | Q from `b(N)` | No local write | Byte CMP at `006EDA62` |
| Ascent exit on link inequality | Current parent candidate | Latest pair word loaded at `006EDA68` | Last intermediate parent, if `006EDA73` ran; otherwise incoming/prior-child value | DWORD CMP at `006EDA6B` |
| Ascent exit after a step reaches nonzero flag | New parent loaded after the intermediate pair store | Pair value read before that intermediate store | Parent written by the last intermediate store | Byte CMP at `006EDA78` |

MOV/LEA/POP/RET do not replace those arithmetic results. Both local returns
therefore have ZF=0. Byte-compare exits also have CF=0 and OF=0; the DWORD
inequality exit retains the actual pointer-word subtraction flags. This is
a local physical observation, not an Original-to-Source flags guarantee.
The external tail's final registers and flags depend on its unopened target.

The direct pair footprint is a readable DWORD at `+0` and readable/writable
DWORD at `+4`. Node operations read DWORDs at `+0/+4/+8` and a byte at
`+31h` along the selected paths: the highest byte requires at least 50 bytes
of addressable backing where used, not a proved allocation or object size.
No explicit node-link, node-flag, color, allocation or release operation exists.

Arbitrary aliases are not excluded. A `[B+4]` write can change a later
link/flag read, including `p(P)` loaded immediately after the intermediate
store. It can also overlap stack storage in an unconstrained raw call, changing
the subsequently popped ESI or return data. Distinct pair words do not create
a transitive guarantee that word `+0` is unchanged: the assertion child and
unproved external aliases remain separate effects. A fault or interruption
after an intermediate write leaves that write in place. No exception cleanup
or memory transaction is supplied by this body.

## Effect on accepted callers

Only accepted caller reports were read; neither Native caller was reopened.

At `006EE5D0`'s accepted `006EE64E` call, ECX is the address of adjacent
F/N argument words and no stack argument is pushed. This child explicitly
changes the N word at `+4`; it does not directly store the F word. The parent's
immediate overwrite of child EAX remains valid, and its later reload of N
can observe a different node. The parent's `I != N0` transplant arm must
remain: the pair mutation is now directly evidenced. This audit does not
prove that arm is taken for a particular real structure or that every parent
nonvolatile register is preserved across the unopened assertion child.

At `006EE960`'s accepted `006EE9F9` call, ECX similarly addresses the actual
F/N argument cells. The parent subsequently pushes its cached pre-advance
F/N into `006EE5D0`, then reloads the pair cells after that child returns.
The owned local schedule supports this distinction. It does not establish
finite-range termination, ordering, valid ownership or whole-loop equivalence.

## Current Source and receipt qualification

There is no admitted exact Source implementation of `006EDA20` at the fixed
baseline. The BF6713 ledger row maps to the actual typed member
`NativeInputDeviceRuntime::invalid_parameter_00bf6713()` in
`src/native_input_device_runtime.cpp`; its declaration overrides a virtual
zero-argument Source interface in `NativeInputEnumerationCalls`. The concrete
body calls current `_invalid_parameter_noinfo()`. It has no `[[noreturn]]`
or `noexcept` annotation. This Source member requires its actual runtime
object and is not a raw zero-argument Native thunk or receiver-layout bridge.

The runtime report's validation at `c2b1437830ed15b33570a84b84ea73438b8a882f`
includes historical Source/UCRT returning-handler fixture evidence. No fixture
or build was rerun here. Both historical header/body snapshots were replayed;
their whole-file hashes differ from current Source. The receipt's aggregate
Source-tree hash is retained as historical, not asserted as a current or
individual-file attestation. The current wrapper body and typed declaration
were inspected directly. The selected BF6713 ledger record is unchanged
despite drift in its whole shard; the names shard has no exact BF6713 record.

Inherited exception/string Source receipts remain version-qualified. All 12
recorded Source artifact hashes replay in canonical Git or documented CRLF
form; six entries differ from current whole files. The earlier owner
constructor's try/catch slice differs from current RAII cleanup, while the
later cleanup-correction receipt reproduces the current constructor slice.
The current raw cleanup migration is a further whole-file change.

Specifically, `destroy_base(NativeLegacyExceptionStorage&) noexcept` now calls
`cleanup_native_allocator_base_00bf6454(&owner,0u)` using its actual existing
40-byte typed owner. The raw MSVC Win32 declaration takes ECX receiver and
an unused EDX word; it has no `noexcept`. Its Source assembly compares the
full control DWORD at `+8` before publishing the profile at `+0`, conditionally
pushes the current `+4` pointer and calls current Source CRT `free`, then pops
the argument into ECX and uses plain RET. It adds no message/control reset.
Only the cleanup include and private helper differ from the inherited loop
packet's baseline; the constructor and RAII arm/disarm order are unchanged.

Three relevant candidate Source pins and the same three primary-review Source
pins match the current canonical files exactly. The primary report's build,
Core membership, emitted-body and typed-consumer/EH observations remain
inherited evidence, not re-executed checks. The existing typed `noexcept`
consumer and its compiler terminate policy do not make the raw adapter
`noexcept` or prove Original CRT/EH identity. No raw 12-byte allocation is cast
to the larger typed owner. None of this supplies BF6713's Native body or
an Original exception/invalid-parameter runtime bridge for this pair helper.

## Replays, completion and holds

All 74 inherited canonical input pins replay exactly at their recorded
revision. Against this packet's fixed baseline, the only changed paths are
the BF names/reconstruction shards and `src/native_legacy_exception_owner.cpp`.
Historical hashes are retained separately from freshly pinned current bytes.
The accepted loop document hash also replays. The report records current
canonical and physical forms, all Native/source captures, and this document.

This is a complete read-only ABI/readiness audit of the owned 99 bytes.
Source implementation, ABI bridge and runtime admission remain held on
actual backing/alias contracts, traversal validity and the external invalid-
parameter provider's register/stack/failure behavior. The already identified
small sibling traversal/rotation/payload helpers remain independent future
packets; their Native bodies were not opened here. No additional body is
needed to finish this bounded report, and no outside scope was requested.

No C++/CMake/ledger/Ghidra changes, build, test, probe, annotation, Source
function/byte credit, Original ABI or game-validation credit is claimed.
