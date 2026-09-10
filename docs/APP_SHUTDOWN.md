# Application shutdown order

Addresses: 00737f30, 004c12b0, 00aa0f70, 004dc5c0, 00a3b6e0, 0051f460, 00be8350, 004c0ef0,
004fc150, 00be7130, 00bbc5d0, 00bb4db0, 004c9c90, 00888750, 00bcfca0, 00736100, 007361a0,
00736250, 00736300, 007363b0, 00736460, 007364a0, 00736540, 007365e0, 007366b0, 00736750,
007367f0, 00736890, 00736930, 007369e0

Packet `app_shutdown`, branch `agent/app-shutdown`. Ghidra was read-only for this packet; names below
are provisional ledger hypotheses, not recovered symbols.

## Anchor

`BSP_Application_Shutdown` at 00737f30 is `void __thiscall f(void *this)`: ECX carries the
application object, there are no stack arguments and the epilogue at 0073835e is a plain `RET`
(RET size 0). The body installs an MSVC SEH frame (handler 00c862e0, `PUSH -1 / PUSH 0xc862e0`
at 00737f36) and uses the trailing EH state slot for the four temporary path strings.

The single caller is `BSP_WinMain` (008f81f0), which calls `BSP_Application_Destruct` (007379a0,
owned by the WinMain packet) afterwards. Near the end of the routine, 0073832d loads ECX with
0x00cfeaec and calls 004c9c90; the bytes at 00cfeaec are the literal `After MitApp::Deinit` and
004c9c90 is a single `RET`. That is a retail-stubbed trace call, and it is the evidence that the
application class is named `MitApp` and that this routine is its deinitialization step. The same
stubbing explains why other class names survive as data: `TRIV_body_004254b0` is the format-string
trace sink and its callers pass literals such as `GGame::OnDestroy()` and
`cFileStore::RemoveFile: %s removed!`.

### Object fields used

| Offset | Meaning | Evidence |
| --- | --- | --- |
| this+0x08 | Pointer to an array of reference-counted system pointers | 00737f80 `MOV EAX,[ESI+8]` indexed by EBX |
| this+0x0c | Element count of that array | 00737f72 / 00737fad loop bound |
| this+0x14 | `GGame` instance | 00737fb6 passes it to 004dc5c0, whose trace literal is `GGame::OnDestroy()`; `BSP_Application_Initialize` writes the game pointer to the same offset |

Each array element is an intrusively reference-counted object: the counter is at element+4
(`LEA ECX,[EDI+4]` then `InterlockedDecrement` through 00ce2220), and reaching zero calls
vtable slot 0 with **no** arguments (00737f9b-00737fa1), which is a destroy entry point rather than
the usual MSVC scalar deleting destructor. The slot is then zeroed. Release order is ascending index.

## Recovered teardown order

Steps are listed in execution order with the address of the call site in 00737f30.

| # | Site | Action |
| --- | --- | --- |
| 1 | 00737f4c | `004c12b0` lazily creates and registers the GUI manager singleton (DAT_00f8bc5c) and returns it in EAX |
| 2 | 00737f53 | `00aa0f70` with ECX = that manager: resets the two screen objects at +0x74 and +0x78 |
| 3 | 00737f63, 00737f70 | Device object DAT_00f8d394: virtual slot +0x0c, then virtual slot +0x14, both with no arguments |
| 4 | 00737f80-00737fb0 | Release loop over the system array (this+0x08, count this+0x0c), ascending |
| 5 | 00737fb9 | `004dc5c0` = `GGame::OnDestroy()` on this+0x14 |
| 6 | 00737fc5 | Scalar deleting destructor of the game object, then this+0x14 = 0 |
| 7 | 00737fda | If DAT_00f8abdc: `00a3b6e0` stops and joins the network threads |
| 8 | 00737fe9 | If DAT_00f8abe8: scalar deleting destructor |
| 9 | 00737ff1 | `XLiveUninitialize()` |
| 10 | 00738000 | If DAT_00f871b4: scalar deleting destructor |
| 11 | 00738008, 0073800f | `0051f460` returns a file-store service singleton (DAT_0109db70); `00be8350` is invoked on it and is a single `RET` in retail |
| 12 | 00738014 | `004c0ef0` destroys singleton DAT_00f8753c |
| 13 | 00738019-00738117 | Release chain, in this order: DAT_0109cecc, DAT_00f8bc4c, DAT_010909b0, DAT_00f8aefc, DAT_00f8c218, DAT_00e1aea0, DAT_00f8c210, DAT_00f8c280, DAT_00f8d39c, DAT_00f8bbf0, DAT_00f8c274, DAT_00f8bbf4 |
| 14 | 00738119-007382c7 | Four input datatable files are removed from the file store (see below) |
| 15 | 007382d2 | If DAT_00f8c26c: scalar deleting destructor |
| 16 | 007382da | `00736250` destroys singleton DAT_00f8bc5c, the manager created in step 1 |
| 17 | 007382df-00738304 | `BSP_SingletonLifetime_GetManager`, unregister DAT_00f8bc4c, destroy it, null the global |
| 18 | 0073830a | `00736300` destroys singleton DAT_00f8bf44 |
| 19 | 0073830f | `007363b0` destroys singleton DAT_0109cf14 |
| 20 | 00738314 | `00bbc5d0` finalizes and frees the global block DAT_01090900 |
| 21 | 00738319 | `00736930` destroys singleton DAT_00f8d420 |
| 22 | 0073831e | `007369e0` destroys singleton DAT_01090490 |
| 23 | 00738323 | `007361a0` destroys singleton DAT_00f8bbf8 |
| 24 | 00738328 | `00bb4db0` static destructor stub: ECX = 0x010904e0, tail jump to 0041cc80 |
| 25 | 00738332 | Stubbed trace of `After MitApp::Deinit` |
| 26 | 00738337 | `00888750`, a single `RET` |
| 27 | 0073834c | If DAT_00e1aed4: scalar deleting destructor |

