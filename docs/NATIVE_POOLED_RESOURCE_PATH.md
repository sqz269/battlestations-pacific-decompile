# Actual pooled resource-path owners

The complete `00BEE690` normalization owner and `00BEE780` copy-and-normalize
constructor now operate on actual eight-byte native string headers. They compose
the existing actual-header lowercase, resize, destruction, copy-construction and
substring bodies. They do not overlay the private fields of a `NativeString`, or
project ownership into `std::string`.

| Native body | Original ABI | Reconstructed interface |
| --- | --- | --- |
| `00BEE690..00BEE780` exclusive, 240 bytes | ECX actual header, plain RET; no stable EAX result | `void normalize_native_resource_path_header_00bee690(void*, NativeStringStorage&)` |
| `00BEE780..00BEE7FE` exclusive, 126 bytes | ECX output, EDX source, EAX output, plain RET | `void* copy_construct_native_resource_path_header_00bee780(void*, const void*, NativeStringStorage&)` |

The length DWORD is at `+00`, data pointer at `+04`. Header identity and all
allocation-visible reads concern those actual addresses. Neither API adds a
null-header guard, a signed maximum-length check, rollback, or old-output release.
These are new C++ interfaces with an explicit storage argument, not binary ABI
replacements. Names describe recovered behavior; no original symbols are claimed.

## Normalization chronology

`00BEE6AC` first calls full `004BCC00`. That helper captures the data pointer,
walks the current unsigned length, and converts only ASCII `A..Z`, including
bytes after an embedded NUL. The original byte stores are preserved by the
existing concrete helper in `src/native_string.cpp`.

The slash pass captures length once at `00BEE6B1`. It runs only for a signed
positive length, reloads current data for each byte, and changes `5Ch` to `2Fh`.
Afterward `00BEE6D4` reloads length. Leading trimming uses unsigned indices and a
captured data pointer; only byte `20h` counts as space. Trailing trimming starts
from wrapped `length-1` and continues only while its signed index is positive,
so it never examines index zero in that loop. Tabs and other whitespace remain.

`end-start+1` uses DWORD wrap. Full `00469840` constructs a separate stack result
with the current source, start, and count. Its preallocation source capture,
`strncpy` padding, actual `00426060` copy-construction, and its own temporary /
flagged-output cleanup are preserved by the concrete substring implementation.
For example, normalization lowercases the counted bytes after an embedded NUL,
but the later substring's `strncpy` can replace them with padding while retaining
the requested length. The all-spaces case reaches an empty substring normally.

Only after the substring returns does `00BEE71B` arm this owner's result cleanup.
The returned header pointer is compared with the actual input. If distinct,
`00BEE72C` resizes the input using the result length captured before allocation,
with preserve set. It then rereads the current result length as the copy guard.
For a nonzero guard it reads current input length, current input data, then
current result data, in that order (`00BEE736/38/3C`). The copy count is the input
length; it is not replaced by a cached or subsequently changed result length.

On ordinary return the owner captures current stack-result data at `00BEE749`,
disarms its cleanup at `00BEE750`, and, if data was nonnull, reads current result
length plus one before returning that block to storage. It leaves the result
header untouched. The supplied actual-header destruction body has the same
data-then-length capture and unchanged-header behavior. Its call is outside the
copyback exception region. `00BEE690` does not establish EAX as its input header:
EAX can retain a helper, copy, or pool-return value, so the public API returns void.

## Constructor chronology and unwind states

`00BEE7A1` captures output/source identity before both output fields are cleared
at `00BEE7AF/B1`. The identity branch follows those stores. Self-construction
therefore abandons the old buffer and still normalizes the now-empty header.
Distinct output is resized with the then-current source length. The subsequent
source-length guard and current output length / output data / source data reads
match the normalizer's inline copy ordering. There is no initial-copy cleanup:
an allocation failure can leave a callback-mutated partial output in place.

Only after the initial copy succeeds does `00BEE7DD` arm bit zero of the local
constructed flag. The constructor then calls the full normalizer. Successful
return explicitly restores EAX to the actual output pointer at `00BEE7EF`.
Normalization failure tests and clears the flag before destroying the current
output header; cleanup neither clears its fields nor restores prior storage.

