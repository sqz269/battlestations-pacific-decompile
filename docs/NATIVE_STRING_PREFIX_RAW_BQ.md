# Raw string assignment, concatenation and prefix cleanup BQ

Addresses: `00425F40`, `004261A0`, `0043C130`. These bodies were already
reconstructed. This packet adds actual-header overloads using the existing
`NativeStringRawPoolContext`; it earns **zero new native-body or byte credit**.
Base: `dc6d5a2823513660be55b775b05e7f56b93de726` in its own worktree.

The existing `NativeStringStorage` assignment/concat and
`ActualNativeStringPoolStorage` prefix interfaces and function bodies remain
unchanged. New callers can select the real raw getter/return path without the
semantic string storage interface's nonthrowing release adapter. No new pool,
manager, allocation callback, default publication or cleanup service is added.
BO/model callers and their metadata are outside this packet.

| Existing native body | Original interface | Raw overload coverage |
| --- | --- | --- |
| 425F40..425F74, 53 bytes, 24 instructions | ECX destination; stack source header; EAX destination; RET4 | Complete identity/resize/current-copy path |
| 4261A0..426245, 166 bytes, 63 instructions | ECX left; stack output/right; EAX output; RET8 | Complete clear, initial copy, append and owned-output cleanup schedule |
| 43C130..43C1B5, 134 bytes, 38 instructions | ECX output, EDX prefix C-string; stack right header; EAX output; RET4 | Complete prefix construction, concat, normal return and two-state cleanup schedule |

The source interfaces add a borrowed raw context. Its actual publication
01090AA8, shutdown gate01090AA4 and manager publication01090AA0 remain the same
cells through every operation and cleanup. Existing raw41DD40, raw41DD20 and
raw41E870 implement their own current-pool schedules. Raw headers are the
original two words, length then data; no `std::string` snapshot is introduced.
Valid accessible header/data storage and the existing Win32 raw access domain
remain prerequisites. There is no binary ABI, hardware-fault or native FH3 claim.

## Assignment and concatenation load order

425F48 compares captured destination/source identities before reading either
header. Equality returns the captured destination without touching the pool or
headers. Otherwise it captures source.length, calls raw41DD40(preserve1), then
rereads source.length at425F56. If nonzero, the copy loads destination.length at
425F5B, source.data at425F5D, destination.data at425F60, then calls BF7680.

4261C6 compares output/left identity before clearing output.length and output.data
at4261CC/CE, including when output equals left. A distinct left supplies the
initial resize length. After resize, 4261DF rereads left.length. A nonzero value
loads output.length at4261E3, output.data at4261E5, **then** left.data at4261E9.
This differs from assignment and the existing typed copy helper. The raw concat
does not reuse that helper or change its legacy behavior.

After the initial copy, 4261FA captures right.length before setting output-owned
bit1 at426203. A nonzero append captures current output.length at42620D, resizes
to old_length+captured_right_length modulo DWORD, then reads current right.data
at42621C and output.data at42621F. It copies the captured appended count to
current output.data+captured_old_length. Consequently a right/output alias uses
the new current buffer after resize; the old right header is not snapshotted.
An output/left alias still loses its old header fields without a protective
old-buffer return, as in the native body.

BF7680 handles backward overlap (BF769A routes to BF7844). Both new overloads
therefore use `memmove`. Assignment and the initial concat copy retain the
existing raw-provider zero-byte call omission only after all corresponding
header loads. The append count is captured nonzero before its copy. Each native
BF7680 call has three stack arguments and caller `ADD ESP,0Ch`; each resize
consumes its length/preserve arguments with RET8.

## Cleanup ownership

Concat's FuncInfo D84D50 declares maxState1, map D84D48. State0 routes to C5E830,
which tests output-owned bit1, clears it before action and tail-jumps to41DD20.
The native state0 is present before the initial copy, but its bit remains clear.
The new source therefore leaves initial resize/copy and the right.length capture
outside its cleanup handler. Only append failure destroys current output once.
No failure adds an old-output return or retries a consumed cleanup.

