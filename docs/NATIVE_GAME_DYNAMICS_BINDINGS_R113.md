# Native game dynamics bindings (R113)

Addresses:004DDB90,00BE4800,00C55F50,00C420E0,00C31A40.

Four game-construction providers now call real reconstructed services: processor count, Dyn engine initialization, world creation and the world callback-owner store. Six game-service providers remain required. Application startup still does not construct this raw game owner.

## Binding contract

`NativeGameDynamicsContext` borrows existing engine and world runtime contexts. The caller supplies the same actual engine publication/allocator/dispatch ownership used by the established constructors. No replacement globals, tables or private task managers are created by the binding.

- Parent4DE11C calls existing79B BE4800 processor environment behavior. Signed results convert to DWORD without clamping.
- Parent4DE129 passes its worker-count cell and borrowed engine context to existing C55F50. A cached engine bypasses descriptor access. New engines retain real profile/task-manager initialization and publication order.
- Parent4DE1D3 passes the captured engine and exact descriptor to existing C420E0 with the borrowed world context. It constructs a48Ch world and appends it to the engine's real pointer vector.
- Parent publishes the world at game+18, then4DE1DE supplies game+1C to C31A40. The complete10B setter writes world+24; null callback owner clears it. Native ABI is ECX world/one stack pointer/RET4, with EAX left equal to that pointer. The new C++ entry does not promise that register ABI.

ProducerC41AD0 zeroes world+24. The `CallbackOwner` label is a descriptive hypothesis based on the parent-provided game+1C interface; callback invocation is not validated by this setter packet.

## Current validation

Strict MSVC Win32 build and all3 CTests passed.17055 live/PE bytes match;427 CALL rows are audited, including the existing36-call read-only score producer and9 calls exercised by the separate Dyn fixture. No Ghidra body repair was required.

The29 composed parent cases passed **1661 ordered observations / 113347532 matching bytes** across83 copied envelopes14653B. They check explicit engine/world context routing, actual callback-owner stores, prior failure stages and no-replay. Engine/world providers remain controlled in this fixture.

The separately preserved Dyn fixture was rebuilt against this worktree and now calls the game defaults. The previously recorded Dyn source inputs are unchanged. Its native image and original PE hashes were rechecked before reuse; current processor/setter bytes were refreshed from the matching PE. It passed:

- Four native/source worlds: count/capacity1/2,2/2,3/6,4/6; cached-engine return with null descriptor; callback-owner set/clear.
- 280 live-buffer comparisons and111 final allocation/free events per side; **9380996 matching bytes**.
- Four original/source processor environment values:7,0,-2,nonnumeric; no clamp. Test changes affect only the probe process environment and are restored.
- 204 real Win32 handles closed, no tracked allocations left; exact original/source dispatch callbacks ran through real CRT atexit with matching deleted-critical-section state.

This Dyn check has zero worker threads and does not execute collision or solver methods. Its complete original method slots do not prove transitive method readiness. Manual diagnostic world/manager disposal is separate from native destructor validation. The composed and separate fixture evidence does not establish a fully connected original game startup.

Application executable equality except timestamps:True; changed application objects:[]. No runtime rerun. One new10B primitive and four reuse/composition fragments were recorded; existing engine/world bodies are not counted again. Prior Ghidra comments/names are retained, with the generic C31A40 inventory label replaced by a provisional behavior name.

## Follow-up packets

Remaining providers: `call_008d9150, call_00432650, call_0087d7b0, call_00717e80, call_0070bd70, call_00727bd0`. Inspect the unit-conversion table's uninitialized stack-word upper bytes, the global Lua loader's full body, resource-parser ownership, the84h grid owner and target-rank table before binding them. Native ABI/FH3, application construction/teardown and gameplay remain open.

Evidence: `reports/native_game_dynamics_bindings_r113.json`, `reports/native_game_dynamics_flow_r113.json`; both probes and their exact inputs/artifacts are sealed before integration.
