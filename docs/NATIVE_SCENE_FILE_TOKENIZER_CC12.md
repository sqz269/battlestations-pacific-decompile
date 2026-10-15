# Native scene file tokenizer (008D9CF0)

This packet reconstructs the complete ordinary file-backed tokenizer constructor
and concrete Source VFS services. It adds a new explicit-context C++ interface;
it does not claim the Original class, register ABI, FS/EH, private CRT or game
equivalence. Names remain descriptive hypotheses.

## Original and Source ownership

The complete Original body is `008D9CF0..008D9EF7`, 520 bytes and 162 reachable
instructions, SHA256
`63d4cd28d3fad86b714407ff9f916e0c88fb392f3535ceef80c37bbd09bd3568`.
Accepted frozen readiness evidence supplies full bytes, assembly, CFG, PE/live
agreement, typed project/program identity and the immediate provider contracts.
The Source work preserves that entire 81-artifact family.

Original ECX is genuine 838h tokenizer storage. Entry stack `+4/+8` is ONE owning
eight-byte `NativeString` passed by value: length, then data. Stack `+0C` is the
borrowed extra-delimiter pointer. EAX returns the same owner; `RET0C` consumes
the three words. Cached pseudocode misidentifies these arguments.

The new Source function accepts `NativeString& consumed_filename` to express
that actual parameter's ownership without inventing a copy constructor. The
object must be genuine and own its buffer through the supplied canonical raw
string domain. The constructor consumes that ownership and leaves its released
header unchanged. Do not explicitly release it again. The existing Source
`NativeString` destructor is trivial. Static assertions establish size8,
alignment4 and nothrow default construction. A placement-constructed genuine
`NativeString` at owner+820 owns the separate retained filename copy. The actual
header-address equality branch remains present.

## Complete storage and access schedule

The constructor writes numeric tokenizer profile D15FE4, zeroes the actual
member header820/824 and word81C, then fills current token `+5..404` and previous
token `+405..804` with exactly400h zero bytes each. It writes individual zero
bytes in order805,80A,809,808; words814,810,80C,818; then bytes807,806. Quoted
byte4 and padding byte80B remain untouched. The three words80C/810/814 are not
given invented semantic names.

Unless member and incoming header addresses are equal, capture incoming length,
resize the member with preservation enabled, then copy CURRENT member length
bytes from CURRENT incoming data to CURRENT member data if the captured length
was nonzero. The accepted resize supplies the terminator. This is not a CString
scan or pointer assignment; accepted BF7680 overlap behavior uses `memmove`.

Load CURRENT publication0109CEEC and its CURRENT manager table; open slot4 with
the actual member header and full mode32. Store the returned stream at828,
including null. There is no AddRef. For a nonnull result, capture CURRENT slot30
size, store EAX low32 at830, and allocate exactly that count through the accepted
current-CRT heap service. Reload CURRENT830 and828 AFTER allocation, publish the
returned buffer82C and cursor834=0, then capture CURRENT stream slot24 and read
the current count with a null actual-count output. No result check, short-read
trim, text zero-fill, added byte or terminator is supplied.

Only successful open builds delimiters. Null extras borrows CURRENT E0C944.
Nonnull extras scans current default and extras, allocates `n+e+2`, stores818,
reloads CURRENT default after allocation and copies it including NUL, rescans
extras including its NUL, reloads818, scans its terminator and copies extras.
Specifically9E78 sets ESI to the extra start itself; the scan advances EAX past
NUL, so9E8F subtracts the start and produces `strlen(extra)+1`. MOVSD/MOVSB copy
that complete count. The allocation's spare final byte remains unwritten.
The initial default happens to be CE5698 (`;`), but Source borrows the live cell
and never hardcodes that default. Allocation arithmetic remains DWORD arithmetic
in the providers' admitted nonoverflowing domain.

Finally consume CURRENT incoming filename storage through the actual raw-string
destructor composition: null data skips length/getter/return; otherwise capture
data and length+1, resolve419CC0 and return through BD1510 with the native small
return policy. Member filename and acquired stream/text/delimiters remain owned
by the tokenizer. The accepted explicit tokenizer destructor later decrements
stream+4 once, dispatches its CURRENT slot0 at zero and clears828; it also frees
text/delimiters and releases the retained member filename.

A null open skips every size/allocation/read/delimiter operation. It leaves818
zero and82C/830/834 UNWRITTEN, then consumes the incoming filename and returns
owner. It is not a normalized empty tokenizer. Subsequent scanning reads size
and cursor, and the accepted destructor unconditionally frees82C. Valid caller
preimages or a separately proved caller failure policy are required before
using those later paths; this packet invents neither and retains the null-open
constructor branch.

