# Native renderer private reset profiles

This extension admits the actual private physical-buffer profiles D61E10 and
D61E34 in B1FD90, alongside pooled D61E58 and D61E7C. It changes the parent and
adds two complete three-byte leaves B4B810 and B4B9C0: **76 owned original
bytes**, comprising the full 70-byte parent and six new leaf bytes. The existing
276-byte recreation bodies and 31-byte focus body remain unchanged.

Both private leaves are exactly `C2 04 00` (`RET 4`), followed in the installed
image by 13 INT3 padding bytes. They leave general registers, EFLAGS and
floating-point state untouched. They never read the receiver, stacked device
value, profile data, owner fields or COM storage. Their new fastcall interfaces
include an unused EDX parameter so the unused device remains stacked.
There is no allocator, retain/release, wrapper lifetime or device policy to add.

## Four-profile context

`NativeRendererResetReadinessProfiles` is now 16 bytes. Its first two fields
remain the borrowed original nine-DWORD pooled index and vertex tables. The
last two fields borrow the private index and vertex tables. The constructor
retains two-argument source compatibility by defaulting both new pointers to
null. A private pointer must be valid only when its corresponding current
profile identity is reached; neither is dereferenced on pooled-only or gate
return paths. Binary layout compatibility with the previous 8-byte context
is not claimed.

The source admits exactly four original profile identities and their four
established +20 targets. It is not general native virtual dispatch: other
identities/selectors remain outside the explicit source domain. Private +20
targets map to the two exact no-op leaves, not to pooled recreation or an
invented success result. All nine DWORDs of each reached borrowed table must
be the immutable original profile data.

Parent ordering remains unchanged: ready byte +1D8C, lost byte +1D8A, first
wrapper +1974, current device +1A10, early ready=1, current first profile and
its +20 selector, first call, current second wrapper +1978, its profile,
current device +1A10, current +20 selector, second call. The source adds no
gate reread, guard, result check or rollback. Its incidental EAX is still not
a semantic parent return contract.

## Concrete provenance

Fresh guarded Ghidra queries checked `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, before each analysis batch. Twenty-two live spans,
1,392 bytes, matched the installed PE. They include all original replayed code,
the two new entries and padding, complete profiles, original pool tables and
bounded physical-profile writers. Binary SHA256 remains
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

Private index construction in B4BF30 calls `operator_new` Bf681B with 2Ch and
installs D61E10 at B4C137. Private vertex construction in B4BC00 similarly
installs D61E34 at B4BE38. Destructors B4B900/B4BAB0 also install those private
profiles at B4B91D/B4BACD, including when destroying pooled objects. The
discovery in `NATIVE_PRIVATE_BUFFER_RECREATE_DISCOVERY.md` records the bounded
writer evidence. This extension neither infers allocator provenance from a
profile nor reconstructs the enclosing logical constructors.

## Current build and replay evidence

The standard strict MSVC Win32 build, both existing CTests and all eight native
seed checks passed. No shared CMake, permanent test, historical audit, Ghidra,
ledger or installed-game changes belong to this worker packet.

The ignored `local/private_reset_profiles` fixture preserves the earlier four
paired scenarios and adds one focused private-profile case. Its first real
pooled creation callback replaces the renderer's second wrapper with a private
index wrapper and changes the current device. The parent reaches the freshly
selected private index no-op. A second parent invocation uses private vertex
then private index with device pointer 1, which neither leaf dereferences.
Two direct leaf calls also use invalid unused receiver/device pointers and
check exact RET4 cleanup, nonvolatile registers, EAX and unchanged EFLAGS.

Four thread-local x86 execution breakpoints observe entry to the two original
and two compiled private leaves without changing their bytes. The probe records
ten actual entries: six reached through parents and four direct ABI calls.
Its vectored exception handler only records the entry context and enables
resume of the trapped instruction; debug-register state is restored afterward.
This instrumentation does not claim native SEH compatibility.

Both complete three-byte leaves match original, COFF, linked and runtime bytes
exactly. The five unchanged/raw bodies total 589 exact original bytes, with
only the existing three DIR32 address normalizations. The parent has a
241-byte executable prefix and 264 bytes of compiler-generated dispatch tables
and padding, all 505 bytes proven from COFF through linked/runtime images.
Both full profile lookup tables are checked for all four identity-to-context
field mappings, and all eight fixed calls resolve to the established providers.

Five original/full-library pairs compare 301,484 bytes of state/trace data.
The probe captures 78 unmodified raw renderer/wrapper snapshots, 106 real COM
calls and 36 terminal-zero buffer releases. Identity normalization is limited
to the actual renderer device and wrapper COM fields. Wrapper fixture storage
is borrowed, with profiles and native fields authored consistently with the
observed writers; no private allocation or whole-owner destruction claim is
inferred from these no-op tests. Existing full raw pooled constructors and COM
reset-release providers remain the setup/cleanup composition.

The actual archive and four exact members are frozen. All 347 mapped COFF
sections and 1,800 relocations match the linked image; 329 immutable sections
also match runtime postimages. All 189 mapped functions, including fixture
inlines, are accounted for. Twenty-eight complete runtime spans remain unchanged
across eleven postimage phases. Full buffer tables are checked throughout each
observed COM lifetime, and original/compiled entry, provider, return-address and
module identities are recorded.

This environment routes each actual device's slot-2 Release through Microsoft-
signed `SysWOW64/apphelp.dll`. The verifier admits that captured slot alone
after checking its Valid Microsoft Windows signature, PE hash and relocated
entry bytes. Other observed device/buffer methods remain pinned to their actual
D3D9 implementations. No target, HRESULT or output is substituted. Devices use
the existing desktop HWND; the fixture creates no window, changes no focus and
calls no presentation method.

The earlier two-profile worker and primary bundles remain untouched. Their
historical audit is retained; this extension's separate audit and immutable
bundle carry the new source and five-case proof. Evidence establishes this
explicit four-profile interface, not arbitrary profiles, every exception or
race, a general binary replacement, a full device-reset parent, or game runtime
validation.


## Primary integration

The unchanged extension passed the main strict Win32 build, both existing
CTests and eight fresh seeds. Primary checked84 worker sealed pins, four owned
inputs,18 current files and all22 fresh spans,1,392 bytes. Four exact main
archive objects preserve the whole reviewed code/data/relocation contents.
The unchanged five-pair fixture linked the frozen actual main library and
matched301,484 bytes,106 real COM calls,36 terminal-zero releases and ten
private-entry observations. All347 full COFF sections,1,800 relocations,189
functions and eleven runtime postimages passed. Immutable evidence is in
`local/private_reset_profiles_primary/`; previous bundles remain historical.

Primary defined both complete three-byte leaves, updated the existing raw
parent record with the four-profile domain while retaining its prior proof,
and saved/refreshed three reviewed annotations. This adds two new bodies and
expands one existing source interface. The wider lifetime/reset/game limits
above still apply.
