# Actual render queue execution: next closure packets

Read-only discovery, 2026-09-10. Full actual `00B1EBE0` queue execution and `00B1D950` command execution remain incomplete. The smallest useful ready packets are **two actual diagnostic-label functions** and **four raw renderer/batch state leaves**. Neither needs a fabricated full renderer or diagnostic-service class. The command still needs concrete target, camera, system, material and scheduler execution; existing semantic interfaces do not implement those missing native paths.

The worker source base is `4f0bc070d4d2835668a342dce437937e339b313d`. Main's committed additions were reviewed at `e36e0faf24d97b338754291b1c5675a5435973f4`: actual string cleanup `3ef5439`, raw batch keys `76040d0`, and pointer-slot sorting `e0ad746`. These dependency additions are explicitly pinned in the [audit](../reports/native_render_queue_execution_next.json), rather than attributed to the worker base. This updates the execution frontier from [NATIVE_RENDER_COMMAND_EXECUTION_NEXT.md](NATIVE_RENDER_COMMAND_EXECUTION_NEXT.md); its recommended job lifetime and the later queue access/row packets are now implemented.

## Evidence and scope

Every live query used `python tools/bsp.py ghidra ...`, whose client verifies project `bsp`, `C:/Users/sqz269/bsp.gpr`, and `/battlestationspacific.exe` before reading. Complete queue/command/diagnostic/leaf listings, command and reset EH data, relevant profiles and string dependencies were checked. **31 live byte ranges, 1,689 bytes, all match the installed PE**. The executable SHA-256 is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. The audit records range hashes and complete prior names/comments; no Ghidra names, comments, prototypes or bytes changed.

This is a bounded closure review, not a fresh transitive reconstruction of the whole renderer and scheduler. Deeper target/camera/system/material/scheduler evidence remains in the pinned prior discovery; current source contracts were rechecked for whether those actual paths now exist. Native tables are observed address profiles, not callable C++ tables. All proposed names remain descriptive hypotheses. No build, fixture execution, ABI-compatible replacement or game validation is claimed for this discovery.

## Queue `00B1EBE0`

Original ABI: `ECX` points to the actual 34h queue, no stack parameters, `RET`. Complete bytes `[B1EBE0,B1EC9B)`. Fields used: command pointer array `+14`, signed count `+18`, control `+20`, current context `+30`. The concrete command table `D5E5E0+0` selects `B1D950`.

1. Initialize index zero and compare the live signed count. A zero/negative count skips all per-command work; it does not release an existing `queue+30` context.
2. Reload the indexed command, capture its `+28` context and the queue's old `+30`. When different, publish incoming `queue+30` **before** incrementing incoming `+04`, then decrement captured old `+04`. Null incoming skips its increment; null old skips its decrement. A zero old count calls that captured owner's current virtual `+0`. Same identity skips assignment and both counts.
3. Reload the command array/cell and call that command's current virtual `+0`. It need not be the command whose context was captured before a terminal callback.
4. After execution, reload the queue's **current** `+30`. If nonnull, decrement that identity, call its current terminal slot on zero, then clear `queue+30` **after** the callback. The clear therefore overwrites a replacement published by that terminal callback. An already-null field skips this release/clear block.
5. Reload control. If zero, reload the current command array/cell; a nonnull cell runs ordinary `B1DDD0` destruction followed by `BF65AC` raw free. This is not command virtual deletion and does not clear the cell before the callback. A null cell skips destruction/free.
6. Increment index and reload the signed count. After the loop, reload control again; only zero invokes `B1CC80(queue+14,0)`, retaining the pointer-array capacity. Nonzero retains the command list.

There is **no queue-level EH prolog or unwind action**. An exception from command execution does not perform the later context release, command destruction or final resize. Do not give this body the host queue helper's unconditional `noexcept` or add a cleanup guard. Actual context `+04`, canonical zero-terminal resolution, `D5E5C4 -> BD30E0/B1D570`, ordinary command destruction and pointer-array resize already exist. The missing concrete `D5E5E0+0` command execution still prevents a full actual queue claim. A generic successful command callback would not close it.

## Command `00B1D950`

Original ABI: `ECX` points to the actual 44h command, no stack parameters, `RET`; no native error-return contract. Complete bytes `[B1D950,B1DAC1)`, including unreachable alignment `[B1DA6D,B1DA70)`. The context is 18h (`+08` camera, `+14` target); batches are 18h (`+08` mode, `+0C` raw list, `+10` signed count). Preserve this ordering:

