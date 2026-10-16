# Native critical-section dispatcher C17653

This packet reconstructs the complete physical 197-byte, 66-instruction helper,
including the returning Watson continuation and the real no-memory filter and
handler. The native library name remains `___crtInitCritSecAndSpinCount`.
The new C++ name is descriptive. Source review and normal build are pending.

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
Only reached raw accesses need valid storage; no whole-table or alignment guard
is added.

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

No annotations, CMake changes, worker build or new tests precede the primary's
full Source review. Later compilation must verify MSVC SEH constraints and the
actual protected range, live reads, filter comparison and raw handler RMW.
