# Startup frontier BB: game-resource factory owner

Addresses: 008f81f0, 007175d0, 007150b0, 00716520, 00716530, 00716560, 007188a0, 0071b870, 0071b810, 004c1400, 00b80720, 00b88260, 00c84c10, 00c84c18.

At source commit `ca2ca633`, the earliest **explicit no-op on the normal WinMain startup path** is `GameStartupHost::publish_game_resource_factory` (`src/game_hosts.cpp:1201`). `run_win_main` calls this after setting thread affinity and before application construction. It only logs `unimplemented`; it neither obtains the native factory nor stores its alias. This is a static source finding, not a claim that every earlier instruction has native fidelity or that an observed run stopped here.

Live assembly confirms `008f840b -> 007175d0`, then `008f8414` stores the result in `00f8d31c`, then `008f8419 -> 00737970` and `008f8429 -> 0073d410`. The getter owns another publication, `00e19b90`. The two cells must remain distinct. The native destructors below clear `00e19b90` but never clear the WinMain alias `00f8d31c`.

The bounded next implementation is the eight-byte factory owner and its deletion entries. The resource objects produced by its virtual Create entry, the resource-manager cache/parser, and rendering stay separate dependencies. There is no reason to invent a resource parser or callable C++ vtable merely to publish this owner.

## What the executable currently calls

| Source path at the captured commit | Concrete behavior and remaining boundary |
| --- | --- |
| `game_main.cpp:219-230`, `winmain_startup.cpp:221-228` | Calls the reconstructed WinMain sequence through `GameStartupHost`; its first explicit normal-path no-op is factory publication. |
| `game_hosts.cpp:1206-1212` | Application construction initializes the projected `ApplicationFrameState`; native subsystem-pointer ownership remains omitted. Its `implemented` log is not proof of complete `00737970` storage fidelity. |
| `game_hosts.cpp:1242-1271`, `game_hosts_vfs.cpp` | Runs VFS phase 2 and the factory tail. Real mount/read services exist; MPAK Create is still described as absent in this captured source and is a separately assigned frontier. |
| `game_hosts_vfs.cpp:281-291` | `004c1400` is represented by a `GameResourceManager` parser-map owner. The ledger calls this reconstructed, but the body does not implement the original raw 28h singleton and cache owner. |
| `game_hosts.cpp:1432-1451` | Online-manager startup and renderer resources remain explicit no-ops. Renderer work belongs to the renderer orchestrator and is excluded here. |
| `game_hosts.cpp:1487-1503`, `game_hosts_fonts.cpp:89-124` | Font startup calls descriptor loading, real VFS resource opens, D3DX-backed resource creation, and fingerprint parsing. It is substantial implemented source; raw singleton lifetime and later renderer settings are separate fidelity boundaries. |
| `game_hosts_frontend.cpp:468-566` | GUI startup calls those font services and loads authored pages/children. GUI-manager creation is projected; texture loading uses the sprite bridge and still explicitly logs the original renderer call as unimplemented. Model/scene-node calls remain distinct unresolved bindings. |
| `game_hosts.cpp:1596-1610` | Calls the reconstructed platform loop when initialization supplied callbacks. This static path proves a call exists, not native simulation or gameplay parity. |
| `game_hosts.cpp:1624-1628` | Application shutdown logs `00737f30` as implemented but its full singleton teardown `00737f80` remains explicitly unbound. The separate actual raw lifetime drain in `GameSingletonHost` already exists. |

No game, window, device, or runtime-log state was changed or newly observed for this discovery. Milestone labels and green host counters are not used as runtime evidence.

## Packet 1: native game-resource factory owner

Proposed packet: `orch2_native_game_resource_factory_bb`. Root accepted this bounded scope after reviewing the getter listing. All five implementation addresses were unleased at the discovery check; root is separately preparing the missing Ghidra definitions and one flow repair. Recheck leases before claiming.

Own only new `include/bsp/native_game_resource_factory.hpp`, `src/native_game_resource_factory.cpp`, `docs/NATIVE_GAME_RESOURCE_FACTORY.md`, `reports/native_game_resource_factory.json`, these five address records, and the one appended source registration in `cmake/startup.cmake`. Do not claim the existing shared deletion dispatcher or process hosts in this packet.

