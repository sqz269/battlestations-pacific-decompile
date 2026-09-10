# Native legacy exception owners

Five complete native bodies now construct, copy, destroy and scalar-delete the
actual 40-byte legacy logic/length-error storage. The implementation preserves
base-message ownership, returning allocation/free continuations and the original
owner cleanup scopes. One original-byte sequence passed with 463 matching trace
words; the strict MSVC Win32 build and both existing CTest checks passed.

Sources: `include/bsp/native_legacy_exception_owner.hpp` and
`src/native_legacy_exception_owner.cpp`. Detailed byte evidence, dependency
bindings and artifact hashes are in
`reports/native_legacy_exception_owner_audit.json`. The storage names describe
observed behavior; they are not recovered native symbols.

| Entry and complete end-exclusive span | Bytes | Original ABI and operation |
| --- | --- | --- |
| `00411700..0041175F` | 95 | ECX owner; stack source SBO address; RET4/EAX owner; logic-error construction |
| `00411780..004117B2` | 50 | ECX owner; RET0; no semantic result; ordinary destruction |
| `004117C0..00411807` | 71 | ECX owner; stack flags; RET4/EAX original owner; scalar deletion |
| `004118D0..00411935` | 101 | ECX destination; stack source owner; RET4/EAX destination; logic-error copy |
| `00411940..00411959` | 25 | Same copy ABI; call logic copy, then publish length-error table |

## Actual storage and raw lifetime

`NativeLegacyExceptionStorage` is 28h bytes: native vtable DWORD +0, base message
pointer +4, base ownership DWORD +8, and the existing 1Ch-byte
`NativeLegacySboStringStorage` at +0Ch. The member string's preserved leading
word is therefore +0Ch, inline buffer/heap pointer +10h, length +20h and capacity
+24h. No second string implementation is introduced.

The shared string storage's convenience default member initializers were removed
in integration `52012c4`. Both storage types now assert trivial default
construction; the owner also asserts trivial copyability and exact offsets.
Default-initialize the actual owner without parentheses, for example
`new (raw) NativeLegacyExceptionStorage`, to begin its C++ lifetime without field
writes. The constructor functions then write only the native-observed fields.
They do not create a temporary header or restore a saved snapshot. The member's
leading word and unused inline bytes retain their original values.

The native table DWORDs are `00D69370` for the base exception, `00D69248` for
logic-error state and `00D69260` for length-error state. They are retained as
address data. These raw fields are not callable host C++ virtual tables and do
not make this storage a host `std::exception` object.

## Base copying and owner cleanup

The complete library bodies `00BF632F..00BF6340`, `00BF63A6..00BF63FE` and
`00BF6454..00BF646A` establish the twelve-byte base contract. Their correct library
identities are retained; their field operations are implemented privately using
real CRT `malloc`, `strlen`, `strcpy_s` and `free`.

Default base construction clears message and ownership before publishing the
base table. Base copy publishes that table, captures/copies the source ownership
DWORD, then captures its message pointer. A zero ownership value shares the
pointer. A nonzero value and null message produce a null destination message.
For an owned non-null message it captures `strlen+1`, performs nullable CRT
`malloc`, and publishes the result even if null. It does not substitute the
throwing singleton allocator. On success it reloads the source message pointer
after allocation/publication and calls `strcpy_s` with the captured size and
allocation. The secure-copy result is ignored, and returning handler repairs
remain in the actual owner.

Logic construction initializes the completed base, publishes the logic table,
sets only the member length to zero, capacity to 15 and first byte to NUL, then
calls the existing `00408120` substring helper with offset zero/count `FFFFFFFF`.
Logic copy first copies the base, then performs the corresponding member setup
and copy. The native member-field initialization order is retained separately
for these two entries.

| Native owner frame | Cleanup map | Scope |
| --- | --- | --- |
| `00411700`, FuncInfo `00D83F74` | State 0 -> `00C5E010` -> `00BF6454` | Completed base cleanup when member copying throws |
| `004118D0`, FuncInfo `00D84024` | State 0 -> `00C5E050` -> `00BF6454` | Same cleanup, armed only after base copying returns |

