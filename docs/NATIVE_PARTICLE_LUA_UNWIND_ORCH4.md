# Particle Lua transfer: original FH3 and named-lookup source boundary

## Result

The original CRT recognizes Lua's `80000026h` longjmp exception and invokes
the armed FH3 cleanup actions in crossed base/Particle reader frames. This is
instruction and metadata evidence, not execution of the original EH path.

The base and Particle C++ readers explicitly select
`native_lua_get_by_name_protected`, which uses the pinned Lua 5.1.1 internal
`luaD_pcall` in the existing Lua C frame. A Lua error restores that frame and
the entry stack before a `NativeLuaOperationError{status}` reaches the readers'
existing C++ catches. No output or tracking reference is published on failed
lookup. Ordinary lookup order and native storage remain unchanged.

The raw `native_lua_get_by_name_00b67800` and its `get_by_string` caller retain
their original Lua nonlocal-transfer contract. Existing protected Lua C
callbacks require that contract to receive the Lua status and original error
value; they do not select the new C++ adapter.

This closes the named-lookup error hole. It does not admit a general retained
Particle dispatch bridge: iteration, numeric-to-string allocation, bridge
construction and other Lua entry points still require their own error audit.

Baseline `af9ce1480`. Evidence and validation receipts:
`reports/NATIVE_PARTICLE_LUA_UNWIND_ORCH4.json`.

Original reader ABI: ECX is the actual component, one stacked actual Lua-object
pointer, RET 4. Named lookup `B67800` takes the table in ECX, stacked output
and C-string pointers, returns that output in EAX, RET 8. The reconstructed
entry points retain their existing new C++ interfaces, not these binary ABIs.

## Original transfer and cleanup

