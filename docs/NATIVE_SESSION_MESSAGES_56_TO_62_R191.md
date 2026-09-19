# Session messages 56–62 and numeric float arrays (R191)

## Scope and evidence

Reconstruct 47 complete normal bodies (2,681 bytes): the six lifecycle/codec
bodies for each of seven messages, two numeric-array wrappers, and three profile
slot-4 gates. The two factories remain dependency fragments. Names are descriptive
hypotheses, not recovered symbols. Evidence is from `C:/Users/sqz269/bsp.gpr`,
program `/battlestationspacific.exe`, with 13,677 live bytes matched to the original
PE, 150 ownership-checked direct edges, and 238 fixture relocations.

| Type | Constructor | Predicate | Writer | Reader | Destructor | Scalar delete | Profile | Size |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 56 | 008DC6A0 | 008DC6D0 | 008DC700 | 008DC750 | 008DC7A0 | 008DC820 | 00D16050 | 30h |
| 57 | 008DC840 | 008DC870 | 008DC8A0 | 008DC8E0 | 008DC980 | 008DC9E0 | 00D16064 | 24h |
| 58 | 008DCA00 | 008DCA30 | 008DCA60 | 008DCAA0 | 008DCB40 | 008DCBA0 | 00D16078 | 24h |
| 59 | 008DCBC0 | 008DCBF0 | 008DCC20 | 008DCC70 | 008DCCC0 | 008DCD20 | 00D1608C | 2Ch |
| 60 | 008DCD40 | 008DCD70 | 008DCDA0 | 008DCDF0 | 008DCE40 | 008DCEA0 | 00D160A0 | 2Ch |
| 61 | 008DCEC0 | 008DCEF0 | 008DCF20 | 008DCF60 | 008DCFA0 | 008DD020 | 00D160B4 | 2Ch |
| 62 | 008DD040 | 008DD070 | 008DD0A0 | 008DD0E0 | 008DD120 | 008DD1A0 | 00D160C8 | 2Ch |

## Construction, wire fields and lifetime

Each constructor calls `0075B430(type)`, sets delivery to 1 and installs its own
profile. It clears only owned-string length/pointer headers; other payload and
base padding remain untouched. All predicates compare the full DWORD query with
the fixed type, 54, or the zero-extended mutable type byte. Fixed matches precede
the object dereference.

Writers use ECX for the record and one stack argument for the raw 10h cursor;
readers receive the 18h wrapper whose cursor begins at +4. Both return with RET 4.
They write/read the type byte, owned strings in ascending header order, then:

| Types | Owned headers | Remaining wire fields |
| --- | --- | --- |
| 56 | 18h and 20h | DWORD at 28h, unsigned 2 bits; Boolean at 2Ch |
| 57/58 | 18h | WORD at 20h, unsigned 12 bits |
| 59/60 | 18h | Three numeric floats at 20h/24h/28h |
| 61/62 | 18h and 20h | Boolean at 28h |

The 12-bit reader clears the upper four WORD bits while retaining padding at
22h/23h. String operations reuse `00429AC0`/`00429F20` and the actual raw pool
providers. The float callers perform x87 FLD/FSTP on the actual `00D7A248` scale,
then call the array wrapper with count 3, zero flag 0, signed flag 1 and width 32.
Complete writer/reader instruction shapes match within each of 57/58, 59/60 and
61/62; these are distinct native bodies with shared recovered contracts.

Destructors stamp their own profile, destroy headers in reverse order, and stamp
root profile `00CE4974` through the seven-byte `008DADB0` support routine. Headers
retain their values after resource release. Source nested `__try/__finally`
preserves remaining cleanup when an earlier release escapes. Scalar destructors
always run cleanup, free through actual `00BF65AC` when flags bit 0 is set, return
the captured address and use RET 4. All seven truncated scalar ADD ESP,4 gaps
were repaired; final flow checks show no gaps.

| Type | EH handler | FuncInfo | Unwind map | Root action | Additional string action |
| --- | --- | --- | --- | --- | --- |
| 56 | 00CA3463 | 00DD576C | 00DD575C | 00CA3450 | 00CA3458 (+18h) |
| 57 | 00CA3478 | 00DD5798 | 00DD5790 | 00CA3470 | — |
| 58 | 00CA3498 | 00DD57C4 | 00DD57BC | 00CA3490 | — |
| 59 | 00CA34B8 | 00DD57F0 | 00DD57E8 | 00CA34B0 | — |
| 60 | 00CA34D8 | 00DD581C | 00DD5814 | 00CA34D0 | — |
| 61 | 00CA3503 | 00DD5850 | 00DD5840 | 00CA34F0 | 00CA34F8 (+18h) |
| 62 | 00CA3523 | 00DD5884 | 00DD5874 | 00CA3510 | 00CA3518 (+18h) |

Metadata/actions match live and PE bytes. Ten-byte handlers tail-call CRT
`00BF6B43`; their missing stored function ownership is recorded as auxiliary raw
evidence. Original FH3 exception dispatch is not established by source cleanup.

