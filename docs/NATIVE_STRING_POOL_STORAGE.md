# Actual native string-pool allocation and return

Packet `orch3_native_string_pool_storage_m`, 2026-09-11. Source:
`src/native_string_pool_storage.cpp`; storage layout and ownership:
`native_string_pool_owner.hpp`, `docs/NATIVE_STRING_POOL_OWNER.md`.

The three complete bodies now operate on the recovered `0x8AD4A0` owner and
its embedded ring. `ActualNativeStringPoolStorage` supplies the existing narrow
and wide string APIs with this owner through the application's one publication
slot, shutdown gate and canonical lifetime domain. It calls the getter on every
allocation and release, including large requests and disabled small returns.
There is no cached pool pointer or second lifetime manager. The registered owner
uses `NativeStringPoolLifetimeBinding` to invoke the rebuilt deleting destructor.

Descriptive symbols are hypotheses. These typed C++ interfaces are not drop-in
binary replacements, and isolated function evidence is not game validation.

## Native spans and ABI

| Body | Inclusive span | Original interface |
|---|---|---|
| Allocate | `00BD1120..00BD11D1` | ECX owner; stack size, unread word; RET8; EAX block |
| Return block | `00BD1510..00BD156E` | ECX owner; stack block, size, unread word; RET0C |
| Push small block | `00BD12A0..00BD1374` | ECX embedded ring; stack address of block, class; RET8 |

The saved Ghidra prototypes omit register arguments. Assembly establishes the
interfaces. Live Ghidra bytes were compared with the installed PE for all three
complete spans; lengths and SHA256 values are in the report. The eight bytes at
`BD12D8` (`8D A4 24 00 00 00 00 90`) are unreachable alignment after the jump
at `BD12D6`, not a missing fall-through after a call.

## Allocation and synchronization

`BD1125..BD113D` sends unsigned sizes at least 150 directly to `BF9F1A` malloc.
That branch does not dereference the owner and preserves a null allocation
result. The unused native word is not interpreted. The reconstruction binds
the current CRT allocator, with no added new-handler or exception policy.

For smaller sizes, `BD1140..BD114D` enters the actual critical section at
owner `+8AD484` and increments the DWORD at `+8AD49C`. The ring starts at
`+6ACFCC`, with `0x80000` pointer slots, 150 heads and 150 tails. Empty means
`head[size] == ((tail[size]+1)&0x7FFFF)`.

An empty class returns owner `+8+bump` and adds size to the DWORD bump at
`+6ACFC8`. Zero size does not advance it. There is no alignment or arena bound
check. Integer pointer arithmetic preserves native DWORD wrapping; callers
remain responsible for valid memory before dereferencing a result.

The nonempty branch decrements live count before reloading the class tail,
captures that slot's pointer, and stores `(tail-1)&0x7FFFF`. Both branches
decrement the tracked lock depth before leaving the captured section and return
the captured pointer. No new RAII unwind cleanup is inserted into these bodies.

## Return and ring displacement

`BD1515..BD1531` frees large blocks before reading either owner or shutdown
gate. Small returns first read the actual volatile `01090AA4` word. Any nonzero
value returns without touching the owner. Otherwise the function enters the
embedded section, increments tracked depth, passes the address of its captured
block argument to `BD12A0`, then decrements depth and leaves the section.

The ring push captures `*block` at `BD12A5` before changing any ring field. It
increments the selected tail modulo `0x80000`, then walks subsequent classes
modulo 150 while that tail collides with the next head. A nonempty next class
exchanges its head slot with the carried block. Each traversed class reloads and
increments its head and tail. The walk stops on a noncollision or return to the
original class, then stores the carried pointer at the last tail. No capacity
guard, extra head movement or allocator is introduced.

Live count increments as a DWORD. `BD136A` uses signed JLE to decide whether
to replace peak with the newly captured live value, including after overflow.
Class values outside 0..149 and invalid slot indices are outside the caller
contract; the reconstruction does not add recovery for them.

## Validation and boundaries

The strict Win32 build and both existing CTests passed with all sources
registered. One ignored fixture compares copied original allocation, return
and ring-push instructions with the reconstructed functions. Only relative
malloc/free/push calls, Windows import cells and the actual gate operand are
rebound; the report enumerates them. It compares returned arena offsets and
every ring byte, plus bump and tracked depth. Cases cover zero/small requests,
unchecked bump wrap, LIFO reuse, occupied/empty neighbor displacement, class
and slot wrap, full ring traversal, input-slot alias, signed peak overflow,
nonzero gate, large malloc/free, null free and failed large malloc.

A composition probe constructs narrow and wide strings through the actual
owner, observes small-block reuse, and drains its real registered deleting
callback through the shared manager. A subsequent small release recreates and
registers the owner before reading the still-disabled gate. Construction never
resets that gate. No permanent test files or framework were added.

Current CRT/Windows behavior is the binding boundary. Original CRT errno,
new-handler, exception and process-wide thread policies are not reimplemented.
The existing `NativeStringStorage::release` interface is `noexcept`; normal
returning-getter behavior is covered, while a C++ failure during lazy owner
recreation terminates under that interface. Native EH/SEH ABI parity is not
claimed. The low-level APIs retain their separate contracts.

This packet supersedes the allocation-layer boundary in the owner packet and
the native-pool-binding boundary in the wide-string packet. Startup consumers
still need the application's actual header/global ownership integrated; the
runtime's existing `std::string` language projection is not silently replaced.
No game entry was executed or installed game file modified.
