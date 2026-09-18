# Actual language catalog storage and VFS inputs (R162)

## Result

native_language_catalog_storage.hpp/.cpp supplies seven complete normal entry
schedules needed by the actual 008D7BC0 language-table producer. The four
strings in each 20h row remain actual pooled 8h headers; the catalog is an
actual pointer/count/capacity vector, as used by globals F88974/F88978/F8897C.
The existing projected language catalog is unchanged. The ordinary raw
producer and profile-hints owner remain integration dependencies.

| Entry | Bytes | Original contract |
| --- | ---: | --- |
| 008D47F0 | 35 | ECX pointer-to-scanner, RET; destroy/free captured scanner, then clear holder |
| 008D4F50 | 186 | ECX 20h row, RET; release four strings from +18 down to +0 |
| 008D5890 | 298 | ECX destination row, stack source, EAX destination, RET 4 |
| 008D59C0 | 221 | ECX 0Ch vector, signed requested capacity on stack, RET 4 |
| 008D6AF0 | 111 | ECX vector, source row on stack, RET 4 |
| 00553C80 | 73 | ECX string header, prefix/unsigned position on stack, raw EAX, RET 8 |
| 00886280 | 227 | ECX manager, output/directory/extension/flags on stack, EAX output, RET 10h |

## Ownership and ordering

Row construction zeroes each destination header immediately before its copy.
Identity construction therefore abandons all four original buffers. For a
nonidentity member, it captures requested length, resizes through the actual
pool, then tests the **current** source length and copies the **current**
destination length. It does not pre-capture all four source strings. The
ascending member order matters when row storage overlaps.

Row destruction visits members in descending order. Each nonnull data pointer
is captured before its current length plus one. Every return resolves the
actual pool getter; headers remain dead rather than being cleared.

Reserve clamps signed requests to at least one and grows only. It allocates
the wrapped requested count times 20h, copies while rechecking current count
and backing, destroys current old rows in ascending order, and frees the
current backing. Only after that callback returns does it publish the captured
new allocation and requested capacity. It does not restore or reset count.
Append grows only when count equals capacity, using wrapped signed doubling
and the native clamp, then calculates its slot from current fields. A zero
calculated slot skips construction but still increments count. A source row
inside a backing freed by real growth is outside the valid lifetime domain.

The scanner-holder helper captures the scanner once, delegates its full R161
raw destructor, frees that captured pointer, then clears the holder. The
holder's current value after either callback does not select a second owner.

Prefix matching preserves the full observed EAX contract: null data, null
prefix, or unsigned position beyond length returns the data pointer with AL
cleared. Once the scan path is admitted, its return is exactly zero or one.
The counted length gates only the starting position; the comparison itself
walks NUL-terminated strings.

The VFS wrapper allocates an actual 10h sentinel, writes output +4/+8 while
preserving +0, then clones and normalizes a raw directory temporary. Its
concrete default dispatch calls existing enumerate_native_vfs_resources_00bdd990
with the actual manager and list. The directory temporary is returned to the
same pool afterward. Supplied raw and owning-pool contexts must share the
actual publication cells. No standard-string/vector substitution is introduced.

Caller-retained operations expose partial row/vector/scanner or temporary-list
ownership after an escaping source callback and reject replay. A caller must
resolve those allocations before acknowledging cleanup. These records are an
explicit source failure contract; they do not implement the original FH3/SEH
handlers or reproduce the native private stack.

## Listing repairs and validation

The configured BSP Ghidra project/program and installed PE were checked.
Eight live spans match the PE: 1151 code bytes plus the 12-byte catalog header.
Two call-site no-return gaps concealed important stores: nine bytes at
008D4807 clear the scanner holder; ten bytes at 008D5A80 publish the new vector
backing/capacity. Both were restored under the Ghidra write lock, saved and
exported; the snapshot was forced to refresh the call graph. The remaining
three-byte gap is unreachable alignment after an unconditional JMP.

Strict MSVC Win32 /MD /W4 /WX /fp:strict and all three existing CTests pass.
One focused copied-native/source harness has 15 paired groups and 1897 matching
observed bytes: three row cases, seven reserve/append cases, one eight-input
prefix group, two scanner-holder cases and two enumeration-wrapper cases.
It compares live row strings, normalized header ownership, actual pool return
order through reclaimed-buffer identity, vector fields and allocation/free
traces, raw prefix EAX, scanner clear/free, normalized directories and actual
list contents. A free callback mutates vector fields before publication; a
scanner free callback replaces the holder before its final clear.

Both lanes use actual pooled-string services, existing full path normalization
and sentinel/node/list storage. Native helper edges are relocated to those
services and the host CRT. The scanner destructor is the already-compared R161
source on both lanes. BDD990 is a controlled enumeration callback in this
fixture; the new production default calls its existing actual VFS implementation.
A source-only enumeration exception verifies retained temporary/sentinel
ownership, replay rejection, manual cleanup and the actual final pool drain.

No original ABI, FH3/SEH, original CRT internals, asynchronous mutation, hardware
fault boundaries, dangling row source during growth, whole-application
admission or gameplay parity is claimed. The mounted-file R161 diagnostic
remains separate from these checks. Exact hashes, saved annotations, call-site
checks and immutable artifacts are in
reports/native_language_catalog_storage_r162.json.
