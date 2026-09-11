# Actual renderer synchronization leaves

This packet reconstructs five complete native leaves against borrowed original
storage. The APIs use the actual raw mode bytes, nesting DWORD, renderer pointer
fields, 1Ch tracked critical sections and 8-byte local guard record. They add no
owning renderer model, lock provider, initialization policy or synthesized guard
constructor. Names and the new C++ signatures are descriptive reconstruction
interfaces, not recovered symbols or binary entry replacements.

| Original span | Native ABI and effect |
| --- | --- |
| `00B33AA0..00B33AB0` (17 bytes) | Stack argument low byte; `RET 4`; ECX unused. Store the same raw byte to `0108D6DC` and `0108D6DD`. |
| `00B33AD0..00B33AFD` (46 bytes) | ECX actual renderer; `RET`; only AL is the result. Increment global nesting first, then optionally enter the actual lock pointer at renderer+04. |
| `00B33B00..00B33B42` (67 bytes) | ECX actual renderer; `RET 4`. Ignore the saved result stack word, decrement global nesting, and optionally leave the current renderer+04 lock according to current mode. |
| `00B21110..00B21125` (22 bytes) | ECX actual guard record; `RET`. Test current global mode before reading the record, then call `00B33B00` with the saved renderer and zero-extended saved byte. |
| `00B20220..00B20230` (17 bytes) | ECX actual renderer; `RET`; AL is signed `(depth > 0)` for the separate lifecycle lock pointer at renderer+199Ch. |

All 169 bytes were freshly compared between saved Ghidra project
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, and installed PE
`I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe`.
The installed PE SHA-256 was
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The audit stores complete preimages, individual hashes, actual return
instructions and the verification timestamp. Current Ghidra prototypes still
use `undefined ...(void)` and do not establish these ABIs; assembly does.

## Borrowed storage and ordering

`NativeRendererSynchronizationGlobals` is exactly 8 bytes: raw volatile BYTE
mode at +00, raw volatile BYTE observed mode at +01, two preserved bytes at +02,
and a volatile wrapping DWORD nesting counter at +04. These correspond to
`0108D6DC`, `0108D6DD` and `0108D6E0`. The type has no constructor or member
initializers. Mode values such as `80h`, `81h` and `7Fh` retain their bits; only
the branches test zero versus nonzero. Counter updates remain separate,
non-atomic DWORD operations, with unsigned storage preserving native wrap.

`NativeRendererOptionalGuardStorage` is exactly 8 bytes: raw saved entry BYTE
at +00, three untouched bytes at +01, and actual renderer pointer at +04. Native
callers conditionally store the renderer before entry, then store returned AL.
Skipping entry leaves the record uninitialized. This packet does not add a
constructor, automatic entry or unconditional initialization to change that
behavior. The cleanup leaf first checks *current* global mode. Disabled cleanup
returns without reading either saved field or decrementing nesting. Enabled
cleanup reads the byte before the renderer pointer and does not change the
record. If mode becomes enabled after a skipped entry, the original path can
read an uninitialized record; the reconstruction does not silently repair it.

Entry increments nesting before reading mode. With mode disabled it does not
read the renderer. Otherwise it captures renderer+04; a null pointer returns
AL=0 after the nesting increment. For a nonnull lock, the actual
`EnterCriticalSection` receives the captured lock's native +00 storage, then
the captured lock's raw DWORD at +18 increments. The pointer is not reloaded
after the API call. The existing `TrackedCriticalSection` type supplies the
real Win32 18h `CRITICAL_SECTION` plus its +18h depth word; compile-time layout
checks enforce the original 1Ch size. Arithmetic addresses the depth as raw
unsigned bits, including `FFFFFFFFh -> 00000000h`.

