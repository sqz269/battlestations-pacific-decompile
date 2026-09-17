# Actual online-manager lifetime and shared drain (R102)

Addresses: 00a3f530, 00a3f5d0, 00a3f670, 00a3f840, 00a3f9d0, 00a3fdc0, 00a4c280

The native loading-message pump consumes a 3F0h online allocation. The older
`XLiveOwnerAllocation` owns a C++ projection with a different layout and cannot
be cast to that storage. This packet supplies seven normal lifetime bodies over
the existing `NativeOnlineManagerStorage`, borrowing the actual AA0/F8ABE8 cells.
It also adds an optional raw-online binding to the shared singleton drain. Full
A40DF0 construction and application online startup remain separate work.

## Recovered behavior

- A3F530 writes D24138 at +0 and preserves every other allocation byte. It gets
  the manager, captures/enters its +10 section and increments section+18,
  publishes the allocation, gets the manager again, reloads F8ABE8 and registers
  that identity. It releases the captured first section.
- A3F5D0 unregisters the **current** F8ABE8 from the second manager, even when it
  differs from self. It clears F8ABE8, releases the captured section, and stamps
  CE3818 on the captured object. Failed base construction leaves publication
  intact while releasing the guard and restoring the root profile.
- A4C280 ignores null/minus-one IPC handles; other handles reach existing
  concrete A4BDE0 teardown. +3AC remains unchanged. The IPC allocation and its
  borrowed services must remain valid through worker teardown.
- A3F9D0 writes D2413C, closes +3AC, retains the two native +14C loads, frees and
  zeros the storage buffer, frees +364 and zeros +364/+368/+36C, then destroys
  the base. It preserves vector+0 at manager+360 and all unrelated bytes. It
  performs no listener, SDK, Winsock, or achievement-batch cleanup.
- A3F840 takes the embedded vector at manager+360, frees nonnull header+4 and
  zeros header+4/+8/+C, including when its storage is null.
- A3F670/A3FDC0 destroy the base/derived owner, free only for flags bit0 and
  return the captured pointer even after release.

Native entries take ECX=self (or vector/handle); scalars consume a DWORD flags
slot and RET4. Source lifetime functions add EDX=context. Profiles are original
identity words, not executable C++ vtables. No second allocation/publication or
projected owner is created. Allocations use the existing CRT release domain.

The raw drain binding is appended at offset148, retaining all older binding
offsets. D24138/D2413C dispatch takes the popped object regardless of the current
publication. Supplying both legacy and raw online bindings is rejected before
either destructor is selected.

## Evidence and validation

The report records 688 bytes verified against live `bsp.gpr` /
`battlestationspacific.exe` and the installed PE: 574 normal-body bytes, six
unwind funclets, three unwind maps, and base/derived/root profiles. All function
listings are gap-free. DE9D90/DE9DC4 unwind guard before root; DE9DF8 unwinds the
vector then base, leaving +14C alone if IPC close throws. Source C++ cleanup
preserves those ownership actions; original FH3/SEH and stack aliases are open.

Strict MSVC Win32 build and all three existing CTests passed. An isolated local
fixture executes copied original instructions and the source implementation
against the same checks. Only direct target/publication/IAT relocations change;
original control flow and stores remain. Original copies run on nonthrowing
paths; their untouched FH3 handler addresses are not exception-tested.

The fixture verifies null/non-null standalone ID-vector cleanup; base/derived
flags0/1/100/101; null/minus-one IPC guards; complete3F0 byte preservation; and
manager/publication changes while forwarding actual Win32 lock calls. It also
verifies the source D2413C shared drain with three ordered real CRT frees and a
source C++ registration failure retaining publication while restoring root.
No permanent tests were added. Nonnull IPC teardown and its exception path were
inspected but not run in this fixture; no online session or IPC worker started.

The ordinary application smoke also exits normally after two ticks/one Present,
joins its renderer worker and ends with device/API COM counts0/0. This does not
exercise native online startup, which is still unbound.

## Follow-up dependencies

Compose full A40DF0 construction with real SDK outputs, retained IPC globals and
services, raw online pump/sign-in contexts and the canonical singleton domain.
Then migrate application consumers from the projected online publication before
binding the procedural sampler loading-message pump. The first sampler stack
word remains unresolved; this lifetime packet does not normalize it. Full
material/preload startup, binary ABI and gameplay validation remain open.

Evidence: `reports/native_online_lifetime_r102.json`; immutable local tested and
combined-build archives are recorded there with hashes and artifact manifests.

## Correction from docs/NATIVE_ONLINE_STARTUP_R103.md

R103 recovered an older, unintegrated constructor/SDK/raw-IPC implementation.
Its complete lifetime modules now supersede R102's standalone source/header.
The shared drain keeps the `native_online` binding at offset148, borrowing the
complete lifetime with raw2Ch IPC services and the matching allocation domain.
R102's fixture and archives remain historical evidence for its tested source;
R103 records fresh verification of the consolidated implementation. In
particular, the canonical native IPC allocation contains no appended service
pointers; those services remain outside its 2Ch storage.
