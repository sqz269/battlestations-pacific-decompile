# Raw material diagnostics draw recording (CN)

B16F80..B171B5 is the 566-byte diagnostics operation called by B43410's
material pass. It uses the actual diagnostics object's enable byte at +681h,
grouping string header at +684h and 300-byte record vector at +68Ch. The source
composes the existing raw record/vector, effect-name, string and pool providers;
it introduces no replacement container or abstract diagnostic callback.

The original entry has ECX diagnostics, six stack words (effect, mode,
primitives, vertices, vertex-shader registers, pixel-shader registers), and
RET18h. The source fastcall entry adds the actual raw pool context in EDX.
Its naked adapter preserves those six public argument slots for current reads
after calls. Private stack spills, compiler EH and CRT identities remain source
interfaces rather than original binary replacements. The descriptive name is
a behavioral hypothesis, not a recovered symbol.

## Search and creation

The disabled byte returns before inspecting the vector or effect. Enabled
search captures the initial begin pointer, applies the returning invalid-
parameter checks, and reads the current end/begin at each original check.
Iterator increments remain unchecked wrapping additions of 300 bytes. The
native self-comparison of the vector owner is always equal, so its impossible
error call has no source branch. Every other check uses the concrete source
CRT invalid-parameter service and may return.

For each record, the source reads the current effect argument and compares it
with the record's pointer. Equal effects compare the record's +11Ch string with
the diagnostics' current grouping header using the existing case-insensitive
raw-header provider. Search continues after a match; the last matching record
wins. Captured iterator positions and fresh vector bounds follow the assembly.

If none matches, B106C0 initializes a temporary record. The source arms its
cleanup, appends it through B15610, then derives the last record from the
captured current end and performs the original returning bounds checks.
The new record is already in the vector when its effect pointer and names are
populated. First it assigns the grouping header to record+11Ch. It then reloads
the record's effect, gets its actual name header through B172D0, concatenates
group and effect name into a temporary header, and assigns that result to
record+124h. The two native inline assignments exactly follow the existing raw
B425F40 provider's identity/resize/current-length/data-copy schedule. Its concrete
memmove handles valid overlap and omits zero-byte calls after the header loads.

## Temporary lifetime and counters

Handler CBC4C6 references FuncInfo DF4714 and the two-entry unwind map DF4704.
There are no catch blocks. State 0 destroys the completed record temporary via
CBC4B0/B10740; state 1 first destroys the concatenated header via CBC4BB/41DD20,
then returns to state 0. The source arms each cleanup only after its constructor
returns. On normal cleanup it captures concatenated data before disarming state
1, reads current length before resolving the actual pool, then returns that
captured block. State 0 is disarmed before normal record destruction. Normal
cleanup failures propagate without retry; an escaping cleanup failure during
C++ unwinding terminates. No hardware SEH/private FH3 identity is claimed.

After search/creation and temporary cleanup, a small Win32 assembly helper
performs the five original DWORD read/modify/write additions. It captures mode
once and interleaves current argument reads with updates to +3Ch primitives,
+74h vertices, +4h draws, +ACh vertex-shader registers and +E4h pixel-shader
registers. Mode is not clamped; index/address and counter arithmetic wrap at
32 bits. Prior effects remain if a source provider throws.

## Evidence boundary

The report pins the complete function, both unwind actions, handler and EH data
against the installed PE and live bsp.gpr program. It records original direct
calls, concrete provider hashes, generated-code review, exact committed build
and any focused source probe separately. Existing math CTests do not establish
diagnostics behavior. B42350 system/material constants and the encompassing
render-pass, geometry and submission paths still require raw composition.
