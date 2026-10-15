# Raw float display string and canonical Lua composition

This packet reconstructs the whole ordinary Source body of `0043BDF0` and
provides a concrete binding for the NUMBER arm of the existing actual-storage
`native_lua_to_display_string_00b68630`. The formatting service is explicitly
**the current CRT `std::sprintf`**. It does not reconstruct or equate the game's
private CRT numeric text, locale, rounding, exception or startup behavior.
Names are descriptive hypotheses. No existing Source, Ghidra, shared metadata,
CMake or application wiring is changed by this packet.

The Original body is the complete contiguous `0043BDF0..0043BED4`: 229 bytes,
82 instructions, all reachable in its ordinary control-flow graph. ECX is a
fresh actual eight-byte NativeString header. Entry stack+4 is a float32,
EAX returns the header and the callee uses RET4. ST0 is not an input.
`43BE08` loads the stack float; `43BE13` materializes an eight-byte double
vararg. The caller pushes the immutable `CE4334` bytes `25 66 00` (`%f`),
then the local destination, calls `BF7A6A`, and cleans sixteen bytes.
The complete 123-byte, 51-instruction BF7A6A body establishes the normal
cdecl `sprintf(char*, const char*, ...)` signature and unbounded buffer role.

The new API is `construct_native_string_float_header_0043bdf0(fresh, strings,
float)`. It is a context-bearing C++ function with a different binary ABI.
`fresh` is genuine writable aligned eight-byte header storage. Both the
returned output and the distinct, genuinely constructed NativeString local
use the supplied actual raw pool/manager/gate domain. NativeString is size8,
alignment4, nothrow default constructible and trivially destructible; it has
no implicit owning destructor. The caller destroys the returned output once
through the same raw context, after its final use.

The Source service passes an explicit `char*` to `std::sprintf`, so a secure
C++ array overload cannot select a bounded alternative. It uses only `%f`,
ignores the integer return and scans through the full first NUL. It does not
zero the formatting buffer, clip output, synthesize an error string, append
an extra terminator, force a locale, change FP controls or filter any float.
Negative zero, infinities and NaNs reach the same current service as every
other float32. Their text and FP behavior belong to that service.

The Source scratch is 52 bytes. This represents the observed accessible
Original prefix from entry-stack-40h to the next known saved-FS object at
entry-stack-0Ch. A declared Original array extent was not recovered.
The current-service result including NUL must fit the actual accessible
Source buffer and the Original prefix being compared. This is an explicit
unbounded-formatting/no-overflow precondition, not a universal locale bound.
The active current locale/options/FP environment are used without replacement.

The full two-string schedule matters because allocations, returns and failures
are observable in the genuine pool:

- Materialize the input double, zero output length and pointer, then format.
- Construct and zero the genuine local header after formatting; scan, resize
  it with preserve=true, and capture its current data and length after resize.
  Copy captured length+1 bytes, including NUL, when captured data is nonnull.
- Enter the sole completed-temporary cleanup scope. Retain the self-header
  comparison; otherwise resize output using the captured temporary length.
  When that captured length is nonzero, reload output length/data after the
  resize and copy the current output length from captured temporary data.
  The final copy excludes NUL; the raw resize already wrote its terminator.
- Leave the cleanup scope before normal release. Resolve the current raw
  getter and return the captured temporary pointer with captured length+1.
  Return the actual output header. Do not simplify this to one allocation.

As in the accepted raw memory helpers, a zero-byte final copy is omitted.
The compiler also elides its unused data-pointer load on that zero-count
path. No Original fault/read-site identity is claimed. This does not filter
numeric zero: zero still takes the formatting and two-string path.

Original FuncInfo D85B7C has one state0 unwind entry through C5F310 to the
current-header destructor0041DD20. No temporary cleanup is registered during
formatting, initial resize or its first copy. Only a completed temporary is
cleaned during output work. There is no failed-output cleanup. State-1 precedes
normal temporary release, so a throwing getter there cannot cause a second
release. The Source implements that limited scope with an explicit catch and
the throwing raw destructor; no RAII transaction or rollback is added. Failure
of cleanup itself has the new C++ interface's behavior; private CxxFrameHandler,
FS/SEH, continuation, fault and double-exception behavior are not reproduced.

`GameNativeLuaDisplayStringBindings` is a noncopyable and nonmovable concrete
holder. It borrows one genuine retained GameNativeStringProcess. Existing
non-NUMBER arms receive `process.strings()` and its fixed internal numeric
adapter receives `process.raw_context()`: both use the same actual01090AA8,
01090AA4 and01090AA0 cells. Consumers cannot select a formatter or callback.
The holder directly calls the existing B68630 API; the already established
B66270 float32 rounding and B686F5 float spill are retained. It owns no pool,
Lua state or result, creates no native owner, and performs no startup,
registration or publication. The process and all Lua owners must outlive calls
and returned-string cleanup. The existing non-NUMBER storage interface's
noexcept-release qualification is unchanged; NUMBER uses the throwing raw API.

Strict Win32 `/MD /O2 /Ob2 /Oy- /W4 /WX /EHsc /Gy /GL- /fp:strict` compiled
both changed TUs successfully. Every emitted code/data/metadata section,
primary symbol, auxiliary record, relocation and directive was inspected:
36 sections, 84 primary symbols/120 records, 18 code sections, 797 code bytes,
310 decoded instructions, 31 code and eight metadata relocations.
The core has FLD DWORD at+3Dh, FSTP QWORD at+40h and distinct output stores
at+48h/+4Ah. SSE MOVSD then copies those double bits to the actual vararg.
There is no forwarding of the unrounded Lua double or numeric narrowing guard.

The emitted state0 store precedes the register-only self comparison; Original
compares first and stores state0 before branching. Captured length+1 arithmetic
also moves after the normal getter in emitted code. These pure comparisons
and unsigned arithmetic change no intervening memory read/call/alias dependency
in the admitted coherent ordinary domain. Literal instruction/flags ordering
is not claimed. Current output-field reloads remain after output resize.
Only six unreachable INT3 padding bytes occur in the complete core section.

The current-CRT wrappers emit `__stdio_common_vsprintf` IAT calls with SIZE_MAX,
NULL locale and current stdio options. Their standard options COMDAT, AVX weak
fallback, digit-pair constants, debug/checksum metadata and compiler constructor
iterator are retained and classified in the complete COFF review. None is a
fabricated Original formatter table, locale/PTD or native publication.

The required own repository build reached natural exit0 and its configured
2/2 checks passed in7.89 seconds. The existing duplicate spawn_request_id_matches
LNK4006 warning is retained. Both new TUs remain unregistered, so that build
validates the registered baseline; strict compilation covers these new TUs.
No tests or ad hoc probes were added, and neither new API nor Original target
nor the game was invoked. Root owns independent admission, CMake registration,
Main compilation/integration and any later validation.

The genuinely unclosed Original route remains BF7A6A→C08748→current locale/PTD
and decoded actualE15BE0→C06768→C0657E. InitializerBF69E8 installing a pointer
is not canonical startup proof. Private dispatch ownership/encoding, locale
updates, decimal rounding, NaN/Inf/sign/payload text, errno/invalid parameter,
FP flags and EH/fault behavior remain separate. The current-CRT service closes
ordinary Source composition without claiming those equivalents. This packet
also does not reconstruct whole enum/Lua8F2F70 or close enum-global startup.

The companion JSON report and ignored `local/WholeNativeStringFloatFormatSource01`
retain the immutable readiness snapshot, exact Original inputs, compiler include
closure, complete objects, raw original strict/build call-result receipts,
registration qualification and the final family seal.