## Numeric arrays and profile gates

`00429790` (71 bytes) writes numeric float arrays; `004294F0` (73 bytes) reads
them. Native ECX is the cursor, followed by six stack arguments: pointer, unsigned
count, zero flag, signed flag, captured scale and width; RET 18h. Each iteration
x87-loads/stores the captured scale. The writer then x87-loads/stores the current
element and calls `004295C0`; the reader calls `004293F0` at the current address.
Pointer/index arithmetic wraps at 32 bits. Zero count accesses neither data nor
scale through x87. No signed-count clamp or new bounds checks are introduced.
Iteration reads remain observable under overlap; the writer does not snapshot
the complete input array. Existing numeric codecs and CRT conversion are reused.

`008DAE20` ignores native ECX, reloads current-game pointer `00E188A8`, and returns
whether the full DWORD at +21A4h is nonzero. Its higher-level meaning is unproved.
`008DC920` and `008DCAE0` first perform that check, returning false before any
record/table access when it is zero. Otherwise they zero-extend the full WORD
at +20h, compare it as signed against `00F89A10`, subtract low base `00F89A0C` or
high base `00F89A60`, and use low table `00F89A54` or high table `00F89AA8`.
The result tests the DWORD at wrapping `table + (index << 4) + 0Ch` for nonzero.
ID zero has no special null rule. Existing `ObjectHandleTables` supplies the
actual cell references; resolver `006AD080` has a different zero-ID contract and
is not reused. Source profiles borrow context beyond the five native slots;
these adapters do not prove whole binary ABI compatibility.

## Factory dependency boundary

Factory `00768530` indexes table `0076A298` with unsigned `(type_byte - 1)` up to
E8h. All eight slots at `0076A370..0076A38F` for types 55–62 point to `00768C5E`,
whose raw CALL targets `008E1530` with the selector still in ECX. That CALL has no
stored Ghidra function membership and is recorded separately from the 150
verified owned edges.

The complete 454-byte delegated creator subtracts 55 from the full DWORD ECX
selector and rejects unsigned results above 7. Table `008E16F8` dispatches eight
allocations: 34h,30h,24h,24h,2Ch,2Ch,2Ch,2Ch. The seven calls to the reconstructed
constructors belong to this stored function. Native allocation receives one
size argument; the pseudocode's extra selector argument is spurious. Per-branch
EH guards free raw allocation when construction escapes. This creator is only
a dependency fragment in source: type 55's nested collections remain unresolved.

## Validation and fixture corrections

Strict MSVC Win32 build and all three existing CTests pass. The final local
fixture is `local/session_messages56_to62_r191/fixture4`, with 6,088 original/source
pairs and 37,215,976 matching observation bytes:

- 196 constructor cases; 4,704 record cases; 392 process/table gates;
  768 numeric-array cases; 28 freeing scalar cases.
- All eight cursor alignments, string reuse modes, embedded NUL/fallback cases,
  buffer guards and pool state; signed owner indices and retained padding.
- Numeric edge patterns including signed zero, denormals, infinities and NaNs;
  four rounding modes, both conversion selectors and x87/MXCSR status flags.
- Handle IDs through 65535, signed splits, wrapping table offsets, null/non-null
  entries and zero-process-flag short-circuit with invalid record/table inputs.
- Ten source-only cleanup fault cases observe remaining reverse cleanup, root
  stamping and pool state. They do not test original FH3 dispatch.

Three fixture errors were corrected while production source and library stayed
unchanged (library SHA256 `09fbd5f250fefc781e3083e21b0c73d9b4095b271998b574e88638e23933d7fd`):

1. Initial loader omitted actual root-stamp support `008DADB0`; its verified seven
   bytes were added to the fixture.
2. Table backing did not cover the full valid 16-bit handle domain; owned arrays
   were enlarged to cover the exercised positive and negative indices.
3. Cloned data shadowed mutable CRT conversion selector `0109EEA4`, so original
   code remained on mode 0 while source used mode 1. Live globals now resolve
   before copied regions. A focused 12-pair scalar/array diagnostic confirms
   matching x87/MXCSR behavior in both modes. Wire bytes had already matched.

Failed fixtures and corrected diagnostics are retained in the local evidence
archive. Seventeen missing normal functions were defined. Names/comments retain
prior values, are applied under the Ghidra write lock, saved, read back and
re-exported; exact receipts and archive hashes are in the companion report.

## Follow-up packets and limits

Type 55 (`008E0170`, profile `00D16124`) owns a list and vector. Its writer
`008E0600`, reader `008E1930`, resize `008E0EC0`, element constructor `008E08F0`
and growth `008E17C0` require separate ownership and full register/assembly
evidence. Sentinel allocator `008DB560` appears to be STL support; establish
its contract without porting library implementation as game code. The reader's
register ABI and nested ownership must be resolved before binding the creator.

Full factory, packet recorder, network workers, startup composition and gameplay
remain open. Fixtures do not establish arbitrary alias safety, concurrency,
allocation failure behavior, original FH3 dispatch or whole executable ABI.
