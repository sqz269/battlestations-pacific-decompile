# Complete session target history initialization and scalar lifetime

Packet `cc11_health_target_history_lifetime` reconstructs two complete ordinary
bodies. All ends below are **exclusive**, including the entire return instruction:

| Body | Range | Bytes | Original ABI |
| --- | --- | --- | --- |
| Initializer | `[00782F40,00782FD8)` | 152 | ECX history; stack count, float word; RET8 |
| Scalar destructor | `[00783970,0078399B)` | 43 | ECX history; stack flags; EAX captured history; RET4 |

The initializer's native return value is not established; the source interface is
`void`. Both source functions use new interfaces, not native replacement ABIs.

## Actual storage and services

The observed target constructor `007839D0` allocates `18h` bytes for each history.
The source storage preserves its six four-byte fields: profile `00`, count `04`,
initial float word `08`, sample-array pointer `0C`, index `10`, and total float word
`14`. Float representations and sample elements are raw DWORDs; initialization
copies their bits without a floating conversion. Storage has no defaults or
automatic lifetime operations.

`00BF55BE` is exactly `JMP 00BF681B`, and `00BF6989` is exactly `JMP 00BF65AC`.
The implementation reuses `singleton_lifetime_allocate` with
`{SingletonAllocationKind::object, bytes, bytes}` and `singleton_lifetime_free`
from `src/singleton_lifetime.cpp`. These existing services implement the host
`malloc`/`_callnewh` retry-or-throw boundary and matching `std::free`. No new
allocation domain, arena, lifetime manager or default callback is introduced.

The initializer preserves the profile and stores the original count/initial word
before allocation. Its unsigned multiplication by four saturates an overflow to
`FFFFFFFF`; it neither throws a separate overflow exception nor skips a zero-byte
allocation. It publishes the returned array and fills it using the original
arguments, signed loop comparisons, four-element unrolling and a remainder loop.
Every sample write reloads the actual array pointer at `+0C`.

The final instructions remain `FILD` of the original signed count, store index
zero, `FMUL` of the original float argument, and `FSTP` directly to the total word.
There is no intermediate host float/double, SSE conversion, control-word change or
read of mutable member count/initial fields for that product.

No null guard, catch or rollback is added. A thrown allocation leaves the new
count/initial fields and the remaining preimages. Reinitialization does not free
a previous array. These schedules are source/assembly evidence, not injected
failure-test results.

The scalar destructor first captures array `+0C`, then writes numeric profile
stamp `00D04268` and frees that captured array. It tests only flags bit zero,
optionally frees the history itself, and returns the captured history value even
after self-release. It clears no fields. Self-release requires a matching
canonical allocation; the caller must finish consumers first. The numeric stamp
is not an executable translated profile.

## Native evidence and analysis limitation

The complete initializer has 48 instructions; its allocator call is
`00782F69 -> 00BF55BE`. The scalar has 15 instructions, including array capture at
`00783973`, profile stamp at `00783977`, array release at
`0078397D -> 00BF6989`, optional self-release at `0078398D -> 00BF65AC`, and RET4 at
`00783998`. Full scalar disk bytes match a current live 43-byte read.

Stored Ghidra scalar body metadata still ends at `00783981` inclusive. The primary
decoded the returning-free tail, but did not extend the stored body or enable
bridge script execution. See
`reports/transport_buffer_destructor_flow_recovery_cc11.json`. The optional
self-free CALL is therefore recorded separately as complete raw-byte evidence,
not as a successful stored-body CALL check. No full flow repair is claimed.

## Focused verification

The ignored fixture freshly compiles the actual history and canonical allocator
translation units under MSVC Win32 `/O2 /MD /W4 /WX /fp:strict`, with an embedded
`asInvoker` manifest. It copies all 152 original initializer bytes from the
read-only original PE into isolated memory. Only the four-byte displacement of
the sole allocator CALL changes, to a cdecl adapter that forwards the raw byte
count to the same existing canonical source service. Internal branches and the
complete RET8 remain intact. This compares normal bodies using the host CRT
boundary; it does not execute the original allocator or prove native EH/ABI.

All **229 checks pass**. There are 27 complete original/source comparisons:
24 combinations of PC24/53/64, four rounding controls, and empty/three-occupied
x87 stacks, using count 50 and float word `3EAAAAAB`; plus the observed constructor
input `50/+0`, a remainder-only `3/-0`, and `0/-0` with a real zero-byte allocation.
Each comparison checks profile preimage, original input words, all sample words,
index, total float bits, control/status/tag state, all eight 80-bit x87 register
images and MXCSR. Instruction/data pointer addresses are excluded because the
two code/storage locations differ. Exceptions are masked and stack capacity is
available; no negative/overflow allocation, NaN or unmasked-exception matrix is
claimed.

Each source array is explicitly released with flags bit zero clear; other raw
field bytes remain unchanged apart from the numeric profile stamp. One separately
allocated `18h` source history exercises flags bit zero set and captured identity
return without reading freed storage. Release ordering is additionally confirmed
in the generated assembly; no allocation/free interception replaces the services.

All ten source/header snapshot and workspace inputs and all three copied/main
support libraries match before and after linking. The report records their
SHA-256 values, original byte hashes, artifact paths and verification boundaries.

The whole target constructor, target profiles and virtual dispatch, tracked lock,
shared slot mask, derived publication and lifetime, historical CRT/private EH,
concurrency, native replacement ABI and gameplay remain outside this packet.
Primary integration and registration at `e702215a6966f81cc11b3b22407966edb9a8d3dc` passed the full Win32 build and all three existing CTests. Its independent fresh history/allocator/probe compilation pinned 11 current-main source/header inputs and three current-main libraries before and after linking. The manifested x86 probe repeated all 229 checks and 27 complete original initializer comparisons successfully. Four stored-body CALL/JMP checks passed again; the optional self-free remains supported by complete raw/live bytes because stored Ghidra metadata is still truncated. Native CRT, enclosing target lifetimes, original ABI and gameplay remain unverified.
