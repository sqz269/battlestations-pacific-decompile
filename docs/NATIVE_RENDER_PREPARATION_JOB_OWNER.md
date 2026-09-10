# Actual preparation-job singleton lifetime

This packet reconstructs the five preparation-job lifetime entries below using an actual eight-byte owner, the existing `SingletonLifetimeDomain` and its real pointer-container manager, and the existing allocator/Win32 critical-section operations. It does not implement the primary execute target `B1BF70`, batch preparation, frame scheduling or render-command execution.

| Address span, inclusive | Original ABI | Reconstructed method |
|---|---|---|
| `00B0FFB0..00B1007D` | No inputs; EAX current singleton; RET | `get_singleton_00b0ffb0` |
| `00B0F1D0..00B0F1E9` | ECX primary owner; RET; no meaningful return | `destroy_body_00b0f1d0` |
| `00B0F210..00B0F243` | ECX primary owner; stack flags; RET4; EAX original primary pointer | `delete_primary_00b0f210` |
| `00B0F1C0..00B0F1C7` | Secondary ECX minus four; tailcall `B0F210` | `delete_secondary_00b0f1c0` |
| `00B0D930..00B0D958` | ECX construction-base pointer; stack flags; RET4; EAX same base | `delete_construction_base_00b0d930` |

Names remain hypotheses inferred from the command's preparation-job use. The new C++ interfaces are not original ABI entrypoints, even though raw owner size and field offsets are preserved on MSVC Win32.

## Storage and concrete dispatch

`NativeRenderPreparationJobStorage` is exactly eight bytes: primary table DWORD `+00`, then `NativeRenderPreparationJobSecondary` at `+04`, itself exactly one DWORD. Static assertions enforce both sizes and the subobject offset. There is no host vptr, refcount, queue pointer, destructor callback or manager pointer in these bytes. The companion borrows the shared lifetime domain, actual `F8D444` publication slot, and actual final secondary profile; its own destruction performs no native cleanup.

The native getter writes table transitions in this order:

1. `owner+04 = D5E154`, the construction-secondary profile whose entry zero is `B0D930`.
2. `owner+00 = D5E160`, whose entry zero is the separately unimplemented execution target `B1BF70`.
3. `owner+04 = D5E15C`, whose entry zero is `B0F1C0`.

Final manager registration is the actual secondary pointer `owner+04`. Existing `ConcreteSingletonLifetimeManager::register_object` already stores arbitrary pointers unchanged, and its destructor passes the exact stored pointer and native flag 1 to `SingletonLifetimeCallbacks::destroy_registered`. No manager API or storage extension was necessary.

The new `try_delete_registered` is a concrete callback route: it reads the registered secondary's current table, requires `D5E15C`, reads that supplied profile's current entry zero, and requires `B0F1C0`. It then actually executes `delete_secondary_00b0f1c0`, which subtracts four and calls primary deletion. Returning true means destruction ran. Returning false means the object was left untouched for another concrete dispatcher or explicit error handling. It must not be treated as successful disposal. The application callback aggregator must call this route for these registered owners; the packet does not replace unrelated owner dispatch.

The profile pointer must supply the actual immutable `D5E15C` word and remain valid. Null binding is rejected when constructing the companion, before native work. This is a host binding constraint, not a new native constructor branch. Stored native table addresses are identity tags, never callable host function pointers.

## Publication, locking and exception order

`B0FFB0` first reads `F8D444`; its nonnull fast path returns that captured pointer without manager, table or section access.

On the slow path it calls the real lifetime-domain getter and captures that returned manager's section `+10`. It enters the captured section, then increments the actual recursion counter at native section `+18`. Only successful completion of entry and increment arms cleanup. The singleton is checked again while holding that same section.

If still null, allocation requests eight native and eight host bytes. Placement initialization preserves the uninitialized preimage until the three ordered table writes. The getter publishes the constructed pointer, or null if the allocation boundary returned null. It then reloads `F8D444` and captures secondary `+04` or null **before the second manager getter**. Registration uses that captured argument on the second returned real manager; it does not reload publication after the getter. The real registration function is called even for null and retains its existing validation/null behavior.

On normal exit, the guard decrements the captured section's `+18` counter before LeaveCriticalSection. The final singleton reload and return occur outside the guard's scope, **after unlock**. A pointer replacement during the second manager getter must not change the registration argument; a replacement during unlock must affect the returned pointer.

