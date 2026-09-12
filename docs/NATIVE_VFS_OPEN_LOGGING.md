# Native VFS opened-resource builder

The complete installed `00BDE9C0` path builds `<FILE><` + name + `>` in native
pooled storage, then destroys that storage. **It does not emit the string.**
There is no sink call, stream operation, registration, or dereference of global
`0109CEE8` anywhere in the six complete bodies reconstructed here. The slot is
only a nonnull gate. This corrects the earlier emission interpretation in
`VFS_PRELOAD_BOUNDARY.md` and the unresolved logger-owner boundary in
`NATIVE_VFS_OPEN_ROUTE.md`; it does not establish behavior of other logging paths.
No output callback, no-op sink, `std::string`, stream owner, or logger owner is
introduced to fill that earlier interpretation.

## Routines and native ABI

All descriptive names are hypotheses. Each interface is a new MSVC Win32 C++
API, with an explicit storage/context parameter, rather than a binary ABI or
FH3/SEH replacement.

| Entry | Inclusive end | Coverage | Original ABI and behavior |
|---|---|---|---|
| `00BDE9C0` | `00BDEB38` | complete | ECX manager; name, unused stream, mount DWORD stacked; `RET 0C` at BDEB36 |
| `00426500` | `00426515` | complete | ECX builder; `RET`; EAX same builder after 4264A0 |
| `004264A0` | `004264F8` | complete | ECX builder; `RET`; EAX builder; constructs three 8-byte headers |
| `00BD1A20` | `00BD1A59` | complete | ECX builder; source header stacked; `RET4` at BD1A57; EAX builder |
| `00BD1A60` | `00BD1B0C` | complete | ECX builder; nullable C string stacked; `RET4` at BD1AF6/BD1B0A; EAX builder |
| `00425F80` | `00426017` | complete | ECX builder; `RET`; releases three headers, return unspecified |

The current saved Ghidra bodies have no listing gaps. All six complete bodies,
their next 16 bytes, six literals, and the four EH maps/handlers match the
read-only installed PE. `reports/native_vfs_open_logging.json` carries the exact
spans, hashes, bytes, call sites, native cleanups, and remaining evidence limits.
No function definition, disassembly repair, rename, comment mutation, or save
was performed by this worker; the integrator owns Ghidra annotation.

## Actual storage and ordering

`004264A0` is the layout producer: it clears the first length/data header,
constructs `%d` at +8 from `00CE3A34`, then `%.3f` at +10h from `00CE3A48`.
This is exactly 18h bytes, containing three established actual 8h native string
headers. The code accepts raw storage, and reuses the existing actual-header
resize/constructor/destructor plus `ActualNativeStringPoolStorage`; it declares
no competing manager, builder string type, pool, or global owner.

BD1A20 captures source length in EDI and old destination length in EBX before
resize, then reloads source and destination data pointers after it. Self-append
therefore uses the relocated current data. BD1A60 first constructs a native
temporary and captures its length/data in EDI/EBP; it uses those captured values
for the normal append and release, including if allocation reenters. Null input
returns before reading the destination. Both preserve `memcpy`'s native valid
memory/nonoverlap requirements and DWORD size arithmetic. Every `memcpy` call
has `ADD ESP,0C`; resize returns with `RET8`, string construction with `RET4`,
and pool return with `RET0C`. The pool getter takes no arguments and leaves the
already-pushed pool-return arguments in place.

BDE9C0 captures manager at BDE9DE and name into EBP at BDEA17. EBP remains the
name through BDEAF0; the original name stack slot is reused to store the dot
position at BDEA2E. The stream stack slot is never read. BL starts zero and is
changed only by the digit predicate; the mount argument is read as one byte at
BDEAD7 despite the three-word stack cleanup. The append arguments are pushed
together at BDEAEB/BDEAF0/BDEAF1, then consumed separately by the three `RET4`
calls, with the returned EAX builder becoming the next ECX.

