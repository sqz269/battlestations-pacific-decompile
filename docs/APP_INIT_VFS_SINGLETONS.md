# VFS singletons and resource parsers in cSkeletonAppMidway::Init

Addresses: 00beda60, 00be0660, 00736a90, 00736b60, 00736c30, 004c1400, 00736dd0,
00736ea0, 00bda6f0, 00be1dc0, 00bb83a0, 00bb4fb0, 00bb8240, 00bd9230, 00bd9f90,
00bb40b0, 00530620, 00735b30.

This packet joins the VFS bring-up in phase 2 to the resource-parser bring-up in
phase 6 of `BSP_Application_Initialize` (0073d410) and fills the two singletons
no existing doc covered, 00736b60 and 00736c30. Everything below is read from
the saved Ghidra program and the installed PE; names are descriptive hypotheses,
not recovered symbols.

## The ordered sequence

Three non-contiguous spans of Init belong to this packet. Renderer, input,
settings and audio bring-up run between the second and the third.

| # | Address | Call | ABI | Effect |
| --- | --- | --- | --- | --- |
| 1 | 0073d604 | gate `DAT_0109ceec == 0` | `CMP`/`JNZ 0073d890` | Skips steps 2-13 when the provider manager already exists |
| 2 | 0073d610 | 0073c3b0 hardware probe | ECX app, RET | Bootstrap packet; inside the gate |
| 3 | 0073d615 | `malloc(0A0h)` | cdecl | Provider manager storage; a null result skips step 4 |
| 4 | 0073d637 | 00beda60 | ECX block, RET | Constructs the manager and registers physical factory 00bed990 |
| 5 | 0073d642 | `manager+90h = 00530620` | store | No-op mount-failure handler |
| 6 | 0073d652 | `manager+8Ch = 00735b30` | store | Second no-op handler |
| 7 | 0073d66d/675 | 004fc150 then 00be0660 | RET / ECX+stack RET 4 | Registers the FileStore factory |
| 8 | 0073d680/688 | 00736a90 then 00be0660 | RET / ECX+stack RET 4 | Registers the MPKG factory |
| 9 | 0073d697 | `GetCurrentDirectoryA(0FAh, buf)` | stdcall | Shared system-path buffer |
| 10 | 0073d6ac | `_strcat_s(buf, 100h, "\")` | cdecl | Literal `\` at 00ce7894 |
| 11 | 0073d6f9 | 00be1890 | ECX manager, 5 stack args, RET 14h | Mount cwd -> `.` at priority 0 |
| 12 | 0073d792 | 00be1890 | as above | Mount cwd -> `persistent_data` at priority 99 |
| 13 | 0073d829 | 00be1890 | as above | Mount `filestore` -> `.` at priority 300 |
| 14 | 0073d881, 0073d888 | 0073cb10 twice | ECX app, RET | The `.` / `mpkg` package scan |
| 15 | 0073d894 | 00738360 | ECX app, RET | Resource search paths; the `JNZ` target, so ungated |
| 16 | 0073d955/95d | 00736b60 then 00be0660 | RET / ECX+stack RET 4 | Registers the MPAK factory; ungated |
| 17 | 0073d968/970 | 00736c30 then 00bd9230 | RET / ECX+stack RET 4 | `manager+88h` = the PAK registry |
| 18 | 0073d983 | 00bd9f90 | ECX manager, stack byte, RET 4 | `manager+78h` = `DAT_00e1ae76`, the cachedload byte |
| 19 | 0073d988 | 00bb40b0 | cdecl, RET | `DAT_010904e0` = a new critical section |
| 20 | 0073db41/48/50 | 004c1400, 00736dd0, 00b80a50 | RET / RET / ECX+stack AL RET 4 | Registers the `AnimationChannels` parser |
| 21 | 0073db55/5c/64 | 004c1400, 00736ea0, 00b80a50 | as above | Registers the `Bone` parser |

Steps 11-13 push, in order, system path, virtual path, signed priority,
ownership byte and device id. The exact values are:

| Call site | System path | Virtual path | Priority | Ownership | Device id |
| --- | --- | --- | ---: | ---: | ---: |
| 0073d6f9 | current directory + `\` | `.` (00ce3a70) | 0 | 1 | -1 |
| 0073d792 | current directory + `\` | `persistent_data` (00cff208) | 99 | 1 | -1 |
| 0073d829 | `filestore` (00cff1fc) | `.` (00ce3a70) | 300 | 0 | -1 |

Every 00be1890 return value is discarded at all three sites, and every temporary
native string is released through 00419cc0 + 00bd1510 immediately afterwards.

## Globals

| Global | Written by | Read by |
| --- | --- | --- |
| `DAT_0109ceec` provider manager | 00bda6f0 (`this`), 00bda790 (0 on teardown) | 0073d604 gate, then 0073d63c/64c/65c/67a/6eb/784/80a/94f/962/97c, the frame tick 00bdb0b0, 00bdcb54, 00885350 |
| `DAT_010904f4` MPKG factory | 00736a90 | 00736a90 only |
| `DAT_010904d4` MPAK factory | 00736b60 | 00736b60 only |
| `DAT_010904d8` PAK registry | 00736c30, cleared by 00bb4610 | 00736c30, 00bb8240 |
| `DAT_010904dc` cached mpak provider | 00bb82f0 | 00bb83a0 |
| `DAT_010904e0` PAK lock | 00bb40b0 | 00bb82f0, 00bb83a0 |
| `DAT_00e1ae76` cachedload | 0073cf5d in 0073ce20 | 0073d975 |
| `010901c4` resource manager | 004c1400 | 16 call sites including 00717e80 |
| `DAT_0109cf00` platform gate | 00737e20 path | 0073d899 |

## Gates

The first-time gate is `DAT_0109ceec == 0` at 0073d604 and it covers steps 2-14.
`DAT_0109ceec` is not stored by Init or by 00beda60 directly: the singleton base
constructor 00bda6f0, reached through 00be1dc0, takes the singleton-lifetime
lock, assigns `DAT_0109ceec = this` and registers the manager for lifetime. The
manager destructor 00bda790 unregisters and zeroes the same global, so a second
Init pass after a teardown runs the whole block again.

Steps 15-21 have no gate of their own. Step 15 is the `JNZ` target of the
first-time gate. Steps 16-19 sit after the separate `DAT_0109cf00 == 0` gate at
0073d899, which covers only 00737e20, and steps 20-21 are unconditional.

No command-line flag reaches this packet before step 16. The full switch table
`BSP_Application_ParseCommandLine` runs at 0073d94a, after every mount and both
package scans, so `-cachedload` cannot influence phase 2. Its only effect here is
the byte handed to the file manager at step 18.

## 00736b60, the MPAK provider factory

`00736b60` is a plain `RET` at 00736c2d taking no argument and returning the
singleton in EAX. It is the same double-checked lazy 8-byte singleton shape as
the MPKG factory and the two parsers: `malloc(8)` at 00736bba under the lifetime
lock, base lifetime vtable 00cfea00 written at +4, then primary vtable 00cfea20
at +0 and final lifetime vtable 00cfea1c at +4, with `object+4` registered
through 00bd0c30. Primary vtable 00cfea20 holds `{007370d0, 00bb83a0}`.

`00bb83a0` is the create method: ECX factory, two stack strings, `RET 8` at
00bb84b3 and three sibling epilogues. When the system-path length exceeds 4 it
takes the last five characters through `BSP_NativeString_Substring` and compares
them to the literal `.mpak` at 00ceba84 through 00425850; anything else returns
null. On a match it enters critical section `DAT_010904e0`, reads
`DAT_010904dc` and returns that cached provider when it is set, otherwise
allocates 0x44 bytes and constructs the provider through 00bb8240.

So the startup factory list is four entries, not three: physical directory
(00bed990, registered inside 00beda60), FileStore (004fc150), MPKG (00736a90)
and MPAK (00736b60). The MPAK factory is registered at 0073d955, after both
0073cb10 package scans, and 0073cb10 enumerates extension `mpkg` only. No
`.mpak` archive can therefore be mounted by startup package scanning; the factory
exists for later explicit mounts.

## 00736c30, the PAK archive registry

`00736c30` is a plain `RET` at 00736cf6 returning `DAT_010904d8`. It lazily
allocates 0x1c bytes at 00736c8a under the lifetime lock and constructs them with
00bb4fb0, whose lifetime subobject is at `+8`; that adjusted pointer is what
00bd0c30 registers. 00bb4fb0 writes base vtable 00ceb130 then final primary
vtable 00d64190 at `+0`, reference count 1 at `+4`, base lifetime vtable 00d64188
then final 00d6418c at `+8`, decimal 100 at `+0Ch`, and zeroes `+10h`, `+14h`
and `+18h`.

Primary vtable 00d64190 is `{00bd30e0, 00bb5410, 00bb5770, 00bb5910}`. The last
two resolve names through `BSP_VFS_ResolveExistingName` and walk a list whose
count sits at `+14h`. The mpak provider constructor 00bb8240 calls this getter
and copies the `+0Ch` value 100 into every provider at `+18h`, right before
logging `+PAK begin %s` and calling manager virtual `+4`. 00bb4610 zeroes the
global during teardown.

Step 17 publishes the same object on the provider manager at `+88h`. This is the
piece `APP_INIT_PLATFORM.md` left open: the lock 00bb40b0 creates is the lock
`00bb83a0` takes, and the registry it guards is reached from the MPAK provider
factory that step 16 just registered.

## Corrections to existing docs

These docs are not edited by this packet; the corrections are listed here.

1. `APP_INITIALIZE_MAP.md` step 9 says 00beda60 constructs "into `DAT_0109ceec`".
   The store happens two frames deeper, in 00bda6f0 through 00be1dc0, under the
   singleton-lifetime lock and together with a lifetime registration. Init only
   supplies the `malloc(0A0h)` block.
2. `PROVIDER_FACTORY_STARTUP.md` says "The resulting initial order is physical
   directory, FileStore, MPKG", and `ARCHIVE_PROVIDER_ENTRY.md` describes the
   same three. A fourth factory, MPAK 00736b60, is registered at 0073d955.
3. `PROVIDER_FACTORY_STARTUP.md`'s startup mount table lists only the two
   physical mounts. The third mount, `filestore` -> `.` at priority 300, pushes
   ownership byte **0**, not the 1 the two physical mounts push.
4. `APP_INITIALIZE_MAP.md` step 12 describes the mounts as "three
   `BSP_VFS_MountSystemPath` calls at priorities 0, 99 (`persistent_data`) and
   300 (`filestore`)". `filestore` is the *system* path of that third call and
   `.` is its virtual path, the reverse of the first two calls.
5. `APP_INITIALIZE_MAP.md` step 9 names only `+90h`. `manager+8Ch` receives
   00735b30, which like 00530620 is a single `C3` byte followed by INT3 padding,
   so both handlers are verified no-ops. 00735b30 has no Ghidra function.
6. `APP_INIT_PLATFORM.md` says of `DAT_010904e0` that "which registry that is was
   not established". It is the registry behind the MPAK provider factory: the
   lock guards `DAT_010904dc`, the cached mpak provider that 00bb83a0 returns.
7. `RESOURCE_MANAGER_REGISTRATION.md` gives the two application parsers' primary
   vtables but not their lifetime vtables. `AnimationChannels` writes base
   00cfea08 then final 00cfea34; `Bone` writes base 00cfea0c then final 00cfea44.
   Both are confirmed at 00736e38/e3f/e45 and 00736f08/f0f/f15.

Everything else those docs state about 00beda60, 00be0660, 004fc150, 00736a90,
004c1400, 00736dd0, 00736ea0 and 00b80a50 was checked against the program and
matches.

## Consequences for later phases

- The manager in `DAT_0109ceec` is required by steps 17-18, by the per-frame
  provider tick 00bdb0b0 in `run_application_frame`, by 00bdcb54 and by 00885350.
- Mount priority ordering is descending and signed, so `filestore` (300) is
  searched before `persistent_data` (99) before the loose root (0). Every asset
  load after phase 2 sees that order.
- The two parsers of steps 20-21 extend the six the manager constructor already
  registers. The five game parsers of 00717e80 are registered later still, from
  004ddb90, outside Init.
- Init calls 004c1400 twice, once per registration, rather than caching it.

## Uncertainties and what remains

- The decimal 100 at PAK-registry `+0Ch` and the list at `+10h`/`+14h`/`+18h` are
  not interpreted. The name `BSP_PakArchiveRegistry_GetSingleton` rests on the
  mpak provider being the only observed consumer, and is provisional.
- 00bb5770 and 00bb5910 are read only far enough to see the VFS name resolution
  and the `+14h` count; their bodies are not reconstructed.
- Step 5 and step 6 re-read `DAT_0109ceec` with no null check, so a failed
  `malloc(0A0h)` at step 3 would store through a null manager. Whether the native
  allocator can return null there is not established.
- 00be0660 performs no duplicate check, so a second Init pass would append the
  MPAK factory again. Whether Init is ever re-entered is not established.
- The current-directory buffer is filled with a length of 0FAh but passed to
  `_strcat_s` with a size of 100h. The declared buffer size is not recovered.
- `00bb82f0`, which writes `DAT_010904dc`, is not analyzed here.

## State reached

| Address | State |
| --- | --- |
| 00beda60 | exported, analyzed, verified against existing docs |
| 00be0660 | analyzed, verified; append-without-duplicate-check confirmed in assembly |
| 00736a90 | exported, analyzed in full, named |
| 00736b60 | exported, analyzed in full, named |
| 00736c30 | exported, analyzed in full, named |
| 004c1400 | analyzed, verified against `RESOURCE_MANAGER_REGISTRATION.md` |
| 00736dd0 | analyzed, verified; lifetime vtables added |
| 00736ea0 | analyzed, verified; lifetime vtables added |
| Sequence | reconstructed and build-tested as `bsp::run_vfs_startup` |

`include/bsp/vfs_startup.hpp` and `src/vfs_startup.cpp` project the three spans
onto `VfsStartupHost`, one method per native call site or store, in the style of
`bsp::run_application_frame`. They build Win32 under `/W4 /WX` and the existing
CTest passes. No new test was added: the sequence is straight-line, and a test
over the mount table would only mirror the constants it asserts. There is no
native ABI replacement, no fixture parsing and no game validation in this packet.
No Ghidra metadata was changed; the three names above are ledger records only.
