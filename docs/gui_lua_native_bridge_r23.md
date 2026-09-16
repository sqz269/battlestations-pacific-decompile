# GuiLua host to native object bridge: retained error boundary

## Outcome

This packet adds evidence only. A production retained bridge from `GuiLua51Host`
and `GuiLuaRef` to the actual native `14h` object is not implemented. Ordinary
stack ownership is feasible; the current Particle reader's Lua-error contract
is insufficient to expose that bridge for dispatch. No native body, application
dispatch, Lua allocator, error ABI, `DoFile`, or shared CMake file was changed.

Source baseline: `2b4614890`. Evidence: `reports/gui_lua_native_bridge_r23.json`.
The ignored probe remains under `local/gui_lua_native_bridge_r23*` in worktree
`agent/orch4-20260916-particle-lua-bridge-r23`.

## Current source ownership

`GuiLua51Host::Impl` owns or borrows the actual `lua_State`, records tracked
registry references separately from globals and borrowed indices, and has a
private `push` boundary. A `GuiLuaRef` integer is not a native stack object.
There is currently no public retained native-root operation.

`NativeLuaStateStorage` is the actual `4C8h` owner: fifty slots with five actual
object addresses per slot. `00B66BD0` initializes counts and high-water metadata;
it does not attach an interpreter. Native tracking uses
`slot = stack_offset_0c + object.index_08`. `00B67690` registers the destination's
address; `00B66DE0` can remove a stack value and shift every later live object's
index. Moving or copying bytes of these objects is therefore invalid.

A candidate source adapter would attach the existing state after `00B66BD0`,
never call `00B66C00` merely to obtain a borrowed owner: the latter also installs
the application's real global `DoFile` callback. It must keep the host and any
externally borrowed interpreter alive, retain the actual root value through the
private registry push boundary, and retain its immutable owner/root addresses
through every acquired child and resource-drain obligation.

## Normal-return experiment

The focused Win32 probe uses the linked Lua 5.1.1 library and the current
`gui_lua_runtime.cpp` and `native_lua_objects.cpp`. It evaluates an actual root
table with `GuiLua51Host`, makes/releases host references, and accesses that same
root in the same interpreter. The probe resolves the root through globals;
the unavailable private registry push boundary is **not** claimed as tested.

With 63 caller values already on the stack, the retained root is at index 64.
An immovable actual owner uses offset -64, making the root slot 0 and its child
slot 1. Native assignment records a second actual root address; native lookup
reads 23. Native release of child, copy and root restores exactly 63 caller
values, including the last sentinel. This establishes a normal-return storage
experiment, not a callable production adapter or arbitrary alias coverage.

## Lua-error experiment

The second probe calls `lua_cpcall` with an externally retained, trivial frame.
Inside the callback, actual native lookups retain a root and one child, then a
missing-key `__index` produces a Lua runtime error. No nontrivial automatic
destructor is crossed in that callback, so this fixture does not depend on
ordinary C++ RAII across `longjmp`.

Observed result:

```text
status=2 catch_entered=0 after_error=0
callback_top=3 returned_top=1
root_index=2 child_index=3 root_count=1 child_count=1 root_type=-1
```

The surrounding C++ catch was bypassed. The retained native objects remain
bound and their owner still records both addresses, but `lua_type` reports
`LUA_TNONE` at the retained root index. The protected call has discarded that
callback's stack frame. Native destruction using those stale indices would be
invalid; the fixture instead ends the entire borrowed state after recording
the result. It does not repair or replay the native cleanup.

This is directly relevant to `read_native_particle_component_00871d00`: its
failure identity and cleanup are in `catch (...)`. Its base reader also uses
automatic tracked objects. A Lua nonlocal transfer cannot be assumed to run
that catch. A retained owner may consequently contain invalid stack indices
and addresses of locals whose invocation has ended. This Particle consequence
is source analysis; the probe does not invoke Particle/resource acquisition.

Wrapping an already-created bridge in a later `lua_pcall` is also insufficient:
Lua stack indices are relative to the current C frame. A new callback changes
that base before the native read, and returning from it discards its frame even
on normal return unless all native stack obligations are already finished.
Root registry retention alone cannot retain those stack positions.

## Original executable error path