Leave loads mode before decrementing nesting. A nonzero mode enables a fresh
read of the current renderer+04 pointer, regardless of the saved stack word.
With a nonnull pointer it decrements the actual +18h word *before* calling real
`LeaveCriticalSection`. Assembly at `00B33B23` reloads mode only after an actual
Leave call. Disabled mode and null-lock paths retain the earlier mode byte.
At nesting zero, the observed byte is compared with this retained or reloaded
value and written only when different. At nonzero nesting it remains untouched.

The lifecycle observer reads renderer+199Ch and that lock's signed +18h depth.
It does not use the optional lock at +04, acquire a lock, check null, modify
storage or convert the signed depth test into an unsigned/nonzero test. Values
`80000000h` and `FFFFFFFFh` return AL=0. Only the low result byte is specified
by the original leaves; upper EAX bits are not promoted to canonical results.

## Focused differential verification

One private fixture in ignored `local/native_renderer_sync_check.cpp` invokes
the five complete original spans copied from the installed executable only
after the fresh comparison and the existing eight-seed guard passed. It keeps
the original `00B21110 -> 00B33B00` relative CALL intact. Thirteen verified
absolute memory operands relocate only the three actual global addresses and
two native Win32 IAT cells. No instruction body, internal helper or conditional
branch is replaced. Code pages are RX, unused code bytes are INT3, unused image
pages remain inaccessible, and the globals and IAT use separate RW pages.

Both the copied native IAT cells and the reconstructed object's host import
cells route through the same two private observation wrappers, each of which
calls the actual Win32 `EnterCriticalSection` or `LeaveCriticalSection`. The
wrappers neither fake API results nor supply substitute locks. They record
state around actual calls; controlled field changes after those calls establish
the captured-lock and post-Leave mode-reload ordering. The fixture restores its
host import cells before exiting. It never modifies the installed game.

The single native-versus-C++ sequence agrees on all 345 event words and checks:

- raw setter bytes and preservation of global padding and nesting;
- disabled entry and direct leave with an inaccessible renderer pointer;
- null optional lock and cleanup of a noncanonical saved entry byte;
- a real critical-section entry whose wrapper changes renderer+04 afterward,
  proving that the captured first lock's depth increments;
- the original cleanup caller passing saved zero while current mode still
  requires leaving, and a post-Leave mode change observed at nesting zero;
- simultaneous DWORD wrapping in nesting and tracked depth, with real lock
  acquisition/release balanced independently of the raw tracked value;
- disabled cleanup with the actual uninitialized guard page set NOACCESS,
  followed by an exact eight-byte preservation comparison;
- signed lifecycle depths `80000000h`, `FFFFFFFFh`, zero, one and `7FFFFFFFh`
  through the distinct +199Ch pointer, without writes or acquisition.

Unmodeled renderer bytes and all preserved guard/global padding remain intact.
The fixture compiles the actual production source with MSVC Win32 C++17,
`/W4 /WX /O2 /fp:strict`, and executes two real Enter/Leave pairs per sequence.
It adds no tracked test case or test framework. Full repository build and
existing CTest results, private artifact hashes and exact commands are recorded
in `reports/native_renderer_synchronization_actual_audit.json`.

## Evidence limits and integration

These are complete leaf reconstructions and a focused native differential,
with the original cleanup caller executed. The fixture's entry setup follows
the established caller contract; it does not execute an entire original
renderer worker, stop pipeline, device lifecycle or outer exception frame.
None of these five spans contains FS access, x87 instructions or an exception
registration/unwind prolog. This packet does not claim those surrounding
systems, concurrent mode transitions or gameplay are validated. Volatile
access preserves the selected Win32 field operations; it does not add an
atomic or portable C++ concurrent synchronization guarantee.

The primary integrator owns source registration, sharded ledgers, Ghidra
annotations and broader renderer integration. This packet changes exactly its
header, source, this document and its audit. The audit retains prior Ghidra
names/comments for those later annotation updates, including the old setter
comment's semantic boolean wording and the previously unnamed cleanup leaf.