The function always constructs a pooled dot temporary, searches backward using
the existing `00467CF0`, then returns its captured buffer. If the dot index is
positive, it reconstructs the now-dangling temporary header as underscore,
searches before the dot, and returns that buffer. Both searches exclude index
zero. Suppression requires positive underscore, dot minus underscore equal to
eight, and seven ASCII digits in between. The loop reloads name+4 for each byte.

Only after these allocations/searches/releases does it read `0109CEE8`, manager
byte +79h, and the mount argument's low byte, in that short-circuit order; the
last gate is suppression. Manager constructor `00BE1DC0` writes byte +79h to 1
(write at BE1ED3; this is a consumed existing manager, not an owner reconstructed here).
When enabled, the three-string builder is constructed, receives the prefix,
actual supplied header and suffix, and is destroyed in +10h, +8, +0 order.
Destruction leaves all header bytes untouched. The global's pointee and stream
need not be readable because this function never dereferences either.

## Exception states

`BDE9C0` uses FH3 handler CC6478, metadata E00988, and one state: state0 at
BDEAF8 arms CC6470, which tail-jumps to 425F80 for the stack builder. State stays
-1 throughout the two filename temporaries and builder construction; it is
reset to -1 at BDEB17 before normal destruction. No extra cleanup is armed for
failed temporary construction/search or failed builder construction.

`4264A0` uses handler C5E8C3, metadata D84DE4, states0/1 and unwind map D84DD4.
State0 destroys header +0 through C5E8B0; state1 first destroys +8 through
C5E8B8, then state0. Header +10h is not armed while its construction is pending.
The source catches match those ownership states and abandon the failing header.
`426500` only has a local zero word and a direct constructor call; no EH frame.

`BD1A60` uses handler CC54D8, metadata DFF540, state0 and unwind map DFF538.
State0 is set only after the temporary constructor returns and its fields are
captured. Unwind CC54D0 tail-jumps to 41DD20, rereading the current local header;
normal cleanup instead uses captured EBP/EDI. `BD1A20` has no local EH frame.

`425F80` uses handler C5E803, metadata D84CF8, states1/0/-1 while destroying
+10h/+8/+0. Its two unwind entries C5E7F8/C5E7F0 release remaining +8/+0.
The established storage interface makes release `noexcept`, so source cleanup
preserves the returning-release domain without claiming native throwing-release
or FH3 compatibility. None of the original exception handlers is executed by
the normal-path differential fixture.

## Verification and integration boundary

The ignored local fixture maps the read-only installed PE into its own process,
executes all six bodies with seven known data operands rebased, and bridges only the already
reconstructed string constructor/resize/reverse-find, pool getter/return, and
CRT memcpy. It compares the source against original instructions across filename
and gate cases using the same actual native pool address. Every byte before the
pool's critical section is compared, including pooled string content, allocation
bump, free-ring cells, and ring counters. Invalid nonnull stream/global values
and a guarded unreadable manager exercise the observed read boundaries.

All 16 cases passed, each matching the 9,098,372-byte pool prefix. The original
00410000 region was already occupied by a process mapping; the fixture leaves
that mapping intact and uses 50000000 for the PE. Because this installed PE has
no relocation directory, it adjusts the seven explicitly verified data operands
at BDEA0A/BDEA4F/BDEAC6/BDEAEC/BDEAF2/4264C5/4264D6. Opcodes, stack handling,
relative calls/branches, and FS:[0] remain unchanged. This address adjustment is
fixture instrumentation, not proof of a generally relocatable game image.

The fixture is instrumentation, not an original-game run, and covers normal
paths only. The strict Win32 build and both existing CTests also passed. The
report records artifacts and hashes. No permanent tests are added. The caller must supply its
actual global slot and actual pool/lifetime domain; there is no remaining sink
owner contract for this particular consumer. Primary integration connects this
context to the native open route. Other providers, stream owners, and logger
creation/teardown outside this function remain separate work.
