# Physical failure callable publication design

**Propose one explicit opt-in method on the existing application/runtime owner: `void* publish_and_borrow_raw_failure_manager()`.** It publishes the qualified raw C3 symbol at the genuine initialized manager’s `+90` and returns that same borrowed manager. Normal startup retains numeric Original identity mode. This is a design for review; no Source change, build, link, probe or execution is performed or authorized here.

## Existing owner and the missing checks

`GameNativeVfsApplication::Impl::initialize_core` allocates the real A0h block, retains its runtime and services, binds it to the existing singleton host, and constructs/registers the owner. The runtime marks `core_registered` only after constructor, identity installation and FileStore/MPKG registration complete. The application then binds the registry domain. Current `runtime()` checks only retention; `actual_manager()` returns the stored pointer; `borrow_raw_services()` explicitly permits pre-core borrowing. None is a completed callable-owner certificate.

Add a private application completion flag set only at the final successful end of `initialize_core`. The new public application method requires that flag and delegates to a **private** runtime method through `friend class GameNativeVfsApplication`. This prevents the current `runtime()` accessor from bypassing the completion guard after a late initialization failure. Failed initialization retains the existing shared-drain obligation; no retry, allocator, synthetic publication, owner class or rollback is added.

## Concrete API and ordered contract

```cpp
// Public on GameNativeVfsApplication; private on GameNativeVfsRuntime.
// The runtime declares GameNativeVfsApplication as a friend.
// The result is borrowed only; no arbitrary owner parameter is accepted.
void* publish_and_borrow_raw_failure_manager();

// Shared helpers in native_vfs_startup_callbacks.hpp/.cpp.
std::uintptr_t qualified_native_vfs_raw_failure_target();
bool invoke_native_vfs_startup_failure_target(std::uintptr_t captured_target);
```

1. Require MSVC Win32 and four-byte pointers. Require completed application initialization, runtime `core_registered`, and `!retired` before reading owner storage.
2. Under caller-provided exclusive access, read the existing volatile publication once. Reject null or a pointer different from `inputs.actual_vfs_storage_a0`.
3. Resolve **`&raw_ignore_native_vfs_mount_failure_00530620`**, the existing exact C3 entry. Read its single volatile code byte at that trusted linked symbol address and require `0xC3`, otherwise throw. This reads code and never calls it. The future image must establish readable executable storage and lifetime; the ordinary typed ignore helper and literal `00530620` are not substitutes. No caller-controlled address or unloadable module is introduced.
4. Require the captured owner’s `+90` to contain either `00530620` or that exact qualified symbol address, and its `+8C` to remain `00735B30`. Reject all other states before any store.
5. Store only the actual symbol address into that owner’s `+90` DWORD and return the captured non-owning pointer. Invoke no callback/notifier and write neither `+8C`, `+18` nor a publication cell. Repeating the opt-in is idempotent when all guards still hold; no reverse-mode API is needed.

These methods can throw on failed phase, owner, preimage or code qualification. The raw leaf and notifier remain `noexcept` and byte-exact. The new operation supplies no notifier EDX or discarded stack word.

## Preserve both startup reads and the separate +8C slot

The retained Original fragment reads actual global 0109CEEC into ECX, stores `+90=00530620` at `0073D642`, reads the global again into EDX, then stores `+8C=00735B30`. Current Source independently reads the volatile publication twice. Its complete installer is 32 bytes / 9 instructions with zero relocations.

Leave that installer unchanged. The proposed later `+90`-only operation is not a replacement of the four-instruction fragment. It must not collapse the two startup reads, re-run their stores, turn `+8C` into a function pointer, or claim the Source publication reference is Original absolute 0109CEEC. This packet freshly verified only the 10-byte store `C7819000000020065300`; the enclosing 32-byte evidence is retained from the prior audit.

## Compatibility and exact ABI

