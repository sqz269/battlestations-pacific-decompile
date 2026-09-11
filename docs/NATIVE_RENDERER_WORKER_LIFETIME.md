# Native renderer embedded worker lifetime

This source packet reconstructs five complete entries, totaling 409 original
bytes. The name describes the observed stop/completion protocol; it does not
recover the original class name, worker body, or complete allocation size.

| Entry | Complete original extent | Original ABI | Reconstructed entry |
|---|---|---|---|
| B5E270 | B5E270..B5E2E8, 121 bytes | ECX owner, EAX owner, RET | `construct_native_renderer_worker_00b5e270` |
| B5E2F0 | B5E2F0..B5E372, 131 bytes | ECX owner, RET, no semantic result | `destroy_native_renderer_worker_00b5e2f0` |
| B5E050 | B5E050..B5E0B5, 102 bytes | ECX owner, RET, no semantic result | `stop_native_renderer_worker_00b5e050` |
| BD1860 | BD1860..BD1886, 39 bytes | no arguments, EAX tracked section, RET | `create_native_tracked_critical_section_00bd1860` |
| 415270 | 415270..41527F, 16 bytes | ECX header, EAX header, RET | `initialize_native_string_header_00415270` |

The owner constructor/destructor add a fixed EDX context. It borrows only an
`ActualNativeStringPoolStorage` adapter using the application's one actual pool
publication, shutdown gate, and canonical lifetime domain with the concrete
pool binding installed. All must outlive the owner. It introduces no ownership,
allocator, OS, or worker callbacks. The other raw entries retain their stated
register/stack contracts, but no installed binary replacement is claimed.

## Actual storage and construction

The caller B32410 passes renderer+1D2C at B32756/B32764. The observed accessed
prefix is 54h bytes: five DWORDs at +00..+10, five eight-byte string headers at
+14..+3B, DWORDs +3C/+40, bytes +44/+45, tracked-section pointers +48/+4C,
and handle +50. Bytes +46/+47 remain untouched. This establishes neither a
complete record size nor the identity of the unresolved logical-stream +4C
terminal. This embedded owner's +4C has a positively identified tracked lock.

415270 writes length zero and then data zero, returning the header. B5E270
constructs exactly five consecutive headers. A completed prefix advances only
after each element returns. It then clears +3C, +40, byte44, byte45, and +50 in
that order. Its outer unwind state becomes zero at B5E2B5, after those stores.
The following five DWORD stores clear +00, +04, +08, +0C and +10. It creates and
publishes the first lock at +48, then creates and publishes the second at +4C.
No old string, lock or handle is released when this constructor is called on
previously populated storage; callers must provide its raw construction domain.

Fresh FH3 evidence is CC1156 -> DF9E5C, one-state map DF9E54 -> CC1140.
CC1140 destroys only the five headers through the reverse vector helper.
Consequently a second allocation failure leaves the first lock published and
allocated, with +4C still holding its previous bits. Source rollback deliberately
preserves this leak. Cleanup reads the current string headers, including a
header changed by the current new handler after construction.

The two fixed five-header loops specialize this owner only. They do not claim
generic BF7CD1, BF7C6E or BF7C10 reconstruction. Their complete original bodies
and static cleanup entries are retained as evidence. The source tracks completed
elements, but 415270 has no throwing C++ operation on valid storage, so MSVC
eliminates unreachable inner C++ cleanup. Partial-header hardware faults and the
original compiler SEH filters/unwind/terminate ABI remain outside this interface.

## Allocation and operating-system boundaries

BD1860 allocates exactly 1Ch raw bytes through the existing
`singleton_lifetime_allocate({critical_section, 0x1c, 0x1c})` service. Both the
native and host sizes are 1Ch; no larger `OwnedCriticalSection` projection is
used. The accepted BF681B host boundary calls current CRT malloc, calls the
current new handler after failure, retries when that handler returns nonzero,
and throws `std::bad_alloc` otherwise. The old random-thread creation wrapper
uses `new(std::nothrow)` and is deliberately not this provider.

