# Native resource hierarchy frontier BE

The next ready packet is **B7D640/B7D7D0/B88180, 290 original bytes**: numeric
reference-array reserve, resize, and hierarchy field destruction. It requires
the existing allocation/free services and `NativeStringRawPoolContext`. It is
independent of the sibling's six resource pointer-array helpers and supplies
the direct field-cleanup dependency of `B88430`.

The hierarchy pool is **distinct actual storage at `0109022C` using existing
88h material-parameter pool mechanics**. Its allocator, constructor, destructor,
slab initializer and trim bodies already have concrete source. The missing work
is the hierarchy owner binding and wrappers, plus an address-specific entry for
the already implemented return algorithm. The string pool supplies the record's
name buffer; it cannot supply the record's slab slot.

The [report](../reports/native_resource_hierarchy_frontier_be.json) contains
complete inclusive code spans, hashes, original ABIs, call rows, all incoming
core/return calls, unwind maps and explicit partial-audit boundaries. Every live
batch used `bsp.py ghidra`, verifying `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, bridge8089. Code/data spans matched the installed
PE; its SHA-256 was
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
This is read-only discovery: no production source, build, test, game, Ghidra
mutation, export, or native-ABI validation was performed.
`verify_report_calls.py` passed72 direct call rows with0 failures. Twelve
indirect calls retain their operands/imports; the two raw startup calls need
the function definition described below before mechanical verification.

## Immediate field-cleanup packet

| Function | Complete inclusive span | Bytes | Original ABI | Coverage |
| --- | --- | ---: | --- | --- |
| Numeric-reference reserve | `B7D640-B7D69E` | 95 | ECX0Ch header; signed capacity stack; RET4 | Complete |
| Numeric-reference resize | `B7D7D0-B7D81F` | 80 | ECX0Ch header; signed count stack; RET4 | Complete |
| Hierarchy fields destroy | `B88180-B881F2` | 115 | ECX record; no stack arguments; RET; FH3 | Complete |

The numeric header is data/count/capacity at offsets0/4/8; each value is one
DWORD. Producer `B7EB90` writes integers returned by `BE9A00`, preserving order
and duplicates. These are numeric references, not retained item pointers.

Reserve clamps a signed request below8 to8, then compares the current signed
capacity. Growth allocates `request*4` with DWORD wrap through `BF55BE`, whose
actual body jumps to the existing `BF681B` allocation service. It reads count
after allocation, copies forward, rereads current old data/count in the loop,
and skips only a null computed destination slot. It frees the current old data
through `BF6989`, then publishes the captured new pointer and capacity. Count
and unused slots remain unchanged. There is no added overflow, rollback,
bulk-copy, empty-allocation or pointee-lifetime policy.

Ghidra omits `B7D691-B7D699` after the returning free at `B7D68C`. The matched
bytes `83 C4 04 89 1E 89 7E 08 5B` decode as `ADD ESP,4; MOV [ESI],EBX;
MOV [ESI+8],EDI; POP EBX`. The listed `POP EDI; POP ESI; RET4` follows. A
read-only flow query confirms the gap. The integrator owns any repair and
subsequent annotation/export; the current decompiled early return is incomplete.

Resize reserves only when the requested signed count exceeds current signed
capacity. It reads count after reserve, zeros computed added slots if nonnull,
and rereads data on each iteration. It then compares against the current count
again, decrements the actual count repeatedly when shrinking, and stores the
request unconditionally. Negative values retain the observed signed branches
and DWORD address arithmetic; removed values receive no cleanup.

`B88180` arms state0, calls resize0 on `record+4C`, reloads and frees that
header's current data, then reads current name data at `record+8`. It disarms
state0 before normal name cleanup. A nonnull name captures current length+4
plus one and returns the buffer through `419CC0/BD1510`. No headers are cleared,
and the record slot is not returned here. With negative array capacity, resize0
can allocate32 bytes before final free; an empty shortcut loses that behavior.

The three pushed words before `B881D6` belong to `BD1510`, not `419CC0`.
The getter takes **no native arguments** and uses plainRET; `BD1510` receives
ECX=the returned owner and `(pointer,length+1,1)` on the stack, consuming them
with RET0C. The raw `41DD20` overload already accepts `NativeStringRawPoolContext`
and lets getter exceptions escape. The older `NativeStringStorage` overload is
`noexcept`; substituting it would change this cleanup boundary.

All direct callers are covered: `B7D640` is called at `B7D7DE`, `B7ECCB`, and
`B7D9E9`; `B7D7D0` only at `B881AD`; `B88180` at `B884BC` and `B88323`.
The report records each containing function and argument provenance. The
optional `B7D9D0-B7DA09` append helper is58 bytes, ECX header, stack pointer to
a DWORD, RET4. It loads that DWORD after growth. The parser instead captures
the reader's integer in EDI before its inlined append. `B7D9D0` has no live
xrefs and is unnecessary for the immediate destructor packet.

## Producer, payload and exceptional lifetime

The complete769-byte `B7EB90-B7EE90` producer was byte-rechecked; this audit
covers layout and lifetime, with the remaining field helpers consumed as
external contracts. It supplies ECX=84h to `B87A90`; that wrapper discards the
size and selects the actual pool. The producer initializes this132-byte prefix:

| Offset | Producer field | Initial value |
| --- | --- | --- |
| `00` | Parent numeric value | FFFFFFFF |
| `04/08` | Owned name length/data | 0/0 |
| `0C-4B` | 64-byte matrix | Unwritten; no identity default |
| `4C/50/54` | Numeric Resource values data/count/capacity | 0/0/0 |
| `58` | Flags; bit meanings unknown | 0 |
| `5C/60/64/68` | Sphere center/radius | 0,0,0,1e10 |
| `6C/70/74` | Box minimumXYZ | -1e10 each |
| `78/7C/80` | Box maximumXYZ | 1e10 each |
| `84` | Hidden slab index, outside the payload | Written by slab initializer |

`B17510` initializes128 slots in a4504h-byte slab. It writes descending WORD
free indices at4400h, free count128 at4500h, and the supplied DWORD slab ID
at every slot+84h, stepping88h. It leaves every84h payload and the final two
padding bytes untouched. This establishes the hidden field from its producer.

The parser hands the same raw record to `B87AE0` at `B7EE78`. Existing
`structured_hierarchy.cpp` implements the typed fields, matrix and sphere;
that does not implement the raw88h allocation/append ownership path. Parent
is the first DWORD, so this audit establishes **no hierarchy-record vtable**.
The null allocation branch is not a graceful error path.

`B88180` handler `CC257B` points to FH3 info `DFB9C8`, map `DFB9C0`:
state0→-1 invokes `CC2570`, which loads the saved record from `[EBP-10]`, adds4,
and tail-calls `41DD20`. Only name cleanup is registered. No numeric-array
free or record-slot return is added by this unwind map. An exception before
the state-1 write therefore reaches name cleanup; one during normal name
return does not retry it.

The producer's handler `CC2000`, info `DFB168`, map `DFB158`, has only two
actions: state1→0 `CC1FF8` destroys the temporary name at `[EBP-2C]`; state0→-1
`CC1FF0` cleans the current child wrapper at `[EBP-34]`. There is no record
destructor or slot-return cleanup in that map. Full automatic record rollback
would introduce behavior. `B88320` likewise has no local FH3 frame; if
`B88180` throws, its subsequent pool return is not reached.

## Actual hierarchy pool and reusable source

| Actual location | Pool offset | Role |
| --- | --- | --- |
| `109022C/1090230/1090234` | 00/04/08 | Allocator vtable/previous/next |
| `1090238` | 0C | Actual24-byte Windows critical section |
| `1090250` | 24 | Signed recursion/depth DWORD |
| `1090254` | 28 | Slab table pointer |
| `1090258/109025C` | 2C/30 | Unsigned slab count/table capacity |
| `1090260` | 34 | First nonfull slab; FFFFFFFF sentinel |

The owner is38h bytes. `CD82D0` calls the **same** `B18340` constructor with
ECX=`109022C` and registers `CE0ED0`; that exit wrapper selects the same owner
and tail-jumps to `B18470`. Constructor `B18340` installs `D5E51C`, and matched
data at that table gives virtual0=`B18500`. This is the same profile as the
parameter pool, with separate storage, lock, table and allocator-list element.
Both owners join the existing `E188B4` list. It is not sufficient to allocate
zeroed38h storage or pass the material companion bound to `F8D3E4`.

The five existing bodies were read and byte-rechecked: `B17510-B17555`70 bytes,
`B18340-B18412`211, `B185A0-B186DB`316, `B18470-B184F9`138, and
`B18500-B1859F`160. In `native_material_pools.cpp`, `NativeMaterialParameterPool`
borrows the supplied `storage_` and shared `AllocatorListDomain`. These methods
use that storage and shape/profile constants; their descriptive
`native_global=F8D3E4` constant does not redirect an operation to that address.
The allocator constructs no payload. Destroy/trim likewise do no payload
destruction. Existing constructor EH handles table, section, then list cleanup;
its map bytes were rechecked without proposing another implementation.

`B17AF0-B17B57` is the104-byte standalone return entry. It enters actual pool+0C,
increments+24, **then** reads slot+84 and table+28. Signed byte-delta division by
88h uses `IMUL78787879; SAR6; SHR31` correction. It stores the WORD slot index
at `slab+4400+freeCount*2`, increments the current WORD at4500h, lowers firstFree
with an unsigned comparison, decrements+24 and leaves the same section. It
does no payload cleanup, validation or reclamation. All three actual callers
were inspected: `B191FC/B19299` pass `F8D3E4`; `B88335` passes `109022C`.
The equivalent shape-templated return already backs
`return_slot_00b193fa_fragment`; existing `gui_text_material.cpp` reuses it for
`B17AF0`. An address-specific entry can delegate to that concrete mechanism.

`B88430` independently inlines the same return using globals
`1090238/1090250/1090254/1090260`, after direct `B88180` and a nonnull check.
Do not individually CRT-free records. The current slot ID must be read under
the lock because trim rewrites the IDs of moved slab-table entries.

An explicit canonical hierarchy binding and these wrappers form a follow-up:

| Entry | Complete span / bytes | Original ABI and dependency |
| --- | --- | --- |
| Allocate wrapper `B87A90` | `B87A90-B87A99` /10 | Ignore incomingECX; select109022C; tailB185A0; EAX slot |
| Startup `CD82D0` | `CD82D0-CD82E5` /22 | No inputs; B18340, BF6FF5(CE0ED0), POP ECX; RET with registration result |
| Shutdown `CE0ED0` | `CE0ED0-CE0ED9` /10 | Select109022C; tailB18470; payloads must already be gone |
| Return `B17AF0` | `B17AF0-B17B57` /104 | ECX initialized pool; stack slot; RET4; existing mechanism |
| Flags cleanup `B88320` | `B88320-B8833F` /32 | ECX record; stack flags; RET4; fields always, return iff low bit0; EAX original record |

`B88320` has no live xrefs. It is useful as a bounded wrapper after the field
packet, but does not prove any virtual dispatch profile. `CD82D0` has no live
Ghidra function, despite its pointer at matched `CE35B0`. Its22 bytes and raw
calls are verified separately; the integrator must define it before standard
call-row verification can cover those two sites. No mutation was made here.

Reuse remains limited to valid initialized native storage and the existing
source domain. The companion's typed/placement-new operations do not establish
all-overlap or all-null-allocation equivalence. No private owner, copied state,
second free list, new allocation callback or competing exit registry is needed.
The name pool remains `419CC0`, publication`1090AA8`, gate`1090AA4`, and lifetime
manager`1090AA0`, with its different arena/ring layout and96h size classes.

## Remaining resource destructor frontier

This packet does not finish `B88430`. Actual manager getter`4C1400`, erase
`B801C0`, find`B7E7B0` and checked iterator erase`B7FA60` remain external
named-but-incomplete source/binding contracts. `B87AE0` raw hierarchy append
also has no reconstruction entry in this checkout; the sibling owns its
reserve dependency, not that append. `B7F290` cache pair and resource
construction are already complete within their recorded scopes.

Primary item dispatch is separate from hierarchy cleanup. `B88430` decrements
each primary item+4 and calls virtual0 only at zero. Matched `D631C0` contains
`BD30E0/B86990`; the former's source needs a finite deleting-profile provider,
and the eight-byte fallback's `B86990` scalar body is still only named. Its
known schedule stampsD5C104, callsBD30F0 and conditionally frees throughBF65AC.
Typed Mesh/Note/GroupParams parsers do not prove their native deleting profiles
or reference handoff. Existing input/shader/text providers do not supply them.

Implement the immediate three-function packet in new
`native_resource_hierarchy_fields.hpp/.cpp` plus its doc/report, claiming its
addresses and disjoint files first. Add the separate canonical pool binding and
wrappers after coordinating the existing shared pool source with its owner.
Keep cache, item-profile work, sibling arrays and `B88430` implementation with
their owners. The local evidence is retained under
`local/resource-hierarchy-frontier-be/` in this worker's worktree.

## BF integration correction, 2026-09-13

The isolated BF integration on 2026-09-13 defined and range-verified all eleven previously missing functions, including `CD82D0-CD82E5`, against the installed PE and live Ghidra bytes. The seven nondeleting leaves, token startup entry and two compiler exception handlers remain analysis-only. Evidence is retained in `reports/native_resource_lifetime_bf_function_definitions.json`. The local `B7D68C` call override was cleared and its nine-byte fall-through gap decoded; the stored `B7D640-B7D69E` body now covers all95 bytes with zero remaining call gaps. Existing full-function documentation was archived before and after repair. The repair changes analysis metadata, not the installed game. See `reports/native_resource_lifetime_bf_flow_repairs.json`.

The unchanged report now passes all72 direct-call rows with zero failures. Historical missing-function flags and worker-stage limitations above describe the retained original observations. New source comprises nine complete bodies (494 original bytes) across the three BF source packets; pool wrappers remain build-only. Combined candidate validation is recorded separately; this correction does not claim gameplay validation.