Native handler `CBBC48` references EH info `DF3EB8`, whose one-entry unwind map `DF3EB0` runs `CBBC40 -> 411EE0`. That action restores guard table `CE37FC`, decrements the captured section counter, and leaves the section. The typed guard reproduces the observable section/exception ordering. If registration throws, the published job remains published and allocated; only the captured lock is cleaned up. No rollback, unregister, job deletion or publication reset is invented. If initial entry throws, cleanup was not armed and no leave is added. Native SEH stack-frame representation is separate from the new C++ guard ABI.

## Destruction and valid-owner domain

`B0F1D0` unconditionally clears the actual `F8D444` slot, then writes `CE3818` only to primary owner `+04`. It does not compare the destroyed object with current publication, reset primary `+00`, unregister or free.

`B0F210` performs that same reset, ordinary-frees the original primary allocation iff `flags & 1`, and returns its original address. `B0F1C0` receives the registered secondary, subtracts four, and tailcalls that primary route. The adjusted primary allocation is the free argument and return value.

`B0D930` is a different construction-base scalar destructor. It clears `F8D444`, writes `CE3818` at the supplied base pointer itself, frees that same pointer iff `flags & 1`, and returns it. It must not replace the final secondary adjustment route. The neighboring `D5E158 -> B0F1F0` is an unrelated service destructor; its unregister behavior is not part of this owner.

All destructor entrypoints require valid nonnull owners. Native `B0F1D0/B0F210` null paths attempt a write to address zero; these APIs neither silently accept null nor claim to reproduce access violations. The original functions do not remove registrations: manual deletion while a manager still retains that pointer is not a valid later-drain lifecycle. Normal manager drainage removes its slot before calling the concrete deletion route. The construction-base pointer must itself be a valid ordinary-free allocation when flag 1 is used.

## Evidence and integration

The packet started on main `52012c4`. Every analysis batch used `bsp.py ghidra`, which verifies project `bsp`, program `/battlestationspacific.exe`, language and image base before querying. The configured project is `C:/Users/sqz269/bsp.gpr`. Fifteen live spans, totaling 441 bytes, match the installed executable. The companion JSON records exact ranges and SHA-256 values; executable SHA-256 is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

The integrator should create the currently undefined functions at `B0F1D0`, `B0F1C0` and `B0D930` using the complete spans above, and preserve existing comments/old names before annotation. Do not rename enclosing candidate functions. `B0FFB0` already exists as `BSP_RenderBatchPreparationJob_GetSingleton_Provisional`; `B0F210` exists as `CG_scalar_deleting_dtor_00b0f210`.

`B0F210` has a concrete false-noreturn gap after its free call: live/disk bytes at `B0F235..B0F243` are `56 e8 71 73 0e 00 83 c4 04 8b c6 5e c2 04 00`. The missing `B0F23B..B0F23D` instruction is `ADD ESP,4`, followed by `MOV EAX,ESI; POP ESI; RET4`. `B0F1C0` bytes `83 e9 04 e9 48 00 00 00` prove the minus-four adjustment and jump to `B0F210`. Free support `BF65AC` is a five-byte jump (`e9 17 38 00 00`) to ordinary `_free` at `BF9DC8`. No Ghidra mutation or metadata edit was made by this worker.

Primary integration needs the new source registered in `bsp_core`, the five address/name/reconstruction entries and evidence annotations, and the concrete callback route wherever this owner is instantiated in a shared lifetime domain. Worker build registration is confined to ignored `local/preparation_job.cmake` via the local CMake cache; no shared CMake file is part of this commit.

## Verification boundary

`./scripts/build.ps1` passed with MSVC 19.51.36244.0 targeting Win32 Release, including this new source and the local fixture. The existing `reconstructed_math` test passed (1/1). The one local `bsp_preparation_job_lifetime_fixture` then passed both original and reconstructed paths.

The fixture uses copied installed entry bytes with explicit relocated globals/callouts, the real existing manager and registration/drain implementation, and real Win32 section operations. Its local observation hooks change publication during the second manager getter and after unlock; they do not replace registration or deletion with call records. It confirms that registration retains the initially captured `owner+4`, the final return observes the after-unlock replacement, and real manager drainage invokes final secondary-to-primary deletion/free. It also checks the nondeleting/body/base reset behavior with a different currently published singleton. The original EH metadata is relocated for the copied getter, but native exception unwinding is not claimed as fixture coverage. No allocation-failure or native null-fault runtime claim is made.

This is a complete typed reconstruction of the five bounded lifetime entries. It is not a native executable replacement, native scheduler implementation, game run, or rendering validation.
