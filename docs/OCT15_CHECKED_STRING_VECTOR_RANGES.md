# Native checked string-vector range assignment

Packet `oct15_checked_string_vector_ranges` covers `006A29C0` and `006A2A00`.
Both Source entries compose the accepted owning assignment service directly.
There is no host vector,
callback, borrowed uninitialized assignment, proxy bridge or replacement
assignment implementation. Existing descriptive library-role names
`STL_CopyStringVectors_006a29c0` and `STL_FillStringVectors_006a2a00` are retained;
they are hypotheses, not recovered symbols. Earlier claims of the same body as
`006A2200` / `006A2240` were not used to establish these contracts.

| Address | Inclusive body | Original machine interface | Source result |
| --- | --- | --- | --- |
| `006A29C0` | `006A29C0..006A29F8`, 57 bytes / 27 instructions | cdecl(first,last,destination), three physical stack words, plain RET | cached wrapped destination end |
| `006A2A00` | `006A2A00..006A2A25`, 38 bytes / 18 instructions | cdecl(first,last,value), three physical stack words, plain RET | void; native ambient EAX qualified below |

Live queries verified project `bsp`, configured at `C:/Users/sqz269/bsp.gpr`,
program `/battlestationspacific.exe`, language and image base before analysis.
Complete pseudocode was followed by both complete assembly listings. Each has
one assignment site, no listing gaps, and an explicit plain RET; no
no-return or flow repair was needed. The complete original `00506C80` listing
and accepted Source/header were also read. The installed PE spans independently
confirm the two call targets and final RET bytes. Ghidra and the game were not
modified. All names and historical evidence are retained, with corrections
appended in the owned ledger records.

These are ranges of actual **16-byte live checked-vector headers**, each with
opaque/proxy+0, begin+4, end+8 and capacity end+C. Their elements are actual
8-byte pooled-string headers. `00506C80` reads +4/+8/+C and uses SAR3 for inner
string counts. Accepted actual producer `00432090` clears/publishes +4/+8/+C
with the same 16-byte header contract. Original caller `006A6680` uses
SAR4/SHL4 for its outer range and passes header addresses to `006A29C0`.
Insert caller `006A5C50` passes its temporary at [EBP-28h] and a destination
range to fill; its full construction/insert behavior is outside this packet.
No `NativeStringVectorStorage` 0Ch overlay or object is
invented. Destinations must already be valid owning headers for actual
assignment; these wrappers do not initialize raw backing or make borrowed
storage safely ownable.

The callee retains its actual self-assignment, empty/clear, shrink, grow and
replacement behavior, including element assignment/construction/destruction,
current-begin free, fresh header reads, pool publications and exception effects.
The explicit `NativeStringRawPoolContext` binds the application's actual pool
publication `01090AA8`, shutdown/return gate `01090AA4`, and manager publication
`01090AA0` through the accepted service. It owns no storage and adds no callback.
Actual inner-string/pool/backing ownership and the current-CRT allocation/free
domain must satisfy that service's contract. Its diagnostic, zero-copy and
Source exception limits remain inherited; see
[assignment evidence](OCT15_STRING_VECTOR_ASSIGNMENT.md).

Copy captures first, last and destination. Before any assignment it caches
`destination + ((SAR4(last-first)) << 4)` with native DWORD subtraction,
arithmetic right shift, left shift and addition. All operations wrap as 32-bit
words; Source expresses SAR's sign extension with unsigned operations and uses
no C++ pointer subtraction or out-of-array pointer arithmetic. For every DWORD
delta the SAR4/SHL4 pair is also equivalent to masking with `FFFFFFF0h`.
On nonempty entry it captures `destination-first`, then repeatedly assigns
source cursor to destination cursor-plus-delta, advances source by10h, and
compares to captured last. Child return values do not replace the cached end.
Empty returns destination without touching any pointed storage.

Fill captures the value **pointer**, not the vector contents. Every iteration
calls the real assignment service on the next10h-byte destination using that
same pointer, so the service reads the value's current header and records.
Value may alias an earlier/current/later destination when every executed child
access and ownership operation remains valid. Copy likewise retains forward
overlap behavior: an earlier destination write may affect a later source read.
Neither wrapper takes a whole-header/range snapshot or adds overlap repair.
Both native equality loops require last to be reachable by native32 +10h steps
and every taken actual access to be admitted. Malformed ranges are not guarded.
The provided Source context and actual storage must remain live across child
pool/new-handler/EH activity; private native/Source frame aliases are excluded.
There is no local EH frame, rollback or compensating cleanup: earlier completed
assignments remain visible if a child throws.

Copy EAX is a semantic return: caller `006A5C1B` stores it into EDI at
`006A5C2B`; caller `006A66CD` passes it in ECX at `006A66DE`. The third observed
copy site `006A6748` overwrites EAX before use. All three pass three physical
pointer words, with caller-side ADD ESP,0Ch cleanup. The Source copy returns the
cached address exactly, including wrapped/high-bit arithmetic.

Fill itself never writes EAX. Nonempty native execution leaves the last
`00506C80` destination result; empty native execution preserves incoming ambient
EAX. Both live fill sites `006A5E44` and `006A5EA0` belong to actual function
`006A5C50`, **not catch-all funclets** as the old reviewed-name evidence says.
Their continuations do not consume EAX as data. Common temporary cleanup either
overwrites it before use or carries ambient bits through return. This does not
establish a semantic input/return value, so Source fill is void and introduces
no artificial incoming-EAX argument. Native EAX facts are recorded rather than
promised by the void C++ interface. Ambient flags are likewise outside it.

Both new cdecl Source entries take a fourth explicit context reference and
therefore have a different physical ABI. Actual Release COFF was inspected:
copy caches `((last-first)&FFFFFFF0h) + destination` before the first call, uses a
forward10h loop and returns the cached EDI value. Fill uses the forward10h loop
and, in this exact build, also leaves EAX untouched apart from the child call.
Compiler output may reload invariant Source parameter words (copy's last and
fill's value) from their private frame, unlike native saved registers; the
declared frame-alias exclusion is material. This is static compiled-source
inspection, not original byte equality or a frozen original register ABI.
Neither original wrapper contains x87/FP instructions; NaNs are not involved.

`./scripts/build.ps1` passed the exact C++ source and all three existing suites:
reconstructed_math, native_math_differential, tool_tests. No new tests were
added; those suites provide regression/seed coverage, not native differential
coverage for these two wrappers. Exact direct/caller call checks and whitespace
checks are recorded in the JSON report. No original binary ABI, native
exception execution, startup or gameplay validation is claimed.

Exact native CALL sites (instruction address followed by target):

```text
006A29E6 CALL 00506C80
006A2A16 CALL 00506C80
006A5C1B CALL 006A29C0
006A66CD CALL 006A29C0
006A6748 CALL 006A29C0
006A5E44 CALL 006A2A00
006A5EA0 CALL 006A2A00
```
