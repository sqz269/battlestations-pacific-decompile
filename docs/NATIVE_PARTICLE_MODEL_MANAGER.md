# Actual particle model manager lifetime

This reconstructs 13 complete functions over the existing 34h manager at
F8C274, previously named FoliageGroupManager. These descriptive names are
hypotheses. It supplies concrete weak model registration to AF74A0 and
removal to AF6C50 without another owner, registry or reference count.

The actual layout is table00, two 0Ch pointer/count/capacity arrays at04 and10,
untouched words1C/20, zero words24/28, captured D7A24C bits at2C, and an actual
separately allocated1Ch tracked critical section pointer at30. Construction
preserves the allocation preimage at1C/20 and does not publish30 until the real
BD1860 allocator/InitializeCriticalSection returns.

| Native function | Complete span (exclusive end) | Behavior |
|---|---|---|
| AF0630 | AF0630..AF068F | Signed minimum1 reserve, wrapped capacity*4, BF55BE -> BF681B |
| AF07E0 | AF07E0..AF0819 | Append through pointer argument, loaded after growth |
| AF0900 | AF0900..AF0950 | Signed resize; new cells zero, no pointee destruction |
| AF0A60 | AF0A60..AF0AC7 | First-match remove, swap last, preserve stale last cell |
| AF0AF0 | AF0AF0..AF0B07 | Resize0/free current backing; retain stale pointer/capacity |
| AF0950 | AF0950..AF098A | Append raw model value to manager04 |
| AF0AE0 | AF0AE0..AF0AF0 | Remove model from manager04; return AL |
| AF06A0 | AF06A0..AF0731 | Base table, captured lifetime section, publish/register |
| AF0740 | AF0740..AF07D9 | Current-global unregister, unconditional clear, base table |
| AF0870 | AF0870..AF088E | Base destroy/free iff flags bit0 |
| AF0B10 | AF0B10..AF0B82 | Derived field initialization and real section creation |
| AF0B90 | AF0B90..AF0C41 | Section drain/free, second array, first array, base |
| AF1080 | AF1080..AF109E | Derived destroy/free iff flags bit0 |

All original functions use ECX for actual owner/header. Array operations and
scalar deletion take one stack DWORD and RET4; constructors/destructors and
member cleanup RET. Constructors/scalar deletion return the original pointer.
The new C++ interfaces add explicit application lifetime/publication access.

Base construction and destruction capture the first 415350 result's section,
enter its real OS lock and increment its physical counter, then call415350
again before loading the current F8C274 for BD0C30/BCFCA0. Unlock uses the
captured section even if publication changed. FH3 base unwind writes CE3818;
registration failure retains publication. Derived constructor unwind destroys
the second array, first array and base, following DF269C's three states.

AF0B90 captures30, drains positive signed recursion depth, deletes the real
section and frees it. The existing 41CC80 implementation clears its argument
slot, so the adapter passes a local captured slot: AF0B90 itself leaves the
published30 pointer stale. It also preserves freed array pointers/capacities.
The two array members own backing only; model references are never adjusted.

23 complete live-Ghidra/installed-PE spans match, including all13 bodies,
tables, constants, FH3 maps/funclets, the singleton base leaf and BF55BE thunk.
The focused original-byte fixture executes all13 functions with the existing
real shared singleton manager, CRT allocation/free and Win32 sections. Four
paired manager trajectories compare all52 bytes after normalizing allocation
addresses and exercise duplicates/nulls, swap-last, growth/shrink, nullable
singleton locks, recursive section drain and stale fields. Eight scalar pairs
cover base/derived deletion with flags0/2/3/100; member cleanup covers minimum1.

Original FH3 exceptions, hardware faults, concurrency and gameplay were not
executed. C++ cleanup uses established host CRT/exception boundaries. Pointer
arithmetic and allocation sizes preserve native DWORD wrapping; malformed
backing is outside the valid interface domain. Whole-batch validation and
saved-analysis changes are recorded in reports/native_particle_model_manager.json.

## Follow-up packets

AF0B10's existing startup sequence still exposes a semantic host surface.
Bind its actual allocation/lifetime dispatcher to this same storage before
claiming an end-to-end startup path. AF0CC0 onward manager traversal/update
and actual renderer stream factories remain separate reconstruction work.

## AL saved-analysis and combined-build integration

All four AL modules are registered in bsp_core. The standard Win32 build and
both seeded CTests passed. The report records saved name/signature preimages,
old-comment preservation, full body-range readback and the current-library
replay of bounded fixtures. Returning-free gaps and missing function definitions
are repaired and saved. Earlier worker pending notes describe their original
snapshot. Full constructor/teardown coverage, native EH compatibility and
gameplay remain bounded as documented above.