1. Read current `F8D394`; readiness `B20240` false returns before any command cleanup guard.
2. Capture command context, current renderer and target-binding virtual `+98`. Capture camera from context `+08` before binding context `+14` target. The camera remains captured across the entire command; there is no temporary retain.
3. Execute `B71360(captured camera)`, then reload command scene `+04` into `ECX` and pass captured camera in `EDX` to `B46A70`.
4. Reload optional `F8D39C`; when nonnull, `B13030` copies command's actual string header at `+14`. Failures so far have no command diagnostic-reset cleanup.
5. Capture current renderer/table `+114`; arm EH state zero at `B1D9D6` immediately before calling it with command `+1C`. Concrete profile target `B20210` is a real `RET4` no-op.
6. Reload current renderer/table `+2C` and test `AL`. When active, synchronization byte `108D6DC==0` and **both** signed `B51B20` counts `>50`, in short-circuit order, select preparation jobs.
7. Each job iteration first writes the current batch's `+08` mode. Call scheduler getter `4C1130`, reload that command batch cell, capture scheduler subobject `+04`, then call job getter `B0FFB0`. After that getter, read the captured scheduler subobject's current virtual `+04` and enqueue the returned primary job plus captured batch. After both enqueues, call `4C1130` again and that returned scheduler's subobject virtual `+08` with mode 1.
8. Serial preparation reloads each current batch/table `+0C` and calls it with its index. Unlike the job path, it makes no preceding mode-field write.
9. Execute both current batches in index order through their current virtual `+08`, passing index and the original captured camera. Concrete `B55550` ignores both stack arguments; do not generalize that to other tables.
10. Disarm state at `B1DAA2`, then call `B13510`, including the inactive-frame path. Handler `CBCA68`, EH info `DF4EF0`, map `DF4EE8` and action `CBCA60 -> B13510` perform the same reset only for armed exceptions. A reset exception after normal disarming does not trigger a second reset.

## Ready packet: actual diagnostic label pair

Suggested packet `native_render_diagnostic_labels`: full functions `B13030` (57 bytes) and `B13510` (165 bytes), dedicated `include/bsp/native_render_diagnostic_labels.hpp`, `src/native_render_diagnostic_labels.cpp`, evidence/annotation records. Reuse `NativeStringStorage`, actual `resize_native_string_header_0041dd40`, existing constructor body `NativeString::assign_0041e870`, and actual cleanup `destroy_native_string_header_0041dd20` from **`3ef5439`**. Bind the actual service pointer/global slot and its actual eight-byte header at `+684`; this is a borrowed touched region, not a claim to know the service's whole size, constructor, statistics or system-time ownership.

`B13030`: `ECX=actual service`, stack argument actual source header, `RET4`. Capture source and destination `service+684`; exact header identity returns immediately. Otherwise read source length, call actual resize on destination with preserve=true, **reread source length** for the nonzero gate, then read current destination length, source data and destination data. Copy **destination's current length** bytes, without a terminator copy. Do not replace these reads with a saved source length or `std::string::assign`. There is no local EH guard.

`B13510`: no inputs, `RET` (`ECX` unused). Initially test the **current global** `F8D39C`; null returns. Construct a fresh actual eight-byte temporary from fixed `CE9A38` bytes `58 00` (`X`). Only after construction, reload `F8D39C`, capture temporary data and length, form destination `service+684`, and compare destination/temporary header identity. Arm state zero at `B1355C` before the copy branch. Unless aliased, resize destination using the **captured** temporary length; when that captured length is nonzero, copy current destination length bytes from the **captured** temporary data. Unlike `B13030`, the source length/data are not reloaded here.

At normal cleanup, test captured temporary data, disarm at `B13586`, then free that captured buffer with captured length+1 (DWORD wrap) through the existing sized storage boundary. Leave temporary header words untouched. The EH route is different: handler `CBC278`, info `DF4360`, map `DF4358` (state `0 -> -1`, action `CBC270`), and `CBC270: LEA ECX,[EBP-14]; JMP 41DD20` clean the temporary header's **current fields**. The constructor runs before this guard; construction failure has no reset-temporary unwind action. A global replacement during allocation must affect the subsequent destination. An initially nonnull global becoming null has no second native null guard. Do not add silent success, a fabricated service, an empty reset string or catch-and-retry cleanup.

This packet can implement both whole functions using existing allocation/string boundaries. Reuse existing string/build checks; add only a focused test if implementation review reveals a specific capture-versus-reread or unwind risk. It does not implement `B16F80` statistics or the full diagnostic service.

## Ready packet: four actual state leaves

Suggested independent packet `native_render_execution_state`: dedicated header/source plus evidence records. Use raw borrowed storage for the exact touched offsets and existing `NativeRenderBatchStorage`; no full renderer owner or resolver interface is required just to read these fields.

