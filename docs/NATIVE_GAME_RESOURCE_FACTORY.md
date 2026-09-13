# Native game-resource factory owner

Addresses: 007175d0, 007150b0, 00716520, 00716530, 00716560, 00c84c10, 00c84c18, 008f81f0, 007188a0.

This packet implements the five actual-storage owner bodies identified in `STARTUP_FRONTIER_BB.md`: an eight-byte factory singleton, its direct destructor, primary scalar deletion, secondary adjusting thunk, and transient-base scalar deletion. It uses the existing actual raw singleton manager, Win32 critical section, allocation/free services, and captured guard. It adds no private lifetime domain or resource-creation stub.

The process publication method in `game_hosts.cpp` remains a separate integration packet. This source can represent the owner reached by `008f840b -> 007175d0`; publishing it in the executable and admitting its profile in the shared raw deletion dispatcher still require those bindings. The native resource factory's virtual Create (`0071b870`), resource-manager/cache and structured parser remain independent dependencies. Font and GUI source already present in the executable is not reclassified as absent by this work.

## Coverage and original interfaces

| Entry | Inclusive span | Bytes | Native ABI | Source API |
| --- | --- | ---: | --- | --- |
| `007175d0` | `007175d0-0071769d` | 206 | No consumed input; EAX pointer; RET | `get_native_game_resource_factory_007175d0` |
| `00716530` | `00716530-00716551` | 34 | ECX primary; RET; no semantic result | `destroy_native_game_resource_factory_00716530` |
| `00716560` | `00716560-00716599` | 58 | ECX primary, flags stack; EAX primary; RET4 | `delete_native_game_resource_factory_00716560` |
| `00716520` | `00716520-00716527` | 8 | ECX secondary; SUB4/JMP primary scalar | `delete_native_game_resource_factory_secondary_00716520` |
| `007150b0` | `007150b0-007150d8` | 41 | ECX base allocation, flags stack; EAX input; RET4 | `delete_native_game_resource_factory_base_007150b0` |

All five source bodies are complete within the explicit borrowed service interfaces: 347 original bytes. Construction is inlined in the getter; it is not counted as another recovered function. The direct destructor is a complete neighboring leaf without a recorded native caller, not an asserted startup call. Descriptive names are hypotheses, not recovered symbols.

`NativeGameResourceFactoryContext` borrows stable references to the actual mutable `01090aa0` manager and `00e19b90` factory cells. WinMain stores the returned factory in another cell, `00f8d31c`, at `008f8414`. None of these five bodies reads or clears that alias. Do not collapse the cells or automatically clear the alias when deleting the owner.

## Getter ownership and observation order

The first publication load is returned immediately when nonnull. On a miss, the getter captures the first actual manager's section pointer at +10h, constructs the raw eight-byte guard `{CE37FC, section}`, and, if nonnull, enters and increments physical section+18h. The protected state begins only after the increment.

After the locked recheck, allocate exactly eight bytes. The inlined construction stores secondary+4=`CFD7F8`, primary+0=`CFD850`, secondary+4=`CFD84C`, in that order, then publishes the allocation. Null allocation is explicitly published as null. There is no extra constructor call or allocation rollback state.

Reload the publication and capture nullable secondary+4 **before** the second `00415350` call. Native `0071766d` pushes that pointer before `0071766e` calls the manager getter. Pass the captured pointer to `00bd0c30`, even when null. The manager lookup and registration may observe changed publication values; neither permits replacing the captured registration argument.

Normal release decrements and leaves the originally captured section. The slow result then reloads current `00e19b90` at `00717689`. This differs from the fast result, which preserves its first read. The source uses volatile actual cells and DWORD stores to retain these native observation boundaries.

| Original call site | Target / contract |
| --- | --- |
| `007175f6`, `0071766e` | Existing actual `00415350` manager getter |
| `0071760f`, `00717683` | Actual Win32 Enter/LeaveCriticalSection through native imports `CE2218` / `CE2210` |
| `0071762c` | `00bf681b`, one size argument 8; cleanup `ADD ESP,4` at `00717631` |
| `00717675` | Existing raw `00bd0c30`, ECX second manager, captured nullable secondary stack argument; RET4 |
| `0071658c`, `007150cb` | `00bf65ac`, captured allocation base; caller ADD ESP,4 |
| `00716523` | Tail JMP `00716560` after ECX-=4 |
| `00c84c13` | Sole getter unwind tail JMP `00411ee0`, ECX guard=EBP-14h |

