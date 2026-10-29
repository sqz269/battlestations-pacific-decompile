# GUI Icon delayed-service process custody

This dormant P3c0 dependency supplies one typed Source F8BD98 backing cell and
retained C++ service metadata. It composes the existing genuine
`NativeGuiIconDelayedLoads`; no Icon, native singleton, manager, registration,
reference count, delayed entry or file-delivery path is created here. The new
worker files remained **unbuilt** during the immutable A1 series. Integrator
compilation and inactive-path checks are recorded below.

## Original preimage and evidence

The supported installed executable is
`I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe`,
SHA256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Read-only PE parsing on 2026-10-28 found image base00400000 and `.data` with RVA
A08000, virtual size297EDC, raw size10000 and raw file offsetA08000. F8BD98 lies
at section offset183D98, beyond raw data and within the virtual section. Its
four-byte preimage is therefore loader zero, not guessed disk bytes or an
initializer inferred from a C++ pointer type.

The configured `C:/Users/sqz269/bsp.gpr` / `/battlestationspacific.exe` target was
checked before the read-only analysis batch. `bsp.py ghidra bytes 00f8bd98
--length 4` returned `00 00 00 00`. The eight returned cross-references were:

| Site | Use |
| --- | --- |
| AB5715, AB574C | AB5700's initial and captured-lock recheck reads |
| AB5775 | Getter publication store |
| AB577F, AB579C | Getter's later current publication reads |
| AB55A1 | AB5560 destruction clears the current cell |
| AB21C0, AB2298 | Existing base destructor/scalar destructor null stores |

The existing exports for AB21C0/AB2290 explicitly store zero; they do not
provide a separate initializer. No initializer reference was observed in this
bounded query. The recovered native publication is owned by the real getter /
registered deletion schedule, not by a per-Icon copy. No Ghidra mutation,
renaming, export refresh or address/body reconstruction credit was added.

## Source API and ownership

`game_native_gui_icon_process()` follows `GameNativeGuiTextState`'s retained
process-owner pattern: a function-local pointer to lazily allocated C++
metadata, with no C++ atexit destruction. The one private typed volatile
publication begins null. It is not mapped at Original VA F8BD98 and never
contains a callable Original-code substitute.

`prepare_delayed_loads(NativeStringRawPoolContext&)` constructs only the
existing service's C++ metadata over that private cell. The service constructor
stores borrowed references and admits the fresh null publication; native
get/cancel/delete and SoundLifetimeAccess are not entered. Only after successful
metadata construction are the exact context pointer and service published.
Ordinary allocation/admission failure leaves this owner unprepared and adds
no entered-native-operation failure status.

The successful borrow is immutable. A different context object rejects, even
if it describes equivalent aliases. Repeating preparation with the SAME object
returns the SAME service; it does not clear a publication/failure, replace the
service, register an owner or acknowledge a drain. This preserves CURRENT
raw-string publication/import values and the SAME01090AA0 manager cell rather
than copying their values. Callers retain that stable context and its complete
provider graph through every reached operation, full drain and disposition.

`delayed_loads()` rejects before preparation and thereafter returns the existing
service. This accessor is not admission to replay or ignore a failed service.
`publication_00f8bd98()` returns only the current pointer value; it does not read
the pointee, increment a count, expose a writable cell or prepare anything.
All access is externally serialized with raw Icon calls and shared native drain.
New-host/post-drain service reuse is outside this packet.

## Status and failure boundary

The existing low-level `requires_process_retention()` deliberately includes
normal registered custody. The process view distinguishes these cases:

| Status | Meaning |
| --- | --- |
| unprepared | No successfully published C++ service/context borrow. |
| idle | Prepared service with no incomplete/failed operation or outstanding registration; also observable after completed normal drain. |
| registered_owner | Normal native registered-owner custody; keep the cell/service/provider graph through full shared drain. This alone does not require process exit. |
| incomplete_operation | Current diagnostic phase has not completed, including an in-flight call or retained nonlocal escape. Preserve entered state; do not treat it as idle or silently reuse it. |
| failed_operation | The genuine service stored an entered operation's exception. Preserve the graph and use the existing sticky failure/process-exit policy. |

The status is metadata observation, not a recovery/cleanup capability or proof
of every opaque provider's lifetime. It never dereferences retired payloads.
Allocation failure before the genuine service publishes an operation record
can leave idle status; that is distinct from an entered operation failure.

## Required root integration and limits

Root owns CMake and shared host changes. Before ANY `get_00ab5700`, assign
`singletons_->native_deletion_bindings().gui_icon_delayed_loads` to the SAME
service returned by this owner. The existing shared D5C4B8 branch must retain
that pointer through popped-owner AB57C0 deletion. The raw Icon storage context
must borrow that same service and deletion-bindings object; its existing
preflight rejects a mismatch before AB5700. Preparation alone does not install
the dispatch arm or establish native registration.

The later host binding is metadata only:

```cpp
auto& icons = game_native_gui_icon_process();
auto& delayed = icons.prepare_delayed_loads(vfs_->raw_strings());
singletons_->native_deletion_bindings().gui_icon_delayed_loads = &delayed;
```

Register the new TU alongside the existing Text process/state sources in
`bsp_game` when the integrator opens the build window. The snippet does not
call the service getter, instantiate a raw Icon or change the page switch.

Ordinary AB5E70 Icon destruction reaches the real getter/cancellation even
with `DelayedTextureLoad=false`. The getter alone performs its native
publication/manager registration; this process owner must not add another.
Keep the process cell/service AND canonical raw-string/manager/import graph
alive until every raw Icon and registered singleton has retired. Once shared
drain returns, verify idle/null with no interrupted operation before ending
borrowed providers; no further operations are admitted after those providers
expire. The retained C++ metadata is not another native payload owner.

No queue insertion AB5F00, delivery AB51B0/AB58D0, semantic Icon cast, duplicate
texture/state/count authority, factory activation or P4 owner transfer is
supplied. DelayedTextureLoad=true remains unsupported. Root must still supply
genuine raw Icon factory/properties/rebuild/current dispatch and parent/child
retirement before production activation.

Integrator source `eb18518db` registers the TU in bsp_game and passes MSVC Win32
Release plus both existing CTests. Candidate SHA
`05F362BB61DC3618D6CB560A0F3F3AEB332E720282CA726F0D62B04A89D2A0D2`
passes inactive reference4800/MSD1600 comparisons against accepted A1 ON799:
both exit0/pair0, ordered0, teardown0 and unchanged startup/RNG. The process
getter/preparation is uncalled in these runs, and no shared deletion hook was
added. Same-service binding, normal native drain and failure custody still need
independent proof before any raw Icon getter. Compilation cannot establish it.
The worker checkpoint kept P3a2 c5594a19a +066a46466 and A1 files unchanged.
Integrator validation includes the documented Page header/include fix and accepted A1 ON.
