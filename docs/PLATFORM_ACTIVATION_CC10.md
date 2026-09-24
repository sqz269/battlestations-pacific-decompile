# Platform activation message fragment (CC10)

Addresses: `00BED3F3`, `00BED45D` within `00BED3B0`; recovered provider addresses
are listed in the two native focus module documents.

`handle_platform_activation_message_00bed3b0_fragment` implements the complete
`WM_ACTIVATE` arm as a partial projection of the larger message handler. It
returns false without effects for every other message. All host operations are
required interfaces. There are no default providers or invented successful
fallbacks. The primary application integration owns their actual bindings.

| Routine or fragment | Coverage |
| --- | --- |
| BED3B0 activation dispatch | Partial: owned BED3F3..BED507; consumes shared prefix BED3B0..BED3F2 and default return BED669..BED67D. Other arms BED508..BED668 and BED67E..BED761 remain outside this module. |
| A7A480, A7A4A0 typed adapters | Complete bodies; use canonical SoundSystemOwner fields and existing A7A3F0 provider. |
| Recovered native_window_focus_dispatch | Eight complete raw bodies, historical source restored; see its document for virtual target restrictions. |
| Recovered native_platform_focus_owners | Fourteen complete bodies; AA5260 specializes its fixed whole-range call into AA49E0. |
| AA49E0 fixed-call specialization | Partial: AA4A21..AA4A5E effects for AA5260's valid nonconcurrent fixed arguments; arbitrary-range AA4A60..AA4AA8 and returning validation inputs excluded. |

The native handler has five stack slots: explicit active receiver, HWND, message,
WPARAM and LPARAM, ending in `RET 14h`. The new typed interface is not that ABI.
The window-extra receiver returned by `GetWindowLongA(hwnd,0)` stays captured
across callbacks. The active receiver is independent. In the active path,
`SetForegroundWindow` precedes `SetFocus`, with the active HWND reloaded between
them. Active fullscreen is read again after focus, then F8D39C is read and tested.
The inactive path never changes `+40`; it stores the low WPARAM byte to `+41`,
which is necessarily zero because the branch tests the entire low word (`BX`).
The fullscreen comparison precedes this store. Fullscreen dimensions and Z order
come from the captured window-associated receiver, not the active receiver.

| Call sites in BED3B0 | Native operation and owner contract |
| --- | --- |
| BED3C5 | GetWindowLongA(hwnd,0), capture returned window-associated receiver |
| BED3FA / BED45F | C2F172 XInputEnable(0 / 1), stdcall RET4 |
| BED41B / BED47E | SetWindowPos(window HWND, NOTOPMOST / TOPMOST,0,0,width,height,20h), stdcall RET1Ch |
| BED427 / BED4A0 | A7A480 / A7A4A0, ECX=current F8BBD8, no stack arguments |
| BED42E / BED4A7 | 4C1710 media getter, no arguments, returns current owner in EAX |
| BED435 / BED4AE | A4C2D0, ECX=getter result; pause DWORD 1 / 0; exactly RET4 with no data reads |
| BED43C / BED48E | 4C12B0 GUI getter, no arguments, returns current owner in EAX |
| BED443 / BED495 | AA33A0, ECX=getter result; pause DWORD 1 / 0; RET4 |
| BED4B9 | B24FB0, ECX=current F8D394; external renderer-specific contract |
| BED4C6 / BED4D0 | SetForegroundWindow / SetFocus, separate active receiver's HWND with reload |
| BED4EE | B0D1E0, ECX=captured nonnull F8D39C after active fullscreen gate |
| BED450 / BED4FB / BED671 | DefWindowProcA with unchanged HWND/message/full WPARAM/LPARAM; preserve its result |

The pause DWORD is pushed **before** each media/GUI getter and remains on the
stack until the next consumer returns `RET4`. It is not a getter argument. The
getters' own `RET` and consumers' cleanup prove this despite misleading old
pseudocode. Sound wrappers each use `RET`; their A7A3F0 call pushes FFFF and that
callee returns `RET4`. BED3B0's EBX is the original WPARAM, EBP the original HWND,
EDI the message, and ESI the captured window receiver until BED4BE reloads the
explicit active receiver. These registers are callee-saved across provider calls.

Direct-callee bodies were inspected before naming contracts. Xrefs show the
sound wrappers, B24FB0 and B0D1E0 have only the listed activation caller. Other
AA33A0 callers at 52E21F/52EF83 pass 0/1. A4C2D0's additional callers use the same
getter/consumer stack shape; because its entire body is `RET4`, it neither reads
the receiver nor stores media pause state. B24FB0 tests 108D4BB, calls B22030 on
receiver+1A98, current receiver virtual+120, then tail-calls B21F70 on +1A74.
This packet does not rename or implement that renderer-specific behavior.

## Recovered source and production compatibility

The previously unmerged `9f62a71ac` and `f9e68cc13` modules were recovered by
individual file from repository history, not by merging their old branch.
Their header/source bytes are unchanged. Their documents and reports retain
the old evidence with an explicit recovery note. All dependencies compile
against the current providers. No Ghidra mutation occurred in this packet.

The raw sound functions require the native 178h storage. The application's
`SoundSystemOwner` is a semantic owner with canonical `flag_50` and `levels`
references, so this packet also supplies typed pause/resume adapters. They store
the flag before calling the existing `dirty_sound_classes_00a7a3f0` with FFFF.
No native offset cast or second sound state is introduced.

Recovered raw GUI/media getters require the shared native manager publication,
actual F8BC5C/F8AEF8 cells and canonical resource owner registry. They cannot be
fed semantic GUI objects or a detached empty GUI and then called production
movie propagation. The GUI traversal requires real raw page/child/decoder
storage, supported current numeric profiles, the readonly table owner and the
shipped BinkPause import. The primary integrator owns that producer/binding work.
Renderer B24FB0/B0D1E0 and XInput runtime bindings also remain the integrator's
responsibility. The media consumer's empty body is established native behavior;
omitting any of those other dependencies is not equivalent.

## Current verification and limits

Fresh read-only BSP CLI queries verify the configured `bsp.gpr` and
`/battlestationspacific.exe`. All 23 bodies (2,311 bytes) match the current
installed PE; the 22 recovered provider hashes also match their historical
receipts. The complete BED3B0 body is 946 bytes. Direct call report checks pass
for both recovered modules (15 and 22 rows).

An ignored MSVC Win32 fixture executes the original BED3B0 body with recorded
external call providers and rebased imports, singleton operands and jump table.
The activation instructions are unchanged. Twenty-four original/source pairs
cover low-word zero with a nonzero high word, activation values 1/2/100h/8000h,
both receiver fullscreen flags, optional render-service publication, replacement
of window-extra publication during positioning, HWND replacement during
foreground activation, and fullscreen change during SetFocus. Results, complete
call arguments/order and observed flag timing match. The same 24 pairs also pass
when linked against the actual worktree `bsp_core.lib`. An unrelated-message case
returns false without calls or result modification. The recording fixture is
not a real-window, real-audio, Bink-display or gameplay test. Historical provider
fixtures have not been rerun by this packet; their results remain historical.

The fixture's first compile caught unsigned-byte/bool comparison warnings; its
casts were corrected and the subsequent warnings-as-errors build and run pass.
The full strict Win32 `scripts/build.ps1` run passes all three existing CTests,
including native math differential after all eight native seeds matched.
Current full build/CTest results and artifact hashes are recorded in
`reports/platform_activation_cc10.json`. Runtime application claims must come
from the primary integration's real-window log, not this provider fixture.
