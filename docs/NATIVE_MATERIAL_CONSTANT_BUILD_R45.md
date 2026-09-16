# Material constant builder B42350

R45 adopts the corrected B42350 body from `a6b2884f2`, matching the primary
`7da09ce9` checkpoint after line-ending normalization. It covers the complete
4,288-byte B42350..B4340F body. The original ABI is ECX=pass, two stacked
entry/override words, RET8; the new C++ interface adds borrowed context and
persistent invocation state and is not a drop-in native replacement.

## Shared storage and providers

The unchanged historical context borrows the SAME VS bank0108EBF4 and PS
bank0108DBEC used by pass execution, including each live preceding DWORD header.
It borrows actual publication cells, type cells, float literals, profile words
and the existing texture-binding/threshold contexts. It owns no renderer, bank,
table snapshot or texture substitute. All raw extents/indices, live storage,
clear DF and the native floating-point environment remain caller obligations.

The old constant-build-leaves pair is omitted. Existing metadata providers serve
B17390/B5B880/B47900/B75E50/B782D0/B7AAB0; model-stream queries serve
B8FF00/B90620/B61E10. The B75E50 input is a volatile reference to the same live
first type cell. B90620/B61E10 use the existing ignored-EDX DWORD parameter.
Current texture-source and gather providers supply C302F0/BBCC40/BBCBD0 and
A8FCF0. The declaration getter B48CE0 remains the existing concrete provider.

## Preserved schedules

Parameter copies use overlap-safe memmove; matrix width2 copies bits through
scratch storage, while widths3/4 perform the original individual x87 crossings.
The decode loop initializes its step once, retains the live one read before the
second decoded component is spilled, and preserves signed/unsigned loop tests.
Unsigned light-count conversion retains its float rounding spill and subsequent
zero/store order. Animator/renderer profiles and matrix widths remain captured
at the original points; later owners, section pointers, binding pointers and
selected fields reload where the native body reloads them.

Each call requires its own fresh persistent `NativeMaterialConstantBuildFrame`.
Reached callsites are retained; ordinary C++ failure marks failed and preserves
all prior bank writes and child effects. Reuse of a completed/failed frame is
rejected. No cleanup, rollback, frame acknowledgement or retry is synthesized.
Unknown reached virtual targets fail explicitly instead of calling numeric game
addresses or providing successful fallback behavior.

## Validation boundary

The report records fresh current PE/live bytes, call receipts, strict Win32
`/MD /W4 /WX /fp:strict` build and all three existing CTests, and the restored
primary fixture freshly linked against current providers. Its two variants
compare both 4,096-byte banks against the complete copied native body and cover
matrix widths, overlap, skinning, decode, fade/diffuse/light writes and retained
partial failure. The fixture shares concrete B48CE0 and host memmove/invalid-
parameter boundaries. Its texture-source child checks use real source factories.

The full builder fixture has no texture-binding entries and uses shadow sampler
sentinels. Nonempty renderer/COM binding, the empty-light invalid-parameter edge,
all possible alias/fault schedules, native FH3/private stack/register ABI,
parent composition, active rendering and gameplay remain unproven. No permanent
tests, application binding or additional leaf bodies are introduced.
