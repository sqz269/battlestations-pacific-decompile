# Actual physical provider pending I/O

Addresses: `00BF43B0`, `00BF46B0`, `00BF4240`, `00BF41C0`, `00BF3D00`.

`native_physical_pending_io.cpp` reconstructs the submission and explicit pump
over the actual physical provider's pending descriptor at `+14h`, reusing the
established actual record, string pool and intrusive memory-stream owners. It
does not create a second queue owner. The completion service dispatches the
record's actual callback DWORD and borrowed native string headers. The primary
integrator supplies that dispatch and manager/provider graph binding.

| Entry and inclusive body | Original ABI | Coverage |
|---|---|---|
| BF43B0..BF46A6, 759 bytes | ECX provider; first header, second header, callback, flags on stack; AL accepted; RET10h | complete, explicit service boundaries below |
| BF46B0..BF47DA, 299 bytes | ECX provider; no stack arguments; RET | complete |
| BF4240..BF429F, 96 bytes | ECX queue; signed index on stack; RET4 | complete |
| BF41C0..BF4239, 122 bytes | ECX queue; source record on stack; RET4 | complete |
| BF3D00..BF3D9E, 159 bytes | ECX destination; source record on stack; EAX destination; RET4 | complete |

These descriptive source names are hypotheses. The interfaces add explicit
contexts and are not original binary ABI or FH3 runtime replacements. Every
owned entry is currently present in Ghidra. BF4240 now has its full body. The
current complete listings and min/max extents nevertheless hide function-body
membership holes at BF4617..BF4619 and BF4793..BF47C3 (inclusive). All these
bytes match the installed PE; the pump's missing range contains calls BF479E
and BF47B9. The report retains both automated call-check failures for primary
analysis repair. No annotation or save was performed by this packet.

## Storage and dependencies

The producer BF43B0 and the established BF3C10/BF3ED0 records agree on stride
`38h`: handle `+0`, OVERLAPPED pointer `+4`, kind `+8`, original allocation
`+Ch`, aligned pointer `+10h`, original size low/high `+18h/+1Ch`, first/second
eight-byte headers `+20h/+28h`, callback DWORD `+30h`. Fields `+14h/+34h` stay
untouched. Data/count/capacity at provider `+14h/+18h/+1Ch` are the existing
queue header; virtual slot `+1Ch` is a separate path-builder operation.

Consumed bodies were inspected before their semantic labels were used. The
report enumerates every CALL in the five owned listings with its enclosing
body, callee or indirect operand, and recovered argument contract.

| Established callee | Binding and contract |
|---|---|
| BF3970 | Current D69168 table, slot1Ch must equal BF3970; call existing actual root/suffix concatenation. Native ECX provider, stack output/suffix, RET8. Unexpected profile/slot throws an explicit source-boundary exception. |
| 425F40 / 41DD40 / BF7680 | Actual header assignment/resize and overlap-aware copy; current fields reread after allocation, no shadow strings. ECX header plus source / count,preserve on stack, RET4 / RET8; copy has three cdecl stack words, ADD ESP,0Ch. |
| BF3DA0 / BF3C10 / BF3880 | Existing actual reserve/copy-construction/name destruction. Queue relocation copies pointer values without freeing I/O resources. Name destruction releases second then current first. |
| BEFA40 | ECX aligned bytes, original low/high size stacked, RET8. Existing actual backing and stream construction copies LOW size only; high is unused by that body. |
| BD30E0 and current stream slot4 | Real InterlockedDecrement at stream+4; at zero, existing actual memory-owner dispatch rereads current numeric profile and performs the established deleting destruction. No shadow retain count. |
| BF55BE / BF681B / BF65AC | Shared malloc/new-handler retry and free services. BF55BE is an array-new thunk into BF681B. CRT state, exact allocator addresses and historical CRT exception identity remain host boundaries. |
| 4254B0 | The body is one RET. Source preserves diagnostic argument loads without introducing logging or formatting effects. |

Production strings borrow `ActualNativeStringPoolStorage` through the existing
physical context. Its pre-existing noexcept release cannot reproduce a
throwing lazy pool getter. Callback and retained-memory contexts own no queue,
stream or provider references. Submission opens its own overlapped handle;
the separate synchronous physical stream cache is not called by BF43B0.

## Submission and failure states

The gate is `(flags&1)==0 && (flags&0Eh)==2`; values 2 and 32h both pass. Path
construction uses the current provider slot at BF441E. CreateFileA receives
GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, null security/template and flags
60000000h. The temporary path is destroyed before INVALID_HANDLE_VALUE is
tested. It is not an armed local in this routine's FH3 state map.

GetFileSizeEx writes the record's original low/high size. A false result is
rejected only when the subsequently read GetLastError is nonzero. Allocation
uses wrapping DWORD `low+20000h`; the usable pointer rounds upward to 10000h.
The read count rounds the LOW DWORD to the same boundary with wrap. The native
carry into a rounded high-word local is dead; the original size pair stays in
the record, and only the rounded low word reaches ReadFile. No empty/high-word,
signed-size, initialized-length or overflow guard was added.