Both owner unwind maps destroy **only the base**. They do not add a member-string
destructor after a failed copy. Whatever current member state the shared SBO
helper leaves on failure remains. A throwing secure-copy handler during base
copying occurs while the owner's state is still -1, so it receives no owner
cleanup and member initialization has not yet happened. The implementation puts
base copying outside its member-copy catch scope to preserve that distinction.
Length-error copy publishes its derived table only after the whole logic copy
returns successfully.

## Returning free and scalar deletion

Ordinary destruction publishes the logic table, uses the existing string
destructor to release any heap buffer, then resets member capacity/length/first
byte after that free returns. Native `00411798` is a real continuation despite
the false no-return annotation on `_free`.

Base destruction tests the current ownership DWORD before publishing the base
table. A nonzero value frees the current base message, including null. It does
not clear the message or ownership fields afterward. Thus a member-free callback
can change the base state consumed by the following destructor, and a base-free
callback's final field changes survive the return.

Scalar deletion performs the same complete destruction, then checks flags bit 0.
Other bits do not request owner disposal. If set it calls the existing concrete
`singleton_lifetime_free` on the actual owner, executes the post-free continuation
at `004117FE`, and returns the original address even though its allocation has
ended. Ordinary destruction leaves the owner allocation to its caller.

## Verification and limits

Current `bsp.py ghidra bytes` reads verified project
`C:/Users/sqz269/bsp.gpr` and program `/battlestationspacific.exe` before every
batch. Thirteen complete code spans, eight dependency entry preimages, five data
spans and eight remaining-factory support spans matched the installed PE with
SHA-256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The eight existing seed checks also passed before native execution.

`scripts/build.ps1` passed `/W4 /WX /fp:strict` and CTest 2/2 in the isolated
worktree. An ignored local CMake include registered only the owner source. The
shared string code/header came from the integrated dependency commits, including
the raw-storage correction.

One ignored fixture, `local/native_exception_owner_check.cpp`, compares the
complete five installed bodies, all three base library bodies and original
`strcpy_s` with the compiled production implementation. Both use the actual
shared SBO helper. The original owner frames retain their two verified FuncInfo
maps and cleanup funclets, with relocated map pointers and host handler entries
that dispatch the real host `__CxxFrameHandler3`. This tests native cleanup
selection for host-thrown test exceptions; it does not establish full native
throw ABI compatibility.

The matching 463-word sequence includes raw 40-byte preimages, inline/heap
construction, borrowed/owned base copying, captured ownership and reloaded source
message around allocation, an injected null malloc return, returning member/base
free mutations, scalar flags 2 and 3 with actual owner disposal, member-copy
exceptions selecting base-only cleanup, failure before state-0 activation, and a
returning secure-copy handler whose repairs persist. It checks four invalid
handler invocations and nine observed frees, including one `free(nullptr)`.
The fixture intercepts only the isolated executable's CRT import slots during
tested operations, performs real allocations/frees except for the explicit null
failure injection, and restores those slots afterward. Trace-buffer allocations
are excluded from observation. No installed file or game process is modified.

This does not claim native allocator internals/new-mode configuration, every
malformed alias case, concurrency, full binary exception interoperability or game
execution. Throw metadata/table dispatch and the exception's `what` entry remain
outside these five raw-storage operations. No tracked test suite was added.

## Remaining error factories

`004CE780` remains separate: it checks unsigned `1FFFFFFF - current_count` against
the increment, constructs the sixteen-byte text `list<T> too long`, constructs
this 40-byte logic owner, publishes the length-error table and passes native
ThrowInfo `00D83F98` to `00BF6885`. Its temporary-string cleanup becomes active
only after counted assignment returns. A later reconstruction must preserve
that scope and the original unsigned arithmetic, including already-corrupt counts.

`00BF5695` and `00BF56D4` additionally depend on the still-separate `00407290`
C-string constructor. They build `string too long` or `invalid string position`,
use the logic owner constructor, publish length-error or out-of-range table data
and invoke the native throw machinery. Native `00BF6885`, ThrowInfo/catchable-type
handling and the out-of-range derived operations are not supplied with a fake
throw bridge here. Ghidra function creation/annotation/export refresh, source
registration and ledgers remain with the primary integrator; `00411940` had no
defined Ghidra function when this packet captured its verified body.
