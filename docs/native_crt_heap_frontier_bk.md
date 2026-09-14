# Native CRT heap ownership and mode selection BK

Base `eaa1f87cc5594484494824afe5d1fa7f2efb5545`. Discovery only: no C++ changes,
Ghidra writes, game/OS experiments, tests or builds. Thirteen complete live/PE
spans total994 bytes. The report retains37 verified direct call/tail rows,
12 indirect import/register sites and three separately checked raw SEH calls.
Five external allocator spans came from constructor commit
`7e99c90a9f0311081dbb622122ade6f5c4b24d20`; their retained hashes and current PE
bytes were rechecked, without claiming a new live allocator capture.

## Original ownership and startup

The PE entry is BFD2BD: security initializer C1815E, then tail-jump BFD0DD.
BFD0DD temporarily allocates94h bytes through `HeapAlloc(GetProcessHeap(),0,94h)`
for OSVERSIONINFOA. It writes size94h and calls `GetVersionExA`. A failed query
frees the temporary and returnsFF without publishing the OS snapshot or creating
the CRT heap. A successful query captures platform, major, minor and masked
build, frees the temporary through the process heap, then publishes:

| Order | Actual word | Value |
|---|---|---|
| 1 | 109DD84 | reported platform |
| 2 | 109DD8C | `(major << 8) + minor`, wrapping DWORD |
| 3 | 109DD90 | reported major |
| 4 | 109DD94 | reported minor |
| 5 | 109DD88 | `build & 7FFFh`, OR8000h if platform is not2 |

After BFD09C, startup calls **C119BF(1)**. The resulting109E1BC is a separate
private CRT heap created by `HeapCreate(0,1000h,0)`. It is not the process heap
used for the version temporary, nor the rebuilt process's host `malloc` domain.
Original startup calls C053DC `__mtinit` only after this heap attempt. Failure
uses the original diagnostic/exit services; their complete behavior is external.

## Complete initializer and selector contracts

| Entry | Bytes | Interface |
|---|---:|---|
| C119BF | 90 | cdecl stacked DWORD; EAX0/1; RET |
| C11964 | 91 | no arguments; EAX1/3; RET |
| C11CF5 | 72 | cdecl stacked threshold; EAX0/1; RET |
| BFBAB2 | 55 | cdecl output DWORD pointer; EAX0/22; RET |
| BFBB61 | 60 | cdecl output DWORD pointer; EAX0/22; RET |

C119BF computes `HeapCreate` flags as1 for a zero incoming DWORD, otherwise0.
It publishes the returned heap handle before testing it. Null returns0 and
leaves mode unchanged. It calls C11964, publishes its result109ED7C, and only
mode3 calls C11CF5(3F8h). A failed SBH initializer calls `HeapDestroy` on the
current109E1BC, ignores the API result, clears that heap word and returns0.
**Mode remains3.** Non3 mode and successful mode3 return1. No fallback heap,
global rollback, additional cleanup or exception translation occurs.

C11964 calls BFBAB2 and BFBB61 into two zeroed local DWORDs. Each getter failure
is followed by BF65BB Watson with five zero arguments. After successful getters:

| Actual captured values | Selected mode |
|---|---:|
| platform==2 and **unsigned** major>=5 | 1 |
| other values | 3 |

The two getters reject a null output or current platform109DD84==0, write22 to
the owning BFFB8B errno location, invoke BF66EF with five zeros, and return22
if it returns. Neither failure writes output. Otherwise they copy the current
platform or major. Full selector bytes include `ADD ESP,14h` after both Watson
calls; these six bytes are absent from the current live listing due to no-return
metadata, and were retained through PE/live comparison and raw disassembly.

Microsoft documents platform2 as VER_PLATFORM_WIN32_NT. `GetVersionExA` may
report6.2 depending on the executable manifest, and compatibility settings can
change its reported OS. Major6 or10 satisfies the native test, but the installed
OS name alone is not evidence of current game globals. Mode1 remains conditional
on the actual reported/stored values. [GetVersionExA](https://learn.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getversionexa),
[OSVERSIONINFOA](https://learn.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-osversioninfoa).

## Small-block initialization without fabricated providers

C11CF5 uses actual `HeapAlloc(current109E1BC,0,140h)` and has no lock, TLS,
generic callback, EH frame or C++ object constructor. The140h bytes are not
zeroed here. Its complete success publication order is:

1. 109ED68 = allocation result, including null.
2. On nonnull only: 109E310 =0; 109ED64 =0.
3. 109ED70 = same allocation; 109ED6C = incoming threshold3F8h.
4. 109ED74 =10h; return1.

Null returns0 immediately after step1, leaving all other fields alone. Names
for these words beyond this observed schedule remain provisional. Initialization
does not establish the separate SBH allocation/free operations; those were not
expanded in this packet.

## Consequences for raw node allocation and the next packet

A structured node requests24h bytes, **36 decimal bytes**. Under actual mode1,
BF9F1A passes that unchanged to `HeapAlloc(current109E1BC,0,24h)`. BF9DC8 frees
nonnull non3-mode storage with the same heap through `HeapFree`. Mode3 first
uses the SBH allocation/release routes and lock4; fallback allocation rounds to
16 bytes. Host `malloc/free` and this private heap/SBH domain cannot be mixed.

Image-initial heap/mode/newmode words are zero. Bounded datarefs found only the
initializer writing heap/mode, startup writing platform/major, and four reads
with no direct writer for newmode109E314. This is static image/analysis evidence,
not an assertion that runtime values cannot change indirectly.

The smallest independent source packet is **C11CF5**, using the actual owning
heap and borrowed writable field bindings. Its success and failure schedules
are complete and require no new callback interface. **BFBAB2** can also extend
the existing BFBB61 error context without inventing a new service abstraction.

The combined **C119BF+C11964+C11CF5+BFBAB2**, reusing BFBB61, has bounded and
fully recovered mechanics for both modes. Its general error behavior is not
source-closed: BF65BB has no identified native source, and original OS globals,
owning errno/invalid-parameter services remain required. Existing BFBB61 and
C04FDE/C04F67 sources borrow these domains; they do not create original TLS/PTD
or handler ownership. Existing `singleton_lifetime_allocate/free` deliberately
uses host CRT services and supplies none of this private-heap ownership.

Later allocation failure still needs current newmode, decoded new-handler,
owning errno, and BF681B static bad_alloc/atexit identity. Free failure needs
owning errno and BFFB50 error mapping. These remain explicit prerequisites;
this discovery does not establish native CRT, EH/SEH, executable or gameplay parity.
