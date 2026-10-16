# Explicit owning-UCRT Text comparison services, packet 61

The existing `game_hosts_text.hpp/.cpp` now exports two optional stateless Source
services. `source_ucrt_text_name_comparer()` returns the actual `::_stricmp`
function pointer. `source_ucrt_text_content_comparison()` constructs packet 52's
explicit Source-current profile with a private UTF16-to-wide callback. Neither
service is installed into the normal host by this packet.

These are modern owning-UCRT Source substitutions. They do not use a fabricated
Original locale flag, reconstruct the private CRT runtime, or claim equivalence
to Original parents 46/48/49, their entry ABIs or their non-C Unicode versions.

## Type, input and allocation contract

The build asserts MSVC Win32 pointer/int widths, two-byte `char16_t` and unsigned
two-byte `wchar_t` covering 0..FFFF, and exact type equality between the actual
`::_stricmp` pointer and `GuiNativeNameCompare`. The narrow service adds no
wrapper, input conversion or cached state.

The wide Source callback receives the actual original stored/copied u16 pointers
from the unchanged equality helper. It scans each valid terminated input and
makes **two temporary `std::wstring` value copies** into real `wchar_t` objects.
There is no cast between distinct character object types or unrelated CRT
locale/PTD types. Only after both copies does it call
`::_wcsicmp(wide_left.c_str(), wide_right.c_str())`; the owning UCRT consults its
then-current locale at that final call. Profile construction records no locale,
comparison result, Original flag, TLS view or process owner.

UCRT intentionally receives the wchar_t copy pointers, **not the original u16
pointers**. These copies may allocate and throw ordinary C++ exceptions. This
API therefore makes no Original pointer-forwarding, no-allocation, native fault,
SEH, original code/register or entry-ABI equivalence claim. Callers must supply
stable, valid, null-free terminated inputs throughout each call and serialize
locale mutation according to their owning Source runtime's contract. The wide
adapter's valid-input contract excludes reproducing UCRT's invalid-pointer
handler behavior before conversion. No locale setter is added to production.

## Existing adapters and preserved composition

The exact Source search found no public reusable UTF16/current adapter.
`FontGeometryOwner` already value-copies u16 data into `std::wstring` before its
own `_wcsicmp` call, but does not export this callback. Existing raw narrow
uses are member/header-specific; the public `compare_insensitive_00438e10`
adds null/pointer fast paths and carries another reconstructed function's
contract. Returning the actual `::_stricmp` pointer supplies the exact service
without duplicating that wrapper or relabeling it as Original BF7FBF.

The services return explicit values for a future caller to select. No default
binding, registry, private publication, locale mirror or new normal-host route
is installed. Packet 56's `font_names{names, b.font_registry, b.fonts, ...}`
initializer and the rest of the existing Source implementation remain unchanged
from Root `7e8390fecd4bfe6c46e0a9145517263d5a27d738`, including packet 59's
factory/producer registry admission. Packet 52's empty gates, post-provider
equality point and live input forwarding to the **Source callback** are unchanged.

## Actual owning-UCRT probe

A manifested MSVC Win32 `/MD /O2 /W4 /WX` standalone console probe compiles the
production `game_hosts_text.cpp` and links the real rebuilt libraries and 140
existing canonical `bsp_game` dependency objects. `/GL /Gy /Zc:inline`,
`/LTCG /OPT:REF /OPT:ICF` are used for this separate probe. No unresolved
production-host function is supplied by a dummy stub. The probe supplies its
own `main`; it does not call game WinMain or construct normal Text/FontHost.

The profile is created while the probe process uses `LC_CTYPE=C`. An earlier
callback changes only that probe process's UCRT `LC_CTYPE` to the actually
accepted `English_United States.1252`. For `Ä` versus `ä`, direct owning-UCRT
wide and narrow results change from **-32 to 0**. The same pre-created content
profile observes false -> true -> false when those callbacks select
C -> non-C -> C. The narrow service is verified to be the actual `::_stricmp`
pointer and matches the owning UCRT in both states. The original `C` locale is
restored and queried successfully before exit. The probe records a skip/failure
if no genuinely locale-sensitive non-C candidate is available; this run passed
and was not skipped.

The ordinary `scripts/build.ps1` also exited zero and all three existing suites
passed: `reconstructed_math`, `native_math_differential`, and `tool_tests`.
Primary Source review passed the complete two-file diff. No CMake source,
Ghidra/Native ledger, installation, system locale, audio/settings or normal
native Text activation changed. The actual runtime evidence covers these Source
services and equality helper in their private probe process; full GUI/game and
Original CRT compatibility remain unproved.

Receipt: `reports/cc12_native_text_source_ucrt_comparisons_61.json`. Reproduction
artifacts are under `local/cc12_native_text_source_ucrt_comparisons_61/`.