## Concrete VFS and lifetime composition

`NativeSceneFileTokenizerVfsCalls` is a final Source service object borrowing
the existing concrete `NativeVfsRuntimeBindings`. It captures actual current
numeric owner tables and calls their reconstructed services. Manager D685B4 and
D68D04 slot4 select BDF310 with mode32; existing physical/FileStore/MPAK provider
opens are reused. Stream size/read/release support the existing D642C0 memory,
D691B0 physical, D68DB0 adopted and D64400 raw-inflater profiles. Unsupported or
mutated identities remain the existing Source boundary, not fabricated failure
or a callable-address fallback. No fake original stream/table is constructed.

The constructor uses the accepted qualified BF79F0 provider with a borrowed
genuine canonical0109EEA4 feature-cell reference for both fills. The existing
`GameNativeMutableCrtData::feature_word()` can supply that cell. No guessed flag
or CPU default is introduced. Native CPU/OS/DF and storage preconditions remain
those of that accepted provider.

`GameNativeSceneFileTokenizerBindings` owns only the Source calls and contexts.
It borrows actual `GameNativeVfsRawServices`, matching canonical raw strings,
live whitespace/default-delimiter cells, original unknown-file text needed by
the shared tokenizer context and the CRT feature cell. Caller supplies genuine
live838h storage and owns its provenance. The same calls/strings/memory/defaults
feed construction and `tokenizer_context()` for all later operations and
explicit destruction. Copy/move are disabled because those contexts refer to
the embedded service object. The holder performs no native allocation, I/O,
startup, publication, registration or implicit tokenizer destruction. Retain it
and all borrowed owners until every tokenizer is explicitly resolved/destroyed,
before shared VFS/pool drain. No application wiring is added.

Existing `NativeSceneTokenizerCalls` defaults admit only D642C0 and are not used
for actual file-stream dispatch. Borrowed-stream constructor008D9F20 increments
the supplied stream and is not called or substituted here. `SceneLexer` and host
string-buffer projections are not used.

## Source exceptions and qualification

Original CA3326/FuncInfoDD55F4 has two string cleanup states: member820, then the
by-value filename. It has no stream/text/delimiter unwind action. Source keeps
those same two explicit cleanup obligations for C++ exceptions and sets state-1
before final ordinary argument release. Cleanup is attempted in member-then-
argument order; raw cleanup itself may fail. No transaction, blanket RAII
rollback, replay, normal-destructor-on-failure or complete cleanup guarantee is
added. Failed construction can leave acquired stream/text/delimiter obligations
and dangling released string headers; preserve actual storage and services to
resolve them. Original FS/SEH/unwind/fault/private-CRT behavior is not proved.

Admission is the immediate providers' genuine initialized, stable/disjoint,
coherent current-CRT storage domain, finite valid strings/chains, nonoverflowing
extents/counts, supported actual VFS tables and native CPU/DF domain. No caller
class, whole-loader startup, runtime or game equivalence follows.

## Compilation and full emitted-object inspection

Both changed TUs passed strict Win32 MSVC compilation with
`/std:c++17 /O2 /Ob2 /Oy- /W4 /WX /EHsc /Gy /GL- /fp:strict`.
The first successful attempt is retained: the optimizer combined neighboring
flag-byte stores. Only the byte-store helper changed to explicit byte asm stores;
the final original strict chunk5770b6 exited0. All six final stores are separate
and in the recovered order. No Source changed after final strict compilation.

All emitted code, symbols, branches and relocations were inspected: native
object44 sections/20 code sections/1211B/434 instructions; game object14/5/123B/48
instructions. Together there are36 code relocations (31 REL32,5 DIR32), and44
metadata relocations (35 DIR32,9 DIR32NB). The only unreachable code bytes are
six compiler INT3 padding bytes after the nonreturning Source rethrow. Source
vft/RTTI/EH/debug records describe the service objects, not Original tokenizer
or stream class ABI. New Source frames, instruction/register/flag schedules and
current CRT copies differ from Original; ordinary effects and documented
ownership/access dependencies are the admission claim.

The own normal build launched with original chunk7f2b46/session21060, yielded
poll9b2ebf, and naturally terminated with414930/exit0. Both configured CTest
suites passed in8.76s. One existing Lua-spawn duplicate-symbol LNK4006 was emitted.
The new TUs remain unregistered: strict objects cover them, while the normal
build covers its registered baseline. No new test/probe/target runtime was added.
Exact original paired history/tool-object receipts and frozen Source inputs are
retained under `local/WholeSceneFileTokenizerSource01`.
