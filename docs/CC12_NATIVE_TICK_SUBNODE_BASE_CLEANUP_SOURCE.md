# Tick subnode base cleanup: Source candidate

Current Source is registered and build-tested. The complete owned Native84/30 schedule maps to an emitted97-byte /33-operation borrower with explicit publication references and RET8. Original ABI, production binding and execution remain unproved.

## Integrator compiled review

Root independently replayed the candidate pins, all thirty Native positions, all thirty-three Source operations and the five normal stack paths, then reviewed the complete emitted object and its indexed relocation graph. The normal MSVC Win32 build passed all three existing checks. Four relocations select the admitted getter and unlink leaf plus Win32 Enter/Leave; the new root has one actual Core definition and is absent from the application map. All eleven prior whole objects remain byte-identical, including all twenty Legacy code/relocation bodies and eight EH payload/relocation contracts. No probe, new test, storage, consumer, forced retention or facade binding was added. The immutable worker report below records its unregistered candidate stage; current admission is recorded in the primary report.

Current evidence: `reports/cc12_native_tick_subnode_base_cleanup_primary_review.json`.

## Worker candidate snapshot

This unregistered MSVC Win32 borrower translates the complete owned
`00875B30..00875B83` schedule. It preserves the comparison before the profile
write, the parent capture before the getter/lock, and the section test before
the parent clear. It calls the existing admitted Source getter and raw unlink
leaf, with current Win32 Enter/Leave imports. It introduces no storage, type,
production caller or virtual-facade binding. Build and admission remain pending.

Baseline: `e1a33b79838f2b0c6778851a0c97411cb4cd8ac8`. The explicit new API is:

```cpp
void __fastcall cleanup_native_tick_subnode_base_00875b30(
    void* actual_node,
    std::uint32_t unused_edx,
    void* volatile& actual_registry_publication_00f878cc,
    void* volatile& actual_manager_publication_01090aa0);
```

ECX supplies the actual node. EDX is an unused placement word, not an instruction
to clear or preserve EDX. Two stack arguments contain addresses of the actual
publication cells. Source `RET 8` consumes those two words. The Native entry
takes no stack argument and ends in plain RET. The definition is naked; the
declaration does not add `noexcept`. The void return type supplies no common
semantic EAX result.

## Accepted whole-body gate

Root's selected capture and its separate approval are:

- `local/cc12_launch_task_cleanup_primary/capture.json` in the primary worktree;
- `local/cc12_launch_task_cleanup_primary/Root_Astra_gate_approval.json` there.

The selected body is 84 bytes / 30 operations, SHA-256
`e4c3de521956ae548c1ddb07fd451b7634933e69c501dddf0d2816f0b5ebbd99`.
The capture identifies `/battlestationspacific.exe` and original-image SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The capture's initial `Root_Astra_whole_body_gate` field remains false; the
separate later Root Astra approval records the full assembly review and binds
this same address, byte count, operation count and hash. The report preserves
both facts, the complete selected body and the selected approval record.

The generic saved `undefined FUN_00875b30(void)` prototype is metadata, not
the receiver ABI contract. The whole listing establishes ECX input, conditional
saves and plain RET. Four physical calls are present: the getter, Enter, the
unlink leaf and Leave; the metadata's two named calls are not a count of all
physical calls. This worker uses the accepted gate and current Source. It opens
no further Native child, caller, handler, profile data, or Ghidra batch and
makes no analysis mutation.

## Owned schedule and Source changes

Let `N` be entry ECX, `P` the selected `N+4` capture, and `K` the returned
registry's `+4` section capture. The names describe these positions only; they
do not establish a Node class or a callable profile.

