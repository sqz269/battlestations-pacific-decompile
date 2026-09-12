# XLive manager owner lifecycle

This packet reconstructs the complete normal bodies of `00A40DF0`, base construction/teardown `00A3F530`/`00A3F5D0`, derived teardown `00A3F9D0`, deleting wrappers `00A3FDC0`/`00A3F670`, vector cleanup `00A3F840`, and IPC wrappers `00A4C250`/`00A4C280`. Names are hypotheses, not recovered symbols. These typed C++ interfaces are not native-layout or ABI replacements. Ghidra project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, was queried/exported read-only; raw disk instruction windows resolve false no-return gaps.

The earlier `start_online` in `audio_online_startup.cpp` is a partial reconstruction. Its whole-state assignment and comment claiming that the native object is zeroed must not be used as constructor evidence. This packet does not edit that existing implementation; the new complete entry is `construct_xlive_manager_00a40df0`.

## Publication and ownership

`XLiveManagerOwner` borrows the same online, flags, pump, sign-in, and runtime context used by the rest of the program. `XLiveManagerLifetimeAccess` supports the original canonical `SingletonLifetimeDomain` or the application's borrowed raw `SoundLifetimeAccess`, with one supplied `XLiveManagerOwner* volatile&` corresponding to F8ABE8. Current-manager consumers derive `&published_owner->context` from that slot; there is no independently published context. Raw registration requires `XLiveOwnerAllocation`, whose first word is the canonical `storage.vtable_00` and whose owned projection refers to the same borrowed state. Semantic registration retains the legacy projection identity. See [XLIVE_OWNER_LIFETIME.md](XLIVE_OWNER_LIFETIME.md) for allocation/free and shared-domain contracts. This source allocation boundary does not claim the native 3F0h layout or allocator at caller `0073DC50`; field preimages remain required.

Base construction writes vtable D24138, gets the lifetime manager, captures its +10 critical section, enters it and increments section+18, publishes the owner, gets the manager again, reloads the published slot, and registers that pointer. The captured section is decremented and released. Publication is observable before any derived member initialization. If registration throws, the slot is not rolled back: state1 first releases the captured lock via CB4128, then state0 restores root vtable CE3818 via CB4120. Unwind map DE9D90 contains {toState=-1, action=CB4120} and {toState=0, action=CB4128} in that order; unwinding visits state1 before state0.

Base destruction writes D24138, captures/enters the first manager's section, re-gets the manager, and unregisters the **current published pointer**, which can differ from the object being destroyed. It then clears F8ABE8, unlocks, and writes CE3818. A failure before the clear leaves publication unchanged. DE9DC4 similarly establishes CB4148 lock cleanup first, then CB4140 root-vtable cleanup.

## Constructor stores and service order

The assembly at A40E18..A41024 establishes this sequence:

1. After base publication, write derived vtable D2413C; clear debounce +4, ticks +8/+C, frequency high +14, callback +18; write frequency low +10 = 1 and supplied callbacks +20/+24. The +10 word is `OnlineSystemState.field_10`, not separate timestamp storage.
2. Clear +2D/+2E/+2F/+30/+31, +86/+87, float bits +88, flags +119/+120, vector slots +364/+368/+36C, +3BC/+3BD, +3E8/+3E9. Preserve +2C, +28, cached name/XUID/privilege, and other fields not explicitly written. The original constructor's log calls target verified bare RET `004254B0` and are not turned into callbacks.
3. Overwrite achievement batch +3A0 with null; zero only overlap +384..+394 (five DWORDs), preserving +398/+39C. Clear count/result +3A4/+3A8. Pointer stores do not free preimages. The vector uses placement replacement for the native three zero stores; its prior buffer is not implicitly deallocated.
4. Build the 1Ch zero-initialized initialization block; obtain the current renderer device, re-obtain its present-parameters address, obtain the language WORD, and call XLiveInitializeEx with 20029900h. The result does not control flow. Initialize actual IPC slot +3AC, then XOnlineStartup.
5. XWSAStartup(0202h) is followed by comparison of the returned output word with 0202h regardless of its result. A different word calls XWSACleanup. No fabricated zero output is supplied: an adapter must provide the actual output or explicitly report unavailable output. Call XSocketNTOHS(0C02h) and pass its result to XNetSetSystemLinkPort.
6. Zero all seven profile overlap DWORDs +3C0..+3D8, connected +8C, and link-failure +128; create listener for areas 2Fh and store the actual result at +1C. Null and -1 suppress only the no-effect success log.
7. Invoke recovered reset A40020 and recovered pump A409F0 using the same context. Only after they return, overwrite storage buffer +14C with null **without freeing it**, then write storage state +12C = 0. The pump can observe or mutate their preimages. The released host allocation remains the caller's responsibility, matching the native omission rather than silently adding a free.

