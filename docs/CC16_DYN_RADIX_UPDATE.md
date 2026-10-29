# Dyn variance sweep and radix rebuild

This packet recovers the complete `00C54DA0..00C55A55` body: 3254 bytes,
1001 listed instructions and twelve original calls. The descriptive public
name is `bsp::cc16_dyn_radix_update::rebuild_radix_pairs_00c54da0`.
It takes an actual raw manager, the caller's genuine `DynLiveCrt`, the actual
canonical memset feature cell `0109EEA4`, and the actual native double cell
`00D7A280`. No game object, game class, vtable, global, default provider or heap scratch
is created. Live `00D7A280` bytes `00 00 00 00 00 00 E0 3F` identify double 0.5;
the source still borrows the real cell rather than supplying a replacement.

Every live query/export used the verifying `bsp.py ghidra` client for project
`bsp`, `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86 LE 32-bit,
base `00400000`. The complete refreshed pseudocode and assembly were read.
Seven bounded live byte reads covered the full contiguous body, including
alignment bytes omitted from the instruction listing. Hashes are in the report.

## Instruction schedule and stack custody

The mixed SSE/x87 body uses explicit MSVC Win32 assembly, following the
existing `native_dyn_sap_processing.cpp` kernel/context pattern. Every original
instruction is address-marked in the source. All arithmetic, spills, repeated
field reads, signed/unsigned branches, register operands, stack slots, radix
stores and internal branch targets retain the native instruction schedule.
The only adaptations are dependency/context bridges, the borrowed constant
load, and removal of the extra context word on return.

Native ABI is ECX manager, no stack arguments, plain RET, with EBP/EBX/ESI/EDI
preserved. The private kernel adds one stack context argument and uses RET4;
the public interface is a new ordinary C++ function. Neither is a drop-in
original ABI adapter. Returned bits are observed final RDTSC EAX, without
asserting that native callers consume them as a semantic timing API. Both
original nonserializing RDTSC instructions remain.

The fixed frame remains `1040h` bytes through the actual MSVC `_alloca_probe`.
Three actual `_alloca_probe_16` calls allocate uint32-wrapping `count*8` bytes:
two object-entry buffers, then one endpoint-entry scratch buffer. The kernel
uses each returned ESP directly and retains all allocations until its original
frame epilogue. No heap allocation substitutes for these stack allocations.
The fixed 4096-byte interleaved histogram is at EBP-1040h. The actual target
compiler runtime symbols are reused; their vendor bodies receive no credit.
Context/call frames and compiler runtime identity qualify stack-overflow,
fault-location and exception-transport equivalence.

## Preserved phases

The initial count is manager+F4h plus manager+190h, wrapping at 32 bits.
`CVTSI2SS` converts its signed representation under ambient MXCSR. Object
rings at +ACh / +148h are walked to sentinels +B0h / +14Ch in that order.
Each object's reference size +34h is zeroed and its pointer stored in scratch.
The native x87 mean/second-moment accumulation retains every intermediate
float spill and the live half/count stack values across both walks. Largest
variance selection uses the original `FCOMIP`/JBE ordering; ties and unordered
comparisons retain the earlier selected axis. No control word is changed.

Only the collected-object count controls selected-axis key publication, using
FLD/FSTP before the native sortable-bit transform. The initial pool-count sum
controls the object radix sort and sweep. Those counts are not reconciled or
guarded. Four interleaved byte histograms become exclusive prefixes, then four
stable low-to-high byte passes alternate buffers and end in the original input
buffer. Equal keys retain their previous order; no comparison sort is used.

If pair count +58h is nonzero, the native +44h chain is followed to +48h,
spliced onto free head +34h, and its exact head/tail/count publication order is
retained. The selected-axis sweep caches the current first endpoint+4h, rereads
its first pointer for flags/add, and checks candidate bounds with FLD/FCOMIP.
The seven `JA` tests skip only ordered greater values; unordered comparisons
continue. At least one zero +1Ch flag and six full-AABB comparisons gate the real recovered
`cc16_dyn_storage::add_endpoint_pair_00c40a30` with the same manager/endpoints/CRT.

Pending +194h/+198h entries are then appended on axes 0,1,2 via the real
`cc16_dyn_sap_update::append_axis_endpoints_00c40900`. Each pointer is captured
once; data and unsigned live count reload for each iteration. Its final flag
store follows all three appends, and pending count clears only on normal
completion. This body does not substitute the other pending-dispatch routine.

The resulting axis-0 count +8h is captured once for all three axes. Per-entry
key publication reloads each axis data pointer and separately rereads tag bits.
These bound transfers use native `MOVSS`, preserving raw float bits without
the selected-object phase's x87 conversion. Axes 0,1,2 are independently sorted
by the same four stable radix passes, reusing the third stack buffer. Final
index publication reloads each axis pointer and tests its tag byte before
reading the tag dword, then stores the truncated 16-bit index. Other axis
counts are not independently checked. Invalid/native count, tag, ring and
buffer invariants remain unguarded; no rollback or catch is supplied.

## Bindings and static verification

Four memset calls use the existing qualified
`fill_native_crt_bytes_00bf79f0`, borrowing the same canonical feature cell.
It retains that provider's actual CPU/OS/DF and writable-extent preconditions.
Append/add bridges forward the same real objects and the same CRT. Their stack
shims preserve the original caller cleanup after removing only the inserted
context word. The bridge argument order and generated public wrapper were
inspected separately. No fabricated default or returning stub is involved.

Strict optimized MSVC Win32 C++17 object compilation passed with
`/permissive- /EHsc /fp:strict /O2 /W4 /WX`. A read-only COFF/Capstone comparison
checked all 1001 original listed instructions against the compiled kernel:
all unadapted instruction IDs, operand types/widths/registers/displacements,
all 97 internal branch sites, and all twelve call relocations matched. The
compiled kernel has 1011 instructions: eight context pushes and two constant
binding loads account for the ten additions. The RDTSC pair, SSE/x87 operands,
fixed-frame and aligned-stack probes are included in that comparison. All
twelve original native call rows passed `verify_report_calls.py`.

Ignored local evidence is `local/cc16_dyn_radix_update_object_audit.json`,
with the bounded comparison script and compiler listing beside it. The tracked
report records its result and hashes. These are static source/object/call-site
checks, not native differential or runtime proof. No tests, executables,
native/game launches or Ghidra mutations were added.

Root must register the source, perform the common Win32 link/build/checks,
annotate/save/refresh Ghidra, and bind genuine manager/CRT/canonical cells at
the actual caller (`00C5A850` is the live incoming body) before activation.
Original game ABI, FH/SEH/frame/fault identity, fixtures, provider behavior under
allocation failure, native differential and game behavior remain unproven.
The three vendor/compiler dependency bodies are excluded from in-house
reconstruction credit.