Nothing in the routine writes a settings file, a profile or any other persistent state, and there is
no flush call that reaches disk: step 11's hook is empty in this build. Saving therefore happens
elsewhere, before `BSP_WinMain` reaches shutdown.

### Release-chain safety

Steps 13, 15 and 27 call vtable slot 0 with argument 1 (the MSVC scalar deleting destructor) on a
global **without** clearing it. That is not a double free: every one of these singletons derives from
the singleton base whose destructor (for example 007365e0, 00736750, 00736890) clears its own global
under the lifetime manager's lock. Step 17 re-tests DAT_00f8bc4c after step 13 destroyed it and takes
the null path. The one-object-file helpers at 00736xxx encode exactly this contract.

### Datatable unload (step 14)

Each of the four blocks is identical apart from the path and the EH state index (0, 1, 2, 3):

1. `BSP_NativeString_Resize(length, 1)` allocates the buffer; length and pointer live in the local
   pair (`[ESP+8]` = length, `[ESP+0xc]` = pointer), then `_memcpy` copies the literal.
2. `004fc150` returns the file-store factory singleton (DAT_0109db68) into EAX.
3. `PUSH &path`, ECX = factory, `CALL 00be80b0` (`BSP_FileStoreFactory_GetOrCreate`, no stack
   arguments, plain `RET`, provider in EAX).
4. ECX = provider, `CALL 00be7130`, which consumes the pushed `&path` and returns with `RET 4`.
5. The string buffer is returned to the sized storage pool
   (`BSP_SizedStoragePool_GetSingleton_Provisional` then `BSP_SizedStoragePool_ReturnBlock_Provisional`).

Paths, in order: `scripts/datatables/inputs.lua` (0x1d bytes), `scripts/datatables/keyboardsetup.lua`
(0x24), `scripts/datatables/controllerinputnames.lua` (0x2b), `scripts/datatables/controlpresets.lua`
(0x25).

The single push shared by two calls is the reason the decompiler is wrong here: Ghidra models
00be80b0 as returning void and 00be7130 as taking the string in ECX. The assembly at 00738131-0073813b
shows `PUSH EDX / MOV ECX,EAX / CALL 00be80b0 / MOV ECX,EAX / CALL 00be7130`, and 00be7130's prologue
reads its stack argument at 00be7148 (`MOV EDX,[ESP+0x20]`, which is entry ESP+4) and returns `RET 4`.
So 00be7130 is `__thiscall RemoveFile(this = provider, const cString *path)`.

## The 00736xxx helper object file

Every routine between 00736100 and 007369e0 is an instantiation of the same singleton template, all
sharing the layout of the lifetime manager: the manager pointer comes from
`BSP_SingletonLifetime_GetManager` (00415350), its lock object is at manager+0x10, and that lock is a
`CRITICAL_SECTION` followed by a depth counter at lock+0x18 (`ADD dword ptr [ESI+0x18],1` at
007361e1, matched by a decrement before `LeaveCriticalSection`). The decompiler renders that counter
as `lpCriticalSection[1].DebugInfo` arithmetic, which is noise.

Three roles appear:

- **Register** (00736100, 007364a0, 00736540, 007366b0, 007367f0): `__fastcall f(void *this)`
  returning `this`. Stores the class vtable into `*this`, takes the manager lock, publishes `this`
  into the type's global, and calls `BSP_SingletonLifetime_Register` (00bd0c30).
