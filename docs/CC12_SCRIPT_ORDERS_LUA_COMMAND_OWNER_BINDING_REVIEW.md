# ScriptOrders Lua command owner binding review

The production mission frame reaches the ScriptOrders think walk, but it does
**not currently reach** `GameScriptOrdersHost::lua_run_string_006b8ad0` through
that walk: the preceding GC predicate always returns false. The command method
also discards its arguments. These are two separate unfinished bindings; this
review does not enable the predicate or claim an observed dropped command.

There is a concrete Source owner for a future ScriptOrders command binding:
the existing `GameMissionLuaHost` borrowed by the same mission frame. Passing
that owner as a borrowed `MissionLuaHostServices&` at the single ScriptOrders
construction site is sufficient to avoid inventing another interpreter or
using an unrelated Lua state. No C++ GO is requested or assumed.

## Scope and evidence

Assigned accepted Source is `ea3c9fc17f79f93b8c842a08e41ef531b6eea0fc`.
The previous worktree HEAD was `9d371a997`. Initial sync correctly refused
while that packet awaited Root review. After Root accepted it, the worktree
fast-forwarded to `4c0e429c3493d48af726c528a7e9854ef9cafad8`; its complete
`src/` and `include/` tree equals the assigned Source epoch. The capture records
both commits rather than relabeling earlier Source or Native evidence.

Both complete Git and working bytes of all 4,206 tracked Source/header files
are retained. Nine additional complete Git/working inputs include build files,
tools, `MISSION_LUA_HOST`, `ENTITY_THINK_DISPATCH`, and their whole reports.
The 13 reviewed Source roots close through 300 project files and four Lua API
headers from the configured, hash-checked archive. System/SDK headers remain
explicit compile dependencies; this is not a compiler or SDK-state receipt.
The Lua archive is the configured input before the documented build transform.

No fresh Ghidra function, body, bytes or prototype query was made. No installed
Native image was opened. The finite Native call record is the retained prior
`entity_think_dispatch` report's `host_steps[address=00929588]`, checked against
its complete report. Prior Native evidence remains prior evidence.

## Actual caller and gates

The complete Source searches establish this production route:

1. `GameMissionFrameHost` constructs one `GameScriptOrdersHost` during scene
   loading, then attaches it to `host.lua`.
2. After `run_mission_frame`, a simulated result and nonnull orders host cause
   `run_script_timers(raw_delta)`.
3. Timers return early without cached callback state or script entities.
   The enabled fixed-step branch calls `run_script_think_pass` for each due
   `0.05f` step.
4. That pass calls `run_entity_think_list_00929460(..., *this)`.
5. After the think walk, an expired countdown is refilled. Only a true
   `gc_gate_predicate_0109cefc_vtable0c()` permits the string command. The
   ScriptOrders implementation is an unconditional `return false`.

The only retained production call expression for the string method is inside
that guarded generic walk. Its command is `collectgarbage()` and its mode is
`2`. Splicing and clearing pending registrations follow the command position.
The current caller summary's `garbage_collected` flag is set after invoking the
void method; it would not independently prove successful Lua execution.

The ordinary fan-out also calls the generic think walk through a **different**
`EntityThinkBinding` in `game_hosts_ready.cpp`. That binding carries only the
log, returns false from its GC predicate, and has a separate unimplemented
string method. It is not a call to the ScriptOrders no-op. It has its own
countdown/lists and no concrete Lua receiver bound to that local adapter.
This review proposes no change to that parallel route or either predicate.

The archive callback `SettingsArchiveReadHost::publish_xbox_compatibility`
remains a separate seam. No compatibility command is involved in this think
walk; the earlier unbound archive route is not evidence for this route's
production reachability or owner.

## Receiver and lifetime

The retained prior Native record identifies the command receiver at
`00929588` as the mission Lua machine from game `+1A08h`, then host `+4h`.
`MISSION_LUA_HOST` distinguishes that machine from its own `+4h` Lua state.
It establishes `006B8AD0(text, null results, null error, 2)`, `RET 10h`,
forwarding `(text, strlen(text), text, ...)` to `006B89F0`.

Source gives a concrete counterpart for the ScriptOrders route:

