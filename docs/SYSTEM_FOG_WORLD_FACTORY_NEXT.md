# World-construction fog factory: bounded implementation handoff

Read-only discovery for `[004DF6A3,004DF7A5)` within `004DE610`. The 258-byte
interior is ready for a bounded implementation using the existing
`SystemFogOwner`, its real host allocator and counted slot setters. This does
not establish a whole game/world constructor, native CRT exception ABI, or game
validation. No C++, Ghidra annotation, ledger or test was changed in this packet.

## Actual object identities

| Native binding | Established concrete identity | Required host binding |
| --- | --- | --- |
| game+19FC | 458h Operator/camera allocation, constructor `00B71A80`, concrete vtable D62CF0 | The same live `CameraFrameState` used by camera/fog consumers; its actual `fog_184` slot |
| game+19E8 | 40h receiver, constructor `00BBDFF0`, concrete vtable D64518 | A borrowed reference to this object's actual counted fog pointer at +10 |
| game+5FC | Selected scene-record pointer; its directional words are at +A20..A5F | The actual live pointer slot, reloaded for the guard and each record copy |
| parent stack+18 | Four-byte temporary assembled here and consumed at `004DF7A5` | An output reference to the caller's actual packed-color temporary |

No RTTI/type name was recovered for D64518. Existing world/ocean documents call
it an ocean owner and its +3C child an ocean object; those are semantic
hypotheses. It is **not** the separately allocated world object at game+19CC.
The compatible core API name `set_system_fog_world_owner_00bbdf20` refers to
this D64518 receiver's +10 slot; a neutral Ghidra name such as
`BSP_FogReceiver_SetSystemFogOwner` avoids asserting a recovered class name.

The camera allocation is made through `00B71930` at `004DE744`, its constructor
is called at `004DE782`, and its pointer is stored at game+19FC at `004DE78E`.
The constructor stores D62CF0 at `00B71AB4` and zeroes its fog slot at
`00B71AE3`. This existing constructor fragment and fog slot projection are
already represented by the core APIs. The full camera/node allocator and
constructor remain outside this packet.

Both branches preceding the factory allocate 40h and call `00BBDFF0`; they
publish D64518 to game+19E8 at `004DF47D` or `004DF642`. That constructor stores
the vtable at `00BBE065` and zeroes its +10 fog slot at `00BBE06E`. The factory
nevertheless treats the receiver as optional and never allocates it itself.

`004C6890` selects a node from the game+5F4 list, takes node+8, and stores that
record pointer to game+5FC at `004C68EB`. It is a borrowed selection here, with
no retain or copy at this store. Mission loading passes the same pointer to
`0046DF00` at `004E01D7`, immediately before calling `004DE610` at `004E01DE`.
The scene-list allocation/destruction and full parser are named but incomplete
dependencies; the factory must receive live storage instead of inventing a
scene-record layout or its defaults.

## Exact factory order

1. `004DF6A8` requests 94h through `00BF681B`. The returned raw allocation is
   recorded in parent stack+24, then state 24h is installed before constructor
   `00B84E50` at `004DF6C5`. The constructor returns the same owner with reference
   count 1 and preserves the raw directional allocation preimage.
2. `004DF6D0` reads game+19E8 **after allocation/construction**. Restore the outer
   EH state to -1; if the receiver is nonnull, call `00BBDF20` on its actual +10
   slot. The integrated setter publishes new, increments new, then releases old.
3. `004DF6E7` independently reads game+19FC. `004DF6EE` publishes the same owner
   through `00B71940` to that camera's actual +184 slot. No null-camera gate
   exists. Preserve the read timing; an allocator handler or old-owner release
   can make an entry-time receiver snapshot stale.
4. `004DF6F7` decrements the allocation's temporary reference through the live
   `InterlockedDecrement` import held in EBX. A zero result dispatches owner
   vtable+0 at `004DF704`. With valid native bindings, camera publication has
   retained the owner, so it remains alive for the following writes. An optional
   receiver leaves a second reference. Do not return an additional owning
   reference from the host factory or defer this temporary release.
