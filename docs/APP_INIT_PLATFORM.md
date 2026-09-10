# Platform phase of cSkeletonAppMidway::Init

Addresses: 00becda0, 00beb2c0, 00bedfb0, 00be2900, 00bd9230, 00bd9f90, 00bb40b0, 00737e20,
00becee0, 00be2750, 00bede00, 00736100, 00bd1860, 00bb4fb0

Packet `app_init_platform_window`. Covers the object construction at `0073d899`-`0073d988`, the
knobs set around it, and the window configuration that `0073dc25` reaches. Device creation is the
renderer packet and is not entered here: the routine stops at the recorded renderer arguments.

Every descriptive name below is a hypothesis, not a recovered symbol. Field roles are inferred from
call sites and from the one reader each field has; they are not from a recovered class layout.

## Call order in the parent

Read from the `0073d410` listing. `00bf681b` is the raw allocator; each `new` is followed by a null
check, so every constructor below is skipped when its allocation fails.

| Site | Size | Callee | Gate |
| --- | --- | --- | --- |
| `0073d478` | `0xc` | `00be2900` allocation statistics | allocation non-null |
| `0073d4b1` | `0x80` | `00bedfb0` frame clock | `DAT_01090ab0 == 0` |
| `0073d8c0` | `0x4` | `00737e20` lifetime singleton | `DAT_0109cf00 == 0` |
| `0073d8cc` | - | `00bbcb40` water texture sources | none, other packet |
| `0073d8f3` | `0x184` | `00becda0` Win32 platform | allocation non-null |
| `0073d921` | `0x540` | `00beb2c0` save storage | allocation non-null |
| `0073d970` | - | `00bd9230` on `DAT_0109ceec` | none |
| `0073d983` | - | `00bd9f90` on `DAT_0109ceec` | none |
| `0073d988` | - | `00bb40b0` shared lock | none |

Two of those calls are dispatched on a global the phase itself fills, so the ordering is real:
`00bd9230` and `00bd9f90` both take `ECX = DAT_0109ceec`, the file/VFS manager built in phase 2, and
`00bd9f90` reads a byte that `BSP_Application_ParseCommandLine` writes at `0073d94a`, six
instructions earlier.

## Corrections to docs/APP_INITIALIZE_MAP.md

Three claims in the map are wrong and the evidence is in this packet's addresses.

1. `DAT_00e1ae7c` is **the WinMain `HINSTANCE`, not the `cachedload` argument**. It is written once,
   at `0073d92d`, from `Init`'s second parameter, and read once, at `0073dbc7`, where it becomes
   argument 11 of `00becee0`. That argument is `WNDCLASSA::hInstance` and the `hInstance` of
   `CreateWindowExA`. The `cachedload` switch is a different global, `DAT_00e1ae76`, written by
   `BSP_Application_ParseCommandLine` when the token compares equal to `"cachedload"` and read only
   at `0073d975`.
2. `00bd9230` and `00bd9f90` are **file-manager methods, not memory-manager knobs**. The map lists
   them under `memory`; the receiver at both call sites is `DAT_0109ceec`. The memory knob in this
   phase is `00be2900`.
3. `00bedfb0` is **the frame clock, not a separate timer service**. See below.

## The indirect call at 0073dc25

Resolved. `0073dbee` loads `EAX = [0x0109cf04]`, the platform singleton; `0073dbfa` loads
`ECX = [EAX]`, its vtable `00d68cc4`; `0073dc22` loads `EAX = [ECX+4]`. Slot `+4` of that vtable is
`00becee0`, already identified in `docs/PLATFORM_LOOP.md` as the window creation entry. The map
called this "the window-title call" because the argument built just before it is the title; the call
is the whole window creation.

Calling convention: `__cdecl` with eleven stack arguments, the platform pointer pushed last and
therefore first in argument order. The caller executes `ADD ESP,0x2c` at `0073dc2b` and the callee
returns with a plain `RET`. Arguments in order, with the global each is read from:

| # | Source | Meaning |
| --- | --- | --- |
| 1 | `[0x0109cf04]` | platform object |
| 2 | `LEA [ESP+0x34]` | temporary `cNativeString` holding `"Battlestations Pacific"` |
| 3 | `movzx [0x00f8899e]` | fullscreen |
| 4 | `[0x00f889e0] != 0` | colour-depth selector |
| 5 | `[0x00e1ae88]` | window x |
| 6 | `[0x00e1ae8c]` | window y |
| 7 | `[0x00f88994]` | width |
| 8 | `[0x00f88998]` | height |
| 9 | `[0x00f889d8]` | renderer option, forwarded unchanged |
| 10 | `EBP` | the application object |
| 11 | `[0x00e1ae7c]` | `HINSTANCE` |

