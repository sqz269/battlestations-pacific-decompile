# B317E0 declaration-name composition attempt

The existing-source fixture compiled under strict MSVC Win32 and the three
existing CTests passed, but it did not reach B317E0, B305F0 or B2DBD0. Loading the
pinned installed AlterBSP `xlive.dll` failed before the DLL constructor returned.
The diagnostic run establishes a write to a required original-host image offset
that is outside the source fixture's executable. There is no successful
decoder/cache, reference-count, token CRT callback or manager-drain result from
this packet. No production C++ changes were made.

## The bounded source composition

The one normal input is `PF44UF42UF41.MVFM`, with one miss and one identical hit.
The fixture uses the canonical GameNativeStringProcess string interface and
cells, GameNativeGraphicsPoolProcess declaration pool, retained immutable native
data, GameNativeVertexDeclarationsProcess token backing, GameSingletonHost and
the production NativeRenderActualOwnerRegistry. It uses corrected production
B48AF0; it never placement-constructs a count in the fixture. Planned credits
are creator1, miss-return2, hit3, caller releases2/1, cached retirement0 through
B32210(flags0). Those credits were not reached in either runtime attempt.

The preferred full BE1DC0 construction ran: genuine manager registration,
six real member sentinels and all six original canonicalizer probes. Both runs
observed 12 genuine string-pool getter/return calls, 39 lowercase calls and two
registered singleton slots. The observation wrapper delegates to the actual
NativePathCanonicalizerRuntimeServices; it supplies no successful substitute.
The empty mounted-tree date route, explicit BE25C0(flags1) retirement and final
host shutdown were planned but were not reached.

The final audit is registered before the declaration pool's real CRT callback.
The canonical token tables would initialize in B2DBD0 and register their actual
CE0C30/CE0C10 callbacks through std::atexit. The audit checks all 25 exact token
buffers in their expected return-ring positions while the genuine pool gate is
open, then verifies declaration-pool unlink and drains the canonical host.
Data, strings, host/log/bindings and audit state remain retained through exit.
The DLL failure prevents this completion proof. Source ordering and compiled
registration evidence do not substitute for observed callbacks.

## Sealed build and failed runtime evidence

Baseline: `7a6025ddbd7cc062cf6444427cffe21022aaaa4a`.
Fresh `scripts/build.ps1` passed strict Win32 compilation and all three existing
CTests. Standalone fixture compilation uses /MD /O2 /W4 /WX /fp:strict,
active assertions and /MANIFEST:EMBED. No new tracked test was added.

An initial standalone link failed with ten unresolved canonical game-provider
symbols because those providers belong to the game target, not bsp_core.lib.
That failure was archived before correction. An ignored provider library then
sealed the 72 already-built game-target CMake objects, excluding game_main.obj;
no provider was recompiled. The final map selects 35 of those members and 523
members across all four libraries. Every selected member equals its exact
CMake object. Merely archived or linked members are not execution claims.
The conservative readiness include closure omitted some game-target sources;
actual archived objects and sources were sealed separately against the baseline.
The first packaging-guard failure and uncredited first package are preserved.

Five functions have object/library-member/link-byte proof, including B317E0,
B305F0, B2DBD0, the B48AF0 constructor and the canonical token registration
adapter. B48AF0 retains one count store at +18h between base and derived stores
at +12h/+2Ch, with only its existing usage-array helper relocation. The token
registration adapter has the real atexit relocation. Native ranges and byte
evidence come from the unchanged frozen readiness packet; its six call reports
had 260 reported rows, zero failures and 11 inventory-only indirect/IAT rows.
The diagnostic's logger is additional C++ fixture code, not a native ABI shim.

Attempt1: parent75944/child22420 both exited C0000005. All 1432 sealed inputs
were unchanged and process closure was zero. Its last receipt localizes the
failure between the successful VFS receipt and the DLL-loaded receipt; it does
not establish the exact fault site. An empty bsp_debug.log created by the DLL
was discovered after that archive was frozen. It is preserved as a separately
pinned addendum, not claimed as an original member of that immutable ZIP.

One separately reviewed diagnostic replay used a C0000005-only vectored logger
with stack-only formatting, a preopened file, recursion protection, no reads
from fault addresses/targets, no context mutation and CONTINUE_SEARCH on every
path. Parent83952/child32324 both exited C0000005. All 1444 sealed inputs were
unchanged and process closure was zero. Every captured fault is preserved,
including repeated later ntdll faults; this packet assigns no cause to those
later faults. The diagnostic controller's exit0 means its capture/closure checks
completed, not that the fixture succeeded.

## Exact first-fault diagnosis and dependency limit

Installed module: I386 AlterBSP xlive.dll, SHA256
`71b50b3e91b5e17603f1d8fd44f1fc1da53c305f191dd126fb5059e5d3c841b2`.
The diagnostic first fault is EIP573B9049, module allocation570A0000,
RVA319049, write access to10640F5E. The exact pinned disk bytes8917 decode as
`mov dword ptr [edi],edx` in the copy-loop tail.

Static bytes provide the host-address provenance. At DLL RVA16B76A the code
pushes0, calls GetModuleHandleA through IAT104DD06C and stores EAX in the module
base cell104D8450. At RVA18C25E it pushes a five-byte size and stack source,
loads that cell, adds640F5E and calls the copy thunk154FD1, which jumps318B40.
That copy routine contains the observed failing write. These are exact static
instruction windows, not a claimed complete DLL function/caller reconstruction.
The observed destination equals the probe base10000000 plus640F5E. The probe's
SizeOfImage is A6000, so the destination is outside its image. With the original
base400000 this offset corresponds to A40F5E inside the original imageE2F000.
The installed DLL therefore reaches an original main-host image patch during
initialization before this fixture can call the declaration loader.

No supported explicit dependency configuration was identified for this host
requirement. XLiveLibrary's optional dependency vector loads additional absolute
DLLs; it does not supply bytes at an offset of GetModuleHandleA(NULL)'s image.
The installed bsp.exe.cfg is Liveconfig/title data, and the inspected DLL's
configuration string mentions title.cfg without establishing a disable-host-
patch option. This limited configuration inspection is not a universal claim
that no other configuration exists. It supplies no approved path around the
observed main-host image requirement. No system-DLL fallback, dummy host region,
module/native-memory patch, config edit or original-game launch was performed.

The proposed miss/hit, cache date, canonical count/retirement and real CRT audit
remain unvalidated by this packet. Native register/stack/FH3 ABI, private-frame
aliases, invalid/null cache rows, changed-name secondary scan, mounted filesystem
providers, full B4E470/source0/startup/draw and gameplay remain outside scope.
All prior readiness/dynamic-owner/atomic-lifetime artifacts remain unchanged.
Archive identities, inputs and the static diagnostic are listed in the report.