5. Execute `FLDZ; FSTP m32` and set owner+68 through `00B84D00`; repeat and set
   owner+78 through `00B84D40`. These writes occur **after publication and the
   temporary release**, even when a constructor default already equals zero.
6. Execute `FLDZ; FDIV m64 [00CE4B48]`, whose verified double is 255.0. Build color
   words from `00CE7D60,00CE7D5C,00CE7D58`, with raw bits
   `3F25A5A6,3F23A3A4,3F2BABAC`, and spill x87 alpha to the fourth word. Call
   `00B84C40` at `004DF771`. Do not recalculate RGB constants from decimal ratios.
7. At `004DF776`, test the live game+5FC slot. Null skips the entire directional
   loop, preserving the owner's constructor/allocation directional bytes.
   Otherwise, reload game+5FC at `004DF786` for each index 0..3 and call
   `00B84FA0` with the current record+A20+10*index. Each setter retains native
   forward DWORD copy behavior. The initial guard does not replace these four
   reloads; a later null/invalid pointer would be a native fault, not a new skip.

The null allocation branch is emitted compiler scaffolding, not a usable
out-of-memory fallback: the actual `operator_new` returns a nonnull allocation
or throws, and this fragment unconditionally accesses `owner+4` after publication.
Do not allocate a zero owner, catch allocation exhaustion into success, or add
directional defaults when the scene record is absent.

### Packed-color continuation output

While the x87 alpha is still live, instructions `004DF753,004DF758,004DF75D,
004DF762` write bytes A5, A3, AB, 00 to offsets +2,+1,+0,+3 of the original
parent stack+18 word. Its final value is `00A5A3AB`. At `004DF7A5`, just outside
the factory range, the parent loads that word and calls `00B6FE50`, whose exact
body stores its argument to camera+190 and returns with `RET 4`.

The bounded implementation must expose this output; silently discarding the
temporary would leave the continuation unbound. Do not include `00B6FE50` or
the following environment construction in the factory's implementation scope.
Native EBX/EDI/EBP scratch-register outputs need no separate host ownership
domain; the C++ interface is not an ABI replacement for jumping into this range.

## Allocation, EH and release closure

`00BF681B` is a returning cdecl allocator with size at stack+4 and EAX result.
Its success tail is `00BF683D: LEAVE; 00BF683E: RET`. On malloc failure it calls
`00C055B1(size)`, retries for a nonzero result, and constructs/copies a
`bad_alloc` exception before `__CxxThrowException@8` at `00BF687F` when the
handler declines. The full bounded extent is `[00BF681B,00BF6884)`.

`__callnewh` reads encoded handler global 0109DE44, resolves it through
`00C04FDE`, calls a nonnull handler with the original size, and normalizes its
result to 0 or 1. The resolver's decompilation identifies its DecodePointer/TLS
compatibility path; no modern host pointer is substituted into that native
global. Native `_malloc` also has a 0109E314-controlled inner new-handler retry
and native heap selection. Those CRT internals and exception-object identity
remain explicit library boundaries. The integrated
`singleton_lifetime_allocate` uses actual host `malloc/_callnewh/std::bad_alloc`,
and `allocate_system_fog_owner()` requests native94h/host `sizeof(SystemFogOwner)`.
Reuse these real services; no injectable fake allocator is needed for this packet.

The parent handler thunk `[00C671A0,00C671AA)` loads FuncInfo D8FD04 and jumps
to `00BF6B43`. FuncInfo has magic 19930522, 2Dh states and unwind map D8FD28.
Its state-24h record at D8FE48 is `toState=-1, action=00C6713D`. That action loads
the raw allocation from the native EH frame's EBP-7C (the parent's stack+24
slot), calls free thunk `00BF65AC`, then executes `POP ECX; RET`.