| Entry | Inclusive native span | Bytes | Original ABI / coverage of this discovery |
| --- | --- | ---: | --- |
| `007175d0` | `007175d0-0071769d` | 206 | No consumed input; EAX factory pointer; RET. Complete normal control flow and sole FH3 unwind action read; source absent. Construction is inlined, not a sixth function. |
| `00716530` | `00716530-00716551` | 34 | ECX primary owner; RET; no semantic result. Complete raw direct-destructor body read. |
| `00716560` | `00716560-00716599` | 58 | ECX primary owner; stacked flags; EAX captured primary; RET4. Complete body read including disk-confirmed omitted ADD ESP,4. |
| `00716520` | `00716520-00716527` | 8 | ECX secondary owner; SUB ECX,4 then JMP `00716560`; inherited RET4. Complete raw thunk read. |
| `007150b0` | `007150b0-007150d8` | 41 | ECX base allocation itself; stacked flags; EAX captured input; RET4. Complete raw transient-base deletion body read. |

Total: **five candidate bodies, 347 bytes, zero bodies implemented by this discovery**. The direct destructor has no recorded incoming xref; it is a reviewed neighboring leaf, not a newly claimed startup edge. It can serve shared source logic without claiming that native startup calls it directly.

### Getter and constructor contract

1. Capture `00e19b90`; if nonnull, return that captured pointer. No manager call or lock occurs on the fast path.
2. At `007175f6`, call existing actual `00415350`. Capture that manager's section pointer at +10h in ESI. Initialize local guard profile `00ce37fc` and pointer. If nonnull, EnterCriticalSection and increment the current physical DWORD at section+18h. State 0 is armed only after this completes.
3. Recheck `00e19b90`. If still null, allocate exactly eight bytes at `0071762c -> 00bf681b` (one pushed argument; `ADD ESP,4` at `00717631`). On success write secondary+4=`00cfd7f8`, primary+0=`00cfd850`, secondary+4=`00cfd84c`, in that order, then publish the allocation. On null allocation explicitly publish null. There is no separately callable constructor or allocation-owning EH state in this getter.
4. Reload the publication. Capture nullable secondary+4 and **push it before** the second manager getter (`0071766d` push, `0071766e -> 00415350`). Register that captured subobject via `00717675 -> 00bd0c30`. Do not recalculate it after a callback changes the publication. A null result is still passed to registration. A registration exception retains the published allocation.
5. If the originally captured section was nonnull, decrement its current physical +18h word and LeaveCriticalSection. Return a fresh `00e19b90` load at `00717689`. Do not replace this with the fast-path captured pointer or reload a different manager's section.

The final primary profile at `00cfd850` contains scalar deletion `00716560` and Create `0071b870`. The final secondary profile at `00cfd84c` contains adjusting deletion `00716520`. The transient base profile at `00cfd7f8` contains `007150b0` and is written only during inlined construction; it is not the final registered profile.

### Deletion and EH

The primary direct and scalar bodies clear current `00e19b90` unconditionally, write secondary+4=`00ce3818`, then primary+0=`00cfd7dc`. They do not unregister, release another object, or clear `00f8d31c`. The scalar entry reads flags bit 0, optionally frees the captured **primary allocation**, and returns that captured address. Its null branch computes secondary=null and then still writes through it; the source must not invent graceful null handling. The secondary thunk subtracts four regardless of null before entering the primary scalar body.

The transient-base scalar entry clears `00e19b90`, writes only its own +0=`00ce3818`, and optionally frees that exact ECX allocation. It does not subtract four. This difference must survive the source API and any future profile dispatcher.

Getter FH3 handler `00c84c18-00c84c21` loads info `00db353c` and jumps to the existing CRT handler. Info magic is `19930522`, max state 1, unwind map `00db3534`. Its sole row is `toState=-1`, action `00c84c10`. That action computes `guard=EBP-14h` and tail-jumps to already reconstructed `00411ee0`, which consumes the captured guard section. There is no allocation rollback action. This is statically audited FH3 metadata; a new explicit C++ service interface must not claim original FH3/ABI compatibility merely because equivalent source cleanup is provided.

Ghidra preparation discovered: `007150b0`, `00716520`, `00716530`, and the getter handler `00c84c18` lack function definitions. Only the first three are proposed new implementation bodies. `00716560` omits three bytes `00716591-00716593` after returning `_free`; disk and live bytes establish `ADD ESP,4`. No writes were made by this worker.

