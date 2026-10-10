# ScriptOrders command owner: primary review

The mission frame has a concrete existing Lua owner that could supply the
ScriptOrders command service. The present production think walk does not call
the argument-discarding command method: its preceding predicate always returns
false. This Source-only review establishes ownership and lifetime, without
activating a predicate, issuing a command or claiming a dropped runtime command.

Worker `2929fe387ab428d488a38baa847185a1f8246a33` retains assigned Source
`ea3c9fc17` and capture HEAD `4c0e429c3` separately; their Source trees agree.
The primary copied its complete evidence, document and report. The unchanged
artifact-only verifier passed from the copy, and a separate Git check verified
4,215 complete blobs. The primary independently compared every one of 4,206
complete Source files to current working bytes: 4,201 exact, five CRLF/LF-only.
Nine further complete inputs match after LF normalization. It independently
reproduced all 14 complete-corpus searches and 48 exact preimage-derived spans.
The verified include closure has 13 roots, 300 project files and four configured
Lua API headers; compiler/system SDK state remains outside the receipt.

## Owner and call location

The outer mission owns `GameMissionLuaHost`; its frame borrows that exact
object. The frame constructs one ScriptOrders host and attaches it to that
owner. Frame release detaches and destroys ScriptOrders before releasing its
units, and outer teardown/replacement releases the frame before Lua. Those
relationships provide a bounded borrowed `MissionLuaHostServices&` contract.

The production route runs timers and fixed-step ScriptOrders think passes into
the generic entity walk. Only the expired-countdown branch with a true virtual
predicate reaches `collectgarbage()` with mode 2. The current predicate is
false. The parallel `EntityThinkBinding` in `game_hosts_ready.cpp` is a distinct
adapter with its own lists, countdown and false gate; it has no Lua receiver.
The unbound settings compatibility callback is a third, separate seam.

The retained Native call record at `00929588` selects the mission machine from
game `+1A08h` and host `+4h`, with null result/error outputs. The primary checked
that selected record against the whole prior report. It is reused evidence,
not a fresh Native body, image or Ghidra query. The actual predicate body is
outside this Source-only packet; a later finite profile review must retain its
own evidence and qualify the earlier open question.

The cached ScriptOrders `machine_state_` is a trampoline argument, without a
proven identity check against the owner's root state. It is not the proposed
command receiver. Existing `run_lua_chunk` on the borrowed owner can preserve
the synchronous call, source/name, null sinks and mode at the guarded location.
A later active route still requires successful initialization, owner lifetime
and interpreter reentrancy review. Constructor order and a nonchecked startup
return are not an unconditional machine-readiness proof.

The smallest proposed service binding changes the ScriptOrders constructor,
its command method and its single frame construction site. This review grants
no C++ or activation approval and does not create another interpreter, queue,
broadcast or archive callback. There was no build, test or runtime execution.
The current void-call summary would also not prove successful Lua execution.

The complete primary receipt is
`reports/cc12_script_orders_lua_command_owner_binding_primary_review.json`;
whole retained evidence and replay outputs are under `local/cc12_lua_owner_primary/`.
