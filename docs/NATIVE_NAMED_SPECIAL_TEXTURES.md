# Native named cube and volume constructors

`native_named_special_textures.cpp` reconstructs the complete named cube
`00B3CED0` and volume `00B3CFA0` constructors and their `00B34280` / `00B342D0`
named-base wrappers. These are new MSVC Win32 C++ interfaces over actual owner
storage. The descriptive names are hypotheses; no original ABI replacement or
game validation is claimed.

| Entry | Complete bytes | Original contract |
|---|---|---|
| `00B3CED0` | `[00B3CED0,00B3CF51)`, 129 | ECX owner; stacked name, cube COM, flags; EAX owner; RET 0Ch |
| `00B3CFA0` | `[00B3CFA0,00B3D02F)`, 143 | ECX owner; stacked name, volume COM, flags; EAX owner; RET 0Ch |
| `00B34280` | `[00B34280,00B342A5)`, 37 | Same three arguments; actual B34120 then D5F280; RET 0Ch |
| `00B342D0` | `[00B342D0,00B342F5)`, 37 | Same three arguments; actual B34120 then D5F2C0; RET 0Ch |

The sole observed caller is `B2C2D0`: cube call `B2C724` follows allocation
`B3F2C0` and receives the nonnull D3DX cube output; volume call `B2C7F7` follows
`B3F2D0` and receives the corresponding volume output. EBP supplies the actual
name header, flags are zero, and ECX is the allocator result. The caller later
assigns the retained input stream to cube `+2C` or volume `+30` through B23640.
Those producer and later ownership operations belong to the loading packet.

## Actual domains and ownership

Cube construction takes the existing `NativeCubeTextureOwnerContext`. Volume
construction takes `NativeNamedVolumeTextureContext`, which borrows the existing
`NativeVolumeTextureOwnerContext` and the same actual `0108D6E8` serial word used
by cube and 2D construction. Both require notification's string provider to be
the same `ActualNativeStringPoolStorage` used for later destruction. A semantic
string pool cannot silently satisfy this interface.

Both base wrappers call the established actual B34120 implementation. It creates
one native `+04` count, copies the name into the actual string pool, stores the
incoming COM reference and flags, stores the current serial, then increments
that same serial word with native wrapping. The wrappers only change the profile
after B34120 returns; they add no count or serial operation. There is no new
companion registry, pool, resource cache or ownership counter in this module.

The constructors do not allocate or return a pool slot, call COM AddRef, or
release the incoming COM reference. On completed construction the existing cube
or volume terminal consumes that incoming reference. On constructor failure the
caller still owns the raw slot and COM acquisition; the constructor's unwind
cleans only the completed named base. `NativeNamedSpecialTextureAcquired` can
retain that input identity, completion and cleanup diagnostics without taking a
second reference or automatically rolling back anything. A supplied frame is
single-use, including an attempt rejected by the actual-string-domain check.
Canonical registration remains the caller's responsibility.

## Native query and store order

Assembly is required because the decompiler does not recover the two indirect
COM calls' stack cleanup and produces spurious stack/register variables.
Both constructors capture the input COM in EDI and owner in ESI. They install
their final profile before clearing the retained-source field, then query the
captured input COM's current table `+44` with `(COM, level0, descriptor)`;
this is a stdcall 12-byte GetLevelDesc invocation. They reload the same input
object's current table for `+34`, a stdcall four-byte GetLevelCount call.
They never substitute a reload of owner `+10` for the captured input.

Cube installs D61870, clears `+2C`, stores descriptor Width from `+18` into
owner `+24`, obtains/stores the level count into `+14`, then reads and stores
descriptor Format into `+18`. Owner `+28` and the cube slot index `+30` remain
unchanged. Volume installs D618B0, clears `+30`, captures all three descriptor
dimensions before any dimension store, then writes Height `+28`, Width `+24`
and Depth `+2C`. After GetLevelCount it reads Format before storing count and
format. The volume slot index `+34` remains unchanged.

Descriptor storage is not initialized and both HRESULTs are ignored. DWORD
loads/stores retain these observed orders; no zero-filled descriptor or
successful-result fallback is introduced. Failed COM queries leaving unwritten
fields do not establish deterministic metadata.

## Failure evidence and analysis bounds

Each native constructor has one FH3 cleanup state, armed after the first COM
slot is captured and before its invocation. The cube FuncInfo DF7518 uses
unwind map DF7510, whose sole state calls CBED20: recover ECX from `[EBP-30]`
and jump to B34090 at CBED23. Volume FuncInfo DF7544 uses DF753C / CBED40,
recovering ECX from `[EBP-2C]` and jumping to B340F0 at CBED43. Both reach
the existing actual B33F50 named-base cleanup. There are no catch maps.
Source uses armed cleanup and preserves the established FH3-compatible
termination boundary for a second C++ exception during name cleanup.

All four owned bodies are fully present in Ghidra. The supporting handler stubs
`[CBED28,CBED32)` and `[CBED48,CBED52)` are not currently Ghidra functions;
their pinned bytes load DF7518 / DF7544 into EAX and jump to the existing
BF6B43 CxxFrameHandler3. They are recorded as supporting analysis gaps, with
no claim to reconstruct or execute those original handlers. Ghidra was read
only, using the BSP wrappers against `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`.

## Validation

The normal `scripts/build.ps1` Win32 build passed, with the existing CTest test
passing 1/1. The new module was then compiled directly with `/W4 /WX` in the
focused fixture because `cmake/startup.cmake` remained leased by another
orchestrator. Registry integration is still required when that lease clears.

Ignored `local/named_special_probe.cpp` executes all 346 copied original
constructor/base-wrapper bytes and the reconstructed constructors against real
HAL-created D3D9 cube and volume textures. Only the base wrappers' B34120
calls are relocated to the established concrete source callee, using the same
actual string pool and serial word. The two constructor-to-wrapper CALLs are
relocated within the isolated instruction page; installed files are unchanged.

The fixture passed matching owner DWORDs and copied names, serial wrap and
increment, unchanged constructor COM counts, preserved opaque/pool fields,
completed creator diagnostics, real zero-count B3F410/B3F430 terminals, COM
releases, native slot returns, both actual pool shutdowns and actual string
singleton shutdown. COM observer references are explicit fixture references,
released after verifying the real owner terminals consumed exactly one each.
The volume pool reuses the actual 38h-slot algorithm over distinct volume pool
storage, with the same shared allocator list; no surface pool is substituted.

The renderer and empty registry are supplied raw preimages for reached terminal
notification, not products of a tested renderer constructor. Normal constructor
return is tested; original FH3 exception dispatch, pool-failure paths, canonical
registration, D3DX/cache loading, device reset and game execution are not.
See `reports/native_named_special_textures.json` for pinned byte hashes, every
primary call site and the exact retained fixture/build artifacts.