The title string is built at `0073db96` by `BSP_NativeString_Resize(0x16, 1)` plus a `memcpy` from
`0x00cff1b8`; `0x16` is 22, the length of `"Battlestations Pacific"`. It is released at `0073dc39`
immediately after the call, so it is a temporary and the platform object keeps its own copy.
Arguments 5 and 6 have no writer anywhere in the image: both globals are read-only at these two
sites, so the window is always created at 0,0. Arguments 7, 8, 3 and 4 come from the settings block
at `0x00f88980` that `BSP_GameSettings_LoadFromRegistry` fills, which is why the map's ordering
constraint from that load to this call is real.

## 00becee0, window class, style, size and title

`00becee0` was already named and its creation core already ported as
`create_platform_window_00becee0_fragment` (`src/win32_window.cpp`, `docs/WINDOW_CREATION.md`). This
packet re-read it for the surrounding sequence and found one error in that fragment.

**The registered class style is `0x2000`, which is `CS_BYTEALIGNWINDOW`, not `CS_GLOBALCLASS`.**
`00becf36` is `MOV dword ptr [ESP+0x38],0x2000`, and `00becf83` takes the address of that same
structure for `RegisterClassA`. `CS_GLOBALCLASS` is `0x4000`. The existing fragment and both existing
docs say `CS_GLOBALCLASS`. Not changed here: `src/win32_window.cpp` and those docs are outside this
packet's ownership. `src/platform_window.cpp` uses `0x2000`.

The rest of the class matches what was already recorded: procedure `00bec3b0`, `cbClsExtra` 0,
`cbWndExtra` 24, `hIcon` null, arrow cursor `LoadCursorA(nullptr, 0x7f00)`, null background and menu.
The class name, the `CreateWindowExA` class name and the window title are the same pointer, the data
of argument 2, falling back to the shared empty byte `DAT_0109db8d` when that pointer is null.

Sequence after creation, which the existing fragment stops short of:

1. `00bed088` copies the title into the platform object's own string at `+04`.
2. `00bed0b3` takes `GetClientRect(GetDesktopWindow())` and computes the desktop aspect.
3. `00bed0f2` stores the requested size to `+24`/`+28` and the fullscreen flag to `+0c`.
4. `00bed149` sets `+14` when the requested size is exactly `0x500` by `0x2d0`, that is 1280x720.
5. `00bed15c` splits: windowed reads the real colour depth and keeps the requested position;
   fullscreen substitutes the argument-4 boolean and 0,0.
6. `00bed1a7` sets `+42`, the byte that gates the frame slot in the ported loop, then `ShowWindow`
   with `SW_SHOWNORMAL`.
7. `00bed1b8` calls the renderer singleton `DAT_00f8d394` virtual `+4`. Renderer packet; recorded,
   not entered.
8. Afterwards: a `0x14`-byte allocation through `00bec870`, `00bec3e0(10000)`, the power-scheme
   save and override through `GetActivePwrScheme`/`ReadPwrScheme`/`SetActivePwrScheme`, and
   `SystemParametersInfoA(SPI_SETSCREENSAVEACTIVE, 0, nullptr, 0)`. Machine-wide settings; not
   ported.

### The aspect arithmetic is x87

Read from the listing because the pseudocode hides it. `00bed0d9` is `FILD` of the desktop width,
`00bed0e7` is `FIDIV` by the desktop height, `00bed0fb` is `FSTP` to a float slot and `00bed0ff`
`FLD`s it back before `FST` to `+3c`. The same shape repeats at `00bed10a` for the requested size
into `+10`. So the quotient is computed at the x87 intermediate precision and then rounded to float
once before it is stored and once more, from the same float, before it is compared. The comparison at
`00bed129`-`00bed135` is `FLD` of `DAT_00d5bd98`, `FXCH`, `FCOMIP`, `JBE`, so `+0d` is set when the
active aspect is strictly greater than the threshold. `DAT_00d5bd98` is `0x3faaaaab`, that is
`1.3333334f`, 4:3.

`aspect_ratio_x87` reproduces this with a `double` intermediate, which matches the default 53-bit
control word. A single-rounding `float` divide would not be the same function. Residual risk: if the
control word is ever set to 64-bit precision, the double rounding this code performs can differ from
the native result in the last bit; no such change was found on this path, and it cannot bite at
realistic screen dimensions.

### Colour depth field encoding is inconsistent