Before this discovery commit, the primary integrator defined the three proposed raw bodies, repaired the scalar-delete tail, saved the project, and force-refreshed all five exports. Readback confirms those definitions and 18 instructions / zero gaps for `00716560`. The original discovery state remains recorded above. The current report-call check passes all 16 direct/tail rows; two Win32 import rows are explicitly outside that mechanical check.

### Existing contracts and named-but-incomplete dependencies

Use actual raw manager publication `01090aa0` through existing `native_singleton_publication`, `native_singleton_vector_registration_wrappers`, `native_diagnostic_sink_lifetime`, and `singleton_lifetime` source. Reuse the established Win32 lock, allocation, and guard conventions in `native_filestore_factory.cpp`, while retaining this getter's simpler EH state and inlined construction. Do not create a second lifetime domain or a semantic vector substitute.

`007188a0` is already reconstructed as `load_marker_resource_007188a0` in `marker_classes.cpp`, but it delegates its factory/manager/cache to host contracts. The native wrapper has only an ECX name input: both direct getter callers were audited (`008f840b`, `007188a4`), and incoming EAX is destroyed by the getter's first instruction.

`0071b870` (factory virtual+4) allocates 74h and calls `0071b810`; `0071b810` calls base `00b88260` then writes derived profile/list state. These are named but have no concrete reconstruction records at this source snapshot. `00b80720` load/cache, raw `004c1400`/`00b81040` manager ownership, and structured reader/root/parser dependencies remain larger source work. They are not required to reconstruct owner publication and must not be stubbed as successful resource creation. No resource/renderer packet is reassigned by this proposal.

## Packet 2: dependent process publication and raw deletion binding

Proposed packet: `orch2_game_resource_factory_process_binding_bb`, after packet 1 and after shared leases are released and peer changes merged.

Scope: add an actual `00e19b90` factory publication and separate `00f8d31c` alias to the application-owned singleton services; implement `GameStartupHost::publish_game_resource_factory` using packet 1; admit final numeric profile `00cfd84c` in the existing raw deletion dispatcher using the exact secondary thunk; retain context/cells through normal `008f8449` drain and exceptional host destruction. Installation of the deletion binding must precede factory registration. The alias must not be silently cleared by native deletion. Do not wire the Create virtual until its concrete source dependencies exist.

Candidate owned files: `include/bsp/game_hosts_singletons.hpp`, `src/game_hosts_singletons.cpp`, `src/game_hosts.cpp`, `include/bsp/native_singleton_destruction.hpp`, `src/native_singleton_destruction.cpp`, and new `docs/GAME_RESOURCE_FACTORY_PROCESS_BINDING.md` / `reports/game_resource_factory_process_binding.json`. Own only integration fragments at `008f840b` and the `00bd0400` profile dispatcher; packet 1 retains the five owner-body records. Avoid unnecessary `game_hosts.hpp` churn unless the final composition requires it.

At 2026-09-13 07:44 UTC the three application-host files were leased by `orch6_observer_application_r`; the deletion header/source were leased by `orch4_input_settings_lifetime_aq`. Therefore this packet is **dependent and not immediately claimable**, even though the pure owner packet is ready. Shared service lifetime, new dispatcher fields, and struct-size assertions must be reviewed against the then-current merged sources.

Proportionate validation for packet 1 is strict Win32 compilation plus existing tests and one focused original/source owner-lifetime fixture if needed for captured/reloaded publication and secondary-pointer risks. Packet 2 additionally needs an actual raw-manager registration/drain proof using that numeric profile and the same stable publication cells. Neither evidence class demonstrates resource parsing, title parity, native simulation, or gameplay validation. A future authorized process run should verify the formerly unimplemented call becomes concrete without treating the removal of that log line as the sole proof.

## Evidence and limits

`reports/startup_frontier_bb.json` carries source pins, all five complete captured listings, original call rows, missing-body records, and both packet ownership proposals. Ignored `local/startup-frontier-bb-evidence/` retains ten live-memory responses and binaries plus `capture.json`; capture SHA-256 is `f65182f06e8104e377656fe9a23327dfd6e3fbf8ad33930070a03ed798e50a9a`. Every captured span matches the installed PE (`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`) and live `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` memory via the project-verifying CLI. The installed game and saved Ghidra analysis were untouched by this discovery worker.