Both existing typed dispatchers reject a Source pointer today. The shared finite helper must preserve Original identity dispatch and add **only the exact known raw symbol**: identity calls the existing typed no-op; matching Source address is qualified then calls the named raw C3 function directly. Unknown words return false without dereference, letting each caller keep its existing rejection/diagnostic. Never cast an arbitrary word to a callback.

| Current consumer | Required treatment |
| --- | --- |
| `NativeVfsStartupCallbacks::mount_failure_00be18b8` | Use finite helper; keep its current mount-specific rejection |
| `NativeVfsRuntimeBindings::open_failure_entry` | Use the same helper; keep `unsupported()` rejection |
| Runtime FileStore dispatcher and typed BD9E30 | Already forward captured target/manager to that binding; no caller change |
| Mount failure and bound VFS open failure | Already use those two dispatch routes; no caller change or publication recapture |
| Unbound VFS open / typed physical ReadFile failure | Already directly call `+90` with two fastcall register inputs; qualified C3 accepts that call. Identity mode alone does not authorize direct calls |

Raw BD9E30 remains exactly `8B8190000000FFD0C20400`: load EAX from `[ECX+90]`, call EAX, `RET4`. ECX is the genuine borrowed manager; EDX reaches C3 unchanged; the separate stack DWORD is discarded after return. **No equality between EDX and the discarded DWORD is assumed.** This entry reads neither global 0109CEEC nor manager `+18`. With C3 only, EAX remains the target and other non-stack registers/flags survive. Typed dispatch has no such register-ABI claim. Physical BF5030’s own publication loads, `+18` provenance and connected raw-read qualification remain separate.

## Implementable lifetime contract

The pointer creates no ownership or synchronized lease. The caller serializes startup, opt-in publication, all uses, and shutdown; no concurrent publication replacement, callback mutation or drain is permitted. Finish **every borrowed use before `GameSingletonHost.shutdown()` begins**. Keep the application, runtime, host, referenced data/services and executable code alive through uses and shared drain; destroy the application afterward. Retirement rejects later borrows before owner dereference. A new lock, borrow counter, owner class or lifetime guarantee cannot be inferred from the current code.

## Disjoint future Source file sets

| Set | Existing files | Changes |
| --- | --- | --- |
| A | `native_vfs_startup_callbacks.hpp/.cpp`, `native_vfs_runtime_bindings.hpp/.cpp` | Known-symbol qualification/helper, two finite dispatch callers and comments; preserve installer body |
| B | `game_native_vfs_application.hpp/.cpp`, `game_native_vfs_runtime.hpp/.cpp` | Completion flag, forwarding method, guarded publication and borrowed return |

Headers are under `include/bsp`, bodies under `src`. Set B depends on A’s agreed helper contract despite disjoint files. Raw failure entry files, current physical/mount/open/FileStore caller bodies and shutdown owner remain untouched. All modules are already registered; no new CMake source list is needed. Root must authorize a separate implementation packet after reviewing this design.

## Evidence and validation boundary

Accepted capture: `local/cc12_physical_failure_callable_publication_design_20261008a/audit03`. Manifest SHA-256 `e50db2670af280526d5e7c7e7bc154a51d1bad2969be8ebc90585469837ca850`. All 66 physical transfer rows match capture/copy/after hashes: 10 whole provider/header pairs, 10 whole existing objects, 8 exact unique core archive members and 2 direct game objects, plus selected contracts/build recipes/tools. All 20 selected Source functions decode completely, including the 1-byte C3 and 11-byte notifier. Copies were made after earlier builds; no fresh build, exhaustive compiler/OS closure, linked/loaded image or initialized live manager is claimed.

Future validation must retain whole affected build artifacts and qualify the actual linked/loaded raw symbols before separately authorized execution on the genuine owner. Existing Source/archive proof does not establish runtime readiness. The pending owner-producer report is preserved unchanged; current contracts were checked directly. Details and file anchors are in [the JSON report](../reports/cc12_physical_failure_callable_publication_design.json).
