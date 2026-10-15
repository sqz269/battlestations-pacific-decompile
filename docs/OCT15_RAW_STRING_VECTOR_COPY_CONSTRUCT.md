# Actual pooled-string vector copy construction

Addresses: `00557BC0`, `00556B10`, `00556B5A`, `00557C84`, `00432440`.

This packet reconstructs the complete constructor and its two exception scopes
over the actual eight-byte `NativeString` headers and accepted raw-pool services.
The earlier qualified `native_input_settings_vector_storage` implementation and
its evidence remain separate and unchanged. The source is a new MSVC Win32 C++
interface with an explicit `NativeStringRawPoolContext`, not a native ABI entry.

| Native address/name | Original ABI | Coverage and Source binding |
| --- | --- | --- |
| `00557BC0`, `STL_CheckedNativeStringVector_CopyConstruct_00557BC0` | `ECX` destination; one stacked source owner; `RET4`; `EAX` destination; incoming `EDX` unused | complete: `construct_native_string_header_vector_copy_00557bc0`, plus explicit context |
| `00556B10`, earlier `STL_inst_00556b10` | `ECX` first, `EDX` last; stack destination/owner/unused-tag3/unused-tag4; `RET10h`; `EAX` completed end | complete: `copy_native_string_uninitialized_range_00556b10`, all six physical inputs plus context; exact loop/catch reuse of accepted `0044D0C0` |
| `00556B5A`, `Catch_All@00556b5a` | parent `EBP`, saved original destination and completed end; not an independent ordinary function | complete exception-scope fragment: accepted helper's completed-prefix C++ catch; native entry/metadata identity excluded |
| `00557C84`, `Catch_All@00557c84` | parent `EBP`, saved destination receiver; not an independent ordinary function | complete exception-scope fragment: constructor catch tidies current destination then rethrows |
| `00432440`, `STL_NativeStringVector_Tidy_00432440` | `ECX` actual vector; no stack input; plain `RET`; no semantic result | complete: `tidy_native_string_header_vector_00432440`; exact accepted `004324A0` composition, context in Source `EDX` |

The descriptive helper library name proposed by this packet is
`STL_NativeStringRange_UninitializedCopy_00556b10`. The established constructor
and Tidy library names and generated catch names are retained. No Ghidra write
is performed by this worker.

The constructor is itself a layout producer: `00557C00/C03/C06` clear the three
DWORDs at `+4/+8/+C`; `00557C21/C24/C2A` publish allocation begin, begin and
begin+count*8. The accepted `00432090` producer agrees. Header `+0` is opaque
and untouched. The canonical `NativeString` layout is eight bytes; these are
actual pooled-string elements, without a host `std::vector` or iterator object.

The source owner and original source begin are captured before any destination
store. A null begin gives count zero without reading source end. Otherwise the
count is `SAR32(end-begin,3)` after DWORD wrapping subtraction. Destination
`+4`, `+8`, `+C` are cleared in that order, even for count zero. Zero count then
returns without allocation, reports or element work. A nonzero count above
`1FFFFFFFh` uses the actual length-error service. Allocation is direct
`0042BF40(count,EDX=0)`, rather than the superficially similar Buy wrapper.
The real allocation and error-payload dependencies are now accepted source.

After publishing the new three fields, the native body captures CURRENT source
end and compares CURRENT source begin to that captured end. Only then does
`00557C33` arm state0. A report may return. Next it captures CURRENT source
begin, compares that captured begin with fresh CURRENT source end and reports
again if needed. Both captured source pointers survive reports. The destination
begin is reloaded after those reports. The two unused helper tag words contain
the captured source-begin bits with their low byte cleared; all four physical
stack words are consumed by `RET10h`, but only destination and owner are read.
The helper receives these captured endpoints; a successful result is published
to destination `+8`. No extra bounds/owner/null/alignment checks are added.

No self-copy guard exists. With the same source and destination and a valid
nonempty old vector, the old count is captured, the old fields are cleared,
a new count-sized buffer is published with begin=end, and the subsequent source
range is empty. The old storage is abandoned. This is the raw constructor's
observed behavior; it is not replaced with assignment or old-buffer cleanup.

The helper copies each element through actual `0044BCB0`. Only successful
elements advance its completed destination end. On failure it destroys the
completed prefix in ascending order through actual `00431F40`; the failing
element is excluded. The accepted `0044D0C0/0044D10A` source has exactly this
normal loop, update schedule, consumed owner and catch behavior. Reuse does not
claim that the two original FH3 metadata identities are interchangeable.

