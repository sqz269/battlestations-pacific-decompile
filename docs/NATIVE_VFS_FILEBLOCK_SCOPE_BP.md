# Native VFS FileBlock scope BP

Addresses: `00BE0980`, `00BDC9B0`. Both complete ordinary bodies are reconstructed
over the actual manager, gate list, current-name header and observer. The source
exposes explicit entry and exit calls; an acquired invocation is not a balanced
RAII block and its destructor does not leave the scope.

| Routine | Coverage | Native ABI | Complete body |
|---|---|---|---|
| `enter_native_vfs_fileblock_00be0980` | complete ordinary body | ECX captured manager; stack name, gate low byte; `BE0A26 RET 8`; no stable semantic result | `BE0980..BE0A28`, 169 bytes, 66 instructions |
| `leave_native_vfs_fileblock_00bdc9b0` | complete ordinary body | ECX captured manager; stack name; `BDCA72 RET 4`; no stable semantic result | `BDC9B0..BDCA74`, 197 bytes, 72 instructions |

The complete live listings and installed PE bytes agree, without instruction
gaps. Each analysis batch verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, language `x86:LE:32:default`, image base `00400000`.
No Ghidra writes were made. These new explicit-service Win32 C++ interfaces do
not reproduce native stack, exception or RTTI ABI.

## Actual storage and entry order

Existing manager lifetime code produces the current-name header at +C/+10,
depth +14, observation byte +78, gate byte +79, actual list at +7C (head +80,
count +84) and observer pointer +88. The existing gate-list helpers own 0Ch nodes
containing next +0, previous +4 and saved gate byte +8. No projected list or
separate gate stack is introduced. Scope context string storage and the observer
context must share the same actual string/pool domain.

Entry captures head and its previous node, then calls `7F8390` with the address
of the manager's current gate byte. That dependency reads the gate after
allocation. `7FA3A0(1)` performs checked count growth only after allocation;
overflow leaves the allocated node unlinked. The scope adds no compensating free.
It writes captured head.previous to the new node, reloads new-node.previous,
then writes that node's next link. The current gate is ANDed with the argument's
low byte.

If current +78 is nonzero, entry captures current observer +88, reads its current
table and +8 target, and invokes it with the original actual input header.
Observation is independent of the resulting gate value. After a normal callback
return, it increments the captured manager's current depth, then copies the name
into that manager's actual +C header unless source and destination are identical.
After resize it rereads source length, destination count, source data and
destination data in the original order. It preserves the overlap-safe copy.

If the current gate is nonzero, the diagnostic reads the current input data and
substitutes `0109CEF0` for null before calling `4254B0`. The actual body is one
`RET`; no logging side effect is invented. The argument reads are still present.

## Exit order and returning validation

Exit retains its manager receiver across all calls. If observation +78 is set,
it tests input length before loading current observer/table/+C. A nonempty input
is passed directly at `BDC9D3`; an empty input passes the captured manager's
current-name header at `BDC9DE`. Thus a nested observer can borrow actual manager
name storage, and that storage must remain alive if the observer fails.

Only after the observer returns does exit resize current name to `(0,false)`.
If resulting data is nonnull, it copies the current count from actual `CE3A0C`.
It decrements current depth with DWORD wrapping, then conditionally reads the
input data for the RET-only diagnostic under the current gate. It neither checks
for balanced depth nor restores an outer name.

The following distinct captures and checks are preserved:

1. Capture current head and its last node; compare captured last with captured
   head. Empty calls `BF6713` at `BDCA3A`, which can return.
2. Compare that same captured last with **current** head at `BDCA3F`; equality
   calls `BF6713` at `BDCA44`. Restore the gate from the captured last +8 after
   any returning handler, regardless of a head change.
3. Capture current head and current last again. Equality calls `BF6713` at
   `BDCA59`. Erase this newly captured last through `BDAF40` using actual list
   owner and the invocation's stable two-DWORD output iterator.

The dependency performs its own returning owner/end checks. It unlinks and frees
a non-head node before decrementing current count, then writes output node before
owner. Scope code does not collapse those checks, substitute the first captured
last for the erase node, or treat an empty list as an early no-op.

## Concrete observer and failure boundary

`NativeVfsFileBlockObserverDispatch` receives the reached current entry, actual
observer, actual name and stable nested PakRegistry invocation.
`NativePakRegistryFileBlockObserverDispatch` concretely invokes `BB5770`/`BB5910`,
the `D64190` profile's +8/+C entries, through the already reconstructed BO API.
An unsupported reached target throws at that dispatch; it is not rejected before
earlier scope effects. No registry projection, provider retention or origin-VFS
publication capture is added. The scope retains its manager while BO continues
to perform its own original **current** `0109CEEC` rereads.

Neither scope body installs a local FH3 registration, owns a native unwind map
or contains a local cleanup handler. The full listings consist of ordinary
prologues, calls, branches and epilogues. There are no missing-function ranges
inside either owned body. Consumed dependencies retain their separate published
FH3/source exception contracts; their handlers are not attributed to these scopes.