The separate 14h OVERLAPPED has five zero words, including offset and event.
ReadFile receives a null immediate count output. Immediate success and
ERROR_IO_PENDING both assign the names and append only **after** the I/O has
started. Both return accepted without a callback.

The native failure and unwind states are retained:

- Failed CreateFileA leaves no pending record. Failed GetFileSizeEx with a
  nonzero error closes the handle and destroys the temporary record's names.
- ReadFile failure other than IO_PENDING calls free on the **aligned +10h
  pointer**, not original allocation+Ch, then closes the handle and destroys
  names. BF460D loads `[ESP+40h]` while the 12 diagnostic argument bytes are
  still pushed: this is pre-call ESP+34h, record+10h. The OVERLAPPED leaks, and
  an interior-pointer free can be invalid. This branch is not normalized into
  a safe rejection path and was not triggered in the real-file probe.
- After state0 is armed, a C++ failure destroys only temporary names. There is
  no handle, staging or OVERLAPPED rollback, including failures while copying
  names or appending after accepted ReadFile. The source uses `__finally` for
  this cleanup schedule, without claiming original FH3/SEH personality parity.

Submission's handler CC7BD8..CC7BE1 has no Ghidra function. Its E02964 FuncInfo
has one unwind state at E0295C: state0 to -1 invokes CC7BD0..CC7BD7, which tail
jumps to BF3880 on the local record. Append's handler CC7B9C..CC7BA5 likewise
has no function; E0290C references E02904 state0 to -1 and CC7B80..CC7B9B.
That action computes the current queue position, passes it and stored placement
address to the one-RET 401130 at CC7B93, then ADD ESP,8. It adds no rollback.
Both no-function handler ranges above use inclusive endpoints.

## Pump and erase order

The pump skips only OVERLAPPED.Internal equal to 103h. Other records call
GetOverlappedResult with wait FALSE. Any false result, even IO_INCOMPLETE,
falls through to cleanup without callback. The returned byte count is not
compared with logical or rounded length. Success with kind other than 1 also
skips callback and cleans up.

Success/type1 creates an independently backed native stream. Before callback
dispatch it reloads queue storage and passes `(stream, &first, &second)`; the
actual callback target is record+30h. Native CALL EAX at BF476D has no ADD
ESP,0Ch afterward, establishing a callee-cleanup three-word callback. The
explicit source dispatch preserves that target and all three operands.

After the callback returns, the pump decrements its stream reference, dispatches
zero-reference deletion if needed, then reloads the current record. It frees
original staging+Ch, frees nonnull OVERLAPPED+4 and zeroes +4, closes the handle,
then erases. Callback/stream-destruction exceptions leave the queued I/O
resources untouched by this pump; there is no source cleanup guard.

Erase assigns each later record into its predecessor, repeatedly checking
current count and backing. Assignment copies established scalar words and
both names, leaves +14/+34 unchanged, and has no rollback. It destroys names
of the current last record and then decrements current count without shrinking
capacity. Signed comparisons and wrapping index arithmetic retain native
behavior, including the absence of bounds checks.

Only a skipped pending record advances the pump index. Erasure revisits the
same index against the current count, so callback appends can be dispatched in
the same pump. Relocation invalidates borrowed name wrappers: consumers must
copy them before appending. Recursive pumping, provider destruction or other
queue mutation during a callback has no native guard and remains outside the
supported integration domain. There is no implicit cancellation or draining.

## Verification and remaining integration

The ignored `local/physical_pending_bk/pending_probe.cpp` was compiled with
MSVC Win32 `/MD /O2 /W4 /WX /fp:strict`, linked with `/MANIFEST:EMBED` against
the exact recorded parent `bsp_core.lib`, and passed. It reads only verified
non-executable profile data from the installed PE; no original code executes.
Fresh live-byte comparisons, exact PE and artifact hashes, native call rows,
compiler/probe commands and report-check result are in the accompanying JSON.
`verify_report_calls.py` checked 36 numeric call rows with two failures: BF479E
and BF47B9 lack Ghidra function membership inside the raw pump cleanup range.
The calls remain numeric rows in the report; they were not removed or disguised
as indirect calls to obtain a passing check. Twelve imported/unknown callback
rows and the two resolved virtual rows remain outside direct-target proof.

The probe verifies rejected flags, missing-file rejection, two actual local
unbuffered overlapped reads, callback-before-resource-release, callback-time
append forcing capacity2 to4 relocation, three callbacks in the same pump,
name order, retained stream contents after queue cleanup, and zero final
native memory counters. Logical file lengths are 257 and 65539 bytes.

This is source compilation and real OS fixture evidence, not original-code
differential, original ABI/FH3, complete allocation-failure or gameplay proof.
The primary integrates CMake, manager/provider virtual dispatch and the
FileStore callback target. Its full build remains the integration check.
The packet changes no runtime bindings, original game files, Ghidra annotations
or persistent analysis state.
