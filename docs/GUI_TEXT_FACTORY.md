# GUI Text pool and factory prerequisites

Addresses: `00AB6F60`, `00AB70C0`, `00AB7420`, `00AB7500`, `00AB75A0`,
`00AB76F0`, `00AB77C0`, `00AB78A0`, `00AB79E0`; analyzed external startup
`00CD71F0` and `00CE0A80`.

The pool is reconstructed. A canonical Text object factory is **still missing**.
`00AB78A0` returns unconstructed storage; calling it does not create a
`GuiWidgetOwner`, `GuiLayoutWidget`, or `GuiTextLifetime`. The new implementation
never casts a raw slot to those C++ types or supplies missing current virtuals.
Names below are hypotheses, not recovered symbols.

| Routine | Native ABI and exact final instruction | Coverage |
|---|---|---|
| `AB6F60` slab initialization | ECX slab, stack slab index; EAX same slab; `AB6FA0 RET4`, 3 bytes | Complete valid-storage writes |
| `AB70C0` table cleanup | ECX address of table header; `AB70CD RET`, 1 byte | Complete host-allocation-domain cleanup |
| `AB7420` pool destruction | ECX pool; `AB74A9 RET`, 1 byte | Complete explicit pool effects; no Text destructor |
| `AB7500` pool virtual0 trim | ECX pool; `AB759F RET`, 1 byte | Complete compaction and hidden-ID rewrite |
| `AB75A0` return slot | ECX pool, stack slot; `AB7609 RET4`, 3 bytes | Complete valid-slot return |
| `AB76F0` failed-construction return | ECX raw slot; `AB76FB RET`, 1 byte | Complete explicit binding to the same pool |
| `AB77C0` pool initialization | ECX pool; EAX pool; `AB7892 RET`, 1 byte | Complete pool effects and three cleanup effects in a new C++ interface |
| `AB78A0` allocate slot | ECX pool; EAX raw slot; `AB79DB RET`, 1 byte | Complete valid-storage allocation and publication order |
| `AB79E0` allocation wrapper | Incoming ECX ignored; `AB79E5 JMP AB78A0`, 5 bytes | Complete explicit same-pool binding; no Text construction |

`NativeGuiTextPool` refers to one supplied `NativeGuiTextPoolStorage`, one actual
`AllocatorListDomain`, and its real `AB7500` trim dispatch. It has no secondary
slab table, free stack, widget tree or Text state. Construction of the C++
companion binds profile `D5C5D8`/virtual0 `AB7500` before native initialization can
publish the allocator element. Explicit `initialize_00ab77c0` and
`destroy_00ab7420` control native lifetime. Its C++ destructor removes only the
host dispatch association. Destroy the pool before destroying its companion,
storage or list domain, and after all Text payload owners have finished.

The shared `singleton_lifetime_allocate/free` implementation is the existing
reconstruction's host CRT allocation domain, also used by native node/model
pools. This code can own its newly allocated physical slabs and tables; it must
not adopt pointers allocated by the original executable's separate CRT heap.
No raw original game address is called. No CRT static-startup registration is
installed by this packet.

## Storage and failure behavior

`AB77C0` produces the 38h pool: the existing allocator element at 0, actual
Win32 critical section at +0C, signed recursion depth at +24, slab table/count/
capacity at +28/+2C/+30 and earliest nonfull index at +34. It prepends the same
shared allocator-list head, initializes the critical section, writes the empty
header and sentinel FFFFFFFF, then publishes capacity32 before allocating the
80h table. `DEEC44` gives cleanup states `-1/CB7E40`, `0/CB7E48`,
`1/CB7E53`: unlink via `403970`, critical-section teardown via `402F70`, and
table cleanup via `AB70C0`, executed in reverse construction order. The new
Win32 interface reproduces those effects with an explicit `__finally`; it does
not claim the original native EH-frame ABI or arbitrary memory-fault behavior.

`AB6F60` writes free count64 at slab+7E80, reverse WORD indices63..0 at
slab+7E00, and the slab index at each slot+1F4. There are 64 slots of 1F8 bytes
in a 7E84h slab. The first 1F4 bytes of every slot and final two padding bytes
remain unwritten. A default-constructed Text is therefore not zeroed raw memory.

`AB78A0` takes the real critical section and increments depth, publishes the
new first-free index before slab allocation, and publishes doubled-plus-two
capacity before table allocation. It reloads the live table/count while copying,
frees the old table, publishes the replacement, appends the new slab, pops one
WORD index, then scans only later slabs when the selected slab becomes full.
There is no native unwind region around these allocations. An allocation
exception leaves the lock, depth and prior publications intact; no automatic
unlock, rollback or invented recovery is added. Allocation overflow wraps in
32 bits. A foreign new-handler/CRT failure implementation is outside this host
allocation domain.

`AB75A0` reads the same slot+1F4 identity, computes the low-DWORD address
difference, divides it as signed by1F8 with truncation toward zero, pushes the
low WORD onto the current free stack, reloads and increments its count, and
lowers the earliest index by unsigned comparison. It does not destroy the
payload, detect double return, clear memory, or reclaim slabs.