`+18` receives `GetDeviceCaps(GetDC(window), 12)` in the windowed branch. Index 12 is `BITSPIXEL`, so
the field holds a real bit count, typically 32. In the fullscreen branch the same field receives
argument 4, which is `SETNZ` of a settings byte and therefore only 0 or 1. Open question: either the
field is overloaded, or the fullscreen path expects a selector that a later reader translates. No
reader of `+18` was traced in this packet.

## Object layouts

### Win32 platform, 0x184 bytes, constructed by 00becda0

`__fastcall(ECX=this)`, returns `this`, `RET`. Chain `00becda0` to `00be2ac0` to `00be2960`, and the
base publishes the object into `DAT_0109cf04`. Fields, merging what the constructor zeroes with what
`00becee0` and the already-ported loop write:

| Offset | Field | Evidence |
| --- | --- | --- |
| `+00` | vtable `00d68cc4` | `00becda0` |
| `+04` `+08` | `cNativeString`, class name and title | `00bed08b` treats `this+4` as the string |
| `+0c` | fullscreen | `00bed0f8` |
| `+0d` | widescreen | `00bed146`, aspect greater than 4:3 |
| `+10` | active aspect, float | `00bed126` |
| `+14` | requested size is exactly 1280x720 | `00bed162` |
| `+18` | colour depth or selector | `00bed169` / `00bed18e` |
| `+1c` `+20` | window x, y; zero in fullscreen | `00bed16c` / `00bed19f` |
| `+24` `+28` | present size | `00bed0f2` |
| `+30` | `HWND` | `00becda0` zeroes it, `00bed01f` stores it |
| `+34` `+38` | requested size | `00becf14` |
| `+3c` | desktop aspect, float | `00bed103` |
| `+40` `+41` `+44` | zeroed, role unrecovered | `00becda0` |
| `+42` | frames enabled | `00bed1a7`, read by `00bec1a0` |
| `+43` | loop finished | `00bec1a0` |
| `+48` | the application object | `00becf1a` |
| `+4c` | active power scheme id | `00bed21b` |
| `+50`-`+df` | original `POWER_POLICY` | `ReadPwrScheme` output |
| `+e0`-`+16f` | modified copy, `+118` and `+11c` zeroed | `00bed24c` loop of 0x24 dwords |
| `+170` | zeroed, role unrecovered | `00becda0` |
| `+178` `+17c` | text queue sentinel from `00bec710`, tail | `00becda0` |
| `+180` | close requested, set by `WM_CLOSE` | `00bed3b0` |
| `+181` | loop exit | `00bec1a0` |

`+180` and `+181` are the last two bytes of the `0x184` allocation, which confirms the size.

### Save storage, 0x540 bytes, constructed by 00beb2c0

`__fastcall(ECX=this)`, returns `this`, `RET`. Base `00bd47b0`, vtable `00d68c10`. Only the two
trailing strings are recovered; `+00`-`+52f` was not touched by this packet.

The routine calls `SHGetSpecialFolderPathA(nullptr, buffer, 5, TRUE)`. Folder 5 is `CSIDL_PERSONAL`.
It then builds `"\Battlestations-Pacific"`, 23 characters from `0x00d15af0`, and concatenates. The
concatenation helper `004261a0` takes the left operand in `ECX`, the result slot as its first stack
argument and the right operand as its second, and returns the result slot in `EAX`; at `00beb3bf`
`ECX` is the special-folder string, so the product directory is folder plus suffix, not the reverse.
`CreateDirectoryA` runs on it. `"\save"`, 5 characters from `0x00d68c5c`, is then appended in place
and `CreateDirectoryA` runs again. The result is stored to `+530`. Finally `"\*"`, 2 characters from
`0x00d68c58`, is concatenated onto `+530` with `ECX` set by `LEA ESI,[EBP+0x530]` at `00beb4bf`, and
that goes to `+538`.

| Offset | Value |
| --- | --- |
| `+530` | `<Personal>\Battlestations-Pacific\save` |
| `+538` | the same path plus `\*`, an enumeration pattern |

Both directories are created unconditionally and both results are ignored, so an existing directory
is not an error. A null string data pointer falls back to `DAT_0109db8c` at both `CreateDirectoryA`
sites.

### Frame clock, 0x80 bytes, constructed by 00bedfb0

`__fastcall(ECX=this)`, returns `this`, `RET`. This is not a separate timer service. Base `00bede00`
publishes it into `DAT_01090ab0` under the singleton manager lock, the vtable is `00d68d50`, and
every field the constructor writes lands on a member of the `FrameClock` already reconstructed in
`include/bsp/frame_clock.hpp` at the same offset: `+04` accumulated, `+08` update count, and the five
`ClockTimestamp` pairs at `+10`, `+20`, `+30`, `+40` and `+50`, each with its frequency word set to 1
and its tick word zeroed. `+68` and `+69` are cleared. The constructor's tail call is `00bedbd0`,
which that header already names `initialize_frame_clock_00bedbd0`.