- The outer `GameMissionHost::Impl` owns `unique_ptr<GameMissionLuaHost> lua`
  and constructs `GameMissionFrameHost` with `*lua`.
- The frame stores that reference as `Impl::lua`. It creates ScriptOrders
  alongside its units and calls `host.lua.attach_script_orders` with that
  exact orders object.
- `GameMissionLuaHost` is a `MissionLuaHostServices` implementation. It owns
  `state_`, creates it in `create_state`, and its loadbuffer/pcall methods use
  that member. Thus a services reference supplied from `host.lua` reaches the
  existing frame's owner, not an input, renderer or global-bootstrap owner.
- Frame release detaches orders from Lua, then destroys ScriptOrders before
  units. The outer owner resets the old frame before replacing Lua. Member
  declaration order also destroys the frame before Lua on normal teardown.

The current `machine_state_` in ScriptOrders is weaker evidence: every handled
trampoline dispatch assigns it the incoming `lua_State*`; the ordinary
`state_` is later restored, while that cached pointer remains. The registered
closure carries the `GameMissionLuaHost*` as an upvalue, but no pointer-identity
check equates the callback argument with the owner's root `state_`. This packet
does not assume that equality or use the cached pointer as the proposed
command receiver.

`start_machine_00884be0` marks readiness only after a nonnull state exists.
The outer Source invokes it after creating the frame; the return value at
that call site is not checked. Timer guards and the false GC predicate are
observable Source conditions, not an unconditional machine-ready proof.
The borrowed owner must remain initialized and alive whenever a future active
command is issued. Do not introduce an early start or a new silent null-state
policy to substitute for that requirement. Original Native startup order,
partial-initialization behavior and the actual GC predicate remain unproved
by this Source-only review.

## Smallest bounded Source proposal

For the ScriptOrders route alone, add a borrowed
`bsp::MissionLuaHostServices&` constructor argument/member and supply
`host.lua` at its single production construction site. The existing
`mission_lua_host.hpp` contract then permits this body shape:

```cpp
static_cast<void>(bsp::run_lua_chunk(
    mission_machine_, chunk, static_cast<int>(std::strlen(chunk)),
    chunk, false, false, mode));
```

This is a proposal, not committed C++. Its three production files would be
the ScriptOrders header, implementation and mission-frame construction site.
The declaration can forward-declare `bsp::MissionLuaHostServices`; the current
implementation already includes its defining header and `<cstring>`.

The sole admitted caller supplies a nonnull 16-byte constant string and mode
`2`. Forwarding the incoming mode preserves the interface. Both false flags
preserve the null error/result sinks; using the source as chunk name preserves
the recovered string wrapper. `run_lua_chunk` already preserves its admitted
stack/error handling, and the caller ignores the returned status. Existing
host diagnostics may still record Lua errors even with no requested error
output; null sinks do not mean no interpreter effects or diagnostics.

The forwarding call stays synchronous at the existing guarded position,
between countdown refill and pending-list splice. It adds no owner, broadcast,
queue, replay or clock change. This preserves the current Source call location,
not a newly proven equivalence between this supplemental ScriptOrders pass
and the original full fixed-step fan-out.

Keep both GC predicates false. Recovering or enabling `0109CEFC`'s virtual
predicate is separate work; the prior report explicitly leaves its body open.
Enabling either route also requires reviewing machine readiness, interpreter
reentrancy/finalizer effects and the two Source schedules. This evidence-only
packet does not activate either command path or repair the archive callback.

## Verification

The accompanying JSON pins the evidence under
`local/cc12_script_orders_lua_command_owner_binding_review`.
Artifact-only replay passes for 4,206 Source files, nine further Git inputs,
14 complete-corpus searches, 48 exact excerpts, 57 Source anchors and the
include closure. It checks the reused Native call record without opening an
image or contacting Ghidra. The same replay passes from a separately copied
evidence directory. Its audit hook permits only retained packet reads and
rejects writes, subprocesses, network and dynamic-library loading.

Separate Git mode verifies 4,215 blobs and, when requested, 4,215 working files.
No C++ build, tests, compiler, game or interpreter execution was performed.
No shared Source, SDK, operating-system, game installation or GPR state changed.
