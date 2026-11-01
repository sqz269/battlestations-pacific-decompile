# Raw texture-atlas registry construction and ordinary retirement

Source-only candidate for original `007364A0`, `00737EE0`, `00AEF090`,
`007375E0` and `00737F10`. Names are descriptive hypotheses. The new interfaces
operate on the real raw `1Ch` allocation and its actual arrays; no typed atlas,
sprite parse result, empty substitute or host vtable is overlaid. This packet
does not load `interface/textures/common.ats` or activate Page/Icon/GUI.

The source consists of `native_texture_atlas_registry_storage.hpp/.cpp` and
depends on the genuine item `AEE220` implementation. Its retained source
interfaces add context and frame arguments, so they are not original ABI
replacements. All five ordinary NORMAL payloads are supplied; Original FH3/SEH,
hardware-fault/unwind identities, Native Origin and runtime gates remain held.
No CMake, ledger, Ghidra annotation, tracked-file change, compilation, fixture,
Game run, installation, commit or push was performed for this candidate.

## Original evidence and ABI

| Entry | Original inputs/result | Whole ordinary effect |
| --- | --- | --- |
| `7364A0` | ECX actual registry, no stack arguments, EAX same, RET | Stamp CFEA58; capture first manager's section; enter/increment; publish F8C26C; obtain manager again; register CURRENT publication; decrement/leave original section. |
| `737EE0` | ECX actual registry, no stack arguments, EAX same, RET | Direct full7364A0, CFEAE8, six individual zero DWORD writes at+4 through+18. |
| `AEF090` | ECX actual registry, no stack arguments, RET, no semantic result | Item destruction/free; actual texture reference release; texture-array retirement; item-array retirement; direct full7375E0. |
| `7375E0` | ECX actual registry, no stack arguments, RET, no semantic result | CFEA58; two fresh actual-manager calls; unregister CURRENT F8C26C; clear; release captured section; CE3818. |
| `737F10` | ECX actual registry; DWORD flags stack HOME, BYTE test; EAX captured registry; RET4 | Direct fullAEF090; reload low BYTE of flags HOME; BF65AC only if reached bit0; return original address even if freed. |

Original startup disk bytes independently show `73DDB4 PUSH1C/BF681B`,
`73DDD5 CALL7364A0`, `73DDDA CFEAE8`, six zero stores, and the subsequent
`73DE42 CALLAF0060`. That load precedes GUI and is owned by later parser/loader
work. The real empty postconstructor state is not the populated pre-GUI graph.
`7364F6` publishes the actual allocation; `737649` clears the actual cell.
Constructor `737EE0` itself neither allocates nor performs the common load.

`AEF090`'s 163 exported instructions were inspected as assembly. Three listing
gaps were verified read-only against live Ghidra bytes: `AEF17D 8D4900`,
`AEF19D 8D4900` and `AEF23A 8D9B00000000` are alignment LEAs. No listing
or database repair was made. Live providers show `BF55BE JMP BF681B`,
`BF6989 JMP BF65AC`, and `BF65AC JMP BF9DC8`; their library bodies receive
no reconstruction credit. Shared `singleton_lifetime_allocate/free` supplies
the established source CRT heap/new-handler domain.

## Raw ownership and read schedule

Manager+0 is the current profile. Item-array begin/count/capacity is+4/+8/+C;
texture-array begin/count/capacity is+10/+14/+18. These are owning actual raw
pointers, not Source companion pointers. Native item ownership covers actual30h
items and their strings; the item's+8 texture is borrowed and is not released
by AEE220. Registry texture ownership is the actual texture array's references.

The item loop reads count8 before state2, skips null item slots, captures each
actual item, invokes full AEE220 using the same item/pool/feature context, then
frees that captured item. It reloads current begin4 and count8 for subsequent
iterations. It does not clear consumed slots or retry failed string returns.

The texture loop captures CE2220 once, only on initial signed count14>0. It
reloads current begin10/count14 for each reached iteration, captures one actual
texture, decrements exactly that texture+4, and invokes slot0 only on that
decrement's zero result. There is no null-texture shortcut or later count read
used to invent a zero result. Reached caller profile/slot0 and BD30E0 refreshed
profile/slot4 are read as data. Only mapped D61948/B3F590, D61870/B3F410 and
D618B0/B3F430 families are admitted; words are never host callable pointers.

