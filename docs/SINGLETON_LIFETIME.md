# Concrete singleton lifetime manager

`SingletonLifetimeDomain` implements the manager shared by startup and lazy
singleton constructors. `ConcreteSingletonLifetimeManager` implements the
existing `SingletonLifetimeManager` interface and exposes the same owned Win32
critical section through `SystemSingletonLifetimeOwner`. One domain must be
shared by all those callers. The new classes are typed C++ interfaces, not native
layout or ABI replacements.

## Recovered behavior

| Native address | ABI and implemented behavior |
| --- | --- |
| `00415350` | No input consumed, EAX manager, RET. Read `01090AA0`; if null, allocate native 14h bytes, construct, publish, return captured result. No publication lock. |
| `00BD0960` | ECX manager, EAX same object, RET. Native +0 untouched; zero begin/end/capacity at +4/+8/+C, reserve 256 pointers, then create section stored at +10. |
| `00BD0600` | ECX vector, stack capacity, RET 4. Constructor uses the empty-vector reserve branch. Generic reserve on an already populated vector is not a separate exposed implementation. |
| `00BD1860` | No inputs, EAX tracked section, RET. Allocate 1Ch, initialize the Win32 section, then zero its signed-depth bit pattern at +18. |
| `00BD0C30` | ECX manager, stack object, RET 4. Validate unsigned end >= begin before the null check; ignore null; append nonnull. No lock, AddRef, or duplicate filtering. |
| `00BD0BC0` | ECX vector, stack pointer-to-value, RET 4. Write and advance end when there is capacity; otherwise enter checked insertion. |
| `00BD08D0` / `00BD0700` | ECX vector, four stack words, RET 10h. Append captures the pointed-to value before moving storage; growth is max(size+1, capacity+capacity/2), with native 3FFFFFFFh limit. Preserve native returning validation calls. |
| `00BCF910` | ECX vector, EAX count, RET. Null begin returns zero; otherwise SUB followed by signed SAR 2. |
| `00BCFCA0` | ECX manager, stack object, RET 4. Null is ignored; scan forward and zero the first matching slot. Keep vector length and other duplicates. |
| `00BD0400` | ECX manager, RET. Validate the last iterator, load owner, pop before owner vtable slot 0 with flag 1, then reload count. Skip null holes. Delete the lock after draining, free slots, clear all three slot pointers. |
| `0041CC80` | ECX points to section pointer, RET. Decrement positive signed depth and call LeaveCriticalSection until zero; delete section, free it, then clear owner pointer. |
| `00BCFEB0` | ECX slot count, EAX raw storage, RET; EDX is set to zero by callers. Check count*4 overflow, then call allocating helper. |
| `00BD0500` / `00BD0560` | Three stack arguments, EAX end, RET Ch. Nonempty memmove_s copy and repeated pointer fill, used by insertion. |
| `00BF681B` / `00BF65AC` | cdecl allocation/free boundaries. Allocation loops malloc and __callnewh(size), throwing bad_alloc if the handler declines; free is a CRT thunk. |

The slow append path specializes checked insertion to the same vector owner
passed by `00BD0BC0`. Foreign-iterator ownership failures and standalone generic
middle insertion are outside this interface. Validation functions are ordinary
returning calls. A returning handler does not cause an invented early exit or
exception; the native continuation can still access invalid storage. The focused
comparison exercises the safe end-before-begin plus null-registration case.

## Concrete owner and allocation boundary

The domain requires a real `destroy_registered(context, owner, flags)` callback.
It receives the exact pointer previously registered and native flag `1`; there
is no default destruction callback. That callback must dispatch the actual
owner's deleting destructor and its real unregister/free operations. Whether an
owner unregisters is determined by that owner's implementation. The manager
does not add an unregister operation to every destructor. The typed callback is
`noexcept`; exceptional native deleting-destructor unwinds are outside the
implemented contract.