The vtable settles two more things. Slot `+04` is `00bedbd0`, `+08` is `00bedc30`, `+0c` is
`00bedae0`, `+10` is `00beddc0`, `+24` is `00bedb20` and `+28` is `00bedb60`. So the second unresolved
indirect call the map lists, `CALL EAX` at `0073dac0`, is `enable_fixed_clock_00bedb20` with the
literal `0x32`, that is a 50 ms fixed step, taken when `DAT_00e1ae81` is set. That matches the
existing header comment that startup supplies 50, which had no call site attached to it until now.

The constructor leaves `+70` and `+78`, the fixed-step increment and synthetic counter, untouched.
`construct_frame_clock_singleton_00bedfb0` does the same rather than defaulting them.

### Allocation statistics, 0xc bytes, constructed by 00be2900

`__fastcall(ECX=this)`, returns `this`, `RET`. Base `00be2750` publishes it into `DAT_0109cefc`,
vtable `00d685f4`. `+04` is the literal `0x40000000`, which is 1 GiB when read as a byte count.
`+08` is zero. The reader is `00be3fd0`, which prints `+08` as `"Alloc at startup:%8dK"` after
shifting it right by 10 and then calls vtable `+08` for a second figure it prints as
`"Phy allocated:%8dK"`. That reader is what identifies this object as the allocation tracker and
makes the byte-count reading of `+04` the likely one; it is not proven, since no reader of `+04` was
traced.

### File manager knobs, 00bd9230 and 00bd9f90

Both are `__thiscall` with one stack argument and `RET 4`. Both receive `ECX = DAT_0109ceec`.

`00bd9230` stores its argument to `this+0x88`. The argument is the return of `00736c30`, a
lazily-built `0x1c`-byte singleton in `DAT_010904d8` whose payload constructor is `00bb4fb0`: a
reference-counted object with two vtables, `00d64190` and `00d6418c`, a reference count of 1 at
`+04`, the literal 100 at `+0c` and three zeroed words. What the 100 means was not traced.

`00bd9f90` stores a byte to `this+0x78`. The byte is `DAT_00e1ae76`, which
`BSP_Application_ParseCommandLine` sets to 1 at `0073cf5d` when a 10-character command-line token
compares equal to `"cachedload"`. So this is the `-cachedload` switch reaching the file manager.

### Shared lock, 00bb40b0

`__cdecl void(void)`, `RET`. One statement: `DAT_010904e0 = BSP_CriticalSection_Create()`. The
factory `00bd1860` allocates `0x1c` bytes, calls `InitializeCriticalSection` and zeroes a recursion
depth at `+18`, returning null when the allocation fails; `00bb40b0` stores the null without
checking. `DAT_010904e0` is read by `00bb82f0` and `00bb83a0`, a case-insensitive name registry in
the same module as the mpak and texture-source code. Which registry that is was not established.

### Lifetime singleton, 4 bytes, constructed by 00737e20

`__fastcall(ECX=this)`, returns `this`, `RET`, allocation size 4. Base `00736100` publishes it into
`DAT_0109cf00` and registers it for teardown; the destructor `00737ec0` calls `00737400`, which
unregisters and clears the global. Vtable `00cfeae4` holds one slot, that destructor. The object has
no data members at all: it exists only to be registered and later destroyed.

The pseudocode carries `WARNING: Removing unreachable block (ram,0x00737e68)`, so the listing was
read. `00737e5b` constructs a stack `cNativeString` with `BSP_NativeString_Resize(0, 1)`;
`00737e60` loads its data pointer and `00737e66` jumps over the block when it is null. The removed
block at `00737e68`-`00737e8d` is a `memcpy` from `0x00ce3a0c`, which is four zero bytes, an empty
string literal, followed by the pooled release pair. So the block is the destructor of an empty
temporary string, and the decompiler dropped it because a zero-length resize provably leaves the
pointer null. Nothing is lost: the temporary is constructed and destroyed without being read, and
the assembly agrees with the pseudocode on everything that survives. No behaviour is missing from
the reconstruction because of this warning.

## Reconstruction

`include/bsp/platform_window.hpp` and `src/platform_window.cpp`, registered through
`cmake/startup.cmake`.