The existing host-metadata fields recording initialization results, Winsock status, port, and listener validity are diagnostic observations; they are not additional native stores. Callbacks/SDK adapters must share the supplied canonical owners. Constructor defaults in the borrowed C++ types are not proof that untouched native bytes are zero.

Constructor unwind table DE9F50 has state0 -> base destructor CB4260 and state1 -> ID-vector destructor CB426B -> state0. Any exception after vector construction therefore destroys only that vector and the base; it does not close IPC, clean up SDK/Winsock/listener state, or free the storage buffer. The implementation preserves those omissions. C++ exceptions model this ownership order, not arbitrary Windows SEH exceptions.

## IPC and teardown

`00A4C250` takes the destination slot in ECX. A null destination returns E_INVALIDARG. Otherwise it calls real IPC create A4C030 with a **local output pointer**, then stores that returned handle on nonnegative HRESULT or null on negative HRESULT. The decompiler incorrectly aliases the output with the destination/self pointer. The underlying A4C030 allocates an IPC object, creates event/thread resources and invokes PIPEIPC services; it remains a required substantive boundary, not an XLive DLL export or a fake handle. `00A4C280` ignores null/-1 and tail-calls actual A4BDE0 for other handles without clearing the slot.

Derived teardown writes D2413C, closes the +3AC handle, frees and zeros storage +14C, frees/zeros the ID vector, and runs base destruction. It does not close the notification listener, uninitialize XLive/Winsock, free the achievement batch, or reset the remaining pump flags. The borrowed owners are not destroyed implicitly. If IPC teardown throws, DE9DF8 / CB4160 / CB4168 establish ID-vector and base cleanup while storage remains untouched. A second exception during unwind follows C++ termination semantics rather than a synthetic recovery.

The deleting wrappers inspect only flag bit0 and call the corresponding complete destructor before freeing the captured owner allocation. The legacy projection overloads return `&owner`; the `XLiveOwnerAllocation&` overloads return the captured allocation identity even after free. Raw bytes after `_free` contradict the decompiler's undefined EAX return. The host's required `free_owner_storage` must release that allocation only; it must not add destruction of independently borrowed state. `XLiveOwnerAllocation::free_owner_storage` supplies this callback for the new source allocation and its projection without adding a second free.

## Ghidra flow corrections and validation

All nine function starts already exist. Final instructions and exact ends are recorded in `reports/xlive_manager_owner.json`. False no-return overrides after BF65AC hide only these fallthrough bytes:

| Function | Call | Missing inclusive bytes | Meaning |
|---|---|---|---|
| A3F9D0 | A3FA1C | A3FA21..A3FA29 | ADD ESP,4; MOV [ESI+14C],EDI |
| A3F9D0 | A3FA35 | A3FA3A..A3FA3C | ADD ESP,4 |
| A3F840 | A3F84B | A3F850..A3F852 | ADD ESP,4 |
| A3FDC0 | A3FDD0 | A3FDD5..A3FDD7 | ADD ESP,4 |
| A3F670 | A3F680 | A3F685..A3F687 | ADD ESP,4 |

These are historical findings from the original owner packet. The AC lifetime packet rechecked both scalar functions: their post-free ADD ESP,4 instructions are now in the live bodies and both report zero gaps. It requests no additional Ghidra repair. No missing function entries or invented interior entries are needed. Constructor/base/IPC bodies had no listed gaps.

MSVC Win32 Release `scripts/build.ps1` passed with both existing CTests (`reconstructed_math`, `native_math_differential`); `verify-seeds` matched all eight seeds. One ignored focused fixture, `local/xlive_owner_fixture.cpp`, passed publication/preimage checks, initial-pump storage visibility and nonfreeing overwrite, partial overlap initialization, current-global unregister, native destructor omissions, and constructor/destructor exception cleanup. Logs are `local/xlive-owner-final-build.log`, `local/xlive-owner-seeds.json`, and `local/xlive-owner-fixture.log`.

Build/fixture evidence does not establish live DLL, asynchronous lifetime, binary ABI, or game validation.
