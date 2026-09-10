# Native legacy small-buffer string

Six complete logical string routines are reconstructed in
`native_legacy_sbo_string.hpp/.cpp`. The storage is the native 1Ch-byte MSVC-era
character string. It is separate from the renderer's eight-byte pooled
`NativeString`. The C++ interfaces use references and explicit destruction;
they are not binary entry-point replacements.

| Entry | Descriptive role | Original ABI and body |
| --- | --- | --- |
| `00408720` | counted assignment | ECX destination, stack character pointer/count, EAX destination, RET 8; `00408720..004087E0` inclusive |
| `00408120` | substring assignment | ECX destination, stack source string/offset/count, EAX destination, RET C; `00408120..004081F8` inclusive |
| `004072D0` | destroy and reset | ECX string, RET; `004072D0..004072F5` inclusive; no semantic return value |
| `004089E0` | grow allocation and preserve bytes | ECX string, stack requested capacity/preserve count, RET 8; logical body `004089E0..00408B0F` inclusive, including both catches |
| `00408B60` | allocate character bytes | stack unsigned byte count, EAX pointer, RET 4; ECX unused; `00408B60..00408BB4` inclusive |
| `004087F0` | erase substring | ECX string, stack offset/count, EAX string, RET 8; `004087F0..00408871` inclusive |

Names are hypotheses. The first entry was already named
`BSP_CharacterString_AssignCounted`; the others had `FUN_` names. The audit
preserves every prior comment/name and the saved prototypes. No Ghidra writes
or shared ledger edits were made in this worker packet.

## Storage and ordinary behavior

`NativeLegacySboStringStorage` is statically checked for MSVC Win32: preserved
word at `+0`, a 16-byte union at `+4`, unsigned length at `+14h`, unsigned
capacity at `+18h`, and total size `1Ch`. Capacity below 16 selects inline bytes;
otherwise the first union word is the heap pointer. These routines never alter
the leading word. There is no automatic destructor, copy constructor, or
independent owner token in this raw storage type.

Counted assignment first uses unsigned address comparisons to test the interval
`[current data, current data + current length)`. A source inside that interval
calls substring assignment on the same object. This happens before the maximum
length check, so an aliased count of `FFFFFFFFh` is clamped normally. The
terminator address is outside that interval. A non-aliased count greater than
`FFFFFFFEh` throws the length error. Zero assignment clears length and writes
the current data's first byte; it keeps any heap capacity.

Substring assignment checks `source.length < offset` before clamping count to
the available suffix. For the same object it first erases the suffix after the
selected substring, then erases its prefix. For distinct objects it grows the
destination if required and resolves both current data views after allocation.
The copy uses destination capacity, excluding the terminator byte. Its CRT
status is ignored; length and terminator publication still follow a returning
validation handler.

Erasure checks the offset even when count is zero. It clamps count, calls
`memmove_s` only when something is erased, and passes capacity minus offset as
the destination size. A nonzero erase can make a zero-count CRT call. After a
returning handler it rereads the current length, subtracts the captured erase
count, and resolves the current inline/heap view for the terminator. A zero
erase returns without changing even the terminator.

Destruction frees a current heap pointer through the concrete CRT service, then
sets capacity 15, length zero, and inline byte zero. It leaves the other union
bytes untouched. The saved decompilation's no-return `_free` inference omitted
the heap-path continuation; disk assembly includes its stack cleanup and the
same reset as the inline path.

## Growth and exception continuations

The initial capacity is `requested | 15`. If this exceeds `FFFFFFFEh`, the
requested value is used. Otherwise the previous capacity can select its
one-and-a-half growth value: `rounded / 3 < previous / 2`, with the assembly's
unsigned overflow guard `previous <= FFFFFFFEh - previous/2`. Allocation asks
for capacity plus one using 32-bit arithmetic. The preserve count is a separate
argument, normally the previous length; it is not silently clamped.

The full function installs handler `00CC8890`, whose native FuncInfo is
`00E049B8`. Its four-entry unwind map is at `00E04950`; its two try records at
`00E04990` select catch `00408AE0` for the nested allocation and catch
`00408A50` for the first attempt. The catch handlers are catch-all entries.
The saved analysis splits the function at `00408A50`, `00408A78`, and
`00408AE0`; treating its entry's saved `00408A4F` end as the whole body loses
both recovery paths.

For an ordinary first allocation exception, catch `00408A50` retries exactly
requested capacity plus one. A second allocation exception frees the current
old heap if applicable, resets empty inline storage, and rethrows that second
exception. The success continuation copies preserve count bytes using capacity
plus one, frees the current old heap, stores the replacement pointer/capacity
and preserved length, then writes a terminator using the native current
inline/heap selection. The source deliberately preserves the native behavior
for direct growth calls whose arguments are outside normal assignment use.

