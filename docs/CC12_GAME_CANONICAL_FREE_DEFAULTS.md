# Canonical Source free defaults for the raw game adapters

`NativeGameContainerLifetimeCalls::free_00bf65ac` and
`NativeGameLifetimeCalls::free_00bf65ac` now call the existing
`singleton_lifetime_free` service. This pairs their default release path with
the accepted canonical Source allocation routes. The contract accepts null or
raw storage returned by `singleton_lifetime_allocate`; a foreign allocation
domain must override every reached matching free boundary.

Only the two bodies, one direct include in `native_game_lifetime.cpp`, and two
declaration comments changed. Virtual signatures, possibly throwing overrides,
null calls, call order, and owner policy remain intact. The standalone array and
embedded defaults, grid/physics/child providers, `free_00bf6989`, and production
callers are unchanged. This does not compose the missing application/raw-game
owner or bind the production queue.

## Source and build evidence

The worker began at `83239a8b9b0789ca37d197638428008c720bf857`, which contains the
accepted domain-readiness review `193b1eebf`. All 4,206 tracked `src/` and
`include/` files are retained before and after. Exactly the four authorized
files differ; the other 4,202 are raw-byte identical. The eight existing test
sources are also unchanged.

The retained Source3352-wide epoch contains 3,352 selected inputs and four old
build artifacts. Its selected inputs match the fresh before inputs after
explicit CRLF/LF normalization; seven raw newline differences are listed in
the report. The four old artifacts retain their historical identity and receive
no unchanged-build credit. `Source3352-game-canonical-free-defaults` separately
retains the 3,352 current selected inputs and four fresh after artifacts. Its
only semantic input changes are the four authorized Source files. The actual
after link map is retained; no before local link-map equality is claimed.

An initial artifact cohort was stale: the fresh baseline rebuilt 23 of 1,988
Core objects and 36 of 80 App objects. The correction is compared with the fresh
baseline, not that initial cohort. Both the fresh baseline and corrected builds
used `scripts/build.ps1`, MSVC Win32 Release, and passed the same three existing
checks: `reconstructed_math`, `native_math_differential`, and `tool_tests`.
No tests or probes were added. These checks provide build/regression evidence,
not a direct fixture for the corrected defaults.

The integrator separately ran the repository's `verify-seeds` prerequisite in
the existing `bsp.gpr` and `/battlestationspacific.exe`: eight read-memory queries,
501 bytes, all matching the original file. The worker copied the exact verified
2,995-byte header, SHA-256
`26cdde71970ec8524c92d5e54e17c5587e817198ef16f3b6f2b8be25cd556d2c`.
The receipt and comparison are retained as that separate prerequisite. This
packet performed no fresh Native/Ghidra queries or mutations. The existing
differential test executes its existing math seeds; it does not test the
original free provider or queue.

## Complete emitted-function and object comparison

Each corrected adapter has the same complete 18-byte emitted instruction
payload before and after:

```text
55 8B EC FF 75 08 E8 00 00 00 00 83 C4 04 5D C2 04 00
```

The single call relocation at function offset 7 changes from
`??3@YAXPAX@Z` to `?singleton_lifetime_free@bsp@@YAXPAX@Z`. Both before/after
function records include all instructions, original COFF symbol indices,
physical offsets, relocation records, and operand bytes. The unique concrete
Source provider is in the unchanged whole `singleton_lifetime.obj`; its complete
six-byte body is an indirect tail jump relocated to `__imp__free`. This verifies
the Source call route, not the Native CRT provider or a drop-in binary ABI.

Of all 1,988 actual Core objects, 1,984 are whole-byte identical. The two target
objects differ in their free-call symbol/relocation and COFF metadata. The other
two rebuilt objects, `game_native_game_runtime.obj` and
`native_game_current_virtual_calls.obj`, differ only in their raw COFF timestamp.
All 188 other function payloads and symbolic relocation operands across these
four changed objects are preserved. Every section payload is preserved; only
the two selected executable-section relocation targets change.

All 80 actual App objects are whole-byte identical. The linked executable's
`.text`, `.data`, `.rsrc`, and `.reloc` are identical. Its only two differing
bytes lie inside the PE timestamp at file offset 280 and the debug-directory
timestamp at offset 4,694,596. The comparison checks those exact fields and
preserves the whole before/after files. It does not normalize unexplained
differences away.

Every cohort retains the actual Core and App object census and complete object
archives. Each of the 1,988 ordinary Core library member payloads matches its
actual object in that cohort; the three archive/linker metadata members are
retained separately in the complete index. This avoids substituting a selected
subset for the full App census or treating an old library as the new build.

## Reproduction and limits

The finite bundle is under `local/cc12_game_canonical_free_defaults`. The report
pins its manifests, Source/build archives, whole dependencies, and proof scripts.

```powershell
python local/cc12_game_canonical_free_defaults/verify.py
python local/cc12_game_canonical_free_defaults/verify.py --original-worktree
```

The ordinary replay reads only its copied bundle and this packet's report,
recomputes the complete audit, and rejects external reads, writes, subprocesses,
network access, and dynamic-library loading after prerequisites are imported.
A separately copied physical relocation runs that same replay. The optional
original-worktree mode additionally checks actual Git object payloads and the
current 4,206-file working Source census. It is not needed for portable replay.

No new allocator, fallback forwarder, queue owner, production activation, Native
free-body analysis, broad exception-runtime claim, foreign-domain equivalence,
or game/runtime validation is provided. The accepted application-owner and
queue-binding frontiers remain open. The implementation is ready for the
integrator's independent Source review and normal build.
