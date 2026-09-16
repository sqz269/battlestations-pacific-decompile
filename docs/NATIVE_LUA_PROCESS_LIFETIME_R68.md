# Canonical Lua globals, cache and process exit (R68)

Addresses: `00CD7CE0`, `00CE0D60`, `008D44C0`, `008D5B50`, `00415350`,
`00419CC0`, `00884770`, `00B68340`, `00B66B80`.

## Application behavior

The application now shares one actual 0Ch fundamentals owner through native
`00884770` / `B68340` and the raw VFS/string/manager services. Input, font, GUI
and sound Lua environment consumers borrow `GameNativeLuaServices` through the
existing `GameScriptHost::files()`. Each environment captures size from the
first native getter and bytes from the second. Its existing semantic Lua-owner
input receives a string copy; there is no second fundamentals file read/cache
owner in this production route. Unbound semantic fixtures retain their prior
checked fallback. Interpreter ownership, early environment snapshots and
semantic DoFile remain separate from full native `B6A020` integration.

`GameNativeLuaServices` is retained before the first getter and through all Lua
closes and the actual manager drain. It binds the native deleting destructor to
the same `0108FF1C` process publication. Its guarded bootstrap is also available
for the full native renderer; native interpreter entries must activate its
`NativeLuaServiceBindings` scope. A thrown raw getter records an interrupted
operation. Existing application exit boundaries then retain partial ownership
with `_Exit(1)` rather than inventing a raw rollback; this failure path is not
runtime-validated by this packet.

The Xbox byte and region are now actual fields: byte `0108FF20`, three zero
padding bytes, and the eight-byte `NativeString` at `0108FF24/28`. Script hosts
borrow the canonical process object. The region publisher at
`008D612F..008D6153` now executes raw-pool resize with preserve=false, followed
by the current data/count copy. Compatibility publication writes the same byte
seen by native bootstrap inputs. Neither live field is reset on repeated startup.

## Process lifetime and CRT behavior

`GameNativeStringProcess` owns the canonical `01090AA0` manager publication,
`01090AA8` string-pool publication and `01090AA4` small-return gate. The existing
singleton host and VFS borrow those same cells; no replacement native manager
or pool is created by binding them. Source bookkeeping is intentionally retained
through process termination, since native exit callbacks can recreate owners
after the application object has gone. Application hosts in one process now
share this domain and require one coordinated lifetime/deletion binding.

The complete `CD7CE0` initializer registers `CE0D60` with real `std::atexit` and
preserves its return status. It makes no global assignment. The process wrapper
caches completed registration, keeps nonzero status without rollback, and never
retries an exception. The complete `CE0D60` cleanup captures nonnull region data,
then length+1, calls the actual pool getter, and returns the captured block with
alignment one. It deliberately leaves the header unchanged. Its raw schedule
uses the existing full `0041DD20` implementation. C++ destruction neither
repeats the cleanup nor invalidates its cells.

The executable calls this initializer after the wired graphics pools and before
the mesh pool, preserving their relative CRT order. The original entry is
`CE3544`. Other intervening native CRT initializers remain separate work;
`CD7CC0` initializes the shader-state-list pool at `0108FEE4`, not Lua globals.

## Evidence and validation

Fresh reads from `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, match
the installed PE for ten complete function bodies, the relevant loader-zero
cells and three CRT entries: **2,601 bytes**. Both static functions already
exist in Ghidra; the old handoff's request to define `CD7CE0` is historical.
No flow repair or new function definition was needed.

One local MSVC Win32 probe executes the original 12-byte initializer and
35-byte cleanup, relocating globals and routing their calls to the real source
CRT and reconstructed pool providers. It checks null and pooled-small cleanup,
captured return arguments, unchanged headers, raw region/Xbox producers, and
repeat startup. The actual application host then drains and is destroyed while
both region headers retain their native stale pointer bits. At real CRT exit:

1. The source cleanup recreates the canonical manager and pool. The existing
   nonzero small-return gate prevents touching the stale buffer or new ring.
2. An intervening observer checks the new pool, one registered owner, zero bump
   and empty ring, plus the unchanged source region header.
3. The original cleanup runs through the same current cells/providers; a final
   observer checks its one getter/return, size three, alignment one and unchanged
   header. No extra late-manager drain is added; process termination reclaims it.

An initial fixture expectation incorrectly assumed every new ring tail was zero.
The native constructor partitions size classes. The corrected check verifies
zero bump/live/peak instead; the failing observation is retained in the archive.
Production code did not change to satisfy that incorrect expectation.

The strict Win32 build and all three existing CTests passed. The rebuilt
application, using the hash-verified private Microsoft XLive/dependency pair and
isolated settings, presented two frames and exited zero. Its log records four
native cache getter calls during input startup, **18** by shared drain, and
`fundamentals=null` afterward. The saved 640x480 frame was inspected and shows
the front-end art and sign-in/continue prompt. No permanent tests were added.

These results cover the returning getter and observed startup/exit domain.
Large-region cleanup, allocation/registration failures, original CRT/FH3/SEH
identity, concurrency and gameplay are not established by this probe.

## Follow-up renderer composition

The full R66 renderer is still fixture-only. Its production binding can now
borrow `GameNativeLuaServices::bootstrap()` and `binding()` with canonical
globals/cache, instead of creating private Lua state publications or region
copies. Borrow the VFS's existing type counter, type cells and retained-memory
accounting as well. Vertex-declaration token CRT storage, complete resource
loading, COM lifetime through the shared drain, and focused Reset execution
still need closure. R67 supplies the actual platform HWND/active cells.

The application still uses the projected renderer constructor/device prefix
and milestone frame loop; full settings ApplyAll, native interpreter/DoFile
composition and gameplay remain open. Detailed receipts and separate tested /
integration artifact archives are in `reports/native_lua_process_lifetime_r68.json`.
