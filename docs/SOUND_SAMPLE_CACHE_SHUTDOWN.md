# Sound sample cache shutdown

Addresses: `00A886C0`, `00A88750`.

The derived `D5B460` auxiliary owner contains weak sample pointers and owned
pooled string keys. Destruction clears those keys, frees the tree, and unregisters
the singleton; it does not release a sample reference or call FMOD. The executable
projection is in `sound_sample_cache_shutdown.cpp`. Descriptive names are hypotheses.

| Routine | Native ABI | Coverage |
|---|---|---|
| `00A886C0` | ECX=this; no stack args; RET at `A88749` | complete game-specific normal and EH ownership flow; native STL/SEH supplied by C++ contracts |
| `00A88750` | ECX=this; stack flags DWORD; EAX=original this; RET4 at `A8876B` | complete scalar-wrapper control flow |

Pass the existing `SoundAuxiliaryTreeOwner`, the matching
`SoundSampleRuntime::sample_cache_context()`, and canonical
`SoundOwnerLifetimeBindings`. No additional opaque cleanup host is introduced.
The owner must have completed `create_sound_auxiliary_tree_owner_00a88650` and
retain a live tree. It cannot be destroyed twice. For scalar deletion, transfer
the factory's `unique_ptr` with `release()` before passing flags bit0=1.

## Native ownership evidence

Producer `A88650` first publishes/registers the base via `A880E0`, then writes
`D5B460`. Its `A88270` allocation is a 1Ch tree head: node+19 isnil=1,
left/parent/right self-links; owner+8=head and owner+C=count=0. These agree with
the existing `SoundAuxiliaryTreeOwner` schema, which represents native owner+4
tree storage with a `unique_ptr<std::map<...>>`.

`A886C0` writes `D5B460` before calling `A82C00`. That existing routine repeatedly
dispatches the current first node through the current vtable+8 and writes
`F8BBE4=0` only after the tree empties. The D5B460 slot8 implementation `A83E80`
unlinks a node, returns its owned key to pooled storage, frees the node, and
decrements the explicit count if nonzero. The sample pointer is weak.

After clear, `A8870B` calls `A88490` with the whole begin/end range, then
`A88714` frees the head. The continuation stores owner+8=0, owner+C=0, EH state=-1,
and calls `A88180`. That existing base unregisters the current `F8BBE8` value
(which need not equal this), clears the global, leaves its captured critical
section, and finishes at vtable `CE3818`.

`A88490`'s whole-range branch calls `A88400` on head.parent, then resets the
head links and count. `A88400` recursively visits right, releases the current
node's pooled key, frees that node, and loops over left: reverse inorder. Its
node+14 sample word is never released. The projection extracts nodes from
`std::prev(tree.end())`, explicitly releases each key, and destroys the node
before advancing. It preserves count until all keys are gone, zeros count before
freeing head storage, clears the owner tree pointer, and zeros count again.

## Exception order

`CB6293` loads FuncInfo `DEC43C` (`19930522`, maxState=2, unwindMap=`DEC42C`):

| Current state | Next | Cleanup |
|---|---|---|
| 1 (while `A82C00` runs) | 0 | `CB6288` sets ECX=owner+4 and tail-jumps to `A885E0` |
| 0 | -1 | `CB6280` loads owner and tail-jumps to `A88180` |

`A885E0` performs the same whole-range `A88490`, head free, head/count-zero
sequence as the normal path. The implementation arms base cleanup, then tree
cleanup; a failed clear therefore destroys remaining pooled keys and tree
storage before unregistering. It never performs the clear routine's unfinished
`F8BBE4=0` store on that path. Before normal base teardown, the guard is disarmed,
matching the native state=-1 store before `A88734`.

The scalar wrapper calls the destructor, checks only flags bit0, conditionally
frees owner storage, and returns the captured original pointer. Its native
`_free` call is cdecl: missing bytes `A88765..A88767` are `ADD ESP,4`; final RET4
removes the wrapper flags. The host uses `delete` to match the existing factory's
C++ allocation. The returned pointer is dangling after deletion. A destructor
exception propagates before the conditional storage release.

## Calls and stored-function boundaries

| Call site | Native callee | Ghidra containing function at investigation | Meaning |
|---|---|---|---|
| `A886EE` | `A82C00` | `A886C0` | clear weak keys and reset shared word |
| `A8870B` | `A88490` | `A886C0` | destroy full remaining key/node range |
| `A88714` | `BF65AC` | `A886C0` | free tree head |
| `A88753` | `A886C0` | `A88750` | derived destructor |
| `A88760` | `BF65AC` | `A88750` | conditional owner storage free |
| `A885F6` | `A88490` | `A885E0` | EH tree full-range destruction |
| `A885FF` | `BF65AC` | `A885E0` | EH tree head free |
| `A884DA` | `A88400` | `A88490` | native STL reverse inorder key destruction |

The known truncated-tail call is separate: `A88734 -> A88180` had
`no_ghidra_function` at investigation; control flow is the continuation of
`A886C0`. Stored body ended `A88718`, while disk/live bytes and balanced epilogue
establish inclusive end `A88749`. `A885E0` likewise stored end `A88603` but actual
inclusive RET is `A88613`. `A88400` had an internal missing span
`A88441..A8844B` (stack repair and left-loop test). `A88750` had the internal
three-byte stack-repair gap above. The primary integrator owns Ghidra repair,
annotation and export refresh; this worker performs no Ghidra mutations.

## Verification and limits

Read-only CLI batches verified project `bsp`, configured project file
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, x86 and base
`00400000`. Disk/live parity covers `A886C0..A8876C`, `DEC42C..DEC463` and
`CB6280..CB629C`; SHA256 values and previous names are in the report.

The Release MSVC Win32 build passed with both existing tests (`reconstructed_math`
and `native_math_differential`); all eight seed spans matched disk. The call-site
audit checked eight rows with no failures. The focused local probe passed
normal clear and interrupted clear with three keys, exact remaining-key order
and count observation, shared-word behavior, weak sample-reference preservation,
singleton removal, and scalar flag behavior. It deliberately changes the dispatch
profile in the first key-release callback so the next clear callback throws;
this exercises the C++ ownership path without inventing a game payload cleanup.

This is a C++ behavior interface, not native owner/tree layout, ABI or SEH
compatibility. Standard map extraction updates host topology before key release;
the native recursive eraser does not rebalance/unlink each node that way.
Node/head allocations and deallocations are host library operations. Native tree
shape, invalid-iterator diagnostics, allocator callbacks inspecting topology,
corrupt input, structured exceptions and double exceptions are not reproduced.
Key release is the existing nonthrowing `NativeStringStorage` contract. No FMOD
or in-game shutdown has been validated by this packet.
