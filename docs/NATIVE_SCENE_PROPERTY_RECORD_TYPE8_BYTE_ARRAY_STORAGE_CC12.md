# Native property-record Type8 byte-array storage, CC12

Source admission is **1 for the bounded current-provider raw retain/copy domain**.
The unchanged initializer `[008EF2F0,008EF360)` is 112 bytes/34 instructions;
104 literal bytes remain exact. Only two CALL operand DWORDs bind the current
allocation adapter and memcpy. The existing CPP/HPP remain unchanged. The provisional raw-storage name and
evidence are saved in the configured Ghidra project; the affected export is refreshed.

ECX supplies an actual fresh writable unowned 56-byte root; EDX is padding.
Stack words DATA/COUNT/full FLAG occur at `T+4/+8/+C`; `RET 12` returns the
root in full EAX. The predicate uses only the flag's low byte. COUNT is the
unscaled unsigned byte count. Successful positive, readable, disjoint inputs
are the qualified domain. Both branches set marker byte `+2C=1`, which does
not establish ownership. The body writes 29 bytes and preserves 27, including
owner `+30`. The copied pointer at `+20` is stored before memcpy.

The retained branch keeps the caller-managed input pointer, ECX=root and
EDX=input. Defined flags satisfy `flags & 8D5 = 44`, including defined AF0.
Copy-branch flags derive from the actual final `ADD ESP,16`: `(T-24)+16`.
Copied ECX/EDX provider residuals are observed and unasserted. DF0 is required
around the current CRT bindings. ES is captured; the probe does not assert ES
equality and the Source contract gives no blanket ES/FPU/MXCSR preservation.

## Complete code and sole-process evidence

Root generated three fresh TUs in `local/t8p3`, consuming 184 headers and seven
libraries. All 32 logical symbols/30 physical TU bodies, 287 logical relocation
checks over 281 physical operands, and six alias rechecks are validated.
**48 complete gated code spans** cover all 18 external helpers, normal import
thunks, cookie14, stack43, cold delete16 -> delete5 -> free6 and std thunks.
Alignment bytes remain raw PE context; proven helper alignment is excluded
from runtime helper spans. The named GS-failure frontier is unexpanded.
Cold/EH/OOM coverage is static evidence; those paths were not executed.

Root read all 1,456 instructions in the 5,715-byte Main, with 28 dominance pairs
and 78 failure guards. An independent peer reproduced the code, formal-target,
frame, provider and whole-Main review. The 191-byte/68-instruction raw caller
captures R, Q and T, all three dead argument words before PUSHFD, full registers,
canaries and guards. The 26-byte Source and 25-byte Original ordinary wrappers
are retained and statically reviewed, with no dynamic ordinary calls or mapped
EH handler. Root approval pinned all 22 artifacts before one process.

That process made four raw entries: Source retain, Original retain, Source
copy, Original copy. Full flag words were D6B23900/7C48E100/A53D6C80/2E917F02;
counts were 7/11/11/7. Opaque byte arrays include NULs; copying does not scan.
Four actual canonical 56-byte roots and two copied children of 11/7 bytes
give six explicit allocation/free pairs. All complete roots, both guarded
48-byte inputs, captures and live children are checked after every call.
Thirteen blobs are saved while live, then two children are freed before four
roots and Original RX is released. Borrowed pointers are never child-freed.

The independent recorded-only decoder passed all four full Capture140/root56
cases, both inputs and both children, fullword EAX/RET12/stack transport,
defined branch flags and disjointness. It only opened frozen artifacts.
The exact all-files seal includes all pycache, retained stops and helper logs:
**499 files**, SHA-256 `3bc2d040c041ffada7b3f77f8873763d61941b86f9d599ced9ec844265bd85b7`. Its Source0 summary is immutable
history preceding this external Source1 decision. All 25,438 prior pins and
34 exact prior families remained unchanged. No accepted helper, object, stage
or process was replayed.

## Provider, build and lifecycle boundary

Four current bindings are malloc/free/_callnewh from UCRT and memcpy from
VCRUNTIME140. Pre-root checks establish MEM_IMAGE/AllocationBase, mapped/held
NT paths, held-handle FileID/size, full physical SHA, physical/live PE tuple,
named export/GetProcAddress/IAT and adjusted raw prefixes compared with memory.
Inlined post-free checks repeat IAT, held-handle FileID/size, full mapped-file
SHA and adjusted-prefix equality. They do not repeat MEM_IMAGE, mapped NT path,
GetProcAddress or live PE headers. Serialized prefixes are expected relocated
raw bytes compared live, not a separately captured live-memory dump. The
decoder creates no current live provider/FileID attestation.

Main's Win32 build and all three existing checks passed with 4,041 actual
Source/header/CMake inputs. Root freshly rechecked every input: all remain
byte-identical to that build. Intervening commits were metadata-only. No new
tests were added. The earlier library-edge hold is closed for this bounded
standalone Source domain; its old seals and evidence remain preserved.
Independent consumer admissions are not changed by this decision.

Actual recursive release/publication still requires the established producer,
context, canonical allocator and lifetime contract; marker `+2C` alone does
not authorize freeing retained storage. Original private CRT/EH, cold/OOM/GS,
zero-size/failure/overlap, phase/vtable/class/whole-clone behavior, drop-in native
game ABI, startup and gameplay remain unadmitted.