`NativeVfsFileBlockScopeAcquired` owns stable captures, the allocated gate pointer,
separate restore/erase candidates, output iterator, observer arguments and a
nested `NativePakRegistryBlockInvocation`. It must be published before dispatch.
On an escaping C++ exception it records the exact active native call site and
rethrows without rollback or automatic exit. Retain it along with actual manager,
input and context storage. Replay is rejected; destruction while active or failed
terminates. Completed changes and frees remain completed; captured pointer values
do not imply ownership of backing already freed by a dependency.

| Escaping scope sites | Exact source boundary |
|---|---|
| `BE099A` -> `7F8390` | Allocation failure precedes any returned node. Once allocation returns normally, its pointer is captured before count growth. The scope adds no cleanup for the dependency's native invalid-pointer/fault behavior. |
| `BE09A5` -> `7FA3A0` | Known owning `NativeAliasListLengthError` transport can escape after actual node allocation. Count, links, gate, depth and name have not advanced; retain the captured unlinked node. Host RTTI/throw ABI is not native FH3 equivalence. |
| `BE09D2` -> observer +8 | Node is linked, count incremented and gate ANDed before dispatch. Unsupported entry is a known source `invalid_argument` before BO runs. A BO escape can occur after borrowing stable input/outer headers; depth and name have not advanced. |
| `BDC9D3`, `BDC9DE` -> observer +C | Observation precedes all scope clear/depth/gate/pop work. Empty input borrows actual manager current-name storage. A BO escape leaves those scope fields unchanged, while BO mutations may already be complete. |
| `BE09E6`, `BDC9E9` -> `41DD40` | String allocation can escape after preceding effects. The actual manager header remains caller-owned; no speculative copy rollback or outer name restoration is supplied. Existing `NativeStringStorage::release` is noexcept. |
| `BDCA3A`, `BDCA44`, `BDCA59` -> `BF6713`; `BDCA67` -> `BDAF40` | A handler can return or throw after earlier name/depth/diagnostic effects. Preserve both captured candidates and the reached partial state. The erase dependency retains its existing free/count/output order and callback contract. |

BO retains its own mutable `.mpak`, device and dot headers plus resolver/unmount
acquired objects. This does **not** preserve BDD850's stack visitor/copied-name
locals, BE1740's stack canonical/payload/record locals, or backing released by
existing resolver catches. See `NATIVE_PAK_REGISTRY_BLOCK_CALLBACKS_BO.md` for
the exact nested sites. The BP retained-exception probe covers one known source
cleanup throw, not arbitrary native callbacks, dependency unwind or SEH. Raw
pointer faults, concurrent data races and original stack/register-spill aliases
remain outside the explicit-service `/EHsc` domain.

Production wiring must still publish the actual observer route and acquired
scope/BO/context lifetime before dispatch. This packet changes no production
runtime or FileBlock owner/preparation path and does not claim that all nested
failure storage or balanced scope ownership is integrated.

## Verification

The new TU and ignored probe pass MSVC Win32
`/std:c++17 /EHsc /W4 /WX /O2 /fp:strict /MD`. The probe links with
`/MANIFEST:EMBED` against the separately frozen registered BO library:
`02b33f5248e04e595830d66c353883d0b9d66706e37f653a9d8d8f9545c22444`, code commit
`994e06289522017335e3438a0e499523a8777462`. The receipt records equal source-before,
source-after and frozen-copy hashes from the explicitly assigned primary
`build/win32/Release/bsp_core.lib`. All 75 fixture/header/source inputs are copied
and hashed. No main-checkout library or concurrent root build was used.

One ignored actual-storage fixture runs in three process modes:

- Ordinary: real PakRegistry dispatch, resolver/device 42 and FileStore deletion;
  null mount return behavior; publication switches without redirecting captured
  scope effects; nested gate restoration without outer-name restoration; current
  name identity input; returning CRT head repair and unchanged-head checks. The
  latter reaches all three scope checks plus the erase dependency's check.
- Retained observer failure: actual BO erasures and unmount complete, then its
  `BB5A4D` getter throws. Scope records `BDC9DE`, retains nested headers and keeps
  current name/depth/gate/list unchanged. The process retains all borrowed state
  through termination.
- Growth overflow: actual gate allocation precedes the known checked-growth
  exception at `BE09A5`; the unlinked node remains captured and unfreed, with
  count/gate/depth unchanged. The process retains state through termination.

Whole-report call validation covers twelve direct calls in the owned bodies and
five direct incoming call sites. The three virtual sites are explicitly marked
indirect and separately validated against full instruction bytes and the concrete
profile slots; they are not presented as direct CALL edges. No permanent tests,
CMake, shared metadata, Ghidra mutation or production runtime edits are included.
Full integration build and gameplay validation remain with the primary.
