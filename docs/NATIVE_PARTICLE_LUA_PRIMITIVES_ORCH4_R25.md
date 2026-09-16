# Particle reader Lua primitives: explicit source error boundaries

## Result and scope

The four remaining Particle-reader call sites now select explicit protected
string/iteration adapters. Raw `00B662B0`, `00B67080`, and `00B67190` keep Lua's
nonlocal error-transfer contract. Raw named/string lookups `00B67800` and
`00B68100` are unchanged. Iterator preparation and publication are shared
between the raw and protected entries so their ordinary execution order does
not diverge.

These are source-only C++ reader boundaries over pinned Lua 5.1.1, continuing
[r24](NATIVE_PARTICLE_LUA_UNWIND_ORCH4.md). `luaD_pcall` invokes a trivial callback
in the current Lua C frame and inherits `L->errfunc`. A returned error becomes
`NativeLuaOperationError`, derived from `std::exception`, only after Lua restores
the frame. The handler's error value is consumed. No exception may cross an
arbitrary Lua C callback; a future dispatch bridge must define that translation.

## Original evidence

Project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, original
PE SHA-256 and eight complete PE/live byte regions are in
`reports/native_particle_lua_primitives_orch4_r25.json`. All 27 direct call
instructions in those regions are checked at their exact instruction starts.
The three wrappers contain no local FS/EH registration or FH3 handler; this
does not establish how an outer original handler receives a Lua transfer.
The existing Particle reader's six-state source cleanup schedule is unchanged.

| Native body | ABI and supported order |
| --- | --- |
| `00B662B0`, 18 bytes | ECX object; state in ECX, index in EDX, null length stack argument to `00A67810`; pointer EAX, RET. |
| `00B67080`, 267 bytes | ECX table, stack key/value, RET 8; release value, release key, ignored `checkstack(2)`, push nil, next, publish key then value. |
| `00B67190`, 284 bytes | Same object ABI; release value, then capture key index; top key releases with remove=0, otherwise push copy then release current key with remove=1; next, publish key then value. |

`00A67810` calls conversion `00A6D1E0`, then GC step `00A6CEE0`, then resolves
the index again. `00A6D1E0` calls string allocation at `00A6D231`, writes the
string pointer at `00A6D236`, and changes the TValue tag to STRING at
`00A6D239`. Thus allocation failure can precede TValue publication, while a GC
finalizer error can follow it. The linked `lapi.c:339-353`, `lvm.c:47-56`, and
`lgc.c:445-470` preserve that order. No string conversion rollback is justified.

`00A683A0` calls `00A6EF60`, whose first call is `00A6EEA0`. The latter matches
stock `ltable.c:findindex`, including the invalid-key error; its current Ghidra
name `luaH_next` is too broad. `00A6EF60` matches the actual traversal/pair
publication in `luaH_next`. This audit leaves those unowned names unchanged.
The linked `ltable.c:137-179` raises for an invalid key before publishing the
next pair. Nil-first iteration of a valid table has no invalid-key error.
`lua_next` does not dispatch `__pairs` or an indexing metamethod.

## Failure-state contract

| Adapter | Completed state retained on Lua error |
| --- | --- |
| String | Entry stack height and tracked objects remain; the actual TValue may still be NUMBER or may already be STRING. Error/operation temporaries are removed. |
| First | Value and key have already been released, in that order, and are kind 0. Their stack removals and shifts of other tracked indices remain. Protection starts at this post-release top; only later operation temporaries are removed. |
| Next | Value release and key-copy/detach/release have completed; both objects are kind 0. Protection starts with one detached working key at top. Failure removes that key and error temporaries, retaining every earlier release and index shift. |

No iterator restores the entry top: that would recreate slots whose tracking
records were already removed. Publication after a successful `lua_next` uses
nonallocating `lua_gettop` calls and plain stores, so no linked Lua error occurs
between key/value tracking publication under the valid storage contract.

The next adapter requires the Particle reader's exclusively owned key/value
objects and no surviving alias to the detached work-key. It is not a general
adapter for arbitrary shared tracked aliases. `Impl` owns these objects; no
copy/assignment publishes an alias from the reader. Surviving lower stack
objects and any tracked suffix follow the original release/index-shift logic.
The valid table, interpreter, owner and caller stack/tracking domain must stay
stable across callbacks. Native unchecked slot/reference capacities and the
ignored `checkstack` return remain caller contracts.

Error-handler/finalizer side effects are retained, including prior TValue or
table mutations. The wrappers restore only the state Lua's own `luaD_pcall`
restores (`ldo.c:453-474`) plus the specified stack height; they do not promise
rollback of GC internals, finalizer progress, table changes or foreign callback
behavior. They do not make reentrant changes to the native tracking domain safe.
Null string results, invalid Lua API inputs, access violations and foreign C++
exceptions raised through Lua remain outside this boundary.

Particle's existing catch records the original call site/state, cleans only
its active locals, and rethrows. Previously written component fields and
already appended/retained resources remain. No resource rollback or retry was
added.

## Focused validation

All probes are ignored local artifacts; no permanent tests or CMake targets
were added. The report records final source, library and probe hashes/logs.

- A denied string allocation returns status 4, leaving the tracked numeric
  TValue intact. A real pending `__gc` callback fails after conversion and
  returns status 2, leaving the string TValue intact. The inherited handler
  executes once; the current CallInfo/base and prior error handler survive.
  The finalizer is scheduled with probe-only private Lua state preparation;
  the production allocator and Lua library are unchanged.
- First-iteration stack-growth OOM preserves an already completed key release
  and the resulting tracked-suffix shift. Both output objects remain unbound.
- Genuine invalid-key errors cover both next-key paths. The non-top case
  preserves a surviving suffix through both original index shifts. Raw and
  protected ordinary first/next/exhaustion paths agree and drain tracking.
- Actual current Particle source records `871EDD/0` for scalar conversion OOM,
  `871DD4/3` for table-value conversion OOM, `871DAA/3` for first stack-growth
  OOM, and `871E9C/3` for invalid next. For the last case the genuine cache-hit
  acquisition callback removes the current hash key and rehashes the table;
  a resource is already appended. The catch preserves that entry/retain and
  prior base writes while cleaning root-adjacent tracking and caller stack.
  Genuine component/cache/string-pool teardown then drains the fixture.
- The existing copied 657-byte Particle reader ordinary-flow comparison is
  rerun for scalar/table/empty-table input. Its original EH registration is
  bypassed, so it provides no original FH3/SEH execution proof.
- Final strict MSVC Win32 build and all three existing CTests pass. Probes use
  `/MD`, `/fp:strict`, `/W4 /WX`, and `/MANIFEST:EMBED`.

The result is source/build/fixture evidence. Original error-path execution,
original FH3/SEH compatibility, a retained dispatch bridge and gameplay parity
remain unproved. Ghidra was read-only; proposed comment additions are in the
report for the primary integrator to apply while preserving prior comments.