There is a subtler copy-handler path. Native state 0 remains published during
the first copy. Before that copy, `00408A4B` has replaced the request stack slot
with the allocated pointer. If a CRT validation handler throws there, the
first catch retries using those **replacement-pointer bits as capacity**.
No replacement ownership guard exists, so the abandoned allocation is not
freed by an invented cleanup.

After fallback allocation, catch `00408A50` returns through the EH dispatcher
to `00408A72`. A subsequent copy-handler exception propagates without resetting
the old string. The raw state still reads 2, but the nested catch context has
ended. The focused native-byte execution distinguished this from a second
allocation failure: two throwing copy-handler calls left old length 5,
capacity 16, and null old data unchanged. The reconstruction keeps the fallback
copy outside the nested C++ catch for this reason. A state value alone does not
establish that the earlier catch remains active.

## Concrete runtime boundaries

Allocation uses the existing `singleton_lifetime_allocate` service for
`00BF681B`: real `malloc`, `_callnewh(size)` on failure, retry on a nonzero
handler result, and `std::bad_alloc` when exhausted. Free uses the matching
`singleton_lifetime_free` service for `00BF65AC`. Zero-byte allocation still
calls that service. In `00408B60`, the division branch testing
`FFFFFFFFh / bytes < 1` is unreachable for all nonzero 32-bit one-byte element
counts; zero takes its separate allocator call. No allocator callback or
successful-null allocation substitute was introduced.

The actual host `memcpy_s` and `memmove_s` services implement the observed
`00BF672C` and `00BF67A7` contracts. Both return immediately for count zero.
For a nonzero copy, null destination is EINVAL; a null source or insufficient
capacity clears the destination capacity bytes before the error, with null
source taking EINVAL precedence and a short destination taking ERANGE. Move
does not clear the destination on error. Both set CRT errno and invoke the
actual installed invalid-parameter handler, which may return. The calling
string operations preserve the original continuation after such a return.
The native default handler and host default handler are runtime services;
the host's thread-local handler policy and process-termination machinery are
not claimed to reproduce the game's encoded global slot/Watson ABI.

`00BF5695` passes message `"string too long"` at `00D69274` and length-error
ThrowInfo `00D83F98`; `00BF56D4` passes `"invalid string position"` at
`00D69284` and out-of-range ThrowInfo `00D863A8`. Their type descriptors at
`00E08000` and `00E0817C` establish the types. This packet uses real
`std::length_error` and `std::out_of_range` exceptions with those messages.
It does not fabricate an original `_CxxThrowException` object or metadata.
Host exception-message construction/copy/allocation remains an explicit runtime
boundary and can differ during allocation failure. The legacy 28h-byte
exception constructors, copies, and destructors at `00411700`, `00411780`,
`004118D0`, `00411940`, and their caller `004CE780` are separate work.

There is no new valid-input-only path, ignored validation callback, safe default
for unrepaired bad storage, or cleanup that changes the native branches.
Subsequent native memory accesses still require mapped, accessible storage.
This does not promise meaningful C++ behavior for arbitrary unmapped addresses,
data races, hardware faults, or arbitrary overlapping `memcpy` operands outside
the detected self-alias interval.

## Verification and limits

`reports/native_legacy_sbo_string_audit.json` records code/data byte hashes,
current source and artifact hashes, original names/comments, ABI, dependencies,
and the exact fixture boundaries. The private fixture verified live Ghidra
bytes against the installed PE after checking project `C:/Users/sqz269/bsp.gpr`
and program `/battlestationspacific.exe`. It executed the full six string spans,
the original secure copy/move bodies, and the native growth FuncInfo/try maps.
Its 289 normalized observations matched the reconstructed functions.

The fixture uses actual host allocation/free, errno, primitive copy/move/set,
invalid-parameter, typed length/range throw, and C++ EH services at the recorded
external boundaries. Native growth's handler immediate is checked and bridged
to an equivalent in-image `MOV EAX, FuncInfo; JMP __CxxFrameHandler3` entry;
the fixture uses `/SAFESEH:NO` so Windows can dispatch the private handler.
The native code and native EH data determine the catch continuations. This is
an original-byte control-flow comparison under a host CRT/EH bridge, not
execution of the game's whole CRT or original exception-object ABI.

The sequence covers inline/heap assignment, self-alias and substring paths,
zero assignment/erase, retained capacity, heap reset, length/range errors,
returning handlers that replace current data or alter length, growth-copy
validation, exact retry after both bad allocation and a throwing new handler,
second-failure reset/rethrow, zero-byte allocation, and the throwing copy path.
The latter deliberately follows the native abandoned-buffer behavior; private
fixture allocations are process-scoped. This is focused evidence, not an
exhaustive test of every numeric value, allocation outcome, or handler policy.

The new source passed a separate MSVC Win32 `/O2 /EHsc /fp:strict /W4 /WX`
compile and linked against the existing concrete core for that comparison.
`scripts/build.ps1` passed; after seed verification it passed both existing
tests. This worker did not edit shared CMake configuration, so adding the new
source to the core target belongs to integration. No gameplay or binary drop-in
compatibility is claimed.
