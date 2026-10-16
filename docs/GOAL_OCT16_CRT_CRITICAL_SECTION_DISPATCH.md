# Native critical-section dispatcher C17653

This packet reconstructs the complete physical 197-byte, 66-instruction helper,
including the returning Watson continuation and the real no-memory filter and
handler. The native library name remains `___crtInitCritSecAndSpinCount`.
The new C++ name is descriptive. The primary accepted the whole Source and
required the typed getter's output alignment qualification. The registered
MSVC Release Win32 build and full emitted-code review passed at the qualified
ordinary Source contract below.

The ordinary Source API borrows the actual encoded cache at `0109E454`, the
published codec/support contexts, live incoming argument cells and three raw
private payload cells. It creates no heap, lock, TLS/PTD or registered-handler
owner. Existing codec/support admission limits, including their owning-errno
and invalid-service boundary, remain in force. This packet does not close the
Original free/lock/errno/TLS SCC.

| Raw backing | Offset | Native role |
| --- | ---: | --- |
| parameters8 | 0 | current pointer DWORD at native EBP+8 |
| parameters8 | 4 | current spin DWORD at native EBP+C |
| private12 | 0 | exception code DWORD at native EBP-24 |
| private12 | 4 | result DWORD at native EBP-20 |
| private12 | 8 | platform DWORD at native EBP-1C |

Platform and exception code are distinct cells. Only platform is zeroed before
the decoder, including cache-hit paths. Result and code remain untouched until
their reached native writes. These actual cells may alias reached payload at
the provider contracts. Added binding identities and compiler bookkeeping,
including the new SEH frame, must remain stable and disjoint from that storage.
Only reached raw accesses need valid storage. The private12 backing must be
DWORD-aligned because its platform cell is passed to the existing OS-platform
getter as `uint32_t*` for an ordinary typed store. This inherited admission does
not claim arbitrary unaligned getter output. The raw argument access helpers
still support unaligned argument DWORDs. No runtime guard is added.

On a miss the helper calls the real OS-platform getter and canonical Watson.
A returning Watson call is followed by a fresh platform read. Selection uses
the actual kernel32 module/export or the concrete existing C17643 no-spin
provider. Encoding and cache publication precede the protected region. Cache
hits retain the actual decoded callable, and the selected raw pointer remains
captured across encoding. Source string and fallback code addresses are
relocated; numerical Original address identity is not claimed.

The native scope at E037B8 arms state 0 at C176C9 and disarms at C17708.
The Source `__try` covers only spin-then-pointer raw reads, the selected real
stdcall call and normal result publication. Decoder, getter, Watson, module,
encoder and cache operations remain outside it. The exact `C0000017` filter
reads the real Windows exception record, publishes its captured code, then
compares that captured value. Other exception codes continue search. The
handler rereads the exposed code cell, conditionally invokes real
`SetLastError(8)`, then performs one raw `AND DWORD [result],0`. That operation
reads and zeroes even previously uninitialized result bytes. The common return
rereads the current result cell outside the protected region.

Fresh live and installed-PE bytes match for all 197 code bytes and 28 scope
bytes. The report contains all 66 decoded physical instructions. Formal Ghidra
ownership remains 147 bytes in three ranges; the three returning-Watson bytes
and 47 filter/handler bytes are outside those ranges. The filter/handler are
not current InstructionDB functions. No body, flow, prototype or listing repair
is performed, and no native direct call to C17643 is invented from its selected
address. Seven real incoming calls from six callers push spin 4000 then a
pointer, clean eight bytes and inspect EAX; the API admits the full raw DWORD
input behavior rather than restricting itself to that observed spin value.

The implementation uses actual MSVC Win32 `__try/__except`, without a generic
callback, SDK CRT service, catch-all or C++ allocation exception substitution.
The compiler supplies a new Windows SEH registration and ExceptionPointers.
It does not reproduce Original C07C00/C07C90, FS state, private saved ESP, scope
state writes, stack homes, fault/code PCs or register/flag ABI. The exposed
private12 models only the three native mutable payload cells. No fake Original
frame write or stack-restore credit is taken. Source admission, emitted code,
normal build, Original ABI and game validation remain separate evidence gates.

The primary reviewed the whole CPP/header and all physical main/filter/handler
instructions. Its default `scripts/build.ps1` gate at
`d9e1fce24cc7b7475668db7d14245958ef0d5237` passed all three existing checks in
8.90 seconds. The CPP hash is unchanged. The tested header predates the later
alignment and x87 qualification comments; that delta changes no code. No worker
CMake change, build, probe or new tests were performed. Existing native math
checks do not execute this dispatcher; there is no new-entry, Original ABI or
game execution claim.

The saved annotation retains the library name and exact prior Visual Studio
library comment as its prefix. The tool recorded prior values under the Ghidra
write lock, appended evidence, saved the project and refreshed the formal
export. Readback confirms unchanged formal ranges and flow. Naming and
reconstruction ledger rows record the qualified whole Source with its normal
build evidence and remaining Original transport limits.

The retained COFF object contains all seven CODE bodies, 528 bytes and 199
decoded instructions: public main 406 bytes, standalone filter 57, read helper
19, write helper 13, and three 11-byte AND/address helpers. Full review confirms
platform zero before cache decode, returning Watson and fresh platform reads,
preparation/cache publication before state 0, spin read at main+E0 before pointer
read at +EB, selected call +F6 and result store +102 inside the region. Filter
+13A publishes the captured code compared at +141; handler +156 rereads current
code before real `SetLastError(8)` at +166, then +16F performs one memory AND-zero.
Disarm +172 precedes final current result read +17C.

The compiler inserts **FWAIT at main+F8**, immediately after the selected call,
inside the protected region. Ordinary semantic admission excludes a pending
enabled x87 exception that this extra wait could surface at that point,
including one left by the selected initializer. Original x87 pending-exception
and fault identity is unproved. No Source fix or probe was introduced.

The emitted scope table is 28 bytes, with state -2, exact filter relocation
main+106 and handler relocation main+150; its cookie offset is -76, versus -52
in native E037B8. The new compiler's `__except_handler4`, security cookie, saved
ESP and reused argument homes are distinct bookkeeping. The three exposed
private payload cells are not claims about those new EBP slots. Source public
relocations reach the four existing concrete providers, three actual Win32
imports and genuine fallback/string/scope/compiler symbols. All six helper
bodies have no relocations or calls. The primary confirms one identical archive
member for the retained object; this is static/build evidence only.