Both recorded direct getter callers are audited: WinMain `008f840b` and the name-only resource wrapper `007188a4`. The latter already has sequence source in `marker_classes.cpp`, but obtains the factory/manager/cache through host contracts. The getter overwrites incoming EAX in its first instruction, so that register is not a hidden input.

## Deletion profiles and cleanup

Final primary profile `CFD850` contains primary scalar `716560` and Create `71B870`. Final registered secondary+4 profile `CFD84C` contains `716520`, which subtracts four before primary deletion. Both primary destructor bodies clear current `E19B90` unconditionally, write secondary profile `CE3818`, then primary profile `CFD7DC`. They do not unregister. Scalar deletion frees the captured primary only when flags bit 0 is set and returns that captured primary address.

Transient base profile `CFD7F8` contains `7150B0`. This scalar entry acts on its exact ECX base allocation: clear `E19B90`, write only ECX+0=`CE3818`, optionally free that same address. It performs no secondary adjustment and does not overwrite +4. These are separate source APIs so a future raw-profile dispatcher cannot accidentally conflate them.

The native primary bodies calculate a null secondary for null ECX but then still store through it; the secondary thunk subtracts four unconditionally. No successful-null deletion behavior is invented. Invalid-pointer execution and hardware-fault unwind are outside the C++ source contract and were not invoked by the fixture.

The getter handler `C84C18` loads FH3 info `DB353C`: magic `19930522`, max state 1, unwind map `DB3534`. The sole row is `{-1, C84C10}`, whose action computes EBP-14h and jumps to actual `411EE0`. The source catch calls that existing captured-guard helper and rethrows. There is **no allocation rollback**: an exception during registration retains the published owner. Original FH3/SEH dispatcher behavior, CRT exception identities, and mutable native EH stack spill aliases are not reproduced by the new C++ interfaces.

The primary integrator defined `7150B0`, `716520`, `716530`, repaired the returning-free gap `716591-716593` to restore `ADD ESP,4`, saved Ghidra and refreshed all five exports before implementation. This worker made no Ghidra writes. Original and live PE bytes match for all five spans; support metadata/profile captures are retained with the discovery evidence.

## Validation and remaining integration

Validation results are recorded in `reports/native_game_resource_factory.json`. The local fixture is `local/factory-fixture-bb/replay.py --repo <already-built checkout> --output <fresh directory>`. Its original-mode output is frozen before first source-mode execution. It maps and executes all five original owner bodies, then compares the current MSVC library owner object against that fixed trace.

The strict MSVC Win32 build passed, including the existing `reconstructed_math` and `native_math_differential` CTests. The first focused fixture attempt passed all eleven original/source normal cases and both source-only exception cases. The fixed original trace SHA-256 is `1681e80b0e74ae263d92e3c71a4cf8a1936434393613bc96e1644034100d6de3`; one current owner object was verified as a unique archive member, with 166 source compiler inputs and 11 fixture project headers observed. The report's ten direct/tail call rows pass live verification; the two Win32 import contracts are explicitly not checked by that mechanical verifier.

The eleven normal cases cover fast return, fresh allocation, publication appearing during the first manager call, a second manager call changing both publication and manager, null allocation with a null section, registration changing publication, and all four deletion entry forms including non-bit0 flags. Actual Win32 critical sections are used. Manager, allocation, registration, free and unwind-guard services are shared test-only adapters; this isolates owner-body behavior and is not an actual raw-manager drain proof. The exact source object is verified as a unique current `bsp_core.lib` member; headers/compiler dependency inputs, MAP and source/object hashes are retained for replay.

Two additional **source-only** exception scenarios check allocation and registration throws, single captured-guard cleanup, restored locks/counters, and absent allocation rollback. They do not execute or validate the original FH3 dispatcher. No permanent test was added.

The dependent application packet must bind numeric profile `CFD84C` to the exact secondary deletion entry, using the same raw manager and stable publication cells, before registration. Contexts must outlive normal `008f8449` drain and exceptional startup cleanup. At implementation time, the shared application/deletion files belong to other agents; this packet does not modify them. Neither compilation nor the focused fixture establishes native binary compatibility, successful original resource creation, rendered parity, or gameplay validation.
