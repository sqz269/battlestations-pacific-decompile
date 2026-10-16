# Native DWORD-vector header allocation and length error

This packet completes the three actual entries below through accepted Source
services with exactly matching consumed contracts. Existing library names,
preimages and earlier typed/Hook projections are preserved. The whole owner
insertion at 00698400 remains separate.

| Entry | Native identity | Complete bytes / starts | New Source |
|---|---|---|---|
| 00696890 | STL_allocator_allocate | 81 / 25 | Same-count raw allocation delegate |
| 006978D0 | STL_xlen_throw | 105 / 23 | Full owning legacy error delegate |
| 00C7EBA0 | Unwind | 8 / 2 | Explicit existing completed-message cleanup |

Implementation: `src/native_dword_vector_header_allocation.cpp`; interface:
`include/bsp/native_dword_vector_header_allocation.hpp`.

## Allocation contract

Original ECX is an unsigned element count; EDX and the stack are unconsumed.
The normal return is raw EAX with plain RET. Count zero still requests zero
bytes. For positive count, unsigned FFFFFFFF/count must be at least16; admitted
count is at most0FFFFFFF and the maximum byte request isFFFFFFF0. The request is
count*16. No extra host-size limit or zero shortcut is introduced.

Every byte of the complete81-byte body matches accepted00553300 after
normalizing only the three relative CALL operands, whose actual targets are
identical. The new delegate passes the SAME uint32 count and unused EDX to
`allocate_native_nested_string_vector_00553300`, returning raw void*. That
accepted service uses the actual current object allocation/free domain. It
does not inspect string content or expose a typed string/container allocator.

Using the four-byte553260/BCFEB0 service with multiplied count would change
the independent entry's overflow and count contract; it is not this binding.
Original BF6340/D6923C/E03CC0 allocation-error payload, static CRT heap and
global new-handler identity remain outside the accepted CurrentCpp boundary.

## Owning error and selected action

Original006978D0 has no consumed input or normal return. All105 bytes match
accepted00556C90 except the independently verified native EH-handler address
and three relative CALL operands with identical targets. The literal is18
ASCII characters, `vector<T> too long`, followed by one NUL:19 stored bytes.

Native initializes the actual1Ch SBO temporary with capacity15, length0 and
inline NUL, then performs counted18-character assignment. Only after it
succeeds is state0 armed. Actual00411700 constructs the owning28h legacy
logic-error payload. The length-error wordD69260 and throw metadataD83F98 are
identical to the accepted helper. The new Source delegate invokes the COMPLETE
`throw_native_nested_string_vector_length_error_00556c90` service. It adds no
temporary, generic std::length_error or second cleanup around that service.

The complete44-byte native EH region begins DAB8CC. Its unwind record is
previous-1/actionC7EBA0. FuncInfoDAB8D4 has magic19930522, maxstate1, mapDAB8CC,
no try/catch or IP/type list, and flags1. Accepted reference D97B7C/D97B84 has
the same fields with its own state0 actionC6D7B0. Both actions LEA ECX from
EBP-50h and tail-jump to the SAME actual004072D0 destructor. Both handlers
select the matching FuncInfo and tail-jump to the native frame dispatcher.

The explicit Source action accepts an EXISTING completed
`NativeLegacySboStringStorage&` and invokes actual
`native_legacy_sbo_string_destroy_004072d0` once. It neither constructs nor
copies a message and exposes no EBP/frame function. This separate action is
not invoked around the full throwing delegate, whose completed-message
ownership already includes cleanup. The destructor's accepted current
noexcept/heap behavior is inherited, not a new exception bridge.

## Exact transfer evidence

| Exact native instruction | Scope |
|---|---|
| `0069689D CALL 00BF681B` | Owned allocator |
| `006968C5 CALL 00BF6340` | Owned allocator overflow |
| `006968DC CALL 00BF6885` | Owned allocator throw |
| `00697908 CALL 00408720` | Owned counted message assignment |
| `0069791D CALL 00411700` | Owned legacy payload construction |
| `00697934 CALL 00BF6885` | Owned throw |
| `00C7EBA3 JMP 004072D0` | Owned selected cleanup |
| `00C7EBAD JMP 00BF6B43` | Native handler support only |
| `0055330D CALL 00BF681B` | Accepted allocator reference |
| `00553335 CALL 00BF6340` | Accepted allocator reference |
| `0055334C CALL 00BF6885` | Accepted allocator reference |
| `00556CC8 CALL 00408720` | Accepted helper reference |
| `00556CDD CALL 00411700` | Accepted helper reference |
| `00556CF4 CALL 00BF6885` | Accepted helper reference |
| `00C6D7B3 JMP 004072D0` | Accepted action reference |
| `00C6D7BD JMP 00BF6B43` | Accepted handler support |
| `006984CD CALL 00696890` | Bounded unowned producer |
| `0069846C CALL 006978D0` | Bounded unowned producer |

The allocator producer clears EDX then passes computed element count in ECX.
The length producer proves the unsigned0FFFFFFF bound and no-input invocation.
These25 producer bytes/8 starts are separate from the whole parent body and
make no whole-insertion Source claim.

## Verification and boundaries

Before each Native analysis batch, BSP verified saved project
`C:/Users/sqz269/bsp.gpr` and program `/battlestationspacific.exe`. Owned code is
194 bytes/50 exact current starts. Candidate handler support is10/2, accepted
reference bodies194/50, reference support10/2, and unowned producers25/8:
total433 bytes/112 starts. All PE/live pairs match. Candidate/reference EH is
44 bytes each; the literal is19 additional bytes. Source/provider fingerprints
and full normalized-body comparisons are recorded in the matching JSON report.

The local CMake append follows COORDINATION93-95 without a registry lease or
queue. The required default MSVC Win32 build passed, with all three existing suites
passing in12.60 seconds. Six complete actual COFF checks pass: all three
functions are exactly5 bytes/one REL32 tail-JMP each, to the exact accepted
service with the same Source parameter type. No local ownership/EH/body or
count/allocator bridge appears. No new tests were added.

Accepted services provide CurrentCpp allocation, owning legacy exception and
cleanup behavior. This does not establish original heap/new-handler identity,
D6923C/E03CC0 native bad_alloc or D69260/D83F98 RTTI/transport identity, original
FH3/SEH/access-fault/nested cleanup-failure behavior, native private stack or
ambient registers/flags, binary replacement ABI or game/runtime behavior.
The Source action's explicit reference/private frame is distinct from native
EBP lookup. Static, build and emitted-code checks remain separate from native
exception execution and gameplay validation.
