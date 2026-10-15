# Registry and grid-storage reconstruction

This batch reconstructs thirteen complete owned routines and corrects one
allocator annotation. All sources are registered in `bsp_core`; the MSVC Win32
build and all three existing CTest suites pass. Primary and independent worker
reviews found no correctness blockers. All 51 direct report rows and 15
markdown call/jump claims pass.

| Packet | Native entries | Coverage and execution evidence |
| --- | --- | --- |
| Registry-list teardown | `0041D710`, `0041E290` | Complete source schedules; build checked |
| Settings-vector cleanup | `0041F950` | Complete adjusted-receiver schedule; build checked |
| Registry node producers/constructor | `0041BFD0`, `0041C1F0`, `0041E270` | Complete source schedules; build checked |
| Byte-range primitives | `0041F450`, `0041C630`, `0041C650`, `0041E630` | Complete source; exercised through the linked Source fixture |
| Grid-vector resize/insert/erase | `004219A0`, `00420DD0`, `0041D7D0` | Complete owned kernels; linked Source fixture and static byte comparison |

The actual registered grid-vector object matches all 668 original owned bytes
after masking only twenty CALL rel32 operands redirected to reconstructed
services. The captured original ranges also match the installed executable
byte for byte. This static comparison does not establish provider identity,
Native ABI compatibility or original-game execution.

One ignored Source fixture was independently compiled against the integration
headers and linked to the actual registered archive, using genuine current
CRT providers. It passes growth, both spare-capacity tail branches, overlapping
fill/move cases, checked erase and shrink, output-iterator/header overlap,
length_error, and a returning thread-local invalid handler restored afterward.
Its ordinary count-zero read is proven by the compiled instruction schedule,
not claimed as observable at runtime. No committed test target was added.
The existing native math differential suite covers its own established seeds.

Existing correct library names are retained. The old `0041E290` Clear label is
corrected to Tidy because its target frees the sentinel. The `00418540`
annotation is corrected: zero calls `operator_new(0)`, rather than returning a
literal null. Prior names/comments and the allocator's old evidence were
recorded before edits. The node producer's inclusive span includes the bytes
through `0041C222`. All affected Ghidra names/comments are saved and standard
exports refreshed.

[The batch receipt](../reports/goal15b_reconstruction_batch.json) records source
commits, checks, artifact hashes and local evidence paths. Current Source
allocator, exception and handler identity remain qualified. The six list and
settings routines were not fixture-executed. Native production binding,
original ABI compatibility and gameplay validation remain unproven. The full
reconstruction goal remains active; suggested next candidates require a fresh
readiness/ownership check.