The outer catch invokes actual Tidy and then rethrows. Tidy captures begin,
tests it for zero, captures end only on the taken branch and destroys that
captured range through `00432050`. It then frees CURRENT header begin through
the accepted singleton/current-CRT free service and clears `+4/+8/+C` in order.
It does not free a separately remembered allocation pointer. Full instruction
and dataflow equivalence to accepted `004324A0` was checked. On a helper exception the header end
has not received the helper's partial progress. Returning invalid handlers or
real provider effects can still alter the fields; the catch reads those current
fields. No rollback, retry, cleanup guard or independent catch-frame object is
invented. If inner-prefix cleanup or outer Tidy throws, the current C++ exception
escapes immediately; native double-exception/unwind identity is not established.

Native exception metadata was read directly. Handler `00C6D780` supplies
FuncInfo `00D97B2C`, and handler `00C6D890` supplies `00D97D94`; both jump to
the original FH3 service. Both have magic `19930522h`, maxState2, two null-action
unwind entries with toState -1, one try `{0,0,1,1,handler-array}` and one handler
record `{40h,0,0,catch-entry}`. Helper maps are `00D97B08/00D97B18/00D97AF8`
with entry `00556B5A`; outer maps are `00D97D70/00D97D80/00D97D60` with entry
`00557C84`. The outer protected domain begins after length/allocation/publication
and the first source reads, before the first report. The source uses `/EHsc`
C++ catch/rethrow. FH3 private-frame access, SEH/fault handling, original CRT
throw-object identity and cleanup-failure personality remain qualified.

Full owned listings, catch entries, callee bodies, original stack cleanup,
EH metadata, and all 40 direct constructor caller setups across 17 functions
were inspected. `local/oct15_raw_vector_copy_ctor_callers.json` is an ignored
bounded instruction-context receipt. The following 52 direct call claims include
the 12 owned body/catch sites and all 40 constructor sites; each containing
function is recorded in the JSON report.

```text
00557C13 CALL 00431F70
00557C1C CALL 0042BF40
00557C3C CALL 00BF6713
00557C4C CALL 00BF6713
00557C67 CALL 00556B10
00556B4A CALL 0044BCB0
00556B6A CALL 00431F40
00556B7A CALL 00BF6885
00557C87 CALL 00432440
00557C90 CALL 00BF6885
00432454 CALL 00432050
0043245D CALL 00BF65AC
005C8545 CALL 00557BC0
005DF452 CALL 00557BC0
005DF462 CALL 00557BC0
005DF13F CALL 00557BC0
005DF14F CALL 00557BC0
006A3259 CALL 00557BC0
006A43E8 CALL 00557BC0
006A3397 CALL 00557BC0
00559E47 CALL 00557BC0
0055B98A CALL 00557BC0
0055B99B CALL 00557BC0
006A5C7D CALL 00557BC0
00AA5D49 CALL 00557BC0
008E97F5 CALL 00557BC0
008E99C4 CALL 00557BC0
0055866C CALL 00557BC0
005590F2 CALL 00557BC0
0057CF45 CALL 00557BC0
005C5270 CALL 00557BC0
005C5281 CALL 00557BC0
005C5292 CALL 00557BC0
005C52A3 CALL 00557BC0
005C52B4 CALL 00557BC0
005C52C5 CALL 00557BC0
005C52D6 CALL 00557BC0
005C52ED CALL 00557BC0
005C5304 CALL 00557BC0
005C531B CALL 00557BC0
005C5332 CALL 00557BC0
005C5349 CALL 00557BC0
005C5360 CALL 00557BC0
005C5377 CALL 00557BC0
005C538E CALL 00557BC0
005C53A5 CALL 00557BC0
005C53BC CALL 00557BC0
005C53D3 CALL 00557BC0
005C53EA CALL 00557BC0
005C5401 CALL 00557BC0
005DAF3D CALL 00557BC0
005DAF51 CALL 00557BC0
```

Verification: MSVC Win32 `./scripts/build.ps1` passed all three existing suites
(`reconstructed_math`, `native_math_differential`, `tool_tests`). JSON and Markdown
verification each passed all 52 direct call claims, with no unchecked claims or
warnings. Owned ledger/index and diff checks passed; only the five owned addresses
changed, all earlier name evidence was retained and all prior reconstruction
records are unchanged. No new routine tests are added. The existing native differential
suite covers its prior math seeds, not these routines. No runtime cache binding,
new-function native differential, original ABI compatibility or gameplay
validation is claimed. The accepted allocator/free/pool/manager/current-CRT
policy and valid taken backing requirements are inherited. Source by-value
parameters, extra context, delegated catches and generated private storage do
not expose native private argument-slot aliasing or produce identical faults.
The current full vector assignment is a separate integrated packet; this packet
does not modify it or extend its validation claims.