Live queries verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, through the repository Ghidra client's target
verification. Eight configured seed regions matched the installed executable.
No Ghidra mutation was made. Decompilation was checked against instructions:

| Address | Bounded observation |
| --- | --- |
| `00A69330` `luaD_throw` | ECX is state, EDX status. `00A6933A` stores status at current error-frame +44h; `00A69346` calls `00C03570` with frame+4 and value 1. The no-frame path invokes the configured panic if present, then exits. |
| `00A68B80` `luaD_rawrunprotected` | Builds/restores the state's +70h error-frame chain. `00A68BA6` calls `__setjmp3` at `00C034F4` with count 0. Callback receives state in ECX and context in EDX. Both returns restore the prior chain and return stored status. |
| `00C034F4` `__setjmp3` | Saves registers, stack/return address, FS exception chain, `VC20` signature and unwind fields. Count 0 does not install an explicit unwind callback. |
| `00C03570` longjmp helper | Uses exception `80000026h`. `00C035B9` calls imported `RtlUnwind` when the target exception frame differs from FS:0. Then it may call a saved unwind callback or `__local_unwind2`, restores registers/ESP and jumps to the saved return address. |
| `00A680E0` `lua_pcall` | Computes the saved function stack position with 10h value stride, calls `00A696F0` at `00A68125`, and passes the call callback in EDX. |
| `00A696F0` unnamed protected-call helper | On error calls close-upvalues and `seterrorobj`, restores the prior CallInfo/base and saved call/hook fields. The `luaD_pcall` interpretation is provisional; no rename was made. |
| `00A68AF0` `seterrorobj` | Copies the error into the saved stack location for runtime/syntax errors; `00A68B62..65` sets top to that location +10h. |

The original uses setjmp/longjmp at the Lua layer, rather than a C++ throw at
that layer. **Its CRT performs SEH unwinding.** This packet does not establish
which original Particle FH3 actions run on that transfer, nor equate them to
the linked source build. A statement that the original game necessarily leaks
the probe's references would be unsupported.

## Contract required before adding an API

1. Establish an error boundary that returns control with a defined Lua stack,
   records the native failure site/state, and preserves or finishes each child
   obligation. Inspect original Particle/base-reader FH3 actions on this exact
   Lua nonlocal-transfer path before translating its cleanup contract.
2. Keep owner/root addresses immutable and use the actual host interpreter and
   private reference push. Do not substitute a table snapshot, a numeric-handle
   cast, an empty callback, another interpreter, or a Lua allocator/error rewrite.
3. Define root kinds explicitly: tracked registry value becomes a tracked
   kind-2 stack object; globals retain kind 1/pseudo-index; borrowed indices
   retain their alias semantics and valid C-frame lifetime; unbound remains
   distinct from bound nil and cannot silently become an indexable object.
4. Scope one retained stack domain in one C frame. Capture its base before
   pushing; offset `-(entry_top+1)` permits the first owned position to use slot
   0. Prove at most 50 tracked positions and five references per position,
   including every reader and child; forbid uncontrolled stack or frame changes.
5. Define explicit successful close after native child references and resources
   are resolved. Check active tracking and expected stack shape before native
   root release; preserve host/interpreter lifetime throughout. A destructor is
   not evidence that close runs across Lua longjmp.
6. Cover failure during construction too: Lua 5.1.1 `lua_checkstack` can grow the
   stack through allocation and enter Lua's error path. Its zero return handles
   a stack-size limit and is not a general protected out-of-memory boundary.

A next bounded packet should recover the original FH3 cleanup reached by
`00A69330 -> 00C03570` for the relevant reader states and define a source Lua
operation boundary whose status can reach the retained invocation. Only then
can a bridge's failed-state retention and close behavior be reviewed. This
packet does not propose a broad Lua rewrite.

## Validation limits

The ignored probe compiled as MSVC Win32 with `/MD /W4 /WX /fp:strict` and an
embedded manifest; both experiments passed. Eight Ghidra/disk seed checks
passed. No production C++ changed, so a full build and three CTests were not
rerun for this evidence-only change. Original FH3/CRT equivalence, raw Lua ABI,
Particle dispatch/resource failures, allocator exhaustion and gameplay remain
unvalidated.