The context borrows the concrete `NativeRenderActualOwnerRegistry` and the
existing genuine `NativeTextureLoadOwners` graph. Its 2D context must designate
that SAME canonical registry; special contexts must pass their existing same
renderer/string/serial/retained-memory/pool checks when reached. Caller proves
the actual texture was born and registered through those concrete creators.
Profile bits, counts and a registry lookup cannot establish that provenance.
Zero dispatch resolves the exact existing companion, checks that its count
aliases the captured actual+4, then invokes its existing family terminal
without another decrement. Missing identity or unsupported table remains an
explicit failure after zero, with the partial release record retained. The
existing terminal is noexcept and may terminate for an escaping child failure;
this candidate does not invent catchable retirement or a replacement terminal.

Each array preserves the whole signed-negative-capacity path: allocate4 via
the same source heap, copy using fresh begin/count reads, free captured old
buffer, publish replacement and capacity1. Signed-negative counts retain raw32
count*4 displacement, conditional zero stores and the ADD/JS loop, including
wrap. Positive counts decrement the current count until it is nonpositive.
The final buffer is captured BEFORE count0; that captured buffer is freed;
begin/capacity and consumed bytes remain stale. Texture array retires first,
then item array, then full base destruction. No normalizing or early-empty
shortcut was introduced. All reached physical extents must actually be valid;
the source does not legitimize Original invalid accesses or allocator overruns.

Base routines use the genuine raw source getter00415350, registrationBD0C30
and removalBCFCA0. They keep the first captured section while reloading the
manager and actual publication at the original call epochs. Enter/leave are
typed live callable-import cells, read at their reached sites; the depth+18
read/write ordering is preserved. Current registry publication is not replaced
with captured self or compared against it. Clearing/unregistering therefore
follows Original even if a returning provider changed the publication.

## Retained failure graph and physical domains

Fresh address-stable frames hold actual raw8 guard scratch, captured identities,
states and call history. Each nonnull item destruction and each texture release
gets a stable separately retained node adopted before its first native effect.
Cold metadata allocation has no permission to inspect, mutate or reenter any
Native graph. On metadata failure the outer frame keeps prior native steps and
the current captured identity. Prior nodes are not destroyed or overwritten.

All catch paths store source exception history and barethrow. No outer guard
release, array cleanup, name return, unregister, publication clear, item/texture
free, replay or rollback is added. Retain partial arrays, retired opaque keys,
raw-string/provider debt, zero obligations, locks, frames and the entire actual
provider graph externally. Phase/site flags are diagnostics, not terminal or
NoThrow authority. Dead item/texture/registry addresses are never dereferenced
after their returning free/terminal. Item and array backing, actual atomics,
mapped data, raw pool/live feature, heap, renderer, canonical registry, mutable
publication/import cells and their synchronization must outlive reached uses.

Incoming scalar flags use real initialized ordinary DWORD backing borrowed via
a volatile reference, separate from all frame/private/provider-write extents.
The implementation reads exactly one low BYTE after the complete destructor;
it neither snapshots a full DWORD before the child nor synthesizes flags1.
Native saved-register/stack/FH3 aliases are not reproduced by the new C++ API.

## Admission and remaining work

The frozen authoring base is Root `2a094149a200654ea7ddf63226483c2a72bcba35`.
Parent subsequently reported compiler-only item107 correction `6a620df73`
(`offset` parameters/asm operands renamed `byte_offset`), with unchanged item
public interface. This candidate uses `byte_offset` and does not modify107.
Original handoff SHA is
`b85cfbf02ac3dbb1f279db98d2453a289dc3d13cbf12280f4e495d0a367f0a3e`.

The primary agent must review/admit the exact candidate, add required metadata
and build sources, compile the joined tree, and independently qualify any actual
runtime integration. Parser AEEAF0, loaderAEF280/AF0060, publication-producer
composition, Page/Icon type6 custody and the pre-GUI common load remain outside
this packet. Existing ordinary Source completion alone cannot activate GUI,
establish actual graph custody, or discharge any ABI/Origin/Game gate.