| Native positions | Effect retained by the Source assembly |
| --- | --- |
| `00875B30..00875B31` | Save ESI once and capture N in ESI. |
| `00875B33..00875B3D` | Read/test DWORD `N+8`, write full DWORD `00CFD99C` at `N+0`, then use the original comparison flags to select the slow path. The MOV preserves those flags. |
| `00875B3F..00875B43` | Only when the first read was zero, read/test `N+C` afresh. Zero skips the entire getter/lock/unlink/parent-clear path. The profile write has already occurred. |
| `00875B45..00875B49` | Slow path begins with PUSH EBX, captures P from `N+4`, then saves EDI. It does not repeat the entry ESI save. |
| `00875B4A..00875B54` | Call the actual Source getter; read its result+4 once into EDI; test K. No parent reload follows the getter. |
| `00875B56..00875B5D` | For nonzero K, push it to current `EnterCriticalSection`, then ADD its raw `+18h` depth DWORD by one. |
| `00875B61..00875B65` | Push captured N, form `ECX=P+1Ch` from captured EBX, and call the admitted raw unlink leaf. The leaf uses RET4. This PUSH ESI is the node argument. |
| `00875B6A..00875B73` | TEST actual current EDI first, then clear full DWORD `N+4`; the MOV leaves the TEST flags for the branch. The clear occurs even if the leaf's own zero-link/count condition skipped mutation. |
| `00875B75..00875B7A` | When the section test was nonzero, decrement current `K+18h`, then call current `LeaveCriticalSection`. |
| `00875B80..00875B83` | Slow path restores EDI/EBX; all normal paths restore ESI. Source replaces Native plain RET with RET8 for its two reference-address arguments. |

After the conditional EDI save, the Source adds exactly three operations:
`PUSH [ESP+14h]` for the M-cell address, another `PUSH [ESP+14h]` for the
F-cell address, and `ADD ESP,8` after the ordinary cdecl getter call. The two
equal displacements refer to different entry words because the first PUSH
changes ESP. The Source body therefore has 33 operations: all 30 Native
positions, with direct provider/import names, local branch labels, changed
RET operand, and these three additions. No emitted Source byte count is claimed.

The extra ADD changes arithmetic flags after the Source getter. The following
section TEST overwrites the flags used by the next owned conditional branch.
This does not prove flags at intervening faults or whole Native child-context
equivalence. The earlier CMP/profile/JNZ and later TEST/parent-clear/JZ pairs
have no added flag-writing operation between their producer and consumer.

## Exact extra stack contract

Let S be Source ESP on entry. `[S]` is the return word, `[S+4]` contains the
actual F-cell address and `[S+8]` the actual M-cell address. These reference
words hold addresses, not copied publication values.

| Point | Source ESP / selected words |
| --- | --- |
| Entry ESI save | `S-4` |
| Slow EBX and EDI saves | `S-12` |
| First added `PUSH [ESP+14h]` | Reads `[S+8]`, leaves `S-16` |
| Second added `PUSH [ESP+14h]` | Reads `[S+4]`, leaves `S-20` |
| Getter entry / normal cdecl return | `S-24` / `S-20`; added `ADD ESP,8` restores `S-12` |
| Enter or Leave entry | `S-20` after section argument and return-word pushes; current stdcall return restores `S-12` |
| Raw unlink entry | `S-20` after captured-node argument and return-word pushes; leaf RET4 restores `S-12` |
| Leaf's conditional EDI save | `S-24`, when its mutation arm saves EDI; its internal contract remains in force |
| Slow restores / all-path ESI restore | `S-8`, `S-4`, then `S` |
| Source normal RET8 | `S+12`; Native plain RET instead gives Native entry ESP+4 |

The empty fast path reaches the common ESI restore without reading either
publication-address argument or calling a child. The slow path passes the
actual F/M cell addresses to the existing getter; publication values remain
mutable and are not snapshotted by this wrapper. Their identities and lifetime
must satisfy that getter's contract.

The added address words must still hold those intended identities when their
selected PUSH reads occur. An early profile write or other aliased field write
can otherwise overwrite them. Selected saved-register and return backing must
also remain intact for normal restoration/return. These are additional Source
stack locations and cannot be assumed to have Native stack identity.

The leaf conditionally spills EDI, which this wrapper uses for K. If raw list,
node or neighbor writes overwrite that spill, the leaf can return a changed
EDI. The wrapper deliberately tests and uses actual current EDI afterward; it
does not reload the registry, restore a private K snapshot, or use returned EAX
as the node. ESI remains the captured node under the ordinary child register
contract. Parent EBX was captured before the getter/Enter and is not replaced
by a later `N+4` value. Current imports and children must preserve the required
nonvolatile registers and normal control-stack backing; unconditional
restoration or universal alias equivalence is not claimed.

## Backing, providers, and failure boundary

