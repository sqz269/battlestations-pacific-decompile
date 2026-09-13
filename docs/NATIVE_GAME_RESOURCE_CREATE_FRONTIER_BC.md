# Native game-resource creation frontier BC

This read-only packet identifies one ready **four-body actual-storage
construction packet**, totaling 465 original bytes. It does not complete native
resource loading, cache ownership or teardown. Existing typed resource readers,
parsers and retained models remain valid implementations at their own interface.

The [report](../reports/native_game_resource_create_frontier_bc.json) records
six complete bodies, 1,197 bytes matched between live Ghidra and the installed
PE, eight auxiliary vtable/EH spans, five FH3 maps and 33 verified direct
call/tail-transfer rows. All six bodies have zero current listing gaps. Every
live CLI batch verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`; binary SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Worktree evidence begins at commit `a4aa7914`; complete bytes, listings,
source pins and the peer manager report are retained under ignored
`local/resource-create-frontier-bc/`.

| Body | Inclusive span | Bytes | Original ABI / disposition |
| --- | --- | ---: | --- |
| Game factory create | `0071b870-0071b8ce` | 95 | ECX factory unused; name-header pointer stack; EAX allocation/null; RET4; ready |
| Game resource construct | `0071b810-0071b869` | 90 | ECX raw 74h allocation; name pointer stack; EAX same allocation; RET4; ready |
| Resource base construct | `00b88260-00b88318` | 185 | ECX raw 44h-or-larger allocation; name pointer stack; EAX same allocation; RET4; ready |
| Default factory create | `00b88340-00b8839e` | 95 | ECX factory unused; name pointer stack; EAX allocation/null; RET4; ready |
| Game resource destroy | `00718810-0071886f` | 96 | ECX actual resource; tail JMP to base destructor; dependency frontier |
| Resource base destroy | `00b88430-00b886ab` | 636 | ECX actual resource; RET; FH3 frame; dependency frontier |

Names are descriptive hypotheses. The default creator is still named
`FUN_00b88340` in Ghidra. This packet changes no Ghidra names, body definitions,
production code, tests or build registration.

## Construction contract

`B88260` first stamps `CEB130`, writes reference count 1, and compares
`object+8` with the supplied name header **before** zeroing that header. It then
stamps `D63228`, arms state 0 and clears name length/data. Equal headers skip
the copy entirely; preserve this destructive self-name case. Otherwise it reads
the source length for `41DD40(destination+8,length,1)`, then rereads source
length, current destination length, current source data and current destination
data for the conditional `BF7680` copy. Do not snapshot the header or infer a
C-string length. Only after the copy succeeds are `+10..+40` initialized to zero,
including the six positive-zero float words at `+28..+3C`.

`71B810` calls that base first, stamps `CFD8CC`, then zeros the three pointer
triplets at `+48/+4C/+50`, `+58/+5C/+60` and `+68/+6C/+70`. The words at
`+44`, `+54` and `+64` retain their incoming bits. They are not permission to
construct a current STL container or initialize an invented iterator proxy.

`71B870` allocates exactly 74h through the existing `BF681B` allocation
contract and calls `71B810`; `B88340` allocates 44h and calls `B88260` directly.
Both retain their explicit allocation-null return branches. Their saved ECX
slot is not a second allocator argument. Neither creates a reader, inserts a
cache entry, registers a singleton or publishes the returned resource globally.

Matched factory data proves slot `CFD850+4 -> 71B870` and default-factory slot
`D63060+4 -> B88340`, although current live xref queries report no references to
either creator. The default factory is a different owner; its lifetime selectors
`B7D290` and `B7D9A0` remain incomplete.

## Exception states constrain the implementation

| Owner | FH3 information / map | Reached cleanup |
| --- | --- | --- |
| `71B870` | `DB3C4C / DB3C44` | State 0 calls `C84FA0`, freeing the saved allocation through `BF65AC` |
| `71B810` | `DB3C20 / DB3C08` | Entry state stays -1; its three static map rows are not reached by a state update in this body |
| `B88260` | `DFB9F4 / DFB9EC` | State 0 calls `CC2590 -> BD30F0`, restamping only the ref-counted base |
| `B88340` | `DFBA20 / DFBA18` | State 0 calls `CC25B0`, freeing the saved allocation through `BF65AC` |
| `B88430` | `DFBA64 / DFBA44` | States 3/2/1/0 unwind through hierarchy array, primary array, name and ref-counted base helpers |

There is no name-buffer cleanup action when `B88260` construction fails.
Its later fields remain unwritten, and the outer factory subsequently frees
the allocation. Do not add a name rollback or invoke a full resource destructor
on this path. The two allocation cleanup funclets currently omit their trailing
`POP ECX; RET` from their Ghidra bodies; the full two-byte tails were verified
in the matched auxiliary spans. No mutation was made to repair them.

## Successful-resource ownership remains a real dependency

`718810` frees and clears derived triplets in `+68`, `+58`, `+48` order,
without touching pointee references, then tail-calls `B88430`. The base destructor
decrements each primary-array item's reference count and invokes slot 0 only
at zero. It does not guard null item entries. Even with no items, it calls
`4C1400` and `B801C0` to erase the resource name from the actual manager cache.
That operation cannot be replaced with an empty success callback.

Hierarchy records additionally require `B88180` and the actual pool at
`1090238/1090250/1090254/1090260`. The base destructor retains signed-negative
array branches, separate array cleanup states and conditional name return through
`419CC0/BD1510`. Its array unwind helpers `B87B20/B87B40`, cache services and
finite item deletion profiles remain incomplete dependencies. The audited derived
destructor alone does not establish a usable resource lifetime.

The neighboring manager audit owns `4C1400/B80720`; its implementation worker owns
`B7F290` and `native_resource_cache_pair.*`. That audit records factory slot+4
creation, manager+24 publication, resolved-name reader construction/stream attach,
root dispatch, cache-pair insertion using the original requested name, and return
of a captured resource after cleanup. This packet does not re-audit those bodies.

`structured_reader.cpp`, `structured_resource_registry.cpp` and
`structured_model.cpp` already implement typed reader/node behavior, Mesh/Note/
GroupParams decoding and retained root/hierarchy/bounds traversal. They are not
missing. Their host storage and owning payloads cannot be cast to the actual
74h resource, native reader/node objects or raw parser-result pointers.
`GameResourceManager` remains a semantic parser-map owner. The existing `7188A0`
and `B80D70` load wrappers likewise do not complete their native `B80720` service.

## Ready implementation packet

Claim `B88260`, `71B810`, `71B870`, `B88340` and new
`native_resource_construction.hpp/.cpp`, plus its own report/document. Implement
base construction first, then derived construction and the two creators in one
bounded packet. No files or addresses overlap the cache-pair worker or current
process-publication work. Coordinate ledger/name/CMake edits with the integrator.

Reuse existing `NativeStringRawPoolContext`, raw `41DD40` resize,
`destroy_native_ref_counted_base_00bd30f0`, and the established source
allocation/free service. Preserve the original nonzero `BF7680` overlap domain
through fixed CRT `memmove`; the current `BE0A30` copy fragment uses `memcpy`
and is not an overlap-complete substitute. Record the zero-byte/pointer-domain
boundary rather than porting a CRT algorithm. A new source interface must preserve
partial state and returned raw ownership without claiming original FH3/SEH ABI.

Run the strict Win32 build and existing checks. Only if a concrete risk requires
new coverage, use one focused capsule for initialized-byte masks, self-name alias,
post-allocation header reloads and propagated allocation exceptions. A broad new
suite, dummy owning resource, or stubbed process success would not advance the
actual resource creation path. This discovery has static evidence only; source,
build, fixture, ABI and gameplay validation are not claimed.