The raw routine preserves the original null-result branch, calls real
`InitializeCriticalSection` for a nonnull result, and writes depth+18=0 only
after that call returns. It adds no allocation cleanup if the OS raises an SEH
exception. `singleton_lifetime_free` supplies the matching current CRT free
boundary corresponding to BF65AC's jump to BF9DC8. Native exception objects,
CRT globals and heap ABI compatibility are not claimed by that host boundary.

B5E050 reads current +50 first; zero skips all lock and OS work. Otherwise it
captures EnterCriticalSection once, captures current +4C for entry and increments
that captured lock's depth, then rereads current +4C, sets byte44=1, decrements
the current lock's depth and leaves it. Every polling iteration calls Sleep(10)
before entry, captures and enters current +4C, increments the captured depth,
rereads +4C, snapshots byte45, decrements/leaves the current lock, and tests the
snapshot after return. A nonzero snapshot closes current +50. The handle is
never cleared; API return values are ignored. This is a completion-byte protocol,
not a WaitForSingleObject join or a recovered worker-thread implementation.

B5E2F0 runs full stop first, then captures +48. For each nonnull lock it drains
only positive signed depth, decrementing before each real LeaveCriticalSection,
then deletes and frees it. It rereads +4C only after first-lock teardown. It
never clears either published word, and finally destroys the five current
headers in reverse order through full 41DD20 and the actual pool adapter. The
header destructor also leaves its header unchanged. Caller synchronization and
valid current locks/handles are required; no repair for inconsistent depth,
concurrent retirement, stale handles, or cross-thread lock ownership is added.

`NativeStringStorage::release` already has a noexcept contract. Its actual-pool
adapter may terminate if lazy pool recreation throws. This packet neither
strengthens that existing interface nor claims the original generic array
helper's primary/secondary exception behavior outside the returning release
domain. The public destructor does not claim deletion/freeing of the owner.

## Verification and remaining boundary

The ignored strict Win32 CMake hook adds only this source. The build passed both
existing CTests and all eight original differential seeds. Fresh guarded Ghidra
queries matched the installed PE for 21 spans / 1,437 bytes, including all five
entries, full owned FH3/static cleanup data, vector-helper evidence, allocation,
string destruction/pool providers, and the embedded-owner caller span.

The complete actual worker `bsp_core.lib` was frozen before fixture linking:
SHA256 `a148bcbf7b2add6251c380ff42a191a4bcd62ed6715ceb8a65b6ed14de820da9`.
The five exact archive members and 19 recursive source/header files are pinned.
All 220 mapped COFF sections (136 from the library), all 665 relocations, and
all 13,411 runtime executable bytes were checked. This includes the entire
compiled constructor, destructor, fixed helpers and C++ exception data, not
only the three literal routines. The 16/39/102-byte header/lock/stop entries
match every original byte except documented concrete provider relocations.
All 72 observed import targets matched their actual DLL images with only
loader relocations. The map parser strips all leading `f` and `i` flags.

One ignored actual-library lifecycle probe provides the concrete behavioral
check. A fixture import observer calls real malloc first and injects two
subsequent failures for the second 1Ch creation. The actual first new handler
changes the last raw header and installs a second handler; the real allocator
retries and invokes that current replacement, whose zero result produces
`bad_alloc`. The source preserves the first initialized lock and the +4C
sentinel, performs no lock free, and releases the current header through the
actual pool. The fixture then reclaims the intentionally retained first lock.
A normal construction/destruction uses real critical sections, two held levels
on the first lock, a fixture completion thread, and five real pooled strings.
It confirms stop/completion bytes, closed current handle, five pool returns,
unchanged opaque bytes, and stale published lock/handle/header postimages. A
duplicated thread handle lets the fixture wait separately for its thread return.

The probe does **not** execute the original bodies. It does not exercise native
SEH/partial-header faults, InitializeCriticalSection failure, mutations during
OS calls, malformed lock state, or generic vector-helper exceptions. Source
ordering for those current-field reads is grounded in assembly/complete linked
code, not an unperformed runtime case. No gameplay, complete renderer lifecycle,
native worker body, or native caller ABI validation is claimed.

All evidence is sealed under ignored `local/renderer_worker_lifetime/`; the
tracked audit records the capture/proof hashes and exact provider map. No shared
CMake, Ghidra, names, ledgers, original game files, or permanent tests changed.