| Owner | Native metadata | State transition and action |
| --- | --- | --- |
| Normalize | Handler `00CC75B8`, 36-byte FuncInfo `00E02168`, map `00E02160` | `0 -> -1`, funclet `00CC75B0` destroys current stack result at frame `-14h` |
| Construct | Handler `00CC75E9`, 36-byte FuncInfo `00E02194`, map `00E0218C` | `0 -> -1`, funclet `00CC75D0` tests/clears flag at frame `-14h`, destroys saved output at frame `-10h` if armed |

Both FuncInfo records have magic `19930522`, one state, null exception-specification
list, and EHFlags `1`. The normalizer is state `-1` throughout substring creation.
A substring failure therefore runs the substring helper's own cleanup and, for
the enclosing constructor, its armed output cleanup; it does not destroy this
normalizer's partially constructed result. A copyback failure instead destroys
the current completed result and then the enclosing constructed output.

The native normal-return getter/free sequence occurs after normalizer cleanup
is disarmed. A getter exception there skips result cleanup; an enclosing
constructor still cleans its output. `NativeStringStorage` already supplies the
pool and declares release `noexcept`, so this throwing-getter/free path is an
explicit host boundary. The native-only fixture below verifies the recovered
state ordering without claiming a corresponding throwing host-release behavior.

## Evidence and verification

The audit records 19 freshly checked initialized spans: both complete owners,
their complete funclets/handlers/maps/36-byte FuncInfo records, complete relevant
actual-string dependencies, and the selected external hook preimages. Each live
Ghidra query verified `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, Win32
x86 and the image base through the repository CLI. All selected bytes matched
the installed PE. This comparison does not establish whole-image identity.
Old names, stored prototypes, and comments are retained in the audit for the
integrator's append-only annotation update. This packet made no Ghidra mutations.

One private focused original-owner fixture passed 7,416 normalized words across
14 variants (103 events of 72 words). It compared allocations, releases, sizes,
normalized pointer roles, current source/output/result/substring-local fields,
and twelve bytes from each of five buffers. Observations include the pre-substring
tab / lowercase / embedded-NUL / slash bytes before padding overwrites them.
The native owners made 11 hooked memcpy calls and 13 actual substring compositions;
five original unwind funclet executions covered two normalizer result cleanups
and three constructor flag cleanups. A separate native-only pair produced 1,296
trace words and one further constructor funclet execution when the normal-return
getter threw; the disarmed normalizer never cleaned the result again.

The fixture loads all selected preimages from the installed binary before
patching its private sparse image. The two full owner control flows execute
original code, with only their EH-handler immediates redirected to in-image host
bridges. Eight checked pointer relocations preserve the original FuncInfo and
unwind-map relationships. Original unwind funclets execute through actual host
`__CxxFrameHandler3`. Existing concrete C++ lowercase, resize, destruction and
substring bodies serve the original owners' dependency calls, as well as the
new C++ owners. The substring body in turn uses actual copy-construction and
real host `strncpy`. Allocation callbacks use controlled fixed buffers; returns
record pointer/size instead of freeing. This is composition evidence, not native
pool or native CRT execution, general SEH equivalence, or game validation.

The fixture compiled as MSVC Win32 with `/std:c++17 /O2 /Oy- /EHsc /fp:strict
/MD /Gy /Gw /W4 /WX`. `scripts/build.ps1` passed with the new source and substring
source registered through a private CMake include. All eight existing native
seed spans matched, and both existing CTests passed (`reconstructed_math` and
`native_math_differential`). No tracked test, shared CMake registry, ledger, or
function export was modified. The source retains the existing zero-byte memcpy omission
and nonnull-allocation storage contract. Renderer container removal and any
change to the older `resource_path.cpp` value projection remain separate work.

Machine-readable bytes, hashes, interfaces, old annotations, fixture scope and
validation results are in `reports/native_pooled_resource_path_audit.json`.