| Entry | Original ABI and exact behavior | Boundary |
|---|---|---|
| `B20240`, 27 bytes | `ECX=renderer`, `RET`; if DWORD `+1D90!=0`, EAX=0 and skip lost-byte read; otherwise test byte `+1D8A`, return EAX exactly 0/1 | Actual live renderer reads, unlike the copied `D3D9DrawState` helper |
| `B1FE20`, 11 bytes | `ECX=renderer`, `RET`; compare DWORD `+1998` with zero, `SETNZ AL` | Upper EAX is unchanged; a new bool API must not claim whole-register ABI parity |
| `B20210`, 3 bytes | `RET4`, incoming renderer/metadata ignored; no memory or EAX write | This observed native no-op is valid implementation, not an unresolved stub |
| `B51B20`, 4 bytes | `ECX=batch`, `RET`; load actual DWORD `+10` into EAX | Caller uses signed count; return those exact bits, with no clamping or container conversion |

The first three names already describe these bodies. Suggested descriptive name for current `TRIV_body_00b51b20` is `BSP_RenderBatch_GetEntryCount`. Keep original names/comments in annotation evidence before any later mutation. Normal C++ entry points remain new interfaces unless their complete original calling convention is deliberately implemented and verified.

## Remaining concrete dependency frontier

| Dependency | Current implementation evidence | Still required for actual execution |
|---|---|---|
| Actual command/context/batch/group lifetime | Actual 44h/18h storage, shared actual counts, zero-terminal dispatch, ordinary command destructor | Invoke the same owners; this supplies no missing command execute body |
| Queue access and rows | Actual `B1CB30`, command append/base cleanup; row reserve/resize/destroy at worker base `4f0bc07` | `4C11F0`/`B1F280` actual singleton construction and control/renderer stop remain; see queue-owner and renderer-stop discoveries |
| Batch prepare `B51DF0` | Actual key-loop fragment plus `BF7456`/`B51AB0` at `76040d0`; actual `B1DCE0` and ten comparator/helper functions at `e0ad746`; actual config accessor exists | Real queue getter/config identity and whole orchestration. Full `B51DF0` remains open; passing a vector or successful getter callback does not complete it |
| Job singleton/execute | Actual 8-byte `B0FFB0` lifecycle exists; primary `D5E160+0 -> B1BF70` | `B1BF70` reads stack batch, current table and `batch+08`, substitutes mode as the stack argument, then tailcalls virtual `+0C`; it depends on full preparation |
| Batch execute `B55550` | Full 70-byte body and host fragment | Reload actual renderer/active gate, clear actual `108FBF4`, loop signed live count/list, then actual `entry+04 -> section+20 -> material+7C -> effect.virtual+14`. Reload list/count after each effect call; no native validation returns |
| Effect `D61A00+14 -> B45360 -> B44750/B43410` | Typed `MaterialEntryDispatcher`; full typed `B42350` constant builder | Actual effect/program/override owners and native callbacks, mode/model/geometry/texture/plane/statistics operations; `MaterialEntryOperations` is explicitly a required semantic interface |
| Camera `B71360` | Actual camera owner/frame view and typed camera code | Native reloads `F8D394` separately for prepare `+A0`, viewport `+A4`, Clear `+08`; current helper captures one cache. Preserve `camera+17C` gate, viewport `+180` reload after prepare, and x87 clear-depth load/store |
| System `B46A70` | Full typed body, actual camera/time/parameter projections | Same actual scene outer `+1C`, service/clock domains, 77-register preimage and actual renderer shader blocks/reloads. Typed composition does not close native scene-resource or renderer ABI |
| Renderer/targets | Concrete COM wrappers; actual parameter owner only `+1A14..+1A27` | Actual renderer identity/table/device/counters/default surfaces; target group's `+04` refs, color `+08..+14`, depth `+18`, sRGB `+3C`, renderer target slot and application policy/replacement ordering |
| Scheduler `4C1130`, enqueue `BE3020`, wait `BE3150` | Prior native discovery and typed frame-order interfaces | Actual 138A8h owner/base `+04`, work-count atomics, events/threads, worker registration/draining/wait visibility. Lifecycle and scheduler execution are separate from the preparation-job singleton |

Observed profiles were rechecked: renderer `D5F0A8` has `+08=B21430`, `+2C=B1FE20`, `+98=B24E70`, `+A0=B285A0`, `+A4=B26770`, `+114=B20210`; pooled batch `D5E5AC` has `B55680/B1C630/B55550/B51DF0`; construction-base batch `D62064` differs at deletion and has a purecall execute slot. Scheduler `CE7550` primary is `4BFB30`, and `CE7554` base begins `4BFAC0/BE3020/BE3150`. These values constrain concrete dispatch and do not authorize fallback profiles.

Only this document and its audit report are changed. The discovery adds no C++, CMake, tests, shared ledgers or packet-state updates. The integrator owns subsequent implementation assignments and metadata integration.
