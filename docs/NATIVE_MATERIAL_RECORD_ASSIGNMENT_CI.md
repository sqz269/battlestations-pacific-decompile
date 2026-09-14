# Native record assignment and copy adapters (CI)

These five complete functions are dependencies of the in-place paths in
`00B145E0`, the material diagnostics record-vector insertion routine. The
records are the actual300-byte storage consumed by `00B13180` assignment
and `00B13920` uninitialized copy; no typed vector is substituted.

| Entry | Inclusive end | Native bytes | Behavior |
| --- | --- | --- | --- |
| `00B13310` | `00B13347` | 56 | Decrement source/output before each backward assignment; return final output. |
| `00B13720` | `00B1377B` | 92 | Copy backwards, then calculate the returned head from entry-captured first/last/output. |
| `00B13F50` | `00B13F7A` | 43 | Forward first/last/output to the head adapter. |
| `00B13AD0` | `00B13AF8` | 41 | Assign one captured source into ascending destination records. |
| `00B14550` | `00B14574` | 37 | Forward first/last/output to actual uninitialized copy and return its result. |

The loops retain equality termination, wrapping32-bit cursor arithmetic,
and native capture order. Empty backward ranges return the output argument;
empty fill ranges do not consume the source argument. Assignment occurs
after both backward decrements, or before each forward increment. Assignment
failure propagates with no new cleanup. `00B13720` ignores the child's result
and retains the original signed division-by300 sequence: subtract captured
inputs, signed IMUL by`1B4E81B5h`, SAR5, sign correction, multiply by300 and
subtract from captured output. Its non-divisible/malformed range behavior is
not replaced by an unsigned quotient or a range check.

The source uses the existing concrete raw string-pool context and assignment
provider in `native_renderer_begin_frame`, and the recently reconstructed
`00B13920` provider in `native_material_record_ranges`. The latter retains
its own output argument slot, completed-prefix cleanup and C++ rethrow.

Original `00B13310`/`00B13720` have six cdecl stack words, of which only the
first three are consumed; `00B13F50`/`00B13AD0` have three cdecl arguments.
Source declarations have four cdecl arguments, adding the pool context and
omitting the unused iterator/tag padding. Original `00B14550` ignores its
ECX owner and pops three stack words; the source is stdcall with four stack
arguments and RET10h. Native parameter/result signatures and prior Ghidra
state are retained in the accompanying report and annotation record.

Original stack scratch-byte initialization is omitted only where all child
consumers of those words were verified unused. Original private stack-frame
aliasing, hardware SEH and drop-in binary ABI are outside these new source
interfaces. Build and generated-code review do not establish record execution,
complete vector insertion, material diagnostics, rendering or gameplay.
