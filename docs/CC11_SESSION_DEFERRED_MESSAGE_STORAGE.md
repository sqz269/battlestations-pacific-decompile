# Deferred session-message node storage

This packet reconstructs five complete storage routines used by the real
deferred-create list at `00F871A0`. They construct a sentinel, allocate nodes
from raw message-pointer cells, check count growth, and destroy the list's
nodes and sentinel without destroying the pointed messages.

| Entry | Whole bytes | Native contract |
|---|---:|---|
| 0077BAC0 | 51 | ECX unused; stack next, previous, message-pointer cell address; RET12, EAX node |
| 0077DDA0 | 147 | ECX actual owner; stack unsigned increment; RET4 |
| 00780830 | 26 | No arguments; RET, EAX self-linked sentinel |
| 00780860 | 72 | ECX actual owner; no arguments; RET |
| 007808C0 | 22 | ECX actual owner; no arguments; RET, EAX owner |

All **318 original bytes** match the live program and installed executable.
Exact hashes, full listings, native call rows and ABI details are in the
report. The Source interfaces take explicit cdecl arguments and do not claim
drop-in register-ABI compatibility.

## Actual owner and payload contract

The owner accesses head at `+4` and count at `+8`; unknown `+0` is untouched.
A node is 12 bytes: next, previous, raw message pointer. The sentinel's payload
word is left untouched. Node construction reads the supplied pointer **cell**
after allocation, rather than storing the cell address or capturing its value
before the allocator. The Source retains the native 32-bit pointer guards.

The real static initializer at `00CD04C0` calls `00780830`, publishes head
`00F871A4`, zeros count `00F871A8`, and registers exit stub `00CDD8F0`, which
jumps to `00780860`. These raw initializer/exit sites are evidence, not newly
defined Ghidra functions. The initializer does not call `007808C0`; that
authentic constructor body independently performs the same owner publication.

Destruction captures the first node, resets the sentinel links and count,
then captures each next pointer before freeing the current node. It compares
against freshly loaded owner head after each free, frees the current sentinel
last, and clears owner head afterward. It never reads or destroys payload
messages. All allocation/free calls use fixed existing real services.

## Count provider equivalence

`0077DDA0` forwards to existing complete `004CEE30` Source only after verifying
the full 147-byte native bodies agree except for the handler address and
three direct call operands. Both access actual owner `+8`, use the unsigned
`3fffffff - initial_count < increment` predicate, and store the captured
initial count plus increment on success.

The complete 36-byte FuncInfo records agree after rebasing the unwind map.
Both maps have one state leading to -1, and both complete eight-byte funclets
compute `[EBP-50h]` then jump to `004072D0`. The error calls share the same
counted-string, logic-error constructor and native throw targets. Fresh Source
COFF confirms the direct canonical binding, actual field offset, bound and
normal store. The existing owning Source length-error/cleanup implementation
is unchanged; its host exception ABI and RTTI remain different from native.

## Connected fixture

One fixture passes **164 checks with zero failures**, with two nodes per world.
Original and Source construct genuine heap sentinels and nodes holding real
constructed zero/one messages. Node calls receive actual message-pointer cell
addresses. Checked increments take count from 0 to 1 to 2. The fixture then
performs exactly the original caller's two publication stores at `007806DD`
and `007806E3` as explicit setup, without claiming category-47 dispatch.

Complete nonempty destruction clears owner head/count and preserves all
message bytes. The fixture checks both ring directions and pointer-cell value
semantics; whole surrounding owner bytes including unknown +0; normalized
owner agreement; the actual constructed game/session; its separate incoming
allocation and real OS lock. Payloads are subsequently freed through genuine
message scalar profiles. No substitute operation callback, profile, arena,
manager, or default service is used.

The original RX suite retains all five complete bodies. Only five natural
direct call operands are relocated: two allocation calls, two free calls,
and constructor-to-complete-original-sentinel call. All **298 other bytes**
are checked before and after. The count body retains all **147 bytes without
changes** and restores the original exception chain on the exercised success
path. Its native error calls/handler paths are neither rebound nor executed;
their verification here is structural, not a native unwind execution claim.

## Ghidra tail limitation

The primary used the supported locked tool to decode the returning-free gap
at `00780888..00780892` and the 12-byte tail at `0078089C..007808A7`. However,
the stored function body still ends at **0078089B**. The final cleanup and
owner-head clear are decoded but outside stored membership. This is **not
full Ghidra flow closure**. The report preserves the primary repair history,
live prototype/listing, exact whole 72-byte live/disk agreement and full
Capstone listing. The fixture executes the whole 72-byte body, including the
tail. The worker made no Ghidra changes.

## Build and limits

The strict manifested MSVC Win32 build uses 13 compilation units: the fixture
and 12 fresh repository TUs. Compiler include tracing verifies 24 repository
headers, giving 37 pinned inputs. Flags include `/O2 /W4 /WX /fp:strict /MD
/EHsc`, with an embedded asInvoker manifest. Rebuilt main `b7f1bb6c9` support
libraries were frozen with matching pre/copy/post hashes before linking.
Final input, support and original-image pins pass; preceding append and
broadcast artifact sets remain unchanged. Primary integration owns CMake
registration and the full-main build. No tracked tests were added.

The executable domain is ordinary valid storage and successful count growth.
It does not include failure injection, corrupt/wrapped storage, concurrent
mutation, native private CRT exception execution, complete category-47
dispatch, either session consumer, original process-global registration,
full game/session destruction, or game-runtime validation.