Ghidra currently cuts this funclet at `00C67145`, omitting the returning bytes
at `00C67146..47`. Its verified true extent is `[00C6713D,00C67148)`. The
receiver deleting wrapper also omits `ADD ESP,4` at `00BBE285` in the listing
after the same free thunk; raw bytes establish `[00BBE270,00BBE28E)`. These are
the same returning-free analysis problem encountered in core owner discovery.
No function definition or no-return annotation was changed by this read-only
packet. The free thunk itself is the five-byte jump to native `_free` at
`00BF9DC8`; the integrated host service uses matching real `free`.

State 24h covers raw-storage construction only. It is restored to -1 before
either counted publication; its action is **not** a temporary-owner release or
a rollback of already-published owners. The integrated initializer is `noexcept`
and has no host allocation after the raw allocation. Propagate the allocator's
host exception; do not claim MSVC 2005 SEH identity or install a new handler domain.

At later world teardown, `[004D2BF2,004D2C0B)` invokes game+19E8 vtable+4 with
flag1 and clears the game slot after it returns. D64518 slots 0/1 are
`00BD30E0/00BBE270`; the wrapper calls `00BBE1A0`, where
`[00BBE20A,00BBE226)` decrements its +10 fog owner, destroys at zero, then clears
that slot. The camera's D62CF0 slots are `00BD30E0/00B71FE0`; its destructor
`00B71F10` releases and clears +184 in `[00B71F68,00B71F8A)`, already covered by
`clear_system_fog_camera_slot_00b71f68`. Whole camera disposal scheduling and
the complete D64518 receiver lifetime are not implemented by this factory.

## Proposed host API and packet ownership

One viable borrowed interface is:

```cpp
struct FogReceiverFields {
    const SystemFogState*& fog_10; // actual existing D64518 receiver slot
};
struct WorldFogFactoryGameFields {
    FogReceiverFields* const& receiver_19e8;
    CameraFrameState* const& camera_19fc;
    const void* const& scene_record_05fc;
};
void initialize_world_fog_004df6a3(
    const WorldFogFactoryGameFields& actual_game,
    std::uint32_t& actual_packed_color_temporary_18);
```

These are references to the actual consumer projections and pointer slots, not
owners or entry-time snapshots. The receiver wrapper stores only a reference to
the existing +10 field; it must track the actual selected receiver. The camera
must be the same `CameraFrameState` already used by consumers. Do not construct
a fresh camera or scene record to satisfy the call. Each loaded nonnull scene
record must have at least A60h readable bytes, and objects must survive their
native consumption points. The whole game object remains outside this API.
Do not pre-read the receiver slots before the allocator can invoke its handler.

The record+A20 directional source is precisely record+990+90, the same four
records later read by `copy_authored_fog_0078caa4` from its actual authored block.
That copy fills environment fields and its separate, genuinely constructed B4
private owner; `apply_environment_fog_0078d076` subsequently writes the shared
camera owner. This relationship requires no extra factory fog state.

Proposed implementation packet `system_fog_world_factory_impl` owns only
`[004DF6A3,004DF7A5)` and:

- `include/bsp/system_fog_world_factory.hpp`
- `src/system_fog_world_factory.cpp`
- `docs/SYSTEM_FOG_WORLD_FACTORY.md`
- `reports/system_fog_world_factory_audit.json`

Reuse the implemented owner/core, camera projection, allocator and leaf setters.
The integrator owns CMake, ledger shards and annotation changes. Do not lease or
rewrite whole `004DE610`, native CRT routines, `00BBDFF0`, `00BBE1A0`,
`00B71A80/00B71F10`, scene selection/parser, or authored/apply siblings.
The report proposes evidence comments and lists each named-but-incomplete
dependency separately. Native fixture work belongs to the later implementation
packet; this discovery ran no build/tests and makes no runtime claim.