The directly selected node fields require accessible backing through `N+0Fh`:
profile write at +0, selected parent read and later clear at +4, first link read
at +8, and conditional fresh second-link read at +C. The raw leaf requires its
actual list at `P+1Ch` through list+0Bh (parent-relative +27h), selected writable
node/neighbor links, and its own signed-count and late-reread contract. No
membership, null-parent, count-zero or link-consistency guard is added.

The getter result is dereferenced at +4 without a new null check. Nonzero K
requires the actual current Win32 section backing and the raw depth DWORD at
K+18h. Depth ADDs wrap as DWORD arithmetic; no clamp or RAII layer is introduced.
Profile `00CFD99C` is an opaque stored word: no table contents, virtual call,
RTTI, allocation extent or Native class identity is inferred.

The actual providers and precedent are pinned:

- `get_native_pending_registry_00875280` is the admitted two-reference ordinary
  Source C++ getter. Its prior compiled root is 250 bytes / 85 operations, with
  current allocation/registration and compiler EH policy. Its first publication
  capture and slow-path late rereads are the existing provider's behavior.
  That Source interface and CRT/FH3 machinery are not a Native drop-in getter.
- `unlink_native_tick_sublist_node_00874e60` is the admitted 83-byte / 29-operation
  raw Source leaf, with actual list in ECX, an unused EDX placement word and
  the captured node as its first stack argument. The caller adds no EDX clear.
  Its conditional EDI spill, alias-sensitive rereads and RET4 remain intact.
- The admitted adapter for Original `00875960[69]` supplies the established
  explicit-publication/Win32-import pattern. It has an additional late node-cell
  reference and RET0Ch. It is a precedent, not a callee of this candidate;
  this cleanup instead captures the node directly from entry ECX and uses RET8.

No owned unwind frame or recovery is added around getter, Enter, raw unlink,
depth changes or Leave. Getter failure policy is inherited from its Source
implementation. A later fault or nonreturn can leave the profile, links, parent
clear or depth changes already published; no automatic outer lock release or
rollback is promised. Native exceptions, hardware faults, reentrancy,
concurrency and external provider ownership remain unproved.

On the normal empty path, EAX/EDX are untouched by the owned instructions and
the final selected `CMP [N+C],0` saw zero. On the normal null-section slow path,
the leaf supplies its own EAX/EDX residuals, then TEST-zero flags survive the
parent clear and pops. A nonnull-section normal path ends after Leave, whose
volatile residuals are not assigned a fixed value. Saved-register restoration
is conditional on the backing/child contracts above. There is no blanket
return-value, final-flags, Native-ABI or runtime-equivalence claim.

## Production and receipt qualification

`NativePilotBotTaskOwnerCalls::base_cleanup_00875b30` remains pure virtual in
`include/bsp/native_pilot_bot_task_owner.hpp:50`; the existing call at
`src/native_pilot_bot_task_owner.cpp:182` dispatches that facade. No implementation
or connection to this new raw function is added. The new Source symbol occurs
only in its declaration and definition, and the file is absent from CMake.
Actual receiver, parent/list producer, publication cells, section owner,
production lifecycle and startup/gameplay remain separate obligations.

The current 65 Source pins of the shared registered build at
`2026-10-09T14:48:26Z..14:48:44Z` are replayed as Source content only. That build
reported 11 complete objects / 13 public roots and three existing checks; the
newly admitted Original-byte credit was 55 for the two Lua helpers plus 153 for
the group initializer. This is the prior 208-Original-byte Source admission,
not credit or compilation for the present cleanup. Its emitted three-root
sizes are 28, 27 and 155 bytes. No build or artifact is rerun here.

Historical getter38, leaf53 and predecessor57 Source pin sets are also replayed
at their recorded revisions and compared with this baseline. Their current
differences are separately reported (CMake, plus an older legacy-owner source
pin in the 53-pin receipt). Current file/Source hashes do not refresh historical
ledger counts or establish current execution.

Validation is static: complete approved body and gate association, all 33
declared operations, the three extra Source operations, stack offsets and
normal-path balances, exact input pins, provider signatures, current symbol
scope and four owned files. No new Ghidra batch, CMake registration, ledger
change, compilation, probe, test or Source admission occurs. Original ABI,
startup and gameplay credit is zero.
