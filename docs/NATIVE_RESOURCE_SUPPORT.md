# Native resource-support singleton

The three functions below now have a concrete implementation backed by the
existing shared `SingletonLifetimeDomain`, its actual manager and critical
section, and the existing CRT allocation boundary. The name "resource support"
is provisional: surface construction/destruction calls the getter, but these
three functions alone do not establish the object's wider purpose.

| Native address | Complete extent | Original ABI | Reconstructed function |
| --- | --- | --- | --- |
| `00B3E730` | `[00B3E730,00B3E7ED)`; 189 bytes | No arguments; EAX owner; RET | `resource_support_singleton_00b3e730` |
| `00B61D50` | `[00B61D50,00B61D59)`; 9 bytes | ECX allocation; EAX same owner; RET | `construct_native_resource_support_00b61d50` |
| `00B61D60` | `[00B61D60,00B61D89)`; 41 bytes | ECX owner, stack flags; EAX original address; RET 4 | `delete_native_resource_support_00b61d60` |

These are new C++ interfaces, not binary replacement entry points. The module
requires MSVC Win32. `reports/native_resource_support_audit.json` records complete
byte preimages, original ABIs, source hashes, validation artifacts, and limits.

## Actual storage and shared ownership

`NativeResourceSupportStorage` is exactly eight bytes. The constructor writes
`00D62B64` at `+00`, preserving the four allocation bytes at `+04`. That second
word has no established meaning; it is not modeled as a reference count. The
profile's deleting virtual entry is `00B61D60`. Destruction writes base profile
`00CE3818` and also preserves `+04`.

The getter receives a reference to the caller's actual pointer corresponding to
native global `0108FEDC`. All adapters must pass this same slot and the same
`SingletonLifetimeDomain` corresponding to `01090AA0`. The module has no private
published pointer or registration container. Its lifetime is owned by the real
manager's raw-pointer list.

The integrating application's `SingletonLifetimeCallbacks::destroy_registered`
must dispatch this actual owner to `delete_native_resource_support_00b61d60`
with the supplied flags and that same canonical pointer reference. Native
`00BD0400` pops the entry and invokes deleting virtual zero with flag one. The
reconstructed manager already performs the corresponding real callback. This
module introduces no default or empty destructor callback.

## Publication and locking

The warm path returns the current published pointer immediately, without
looking up the lifetime manager, locking, allocating, or registering again.

The cold path retrieves the actual manager and captures its `+10` critical
section. If present, it enters that Win32 section and increments its actual
`+18` counter. It then rechecks the canonical pointer. If still null, it allocates
eight raw bytes, constructs the owner when the allocation is nonnull, and
publishes the result. It looks up the manager again and reloads the published
pointer before passing it to the real `register_object` implementation.

The same captured section is decremented and released. The final published
pointer is reloaded **after** `LeaveCriticalSection`; evaluating a return value
before an automatic guard's destructor would change this order. The C++ guard
therefore has an explicit release at the native normal continuation.

The native getter has neither an interlocked publication nor a separate object
mutex. Its double check and shared manager section are preserved. The existing
manager getter's own publication behavior remains the established dependency
described in `docs/SINGLETON_LIFETIME.md`.

## Complete deletion and exception evidence

The scalar deleter unconditionally clears the canonical pointer before writing
the base profile, even if the slot currently points to a different object.
Only flag bit zero controls freeing. It does not unregister a manager entry.
The complete native continuation includes `ADD ESP,4` at `00B61D80` after the
returning CRT free call, followed by EAX original identity and `RET 4`. This
continuation was checked directly because an earlier Ghidra body omitted it.

The getter's native exception map at `00DF75C0` has two states. State zero calls
the captured-section guard cleanup through `00CBEDA0 -> 00411EE0`. State one
first frees the captured allocation through `00CBEDA8`, then reaches state zero.
The cleanup leaf includes its returning `POP ECX; RET` at `00CBEDB1..B2`. The
frame handler begins at `00CBEDB3`, with FuncInfo `00DF75D0` and magic `19930522`.

The normal body restores state zero before publishing and registering. If
registration throws, the owner remains published and allocated while the guard
releases the captured lock. A later warm call returns it without retrying
registration. The implementation preserves that behavior; it does not add a
publication rollback. The allocation occurs while the guard is already active.

The native EH leaves and map are byte-verified evidence. The focused comparison
does not install the game's exception dispatcher or inject a native exception.
The recovered constructor is a nonthrowing leaf in the C++ interface; native
access-fault/SEH behavior is not claimed as reproduced by `/EHsc`.

## Validation

One local lifecycle fixture executes the original three resource-support
functions together with the original fifteen-function lifetime-manager slice.
Twenty complete code spans, seven patched-boundary preimages, and nine data
spans match the installed PE and the saved `bsp.gpr` program. Every live query
used `tools/bsp.py ghidra`, which verifies project/program identity. The original
installation and saved analysis were not changed.

The fixture loads only verified spans into isolated executable memory, applies
27 checked absolute relocations, and binds known CRT/Win32 boundaries. Both
paths use real `malloc/free` and actual Win32 critical sections. Allocation
observers seed raw bytes with `A5`; the original and reconstructed manager
implementations own and drain their actual pointer lists. The host path links
the actual reconstructed source and shared lifetime implementation.

Native and host match eight lifecycle events covering allocation under the
shared lock, publication before lock release, one retained manager entry, warm
calls without further work, shutdown with actual recursion depth two, clearing
the canonical slot and writing the base profile before free, preserved `+04`,
and flags `0`, `2`, and `101h`. A direct deletion with a different published
pointer confirms that the clear is unconditional. Both deleters return the
original address, including the freeing branch.

Within that fixture, a host-only throwing validation boundary exercises actual
manager registration failure. It confirms the published allocation survives,
the counter and Win32 recursion return to zero, and a warm call does not retry.
Fixture cleanup restores valid manager storage, registers the surviving owner,
and uses the real manager shutdown callback to delete it.

The module and fixture compile with `/std:c++17 /permissive- /W4 /WX /EHsc /O2
/fp:strict` for Win32. `./scripts/build.ps1` also checks the existing repository
build; this worker leaves CMake integration to the primary agent. The local
fixture is a focused artifact, not an additional tracked test suite.

## Remaining boundary

This packet does not reconstruct the surface wrapper, COM ownership, renderer
reset, or render-target group. It makes their concrete singleton dependency
available. See `docs/NATIVE_SURFACE_OWNER_NEXT.md` for that dependency chain.
The object's class name and unknown second word remain provisional. Allocation
failure, native EH execution, process-exit wiring, and game runtime behavior
were not validated. No Ghidra annotations, ledgers, CMake entries, or unrelated
source files were changed by this worker.
