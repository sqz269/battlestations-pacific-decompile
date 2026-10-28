#pragma once

// cc13-rocketstop (census S15, src/game_hosts_units.cpp run_opportunistic_rocket_009fd0e0).
// The opportunistic rocket block is task+38Ch, one per task object, not one per plane:
//  - both bot task base constructors build it: 0099C6F0 calls 009FAAD0 (task+314h, the
//    gun block) at 0099C790 and 009FCEF0 (task+38Ch) at 0099C79C; 0099C8B0 does the
//    same at 0099C950 / 0099C95C;
//  - 009998A0 ticks the CURRENT task's own block: 00999983 LEA ECX,[ESI+38Ch], 0099998C
//    CALL 009FD0E0;
//  - the dogfight, strafe and rocket Stops (slot +58h: 009AA9E0, 009CD0D0, 007B7BA0,
//    byte twins) exit the state (+310h vtable +08h), release the target node (+3C4h set:
//    006952A0(ECX = +3C4h, EDX = task+3B0h), +3C4h = 0), store [00F87574..7Ch] (zero)
//    to +3A4h..+3ACh and clear +3C8h. Their only caller, 00999EE0, deletes the task
//    right after (00999F0B, vtable+0 with flag 1), so the cleared block is never read.
// What a plane observes after a task change is therefore a FRESH block: the new task's
// constructor values (009FCEF0: class -1, salvo 0, both timers -1.0, target 0, rocket 0).
//
// The host keeps one block per plane (unit.orx). It clears the target at the next
// 009FD0E0 tick once the task that set it (orx_owner) is no longer installed; the
// class, salvo count, timers and rocket carry over from the previous task.

namespace bsp {

// True: the host's task construction point, bot_task_ctor_seeds_0099c6f0 (the
// 0099C790 gun-block step every task install runs), also runs the Stop clear and
// 009FCEF0 on unit.orx and drops orx_owner, as a new task object would carry a new
// block. The tick-time clear stays (it covers a retire with no new task).
// False: the target is cleared at the tick only; the rest of the block carries over.
// Movers when ON: rocket salvo timing on planes that change task (the RocketCheckTime
// timer restarts at -1.0, a salvo in progress is dropped, the chosen class resets).
inline constexpr bool kRocketTargetStopHandlersCc13Bound = false;

}  // namespace bsp