`singleton_lifetime_allocate({kind, native_bytes, host_bytes})` returns raw,
uninitialized `malloc(host_bytes)` storage. On failure it uses the actual host
CRT `_callnewh(host_bytes)` retry protocol and throws `std::bad_alloc` if declined.
`singleton_lifetime_free` uses matching `free`. The size adaptation is explicit:
native manager 14h becomes the C++ manager size, native tracked section 1Ch
becomes the lock plus projection metadata, and each pointer remains four bytes.
`native_bytes` and `kind` describe the caller's allocation boundary; they do not
select a second heap or initialize bytes. This helper can allocate the particle
clock while retaining its native +18h preimage.

The owned lock embeds an actual x86 `CRITICAL_SECTION` followed immediately by
the same `recursion_18` storage seen by the system-time projection. Compile-time
checks enforce section size 18h, counter offset 18h, and four-byte pointers.
The owner projection's `native_owner` is the concrete manager; `section_10`
references its retained pointer to this lock's projection. Manager `lock()` and
`unlock()` adjust the counter themselves. The free functions
`singleton_enter_critical_section` and `singleton_leave_critical_section` perform
only OS operations because the particle constructor adjusts the captured
counter itself.

`shutdown()` preserves publication while callbacks run, then frees the manager
and clears the publication slot in the WinMain order at `008F8449..008F8463`.
Callbacks can therefore obtain the same manager and append or unregister while
the vector drains. Shutdown requires the owning/quiescent thread: the native
lock destroyer forcibly leaves its positive tracked depth. The getter itself is
unsynchronized, matching the native missing publication lock; concurrent C++
publication and recursive domain shutdown are not supported.

The earlier surface-registration audit remains applicable: caller `00B3E730`
checks singleton `0108FEDC`, obtains the lifetime manager, enters its section,
rechecks, constructs its eight-byte owner through `00B61D50`, publishes and
registers it, then decrements depth and leaves. No surface pointer is passed
to that helper. Singleton shutdown registration is distinct from the renderer's
live resource reset list; `SURFACE_REGISTRATION_AUDIT.md` retains that ownership
evidence.

## Evidence and validation

All live queries verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, language and image base through `bsp.py ghidra`.
The audit records original bytes, installed-PE comparisons, address/name/comment
preimages and naming proposals. No Ghidra annotations or shared metadata were
changed by this packet; the integrator owns those edits.

Assembly resolves misleading decompiler early returns after `_free`: vector
reserve/insertion/destruction continues with pointer updates. Raw bytes
`0041CCB3..0041CCBC` are `83 c4 04 c7 45 00 00 00 00 00`, proving stack cleanup and
the post-free owner-pointer clear that the incomplete Ghidra body omits.
`00BF6713` can return, as independently established in
`reports/system_empty_light_handler_review.json`.

MSVC Win32 `/std:c++17 /permissive- /W4 /WX /EHsc /fp:strict /O2` compiled this
source and its ignored local fixture. `scripts/build.ps1` passed the existing
build, reconstructed-math and native-math differential checks (2/2). This worktree's CMake integration remains
the primary integrator's responsibility, so the new source was compiled and
linked separately. `verify-seeds` passed before the final fixture execution.

One focused fixture copied 15 live/disk-matching native code ranges into isolated
executable memory, preserved relative calls, relocated absolute operands and
bound allocation/free/memmove/validation/Win32 imports to controlled host
services. It compared native and reconstructed owner IDs, flag 1, pop counts and
lock depth across 260 actual deleting callbacks. It exercised stable lazy
publication, initial 256-slot reserve, growth to 384, duplicate registrations,
unregister holes, null registration after a returning handler, registration
during a destructor, and teardown. Native free order and post-free clears were
also checked. The C++ manager and particle projection entered the same actual
Win32 critical section.

The fixture does not establish allocator exhaustion/new-handler behavior,
native SEH cleanup identity, concurrent publication, arbitrary corrupt iterator
states, binary drop-in compatibility, or game runtime behavior. Actual
registered-object destructor adapters remain required; this packet supplies
the manager and real host services, not unrelated singleton implementations.