- **Unregister** (007365e0, 00736750, 00736890): `__fastcall f(void *this)`. Stores the class vtable,
  takes the manager lock, calls `BSP_SingletonLifetime_Unregister` on the manager, clears the global,
  and finally stores the base vtable (`PTR_LAB_00ce3818`) so the base destructor runs next.
- **Destroy** (007361a0, 00736250, 00736300, 007363b0, 00736930, 007369e0, and 004c0ef0 outside this
  range): `void f(void)`. Fast-path test of the global outside the lock, then under the lock:
  unregister, invoke the scalar deleting destructor with argument 1, and null the global.

`FUN_00bcfca0` is the unregister primitive: `__thiscall f(manager, object)`, it linearly searches the
manager's pointer vector (bounds from manager+4 and manager+8, count via `BSP_PointerVector_Count`)
and, on a hit, writes 0 into that slot. It does not erase the slot, so the vector keeps its length and
the manager later walks a vector with holes. A null object argument is ignored.

00736460 is unrelated to the singletons: `__thiscall push_back(vector, const void *value)` with
doubling growth through 00735ec0, minimum capacity 1. Nothing in the binary calls it, so it is a
template instantiation kept by the linker; it is listed here because it lives in the same object file.

Global-to-routine map. "Register" and "Unregister" are the base-subobject helpers whose bodies were
read; "Accessor" is a verified double-checked lazy create-and-register; "Other writers" are functions
that write the global but were not opened, so their role is unclassified.

| Global | Register | Unregister | Accessor | Destroy | Other writers |
| --- | --- | --- | --- | --- | --- |
| DAT_0109cf00 | 00736100 | - | - | none in this range | 00737400 |
| DAT_00f8c26c | 007364a0 | - | - | none; destroyed inline at 007382d2 | 007375e0 |
| DAT_00f8bc4c | 00736540 | 007365e0 | - | none; destroyed inline at 007382f2 | - |
| DAT_00f8aefc | 007366b0 | 00736750 | - | none; released at 0073804f | - |
| DAT_010909b0 | 007367f0 | 00736890 | - | none; released at 0073803d | - |
| DAT_00f8bbf8 | - | - | 004bec00 | 007361a0 | 00a92180 |
| DAT_00f8bc5c | - | - | 004c12b0 | 00736250 | - |
| DAT_00f8bf44 | - | - | 007371d0 | 00736300 | 00ac31f0 |
| DAT_0109cf14 | - | - | - | 007363b0 | 004c14c0 |
| DAT_00f8d420 | - | - | - | 00736930 | 004b4f10, 00b1b680, 004de4b0 |
| DAT_01090490 | - | - | - | 007369e0 | 00b97480 |
| DAT_00f8753c | - | - | - | 004c0ef0 | 00418280, 00424c40 |

## Individual callees

**004c12b0** `int __cdecl f(void)`, no arguments, returns DAT_00f8bc5c. Double-checked lazy
construction: allocate 0x88 bytes through the CRT allocator (00bf681b), construct with 00aa5d70,
register with the lifetime manager, all under the manager lock. Calling it first in shutdown means a
GUI manager can be *created* during shutdown if one never existed. 00aa5d70 lives in the segment
whose string keywords include `cGuiManager`, which is the only basis for the GUI attribution.

**00aa0f70** `void __thiscall f(void *manager)`. For each of manager+0x74 and manager+0x78, when
non-null: writes the triple (-1.0f, -1.0f, 0.0f) into a local (the constant at 00d7a260 is
0xBF800000 = -1.0f), passes it to 00aa8240 with the screen as ECX, then calls the screen's virtual
slot +0x34 with a single argument 0. 00aa8240 is a position setter that composes against a parent at
+0x70. The mechanics are certain; reading slot +0x34 as "close" or "hide" is a hypothesis.

**004dc5c0** `void __thiscall GGame::OnDestroy(void *game)`. Traces `GGame::OnDestroy()`, writes the
state value 0x10 to game+0x5d4, then calls, in order: 0046f3e0, 004da780, 004da650; if DAT_00f8a2fc,
its virtual slot +8 followed by 00992ac0; then 00b65e80, 008844f0; deletes the members at game+0x1a08,
game+0x21a0, game+0x21d8 and the globals DAT_00e18678, DAT_00e1867c, nulling each; finally 004a9ac0.
004da780 is the scene teardown and carries the literals `collectgarbage("collect")`,
`PostScnTermModels`, `PostScnTermShaders`, `PostScnTermSounds`, `PostScnTermTextures` and
`After scene term`, so the scene is terminated and the script garbage collector runs before the
model, shader, sound and texture pools are released. This routine was analyzed only one level deep.