Prefix constructs its private header through raw41E870 before its caller cleanup
is armed. The private source header is uninitialized two-DWORD storage, so the
outer function does not add default zero stores or an automatic string cleanup.
State1 protects the concat call, while output-owned bit1 is still clear. If concat
fails, it supplies its own output cleanup; prefix destroys only its temporary.

After concat returns, 43C176 captures prefix.data, 43C17C sets output-owned bit1,
and43C184 lowers state to0. If captured data is nonnull, only then does the native
path read prefix.length+1 and call419CC0/BD1510. The source follows that order
directly through the concrete raw getter and return providers, using the captured
data and size; it does not reread the header through a later destructor call.
BD1510's original stack is data,size,unused1 and RET0Ch. The existing source
provider omits that unused semantic argument. Normal return leaves the header
unchanged. A failing normal getter/return never gains a second prefix cleanup.

Prefix's FuncInfo D85BDC has maxState2, map D85BCC. State1 goes to state0 through
C5F350 (temporary destroy); state0 goes to-1 through C5F358 (conditional output
destroy after clearing bit1). Thus a failure after successful concat destroys
output only. This includes a failure of normal prefix cleanup. The raw routes
are C5E849..C5E852 and C5F371..C5F37A; each is MOV EAX,FuncInfo/JMP BF6B43.
The report retains exact bounds and separate primary-definition status.

The new handlers consume each cleanup edge before invoking raw41DD20. If that
cleanup also throws, the newer C++ exception propagates and replaces the earlier
exception. There is no retry and no remaining owned action in that state: prefix
temporary cleanup occurs with output-owned clear, while its output cleanup occurs
after the temporary has been disarmed. This explicit source policy preserves
the ownership schedule but does not claim the native FH3 or terminate-on-double-
failure behavior of other interfaces. No cleanup exception is silently hidden
behind a new `noexcept` callback. Private EH-frame/spill aliases and unrestricted
fault-time observations remain outside the source interface.

## Evidence and validation scope

The report pins all three complete native spans, both EH closures and both
FuncInfo/maps against the untouched installed PE and current read-only Ghidra
bytes. The native flow checks report no listing gaps. Original/direct calls and
raw handler jumps carry exact call-site/containing-function evidence. Primary
owns any later handler definitions, signatures, annotations or ledger updates.

The original three typed function bodies are preserved literally, and their
before/after COFF sections and symbolic relocations are compared, allowing only
the anonymous-namespace path hash to differ between worktrees. The before
objects came from the primary build only after verifying the corresponding
primary source bytes match this packet's base (with CRLF normalization stated).
The preserved typed sections are assignment67 bytes/2 relocations, concat219/8,
and prefix133/5. The raw sections are67,219 and226 bytes respectively. Their
current data-load orders and prefix data/ownership/disarm/getter sequence were
read independently. Generated concat moves its pointer CMP after the output
clears; the compared pointers were already captured. Its compiler EH-state
write at object0072 also precedes the right.length load at0079. These preserve
the admitted ordinary C++ semantics but do not reproduce native hardware-fault
or private-frame observations. No exact instruction-order claim includes them.

One ignored source probe uses a real constructed, prepublished native string
pool. It checks normal prefix text/return identity, backward assignment overlap,
right/output concat aliasing, assignment identity, explicit raw string returns
and concrete pool shutdown. It keeps the unused manager cell null, supplies no
synthetic callbacks and compiles against this worktree's current three libraries
with an embedded manifest. Its source/provider/library pins must match before
and after execution. The full Win32 build and configured existing checks are
reported separately. No broad suite, native differential run or injected cleanup-
failure/native-FH3 proof is claimed.

The final Win32 build passes its one configured existing `reconstructed_math`
CTest. The focused real-pool probe passes with equal before/after pins for its
source, providers, recipe and all three current linked libraries. The initial
build caught a missing raw-context forward declaration; that was corrected and
the successful retry used unchanged pinned source. The first probe link attempt
omitted advapi32/shell32, required by existing XLive objects. Its recipe/log/pins
were preserved, and adding those concrete Windows libraries produced the passing
run without changing source. These are source/build checks, not native exception
injection or game validation.