`configure_platform_window_00becee0` takes explicit arguments in the native order and a
`PlatformWindowHost` with one virtual per Win32 call the native routine makes, in call order, so the
whole routine runs against a recording double with no window, no class registration and no device.
It records the renderer arguments in a `RendererInitRequest` rather than issuing them. The existing
`create_platform_window_00becee0_fragment` remains the audited real-Win32 creation core and is not
duplicated or replaced; this routine is the surrounding sequence. Nothing here uses the existing
`bsp_platform_probe`, which drives a real message queue and is not the right shape for a fake host.

Also reconstructed: `construct_win32_platform_00becda0`, `initialize_save_storage_00beb2c0` against
an injected shell interface, `construct_allocation_stats_00be2900`, the two file-manager setters,
`create_shared_lock_00bb40b0` and `construct_frame_clock_singleton_00bedfb0`, which delegates to the
existing `initialize_frame_clock_00bedbd0`.

No C++ is offered for `00737e20`: the object has no state, and a projection of an empty singleton
would be an invention rather than a reconstruction.

## State reached

| Address | Name recorded | State |
| --- | --- | --- |
| `00becda0` | `BSP_Win32Platform_Construct` | reconstructed, build-tested |
| `00becee0` | `BSP_Win32Platform_CreateWindowAndDevice` | reconstructed, build-tested, one prior error corrected here |
| `00beb2c0` | `BSP_SaveStorage_Initialize` | reconstructed, build-tested |
| `00bedfb0` | `BSP_FrameClock_Construct` | reconstructed, build-tested |
| `00be2900` | `BSP_AllocationStats_Construct` | reconstructed, build-tested |
| `00bd9230` | `BSP_FileManager_SetProviderPolicy` | reconstructed, build-tested |
| `00bd9f90` | `BSP_FileManager_SetCachedLoad` | reconstructed, build-tested |
| `00bb40b0` | `BSP_Threading_CreateGlobalLock` | reconstructed, build-tested |
| `00737e20` | `BSP_Application_ConstructLifetimeSingleton` | analyzed, assembly read, deliberately not reconstructed |

Build-tested means the module compiles into `bsp_core` on Win32 with warnings as errors and the
existing CTest case still passes. Nothing here is fixture-tested against native output, ABI
compatible or game validated.

## Validation

`./scripts/build.ps1` succeeds in this worktree and `reconstructed_math` passes. No test case was
added: the project rule is to add none for routine work, and `tests/` holds only the two math
targets, which are the wrong home for a Win32 state-derivation case.

Behaviour was verified instead by a throwaway harness outside the repository, compiled at `/W4 /WX`
against `src/platform_window.cpp` and `src/frame_clock.cpp`, driving a recording `PlatformWindowHost`
through both branches. It checks the class style `0x2000` and 24 extra bytes, the platform pointer
reaching `lpParam`, the adjusted outer size reaching `CreateWindowExA` while the windowed
`SetWindowPos` uses that same outer size and the fullscreen one uses the unadjusted requested size,
the two style pairs, `HWND_TOPMOST` only in fullscreen, the stop-virtual firing only when an `HWND`
already exists, the colour-depth split, both aspect values, the 720p and widescreen flags, the title
copy, the recorded renderer constants, and the constructors. All 24 checks pass. This proves the
recovered control flow and field derivation, not binary equivalence.

## What remains

1. `+18` colour-depth encoding, above. Needs a reader.
2. `+40`, `+41`, `+44` and `+170` on the platform object: zeroed at construction, no reader traced.
3. The first `0x530` bytes of the save-storage object, and its base `00bd47b0`.
4. `00bec870` and `00bec3e0` at the end of `00becee0`, and the `0x14`-byte object the former builds.
5. Which registry `DAT_010904e0` guards, and what the literal 100 in `00bb4fb0` means.
6. What the empty singleton at `DAT_0109cf00` represents. Its only distinguishing feature is its
   position in the sequence, between the search-path registration and the water texture sources.
7. `src/win32_window.cpp`, `docs/WINDOW_CREATION.md` and `docs/PLATFORM_LOOP.md` still say
   `CS_GLOBALCLASS`. Correcting them belongs to whoever owns those files.

### Correction from docs/APP_INIT_VFS_SINGLETONS.md

The registry behind `DAT_010904e0` is the MPAK provider factory's: the lock guards `DAT_010904dc`, the cached mpak provider that `00bb83a0` returns. `00736c30` builds the related 1Ch-byte reference-counted registry in `DAT_010904d8` (refcount 1 at +4, lifetime subobject at +8, decimal 100 at +0Ch) that the mpak provider constructor `00bb8240` reads.