**00a3b6e0** `void __thiscall f(void *client)`. Under the critical section at client+0x14c, sets the
quit flag DAT_00f8abe0 to 1; then loops `Sleep(10)` re-taking the lock each iteration until both
DAT_00f8abe1 and DAT_00f8abe2 are zero; then `CloseHandle(client+0x154)` and
`CloseHandle(client+0x158)`. That is a stop-and-join of two worker threads with a 10 ms poll and no
timeout: a wedged worker hangs shutdown forever. The client global DAT_00f8abdc is built by 00a3cfa0
in the network segment.

**0051f460** `int __cdecl f(void)` returning DAT_0109db70, the same double-checked lazy singleton
shape: 0x34 bytes, constructor 00be94f0. **00be8350** takes that object in ECX and is a single `RET`.

**004fc150** `int __cdecl f(void)` returning DAT_0109db68: 0xc bytes, constructor 00be5320, and the
value registered with the lifetime manager is the object **plus 4**, not the object pointer, so the
registered base subobject sits at +4.

**00be7130** `void __thiscall cFileStore::RemoveFile(void *provider, const cString *path)`, `RET 4`.
Normalizes the path copy (00bee780), looks it up in the container at provider+0x14 with the end
sentinel at provider+0x18 (00be5e90), erases it on a hit (00be6760) and traces
`cFileStore::RemoveFile: %s removed!` or, on a miss,
`WARNING: cFileStore::RemoveFile: remove file %s not in store!`. The temporary is returned to the
sized storage pool.

**00bbc5d0** `void __cdecl f(void)`: if DAT_01090900 is non-null, call 00bbc520 on it and `free` it.
Note it does not null the global afterwards.

**00bb4db0** static destructor stub: `MOV ECX,0x010904e0 / JMP 0041cc80`. **004c9c90** and **00888750**
are both a bare `RET`; 004c9c90 receives the trace message in ECX.

## Reconstruction

`include/bsp/app_shutdown.hpp` and `src/app_shutdown.cpp` implement
`bsp::shutdown_application(ApplicationShutdownHost&, ApplicationShutdownState&)`. None of the
subsystems above are reconstructed, so every effect is expressed through the injected
`ApplicationShutdownHost` interface, and the recovered order is a `constexpr` table
(`bsp::kApplicationShutdownSteps`) that the driver walks in sequence. The reference-count release loop
and the null-guard structure are reproduced literally; the singleton globals are identified by their
binary addresses rather than by guessed subsystem names, because only the address is proven.

No global, type or subsystem was invented to make the module compile, and no pseudocode was pasted in.

## State reached

| Routine | State |
| --- | --- |
| 00737f30 | analyzed, order reconstructed and build-tested through `shutdown_application` |
| 007361a0, 00736250, 00736300, 007363b0, 00736930, 007369e0, 004c0ef0 | exported, analyzed (identical template) |
| 00736100, 007364a0, 00736540, 007365e0, 007366b0, 00736750, 007367f0, 00736890, 00736460 | exported, analyzed |
| 004c12b0, 004fc150, 0051f460, 00be8350, 00be7130, 00bbc5d0, 00bcfca0, 00aa0f70, 00a3b6e0 | exported, analyzed |
| 004dc5c0 | exported, analyzed one level deep; its callees are only classified by segment |
| 00bb4db0, 004c9c90, 00888750 | exported, analyzed (stubs) |

## Uncertainties and what remains

- The identity of every destroyed singleton is unproven beyond its global address. The subsystem
  hints in this document come from segment keyword clusters (sound/FMOD for DAT_00f8bbf8 and
  DAT_00f8bbf4, foliage and terrain for DAT_00f8c210, DAT_00f8c218, DAT_00f8c274 and DAT_00f8c280,
  particles for DAT_00f8d39c, network for DAT_00f8abe8 and DAT_00f871b4), which are candidate-graph
  hints, not evidence. Naming them requires opening each constructor.
- Virtual slots +0x0c and +0x14 on DAT_00f8d394 are unread. The global is consumed by
  `BSP_TextContext_CreateGlyphBuffers`, `BSP_Material_CreateForEffectName` and
  `BSP_DrawSection_RebuildVertexLayout`, so it is a rendering device, and the two calls are its first
  teardown step, before any object is released.
- What populates the system array at this+0x08 was not found; `BSP_Application_Initialize` does not
  write those two fields directly in its decompilation.
- Virtual slot +0x34 on a GUI screen is unidentified.
- 004dc5c0's seven callees are unexplored beyond their segments, so the in-game teardown order below
  the scene level is not recovered.
- `bsp.py lookup 00737f30` lists `After MitApp::Deinit` under strings; the routine references the
  literal only as a bare ECX pointer to a stubbed trace, which is worth knowing before treating that
  string list as call arguments.