`AB7500` has no internal lock. It frees each wholly empty slab, copies the last
table entry into the removed position even for the last entry, decrements count,
and rewrites **all 64** hidden slot IDs of a moved slab. It retries that index
before rescanning for the first nonfull slab. `AB7420` frees all slabs and the
table, drains positive tracked CS depth, deletes the critical section, then
unlinks the allocator element; old table/count fields are not cleared.

## Factory and current-dispatch boundary

The existing native class dispatcher passes type3 to `AA1380`. At `AA139E`,
`AB79E0` ignores ECX=1F4 and returns a raw slot. With a nonnull source, `AA13BA`
calls copy construction `ABB2C0`; with a null source, `AA13DD` calls default
construction `AB9650`. The two unwind leaves `CB6E00`/`CB6E08` load the same
saved raw allocation then tail-jump to `AB76F0`. That return helper calls only
the pool return. It does not run the completed Text destructor.

The child writer uses a different route. `AB9D38` calls the pool directly with
ECX=F8BDF0 and no stack arguments; nonnull storage goes to `AB9650` at
`AB9D4F`. Constructor `AB9650` calls the base with type3 at `AB9671`, selects
the real Text profile `D5C6C8`, and invokes `AB8530` at `AB98D8` while its primary
model is still null. The existing `GuiTextLifetime` already preserves its one
canonical companion publication and derived cleanup at this phase.

After successful construction, `AB9D70` appends the exact child to the parent's
glyph collection **before** cloning the current template's model at `AB9D87`
(current node virtual10, flags26h, parent null), binding it at `AB9D8C`, and
calling `AB8530` again at `AB9D93`. Constructor failure uses `CB80B0` ->
`AB76F0`. A later failure after vector publication needs the actual retained
child/continuation ownership; it is not an unconstructed-slot return.

The ordinary `AA6640` route allocates a fresh model, binds it, then invokes
current74 at `AA66A1`. It is not the child's clone-and-bind route. For Text,
profile `D5C6C8+74` is `AB7700`: capture the current primary model before mesh
allocation, construct and associate the real mesh, release its creator, then
write zero position. Current78 is `AB6AA0`, a tail jump to `AA7170`: invoke the
**current** virtual60(false), then refresh bounds. `gui_text_type_dispatch`
already implements those helpers and Text current60 (`AB87D0`); pool allocation
must not run either74 or78 itself.

Remaining concrete prerequisites are:

- A retained Text type implementation in the same `GuiWidgetOwnerRuntime`.
  Its current `construct_child_00aa6560` rejects Text, and its private base
  construction cannot be replaced by a detached second owner. Complete required
  properties18, size/current4C/64 and current70 behavior before enabling it.
  The new properties18 reader can return a suspended continuation; factory
  completion and loaded78 must wait for that continuation to complete.
- Correct new-C++ allocation transport for one `GuiLayoutWidget`, owner and
  `GuiTextLifetime`, including partial construction and the already published
  child-vector phase. Raw1F4h storage cannot contain those unrelated C++ types.
- Actual template model clone `B752B0` -> `B6F150` and source geometry current10
  composition, followed by the existing same-owner node binding and `AB8530`.
- Full child-tail continuation and recursive Text content operation. The
  resource-name setters and Text74/78 helpers are existing consumers to compose,
  not replacement callbacks. Copy construction `ABB2C0` remains a separate
  native route.
- Explicit startup binding/registration of the one Text pool. Raw `CD71F0`
  selects F8BDF0, calls `AB77C0`, and registers `CE0A80`, which tail-jumps to
  `AB7420`. This packet does not install a global or an atexit surrogate.
  `CD71F0` has no Ghidra function: exact final `CD7205 RET`, length1, endCD7205.
  The existing `CE0A80` body ends with `CE0A85 JMP AB7420`, length5, endCE0A89.
  Both startup entries are analyzed-only external context.

The existing `GuiTextChildDeletion` intentionally frees its typed C++ wrapper
allocation. The newly available raw pool return cannot be inserted there without
an actual association to a pool slot. Typed string/vector destruction remains
distinct from original native pool and SEH equivalence.

## Evidence and verification

Read-only Ghidra batches verified `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`. Every claimed body and all direct/tail callers were
read. Saved-listing holes caused by the free no-return annotation were checked
against live bytes and read-only PE decoding: `AB70CC POP ECX`, `AB743D..47`,
`AB7455..57`, `AB7526..5F`, `AB7879..7B`, and `AB7936..38`. They are interior
continuations, not new function starts. No Ghidra mutations were made.

Strict MSVC Win32 `/std:c++17 /W4 /WX /O2 /MD /fp:strict` compilation passed.
One ignored local probe used the actual pool, shared allocator list, real Win32
critical section and shared CRT service: 65 allocations, freeing the first slab,
trim through actual virtual0 dispatch, moved-slot ID repair, payload preservation,
LIFO reuse, final trim and explicit destruction all passed. It linked the current
parent `bsp_core.lib` and the new object with `/MANIFEST:EMBED`. No test target or
framework was added. Allocation failure and native byte-differential behavior
were not executed. The factory is not reachable in `bsp_game.exe`; no gameplay,
runtime-menu or visual claim follows from compilation or the pool probe.

`reports/gui_text_factory.json` carries precise call rows and remaining
prerequisites. The strong parent verifier passed 23 numeric rows with zero
failures (20 direct calls and three resolved indirect calls). Named imports,
tail transfers and the raw static initializer were read separately. The parent
performs the combined build and integration.
