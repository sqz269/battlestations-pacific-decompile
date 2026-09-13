# Native shader constant header

This packet emits system declarations into the actual `NativeMaterialProgramBuilderStorage`
string at `+4C`. It consumes the existing `NativeSystemConstantRegistryStorage` published at
`0108FE94` and its same `NativeCompiledShaderConstantStorage` rows. No semantic builder,
registry copy, string pool, register-limit snapshot or private format buffer is introduced.
Names are descriptive hypotheses, not recovered symbols.

| Routine | Original ABI | Coverage |
| --- | --- | --- |
| `B38FF0` header | ECX builder; stacked flag low byte; `RET 4` at `B39060` | complete normal body |
| `B38C60` declaration | ECX builder; stacked record/register DWORD; `RET 8` at `B38FDF` | complete normal body |
| `B35110` line formatter | cdecl destination, format, variadic values; `RET` at `B35267` | complete normal body, host CRT formatting |
| `5F1840` decimal constructor | ECX fresh string; stacked signed DWORD; EAX this; `RET 4` at `5F1922` | complete normal body, host CRT formatting |
| `B5B840` second dimension | ECX record; EAX current DWORD `+08`; `RET` | complete |
| `B5B850` first dimension | ECX record; EAX current DWORD `+0C`; `RET` | complete |
| `B5B860` array count | ECX record; EAX current DWORD `+10`; `RET` | complete |
| `B5B890` list header | ECX registry; EAX actual registry `+04`; `RET` | complete |

The existing `native_shader_constant_name_00b5b820` in `native_material_parameters.cpp`
supplies the actual `+14` name header. Its four original bytes are included in the comparison
fixture, but this packet adds no second implementation or replacement ledger record.
The record constructor `B5BBC0` establishes the first/second/array layout and the name at
`+14/+18`; builder constructor `B354D0` establishes the actual output header at `+4C`.

`B38FF0` loads `0108FE94` once and retains the returned list-header address. It captures its
initial data pointer, then after every declaration computes a fresh end from CURRENT count
and CURRENT data. The old cursor advances by `20h`; a replaced base is not adopted as a new
cursor. The unsigned register cursor starts at zero, checks the CURRENT `E13078` before each
row, and advances by CURRENT second dimension times CURRENT array count, with DWORD wrap.
Only its start is checked against the limit. A false low flag byte selects `FFFFFFFF`.
The formatter independently suppresses any register DWORD whose signed interpretation is
negative. Both known header callers, `B391B8` in vertex generation and `B3992B` in pixel
generation, push `1`; the public helper retains the actual low-byte contract.

`B38C60` builds a register suffix only for a nonnegative signed register. It constructs and
releases separate prefix, decimal and closing-parenthesis strings, using the native copy
and append schedules. EBX's suffix-data preimage is refreshed after each resize and retained
across temporary releases. Final release uses that captured pointer and CURRENT suffix length.
Its six type branches select matrix, vector, scalar and optional array using unsigned `>1`;
dimensions and array counts are formatted as signed `%i`. Getter reads used as formatting
arguments occur again after branch selection. Name data is a live pointer to the actual
record header; null names/suffixes use the borrowed `0108D6F2` empty byte.
The six `B35110` caller cleanups are `1Ch`, `18h`, `18h`, `14h`, `14h`, and `10h`, including
destination and format. The output is appended without clearing existing text.

`5F1840` first clears the destination header and formats signed decimal into its 52-byte local
buffer. It allocates a separate actual temporary string, captures its pointer and length,
copies text including NUL, then resizes and copies to the destination. The captured temporary
pointer/length control its final release. Its second caller `5F6C46` passes current global
`F88994`; no nonnegative-only contract is inferred from the shader call.

`B35110` calls CRT `C03CEE` into the SAME supplied `0108D6F8` scratch buffer, scans its current
text, allocates a pooled temporary, captures that temporary's current data/length, and copies
the scratch text AFTER allocation. A callback that overwrites scratch during allocation is
therefore observable. Appending captures the old destination length before resize and copies
from the retained temporary afterward. It releases that captured pointer/length, creates a
second pooled `"\n"` string, appends it, and releases it separately. It is a genuine cdecl
variadic helper: the eleven known caller bodies use varying argument counts. No fixed-arity
host method is inferred from a single caller.

The explicit runtime boundaries are the established `NativeStringStorage` bridge for
`419CC0/BD1120/BD1510`, existing actual-header `41DD40/41E870`, overlap-capable CRT copying
for `BF7680`, and host CRT `sprintf/vsprintf` for `BF7A6A/C03CEE`. The latter routines keep
their correct library names. No private CRT or FH3 implementation is ported. Accessible,
sufficient scratch/string storage and valid native format arguments are required; this
does not claim old/new CRT parity for arbitrary locale-specific or unsupported formats.
The shared scratch has the original reentry/concurrency restriction.

Every entry uses persistent `NativeShaderConstantHeaderOperation` storage. Actual temporary
headers, captured allocation preimages, source record/list/builder, cursor, function and
native site survive a borrowed failure. Failed or running frames terminate on destruction;
replay is rejected before native side effects. The caller must keep the referenced raw
owners, inputs, context and strings alive and exclude their destruction while a frame is
running/failed. These external owner-admission duties are not a new embedded reference count
or native terminal interception. No native rollback or FH3 cleanup is claimed. The explicit
diagnostic acknowledgement is permitted only after external cleanup of retained acquisitions.

The report pins all nine original bodies, call sites and exact artifact hashes. Normal-path
original instructions run with the established resize/string-pool and CRT bridges. The single
focused fixture uses the production 52-row registry and actual pooled-string owner in the SAME
singleton lifetime domain. It compares exact generated text and allocation/release traces,
existing builder text and untouched fields, low-byte flag handling, all six declaration shapes,
signed dimensions/counts, wrapped cursor arithmetic, live array/global changes, and a retained
formatting-allocation failure. Original FH3 execution is excluded from that comparison.

This provides the native constant-header dependency, not the complete vertex/pixel source
generators. Compilation, compiled-bytecode identity, shader execution, rendering, original
binary ABI replacement and gameplay are not established. Current build and fixture status
are recorded in `reports/native_shader_constant_header.json`.