The target was verified as `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Sixteen regions, including the complete readers,
named lookup, handlers, FH3 tables and relevant CRT/Lua instructions, matched
the installed PE and live Ghidra. No Ghidra mutation was made; prior comments
and names are retained in the report for any later integrator annotation.

1. `A69330` stores status in the active `L+70h` error frame at `+44h`, then calls
   `C03570` at `A69346` with saved context `frame+4` and return value 1.
2. `A68B80` installed that frame before invoking its callback. Its `A68BA6`
   call to `C034F4` uses count zero. `__setjmp3` saves FS:0 and the registers,
   places `VC20` at `+20h`, and leaves its optional local-unwind callback null.
3. `C03570` constructs exception code `80000026h`. If saved FS differs from
   current FS, `C035B9` calls `RtlUnwind`. A reader frame registered after the
   protected entry is therefore traversed. With the valid `VC20` signature and
   null callback, its later target-local-unwind branch is skipped; this does
   not suppress the preceding unwind of intervening reader frames.
4. Reader handlers `C95188` and `C96160` load their own FuncInfo and jump to
   `BF6B43`, which calls `C07991` at `BF6B64` with catch depth zero.
5. `C079C0..C079C6` explicitly admits `80000026h`, before the `EHFlags & 1`
   rejection for other non-C++ exceptions. At `C079DA` unwind flags are tested
   with `66h`; nonzero state count and catch depth zero lead to
   `C079F9 -> C069A2(frame, context, FuncInfo, -1)`.
6. `C069A2` reads the current signed state, follows the original 8-byte unwind
   records, writes each predecessor before invoking its action at `C06A19`,
   and ends at -1. This is cleanup, not entry into a reader catch block.

Both FuncInfo records have magic `19930522h`, flags 1 and no try blocks.

| Reader | State | Predecessor | Action | Native local |
| --- | ---: | ---: | --- | --- |
| Base `868BF0`, FuncInfo `DC7040`, map `DC7028` | 0 | -1 | `C95170 -> B67700` | first Lua object, EBP-34h |
| Base | 1 | -1 | `C95178 -> B67700` | first Lua object, EBP-34h |
| Base | 2 | -1 | `C95180 -> B67700` | distance Lua object, EBP-20h |
| Particle `871D00`, FuncInfo `DC8134`, map `DC8158` | 0 | -1 | `C96130 -> B67700` | Particle, EBP-48h |
| Particle | 1 | 0 | `C96138 -> B67700` | UnderWater, EBP-20h |
| Particle | 2 | 0 | `C96140 -> B67700` | key, EBP-34h |
| Particle | 3 | 2 | `C96148 -> B67700` | value, EBP-20h |
| Particle | 4 | 3 | `C96150 -> 41DD20` | table name, EBP-50h |
| Particle | 5 | 0 | `C96158 -> 41DD20` | scalar name, EBP-50h |

Only completed construction arms a state. Base named lookups fail with base
state -1; each base state is armed after its lookup returns. Particle has
state 0 while looking up UnderWater, so that failure cleans Particle only.
The original map contains no rollback of already appended resources.

The transfer returns to `A68B80` before `A696F0` handles its nonzero status.
`A69746 -> A68AF0` places the error at the saved old top; `A69762..A69773`
restore CallInfo/base and call/hook fields. Thus the original FH3 actions run
**before** this Lua frame restoration. A failing nested metamethod may have
changed `L->base`; the existence of the cleanup calls does not prove that their
relative Lua indices are valid on that original path. No original metamethod
error or FH3 execution was claimed or substituted with the linked CRT.

## Source boundary and error-handler contract

The protected callback invokes the existing complete raw named-lookup body and
uses only trivial locals. It includes `lua_checkstack`, key allocation/push, table
access and output publication. It retains the native ignored `checkstack`
return, kind-3 decision, owner/index rereads after pushing the key and actual
tracked-address registration. The callback uses the same interpreter/C frame;
it does not push a Lua C closure, snapshot the table, or create a second state.

The wrapper saves top as a stack-relative byte offset for `luaD_pcall`; Lua
restores internal pointers after reallocation/error. On failure it removes
the error slot and operation temporaries by restoring the entry value count,
then throws the source status object outside the Lua nonlocal-transfer region.
That status type derives from `std::exception` and has a fixed, nonallocating
`what()` message. Existing base/Particle catches therefore see valid pre-operation
objects. Prior component writes are retained, and a Particle acquired frame
keeps its phase, failure site and native state through cleanup. No resource
release, child rollback or retry is added.

`luaD_pcall` receives the **current** `L->errfunc`, preserving the caller's
handler selection. Runtime errors still execute that handler inside Lua,
before the status return; this probe observed one invocation. The wrapper
consumes the resulting Lua error value and exposes its status only. The error
now reaches the source caller as a C++ value instead of transferring to an
outer Lua protected boundary. A future host bridge must explicitly decide
how to translate that value; it must not throw through arbitrary Lua C frames.
Allocator memory errors retain Lua's own handler semantics, not a new policy.

The explicit helper is selected at only five current source sites: base
Autostart/Delay/NoFilterDist and Particle/UnderWater. The source call inventory
shows Particle is the only current caller of the native base reader; neither
reader is registered as a Lua C callback. Future callers must retain this
C++ boundary contract rather than invoking these readers from an unadapted
Lua C callback.

### Existing Lua callback contracts

The following current callbacks deliberately contain trivial automatic owners
and run inside `lua_pcall`. A raw named-lookup error must transfer to that Lua
boundary, where the wrapper reads the original error value and restores top.
Changing the shared raw lookup to a C++ throw would bypass that contract.

| Callback | Source and raw named lookups | Lua protected caller |
| --- | --- | --- |
| `read_ship_depth_protected` | `game_hosts_lua.cpp`: 837, 839, 860, 887, 888, 891 | `read_ship_tuning_lua`, pcall at 941; depth and navigation public wrappers |
| `read_ship_layer_timing_protected` | `game_hosts_lua.cpp`: 967, 968, 985 | `read_ship_layer_timing_input_lua`, pcall at 1020 |
| `read_protected` | `game_ship_avoidance_tuning_lua.cpp`: 29, 52, 57 | `read_ship_avoidance_tuning_lua`, pcall at 84 |
| `read_ramp_protected` and its raw fragment | `ship_ai_path_turn_ramp.cpp`: 31-33, 46, 52, 58 | `read_ship_ai_path_turn_ramp_lua`, pcall at 70 |

These files remain unchanged. Other raw named/string lookup callers also keep
the existing default contract. No consumed Lua error is recreated with
`lua_error`, and no exception is translated across those C callbacks.

The table object, its owner, interpreter and caller stack domain must remain
stable across allocation, metamethods and error handlers. This does not make
foreign callbacks that throw C++ exceptions through Lua safe, or permit a
callback to switch the owner's interpreter. Private Lua headers/API are a
deliberate dependency on the repository's pinned 5.1.1 build, not the original
game's register ABI.

## Focused validation

- Actual current base and Particle sources, compiled with the new native Lua
  implementation, fail in a real Lua `__index` at missing Delay and UnderWater.
  Catches receive status 2. Particle records `871D22/-1` and `871D4C/0`;
  the latter releases its completed Particle object. Root index 64 remains a
  table, tracking contains only that root, and closing it restores all 63
  caller values. Genuine resource/string/VFS fixture domains drain afterward.
- The existing complete 657-byte copied-original Particle probe still matches
  source for scalar/table/empty-table inputs, boolean type semantics, actual
  cache hits/retains, append/growth, resource byte stores and native cleanup.
  Its EH registrations are bypassed: this is ordinary-flow parity only.
- An inherited Lua error handler executes once; the failed output retains its
  preimage, the same CallInfo/base/root survive, and the outer protected call
  returns zero after the source catch consumes status 2. Prior errfunc returns
  to zero. This checks the changed error boundary, not original EH behavior.
- A probe-only Lua allocator refuses the next stack-growth allocation after
  filling the available stack. Failure occurs inside the named lookup's
  `lua_checkstack`, before key push: source status 4 arrives with unchanged
  output bytes, caller stack/sentinel and tracking. This is one deliberate
  allocation failure; the production allocator and Lua library are unchanged.
- All four existing Lua callback wrappers above encounter a genuine failing
  globals `__index`. They return false and the exact original error text,
  preserve their output preimages and restore the caller's stack sentinel.
  The two `game_hosts_lua.cpp` callback/wrapper bodies were compiled from an
  unchanged source slice; the other two use the current built source library.
  The explicit source error also reaches a `std::exception` catch.
- Full strict MSVC Win32 build and three existing CTests are required and
  recorded in the report. The focused probes are ignored local artifacts;
  no permanent test suite or shared CMake source registration was added.

Acquired resource children are not created by these named-lookup failures:
all Particle named lookups precede resource acquisition. Failures after an
acquisition, other allocation failure sites, arbitrary alias/reentrant stack mutation,
original FH3 execution, native ABI compatibility and gameplay remain open.
