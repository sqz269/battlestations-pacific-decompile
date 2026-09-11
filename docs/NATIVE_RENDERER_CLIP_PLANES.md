# Native renderer clip planes

This packet implements the complete native storage paths B23E50 (166 bytes),
B25040 (62 bytes), and B25080 (46 bytes). Existing camera-frame functions
remain typed fragments. These new interfaces use actual renderer offsets,
the established optional guards, and the complete cached render-state path.
Names are descriptive hypotheses; native caller ABI and gameplay are separate.

B23E50 takes ECX renderer and stack index/float4, RET8. Optional guard entry
precedes all coefficient access. Four sequential x87 FLD/FSTP pairs copy to
renderer+190C+DWORD(index<<4). The native EH state arms after the second store,
before the third load. Masked signaling NaNs therefore quiet in the cache
and set the native x87 status, while the caller's original input pointer is
passed to current device+1A10/current table+DC SetClipPlane. Input/cache aliasing
is observed one coefficient at a time. There is no bounds check, mask setup,
counter increment or HRESULT handling. Current mode is read before normal
cleanup disarming. Native handler CBCF38 uses FuncInfo DF5560 and guard
funclet CBCF30. C++ cleanup exceptions preserve the existing termination policy.

B25040 takes ECX renderer and stack float4, RET4. It calls full B23E50 with
current active+19EC, increments the current field only after return, rereads
it and calls full B24460(state98, DWORD((1<<(active&31))-1)). The x86 shift
count wraps to five bits. Callbacks can change active before the increment.
The parent adds no direct pending+19F0 store. Unchecked child cache aliasing
and callback changes remain visible, including changes to neighboring fields.

B25080 takes ECX renderer, plain RET. It derives the same mask from current
pending+19F0, calls full B24460, then reloads pending and publishes it to
active+19EC. A throwing child prevents this final copy; a child mutation of
pending is reflected. Neither parent adds a guard outside its existing child
guards or invents rollback.

The strict MSVC Win32 main build and both existing CTests passed. The primary
independently replayed the original bodies against the frozen actual main
library: all 23 pairs passed, with 517,904 literal DWORD comparisons and
271 snapshots. No pointer differences were normalized. No new permanent
tests, original caller ABI or game validation are claimed.


## Primary validation

The primary verified 79 worker artifact pins and nine unchanged current
source/provider files. Thirteen fresh live/PE spans cover 656 bytes; eight
original seed ranges matched. The unchanged fixture linked only the frozen
actual main library `28dbf127271da094b943a832790b69ef3408dfca38b8cd12ce95875fc2835f10`.
Three exact archive objects and 309 complete COFF sections match all linked
bytes and relocations, including fixture code: 119 entries and 14,885 unique
code bytes. Forty-seven normal postimage phases plus two per termination
child check entire original/linked code, read-only data and observed tables.
All 156 provider calls resolve to proven call sites and real HAL D3D9 or
Windows critical-section providers. Both secondary-cleanup exception
children terminate with marker 73 after real LeaveCriticalSection.

The comparisons cover forward/reverse coefficient aliasing, masked signaling
NaNs, raw and wrapped indices, active/pending alias changes, device/table/lock
mutations, cache-hit child guards, counter wrap, and armed/unarmed cleanup
exceptions. Complete compiled-code inspection separately confirms the EH
arm after the second x87 store and each parent memory access order. The
original bodies execute full guards and FH3 cleanup; the declared B24460
entry bridge invokes the complete established cached-state source provider.

Only masked hardware FP exceptions were exercised here. The actual HAL
returned success for tested out-of-range indices, so ignored HRESULT behavior
is established by code inspection rather than a failing-HRESULT run. A
skipped-entry guard was never made live by changing disabled mode to enabled.
Foreign profiles, original caller ABI and gameplay remain unverified.

Ghidra retains its previous names/comments with reviewed evidence appended
and saved; all three exports and complete function records are refreshed.
