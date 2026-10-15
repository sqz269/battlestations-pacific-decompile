# Actual DWORD-vector allocation and length failure

| Native entry | Source API or mapping | Span | Coverage |
| --- | --- | --- | --- |
| `00466CE0` | `allocate_native_scene_dword_vector_00466ce0` | `[00466CE0,00466D35)` 85B/25ops | complete ordinary Source |
| `0046A400` | `throw_native_scene_dword_vector_length_0046a400` | `[0046A400,0046A469)` 105B/23ops | complete ordinary Source |
| `00C61C50` | canonical completed-message guard | `[00C61C50,00C61C58)` 8B/2ops | complete action semantics; qualified Source mapping only |

The two entries had no indexed or unindexed whole Source before this packet.
Their correct library-role names are retained; DWORD-vector use is descriptive,
not a recovered class or payload type. The cleanup remains a compiler action.
This packet supplies 190 new ordinary-function bytes of coverage and separately
records eight cleanup bytes; no new binary action or private-frame ABI is claimed.

The allocator consumes ECX unsigned count, no incoming EDX or stack input,
and returns raw EAX with plain RET. Zero still reaches actual allocation with
zero bytes. For a positive count the native unsigned FFFFFFFF/count comparison
admits multiplication only through 3FFFFFFF. The admitted endpoint requests
FFFFFFFC bytes and remains subject to the actual current allocator policy.
Larger values construct a null-message exception base in a 12B local payload,
publish D6923C and invoke the native E03CC0 bad_alloc throw boundary. There is
no local EH record or invented cleanup branch. The entire original instruction
sequence matches accepted 00BCFEB0 except three rel32 operands; all three
targets also match. Source delegates to its complete real implementation, whose
current-CRT std::bad_alloc transport is already qualified. It does not newly
pretend that the game's exception/RTTI/static-CRT identity is implemented.

The length helper has no consumed input or normal return. Its whole body
matches accepted 00BD0590 after relocating three rel32 operands and the EH-handler word.
It leaves the leading DWORD of the uninitialized 1Ch temporary untouched,
sets capacity15, length0 and its first inline byte to NUL, then assigns the
18-byte `vector<T> too long` message through actual 00408720. Only after that
returns does it arm state0. Actual 00411700 constructs the 28h payload; native
D69260 is published before D83F98/BF6885 throw. The accepted Source provider
throws owning NativeSingletonVectorLengthError, preserves the actual payload
construction/copy/destruction and does not replace it with std::length_error.

Native handler C61C58 loads FuncInfo D89358 (magic19930522, one state, unwind
map D89350). Its state0 transitions to -1 through C61C50, whose full two-op
body is LEA ECX,[EBP-50h]; JMP004072D0. The canonical CC5470 action has that
same body, so the canonical guard destroys the completed message on payload
construction failure and exception unwinding, while a failed counted assignment
has not armed that outer guard. No Source callable routine is invented at
C61C50; this is qualified reuse of compiler-action semantics through host C++ EH.

| Containing routine | Native listing | Contract |
| --- | --- | --- |
| `00466CE0` | `00466CF1 CALL 00BF681B` | one byte-size argument, ADD ESP,4; includes zero-byte request |
| `00466CE0` | `00466D19 CALL 00BF6340` | ECX exception receiver and one null-message slot pointer, callee RET4 |
| `00466CE0` | `00466D30 CALL 00BF6885` | payload/E03CC0 throw information, no normal return |
| `0046A400` | `0046A438 CALL 00408720` | ECX actual temporary plus text/count18, callee RET8 |
| `0046A400` | `0046A44D CALL 00411700` | actual28h receiver plus completed message, callee RET4 |
| `0046A400` | `0046A464 CALL 00BF6885` | payload/D83F98, no normal return |
| `00C61C50` | `00C61C53 JMP 004072D0` | tail transfer, ECX actual native [EBP-50h] temporary; Source guard mapping only |
| `0046AFB0` | `0046AFF9 CALL 0046A400` | unsigned remaining-capacity check; no arguments/no normal return |
| `0046AFB0` | `0046B05B CALL 00466CE0` | ECX computed growth count, explicit EDX0; plain RET/raw allocation |

These are native transfers, not calls from the owned entries to BCFEB0 or
BD0590. Source composition is separately recorded. Both live incoming sites
were inspected; caller ownership and the full insertion implementation remain
with the primary. Concrete current malloc/_callnewh/retry/throw and owning
legacy message services are existing accepted Source; no new callback, stub,
valid-only adaptation, heap domain or generic exception substitutes for them.

Guarded live batches verify C:/Users/sqz269/bsp.gpr and
/battlestationspacific.exe, x86 language and image base. Full live/installed
bytes, relocation masks/decoded targets, prior annotations and Source hashes
are recorded in JSON. Mutations use the shared write lock, preserve old values
and correct names/comments, save and refresh owned exports. Initial complete
Source status awaits required normal Win32 build and existing checks. No new
tests are added. Original static-CRT errno/encoded-handler/heap/exception RTTI,
ThrowInfo/FH3/SEH/private-frame/register/binary identity, direct candidate
differential, application admission and game validation remain unproved.
