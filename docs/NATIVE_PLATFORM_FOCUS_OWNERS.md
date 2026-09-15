# Native owners needed by window activation

Addresses: `004c12b0`, `004c1710`, `00aa5d70`, `00aa2820`, `00aa2920`,
`00a4c5a0`, `00a4c4a0`, `00a4c2d0`, `00aa0ff0`, `00a4c3f0`, `0041da80`,
`004c8020`, `00aa5260`, `00aa22c0`; fixed-call specialization within `00aa49e0`.

`native_platform_focus_owners` reconstructs the actual 88h GUI and 18h media
singleton owners required by window activation. Fourteen complete native bodies
have source implementations, with the valid whole-range call made by AA5260
specialized inside its implementation. Media is a provisional descriptive class
name. Its publication is F8AEF8; existing settings readers use +10 and +14.
GUI publication is F8BC5C. These are raw owners sharing the existing actual
01090AA0 manager and canonical resource-reference domain.

Both getters return their first captured nonnull publication immediately. The
slow path captures the first manager's section, enters and increments its extra
depth DWORD, then rechecks the publication. Owner allocation cleanup is armed
through construction and disarmed before publication and registration. A second
manager getter precedes reading the current published owner for registration.
The first captured section remains the guard receiver; normal Leave still runs
inside the guard's unwind state. The return reads publication after Leave.

GUI construction writes D5BFCC, allocates an 18h tree node, makes it a black
self-linked sentinel, initializes the vector and reference slots, allocates a
14h circular list sentinel, and captures CE3800 before its later stores. The
literal is **0.5**, stored at +5C and +60. The source preserves all 71 bytes not
assigned by this constructor. Media construction writes D24D94, allocates a 0Ch
list sentinel, captures D7A24C (**1.0**), and sets +8/+C/+10. It preserves +4..7
and +14..17. Allocation leaves preserve their native pointer tests and partial
payload writes; null allocation does not become an invented successful result.

GUI constructor FH3 states unwind slot +2C through 41DA80, vector +14 through
4C8020, tree +8 through AA5260, then base AA0FF0. The source arms the combined
member cleanup at native state3, before +30 and the list allocation; preceding
post-tree stores cannot throw a C++ exception. Media's sole constructor cleanup
is A4C3F0. Both base cleanups clear the **current** publication before resetting
the **original** receiver to CE3818, even if re-entry replaced the publication.
Getter allocation cleanup subsequently frees its captured allocation, then the
guard unwinds. There is no unregister call in these base destructors.

41DA80 captures a raw reference, decrements actual +4, dispatches current
virtual0 only on zero through canonical ownership, and then clears the slot.
An initially null slot is untouched. The existing canonical terminal callback
is nonthrowing. 4C8020 frees captured vector +4 before zeroing +4/+8/+C.
AA22C0 recursively erases the right subtree, reads left after that recursion,
frees the old node and iterates left; byte +15 marks the sentinel. AA5260's
fixed arguments select AA49E0's whole-range branch for valid nonconcurrent
storage. It erases root, resets head links/count in the listing order, frees
the current sentinel and clears header head/count. Arbitrary AA49E0 iterator
inputs, its returning validation paths and single-node branch are not exposed
or claimed. The output iterator is native private scratch.

A4C2D0 is exactly `RET 4`: it reads neither ECX nor the argument. The original
window handler pushes its pause word before calling the media getter. The
getter must still execute even though the following pause operation is a no-op.

## Evidence and validation

Report: `reports/native_platform_focus_owners.json`. All 978 bytes of the 14
bodies match fresh Ghidra memory and the unchanged installed PE. The full
201-byte AA49E0 analysis and 367 bytes of FH3 descriptors, unwind maps, funclets,
profiles and literals are separately retained. All four FH3 state counts/maps
are mechanically decoded and checked. There are 22 direct call rows, including
the specialized whole-range erase call, and six separately recorded indirect
calls. Free fallthrough gaps and stored body ranges for AA22C0, 4C8020 and
AA5260 were repaired under the shared write lock with before-state records.

`scripts/build.ps1` passes with warnings as errors and both existing CTests.
No permanent test was added. A fresh copy of the closed render-entry-cache
fixture compiles 24 consistent explicit source units and selects 23 at link;
all remaining providers come from the current production libraries.

The fixture checks the untouched constructor bytes, sentinel links/flags,
literal values, original receiver returns, actual slow/fast publication,
exactly one manager registration per owner, and pause no-op. Two probe-only
malloc/free import hooks fail the GUI list or media list allocation through
the real CRT bad_alloc path. Before failure, the hook replaces the publication.
The trace proves GUI sentinel free before base publication clear, original
owner free after base reset/publication clear while depth is still one, and
final guard depth zero. Imports, page protection and CRT new handler are
restored afterward. The production source and game instructions are unchanged
by instrumentation. One probe compile failure (DWORD pointer mismatch) and
its source are retained; compile/link attempt2 and execution capture1 pass.

The same run continues through actual platform, renderer, device/focused Reset,
10,000 render-entry records, installed cold DDS and effects, control-worker
exit, focus/timer restoration and explicit cache destruction. Original code
execution remains the previous 39-byte record leaf, 64-byte x87 fragment and
306-byte empty descriptor branch; no new original focus body is replayed.

## Boundaries and follow-up packets

Source context arguments are new interfaces, not original register/stack/FH3
ABI. Numeric profiles retain identity and are not callable host vtables.
Source C++ allocation unwind is exercised; original FH3 execution, hardware
faults, second cleanup exceptions, concurrent mutation, invalid storage,
registration failure and populated reference/vector/tree cleanup are not
validated. Detached empty owners receive explicit fixture disposal; the actual
published owners remain live. Full normal GUI/media destruction is outstanding.

The real WndProc thunk BEC3B0 calls BED3B0 with five stack arguments; BED3B0
is callee-cleans-five (`RET 14h`), while full window initializer BECEE0 is
cdecl11. The full message handler still needs actual sound-manager production,
GUI page pause AA33A0/AA8E40/AAC8A0, sound pause A7A480/A7A4A0/A7A3F0,
renderer refresh B24FB0/B22030/B21F70 and service restore B0D1E0 dependencies.
Window creation, active frame/draw, full shutdown and gameplay remain unproved.

Immutable closure: `local/checkpoints/9ed3a6de/native-platform-focus-owners/validation.json`, SHA-256 `5328da4539e2a7cb51b09f8ac5e27716089701c3cd67241d0d5d298ed81ec870`. It retains 4975 artifacts, 81 physical Win32 modules and 307 selected production source providers. Captured docs/report precede this closure metadata addition.
